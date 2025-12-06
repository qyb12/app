#include "eventManager.h"

// 单例模式（可选，也可直接实例化）
EventManager& EventManager::GetInstance() {
    static EventManager instance;
    return instance;
}