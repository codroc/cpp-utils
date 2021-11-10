#include "logger.h"
#include "logStream.h"
__thread LogStream* t_pLogStream = 0;

Logger::LEVEL Logger::level = Logger::LEVEL::NONE;
MutexLock Logger::lock;
Logger* Logger::instance = 0;

Logger* Logger::getInstance() {
    if (instance == 0) {
        MutexGuard guard(lock);
        if (instance == 0)
            // 这里由于 reordering 的存在，double check 仍旧无法保证线程安全
            // 加个 FENCE 才能保证线程安全
            // 1. 分配地址
            // 2. 在地址上构造对象
            // ============================== FENCE
            // 3. 将地址赋值给 instance
            // FIX ME: use memory order
            instance = new Logger();
    }
    return instance;
}

void Logger::init() {
    setLevel(LEVEL::WARN); // default is warn
}


LogStream& Logger::stream(const char *filename, int line, LEVEL level) {
    if (t_pLogStream == 0)
        t_pLogStream = new LogStream();
    t_pLogStream->makeLog(filename, line, level);
    return *t_pLogStream;
}


LogStream& Logger::stream(const char *filename, int line, LEVEL level, const char *func) {
    if (t_pLogStream == 0)
        t_pLogStream = new LogStream();
    t_pLogStream->makeLog(filename, line, level, func);
    return *t_pLogStream;
}


