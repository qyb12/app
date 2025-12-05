#include "uiObject.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <algorithm>

using namespace std;

UiObject::UiObject() {
    cout << "UiObject 构造函数" << endl;
}

UiObject::~UiObject() {
    cout << "UiObject 析构函数" << endl;
}

// 静态注册表：子类名 → 子类元信息（父类名+创建函数）
std::unordered_map<std::string, UiObject::SubclassMeta>& UiObject::getTable() {
    static std::unordered_map<std::string, UiObject::SubclassMeta> registry;
    return registry;
}

// 扩展注册接口：新增parentName参数
void UiObject::pageRegister(const std::string& className, const std::string& parentName, UiObject::CreateFunc func) {
    auto& registry = UiObject::getTable();
    if (registry.find(className) != registry.end()) {
        std::cerr << "警告：子类 " << className << " 已重复注册！" << std::endl;
        return;
    }
    registry[className] = {parentName, func};
    std::cout << "子类 " << className << " 注册成功（父类：" << parentName << "）" << std::endl;
}

// 核心：按父类优先的拓扑顺序创建所有子类实例
std::vector<UiObject *> UiObject::createAllPage() {
    std::vector<UiObject *> instances;          // 存储最终创建的所有实例
    std::unordered_map<std::string, UiObject *> nameToInstance;  // 子类名 → 实例指针（快速查找父类实例）
    auto& registry = getTable();

    // 步骤1：获取所有子类名，并按「父类优先」排序
    std::vector<std::string> subclassNames;
    for (const auto& [name, _] : registry) subclassNames.push_back(name);
    
    // 拓扑排序：确保父类先创建
    std::sort(subclassNames.begin(), subclassNames.end(), [&](const std::string& a, const std::string& b) {
        // 规则：1. 根父类（parentName为空）优先；2. 若a的父类是b，则b排在a前面
        const auto& aMeta = registry.at(a);
        const auto& bMeta = registry.at(b);
        
        // 根父类（无父类）排最前
        if (aMeta.parentName.empty() && !bMeta.parentName.empty()) return true;
        if (!aMeta.parentName.empty() && bMeta.parentName.empty()) return false;
        
        // 若a的父类是b，则b先创建
        if (aMeta.parentName == b) return true;
        // 若b的父类是a，则a先创建
        if (bMeta.parentName == a) return false;
        
        // 其他情况保持原顺序
        return false;
    });

    // 步骤2：按排序后的顺序创建实例（父类优先）
    for (const auto& className : subclassNames) {
        const auto& meta = registry.at(className);
        UiObject* parentInstance = nullptr;

        // 查找父类实例（若父类名非空）
        if (!meta.parentName.empty()) {
            auto it = nameToInstance.find(meta.parentName);
            if (it != nameToInstance.end()) {
                parentInstance = it->second;
            } else {
                std::cerr << "错误：子类 " << className << " 的父类 " << meta.parentName << " 未注册/未创建！" << std::endl;
                continue;
            }
        }

        // 调用创建函数，传入父类实例指针
        UiObject* obj = meta.createFunc(parentInstance);
        if (obj) {
            obj->UiInit();
            instances.push_back(obj);
            nameToInstance[className] = obj;
            std::cout << "已创建子类 " << className << " 实例（父类实例：" << parentInstance << "）" << std::endl;
        } else {
            std::cerr << "创建子类 " << className << " 实例失败！" << std::endl;
        }
    }

    return instances;
}

// 静态接口：获取所有已注册的子类名
std::vector<std::string> UiObject::getAllSubclassName() {
    std::vector<std::string> names;
    auto& registry = UiObject::getTable();
    for (const auto& [name, _] : registry) {
        names.push_back(name);
    }
    return names;
}