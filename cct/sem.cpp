#include "semaphore.h"
#include "flog.h"

#include <stdint.h>
#include <string.h>

Semaphore::Semaphore(uint32_t val) {
    if (::sem_init(&_sem, 0, val)) {
        LOG_ERROR << "Semaphore::Semaphore error str = "
            << ::strerror(errno) << "\n";
    }
}
Semaphore::~Semaphore() {
    ::sem_destroy(&_sem);
}

void Semaphore::wait() {
    if (::sem_wait(&_sem)) {
        if (errno == EINTR)
            LOG_INFO << "Semaphore::wait interupted by signal! Please try again.\n";
        else {
            LOG_ERROR << "Semaphore::wakeup error str = "
                << ::strerror(errno) << "\n";
        }
    }
}

void Semaphore::wakeup() {
    if (::sem_post(&_sem)) {
        if (errno == EOVERFLOW) {
            LOG_ERROR << "Semaphore::wakeup overflow. error str = "
                << ::strerror(errno) << "\n";
        }
    }
}
