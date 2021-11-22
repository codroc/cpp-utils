#include "list.h"
#include <iostream>
#include <string>
#include <cassert>
#include <functional>
using namespace std;

void intEmpty() { 
    List<int> ls;
    assert(ls.empty() == true);
    for (int i = 0;i < 10;++i)
        ls.insert(0, 10);
    assert(ls.len() == 10);
    while (!ls.empty())
        ls.remove(0);
}

void intInsert() {
    List<int> ls;
    int n;
    cout << "input num:";
    cin >> n;

    for (int i = 0;i < n;++i) { 
        printf("inset to pos %d, value: %d\n", i, i);
        ls.insert(i, i);
    }
    for (size_t i = 0;i < ls.len();++i) {
        assert(ls.get(i) == i);
    }
    // test destructor, if failed will resulting in memleak.
//    while (!ls.empty())
//        ls.remove(0);
}

void strInsert() {
    List<string> ls;
    ls.insertTail("hello");
    ls.insertTail(" world,");
    ls.insertTail(" I am cwp.");
    for (size_t i = 0;i < ls.len();++i) {
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

typedef std::function<void (void)> fnvv;

int main() { 
    fnvv fns[] = {
        intEmpty,
        intInsert,
        strInsert,
    };
    for (auto f:fns)
        f();
    return 0;
}
