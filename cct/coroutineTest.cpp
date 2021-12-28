#include "coroutine.h"
#include "flog.h"

void run_in_coroutine() {
    LOG_INFO << "run in coroutine begin\n";
    Coroutine::GetThis()->swapOut();
    LOG_INFO << "run in coroutine end\n";
    Coroutine::GetThis()->swapOut();
}

int main(int argc, char** argv) {
    LOG_INFO << "main begin\n";
    Coroutine::ptr mainCo = Coroutine::GetThis();
    Coroutine::ptr co(new Coroutine(run_in_coroutine));
    co->swapIn();
    LOG_INFO << "main after swapIn 1\n";
    co->swapIn();
    LOG_INFO << "main after run swapIn 2\n";
    LOG_INFO << "main end\n";
    return 0;
}
