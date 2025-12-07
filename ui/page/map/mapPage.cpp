#include "mapPage.h"
#include "ui_manager/uiManager.h"

#include <iostream>

using namespace std;

REGISTER_PAGE(MapPage, );

MapPage::MapPage(UiObject *parant) : UiObject() {
    mode = CONDITION_MODE;
    eventIds.push_back(typeid(pageEvent).hash_code());
    condition = []() -> bool {
        return UiManager::GetInstance().isVisible("MainPage");
    };
}

MapPage::~MapPage() {
    
}

void MapPage::styleInit() {
    lv_style_init(&style_bg);
    lv_style_set_bg_color(&style_bg, lv_color_make(152, 0, 200));
    lv_style_set_bg_img_opa(&style_bg, 255);
    lv_style_set_pad_all(&style_bg, 0);
}

/**
 * @brief 初始化UI
 * 
 */
void MapPage::uiInit() {
    int width = lv_obj_get_width(_parent);
    int height = lv_obj_get_height(_parent);

    _self = lv_obj_create(_parent);
    lv_obj_set_size(_self, LV_PCT(20), LV_PCT(20));
    lv_obj_add_style(_self, &style_bg, LV_STATE_DEFAULT);
    lv_obj_align(_self, LV_ALIGN_BOTTOM_LEFT, 50, -50);
}

/**
 * @brief 初始化逻辑
 * 
 */
void MapPage::logicInit() { 

}

lv_obj_t *MapPage::Init(lv_obj_t *parent) {
    _parent = parent;
    styleInit();
    uiInit();
    logicInit();
    return _self;
}

void MapPage::Deinit(lv_obj_t *obj) {
    lv_style_reset(&style_bg);
    lv_obj_del(obj);
    _parent = NULL;
    _self = NULL;
}
