#ifndef COROUTINE_H
#define COROUTINE_H

#include <ucontext.h>
#include <stdint.h>

#include <functional>
#include <memory>

class Coroutine : public std::enable_shared_from_this<Coroutine> {
public:
    using ptr = std::shared_ptr<Coroutine>;
    Coroutine(std::function<void()> cb, size_t stackSize = 0);
    ~Coroutine();
    enum Status {
        INIT,
        SLEEPING,
        RUNNING,
        RUNNABLE,
        STOP,
        EXCEPT,
    };

    // 重置协程
    void reset(std::function<void()> cb);
    // 切换到当前协程执行
    void swapIn();
    // 把当前协程切换到后台
    void swapOut();
public:
    // 返回当前协程
    static ptr GetThis();
    // 设置当前线程的协程
    static void SetThis(Coroutine* p);
    static void YieldToRunnable();
    static void YieldToSleeping();

    static uint64_t TotalCoroutines();
    static void MainFunc();
private:
    Coroutine();
private:
    uint64_t _id = 0;
    uint32_t _stackSize = 0;
    Status   _status = INIT;

    ucontext_t _ctx;
    void*      _stack = nullptr;

    std::function<void()> _cb;
};

#endif
