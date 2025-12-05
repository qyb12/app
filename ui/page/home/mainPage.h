#pragma once

#include "uiObject.h"

class MainPage : public UiObject {
public:
    MainPage(UiObject *parant);
    ~MainPage();
    void UiInit() override;
    void UiDeinit() override;
    
private:

};
