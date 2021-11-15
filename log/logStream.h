#ifndef LOGSTREAM_H
#define LOGSTREAM_H

#include "logger.h"
#include <stdio.h> // for test
#include <unistd.h> // for pid_t
#include <string.h>
#include <string>

namespace CurrentThread {
    pid_t gettid();
}
class LogStream {
public:
    using self = LogStream;
    // stream 最终 流 到什么地方，由 outputFunc 决定
    using outputFunc = void (*)(const char *, int);
    using flushFunc = void (*)();
    static void setOutputFunc(outputFunc f);
    static void setFlushFunc(flushFunc f);

    LogStream();
    ~LogStream();

    // 制作日志
    // 日期 时间 线程id 日志级别 源文件名:行号 - 正文 
    // 正文前面的可以在 makeLog 中完成
    // 正文由用户输入完成
    void makeLog(const char *filename, int line, Logger::LEVEL level);
    void makeLog(const char *filename, int line, Logger::LEVEL level, const char *func);

    // 将 buffer 中内容刷新到 日志后端
    void flush();

    // format integer
    template <typename T>
    std::string formatInteger(T v);
    // format time
    char *formatTime();

    // 功能函数
    void reset() { _cur = _buf; }
    int avail()  { return end() - _cur; }
    const char* end() { return sizeof(_buf) + _buf; }

    // over load operator<<
    self& operator<<(bool v) {
        append(v ? "T" : "F", 1);
        return *this;
    }
    self& operator<<(short);
    self& operator<<(unsigned short);
    self& operator<<(int);
    self& operator<<(unsigned int);
    self& operator<<(long);
    self& operator<<(unsigned long);
    self& operator<<(long long);
    self& operator<<(unsigned long long);
    // self& operator<<(const void*);
    self& operator<<(float v) {
        *this << static_cast<double>(v);
        return *this;
    }
    self& operator<<(double);
    self& operator<<(char v) {
        append(&v, 1);
        return *this;
    }
    self& operator<<(const char* str) {
        if (str)
            append(str, strlen(str));
        else
            append("(null)", 6);
        return *this;
    }
    self& operator<<(const unsigned char* str) { return operator<<(reinterpret_cast<const char*>(str)); }
private:
    // append 往 _buf 中追加字符
    // operator<< 基于此实现
    void append(const char *p, int len) {
        if (static_cast<size_t>(avail()) > len) {
            memcpy(_cur, p, len);
            _cur += len;
        }
        else {
            // 往日志后端发送数据，并更新 buffer
            // 如果一条日志制作到一半突然 buffer 空间不够怎么办？
            // 把 _buf ~ _end 的内容拷贝给后端，再把 _end ~ _cur 的内容移到 _buf 最前面并重新执行以下指令就行了
            // for test:
            // 测试时，用户不断写日志，必定导致 buffer 不够，此时将 _buf ~ _end 的内容输出到控制台查看是否有误
            // 前端的 flush 是将 buffer 中数据刷新到后端去。
            flush();

            memmove(_buf, _end, (size_t)(_cur - _end));
            _cur = _buf + static_cast<size_t>(_cur - _end);
            _end = _buf;
            append(p, len);
        }
    }
private:
    // buffer 我们假设一条日志的长度不超过 100 字节，Buffer 可缓存 1000 条日志
    static const int kBufferSize = 1000 * 100; // 100KB
    char      _buf[kBufferSize];
    char*     _cur; // 用于 append 到 buffer 时进行位置定位
    char*     _end; // 用于记录完整的日志尾部，_buf ~ _end 的每一条日志都是完整的
};

#endif 
