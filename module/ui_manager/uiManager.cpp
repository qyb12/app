#include "uiManager.h"

#include <iostream>
#include <list>

using namespace std;

UiManager& UiManager::GetInstance() {  // C++11及以上：局部静态变量初始化线程安全
    static UiManager instance;
    return instance;
}

// 静态注册表：子类名 → 子类元信息（父类名+创建函数）
std::unordered_map<std::string, UiManager::SubclassMeta>& UiManager::getTable() {
    static std::unordered_map<std::string, UiManager::SubclassMeta> registry;
    return registry;
}

// 扩展注册接口：新增parentName参数
void UiManager::pageRegister(const std::string& className, const std::string& parentName, UiManager::CreateFunc func) {
    auto& registry = getTable();
    if (registry.find(className) != registry.end()) {
        std::cout<<"create repeatedly"<<className<<std::endl;
        return;
    }
    registry[className] = {className, nullptr, nullptr, parentName, nullptr, nullptr, func, nullptr, {}};
    if (parentName.empty()) {   // 这里把没有父对象的类放到pages数组里面作为森林的根节点
        pages.push_back(&registry[className]);
    }
    std::cout<<"registered successfully class name ="<<className<<"parent name = "<<parentName<<std::endl;
}

void UiManager::createAllPage() {
    // 构建整个对象森林
    auto& registry = getTable();
    auto registry_copy = registry; 
    while (!registry_copy.empty()) {    // 先构建整个森林
        for (auto& [_, info] : registry_copy) {  // 把表里面的内容全部遍历一遍初始化
            if (info.parentName.empty()) {  // 先把没有父对象的结点全删除
                registry_copy.erase(info.selfName);
                break;
            }
            bool flag = false;
            for (auto& page : pages) {  // 递归遍历森林
                auto found = findFromForest(*page, info.parentName);
                if (found != nullptr) {
                    info.parent = found;    // 标记父结点
                    found->children.push_back(&info);   // 标记子节点
                    registry_copy.erase(info.selfName);
                    flag = true;
                    break;
                }
            }
            if (flag) { // 如果找到之后删除元素，迭代器失效直接退出来
                break;
            }
        }
    }

    // 初始化对象森林上面全部对象(只是构造出来建立连接)
    for (auto& page : pages) {  // 先把全部的对象构造出来
        foreachTree(*page, [](UiManager::SubclassMeta& node) {  // 把有父对象的对象全部初始化
            if (node.createFunc != nullptr && node.uiSelf == nullptr) {   // 如果初始化函数存在
                if (node.parent != nullptr) {   // 如果有父对象(没有父对象也不用赋值了，之前已经赋值过了)
                    node.uiParent = node.parent->uiSelf;  // 把父对象指针赋值
                }
                node.uiSelf = node.createFunc(node.uiParent);   // 获取自己对象指针
                if (node.uiSelf) {  // 如果创建成功
                    std::cout<<"created! class = "<<node.selfName<<"parent = "<<node.parentName<<std::endl;
                } else {    // 创建失败
                    std::cout<<"UiObject obj is nullptr, createFunc fail!"<<std::endl;
                }
            } else {    // 初始化函数为空
                std::cout<<"createFunc is nullptr!"<<std::endl;
            }
        });
    }
}

UiManager::SubclassMeta *UiManager::findFromForest(UiManager::SubclassMeta &node, const std::string &name) {
    if (node.selfName == name) {
        return &node;
    }
    for (auto& child : node.children) {
        auto found = findFromForest(*child, name);
        if (found != nullptr) {
            return found;
        }
    }
    return nullptr;
}

void UiManager::foreachTree(UiManager::SubclassMeta &node, std::function<void(UiManager::SubclassMeta&)> func) {
    func(node);
    for (auto& child : node.children) {
        foreachTree(*child, func);
    }
}

void UiManager::foreachTreeBack(UiManager::SubclassMeta &node, std::function<void(UiManager::SubclassMeta&)> func) {
    for (auto& child : node.children) {
        foreachTreeBack(*child, func);
    }
    func(node);
}

void UiManager::initPage() {    // 开机初始化GUI页面
    for (auto& page : pages) {  // 遍历全部森林的根(找到与firstPage相同的根)
        if (firstPage == page->selfName) {  // 先初始化第一个页面
            foreachTree(*page, [](UiManager::SubclassMeta& node) {  // 遍历树
                if (node.uiSelf && node.lvSelf == nullptr) {
                    if (node.parent) {   // 如果有父对象(没有父对象也不用赋值了，之前已经赋值过了)
                        node.lvParent = node.parent->lvSelf;
                    } 
                    if (node.lvParent == nullptr) { // 如果是空直接赋值为lvgl默认屏幕
                        node.lvParent = lv_scr_act();
                    }
                    node.lvSelf = node.uiSelf->Init(node.lvParent); // 初始化函数, 在UiInit里面实现lvgl初始化代码, 并且一定要返回lvgl指针
                    EventManager::GetInstance().PublishEvent<pageEvent>(node.selfName, PageEventId::DISPLAYED);
                }
            });
        }
    }
}

void UiManager::navigationToPage(const std::string &pageName) {
    for (auto& page : pages) {
        auto found = findFromForest(*page, pageName);
        if (found && found->parent != nullptr) {    // 不是根，直接把同级的对象全部隐藏(非常驻控件)
            for (auto& child : found->parent->children) {
                if (child != found && child->uiSelf->mode == UiObject::UNIQUE_MODE) {
                    foreachTreeBack(*child, [&](UiManager::SubclassMeta& node) {
                        if (node.uiSelf && node.lvSelf) {
                            node.uiSelf->Deinit(node.lvSelf);
                            node.lvSelf = nullptr;
                            node.lvParent = nullptr;
                            EventManager::GetInstance().PublishEvent<pageEvent>(node.selfName, PageEventId::HIDDEN);
                        }
                    });
                } else if (child == found && child->uiSelf->mode == UiObject::UNIQUE_MODE) {
                    foreachTree(*child, [&](UiManager::SubclassMeta& node) {
                        if (node.uiSelf && node.lvSelf == nullptr) {
                            if (node.parent) {   // 如果有父对象(没有父对象也不用赋值了，之前已经赋值过了)
                                node.lvParent = node.parent->lvSelf;
                            } 
                            if (node.lvParent == nullptr) { // 如果是空直接赋值为lvgl默认屏幕
                                node.lvParent = lv_scr_act();
                            }
                            node.lvSelf = node.uiSelf->Init(node.lvParent); // 初始化函数, 在UiInit里面实现lvgl初始化代码, 并且一定要返回lvgl指针
                            EventManager::GetInstance().PublishEvent<pageEvent>(node.selfName, PageEventId::DISPLAYED);
                        } 
                    });
                }
            }
        } else if (found && found->parent == nullptr) { // 是根，直接把同级的对象全部隐藏(非常驻控件)
            for (auto& child : pages) {
                if (child != found && child->uiSelf->mode == UiObject::UNIQUE_MODE) {
                    foreachTreeBack(*child, [&](UiManager::SubclassMeta& node) {
                        if (node.uiSelf && node.lvSelf) {
                            node.uiSelf->Deinit(node.lvSelf);
                            node.lvSelf = nullptr;
                            node.lvParent = nullptr;
                            EventManager::GetInstance().PublishEvent<pageEvent>(node.selfName, PageEventId::HIDDEN);
                        }
                    });
                } else if (child == found && child->uiSelf->mode == UiObject::UNIQUE_MODE) {
                    foreachTree(*child, [&](UiManager::SubclassMeta& node) {
                        if (node.uiSelf && node.lvSelf == nullptr) {
                            if (node.parent) {   // 如果有父对象(没有父对象也不用赋值了，之前已经赋值过了)
                                node.lvParent = node.parent->lvSelf;
                            } 
                            if (node.lvParent == nullptr) { // 如果是空直接赋值为lvgl默认屏幕
                                node.lvParent = lv_scr_act();
                            }
                            node.lvSelf = node.uiSelf->Init(node.lvParent); // 初始化函数, 在UiInit里面实现lvgl初始化代码, 并且一定要返回lvgl指针
                            EventManager::GetInstance().PublishEvent<pageEvent>(node.selfName, PageEventId::DISPLAYED);
                        } 
                    });
                }
            }
        }
        if (found != nullptr) {
            break;
        }
    }
}
