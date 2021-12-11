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

void listAllNodes(const std::string& name, const YAML::Node& node, std::vector<std::pair<std::string, YAML::Node>>& allNodes) {
    allNodes.push_back(std::make_pair(name, node));
    if (node.IsNull()) {
    }
    else if (node.IsScalar()) {
    //    // 如果遍历到常量了，则说明已经可以构建一个配置变量了。
    //    ConfigVarBase::ptr p = Config::find(name);
    //    if (!p) {
    //        // 找不到约定的配置变量，而出现在了 .yaml 文件中
    //        Config::lookup(name, node.Scalar());
    //    }
    //    else
    //        p->fromString(node.Scalar());
    }
    else if (node.IsSequence()) {
    //    ConfigVarBase::ptr p = Config::find(name);
    //    std::stringstream ss;
    //    ss << node;
    //    if (!p) {
    //        // 找不到约定的配置变量，而出现在了 .yaml 文件中
    //        Config::lookup(name, ss.str());
    //    }
    //    else
    //        p->fromString(ss.str());
    //    for (auto it = node.begin(); it != node.end(); ++it)
    //        listAllNodes(name, *it, allNodes);
    }
    else if (node.IsMap()) {
        for (auto it = node.begin(); it != node.end(); ++it) {
            listAllNodes(name.empty() ? it->first.as<std::string>() : 
                    name + "." + it->first.Scalar(), it->second, allNodes);
        }
    }
}

void Config::loadFromYaml(const char* filename) {
    YAML::Node node = YAML::LoadFile(filename);
    std::vector<std::pair<std::string, YAML::Node>> allNodes;
    listAllNodes("", node, allNodes);

    for (auto i : allNodes) {
        std::string name = i.first;
        if (name.empty())
            continue;
        ConfigVarBase::ptr p = Config::find(name);
        if (p) {
            std::stringstream ss;
            ss << i.second;
            p->fromString(ss.str());
        }
    }
}

void Config::traverse(std::function<void(ConfigVarBase::ptr&)> func) {
    for (auto it = _configVars.begin(); it != _configVars.end(); ++it)
        func(it->second);
}
