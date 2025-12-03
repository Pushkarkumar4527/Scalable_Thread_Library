#ifndef SAFE_QUEUE_H
#define SAFE_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

// Module 1: Task Scheduler & Queue Management
template <typename T>
class SafeQueue {
private:
    std::queue<T> queue;        
    std::mutex mtx;             

public:
    SafeQueue() {}

    void push(T item) {
        std::unique_lock<std::mutex> lock(mtx);
        queue.push(item);
    }

    bool empty() {
        std::unique_lock<std::mutex> lock(mtx);
        return queue.empty();
    }
    
    // NEW FEATURE: Get current size for monitoring
    size_t size() {
        std::unique_lock<std::mutex> lock(mtx);
        return queue.size();
    }

    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mtx);
        if (queue.empty()) {
            return false;
        }
        item = queue.front();
        queue.pop();
        return true;
    }
};

#endif