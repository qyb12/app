#pragma once

#include "event.h"

#include <iostream>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>
#include <mutex>

// 事件管理器（核心类）
class EventManager {
public:
    // 单例模式（可选，也可直接实例化）
    static EventManager &GetInstance();

    // 注册监听器：事件类型ID + 回调函数
    template <typename EventType, typename Func>
    void RegisterListener(Func&& func) {
        std::lock_guard<std::mutex> lock(mtx_);
        auto typeId = typeid(EventType).hash_code();
        callbacks_[typeId].push_back([func = std::forward<Func>(func)](const std::shared_ptr<BaseEvent>& event) {
                // 类型安全转换事件
                auto concreteEvent = std::dynamic_pointer_cast<EventType>(event);
                if (concreteEvent) {
                    // 解包tuple并调用回调
                    std::apply(func, concreteEvent->params);
                }
            }
        );
    }

    // 注销所有某类型事件的监听器（也可扩展为注销单个）
    template <typename EventType>
    void UnregisterAllListeners() {
        std::lock_guard<std::mutex> lock(mtx_);
        auto typeId = typeid(EventType).hash_code();
        callbacks_.erase(typeId);
    }

    // 发布事件：创建事件实例并触发所有监听器
    template <typename EventType, typename... Args>
    void PublishEvent(Args&&... args) {
        std::lock_guard<std::mutex> lock(mtx_);
        auto typeId = typeid(EventType).hash_code();
        auto it = callbacks_.find(typeId);
        if (it == callbacks_.end()) {
            return;
        }

        // 创建事件对象
        auto event = std::make_shared<EventType>(std::forward<Args>(args)...);

        // 遍历执行所有回调
        for (const auto& callback : it->second) {
            callback(event);
        }
    }

private:
    EventManager() = default;
    ~EventManager() = default;
    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;

    // 事件类型ID -> 回调函数列表（回调接收BaseEvent指针）
    std::unordered_map<size_t, std::vector<std::function<void(const std::shared_ptr<BaseEvent>&)>>> callbacks_;
    std::mutex mtx_; // 线程安全锁
};