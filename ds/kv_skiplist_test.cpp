#include "kv_skiplist.h"
#include "logger.h"
#include <string>

int main(int argc, char **argv) {
    Logger::setBufferLevel(Logger::kLineBuffer);
    SkipList<int, std::string> lst;
    lst.insert(1, "codroc");
    lst.insert(2, " learn skiplist");
    lst.display();
    return 0;
}
