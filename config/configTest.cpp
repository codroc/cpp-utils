#include "config.h"
#include "flog.h"
#include "logger.h"

#if 0
ConfigVar<int>::ptr g_sp_int = Config::lookup("system.port", (int)2222, "system port");
ConfigVar<float>::ptr g_sp_fint = Config::lookup("system.port", (float)2222, "system port");
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

// 支持自定义类型

#include <string>
class Person {
public:
    std::string name;
    int age;
    bool sex;
};

// from std::string to Person
template<>
class LexicalCast<std::string, Person> {
public:
    Person operator()(const std::string& str) {
        YAML::Node node = YAML::Load(str);
        Person p;
        p.name = node["name"].as<std::string>();
        p.age = node["age"].as<int>();
        p.sex = node["sex"].as<bool>();
        return p;
    }
};

// from Person to std::string
template<>
class LexicalCast<Person, std::string> {
public:
    std::string operator()(const Person& p) {
        YAML::Node node;
        node["name"] = p.name;
        node["age"] = p.age;
        node["sex"] = p.sex;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

ConfigVar<Person>::ptr g_sp_person = Config::lookup("class.person", Person(), "class person");
ConfigVar<std::vector<Person>>::ptr g_sp_vec_person = Config::lookup("class.vec_person", std::vector<Person>(), "class persons");
ConfigVar<std::map<std::string, std::vector<Person>>>::ptr g_sp_map_vec_person = 
    Config::lookup("class.map_vec_person", std::map<std::string, std::vector<Person>>(), "class map vec persons");

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
