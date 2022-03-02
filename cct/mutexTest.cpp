#include "mutex.h"
#include "thread.h"

MutexLock mu;

int i = 0;

void add(int times) {
    for(int a = 0;a < times;++a) {
        MutexGuard lock(mu);
        i++;
    }
}

int main() {
    int a = 10000, b = 20000;
    Thread t1(std::bind(add, a));
    Thread t2(std::bind(add, b));
    t1.start();
    t2.start();
    t1.join();
    t2.join();
    printf("%d == %d\n", i, a + b);
}
