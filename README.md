## cpp-utils

The purpose of this repo:

Using these utils, I can develop C++ applications more conveniently and flexibly. And these little components are prepared for future projects.



## install

```SHELL
$ git clone https://github.com/codroc/cpp-utils.git
$ cd cpp-utils && mkdir build && cd build
$ sudo make install
```

You can see the lib `libcpputils.a` in `/usr/local/bin` and headers in `/usr/local/include`.

Compile your source file just like this:

```SHELL
$ g++ test.cpp -o test -I /usr/local/include/ -L /usr/local/bin/ -lcpputils -lpthread
```



## Platforms

- Linux, FreeBSD



## Interface Description

### cct (concurrency control technique)

`cct/mutex.h:`

**class MutexLock**

**class MutexGuard**

The Encapsulation of pthread mutex.

```c++
#include "mutex.h"

MutexLock g_lock; // declare global lock
···
void func() {
    ···
    { // critical section
		MutexGuard guard(g_lock);
        ···
	} // critical section
    ···
}
```



`cct/condition.h:`

**class Condition**

The Encapsulation of pthread condition variable.

```C++
#include "condition.h"

MutexLock g_lock;
Condition g_cond(g_lock);
int counter = 1;
···
void setZero() {
    MutexGuard guard(g_lock);
    counter = 0;
    g_cond.wakeup(); // Wakeup a sleeping thread who is waiting counter to be zero.
}
void waitZero() {
    MutexGuard guard(g_lock);
    while (counter != 0) {
        g_cond.wait(); // Wait until counter to be zero.
    }
}
···
```



`cct/currentThread.h:`

**pid_t CurrentThread::gettid()**

```C++
void func() {
    ...
    printf("current thread id: %d\n", static_cast<int>(CurrentThread::gettid()));
    ···
}
```



`cct/thread.h:`

**class Thread**

```C++
#include "thread.h"

void routine() {
    printf("current thread id: %d\n", static_cast<int>(CurrentThread::gettid()));
}

int main() {
    Thread t0(routine);
    Thread t1;
    t1.addThreadFunc(routine);
    
    t0.start();
    t1.start();
    
    t0.join();
    t1.join();
    return 0;
}
```

`cct/fiber.h:`

**class Fiber**

```C++
#include "thread.h"
#include "fiber.h"
#include "flog.h"

void func() {
    Fiber::ptr cur = GetThis();
    LOG_INFO << "in func\n";   
    cur->swapOut();
    LOG_INFO << "in func again\n";   
}

void threadRoutine() {
    // create a main fiber
    Fiber::GetThis();
    // new a fiber with 1M stack and let it do func
    Fiber::ptr fiber1(new Fiber(func, 1024*1024));
    // swap in to fiber1
    fiber1->swapIn();
    LOG_INFO << "return from func\n";   
    fiber1->swapIn();
}

int main() {
    // turn on Line Buffer Level
    Logger::SetBufferLevel(Logger::kLineBuffer);

    Thread t(threadRoutine);
    t.start();
    t.join();
    return 0;
}
```


---

### ds (data structure)

`ds/list.h`

Template single list

Usage: see the case in `itc/mqTest.cpp`

```C++
void strInsert() {
     List<string> ls;
     ls.insertTail("hello");
     ls.insertTail(" world,");
     ls.insertTail(" I am codroc.");
     for (int i = 0;i < ls.len();++i) {
         printf("%s", ls.get(i).c_str());
     }
     printf("\n");
     assert(-1 == ls.remove(3)); // 应该要发现是 bad pos 并返回-1
     assert(-1 == ls.remove(-4)); // 应该要发现是 bad pos 并返回-1
     while (!ls.empty()) {
         for (int i = 0;i < ls.len();++i) {
             printf("%s", ls.get(i).c_str());
         }
         printf("\n");
         int ret = ls.remove(-1); // 应该能正确删除倒数第一个元素
     }
     assert(ls.empty() == true);
     ls.remove(-5);// empty list!
     ls.remove(0); // empty list!
 }
```



---

### itc (inter thread communication)

`itc/mq.h`

**class MQ**

Usage: see the case in `itc/mqTest.cpp`



---

### log

**Multi-threaded asynchronous log library**

Appender is the stdout:

```C++
#include "flog.h"

int main() {
    LOG_WARN << "hello\n";
}
```

Appender is a file:

```C++
#include "flog.h"

int main(int argc, char *argv[]) {
    AsyncLogInit log(argv[0]); // initial async log.
    ···
    for (int i = 0;i < 100;++i)
    	LOG_WARN << "hello" << i << "\n";
    ···
    log.destroy(); // AsyncLogInit::destroy must be called before exit!
    return 0;
}
```

