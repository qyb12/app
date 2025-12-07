#include "statusBar.h"
#include "ui_manager/uiManager.h"

#include <iostream>

using namespace std;

REGISTER_PAGE(StatusBar, );

StatusBar::StatusBar(UiObject *parant) : UiObject() {
    mode = CONDITION_MODE;
    eventIds.push_back(typeid(pageEvent).hash_code());
    condition = []() -> bool {
        return UiManager::GetInstance().isVisible("MainPage");
    };
}

StatusBar::~StatusBar() {
    
}

void StatusBar::styleInit() { 
    lv_style_init(&style_bg);
    lv_style_set_bg_color(&style_bg, lv_color_white());
    lv_style_set_bg_img_opa(&style_bg, 255);
    lv_style_set_pad_all(&style_bg, 0);
}

/**
 * @brief 初始化UI
 * 
 */
void StatusBar::uiInit() { 
    _self = lv_obj_create(_parent);
    lv_obj_set_size(_self, LV_PCT(100), LV_PCT(10));
    lv_obj_add_style(_self, &style_bg, LV_STATE_DEFAULT);
    lv_obj_align(_self, LV_ALIGN_TOP_MID, 0, 0);

    btn = lv_btn_create(_self);
    lv_obj_set_size(btn, LV_PCT(10), LV_PCT(100));
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "back");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    
    label = lv_label_create(_self);
    lv_label_set_text(label, "status_start_123456789_stop");
    lv_obj_align_to(label, btn, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    for (int i=0; i<5; i++) {
        lv_obj_t *temp_btn = lv_btn_create(_self);
        lv_obj_set_size(temp_btn, LV_PCT(10), LV_PCT(100));
        lv_obj_align_to(temp_btn, label, LV_ALIGN_OUT_RIGHT_MID, i*150+10, 0);
    }

    set_btn = lv_btn_create(_self);
    lv_obj_set_size(set_btn, LV_PCT(10), LV_PCT(100));
    lv_obj_align(set_btn, LV_ALIGN_RIGHT_MID, 0, 0);
    label = lv_label_create(set_btn);
    lv_label_set_text(label, "settings");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

/**
 * @brief 初始化逻辑
 * 
 */
void StatusBar::logicInit() { 
    lv_obj_add_event_cb(btn, [](lv_event_t *e) {
        UiManager::GetInstance().navigationToPage("WelcomePage");
    }, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(set_btn, [](lv_event_t *e) {
        UiManager::GetInstance().navigationToPage("SettingsPage");
    }, LV_EVENT_CLICKED, NULL);
}

lv_obj_t *StatusBar::Init(lv_obj_t *parent) {
    _parent = parent;
    styleInit();
    uiInit();
    logicInit();
    return _self;
}

void StatusBar::Deinit(lv_obj_t *obj) {
    lv_style_reset(&style_bg);
    lv_obj_del(obj);
    _parent = NULL;
    _self = NULL;
}
