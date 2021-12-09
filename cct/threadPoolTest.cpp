#include "threadPool.h"
#include "currentThread.h"

#include <unistd.h>

void task0() {
    printf("tid = %d: hello\n", CurrentThread::gettid());
}

int main(int argc, char** argv) {
    ThreadPool threadPool;
    threadPool.setNumThreads(2);

    threadPool.start(); // 此时，计算线程已经等待在 任务队列 上了

    for (int i = 0;i < 10;++i)
            threadPool.addTask(task0);
    threadPool.join();
    return 0;
}
