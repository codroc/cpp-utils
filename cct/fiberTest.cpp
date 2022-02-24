#include "fiber.h"
#include "flog.h"
#include "thread.h"
#include <unistd.h>

void func() {
    LOG_INFO << "func\n";
    LOG_INFO << "func swapOut\n";
    Fiber::GetThis()->swapOut();
    LOG_INFO << "from main again\n";
}

void threadRoutine() {
    Fiber::ptr mainFiber = Fiber::GetThis(); // 创建主协程
    Fiber::ptr sonFiber(new Fiber(func, 1024*1024));

    sonFiber->swapIn();
    LOG_INFO << "return from func\n";

    sonFiber->swapIn();
}

// 怎么保证不管用户怎么 swapOut swapIn 都能使得 fiber 被析构？

int main(int argc, char **argv) {
    Logger::SetBufferLevel(Logger::kLineBuffer);
    Thread t[3];
    for(int i = 0; i < 3;++i) {
        t[i].addThreadFunc(threadRoutine);
        t[i].start();
    }
    for(int i = 0; i < 3;++i) {
        t[i].join(); // 主线程来回收 僵尸子线程的系统资源
    }
    // do other things
    ::sleep(2);
    return 0;
}
