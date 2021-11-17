#include "logStream.h"
#include "timestamp.h"
#include <sys/time.h>
#include <time.h>
#include <algorithm>
#include <stdio.h>

__thread char t_formatTime[64];
__thread int64 t_lastSecs;
__thread int  t_needFlush = 0;
extern __thread struct tm *t_tm;


const char digits[] = "0123456789";
const char *levels[] = {
    "NONE",
    "TRACE",
    "INFO",
    "DEBUG",
    "WARN",
    "ERROR",
    "FATAL",
};

void defaultOutputFunc(const char *msg, int len) {
    size_t n = fwrite(msg, 1, len, stdout);
}
void defaultFlushFunc() {
    fflush(stdout);
}

// 由于 g_output 是全局变量，因此会在编译后便被初始化
LogStream::outputFunc g_output = defaultOutputFunc;
LogStream::flushFunc  g_flush  = defaultFlushFunc;

void LogStream::setOutputFunc(LogStream::outputFunc f) { g_output = f; }
void LogStream::setFlushFunc(LogStream::flushFunc f) { g_flush = f; }

LogStream::LogStream() 
    : _cur(_buf),
      _end(_buf)
{}

// 析构前必须刷新缓冲区
LogStream::~LogStream() {
    // printf("go die!\n");
    _end = _cur;
    flush();
}

void LogStream::flush() {
    g_output(_buf, static_cast<size_t>(_end - _buf));
    g_flush();
}

void LogStream::makeLog(const char *filename, int line, Logger::LEVEL level) {
        // _end 用于标记 最后一条完整日志的结尾
        _end = _cur;
        if (t_needFlush == 1) {
            flush();
            _cur = _end = _buf;
            t_needFlush = 0;
        }
        *this << formatTime()
              << ' ' << CurrentThread::gettid()
              << ' ' << levels[(int)level]
              << ' ' << filename
              << ':' << line << " - ";
}


void LogStream::makeLog(const char *filename, int line, Logger::LEVEL level, const char *func) {
        makeLog(filename, line, level);
        _cur -= 3; // " - "
        *this << ' ' << func << " - ";
}

template<typename T>
std::string LogStream::formatInteger(T v) {
    std::string s;
    T i = v;
    do {
        s.push_back(digits[i%10]);
        i /= 10;
    } while(i != 0);
    if (v < 0)
        s.push_back('-');
    std::reverse(s.begin(), s.end());
    return s;
}


int FastSecondToDate(const time_t& unix_sec, struct tm* tm, int time_zone);

char *LogStream::formatTime() {
    char tmp[40];
    int64 microSecs = Timestamp::now().microSeconds();
    int64 secs = microSecs / Timestamp::kMicroSecondsPerSecond;
    int64 us = microSecs % Timestamp::kMicroSecondsPerSecond;
    if (secs != t_lastSecs) {
        // need to flush log to back-end
        if (t_lastSecs != 0)
            t_needFlush = 1;
        t_lastSecs = secs;
        FastSecondToDate(static_cast<time_t>(secs), t_tm, 8);
        // localtime 开销太大了！
        // struct tm *ptm = localtime(reinterpret_cast<time_t*>(&secs));
        strftime(tmp, sizeof(t_formatTime), "%Y-%m-%d %H:%M:%S", t_tm);
        // snprintf 也有性能提升的空间，自己实现一个简单的
        snprintf(t_formatTime, sizeof(t_formatTime), "%s.%06lld", tmp, us);
    }
    else {
        snprintf(t_formatTime + 20, 7, "%06lld", us);
    }
    return t_formatTime;
}

LogStream& LogStream::operator<<(short v) {
    *this << (static_cast<int>(v));
    return *this;
}
LogStream& LogStream::operator<<(unsigned short v) {
    *this << (static_cast<unsigned int>(v));
    return *this;
}
LogStream& LogStream::operator<<(int v) {
    *this << formatInteger(v).c_str();
    return *this;
}
LogStream& LogStream::operator<<(unsigned int v) {
    *this << formatInteger(v).c_str();
    return *this;
}
LogStream& LogStream::operator<<(long v) {
    *this << formatInteger(v).c_str();
    return *this;
}
LogStream& LogStream::operator<<(unsigned long v) {
    *this << formatInteger(v).c_str();
    return *this;
}
LogStream& LogStream::operator<<(long long v) {
    *this << formatInteger(v).c_str();
    return *this;
}
LogStream& LogStream::operator<<(unsigned long long v) {
    *this << formatInteger(v).c_str();
    return *this;
}

int FastSecondToDate(const time_t& unix_sec, struct tm* tm, int time_zone)
{
    static const int kHoursInDay = 24;
    static const int kMinutesInHour = 60;
    static const int kDaysFromUnixTime = 2472632;
    static const int kDaysFromYear = 153;
    static const int kMagicUnkonwnFirst = 146097;
    static const int kMagicUnkonwnSec = 1461;
    tm->tm_sec  =  unix_sec % kMinutesInHour;
    int i      = (unix_sec/kMinutesInHour);
    tm->tm_min  = i % kMinutesInHour; //nn
    i /= kMinutesInHour;
    tm->tm_hour = (i + time_zone) % kHoursInDay; // hh
    tm->tm_mday = (i + time_zone) / kHoursInDay;
    int a = tm->tm_mday + kDaysFromUnixTime;
    int b = (a*4  + 3)/kMagicUnkonwnFirst;
    int c = (-b*kMagicUnkonwnFirst)/4 + a;
    int d =((c*4 + 3) / kMagicUnkonwnSec);
    int e = -d * kMagicUnkonwnSec;
    e = e/4 + c;
    int m = (5*e + 2)/kDaysFromYear;
    tm->tm_mday = -(kDaysFromYear * m + 2)/5 + e + 1;
    tm->tm_mon = (-m/10)*12 + m + 2;
    tm->tm_year = b*100 + d  - 6700 + (m/10);
    return 0;
}
