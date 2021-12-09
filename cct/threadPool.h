#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <memory>
#include <functional>

#include "mq.h"

class Thread;

class ThreadPool {
public:
    using Task = std::function<void()>;

    ThreadPool();
    explicit ThreadPool(int numThreads);
    static int getMaxNumThreads() { return kMaxNumThreads; }

    int getNumThreads() const { return _numThreads; }
    void setNumThreads(int num);

    void addTask(Task t);

    void start();
    void join();
private:
    void init();
    void thread_routine();
private:
    static const int kMaxNumThreads; // 只能从 配置变量 中修改
    int _numThreads;
    std::vector<std::shared_ptr<Thread>> _threads;
    MQ<Task> _taskQueue;
};

#endif
