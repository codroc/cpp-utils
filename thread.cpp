#include "thread.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <cassert>
#include <cstring>
Thread::Thread()
    : _started(false),
      _joined(false),
      _thread(),
      _tid(0), 
      _name(),
      _start_routine(0),
      _threadNum(0)
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
      _threadNum(0)
{
    setDefaultName();
}

void Thread::setDefaultName() {
    int threadNum = _threadNum.fetch_add(1, std::memory_order_relaxed); // 只需要保证原子即可, 因为它就是一个 counter，对 counter 做 increament 只要求原子性
    if (_name.empty()) {
        char buf[16];
        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf), "thread%d", threadNum);
        _name = buf;
    }
}
class ThreadData {
public:
    // std::function<void()>
    using threadFunc = Thread::threadFunc;
    explicit ThreadData(threadFunc start_routine, pid_t *tid, const std::string &name);

    void beforeRoutine();
    void startRoutine() { _start_routine(); }
    void afterRoutine();
private:
    threadFunc  _start_routine; // 用户传进来的 code piece 是std::function<void()>类型
    pid_t*      _tid;
    std::string _name;
};
ThreadData::ThreadData(threadFunc start_routine, pid_t *tid, const std::string &name)
        : _start_routine(std::move(start_routine)),
          _tid(tid),
          _name(std::move(name))
{}

pid_t gettid() { return static_cast<pid_t>(::syscall(SYS_gettid)); }

void ThreadData::beforeRoutine() {
    *_tid = gettid();
    _tid = 0;
    // printf("User routine is going to start by thread: %s\n", _name.c_str());
}
void ThreadData::afterRoutine() {
    // printf("User routine is finished. Thread is going to exit.\n");
}

// 所有线程都把 startThread 当作统一入口，可以在调用用户传进来的 code piece 前，先执行一些“幕前指令”，在执行完用户代码后，可以执行“幕后指令”来收尾，这样才比较优雅。
// 那么用户传进来的 code piece 就需要地方来保存了，因此需要一个 ThreadData 类来记录需要做的”幕前幕后事“以及用户代码
void* startThread(void* obj) { // Thread::start 来调用该例程
    ThreadData *data = static_cast<ThreadData*>(obj);
    data->beforeRoutine();
    data->startRoutine();
    data->afterRoutine();
    delete data;
    pthread_exit(0);
}

void Thread::start() {
    assert(!_started);
    _started = true;
    ThreadData *data = new ThreadData(_start_routine, &_tid, _name);
    void *obj = static_cast<void*>(data);
    pthread_create(&_thread, NULL, startThread, obj);
}

void Thread::join() {
    assert(_started);
    assert(!_joined);
    _joined = true;
    pthread_join(_thread, NULL);
}
