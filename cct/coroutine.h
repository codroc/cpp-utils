#ifndef COROUTINE_H
#define COROUTINE_H

#include <ucontext.h>
#include <memory>
#include <functional>
// 使用 ucontext.h 来实现协程
// 每个线程可以有多个协程和唯一一个调度协程
// 在协程发生阻塞时，通过调用 yield 让出 cpu，并设置 SLEEPING 状态，切换到调度协程，
// 并由调度协程选择下一个要切换过去的协程，最后执行切换动作

class Coroutine : public enable_shared_from_this<Coroutine> {
public:
    using ptr = std::shared_ptr<Coroutine>;
    using Task = std::function<void()>;
    enum class STATUS {
        FREE,
        RUNNING,
        RUNNABLE,
        SLEEPING,
    };
    // 用于创建普通协程
    explicit Coroutine(const Task& task, size_t stackSize = 0);

    void start();
    // 协程主动让出 cpu 执行权
    // 设置 _status = SLEEPING 并 switch 到调度协程
    void yield();

    size_t getStackSize() const { return _stackSize; }
    // 返回当前协程数
    static getNumCoroutine() const { return numCoroutine.load(); }

    STATUS getStatus() const { return _status; }
    void   setStatus(const STATUS& s) { _status = s; }
private:
    // 默认构造函数用于创建 调度协程，不向外暴露
    Coroutine();
    // 调度协程 不断执行 schedule 方法
    void schedule();
    // 其他协程则执行 mainFunc
    void mainFunc();
private:
    uint64_t _id;
    static std::atomic<uint64_t> numCoroutines;
    Task _task;
    ucontext_t _ctx;

    STATUS _status;
    bool   _looping; // 表示调度协程十分在工作

    void*  _stackAddr;
    size_t _stackSize;
};

#endif
