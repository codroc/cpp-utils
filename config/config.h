#ifndef CONFIG_H
#define CONFIG_H

#include <assert.h>
#include <stdio.h> // for test
#include <iostream> // for test

#include <memory>
#include <string>
#include <typeinfo>
#include <sstream>

#include <vector>
#include <list>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>

#include "flog.h"

// 配置变量
// 一般都有 名字 以及对该变量的 描述

class ConfigVarBase {
public:
    using ptr = std::shared_ptr<ConfigVarBase>;
    ConfigVarBase(const std::string& name, const std::string& description = "")
        : _name(name),
          _description(description)
    {}

    virtual ~ConfigVarBase() {}

    const std::string& getName() const { return _name; }
    const std::string& getDescription() const { return _description; }

    // 把 配置变量 转成 字符串
    virtual std::string toString() = 0;
    // 从字符串初始化值
    virtual bool fromString(const std::string& val) = 0;
    
    virtual std::string getTypeName() const = 0;
private:
    std::string _name;
    std::string _description;
};

// cast from F-type to T-type
template<class F, class T>
class LexicalCast {
public:
    T operator()(const F& val) {
        return boost::lexical_cast<T>(val);
    }
};

// cast from std::vector<T> to string
template<class T>
class LexicalCast<std::vector<T>, std::string> {
public:
    std::string operator()(const std::vector<T>& vec) {
        YAML::Node node = YAML::Load("[]");
        for (auto& v : vec)
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(v)));
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// cast from std::string to std::vector<T>
template<class T>
class LexicalCast<std::string, std::vector<T>> {
public:
    std::vector<T> operator()(const std::string& str) {
        std::vector<T> vec;
        YAML::Node node = YAML::Load(str);
        std::stringstream ss;
        for (size_t i = 0;i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

// cast from std::list<T> to string
template<class T>
class LexicalCast<std::list<T>, std::string> {
public:
    std::string operator()(const std::list<T>& vec) {
        YAML::Node node = YAML::Load("[]");
        for (auto& v : vec)
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(v)));
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// cast from std::string to std::list<T>
template<class T>
class LexicalCast<std::string, std::list<T>> {
public:
    std::list<T> operator()(const std::string& str) {
        std::list<T> vec;
        YAML::Node node = YAML::Load(str);
        std::stringstream ss;
        for (size_t i = 0;i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

// cast from std::set<T> to string
template<class T>
class LexicalCast<std::set<T>, std::string> {
public:
    std::string operator()(const std::set<T>& vec) {
        YAML::Node node = YAML::Load("[]");
        for (auto& v : vec)
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(v)));
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// cast from std::string to std::set<T>
template<class T>
class LexicalCast<std::string, std::set<T>> {
public:
    std::set<T> operator()(const std::string& str) {
        std::set<T> st;
        YAML::Node node = YAML::Load(str);
        std::stringstream ss;
        for (size_t i = 0;i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            st.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return st;
    }
};

// cast from std::unordered_set<T> to string
template<class T>
class LexicalCast<std::unordered_set<T>, std::string> {
public:
    std::string operator()(const std::unordered_set<T>& vec) {
        YAML::Node node = YAML::Load("[]");
        for (auto& v : vec)
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(v)));
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// cast from std::string to std::unordered_set<T>
template<class T>
class LexicalCast<std::string, std::unordered_set<T>> {
public:
    std::unordered_set<T> operator()(const std::string& str) {
        std::unordered_set<T> st;
        YAML::Node node = YAML::Load(str);
        std::stringstream ss;
        for (size_t i = 0;i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            st.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return st;
    }
};

// cast from std::map<K, V> to std::string
template<class K, class V>
class LexicalCast<std::map<K, V>, std::string> {
public:
    std::string operator()(const std::map<K, V>& mp) {
        YAML::Node node;
        for (auto it = mp.begin(); it != mp.end(); ++it) {
            node.force_insert(it->first, YAML::Load(LexicalCast<V, std::string>()(it->second)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// cast from std::string to std::map<K, V> 
template<class K, class V>
class LexicalCast<std::string, std::map<K, V>> {
public:
    std::map<K, V> operator()(const std::string& str) {
        YAML::Node node = YAML::Load(str);
        std::map<K, V> mp;
        std::stringstream ss;
        for (auto it = node.begin(); it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            mp.insert(std::make_pair(LexicalCast<std::string, K>()(it->first.Scalar()), LexicalCast<std::string, V>()(ss.str())));
        }
        return mp;
    }
};

// cast from std::unordered_map<K, V> to std::string
template<class K, class V>
class LexicalCast<std::unordered_map<K, V>, std::string> {
public:
    std::string operator()(const std::unordered_map<K, V>& mp) {
        YAML::Node node;
        for (auto it = mp.begin(); it != mp.end(); ++it) {
            node.force_insert(it->first, it->second);
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// cast from std::string to std::unordered_map<K, V> 
template<class K, class V>
class LexicalCast<std::string, std::unordered_map<K, V>> {
public:
    std::unordered_map<K, V> operator()(const std::string& str) {
        YAML::Node node = YAML::Load(str);
        std::unordered_map<K, V> mp;
        std::stringstream ss;
        for (auto it = node.begin(); it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            mp.insert(std::make_pair(LexicalCast<std::string, K>()(it->first.Scalar()), LexicalCast<std::string, V>()(ss.str())));
        }
        return mp;
    }
};
// FromStr: 定义了 T operator(const std::string&);
// ToStr:   定义了 std::string operator(const T&);
template<class T, class FromStr = LexicalCast<std::string, T>, class ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase {
public:
    using ptr = std::shared_ptr<ConfigVar>;
    using OnChangeCallBack = std::function<void(const T& oldVal, const T& newVal)>;
    ConfigVar(const T& val, const std::string& name, const std::string description = "")
        : ConfigVarBase(name, description),
          _val(val)
    {}

    std::string toString() override {
        try {
            // return boost::lexical_cast<std::string>(_val);
            return ToStr()(_val);
        }
        catch(std::exception& e) {
            LOG_ERROR << "ConfigVar::toString exception " 
                << e.what() << " conver: " << typeid(_val).name() 
                << " to string" << " name = " << getName() << "\n";
        }
        return "";
    }

    bool fromString(const std::string& val) override {
        try {
            // _val = boost::lexical_cast<T>(val);        
            setValue(FromStr()(val));
            return true;
        }
        catch (std::exception& e) {
            LOG_ERROR << "ConfigVar::fromString exception " 
                << e.what() << " conver: string to " << typeid(_val).name() 
                << " name = " << getName() << "\n";
        }
        return false;
    }
    
    T getValue() const { return _val; }
    void setValue(const T& v) {
        if (v == _val)
            return;
        if (_cb)
            _cb(_val, v);
        _val = v;
    }

    std::string getTypeName() const override { return typeid(T).name(); }

    void setOnChangeCallBack(OnChangeCallBack cb) { _cb = cb; }
    void delOnChangeCallBack() { _cb = nullptr; }
private:
    T _val;
    OnChangeCallBack _cb;
};

class Config {
public:
    using ptr = std::shared_ptr<Config>;
    
    // 查找相关 配置变量，找不到 返回 nullptr
    template<class T>
    static typename ConfigVar<T>::ptr lookup(const std::string& name) {
        auto it = _configVars.find(name);
        if (it == _configVars.end())
            return nullptr;
        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second); 
    }
    // 查找相关 配置变量，如果找不到 则创建；找到则返回 对应的智能指针,
    // 因为涉及到 val 的具体类型，所以不能用于 yaml 配置文件
    template<class T>
    static typename ConfigVar<T>::ptr lookup(const std::string& name, const T& val, const std::string& description = "") {
        auto sp = lookup<T>(name);
        if (!sp) {
            // 没找到：有 2 中可能
            // 1. map 中没有以 name 为 key 的 entry
            // 2. map 中有，但是 type 不一致，导致 std::dynamic_pointer_cast<ConfigVar<T>>(it->second) 返回 nullptr
            auto it = _configVars.find(name);
            if (it != _configVars.end()) {
                // 情况 2
                LOG_INFO << "Config::lookup [" << name << ":"
                         << it->second->toString() << "] type is not consistency! " << typeid(T).name() 
                         << " is provided, but " << it->second->getTypeName() << " is needed.\n";
            }
            else {
                // 情况 1
                sp.reset(new ConfigVar<T>(val, name, description));
                _configVars[name] = sp;
            }
        }
        return sp;
    }

    // 从 yaml 配置文件中读取 配置变量
    static void loadFromYaml(const char* filename);
    static ConfigVarBase::ptr find(const std::string& name);

    // 遍历所有的 config var，并对每一个 ptr 做 lambda
    static void traverse(std::function<void(ConfigVarBase::ptr&)>);
private:
    // map 用于管理所有的 ConfigVarBase::ptr，其实就是所有的 ConfigVar
    // 全局唯一
    static std::map<std::string, ConfigVarBase::ptr> _configVars;
};

#endif
