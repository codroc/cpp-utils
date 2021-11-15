#include "logger.h"
#include "logStream.h"
#include "asyncLogging.h"
#include "timestamp.h"

#include <stdio.h>
#include <unistd.h>

#include <string>
#include <memory>

void TestThroughput() {
    std::string s(33, 'a');
    std::string out;
    out += "test one";
    out += s;
    // printf("sizeof out: %d\n", out.size());
    const int kNumLogs = 1000;
    for (int j = 0;j < 20;++j) {
        Timestamp start = Timestamp::now();
        for (int i = 0;i < 10;++i) {
            // s 的长度为 100 字节
            for (int i = 0;i < kNumLogs;++i) { // 前端最多缓存 1000 * 100 字节
                LOG_WARN << out.c_str() << i << "\n";
            }
        }
        Timestamp end = Timestamp::now();
        int64_t diff = end - start;
        double throughput = (10 * kNumLogs * 100) * 1.0 / diff;
        printf("average time: %f MB/s\n", throughput);
    }
}

void TestRollFile() {
    std::string s(33, 'a');
    std::string out;
    out += "test one";
    out += s;
    for (int i = 0;i < 10000000;++i) // 1G
        LOG_WARN << out.c_str() << i << "\n";
    Logger::setLevel(Logger::LEVEL::INFO);
    for (int i = 0;i < 10;++i)
        LOG_INFO << out.c_str() << i << "\n";
}

int main() {
    std::unique_ptr<AsyncLoggingWatcher> watcher(new AsyncLoggingWatcher);
    std::unique_ptr<LoggerWatcher> lw(new LoggerWatcher); // 用于退出程序时回收 Logger 中的资源，同时达到刷新 缓冲区 的作用
    // TestThroughput();
    TestRollFile();
    lw = nullptr;
    watcher->stopAsyncLogging();
    return 0;
}
