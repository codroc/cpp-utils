#include "threadPool.h"
#include "thread.h"
#include "currentThread.h"
#include "flog.h"

#include <stdio.h>
#include <string.h>

const int ThreadPool::kMaxNumThreads = 6;

ThreadPool::ThreadPool()
    : _numThreads(0),
      _threads(),
      _taskQueue()
{}

ThreadPool::ThreadPool(int numThreads)
    : _numThreads(0),
      _threads(),
      _taskQueue()
{
    _threads.reserve(10);
    setNumThreads(numThreads);
    init();
}

void ThreadPool::init() {
    std::string baseName = "ThreadPool";
    for (int i = 0;i < _numThreads;++i) {
        char buf[4]; // 线程数不超过 3 位数
        ::memset(buf, 0, sizeof buf);
        ::snprintf(buf, sizeof buf, "%d", i);
        
        std::shared_ptr<Thread> spThread(new Thread(std::bind(&ThreadPool::thread_routine, this), baseName + std::string(buf)));
        _threads.push_back(spThread);
    }
}

void ThreadPool::setNumThreads(int num) {
    if (num > kMaxNumThreads) {
        LOG_ERROR << "the max number of threads is: " 
            << kMaxNumThreads << "\n";
        _numThreads = kMaxNumThreads;
    }
    else if(num < 0) {
        LOG_ERROR << "the number of threads must >= 0\n" ;
        _numThreads = 0;
    }
    else
        _numThreads = num;
    
    init();
}

void ThreadPool::thread_routine() {
    LOG_INFO << "Thread " << CurrentThread::gettid() << " starting work\n";
    while (1) { // 不停地从 任务队列（阻塞队列）中拿任务并执行
        Task t = _taskQueue.pull();
        t(); // 执行任务
    }
}

void ThreadPool::addTask(Task t) {
    _taskQueue.push(t);
}

void ThreadPool::start() {
    for (auto thread : _threads)
        thread->start();
}

void ThreadPool::join() {
    for (auto thread : _threads)
            thread->join();
}
