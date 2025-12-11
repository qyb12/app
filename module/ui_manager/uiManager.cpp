#include "uiManager.h"

#include <iostream>
#include <list>

using namespace std;

lv_indev_t *UiManager::mouse_indev = NULL; // 鼠标输入设备

UiManager& UiManager::GetInstance() {  // C++11及以上：局部静态变量初始化线程安全
    static UiManager instance;
    return instance;
}

// 静态注册表：子类名 → 子类元信息（父类名+创建函数）
std::unordered_map<std::string, UiManager::SubclassMeta>& UiManager::getTable() {
    static std::unordered_map<std::string, UiManager::SubclassMeta> registry;
    return registry;
}

/**
 * @brief 注册一个页面类
 * @param className 页面类名
 * @param parentName 父类名（空字符串表示无父类）
 * @param func 创建函数指针，用于创建页面实例
 * @return 无返回值
 */
void UiManager::pageRegister(const std::string& className, const std::string& parentName, UiManager::CreateFunc func) {
    auto& registry = getTable();
    if (registry.find(className) != registry.end()) {
        // std::cout<<"create repeatedly"<<className<<std::endl;
        return;
    }
    registry[className] = {className, nullptr, nullptr, parentName, nullptr, nullptr, func, nullptr, {}};
    if (parentName.empty()) {   // 这里把没有父对象的类放到pages数组里面作为森林的根节点
        pages.push_back(&registry[className]);
    }
    // std::cout<<"registered successfully class name ="<<className<<"parent name = "<<parentName<<std::endl;
}

/**
 * @brief 批量创建所有页面实例
 * @return 无返回值
 */
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
                    auto itFound = registry.find(info.selfName);
                    itFound->second.parent = found;
                    found->children.push_back(&itFound->second);   // 标记子节点
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
                if (node.parent != nullptr) {   // 如果有父对象(没有父对象也不用赋值了,就让他是空)
                    node.uiParent = node.parent->uiSelf;  // 把父对象指针赋值
                }
                node.uiSelf = node.createFunc(node.uiParent);   // 获取自己对象指针
                // if (node.uiSelf) {  // 如果创建成功
                //     std::cout<<"created! class = "<<node.selfName<<"parent = "<<node.parentName<<std::endl;
                // } else {    // 创建失败
                //     std::cout<<"UiObject obj is nullptr, createFunc fail!"<<std::endl;
                // }
            } else {    // 初始化函数为空
                // std::cout<<"createFunc is nullptr!"<<std::endl;
            }
        });
    }
    // 初始化第一个对象树
    navigationToPage(firstPage);
    //进行鼠标事件回调绑定
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

void UiManager::foreachTreeOrder(UiManager::SubclassMeta &node, std::function<void(UiManager::SubclassMeta&)> func) {
    func(node);
    for (auto& child : node.children) {
        if (node.uiSelf->mode == UiObject::PERMANENT_MODE) {
            foreachTreeOrder(*child, func);
        }
    }
    for (auto& child : node.children) {
        if (node.uiSelf->mode == UiObject::CONDITION_MODE) {
            foreachTreeOrder(*child, func);
        }
    }
}

void UiManager::foreachTree1(UiManager::SubclassMeta &node, std::function<void(UiManager::SubclassMeta&)> func) {
    func(node);
    for (auto& child : node.children) {
        if (node.uiSelf->mode == UiObject::UNIQUE_MODE) {   // 如果是unique模式直接返回不初始化子控件
            continue;
        } else if (node.uiSelf->mode == UiObject::CONDITION_MODE) { // 如果是condition模式，只注册条件函数，也不初始化
            conditionRegister(node);
            continue;
        } else if (node.uiSelf->mode == UiObject::PERMANENT_MODE) { // 如果是permanent模式，递归遍历子控件
            foreachTree1(*child, func);
        }
    }
}

/**
 * 手动判断点是否在 lv_area_t 区域内
 * @param area 目标区域
 * @param x 点的x坐标
 * @param y 点的y坐标
 * @return true=在区域内，false=不在
 */
bool UiManager::area_is_point_in(const lv_area_t &area, lv_coord_t &x, lv_coord_t &y) {
    // 统一区域坐标顺序（处理 x1 > x2 或 y1 > y2 的情况）
    int32_t x_min = LV_MIN(area.x1, area.x2);
    int32_t x_max = LV_MAX(area.x1, area.x2);
    int32_t y_min = LV_MIN(area.y1, area.y2);
    int32_t y_max = LV_MAX(area.y1, area.y2);

    // 核心判断：点的x在[x_min, x_max]且y在[y_min, y_max]
    return (x >= x_min && x <= x_max) && (y >= y_min && y <= y_max);
}

/**
 * 定时器回调：轮询鼠标按下状态 + 坐标判断
 * @param timer 定时器对象
 */
void UiManager::mouse_pressed_event_cb(struct _lv_indev_drv_t *, uint8_t state) {
    if (state == LV_INDEV_STATE_PR && mouse_indev) {
        for (auto& page : UiManager::GetInstance().pressedPagesVector) {
            if (page->lvSelf == nullptr) {
                continue;
            }
            lv_area_t coords;
            lv_obj_get_coords(page->lvSelf, &coords);
            if (UiManager::area_is_point_in(coords, mouse_indev->proc.types.pointer.act_point.x, mouse_indev->proc.types.pointer.act_point.y)) {
                UiManager::GetInstance().pressedPagesMap[page->selfName] = true;
            } else {
                UiManager::GetInstance().pressedPagesMap[page->selfName] = false;
            }
        }
        EventManager::GetInstance().PublishEvent<pagePressedEvent>();
        // printf("mouse_pressed_event_cb %d, %d\n", mouse_indev->proc.types.pointer.act_point.x, mouse_indev->proc.types.pointer.act_point.y);
    }
}

void UiManager::initPage() {    // 开机初始化GUI页面
    for (auto& page : pages) {  // 递归遍历森林
        if (page->uiSelf->mode == UiObject::UNIQUE_MODE) {
            continue;
        } else if (page->uiSelf->mode == UiObject::CONDITION_MODE) {
            conditionRegister(*page);
            continue;
        } else if (page->uiSelf->mode == UiObject::PERMANENT_MODE) {
            foreachTree1(*page, [&](UiManager::SubclassMeta& node) {  // 遍历树创建
                initPage(node);
            });
        }
    }

    mouse_indev = lv_indev_get_next(NULL);
    while(mouse_indev) {
        if(lv_indev_get_type(mouse_indev) == LV_INDEV_TYPE_POINTER) {
            break;
        }
        mouse_indev = lv_indev_get_next(mouse_indev);
    }
    mouse_indev->driver->feedback_cb = mouse_pressed_event_cb;

    navigationToPage(firstPage);
}

void UiManager::navigationToPage(const std::string &pageName) {
    auto& registry = getTable();    // 仅调用一次，复用引用
    auto it = registry.find(pageName);
    if (it == registry.end()) { // 如果没找到直接退出
        return;
    }
    if (it->second.lvSelf) {    // 存在，并没有被释放，直接退出
        return;
    }
    auto temp = &it->second;
    while (temp->parent) {  // 遍历父结点
        if (temp->parent->lvSelf == nullptr) {  // 父对象一直往上找，但凡存在一个是隐藏的直接退出
            return;
        }
        temp = temp->parent;
    }
    // 获取到兄弟结点数据
    std::vector<struct SubclassMeta *> *tempVector = nullptr;
    if (it->second.parent) {
        tempVector = &it->second.parent->children;
    } else {
        tempVector = &pages;
    }
    if (it->second.uiSelf->mode == UiObject::UNIQUE_MODE) {
        for (auto& bro : *tempVector) {
            if (bro->uiSelf != it->second.uiSelf && bro->uiSelf->mode == UiObject::UNIQUE_MODE && bro->lvSelf) {
                foreachTreeBack(*bro, [&](UiManager::SubclassMeta& node) {  // 遍历树
                    deletePage(node);
                });
            } else if (bro->uiSelf == it->second.uiSelf && it->second.lvSelf == nullptr) {
                foreachTree1(it->second, [&](UiManager::SubclassMeta& node) {  // 遍历树创建
                    initPage(node);
                });
            }
        }
    } else if (it->second.uiSelf->mode == UiObject::PERMANENT_MODE && it->second.lvSelf == nullptr) {
        foreachTree1(it->second, [&](UiManager::SubclassMeta& node) {  // 遍历树创建
            initPage(node);
        });
    } else if (it->second.uiSelf->mode == UiObject::CONDITION_MODE && it->second.lvSelf == nullptr) {
        if (pressedPagesMap.find(it->second.selfName) != pressedPagesMap.end()) {
            pressedPagesMap[it->second.selfName] = true;
        }
        initPage(it->second); // 无条件显示自己
        foreachTree1(it->second, [&](UiManager::SubclassMeta& node) {  // 遍历树创建
            initPage(node);
        });
    }
}

bool UiManager::isVisible(const std::string &pageName) {
    auto it = getTable().find(pageName);
    return it->second.lvSelf;
}

bool UiManager::isPressed(const std::string &pageName) {
    auto it = pressedPagesMap.find(pageName);
    if (it != pressedPagesMap.end()) {
        return it->second;
    }
    return false;
}

void UiManager::conditionRegister(UiManager::SubclassMeta &node) {
    if (node.uiSelf->displayCondition) {
        for (auto id : node.uiSelf->displayEventIds) {
            if (id == typeid(pageEvent).hash_code()) {
                printf("displayCondition pageEvent node = %s\n", node.selfName.c_str());
                EventManager::GetInstance().RegisterListener<pageEvent>(node.uiSelf, [&]() {
                    if (node.lvSelf == nullptr && node.uiSelf->displayCondition()) {
                        initPage(node); // 无条件显示自己
                        foreachTree1(node, [&](UiManager::SubclassMeta& node) {  // 遍历树创建
                            initPage(node);
                        });
                    }
                });
            }
        }
    }
    if (node.uiSelf->hiddenCondition) {
        for (auto id : node.uiSelf->hiddenEventIds) {
            if (id == typeid(pageEvent).hash_code()) {
                printf("hiddenCondition pageEvent node = %s\n", node.selfName.c_str());
                EventManager::GetInstance().RegisterListener<pageEvent>(node.uiSelf, [&]() {
                    if (node.lvSelf && node.uiSelf->hiddenCondition()) {
                        foreachTreeBack(node, [&](UiManager::SubclassMeta& node) {  // 遍历树
                            deletePage(node);
                        });
                        conditionRegister(node);
                    }
                });
            } else if (id == typeid(pagePressedEvent).hash_code()) {
                printf("hiddenCondition pagePressedEvent node = %s\n", node.selfName.c_str());
                if (pressedPagesMap.find(node.selfName) == pressedPagesMap.end()) {
                    pressedPagesVector.push_back(&node);
                    pressedPagesMap[node.selfName] = false;
                }
                EventManager::GetInstance().RegisterListener<pagePressedEvent>(node.uiSelf, [&]() {
                    if (node.lvSelf && node.uiSelf->hiddenCondition()) {
                        foreachTreeBack(node, [&](UiManager::SubclassMeta& node) {  // 遍历树
                            deletePage(node);
                        });
                        conditionRegister(node);
                    }
                });
            }
        }
    }
}

void UiManager::conditionrUnregister(UiManager::SubclassMeta &node) {
    for (auto id : node.uiSelf->displayEventIds) {
        if (id == typeid(pageEvent).hash_code()) {
            printf("UnregisterAllListeners node %s.\n", node.selfName.c_str());
            EventManager::GetInstance().UnregisterAllListeners<pageEvent>(node.uiSelf);
        }
    }
    for (auto id : node.uiSelf->hiddenEventIds) {
        if (id == typeid(pageEvent).hash_code()) {
            EventManager::GetInstance().UnregisterAllListeners<pageEvent>(node.uiSelf);
        } else if (id == typeid(pagePressedEvent).hash_code()) {
            EventManager::GetInstance().UnregisterAllListeners<pagePressedEvent>(node.uiSelf);
        }
    }
}

void UiManager::createPage(UiManager::SubclassMeta &node) {
    if (node.uiSelf && node.lvSelf == nullptr) {
        if (node.uiSelf->mode == UiObject::PERMANENT_MODE) {    // 如果页面是常显，无条件显示
            initPage(node);
        } else if (node.uiSelf->mode == UiObject::CONDITION_MODE) {
            if (node.uiSelf->displayCondition && node.uiSelf->displayCondition()) { // 达到条件进行显示
                initPage(node);
            } else if (node.uiSelf->hiddenCondition && node.uiSelf->hiddenCondition()) { // 达到条件进行隐藏
                deinitPage(node);
            }
            conditionRegister(node);    // 无论结果如何把条件函数注册对应事件
        }
    }
}

void UiManager::deletePage(UiManager::SubclassMeta &node) {
    if (node.uiSelf && node.lvSelf) {
        if (node.uiSelf->mode == UiObject::CONDITION_MODE) {
            conditionrUnregister(node);
            deinitPage(node);
        } else {
            deinitPage(node);
        }
    }
}

void UiManager::initPage(UiManager::SubclassMeta &node) {   // 单独构造一个lvgl页面同时发送事件
    if (node.uiSelf && node.lvSelf == nullptr) {
        if (node.parent) {   // 如果有父对象(没有父对象也不用赋值了，之前已经赋值过了)
            node.lvParent = node.parent->lvSelf;
        }
        if (node.lvParent == nullptr) { // 如果是空直接赋值为lvgl默认屏幕
            node.lvParent = lv_scr_act();
        }
        node.lvSelf = node.uiSelf->Init(node.lvParent); // 初始化函数, 在UiInit里面实现lvgl初始化代码, 并且一定要返回lvgl指针
        EventManager::GetInstance().PublishEvent<pageEvent>();
        // printf("initPage %s\n", node.selfName.c_str());
    }
}

void UiManager::deinitPage(UiManager::SubclassMeta &node) { // 单独析构一个lvgl页面同时发送事件
    if (node.uiSelf && node.lvSelf) {
        node.uiSelf->Deinit(node.lvSelf);
        node.lvSelf = nullptr;
        node.lvParent = nullptr;
        EventManager::GetInstance().PublishEvent<pageEvent>();
        // printf("deinitPage %s\n", node.selfName.c_str());
    }
}