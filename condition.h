#ifndef CONDITION_H
#define CONDITION_H

#include "mutex.h"
class Condition {
public:
    Condition(MutexLock &mutex) 
        : _mutex(mutex)
    {
        pthread_cond_init(&_cond, NULL);
    }
    ~Condition() {
        pthread_cond_destroy(&_cond);
    }
    void wait() {
        pthread_cond_wait(&_cond, _mutex.getMutexRawPointer());
    }
    // 如果超时返回 true，否则返回 false
    bool waitForSecs(int secs) {
        struct timespec abstime;

        clock_gettime(CLOCK_REALTIME, &abstime);
        const int64_t kNanoSecondsPerSecond = 1000000000;
        int64_t nanoSeconds = secs * kNanoSecondsPerSecond;
        abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nanoSeconds) / kNanoSecondsPerSecond);
        abstime.tv_nsec = static_cast<long>((abstime.tv_nsec + nanoSeconds) % kNanoSecondsPerSecond);
        return pthread_cond_timedwait(&_cond, _mutex.getMutexRawPointer(), &abstime);
    }
    void wakeup() {
        pthread_cond_signal(&_cond);
    }
    void wakeupAll() {
        pthread_cond_broadcast(&_cond);
    }
private:
    pthread_cond_t _cond;
    MutexLock &_mutex;
};

#endif
