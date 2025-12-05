#pragma once

#include "uiObject.h"

class WelcomePage : public UiObject {
public:
    WelcomePage(UiObject *parant);
    ~WelcomePage();
    void UiInit() override;
    void UiDeinit() override;
private:

};
