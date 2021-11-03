#include <pthread.h>
#include <queue>
// #include <stdio.h>
// 消息队列，用于生产者消费者模式
template <typename T>
class MQ {
public:
    MQ();
    void push(T data); // V
    T pull();         // P
    void clear(); // clear all elements in queue
    // size_t size(); // mq size
private:
    pthread_mutex_t lock;
    pthread_cond_t  cond;
    std::queue<T> q;
};

template <typename T>
MQ<T>::MQ() {
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);
}

template <typename T>
void MQ<T>::push(T data) {
    pthread_mutex_lock(&lock);
    q.push(data);
    pthread_cond_broadcast(&cond);// wakeup all.
    pthread_mutex_unlock(&lock);
}

template <typename T>
T MQ<T>::pull() {
    pthread_mutex_lock(&lock);
    while (q.empty()) { // 避免虚假的 wakeup
        pthread_cond_wait(&cond, &lock); // relase lock and hold it again when return back
    }
    T result = q.front(); // copy constructor?
    // printf("%d ", (int)result);
    q.pop(); // T's destructor?
    pthread_mutex_unlock(&lock);
    return result;
}

template <typename T>
void MQ<T>::clear() {
    pthread_mutex_lock(&lock);
    while (!q.empty())
        q.pop();
    pthread_mutex_unlock(&lock);
}
