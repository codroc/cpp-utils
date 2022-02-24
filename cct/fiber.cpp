#include "fiber.h"
#include "config.h"
#include "flog.h"

std::atomic<int> Fiber::ids = {0};
std::atomic<int> Fiber::totalFiberCounts = {0};

static thread_local Fiber::ptr t_curFiber = nullptr; // 当前协程
static thread_local Fiber::ptr t_threadFiber = nullptr; // 主协程,只有当线程退出时，才会将主协程析构

// 默认情况每个协程栈大小为 5M
static ConfigVar<size_t>::ptr g_config_stack_size = 
    Config::lookup("ucontext.stack_size", size_t(5*1024*1024), "size of ucontext stack");

Fiber::Fiber()
    : _id(ids++)
{
    _stackSize = g_config_stack_size->getValue();
    _stackAddr = ::malloc(_stackSize);
    
    _ctx.uc_stack.ss_sp = _stackAddr;
    _ctx.uc_stack.ss_size = _stackSize;
    ::makecontext(&_ctx, &Fiber::MainFunc, 0);

    ++totalFiberCounts;
    LOG_INFO << "Fiber::Fiber id = " << _id << "\n";
}

Fiber::Fiber(const Fiber::Callback& cb, size_t stackSize)
    : _id(ids++),
      _cb(cb)
{
    _status = INIT;
    _stackSize = stackSize == 0 ? g_config_stack_size->getValue() : stackSize;
    _stackAddr = ::malloc(_stackSize);

    _ctx.uc_stack.ss_sp = _stackAddr;
    _ctx.uc_stack.ss_size = _stackSize;
    ::makecontext(&_ctx, &Fiber::MainFunc, 0);

    ++totalFiberCounts;
    LOG_INFO << "Fiber::Fiber id = " << _id << "\n";
}

Fiber::~Fiber() {
    assert(_status == FREE || _status == EXCEPT);
    if(_stackAddr) {
        ::free(_stackAddr);
        _stackAddr = nullptr;
    }
    --totalFiberCounts;
    LOG_INFO << "Fiber::~Fiber id = " << _id << "\n";
}

// 返回当前协程
Fiber::ptr Fiber::GetThis() {
    // 如果主协程都还没有，那么就创建
    if (!t_threadFiber) {
        t_threadFiber.reset(new Fiber);
        SetThis(t_threadFiber);
    }
    return t_curFiber;
}
// 设置当前协程
void Fiber::SetThis(Fiber::ptr f) { t_curFiber = f; }


void Fiber::MainFunc() {
    Fiber::ptr cur = GetThis();
    // LOG_INFO << "Fiber::mainFunc\n";    
    
    // 执行 callback
    if(!cur->_cb) {
        cur->_status = Fiber::FREE;
    }
    else {
        try {
            cur->_cb();        
            cur->_status = Fiber::FREE;
        } catch(std::exception& e) {
            cur->_status = Fiber::EXCEPT;
            LOG_ERROR << e.what() << "\n";
        }
    }

    FiberExit(cur);
}


// 切换到该协程
void Fiber::swapIn() {
    SetThis(shared_from_this());
    _status = RUNNING;
    ::swapcontext(&t_threadFiber->_ctx, &_ctx);
}
// 切换到主协程
void Fiber::swapOut() {
    SetThis(t_threadFiber);
    _status = RUNNABLE;
    ::swapcontext(&_ctx, &t_threadFiber->_ctx);
}

void Fiber::yieldSleeping() {
    SetThis(t_threadFiber);
    _status = SLEEPING;
    ::swapcontext(&_ctx, &t_threadFiber->_ctx);
}

void Fiber::FiberExit(ptr& sp) {
    // 协程退出
    Fiber* raw_ptr = sp.get();
    sp.reset();

    SetThis(t_threadFiber);
    ::swapcontext(&raw_ptr->_ctx, &t_threadFiber->_ctx);
    assert(false); // never back
}
