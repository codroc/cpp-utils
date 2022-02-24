#ifndef FIBER_H
#define FIBER_H

// Fiber 设计成 非对称协程
// 每个线程有一个主协程以及多个子协程
// 只能在主协程和子协程之间切换上下文，不能再子协程之间切换上下文
#include <ucontext.h>

#include <memory>
#include <functional>
#include <atomic>

class Fiber : public std::enable_shared_from_this<Fiber> {
public:
    using ptr = std::shared_ptr<Fiber>;
    using Callback = std::function<void(void)>;
    // 子协程的构造函数
    Fiber(const Fiber::Callback& cb, size_t stackSize);
    ~Fiber();

    // status
    enum Status{
        FREE, // 没有 callback
        INIT, // 有   callback
        RUNNABLE, // 可以被执行
        RUNNING,  // 正在被执行
        SLEEPING, // 被阻塞
        EXCEPT,   // 异常，阻塞
    };

    // 返回当前协程
    static ptr GetThis();
    // 设置当前协程
    static void SetThis(ptr);
    int getId() const { return _id; }
    int TotalFiberCounts() const { return totalFiberCounts; }

    // 切换到该协程, 一定是从 主协程 切换进来的，主协程没必要设置他的 _status，一直是 FREE 也没关系
    void swapIn();
    // 切换到主协程, 子协程 status 变成 RUNNABLE
    void swapOut();

    // 切换到主协程 并设置 SLEEPING 状态
    void yieldSleeping();
    // 协程准备退出
    static void FiberExit(ptr& sp);
private:
    // 默认构造函数用于构造主协程
    Fiber();

    // 所有协程入口函数
    // 如同所有进程的入口 main 一样
    static void MainFunc();
private:
    int _id = 0;
    Status _status = FREE;

    static std::atomic<int> ids;
    static std::atomic<int> totalFiberCounts;

    ucontext_t _ctx = {0};
    size_t     _stackSize = 0;
    void*      _stackAddr = nullptr;
    Callback   _cb = 0;
};

#endif
