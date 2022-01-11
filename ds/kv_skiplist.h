#ifndef KV_SKIPLIST_H
#define KV_SKIPLIST_H

// 这个数据结构，利用跳表来实现一个简单的 KV 存储

#include "mutex.h"
#include "flog.h"
#include <string.h>
#include <cmath>
#include <iostream>

template<class K, class V>
struct Node {
    K k;
    V v;
    // 一个指针数组，next[i] 表示指向第 i 层的下一个节点的指针
    Node<K, V>** next;
    // 表示从 0 ~ level 层，该节点都将作为索引
    int level;

    Node(K, V, int);
    ~Node();
    K GetKey() const;
    V GetVal() const;
    
    void SetVal(V val);
};

template<class K, class V>
Node<K, V>::Node(K key, V val, int l)
    : k(key),
      v(val),
      level(l)
{
    next = new Node<K, V>* [level + 1];
    ::memset(next, 0, sizeof(Node<K, V>*) * (level + 1));
}

template<class K, class V>
Node<K, V>::~Node() {
    if (next) {
        delete [] next;
        next = 0;
    }
}

template<class K, class V>
K Node<K, V>::GetKey() const { return k; }

template<class K, class V>
V Node<K, V>::GetVal() const { return v; }

template<class K, class V>
void Node<K, V>::SetVal(V val) { v = val; }

template<class K, class V>
class SkipList {
public:
    SkipList(int maxLevel = 8);
    ~SkipList();

    // 查找 key 对应的 node
    Node<K, V>* find(K key);
    
    // 插入节点
    // 如果对应的 key 已存在则返回 1; 插入成功返回 0
    int insert(K key, V val);

    // 删除 key 对应的节点
    void erase(K key);

    // 打印全表
    void display();

    int getMaxLevel() const { return _maxLevel; }
    int getCurLevel() const { return _curLevel; }
    int size() const { return _counts; }
    Node<K, V> getHead() const { return _head; }
private:
    Node<K, V>* createNode(K key, V val, int level);
    // 找到 key 对应节点的 前继节点 pre
    // 可能 key 所对应的节点根本不存在，这样也会返回 pre，不影响的。。。
    Node<K, V>* findPre(K key);
    // 从 0 ~ _maxLevel 中随机取个数出来
    int getRandomLevel();
private:
    // 设定跳表最大为 _maxLevel 层
    int _maxLevel;
    // 当前跳表在第 _curLevel 层
    int _curLevel;
    // 指向跳表的头节点
    Node<K, V>* _head;

    // 跳表中元素总个数
    int _counts;

    // 加个锁，用于 cct
    MutexLock _lock;
};

template<class K, class V>
SkipList<K, V>::SkipList(int maxLevel) 
    : _maxLevel(maxLevel),
      _curLevel(0),
      _head(0),
      _counts(0),
      _lock()
{
    K k;
    V v;
    _head = createNode(k, v, _maxLevel);
}
template<class K, class V>
SkipList<K, V>::~SkipList() {
    if (_head) {
        delete _head;
        _head = 0;
    }
}

// 创建一个 Node 节点
template<class K, class V>
Node<K, V>* SkipList<K, V>::createNode(K key, V val, int level) {
    return new Node<K, V>(key, val, level);
}

// 查找 key 对应的节点
// 未找到则返回 0
template<class K, class V>
Node<K, V>* SkipList<K, V>::find(K key) {
    Node<K, V> *pre = findPre(key)->next[0];
    
    MutexGuard guard(_lock);
    if (pre && pre->GetKey() == key)    return pre;
    return 0;
}

// 查找 key 对应节点的 前继节点
template<class K, class V>
Node<K, V>* SkipList<K, V>::findPre(K key) {
    Node<K, V>* update[_maxLevel + 1]; // 需要更新的节点
    ::memset(update, 0, sizeof(Node<K, V>*) * (_maxLevel + 1));

    MutexGuard guard(_lock);
    Node<K, V>* current = _head;
    for (int i = _curLevel; i >= 0; --i) {
        while (current->next[i] && current->next[i]->GetKey() < key)
            current = current->next[i];
        update[i] = current; // 这里记录 前继节点
    }
    return current;    
}

// 插入节点
// 如果对应的 key 已存在则返回 1; 插入成功返回 0
template<class K, class V>
int SkipList<K, V>::insert(K key, V val) {
    Node<K, V>* update[_maxLevel + 1]; // 需要更新的节点
    ::memset(update, 0, sizeof(Node<K, V>*) * (_maxLevel + 1));

    MutexGuard guard(_lock);
    Node<K, V>* current = _head;
    for (int i = _curLevel; i >= 0; --i) {
        while (current->next[i] && current->next[i]->GetKey() < key)
            current = current->next[i];
        update[i] = current; // 这里记录 前继节点
    }

    current = current->next[0];
    if (current && current->GetKey() == key) {
        LOG_INFO << "key: " << key << ", exists\n";
        return 1;
    }

    int randomLevel = getRandomLevel();
    // 如果 randomLevel 大于 当前跳表层数 _curLevel
    // 那就把从 _curLevel 到 randomLevel 层的跳表的 _head 的 next 都指向 该节点
    if (randomLevel > _curLevel) {
        for (int i = _curLevel + 1; i < randomLevel; ++i)
            update[i] = _head;
        _curLevel = randomLevel;
    }

    Node<K, V>* node = createNode(key, val, randomLevel);
    for (int i = 0; i < randomLevel; ++i) {
        node->next[i] = update[i]->next[i];
        update[i]->next[i] = node;
    }

    LOG_INFO << "Successfully inserted key: " << key << ", value: " << val << "\n";
    _counts++;
    return 0;
}

// 删除 key 对应的节点
template<class K, class V>
void SkipList<K, V>::erase(K key) {
    Node<K, V>* update[_maxLevel + 1];
    ::memset(update, 0, sizeof(Node<K, V>*) * (_maxLevel + 1));
    MutexGuard guard(_lock);
    Node<K, V>* current = _head;
    for (int i = _curLevel; i >= 0; --i) {
        while (current->next[i] && current->next[i]->GetKey() < key)
            current = current->next[i];
        update[i] = current;
    }

    current = current->next[0];
    int l = 0;
    if (current && current->GetKey() == key) {
        l = current->level; // 该节点最高到 level 层
        // 找到对应的节点了，进行删除
        for (int i = 0; i < _curLevel; ++i) {
            if (update[i]->next[i] != current)  break;
            update[i]->next[i] = current->next[i];
        }
    }
    while (_curLevel > 0 && _head->next[_curLevel] == NULL)
        _curLevel--;
    std::cout << "Successfully deleted key "<< key << "\n";
    _counts--;
    delete current;
}

template<class K, class V>
int SkipList<K, V>::getRandomLevel() {
    int k = 1;
    while (rand() % 2) {
        k++;
    }
    k = (k < _maxLevel) ? k : _maxLevel;
    return k;
}

// 打印全表
template<class K, class V>
void SkipList<K, V>::display() {
    MutexGuard guard(_lock);
    std::cout << "\n****** Skip List *******\n";
    int j = 0;
    for (int i = _curLevel; i >= 0; --i) {
        Node<K, V>* current = _head->next[i];
        while (current) {
            if (j++ != 0)   std::cout << "\t";
            std::cout << "(key: " << current->GetKey() << ", val: " << current->GetVal() << ")";
            current = current->next[i];
        }
        j = 0;
        std::cout << "\n";
    }
}
#endif
