#ifndef MUTEX_H
#define MUTEX_H

// 用 RAII 将 pthread_mutex_t 对象就行封装, 让锁的使用方式更加安全和高效。
// 像使用栈变量一样使用锁

#include <pthread.h>
class MutexLock {
public:
    MutexLock() {
        pthread_mutex_init(&_mutex, NULL);
    } 
    ~MutexLock() {
        pthread_mutex_destroy(&_mutex);
    }

    void lock() {
        pthread_mutex_lock(&_mutex);
    }
    void unlock() {
        pthread_mutex_unlock(&_mutex);
    }
    pthread_mutex_t* getMutexRawPointer() {
        return &_mutex;
    }
private:
    pthread_mutex_t _mutex;
};

class MutexGuard {
public:
    MutexGuard(MutexLock &mutex)
        : _mutexlock(mutex)
    {
        _mutexlock.lock();
        locked = true;
    }
    ~MutexGuard() {
        if(locked) {
            unlock();
        }
    }

    void unlock() {
        locked = false;
        _mutexlock.unlock();
    }
private:
    MutexLock &_mutexlock;
    bool locked = false;
};

#endif
