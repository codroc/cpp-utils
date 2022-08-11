#ifndef FLOG_H
#define FLOG_H

#include "logger.h"
#include "logStream.h"
#include "asyncLogging.h"
#include "timestamp.h"
#include "thread.h"
#include <unistd.h>
#include <sys/syscall.h>

class AsyncLogInit {
public:
    AsyncLogInit(const char* progname)
        : _watcher(new AsyncLoggingWatcher(progname)),
          _lw(new LoggerWatcher)
    {}
    ~AsyncLogInit() {
        // printf("~AsyncLogInit()\tThreadId: %ld\n", ::syscall(SYS_gettid));
        _lw = nullptr;
        _watcher = nullptr;
    }
private:
    std::unique_ptr<AsyncLoggingWatcher> _watcher;
    std::unique_ptr<LoggerWatcher> _lw; // 用于退出程序时回收 Logger 中的资源，同时达到刷新 缓冲区 的作用
};

#endif
