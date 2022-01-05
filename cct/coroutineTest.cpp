#include "coroutine.h"
#include "flog.h"
#include <unistd.h>

void run_in_coroutine() {
    LOG_INFO << "run in coroutine begin\n";
    Coroutine::GetThis()->swapOut();
    LOG_INFO << "run in coroutine end\n";
}

void test_coroutine() {
    ::sleep(1);
    Coroutine::GetThis();
    LOG_INFO << "main begin\n";
    Coroutine::ptr co(new Coroutine(run_in_coroutine));
    co->swapIn();
    LOG_INFO << "main after swapIn 1\n";
    co->swapIn();
    LOG_INFO << "main after run swapIn 2\n";
    LOG_INFO << "main end\n";
}

int main(int argc, char** argv) {
    Thread ts[3];
    for (int i = 0; i < 3; ++i) {
        ts[i].addThreadFunc(test_coroutine);
        ts[i].start();
    }
    for (int i = 0; i < 3; ++i)
        ts[i].join();
    return 0;
}
