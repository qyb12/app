#pragma once

#include "uiObject.h"

class WelcomePage : public UiObject {
public:
    WelcomePage(UiObject *parant);
    ~WelcomePage();
    lv_obj_t *Init(lv_obj_t *) override;
    void Deinit(lv_obj_t *) override;

private:
    /**
     * @brief 初始化样式
     * 
     */
    void styleInit();
    /**
     * @brief 初始化UI
     * 
     */
    void uiInit();
    /**
     * @brief 初始化逻辑
     * 
     */
    void logicInit();
    
private:
    lv_style_t style_bg;
    lv_obj_t *_self = nullptr;
    lv_obj_t *_parent = nullptr;
    // 按钮
    lv_obj_t *main_btn = nullptr;
    lv_obj_t *sys_btn = nullptr;
};
