#include <stdio.h>
#include "fiber.h"
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

int main(int argc, char** argv) {
    Fiber::GetThis();
    Fiber::ptr test(new Fiber(performanceTest, 4096));
    t1 = Timestamp::now();
    while(i < kTimes) {
        test->swapIn();
    }
    return 0;
}
