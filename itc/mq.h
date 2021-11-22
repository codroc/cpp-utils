#include "mutex.h"
#include "condition.h"
#include <queue>
// 消息队列，用于生产者消费者模式
template <typename T>
class MQ {
public:
    MQ()
        : _mutex(),
          _cond(_mutex),
          q()
    {}
    void push(T data); // V
    T pull();         // P
    void clear(); // clear all elements in queue
    // size_t size(); // mq size
private:
    MutexLock _mutex;
    Condition _cond;
    std::queue<T> q;
};


template <typename T>
void MQ<T>::push(T data) {
    MutexGuard lock(_mutex);
    q.push(data);
    _cond.wakeupAll();
}

template <typename T>
T MQ<T>::pull() {
    MutexGuard lock(_mutex);
    while (q.empty()) { // 避免虚假的 wakeup
        _cond.wait(); // relase lock and hold it again when return back
    }
    T result = q.front(); // copy constructor?
    q.pop(); // T's destructor?
    return result;
}

template <typename T>
void MQ<T>::clear() {
    MutexGuard lock(_mutex);
    while (!q.empty())
        q.pop();
}
