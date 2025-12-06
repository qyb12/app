#pragma once

#include "lvgl/lvgl.h"

#include <functional>

class UiObject {
public:
    virtual ~UiObject() = default;
    virtual lv_obj_t *Init(lv_obj_t *) = 0;
    virtual void Deinit(lv_obj_t *) = 0;

    enum EXISTENCE_MODE {
        PERMANENT_MODE, // 永远显示
        UNIQUE_MODE,    // 与其他UNIQUE_MODE的控件互斥显示
        CONDITION_MODE  // 到达一个条件之后进行显示和隐藏
    };
    int mode = PERMANENT_MODE;

    using conditionFunc = std::function<bool (const std::string &, int)>; 
    conditionFunc condition;
};