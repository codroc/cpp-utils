#include "mq.h"
#include <functional>
#include <algorithm>
#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
using namespace std;

#define NPRODUCER 1
#define NCONSUMER 2
// int pthread_create(pthread_t * thread, const pthread_arrt_t* attr,void*(*start_routine)(void *), void* arg);

typedef function<void (string&)> fnvstr;
void myReverse(string& str) {
    for (int i = 0;i < 100000;++i)
        reverse(str.begin(), str.end());
}

struct package {
    package(fnvstr, string&);
    fnvstr fn;
    string str;
};
package::package(fnvstr f, string& s)
    : fn(f),
    str(s)
{}


MQ<package> intMQ;
void* producer(void*) {
    printf("producer thread started!\n");
    string longstr;
    string a[] = {
        "hello",
        " world,",
        " I am codroc.",
        longstr,
    };
    for (int i = 0;i < 4;++i){
        fnvstr f = myReverse;
        package p(f, a[i]); // 这里出作用域析构是没问题的，因为 p 已经拷贝到 队列中了
        intMQ.push(p);
    }
    printf("\nproducer thread exited!\n");
}

void* consumer(void*) {
    pid_t tid = gettid();
    printf("consumer: %d started!\n", tid);
    while(1) {
        package ret = intMQ.pull(); // MQ::pull 等待在条件变量上，因此虽然是 while(1) 但不是 busy-waiting.
        // printf("consumer: %d receive msg: %d\n", tid, ret);
        // sleep(1);
        printf("consumer: %d receive   str: %s\n", tid, ret.str.c_str());
        ret.fn(ret.str);
        printf("consumer: %d processed str: %s\n", tid, ret.str.c_str());
    }
}

pthread_t* producer_init() {
    pthread_t *threads;
    threads = (pthread_t*)malloc(NPRODUCER * sizeof(pthread_t));
    for (int i = 0;i < NPRODUCER;++i)
        pthread_create(threads + i, 0, producer, 0);
    return threads;
}
pthread_t* consumer_init() {
    pthread_t *threads;
    threads = (pthread_t*)malloc(NCONSUMER * sizeof(pthread_t));
    for (int i = 0;i < NCONSUMER;++i)
        pthread_create(threads + i, 0, consumer, 0);
    return threads;
}

int main() {
    pthread_t *p = producer_init();
    pthread_t *c = consumer_init();
    for (int i = 0;i < NPRODUCER;++i)
        pthread_join(*(p + i), NULL);
    for (int i = 0;i < NCONSUMER;++i)
        pthread_join(*(c + i), NULL);
    return 0;
}
