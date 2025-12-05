#pragma once

class UiObject {
public:
    // 子类创建函数类型：接收父类实例指针，返回子类实例指针
    using CreateFunc = std::function<UiObject*(UiObject*)>; 
    struct SubclassMeta {    // 注册表项：存储父类名 + 创建函数
        std::string parentName;  // 子类对应的父类名
        UiObject::CreateFunc createFunc;   // 带父类指针的创建函数
    };
    // 虚析构：确保子类析构时能正确调用
    virtual ~UiObject() = default;
    // 纯虚函数：确保Base是抽象类，无法直接实例化
    virtual void UiInit() = 0;
    virtual void UiDeinit() = 0;
    // 静态注册表：子类名 → 子类元信息（父类名+创建函数）
    static std::unordered_map<std::string, UiObject::SubclassMeta>& getTable();
    // 静态接口：子类注册接口（供子类调用）
    static void pageRegister(const std::string& className, const std::string& parentName, UiObject::CreateFunc func);
    // 静态接口：批量创建所有子类实例
    static std::vector<UiObject *> createAllPage();
    // 静态接口：获取所有已注册的子类名
    static std::vector<std::string> getAllSubclassName();
};

// 第二步：扩展注册宏，支持指定父类
#define REGISTER_PAGE(CLASS, PARENT) \
class CLASS##Register { \
public: \
    CLASS##Register() { \
        UiObject::pageRegister(#CLASS, #PARENT, [](UiObject* parent) -> UiObject* { \
            return new CLASS(parent);   \
        }); \
    } \
}; \
static CLASS##Register CLASS##_reg;