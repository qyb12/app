#include "eventManager.h"

// 单例模式（可选，也可直接实例化）
EventManager& EventManager::GetInstance() {
    static EventManager instance;
    return instance;
}

void EventManager::runTasks() {
    if (asyncTasks_.size() == 0) {
        return;
    }
    static int i = 0;
    printf("runTasks asyncTasks_ = %d, i = %d\n", asyncTasks_.size(), i++);
    auto tasks = asyncTasks_;
    asyncTasks_.clear();
    for (const auto& task : tasks) {
        task.callback(task.eventObj);
    }
}