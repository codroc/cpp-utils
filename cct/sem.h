#ifndef SEM_H
#define SEM_H

#include <semaphore.h>
// sem_init

class Semaphore {
public:
    Semaphore(uint32_t val = 0);
    ~Semaphore();

    void wait();

    void wakeup();
private:
    Semaphore(const Semaphore&) = delete;
    Semaphore(const Semaphore&&) = delete;
    const Semaphore& operator=(const Semaphore&) = delete;
private:
    sem_t _sem;
};

#endif
