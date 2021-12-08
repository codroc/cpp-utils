#include "config.h"
#include "flog.h"

ConfigVar<int>::ptr g_sp_int = Config::lookup("system.port", (int)2222, "system port");

int main(int argc, char** argv) {
    LOG_INFO << g_sp_int->getName() << " = " << g_sp_int->toString() << "\n";
    return 0;
}
