#pragma once

#include "event.h"

#include <iostream>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>
#include <mutex>

// --------------- 核心：复合键 + 自定义哈希函数 ---------------
// 复合键：类型哈希 + 对象地址
using CallbackKey = std::pair<size_t, void*>;

// 封装待执行的事件+回调（用于异步队列）
struct AsyncEvent {
    std::shared_ptr<BaseEvent> eventObj; // 事件对象
    std::function<void(const std::shared_ptr<BaseEvent>&)> callback; // 待执行的回调
};
// 为CallbackKey实现哈希函数（注入std命名空间，让unordered_map识别）
namespace std {
    template<>
    struct hash<CallbackKey> {
        size_t operator()(const CallbackKey& key) const noexcept {
            // 组合哈希：将两个值混合成一个唯一哈希（避免简单拼接的冲突）
            size_t h1 = hash<size_t>()(key.first);  // 类型哈希的哈希
            size_t h2 = hash<void*>()(key.second); // 对象地址的哈希
            // 经典的哈希组合算法：避免h1和h2的顺序/值导致冲突
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };
}
// 事件管理器（核心类）
class EventManager {
public:
    // 单例模式（可选，也可直接实例化）
    static EventManager &GetInstance();

    void runTasks();

    // 注册监听器：事件类型ID + 回调函数
    template <typename EventType, typename ObjType, typename Func>
    void RegisterListener(ObjType &obj, Func&& func) {
        static int i = 0;
        // printf("RegisterListener i = %d\n", i++);
        std::lock_guard<std::mutex> lock(mtx_);
        // 生成复合键：类型哈希 + 对象地址
        CallbackKey key = {
            typeid(EventType).hash_code(),  // 事件类型ID
            static_cast<void*>(&obj)        // 对象地址（转为void*统一类型）
        };
        callbacks_[key].push_back([func = std::forward<Func>(func)](const std::shared_ptr<BaseEvent>& event) {
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
    template <typename EventType, typename ObjType>
    void UnregisterAllListeners(ObjType &obj) {
        std::lock_guard<std::mutex> lock(mtx_);
        CallbackKey key = {
            typeid(EventType).hash_code(), 
            static_cast<void*>(&obj)
        };
        callbacks_.erase(key);
    }

    // 发布事件：遍历所有匹配「事件类型」的复合键，执行回调
    template <typename EventType, typename... Args>
    void PublishEvent(Args&&... args) {
        std::lock_guard<std::mutex> lock(mtx_);
        size_t targetTypeId = typeid(EventType).hash_code();
        auto event = std::make_shared<EventType>(std::forward<Args>(args)...);

        // 遍历所有复合键，找到类型匹配的回调
        for (auto& [key, callbacks] : callbacks_) {
            if (key.first == targetTypeId) { // 类型哈希匹配
                for (const auto& callback : callbacks) {
                    asyncTasks_.push_back({event, callback});
                }
            }
        }
    }

private:
    EventManager() = default;
    ~EventManager() = default;
    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;

    // 事件类型ID -> 回调函数列表（回调接收BaseEvent指针）
    std::unordered_map<CallbackKey, std::vector<std::function<void(const std::shared_ptr<BaseEvent>&)>>> callbacks_;
    std::vector<AsyncEvent> asyncTasks_;
    std::mutex mtx_; // 线程安全锁
};