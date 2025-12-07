#include "settingsPage.h"
#include "ui_manager/uiManager.h"

#include <iostream>

using namespace std;

REGISTER_PAGE(SettingsPage, );

SettingsPage::SettingsPage(UiObject *parant) : UiObject() {
    mode = CONDITION_MODE;
}

SettingsPage::~SettingsPage() {
    
}

void SettingsPage::styleInit() { 
    lv_style_init(&style_bg);
    lv_style_set_bg_color(&style_bg, lv_color_make(55, 44, 1));
    lv_style_set_bg_img_opa(&style_bg, 255);
    lv_style_set_pad_all(&style_bg, 0);
}

/**
 * @brief 初始化UI
 * 
 */
void SettingsPage::uiInit() { 
    _self = lv_obj_create(_parent);
    lv_obj_set_size(_self, LV_PCT(50), LV_PCT(100));
    lv_obj_add_style(_self, &style_bg, LV_STATE_DEFAULT);
    lv_obj_align(_self, LV_ALIGN_TOP_MID, 0, 0);
}

/**
 * @brief 初始化逻辑
 * 
 */
void SettingsPage::logicInit() { 

}

lv_obj_t *SettingsPage::Init(lv_obj_t *parent) {
    _parent = parent;
    styleInit();
    uiInit();
    logicInit();
    return _self;
}

void SettingsPage::Deinit(lv_obj_t *obj) {
    lv_style_reset(&style_bg);
    lv_obj_del(obj);
    _parent = NULL;
    _self = NULL;
}
