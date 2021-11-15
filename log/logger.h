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
    using self = Logger;
    enum class LEVEL {
        NONE,
        TRACE,
        INFO,
        DEBUG,
        WARN,
        ERROR,
        FATAL,
    };
    // enum class APPENDER {
    //     CONSOLE,
    //     FFILE,
    // };

    static self* getInstance();
    void init();

    // 日志级别
    static void  setLevel(LEVEL l) { level = l; }
    static LEVEL getLevel() { return level; }

    // // 输出目的地}
    // void     setAppender(APPENDER appender) { _appender = appender; }
    // APPENDER getAppender() { return _appender; }

    // stream 
    // LogStream& stream(const char *filename, int line);
    LogStream& stream(const char *filename, int line, LEVEL level);
    LogStream& stream(const char *filename, int line, LEVEL level, const char *func);

    // for flush buffer, and release resource
    static void release() {
        if (instance) {
            _streams.clear();
            while(!_tms.empty()) {
                struct tm* tm = _tms.back();
                _tms.pop_back();
                free(tm);
            }
            delete instance;
        }
    }
private:
    // 不允许外部构造
    Logger() = default;
    ~Logger() = default;
private:
    static self* instance;
    static LEVEL level;
    // APPENDER     _appender;
    // 线程安全
    static MutexLock lock;

    // 用于 _streams.push_back and _tms.push_back 时的线程安全
    MutexLock initlock;

    // stream
    // 这里设计成每个线程 一个 stream 不就能避免 lock contention 了嘛！

    static std::vector<std::unique_ptr<LogStream>> _streams;
    static std::vector<struct tm*> _tms;
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

class LoggerWatcher {
public:
    ~LoggerWatcher() {
        Logger::release();
    }
};

#endif 
