#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <thread>
#include <functional>
#include <future>
#include <atomic>
#include "SafeQueue.h" // Includes Module 1

class ThreadPool {
private:
    // =========================================
    // MODULE 3: Synchronization & Control
    // =========================================
    std::mutex mtx;                      // Lock for the condition variable
    std::condition_variable cv;          // Signaling mechanism to wake threads
    std::atomic<bool> is_shutdown;       // Atomic flag to stop the pool safely

    // =========================================
    // MODULE 2: Worker Thread Engine
    // =========================================
    std::vector<std::thread> workers;            // The pool of threads
    SafeQueue<std::function<void()>> task_queue; // Queue holds "void" functions

    // The internal loop that every worker thread runs
    void worker_loop() {
        while (true) {
            std::function<void()> task;
            {
                // Wait for a task or shutdown signal
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this] { 
                    return !task_queue.empty() || is_shutdown; 
                });

                // Exit if shutdown is triggered and queue is empty
                if (is_shutdown && task_queue.empty()) {
                    return;
                }

                // If queue is not empty, grab the task
                // We don't need to lock here because SafeQueue handles its own locking!
                if (!task_queue.pop(task)) {
                    continue; 
                }
            }

            // Execute the task outside the lock (for performance)
            task();
        }
    }

public:
    // Constructor: Launches 'n' worker threads
    ThreadPool(size_t threads_count) : is_shutdown(false) {
        for (size_t i = 0; i < threads_count; ++i) {
            workers.emplace_back(&ThreadPool::worker_loop, this);
        }
    }

    // NEW FEATURE: Monitoring Interface (Module 3)
    size_t get_tasks_queued() {
        return task_queue.size();
    }

    size_t get_workers_count() {
        return workers.size();
    }

    // Destructor: Joins all threads
    ~ThreadPool() {
        shutdown();
    }

    // Function to submit tasks (Module 3 API)
    // This is a "template" so it can accept ANY function with ANY arguments
    template<class F, class... Args>
    auto submit(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type> {
        
        using return_type = typename std::invoke_result<F, Args...>::type;

        // Package the task so we can get a future result back later
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<return_type> res = task->get_future();

        // Push a simple void wrapper into the queue
        task_queue.push([task]() { (*task)(); });

        // Wake up one thread to handle this new task
        cv.notify_one();
        
        return res;
    }

    // Graceful shutdown
    void shutdown() {
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (is_shutdown) return; // Already stopped
            is_shutdown = true;
        }
        cv.notify_all(); // Wake everyone up so they can exit
        
        for (std::thread &worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
};

#endif