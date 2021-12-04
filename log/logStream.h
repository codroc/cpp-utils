#ifndef LOGSTREAM_H
#define LOGSTREAM_H

#include "logger.h"
// #include <stdio.h> // for test
// #include <unistd.h> // for pid_t
#include <string.h>
#include <string>
#include <algorithm>

// format integer
const char digits[] = "0123456789";
template<typename T>
std::string formatInteger(T v) {
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
    // 注意这个内容不是指 buffer 中的所有字符，而是 完整的 n 条 日志
    // 由 _end 标记最后一条完整日志的结尾
    void flush();

    // format time
    static char *formatTime();

    // 功能函数
    void reset() { _cur = _buf; }
    // int avail()  { return end() - _cur; }
    int avail()  { return sizeof(_buf) + _buf - _cur; }
    // 完整日志长度
    size_t completeLogLen() { return static_cast<size_t>(_end - _buf); }
    // 剩余不完整日志长度
    size_t remainUncompleteLogLen() { return static_cast<size_t>(_cur - _end); }

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
    self& operator<<(void* pv); 
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
    self& operator<<(const std::string& str) { return operator<<(str.c_str()); }
private:
    // append 往 _buf 中追加字符
    // operator<< 基于此实现
    void append(const char *p, int len);
    // 行缓冲 刷新， ppos 是指向 字符 '\n' 的指针
    void flushLine(char* ppos);
private:
    // buffer 我们假设一条日志的长度不超过 100 字节，Buffer 可缓存 1000 条日志
    static const int kBufferSize = 1000 * 100; // 100KB
    char      _buf[kBufferSize];
    char*     _cur; // 用于 append 到 buffer 时进行位置定位
    char*     _end; // 用于记录完整的日志尾部，_buf ~ _end 的每一条日志都是完整的
};

#endif 
