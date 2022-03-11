#ifndef FLOG_H
#define FLOG_H

#include "logger.h"
#include "logStream.h"
#include "asyncLogging.h"
#include "timestamp.h"
#include "thread.h"

class AsyncLogInit {
public:
    AsyncLogInit(const char* progname)
        : _watcher(new AsyncLoggingWatcher(progname)),
          _lw(new LoggerWatcher)
    {}
    ~AsyncLogInit() {
        _lw = nullptr;
        _watcher->stopAsyncLogging();
    }
private:
    std::unique_ptr<AsyncLoggingWatcher> _watcher;
    std::unique_ptr<LoggerWatcher> _lw; // 用于退出程序时回收 Logger 中的资源，同时达到刷新 缓冲区 的作用
};

#endif
