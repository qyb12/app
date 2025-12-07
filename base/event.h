#pragma once

#include <iostream>
#include <tuple>       // std::tuple、std::make_tuple
#include <utility>     // std::forward（完美转发参数）

// 事件基类（用于统一管理不同类型的事件）
struct BaseEvent {
    virtual ~BaseEvent() = default; // 虚析构函数，让BaseEvent支持RTTI
};

// 具体事件类型（示例：带参数的事件）
template <typename... Args>
struct Event : public BaseEvent {
    Event(Args... args) : params(std::make_tuple(std::forward<Args>(args)...)) {}
    std::tuple<Args...> params; // 存储事件参数
};

/**
 * @brief 页面管理的事件
 * 
 */
struct pageEvent : public Event<> {
    using Event::Event; // 继承构造函数（C++11及以上）
};