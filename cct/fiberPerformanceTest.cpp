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
          cc(lock),
          pc(lock)
    {}
    bool isDone() {
        MutexGuard guard(lock);
        return totalCounts == kTimes;
    }

    bool isFull() { return i == 1; }
    bool isEmpty() { return i == 0; }
    void Produce() {
        MutexGuard guard(lock);
        while(isFull())
            pc.wait();
        ++i;
        ++totalCounts;
        cc.wakeup();
    }
    void Consume() {
        MutexGuard guard(lock);
        while(isEmpty())
            cc.wait();
        --i;
        pc.wakeup();
    }
    MutexLock lock;
    Condition cc, pc;
    int i = 0;
    int totalCounts = 0;
};
PC test;
void Producer() {
    while(!test.isDone())
        test.Produce();
}
void Consumer() {
    while(!test.isDone())
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
