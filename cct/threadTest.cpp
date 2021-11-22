// 使用 Thread 类进行测试：
// 1. 多线程一定比单线程快吗？
//  1. 使用锁
//  2. 无锁
// 2. 多线程在内存、耗时方面的测试。
// 计时函数使用 int gettimeofday(struct timeval *tv, struct timezone *tz);
// 陈硕 采用这个函数的原因是因为，它不是系统调用，不会导致 上下文切换以及陷入内核。
// 1. 其实这个原因有待商榷，陷入内核其实花不了多少时间，而陷入内核也不一定就会导致上下文切换，只有在切换 task 时才会导致上下文切换。2. 还有陈硕说这个函数不需要系统调用，但是所有的计时函数都应该是设计成依赖 linux 的全局变量 jiffies 的，要想知道这个值就得依靠系统调用，或许是把他 cache 起来了？那是不是会导致返回的时间不准？

const int kNumThreads = 8; // 根据自己的真实核数来确定
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
using namespace std;

const int kNumBuckets = 1e7;
vector<int> v;

void subset_sum() {

}
void cpu_bound_task(int start, int end) {
    for (int i = start;i < end;++i) {
        int res = 0;
        int a[3][4], b[4][3];
        for (int j = 0;j < 3;++j) {
            for (int k = 0;k < 4;++k) {
                a[j][k] = (i + j * k) % 10000;
                b[k][j] = (i - j * k) % 10000;
            }
        }
        for (int j = 0;j < 3;++j) {
            for (int k = 0;k < 4;++k)
                res += a[j][k] * b[k][j];
        }
        v[i] = res;
    }
}

// 写 log file 是涉及 io 操作的
const int kMaxFileSize = 1024*1024*1024;
const int kWriteSize = 1024;
void io_bound_task(const char *pathname) {
    // const char pathname[] = "io_bound.txt";
    char buf[kWriteSize];
    memset(buf, 'a', kWriteSize);
    int total = 0;
    int fd;
    if ((fd = open(pathname, O_CREAT|O_RDWR, 0777)) == -1) {
        printf("errno = %d error info: %s\n",errno, strerror(errno));
    }
    // 1e9 次 io
    // 每次往 fd 写 1K 内容，当然由于 disk 空间有限，当写入超过 1G 时从头开始写

    for (int i = 0;i < kNumBuckets;++i) {
        if (total >= kMaxFileSize) {
            if (-1 == lseek(fd, 0, SEEK_SET))
                exit(0);
            total = 0;
        }
        int n = write(fd, buf, kWriteSize); // may be blocked
        if (n == 0)
            printf("write return 0\n");
        total += n;
    }
    close(fd);
}

// do cpu bound task
void test_lock() {

}

// 无锁这里就是将 vector 空间分成 kNumThreads 段，每个线程独立地在自己的段里面操作，不干涉别的线程。
void test_non_lock_cpu_bound_task() {
    Thread t[kNumThreads];
    auto f = [](int start, int end) {
        return [=]()->void { cpu_bound_task(start, end); };
    };
    
    Timestamp start = Timestamp::now();
    for (int i = 0;i < kNumThreads;++i) {
        int s = i*(kNumBuckets/kNumThreads);
        int e = (i + 1)*(kNumBuckets/kNumThreads) > kNumBuckets ? kNumBuckets : (i + 1)*(kNumBuckets/kNumThreads);
        t[i].addThreadFunc(f(s, e)); 
        t[i].start();
    }
    for (int i = 0;i < kNumThreads;++i)
        t[i].join();
    Timestamp end = Timestamp::now();
    int64 ti = end - start;
    printf("%d threads fill up vector cost: %lld us\n", kNumThreads, ti);
}

// 无锁地并发 io，设计成 多个线程 往各自的文件中写，线程间独立互不干扰
void test_non_lock_io_bound_task() {
    Thread t[kNumThreads];
    vector<string> v;
    for (int i = 0;i < kNumThreads;++i) {
        string s("pathname");
        s.append(1, '0' + i);
        v.push_back(s);
    }
    auto f = [](const char *pathname) {
        return [=]()->void { io_bound_task(pathname); };
    };
    
    Timestamp start = Timestamp::now();
    for (int i = 0;i < kNumThreads;++i) {
        t[i].addThreadFunc(f(v[i].c_str())); 
        t[i].start();
    }
    for (int i = 0;i < kNumThreads;++i)
        t[i].join();
    Timestamp end = Timestamp::now();
    int64 ti = end - start;
    printf("%d threads write files cost: %lld us\n", kNumThreads, ti);
}

// namespace CurrentThread {
//     pid_t gettid();
// }
void test_single_thread() {
    Timestamp start = Timestamp::now();
    cpu_bound_task(0, kNumBuckets);
    Timestamp end = Timestamp::now();
    int64 t = end - start;
    printf("Single thread %d cost: %lld us\n", CurrentThread::gettid(), t);
}

void init() {
    for (int i = 0;i < kNumBuckets;++i)
        v.push_back(i);
}

int main() {
    // typename Thread::threadFunc f = [](void)->void {
    //     printf("This is user routine!\n");
    // };
    // Thread t(f);
    // t.start();
    // t.join();
    init();
    test_non_lock_cpu_bound_task();
    test_single_thread();
    // test_non_lock_io_bound_task();
    return 0;
}
