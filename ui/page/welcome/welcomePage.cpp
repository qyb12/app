#include "welcomePage.h"
#include "ui_manager/uiManager.h"

#include <iostream>

using namespace std;

REGISTER_PAGE(WelcomePage, );

WelcomePage::WelcomePage(UiObject *parant) : UiObject() {
    mode = UNIQUE_MODE;
}

WelcomePage::~WelcomePage() {
    
}

/**
 * @brief 初始化样式
 * 
 */
void WelcomePage::styleInit() { 
    lv_style_init(&style_bg);
    lv_style_set_bg_img_src(&style_bg, "/home/qyb/code/app/build/bin/ui/asset/image/welcome/welcome_bg.png");
    lv_style_set_bg_img_opa(&style_bg, 255);
}

/**
 * @brief 初始化样式
 * 
 */
void WelcomePage::uiInit() { 
    int width = lv_obj_get_width(_parent);
    int height = lv_obj_get_height(_parent);

    _self = lv_obj_create(_parent);
    lv_obj_set_size(_self, width, height);
    lv_obj_add_style(_self, &style_bg, LV_STATE_DEFAULT);
    
    btn = lv_btn_create(_self);
    lv_obj_set_size(btn, 200, 50);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "start");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

/**
 * @brief 初始化逻辑
 * 
 */
void WelcomePage::logicInit() { 
    lv_obj_add_event_cb(btn, [](lv_event_t *e) {
        UiManager::GetInstance().navigationToPage("MainPage");
    }, LV_EVENT_CLICKED, NULL);
}

lv_obj_t *WelcomePage::Init(lv_obj_t *parent) {
    _parent = parent;
    styleInit();
    uiInit();
    logicInit();
    return _self;
}

void WelcomePage::Deinit(lv_obj_t *obj) {
    cout<<"WelcomePage Deinit"<<endl;
    lv_style_reset(&style_bg);
    lv_obj_del(obj);
    _parent = NULL;
    _self = NULL;
}