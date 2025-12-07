#include "eventManager.h"

// 单例模式（可选，也可直接实例化）
EventManager& EventManager::GetInstance() {
    static EventManager instance;
    return instance;
}

void EventManager::runTasks() {
    auto tasks = asyncTasks_;
    asyncTasks_.clear();
    for (const auto& task : tasks) {
        task.callback(task.eventObj);
    }
}