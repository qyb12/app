#pragma once

#include "lvgl/lvgl.h"
#include "event.h"

#include <functional>

class UiObject {
public:
    virtual ~UiObject() = default;
    virtual lv_obj_t *Init(lv_obj_t *) = 0;
    virtual void Deinit(lv_obj_t *) = 0;

    using conditionFunc = std::function<bool ()>;

    enum EXISTENCE_MODE {
        PERMANENT_MODE, // 永远显示(隐藏就是析构，显示就是构造)
        UNIQUE_MODE,    // 与其他UNIQUE_MODE的控件互斥显示(隐藏就是析构，显示就是构造)
        CONDITION_MODE  // 到达一个条件之后进行显示和隐藏(隐藏就是析构，显示就是构造)
    };

    int mode = PERMANENT_MODE;  // 对象(用于管理lvgl的对象结点)的显示模式属性(隐藏就是析构，显示就是构造)

    std::vector<size_t> displayEventIds;    // 显示条件函数监听哪些事件(EventIds 存储事件类型的哈希值)
    conditionFunc displayCondition; // 显示条件函数，返回true就进行显示(隐藏就是析构，显示就是构造)

    std::vector<size_t> hiddenEventIds;     // 隐藏条件函数监听哪些事件(EventIds 存储事件类型的哈希值)
    conditionFunc hiddenCondition;  // 隐藏条件函数，返回true就进行隐藏(隐藏就是析构，显示就是构造)
    
};