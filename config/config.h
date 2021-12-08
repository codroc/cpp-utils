#ifndef CONFIG_H
#define CONFIG_H

#include <memory>
#include <string>
#include <map>
#include <typeinfo>
#include <boost/lexical_cast.hpp>
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
private:
    std::string _name;
    std::string _description;
};

template<class T>
class ConfigVar : public ConfigVarBase {
public:
    using ptr = std::shared_ptr<ConfigVar>;
    ConfigVar(const T& val, const std::string& name, const std::string description = "")
        : ConfigVarBase(name, description),
          _val(val)
    {}

    std::string toString() override {
        try {
            return boost::lexical_cast<std::string>(_val);
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
            _val = boost::lexical_cast<T>(val);        
            return true;
        }
        catch (std::exception& e) {
            LOG_ERROR << "ConfigVar::fromString exception " 
                << e.what() << " conver: string to " << typeid(_val).name() 
                << " name = " << getName() << "\n";
        }
        return false;
    }
private:
    T _val;
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
    // 查找相关 配置变量，如果找不到 则创建；找到则返回 对应的智能指针
    template<class T>
    static typename ConfigVar<T>::ptr lookup(const std::string& name, const T& val, const std::string& description = "") {
        auto sp = lookup<T>(name);
        if (!sp) {
            // 没找到，则创建
            sp.reset(new ConfigVar<T>(val, name, description));
            _configVars[name] = sp;
        }
        return sp;
    }
private:
    // map 用于管理所有的 ConfigVarBase::ptr，其实就是所有的 ConfigVar
    // 全局唯一
    static std::map<std::string, ConfigVarBase::ptr> _configVars;
};

#endif
