#include <cstddef>
#include <stdio.h>
#include <stdlib.h>
typedef unsigned int uint32; 
template <typename T>
class Item{
public:
    Item();
    Item(T data);
    T _data;
    Item<T> *_next;// 对象必须由具体的类来产生 Item<T> 就是具体的类
};

template <typename T>
Item<T>::Item()
    : _data()
    , _next(0)
{}

template <typename T>
Item<T>::Item(T data)
    : _data(data)
    , _next(0)
{}

// 在多线程时 应该 保证数据结构 List 的 invariants 特性
template <typename T>
class List {
public:
    List();
    ~List();
    class Item<T>;
    Item<T>* front() { return *_head; }
    // Item<T>* tail() { return *_tail; }
    int insert(int pos, T data);
    int insertTail(T data);
    int remove(int pos);
    T get(int pos);
    bool empty() { return _head == 0 && _len == 0; }
    size_t len() { return _len; }
private:
    Item<T> *_head;
    // Item<T> *_tail;
    size_t _len;
private:
    Item<T>* pToPos(uint32 pos);
    static int posPos(int len, int real_len, int pos);
};

template <typename T>
List<T>::List() // 作用域 即 :: 前，也必须是具体的某个类
    : _head(0)
    // , _tail(0)
    , _len(0)
{}

template <typename T>
List<T>::~List() { 
    while (!empty())
        remove(0);
}

template <typename T>
Item<T>* List<T>::pToPos(uint32 pos) {
    Item<T> *p = _head;
    while (pos-- != 0)
        p = p->_next;
    return p;
}

template <typename T>
int List<T>::insert(int pos, T data) { 
    int pospos;
    if (-1 == (pospos = posPos(_len, _len, pos)))
        return -1;
    else { 
        Item<T> *np = new Item<T>(data);
        // 在这里就要破坏 数据结构 的 invariant 属性了！
        if (pospos == 0) { 
            if (pospos == 0) { 
                np->_next = _head;
                _head = np;
            }
            // else { 
            //     _tail->_next = np;
            //     _tail = np;
            // }
        }
        else { 
            Item<T> *p = pToPos(pospos - 1);
            Item<T> *nextItem = p->_next;
            p->_next = np;
            np->_next = nextItem;
        }
    }
    _len++;
    return 0;
} 

template <typename T>
int List<T>::insertTail(T data) {
    return insert(_len, data);
}

template <typename T>
int List<T>::remove(int pos) {
    if (empty()) {
        printf("Empty list!\n");
        return -1;
    }
    int pospos = posPos(_len - 1, _len,  pos);
    if (pospos == -1)
        return -1;
    Item<T> itm;
    Item<T> *p, *r;
    itm._next = _head;
    _head = &itm;
    ++_len;

    p = pToPos(pospos);
    r = p->_next;
    p->_next = r->_next;
    delete(r);
    --_len;

    _head = _head->_next;
    --_len;
    return 0;
}

// posPos 是来判断将要处理的 pos 是否合法，对于 insert(i, data) 操作(把 data 插到第 i 个位置上)，
// 如果当前 len == n, n >= 0，那么 pos 的范围是：[-1*n-1, n]; 如果是 remove 或 get 操作，那么 pos 的范围是：[-1*n, n - 1];
template <typename T>
int List<T>::posPos(int len, int real_len, int pos) {
    int neg = -1 * len - 1;
    if (pos < neg || pos > len) { 
        printf("List<T>::posPos bad pos\n");
        return -1;
    }
    int pospos = pos;
    if (pos < 0)
        pospos = pos + real_len;
    return pospos;
}

template <typename T>
T List<T>::get(int pos) { 
    if (empty()) {
        printf("Empty list!\n");
        exit(0);
    }
    int pospos = posPos(_len - 1, _len, pos);
    return pToPos(pospos)->_data;
}
