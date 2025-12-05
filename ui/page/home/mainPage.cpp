#include "mainPage.h"
#include <iostream>

using namespace std;

REGISTER_PAGE(MainPage, "");

MainPage::MainPage(UiObject *parant) : UiObject() {
    cout << "构造函数" << endl;
}

MainPage::~MainPage() {
    cout << "构造函数" << endl;
}

void MainPage::UiInit() {
    cout<<"MainPage UiInit"<<endl;
}

void MainPage::UiDeinit() {
    cout<<"MainPage UiDeinit"<<endl;
}