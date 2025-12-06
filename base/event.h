#pragma once

#include <iostream>
#include <tuple>       // std::tuple、std::make_tuple
#include <utility>     // std::forward（完美转发参数）

// 事件基类（用于统一管理不同类型的事件）
struct BaseEvent {};

// 具体事件类型（示例：带参数的事件）
template <typename... Args>
struct Event : public BaseEvent {
    Event(Args... args) : params(std::make_tuple(std::forward<Args>(args)...)) {}
    std::tuple<Args...> params; // 存储事件参数
};