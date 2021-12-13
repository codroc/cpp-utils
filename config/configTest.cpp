#include "config.h"
#include "flog.h"
#include "logger.h"

ConfigVar<int>::ptr g_sp_int = Config::lookup("system.port", (int)2222, "system port");
ConfigVar<float>::ptr g_sp_fint = Config::lookup("system.port", (float)2222, "system port");
#if 0
ConfigVar<float>::ptr g_sp_float = Config::lookup("system.fee", (float)6.5, "system fee");
ConfigVar<std::string>::ptr g_sp_string = Config::lookup("system.ip", std::string("127.0.0.1"), "system ip");
ConfigVar<std::vector<int>>::ptr g_sp_vec_int = Config::lookup("system.vec_int", std::vector<int>{1,2}, "system vec int");
ConfigVar<std::list<int>>::ptr g_sp_list_int = Config::lookup("system.list_int", std::list<int>{3,5}, "system list int");
ConfigVar<std::set<int>>::ptr g_sp_set_int = Config::lookup("system.set_int", std::set<int>{30,50}, "system set int");
ConfigVar<std::unordered_set<int>>::ptr g_sp_unordered_set_int = Config::lookup("system.unordered_set_int", 
        std::unordered_set<int>{50,30}, "system unordered_set int");
ConfigVar<std::map<std::string, std::string>>::ptr g_sp_map_name_phone =
    Config::lookup("system.map_name_phone", 
            std::map<std::string,std::string>{{"cwp", "15057760108"}},
            "system map int int");
ConfigVar<std::unordered_map<std::string, std::string>>::ptr g_sp_umap_name_phone =
    Config::lookup("system.umap_name_phone", 
            std::unordered_map<std::string,std::string>{{"cwp", "15057760108"}},
            "system umap int int");
#endif

int main(int argc, char** argv) {
    Logger::setBufferLevel(Logger::kLineBuffer);

    Config::traverse([](ConfigVarBase::ptr& ptr) {
            LOG_INFO << "before: " << ptr->getName() << " = " << ptr->toString() << "\n";
            });

    Config::loadFromYaml("config.yaml");

    Config::traverse([](ConfigVarBase::ptr& ptr) {
            LOG_INFO << "after: " << ptr->getName() << " = " << ptr->toString() << "\n";
            });
    return 0;
}
