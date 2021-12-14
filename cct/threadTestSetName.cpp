#include "thread.h"
#include "mutex.h"
#include "timestamp.h"
#include "currentThread.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <vector>
#include <string>

void func1() {
    ::sleep(10);
}

int main() {
    Thread t[5];
    for (int i = 0; i < 5; ++i) {
        t[i].addThreadFunc(func1);
        t[i].start();
    }

    for (int i = 0; i < 5; ++i) {
        t[i].join();
    }
    return 0;
}
