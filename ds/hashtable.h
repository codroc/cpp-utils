#include <vector>
template <class Value>
class HashNode {
public:
    Value _value;
    HashNode<Value> *_next;
};

template <class Key, class Value, class HashFnc, class EqFnc>
class HashTable {
public:
    typedef HashNode<Value> node;
    typedef HashFnc hasher;
    typedef Eqfnc eq;
    
    HashTable(uint, hasher, eq);
    // 返回 hashtable 中元素个数
    size_t size() { return _num_elements; }
    // 根据 key 来查找对于的 node 是否已经在 hashtable 中
    bool find(Key);
private:
    std::vector<node*> _buckets; // 由于考虑到 hashtable 桶子的可扩展性，索性直接使用 vector 了
    size_t _num_elements;

    hasher _hash;
    eq     _eql;
private:
    // 根据 value 来计算对于的桶号
    size_t bktNum(Value);

    // 是否需要重建表格，如果需要则重建
    void resize(size_t num_elements_hint);
};
