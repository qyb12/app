#pragma once

#include "uiObject.h"

class SettingsPage : public UiObject {
public:
    SettingsPage(UiObject *parant);
    ~SettingsPage();
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
};
