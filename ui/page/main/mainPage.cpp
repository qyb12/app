#include "mainPage.h"
#include "ui_manager/uiManager.h"

#include <iostream>

using namespace std;

REGISTER_PAGE(MainPage, );

MainPage::MainPage(UiObject *parant) : UiObject() {
    mode = UNIQUE_MODE;
}

MainPage::~MainPage() {
    
}

void MainPage::styleInit() {
    lv_style_init(&style_bg);
    lv_style_set_bg_color(&style_bg, lv_color_make(23, 85, 99));
    // lv_style_set_bg_img_src(&style_bg, "/home/qyb/code/app/build/bin/ui/asset/image/main/main_bg.png");
    lv_style_set_bg_img_opa(&style_bg, 255);
    lv_style_set_pad_all(&style_bg, 0);
}

/**
 * @brief 初始化UI
 * 
 */
void MainPage::uiInit() {
    int width = lv_obj_get_width(_parent);
    int height = lv_obj_get_height(_parent);

    _self = lv_obj_create(_parent);
    lv_obj_set_size(_self, width, height);
    lv_obj_add_style(_self, &style_bg, LV_STATE_DEFAULT);
}

/**
 * @brief 初始化逻辑
 * 
 */
void MainPage::logicInit() { 

}

lv_obj_t *MainPage::Init(lv_obj_t *parent) {
    _parent = parent;
    styleInit();
    uiInit();
    logicInit();
    return _self;
}

void MainPage::Deinit(lv_obj_t *obj) {
    lv_style_reset(&style_bg);
    lv_obj_del(obj);
    _parent = NULL;
    _self = NULL;
}
