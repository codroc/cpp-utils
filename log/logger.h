#ifndef LOGGER_H
#define LOGGER_H

#include "mutex.h"
#include <memory>
#include <vector>

class LogStream;
// 日志器就是日志系统的调用接口
// 日志器必须是全局唯一的
class Logger {
public:
    using ptr = std::shared_ptr<Logger>;
    enum class LEVEL {
        NONE,
        TRACE,
        INFO,
        DEBUG,
        WARN,
        ERROR,
        FATAL,
    };

    static Logger* getInstance();
    void init();

    // 日志级别
    static void  setLevel(LEVEL l) { level = l; }
    static LEVEL getLevel() { return level; }
    // 缓冲级别
    static const int kFullBuffer; // 全缓冲，及默认 缓冲
    static const int kLineBuffer; // 行缓冲
    static void SetBufferLevel(int level) { bufferLevel = level; }
    static int  GetBufferLevel() { return bufferLevel; }

    // stream 
    LogStream& stream(const char *filename, int line, LEVEL level, const char *func = 0);

    // for flush buffer, and release resource
    static void release();
private:
    // 不允许外部构造
    Logger() = default;
private:
    static Logger* instance;
    static LEVEL level;
    static int bufferLevel; // 0 默认缓冲，1 按行缓冲
    // APPENDER     _appender;
    // 线程安全
    static MutexLock lock;

    // 用于 streams.push_back and _tms.push_back 时的线程安全
    static MutexLock initlock;

    // stream
    // 这里设计成每个线程 一个 stream 不就能避免 lock contention 了嘛！

    static std::vector<std::unique_ptr<LogStream>> streams;
    static std::vector<struct tm*> tms;
};

#define LOG_TRACE if (Logger::getLevel() <= Logger::LEVEL::TRACE) \
    Logger::getInstance()->stream(__FILE__, __LINE__, Logger::LEVEL::TRACE, __func__)
#define LOG_INFO if (Logger::getLevel() <= Logger::LEVEL::INFO) \
    Logger::getInstance()->stream(__FILE__, __LINE__, Logger::LEVEL::INFO)
#define LOG_DEBUG if (Logger::getLevel() <= Logger::LEVEL::DEBUG) \
    Logger::getInstance()->stream(__FILE__, __LINE__, Logger::LEVEL::DEBUG, __func__)
#define LOG_WARN if (Logger::getLevel() <= Logger::LEVEL::WARN) \
    Logger::getInstance()->stream(__FILE__, __LINE__, Logger::LEVEL::WARN)
#define LOG_ERROR if (Logger::getLevel() <= Logger::LEVEL::ERROR) \
    Logger::getInstance()->stream(__FILE__, __LINE__, Logger::LEVEL::ERROR)
// fatal 必定退出进程
#define LOG_FATAL Logger::getInstance()->stream(__FILE__, __LINE__, Logger::LEVEL::FATAL)

// 用于推出程序时回收 Logger 中的资源，同时达到刷新 缓冲区 的作用

class LoggerWatcher {
public:
    ~LoggerWatcher() {
        Logger::release();
    }
};

#endif 
