#include "welcomePage.h"
#include <iostream>

using namespace std;

REGISTER_PAGE(WelcomePage, "");

WelcomePage::WelcomePage(UiObject *parant) : UiObject() {
    cout << "构造函数" << endl;
}

WelcomePage::~WelcomePage() {
    cout << "构造函数" << endl;
}

void WelcomePage::UiInit() {
    cout<<"WelcomePage UiInit"<<endl;
}

void WelcomePage::UiDeinit() {
    cout<<"WelcomePage UiDeinit"<<endl;
}