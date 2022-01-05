#include "coroutine.h"
#include "config.h"

#include "flog.h"

#include <atomic>
#include <cassert>

static std::atomic<uint64_t> g_coroutine_count {0};
static std::atomic<uint64_t> g_coroutine_id {0};

// 当前协程
static thread_local Coroutine* t_coroutine = nullptr;
// 主协程
static thread_local Coroutine::ptr t_threadCoroutine = nullptr;

ConfigVar<size_t>::ptr g_sp_stack_size = 
    Config::lookup("system.stack_size", (size_t) 1024*1024, "coroutine stack size");

Coroutine::Coroutine() 
{
    _status = RUNNING;
    SetThis(this);

    getcontext(&_ctx);

    ++g_coroutine_count;
    LOG_INFO << "Coroutine::Coroutine id = " << _id << "\n";
}

Coroutine::Coroutine(std::function<void()> cb, size_t stackSize) 
    : _cb(cb)
{
    _stackSize = stackSize ? stackSize : g_sp_stack_size->getValue();
    _id = ++g_coroutine_id;
    _stack = ::malloc(_stackSize);

    getcontext(&_ctx);

    _ctx.uc_link = NULL;
    _ctx.uc_stack.ss_sp = _stack;
    _ctx.uc_stack.ss_size = _stackSize;

    makecontext(&_ctx, &Coroutine::MainFunc, 0);
    ++g_coroutine_count;
    LOG_INFO << "Coroutine::Coroutine id = " << _id << "\n";
}

Coroutine::~Coroutine() {
    --g_coroutine_count;
    if (_stack) {
        assert(_status == INIT
                || _status == STOP
                || _status == EXCEPT);
        ::free(_stack);
    }
    else {
        // 是主协程
        assert(_status == RUNNING);
        assert(!_cb);

        Coroutine* cur = t_coroutine;
        if (cur == this)
            SetThis(nullptr);
    }
    LOG_INFO << "Coroutine::~Coroutine id = " << _id << "\n";
}

void Coroutine::reset(std::function<void()> cb) {
    assert(_stack);
    assert(_status == INIT
            || _status == STOP
            || _status == EXCEPT);
    _cb = cb;
    getcontext(&_ctx);
    _ctx.uc_link = NULL;
    _ctx.uc_stack.ss_sp = _stack;
    _ctx.uc_stack.ss_size = _stackSize;

    makecontext(&_ctx, &Coroutine::MainFunc, 0);
    _status = INIT;
}

void Coroutine::swapIn() {
    SetThis(this);
    assert(_status != RUNNING);
    _status = RUNNING;

    swapcontext(&t_threadCoroutine->_ctx, &_ctx);
}
// 把当前协程切换到后台
void Coroutine::swapOut() {
    SetThis(t_threadCoroutine.get());

    swapcontext(&_ctx, &t_threadCoroutine->_ctx);
}

void Coroutine::SetThis(Coroutine* p) { t_coroutine = p; }

Coroutine::ptr Coroutine::GetThis() {
    if (t_coroutine)
        return t_coroutine->shared_from_this();

    t_threadCoroutine.reset(new Coroutine);
    t_coroutine = t_threadCoroutine.get();
    return t_coroutine->shared_from_this();
}

void Coroutine::YieldToRunnable() {
    Coroutine::ptr cur = GetThis();
    cur->_status = RUNNABLE;
    cur->swapOut();
}
void Coroutine::YieldToSleeping() {
    Coroutine::ptr cur = GetThis();
    cur->_status = SLEEPING;
    cur->swapOut();
}

uint64_t Coroutine::TotalCoroutines() {
    return g_coroutine_count;
}
void Coroutine::MainFunc() {
    Coroutine::ptr cur = GetThis();
    try {
        cur->_cb();
        cur->_cb = nullptr;
        cur->_status = STOP;
    }
    catch (std::exception& ex) {
        cur->_status = EXCEPT;  
        LOG_ERROR << "Coroutine Except: " << ex.what() << "\n";
    }
    catch (...) {
        cur->_status = EXCEPT;  
        LOG_ERROR << "Coroutine Except\n";
    }
    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->swapOut();
}
