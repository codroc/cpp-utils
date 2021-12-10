#include "config.h"

#include <yaml-cpp/yaml.h>

std::map<std::string, ConfigVarBase::ptr> Config::_configVars;

// map 中 key 的格式 是： "A.B = 10", "A.C = 20"
// 而 yaml 文件中的格式 是：
// A:
//   B: 10
//   C: 20

ConfigVarBase::ptr Config::find(const std::string& name) {
    auto it =  _configVars.find(name);
    if (it == _configVars.end())
        return nullptr;
    return it->second;
}

void fromYaml(const std::string& name, const YAML::Node& node) {
    if (node.IsNull()) {
    }
    else if (node.IsScalar()) {
        ConfigVarBase::ptr p = Config::find(name);
        if (!p)
            Config::lookup(name, node.Scalar());
        else
            p->fromString(node.Scalar());
    }
    else if (node.IsMap() || node.IsSequence()) {
        for (auto it = node.begin(); it != node.end(); ++it) {
            fromYaml(name.empty() ? it->first.as<std::string>() : 
                    name + "." + it->first.as<std::string>(), it->second);
        }
    }
}

void Config::loadFromYaml(const char* filename) {
    YAML::Node node = YAML::LoadFile(filename);
    fromYaml("", node);
}

void Config::traverse(std::function<void(ConfigVarBase::ptr&)> func) {
    for (auto it = _configVars.begin(); it != _configVars.end(); ++it)
        func(it->second);
}
