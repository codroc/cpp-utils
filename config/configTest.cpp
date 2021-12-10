#include "config.h"
#include "flog.h"

ConfigVar<int>::ptr g_sp_int = Config::lookup("system.port", (int)2222, "system port");
ConfigVar<float>::ptr g_sp_float = Config::lookup("system.fee", (float)6.5, "system fee");

int main(int argc, char** argv) {
    Config::traverse([](ConfigVarBase::ptr& ptr) {
            LOG_INFO << "before: " << ptr->getName() << " = " << ptr->toString() << "\n";
            });

    Config::loadFromYaml("config.yaml");

    Config::traverse([](ConfigVarBase::ptr& ptr) {
            LOG_INFO << "after: " << ptr->getName() << " = " << ptr->toString() << "\n";
            });
    return 0;
}
