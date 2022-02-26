#include <stdio.h>
#include "fiber.h"
#include "thread.h"
#include "mutex.h"
#include "condition.h"
#include "timestamp.h"

int i = 0;
const int kTimes = 1000000;
int ms;
Timestamp t1, t2;

void performanceTest() {
    Fiber::ptr cur = Fiber::GetThis();
    while(1) {
        if (++i < kTimes) {
            cur->swapOut();
        }
        else {
            t2 = Timestamp::now();
            ms = t2 - t1;
            printf("time use %d\n", ms); 
            break;
        }
    }
}

void testFiberSwapPerformance() {
    Fiber::GetThis();
    Fiber::ptr test(new Fiber(performanceTest, 4096));
    t1 = Timestamp::now();
    while(i < kTimes) {
        test->swapIn();
    }
}

struct PC {
    PC()
        : lock(),
          c(lock)
    {}
    void Produce() {
        MutexGuard guard(lock);
        if(i == 0) {
            i++;
            c.wakeup();
        }
        else
            c.wait();
    }
    void Consume() {
        MutexGuard guard(lock);
        if(i == 1) {
            i--;
            c.wakeup();
        }
        else
            c.wait();
    }
    MutexLock lock;
    Condition c;
    int i = 0;
};
PC test;
void Producer() {
    while(++i< kTimes)
        test.Produce();
}
void Consumer() {
    while(i < kTimes)
        test.Consume();
    int ms;
    t2 = Timestamp::now();
    ms = t2 - t1;
    printf("time use %d\n", ms); 
}

void testThreadSwapPerformance() {
    Thread th1(Producer);
    Thread th2(Consumer);
    t1 = Timestamp::now();
    th1.start();
    th2.start();
    th1.join();
    th2.join();
}

int main(int argc, char** argv) {
    // testFiberSwapPerformance();
    testThreadSwapPerformance();
    return 0;
}
