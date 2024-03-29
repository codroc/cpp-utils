#include "logger.h"
#include "logStream.h"
#include <stdio.h>
__thread LogStream* t_pLogStream = 0;
__thread struct tm *t_tm = 0;

// static variable initialization
// Logger::LEVEL Logger::level = Logger::LEVEL::NONE;

const int Logger::kFullBuffer = 0; // 全缓冲，及默认 缓冲
const int Logger::kLineBuffer = 1; // 行缓冲
int Logger::bufferLevel = 0; // 默认 缓冲

MutexLock Logger::lock;
MutexLock Logger::initlock;

std::vector<std::unique_ptr<LogStream>> Logger::streams;
std::vector<struct tm*> Logger::tms;

Logger* Logger::instance = nullptr;
Logger::ptr instance_deleter(Logger::getInstance(), [](Logger*) { Logger::release(); });
// LoggerWatcher lw; // 用于推出程序时回收 Logger 中的资源，同时达到刷新 缓冲区 的作用

Logger* Logger::getInstance() {
    if (!instance) {
        MutexGuard guard(lock);
        if (!instance)
            // 这里由于 reordering 的存在，double check 仍旧无法保证线程安全
            // 加个 FENCE 才能保证线程安全
            // 1. 分配地址
            // 2. 在地址上构造对象
            // ============================== FENCE
            // 3. 将地址赋值给 instance
            // FIX ME: use memory order
            instance = new Logger;
    }
    return instance;
}

void Logger::init() {
    setLevel(LEVEL::WARN); // default is warn
}

LogStream& Logger::stream(const char *filename, int line, LEVEL level, const char *func) {
    if (t_pLogStream == 0) {
        MutexGuard guard(initlock);
        std::unique_ptr<LogStream> up(new LogStream()); // RAII
        t_pLogStream = up.get();
        streams.push_back(std::move(up));

        // fix me: 也用 unique_ptr 来 RAII，不过 不能用 delete 而是用 free 释放资源
        t_tm = (struct tm*)malloc(sizeof(struct tm));
        tms.push_back(t_tm);
    }
    if (func == 0)
        t_pLogStream->makeLog(filename, line, level);
    else
        t_pLogStream->makeLog(filename, line, level, func);
    return *t_pLogStream;
}

void Logger::release() {
    if (instance) {
        streams.clear();
        while(!tms.empty()) {
            struct tm* tm = tms.back();
            tms.pop_back();
            free(tm);
        }
        instance = nullptr;
    }
}
