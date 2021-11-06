// 使用 Thread 类进行测试：
// 1. 多线程一定比单线程快吗？
//  1. 使用锁
//  2. 无锁
// 2. 多线程在内存、耗时方面的测试。
// 计时函数使用 int gettimeofday(struct timeval *tv, struct timezone *tz);
// 陈硕 采用这个函数的原因是因为，它不是系统调用，不会导致 上下文切换以及陷入内核。
// 1. 其实这个原因有待商榷，陷入内核其实花不了多少时间，而陷入内核也不一定就会导致上下文切换，只有在切换 task 时才会导致上下文切换。2. 还有陈硕说这个函数不需要系统调用，但是所有的计时函数都应该是设计成依赖 linux 的全局变量 jiffies 的，要想知道这个值就得依靠系统调用，或许是把他 cache 起来了？那是不是会导致返回的时间不准？

const int kNumThreads = 2; // 根据自己的真实核数来确定
#include "thread.h"
#include "mutex.h"
#include "timestamp.h"

#include <vector>
using namespace std;

const int kNumBuckets = 3e7;
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
void io_bound_taks() {

}

// do cpu bound task
void test_lock() {

}

// 无锁这里就是将 vector 空间分成 kNumThreads 段，每个线程独立地在自己的段里面操作，不干涉别的线程。
void test_non_lock() {
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

void test_single_thread() {
    Timestamp start = Timestamp::now();
    cpu_bound_task(0, kNumBuckets);
    Timestamp end = Timestamp::now();
    int64 t = end - start;
    printf("Single thread cost: %lld us\n", t);
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
    test_non_lock();
    test_single_thread();
    return 0;
}
