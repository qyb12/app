#pragma once

#include "uiObject.h"
#include "event_manager/eventManager.h"

#include <string>
#include <functional>
#include <unordered_map>
#include <vector>

class UiManager {
public:
    enum PageEventId {
        DISPLAYED,
        HIDDEN,
    };
    /*
     * @brief 子对象创建函数类型：接收父对象实例指针，返回子对象实例指针
     */
    using CreateFunc = std::function<UiObject*(UiObject*)>; 

    /*
     * @brief 注册表项
     */
    struct SubclassMeta {    // 注册表项
        std::string selfName;   // 子类名
        UiObject *uiSelf; // 子类实例指针
        lv_obj_t *lvSelf; // 子类 lvgl 实例指针
        std::string parentName;  // 父控件名字
        UiObject *uiParent; // 父控件实例指针
        lv_obj_t *lvParent; // 父控件 lvgl 实例指针
        UiManager::CreateFunc createFunc;   // 带父对象指针的创建函数
        struct SubclassMeta *parent; // 下一个注册表项指针
        std::vector<struct SubclassMeta *> children; // 下一个注册表项指针
    };

    /* 
     * @brief 获取单例实例引用
     * @return UiManager& 单例实例引用
     */
    static UiManager& GetInstance();

    /* 
     * @brief 注册子类
     * @param selfName 子类名
     * @param parentName 父类名
     * @param createFunc 创建函数指针，用于创建子类实例
     * @return 无返回值
     */
    void pageRegister(const std::string &, const std::string &, UiManager::CreateFunc);

    /* 
     * @brief 批量创建所有子类实例
     * @return 无返回值
     */
    void createAllPage();

    /* 
     * @brief 批量创建所有lvgl控件实例
     */
    void initPage();

    /**
     * @brief 导航到指定页面
     * @param pageName 页面名
     * @return 无返回值
     */
    void navigationToPage(const std::string &);

    /**
     * @brief 判断指定页面是否可见
     * @param pageName 页面名
     * @return bool 若页面可见则返回true，否则返回false
     */
    bool isVisible(const std::string &);

    /**
     * @brief 判断指定页面是否被按下
     * @param pageName 页面名
     * @return bool 若页面被按下则返回true，否则返回false
     */
    bool isPressed(const std::string &);

private:
    /* 
     * @brief 开启启动时候页面创建顺序和要启动的页面是哪个(里面的成员就是页面的类名)
     */
    std::string firstPage = "WelcomePage";
    std::vector<struct SubclassMeta *> pages;
    std::unordered_map<std::string, bool> pressedPagesMap;
    std::vector<struct SubclassMeta *> pressedPagesVector;

private:
    /**
     * @brief 初始化指定页面的所有实例
     * @param pageName 页面名
     * @return 无返回值
     */
    void initPage(UiManager::SubclassMeta &);
    /**
     * @brief 关闭指定页面的所有实例
     * @param pageName 页面名
     * @return 无返回值
     */
    void deinitPage(UiManager::SubclassMeta &);
    /**
     * @brief 创建指定页面的所有实例
     * @param pageName 页面名
     * @return 无返回值
     */
    void createPage(UiManager::SubclassMeta &);
    /**
     * @brief 删除指定页面的所有实例
     * @param pageName 页面名
     * @return 无返回值
     */
    void deletePage(UiManager::SubclassMeta &);
    /**
     * @brief 把条件结点根据需要监听哪些事件全部监听
     * @param node 要进行监听事件的条件结点
     */
    void conditionRegister(UiManager::SubclassMeta &);
     /**
     * @brief 把条件结点根据需要监听哪些事件全部注销
     * @param node 要进行注销事件的条件结点
     * @return 无返回值
     */
    void conditionrUnregister(UiManager::SubclassMeta &);
    /**
     * @brief 遍历对象森林，对每个对象应用指定函数(先遍历根)
     * @param node 起始对象
     * @param func 要应用的函数，接收 UiManager::SubclassMeta& 类型参数
     */
    void foreachTree(UiManager::SubclassMeta &, std::function<void(UiManager::SubclassMeta&)>);
    /**
     * @brief 遍历对象森林，对每个对象应用指定函数(后遍历根)
     * @param node 起始对象
     * @param func 要应用的函数，接收 UiManager::SubclassMeta& 类型参数
     */
    void foreachTreeBack(UiManager::SubclassMeta &, std::function<void(UiManager::SubclassMeta&)>);
    /**
     * @brief 遍历对象森林，对每个对象应用指定函数(先遍历根，后遍历子树)
     * @param node 起始对象
     * @param func 要应用的函数，接收 UiManager::SubclassMeta& 类型参数
     * @return 无返回值
     */
    void foreachTreeOrder(UiManager::SubclassMeta &, std::function<void(UiManager::SubclassMeta&)>);
    void foreachTree1(UiManager::SubclassMeta &node, std::function<void(UiManager::SubclassMeta&)>);
    static void mouse_pressed_event_cb(struct _lv_indev_drv_t *, uint8_t state);
    static bool area_is_point_in(const lv_area_t &area, int32_t &x, int32_t &y);
    /**
     * @brief 判断指定点是否在指定区域内
     * @param area 目标区域
     * @param x 点的x坐标
     * @param y 点的y坐标
     * @return true=在区域内，false=不在
     */
    static bool area_is_point_in(const lv_area_t &area, lv_coord_t &x, lv_coord_t &y);
    static lv_indev_t *mouse_indev;
    /**
     * @brief 从对象森林中查找指定名称的对象
     * @param node 起始对象
     * @param name 要查找的对象名称
     * @return UiManager::SubclassMeta* 找到的对象指针，若未找到则为nullptr
     */
    UiManager::SubclassMeta *findFromForest(UiManager::SubclassMeta &, const std::string &);
    /* 
     * @brief 获取静态注册表引用
     * @return std::unordered_map<std::string, UiManager::SubclassMeta>& 静态注册表引用
     */
    std::unordered_map<std::string, UiManager::SubclassMeta>& getTable();

public:
    /*
     * @brief 禁用拷贝和移动（确保单例唯一性）
     */
    UiManager(const UiManager&) = delete;
    /*
     * @brief 禁用拷贝赋值运算符（确保单例唯一性）
     */
    UiManager& operator=(const UiManager&) = delete;
    /*
     * @brief 禁用移动构造函数（确保单例唯一性）
     */
    UiManager(UiManager&&) = delete;
    /*
     * @brief 禁用移动赋值运算符（确保单例唯一性）
     */
    UiManager& operator=(UiManager&&) = delete;

private:
    /* 
     * @brief 构造函数（私有，确保单例唯一性）
     */
    UiManager() = default;
    /* 
     * @brief 析构函数（私有，确保单例唯一性）
     */
    ~UiManager() = default;
};


/*
 * @brief 注册子类宏
 * @param CLASS 子类名
 * @param PARENT 父类名
*/
#define REGISTER_PAGE(CLASS, PARENT) \
class CLASS##Register { \
public: \
    CLASS##Register() { \
        UiManager::GetInstance().pageRegister(#CLASS, #PARENT, [](UiObject* parent) -> UiObject* { \
            return new CLASS(parent);   \
        }); \
    } \
}; \
static CLASS##Register CLASS##_reg;
