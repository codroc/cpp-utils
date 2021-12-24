#include "coroutine.h"
#include "config.h"

#include <vector>
#include <cassert>

thread_local Coroutine::ptr t_sched_coroutine = nullptr;
thread_local std::shared_ptr<std::vector<Coroutine::ptr>> t_sp_coroutines(new std::vector<Coroutine::ptr>());

ConfigVar<size_t>::ptr g_stackSize = Config::lookup("system.coroutine.stackSize", (size_t)1024*1024, "coroutine stack size");

std::atomic<uint64_t> Coroutine::numCoroutines(1);


// 一个线程只能调用一次
// 可以参考单例模式的实现?
Coroutine::Coroutine()
    : _id(0),
      _status(STATUS::FREE),
      _looping(false)
{
    assert(t_sched_coroutine == nullptr);
    t_sched_coroutine.reset(shared_from_this());   

    _stackSize = g_stackSize->getValue();
    _stackAddr = ::malloc(_stackSize);

} 

Coroutine::Coroutine(const Task& task, size_t stackSize = 0)
    : _id(numCoroutines.fetch_add(1)),
      _task(task),
      _status(STATUS::FREE),
      _looping(false)
{
    _stackSize = stackSize == 0 ? g_stackSize->getValue() : stackSize;
    _stackAddr = ::malloc(_stackSize);
    t_sp_coroutines->push_back(shared_from_this());

    _ctx.uc_link = NULL;
    _ctx.uc_stack.ss_sp = _stackAddr;
    _ctx.uc_stack.ss_size = _stackSize;
    setStatus(STATUS::RUNNABLE);
}

Coroutine::~Coroutine() {
    assert(getStatus() == STATUS::FREE);
    if (_stackAddr) {
        free(_stackAddr);
        _stackAddr = 0;
    }
}

// 启动调度协程
// 在启动之前，必须先注册好普通协程
void Coroutine::start() {
    ucontext_t main;
    _ctx.uc_link = &main;
    _ctx.uc_stack.ss_sp = _stackAddr;
    _ctx.uc_stack.ss_size = _stackSize;
    ::makecontext(&_ctx, &Coroutine::schedule, 0);
    ::swapcontext(&main, &_ctx);
}

void Coroutine::yield() {
    // 普通协程 主动让出 cpu 执行权，并设置状态为 SLEEPING
    assert(getStatus() == STATUS::RUNNING);
    // 切换到 调度协程，让其去找下一个要执行的协程进行 switch
    setStatus(STATUS::SLEEPING);
    ::swapcontext(&_ctx, &t_sched_coroutine->_ctx)
}

void Coroutine::schedule() {
    _looping = true;
    ucontext_t main;
    uint32_t numFree = 0; // 用于记录已完成协程数
    while (_looping) {
        for (auto &sp : *t_sp_coroutines) {
            if (sp->getStatus() == STATUS::FREE)
                numFree++;
            if (sp->getStatus() != STATUS::RUNNABLE)
                continue;

            sp->_ctx.uc_link = &main;
            sp->setStatus(STATUS::RUNNING);
            ::makecontext(&sp->_ctx, &Coroutine::mainFunc, 0);
            ::swapcontext(&main, &sp->_ctx);
        }
        if (numFree == t_sp_coroutines.size()) {
            // 此时 所有协程 都已经结束了
            t_sp_coroutines.clear();
            _looping = false;
        }
    }
}
void Coroutine::mainFunc() {
    _task();
}
