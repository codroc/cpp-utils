#include "logStream.h"
#include "timestamp.h"
#include "currentThread.h"

#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>

__thread char t_formatTime[64]; // 用于缓存 1s 内的 “日期 时间” 格式
__thread int64 t_lastSecs;
__thread int  t_needFlush = 0;
extern __thread struct tm *t_tm;

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
    fwrite(msg, 1, len, stdout);
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
    _end = _cur;
    flush();
}

void LogStream::flush() {
    g_output(_buf, completeLogLen());
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

void LogStream::flushLine(char* ppos) {
    if (_buf != ppos) {
        ppos++; // 往后移动一个字符，及前一个字符是 '\n'
        g_output(_buf, ppos - _buf);
        g_flush();
        size_t len = (size_t)(_cur - ppos);
        ::memmove(_buf, ppos, len);
        _cur = _buf + len;
        _end = _buf + (_end - ppos);
    }
}

void LogStream::append(const char *p, int len) {
    if (static_cast<int>(avail()) > len) {
        memcpy(_cur, p, len);
        _cur += len;
        if (Logger::GetBufferLevel() == Logger::kLineBuffer) {
            // 定位 '\n' 在 buf 中最后一次出现的位置 pos，把 pos 之前的字符全部刷新到后端 buf
            char *tmp = _cur;
            while (tmp-- > _buf) {
                if (*tmp == '\n')
                    break;
            }
            flushLine(tmp);
        }
    }
    else {
        // 往日志后端发送数据，并更新 buffer
        // 如果一条日志制作到一半突然 buffer 空间不够怎么办？
        // 把 _buf ~ _end 的内容拷贝给后端，再把 _end ~ _cur 的内容移到 _buf 最前面并重新执行以下指令就行了
        // for test:
        // 测试时，用户不断写日志，必定导致 buffer 不够，此时将 _buf ~ _end 的内容输出到控制台查看是否有误
        // 前端的 flush 是将 buffer 中数据刷新到后端去。
        flush();

        memmove(_buf, _end, remainUncompleteLogLen());
        _cur = _buf + remainUncompleteLogLen();
        _end = _buf;
        append(p, len);
    }
}

// 用户态的 函数
// 根据 gettimeofday 返回的 micro seconds 来计算 年月日 时分秒
int FastSecondToDate(const time_t& unix_sec, struct tm* tm, int time_zone);

char *LogStream::formatTime() {
    char tmp[40];
    int64 microSecs = Timestamp::now().microSeconds();
    int64 secs = microSecs / Timestamp::kMicroSecondsPerSecond;
    int64 us = microSecs % Timestamp::kMicroSecondsPerSecond;
    if (secs != t_lastSecs) {
        // need to flush log to back-end
        if (t_lastSecs != 0 && Logger::GetBufferLevel() == Logger::kFullBuffer)
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

// format hex
const char hex[] = "0123456789abcdef";
std::string formatHex(unsigned long long v) {
    unsigned long long tmp = v;
    std::string ret;
    while (tmp) {
        ret.push_back(hex[tmp%16]);
        tmp /= 16;
    }
    ret.append("x0");
    std::reverse(ret.begin(), ret.end());
    return ret;
}

LogStream& LogStream::operator<<(void* pv) {
    *this << formatHex((unsigned long long)pv).c_str();
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
