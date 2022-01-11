#include "kv_skiplist.h"
#include "logger.h"
#include <string>

int main(int argc, char **argv) {
    Logger::setBufferLevel(Logger::kLineBuffer);

    SkipList<int, std::string> lst;
    lst.insert(1, "codroc");
    lst.insert(2, " learn skiplist");
    lst.insert(3, "hello");
    lst.insert(4, "world");
    lst.insert(5, "are you ok");
    lst.insert(6, "what");

    lst.display();

    lst.erase(1);
    lst.erase(5);
    lst.erase(3);

    lst.display();
    return 0;
}
