#include "flog.h"

#include <stdio.h>
#include <unistd.h>

#include <string>
#include <memory>

#include <atomic>
std::atomic<int>  a_counter1;
std::atomic<bool> a_waited;
int kNThreads = 2;

const int kNumLogs = 1000;

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
            for (int k = 0;k < kNumLogs;++k) { // 前端最多缓存 1000 * 100 字节
                LOG_WARN << out.c_str() << k << "\n";
            }
        }
        Timestamp end = Timestamp::now();
        int64_t diff = end - start;
        double throughput = (10 * kNumLogs * 100) * 1.0 / diff;
        printf("TestTroughput average time: %f MB/s\n", throughput);
    }
}

void TestRollFile() {
    std::string s(33, 'a');
    std::string out;
    out += "test one";
    out += s;
    for (int i = 0;i < 2000000;++i) // 200M
        LOG_WARN << out.c_str() << i << "\n";
    Logger::getInstance()->setLevel(Logger::LEVEL::INFO);
    for (int i = 0;i < 10;++i)
        LOG_INFO << out.c_str() << i << "\n";
}

void routine() {
    std::string s(33, 'a');
    std::string out;
    out += "test one";
    out += s;

    for (int j = 0;j < 20;++j) {
        while (!a_waited) {}
        for (int i = 0;i < 10;++i) {
            // s 的长度为 100 字节
            for (int i = 0;i < kNumLogs;++i) { // 前端最多缓存 1000 * 100 字节
                LOG_DEBUG << out.c_str() << i << "\n";
            }
        }

        ++a_counter1;
        // 等待 master 把 平均 吞吐量算出来
        while (a_waited) {}
        --a_counter1;
    }

}

void TestMultiThreads() {
    Thread t[kNThreads];
    for (int i = 0;i < kNThreads;++i) {
        t[i].addThreadFunc(routine);
        t[i].start();
    }

    int j = 20;
    while (j--) {
        while (a_counter1 != 0) {}
        int64_t diff;
        Timestamp start = Timestamp::now();
        a_waited = true;
        // 等待 slave 把日志输出完
        while (a_counter1 != kNThreads) {}
        Timestamp end = Timestamp::now();
        diff = end - start;
        // 计算吞吐量
        // printf("g_num_logs = %d g_diff = %d\n", g_num_logs, g_diff);
        double throughput = (kNThreads * kNumLogs * 10  * 100) * 1.0 / diff;
        printf("average time: %f MB/s\n", throughput);

        // 让 slave 继续
        a_waited = false;
    } 
    for (int i = 0;i < kNThreads;++i) {
        t[i].join();
    }
}

AsyncLogInit mylog("loggerTest");
int main(int argc, char *argv[]) {
    if (argc > 1)
        kNThreads = atoi(argv[1]);
    TestThroughput();
    // TestRollFile();
    // TestMultiThreads();
    return 0;
}
