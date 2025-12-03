#include <iostream>
#include <vector>
#include <chrono>
#include <atomic>
#include "ThreadPool.h"

// Task: Simulates work for 500ms
void heavy_task(int id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

int main() {
    // 1. Setup
    int cores = std::thread::hardware_concurrency();
    std::cout << "Scalable Thread Pool | Hardware Cores: " << cores << std::endl;
    std::cout << "------------------------------------------------" << std::endl;
    
    ThreadPool pool(cores);
    
    // 2. Submit many tasks (200 tasks)
    int total_tasks = 200;
    for(int i = 0; i < total_tasks; ++i) {
        pool.submit(heavy_task, i);
    }

    // 3. Monitoring Loop (The Dashboard)
    // While there are tasks in the queue, print status
    while(true) {
        size_t pending = pool.get_tasks_queued();
        
        std::cout << "\r[Status] Workers: " << pool.get_workers_count() 
                  << " | Pending Tasks: " << pending << "   " << std::flush;
        
        if(pending == 0) break;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::cout << "\n------------------------------------------------" << std::endl;
    std::cout << "All tasks processed." << std::endl;

    return 0;
}