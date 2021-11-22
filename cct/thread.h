#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <functional>
#include <string>
#include <atomic>

// int
// pthread_create(pthread_t *thread, const pthread_attr_t *attr, 
//              void* (*start_routine)(void*), void *arg);

class Thread {
public:
    using threadFunc = std::function<void(void)>;

    Thread();
    explicit Thread(threadFunc start_routine, const std::string &s = std::string());

    void start(); // pthread_create
    void join(); // pthread_join 类似于 进程的 wait

    bool  started() const { return _started; }
    pid_t tid() const { return _tid; }
    const std::string& name() const { return _name; }

    void addThreadFunc(threadFunc start_routine) { _start_routine = std::move(start_routine); }
private:
    void setDefaultName();
private:
    bool _started;
    bool _joined;
    
    pthread_t _thread;
    pid_t     _tid;
    std::string _name;
    threadFunc  _start_routine;
    std::atomic<int> _threadNum;
};

#endif
