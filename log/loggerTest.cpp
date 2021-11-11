#include "logger.h"
#include "logStream.h"

int main() {
    for (int i = 0;i < 1000000;++i)
        LOG_WARN << "test one\n";
    return 0;
}
