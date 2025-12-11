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
    lv_style_set_bg_color(&style_bg, lv_color_make(255, 85, 99));
    // lv_style_set_bg_img_src(&style_bg, "/home/qyb/code/app/build/bin/ui/asset/image/welcome/welcome_bg.png");
    lv_style_set_bg_img_opa(&style_bg, 255);
    lv_style_set_pad_all(&style_bg, 0);
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
    cout<<"_self "<<_self<<endl;
    
    main_btn = lv_btn_create(_self);
    lv_obj_set_size(main_btn, 200, 50);
    lv_obj_align(main_btn, LV_ALIGN_BOTTOM_RIGHT, -40, -40);
    lv_obj_t *label = lv_label_create(main_btn);
    lv_label_set_text(label, "start");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    
    sys_btn = lv_btn_create(_self);
    lv_obj_set_size(sys_btn, 100, 100);
    lv_obj_align(sys_btn, LV_ALIGN_TOP_RIGHT, -40, 40);
    label = lv_label_create(sys_btn);
    lv_label_set_text(label, "system_page");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

/**
 * @brief 初始化逻辑
 * 
 */
void WelcomePage::logicInit() { 
    lv_obj_add_event_cb(main_btn, [](lv_event_t *e) {
        UiManager::GetInstance().navigationToPage("MainPage");
    }, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(sys_btn, [](lv_event_t *e) {
        UiManager::GetInstance().navigationToPage("SystemPage");
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
    lv_style_reset(&style_bg);
    lv_obj_del(obj);
    _parent = NULL;
    _self = NULL;
}