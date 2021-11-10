#include "logStream.h"
#include "timestamp.h"
#include <sys/time.h>
#include <time.h>
#include <algorithm>

__thread char t_formatTime[64];

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

LogStream::LogStream() 
    : _cur(_buf),
      _end(_buf)
{}


void LogStream::makeLog(const char *filename, int line, Logger::LEVEL level) {
        // _end 用于标记 最后一条完整日志的结尾
        _end = _cur;
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

char *LogStream::formatTime() {
    char tmp[40];
    int64 microSecs = Timestamp::now().microSeconds();
    int64 secs = microSecs / Timestamp::kMicroSecondsPerSecond;
    int64 ms = microSecs % Timestamp::kMicroSecondsPerSecond / 1000;
    struct tm *ptm = localtime(reinterpret_cast<time_t*>(&secs));
    strftime(tmp, sizeof(t_formatTime), "%Y-%m-%d %H:%M:%S", ptm);
    snprintf(t_formatTime, sizeof(t_formatTime), "%s.%03lld", tmp, ms);
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

