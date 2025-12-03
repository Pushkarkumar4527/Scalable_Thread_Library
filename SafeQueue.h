#ifndef SAFE_QUEUE_H
#define SAFE_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

// Module 1: Task Scheduler & Queue Management
// This class implements a thread-safe queue using a standard queue and mutex locks.

template <typename T>
class SafeQueue {
private:
    std::queue<T> queue;        // Standard queue to hold items
    std::mutex mtx;             // Mutex for thread synchronization

public:
    SafeQueue() {}

    // Function to push an item into the queue safely
    void push(T item) {
        std::unique_lock<std::mutex> lock(mtx);
        queue.push(item);
    }

    // Function to checking if the queue is empty
    bool empty() {
        std::unique_lock<std::mutex> lock(mtx);
        return queue.empty();
    }
    
    // Function to pop an item from the queue safely
    // Returns true if successful, false if queue is empty
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