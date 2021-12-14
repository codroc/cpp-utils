#include "thread.h"
#include "currentThread.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <cassert>
#include <cstring>

// declare
// namespace CurrentThread {
//     pid_t gettid();
// }
// using namespace CurrentThread;
// pid_t CurrentThread::gettid();

std::atomic<int> Thread::threadNum(0);
Thread::Thread()
    : _started(false),
      _joined(false),
      _thread(),
      _tid(0), 
      _name(),
      _start_routine(0),
      _sem()
{
    setDefaultName();
}
Thread::Thread(threadFunc start_routine, const std::string& s)
    : _started(false),
      _joined(false),
      _thread(),
      _tid(0),
      _name(s),
      _start_routine(std::move(start_routine)),
      _sem()
{
    setDefaultName();
}

void Thread::setDefaultName() {
    int tn = threadNum.fetch_add(1, std::memory_order_relaxed); // 只需要保证原子即可, 因为它就是一个 counter，对 counter 做 increament 只要求原子性
    if (_name.empty()) {
        std::string name = "thread#";
        name += std::to_string(tn);
        _name = name;
    }
}
class ThreadData {
public:
    // std::function<void()>
    using threadFunc = Thread::threadFunc;
    explicit ThreadData(threadFunc start_routine, pid_t *tid, const std::string &name, Semaphore& sem);

    void beforeRoutine();
    void startRoutine() { _start_routine(); }
    void afterRoutine();

    Semaphore& getSemaphore() { return _sem; }
private:
    threadFunc  _start_routine; // 用户传进来的 code piece 是std::function<void()>类型
    pid_t*      _tid;
    std::string _name;
    Semaphore&  _sem;
};
ThreadData::ThreadData(threadFunc start_routine, pid_t *tid, const std::string &name, Semaphore& sem)
        : _start_routine(std::move(start_routine)),
          _tid(tid),
          _name(std::move(name)),
          _sem(sem)
{}

// pid_t gettid() { return CurrentThread::gettid(); }

void ThreadData::beforeRoutine() {
    *_tid = CurrentThread::gettid();
    _tid = 0;
    ::pthread_setname_np(::pthread_self(), _name.c_str());
    // printf("User routine is going to start by thread: %s\n", _name.c_str());
}
void ThreadData::afterRoutine() {
    // printf("User routine is finished. Thread is going to exit.\n");
}

// 所有线程都把 startThread 当作统一入口，可以在调用用户传进来的 code piece 前，先执行一些“幕前指令”，在执行完用户代码后，可以执行“幕后指令”来收尾，这样才比较优雅。
// 那么用户传进来的 code piece 就需要地方来保存了，因此需要一个 ThreadData 类来记录需要做的”幕前幕后事“以及用户代码
void* startThread(void* obj) { // Thread::start 来调用该例程
    // 通知主线程，我已经 run 起来了！
    ThreadData *data = static_cast<ThreadData*>(obj);
    data->getSemaphore().wakeup();
    data->beforeRoutine();
    data->startRoutine();
    data->afterRoutine();
    delete data;
    pthread_exit(0);
}

void Thread::start() {
    assert(!_started);
    _started = true;
    ThreadData *data = new ThreadData(_start_routine, &_tid, _name, _sem);
    void *obj = static_cast<void*>(data);
    pthread_create(&_thread, NULL, startThread, obj);
    _sem.wait(); // 等待子线程 run 起来
}

void Thread::join() {
    assert(_started);
    assert(!_joined);
    _joined = true;
    pthread_join(_thread, NULL);
}
