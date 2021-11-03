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
