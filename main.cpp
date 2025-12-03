#include <iostream>
#include <vector>
#include <chrono>
#include <fstream> // Added for file handling
#include <thread>
#include "ThreadPool.h"

// --- ANSI COLOR CODES ---
const std::string RESET = "\033[0m";
const std::string GREEN = "\033[32m";
const std::string CYAN = "\033[36m";
const std::string BOLD = "\033[1m";

void heavy_task(int id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

int main() {
    // 1. Setup Logging
    std::ofstream logFile("performance_log.csv");
    logFile << "Time_ms,Pending_Tasks,Active_Workers\n"; // CSV Header

    int cores = std::thread::hardware_concurrency();
    ThreadPool pool(cores);
    int total_tasks = 500;
    
    for(int i = 0; i < total_tasks; ++i) {
        pool.submit(heavy_task, i);
    }

    // 2. Monitoring Loop
    auto start_time = std::chrono::high_resolution_clock::now();
    
    while(true) {
        size_t pending = pool.get_tasks_queued();
        size_t workers = pool.get_workers_count();
        
        // Calculate elapsed time
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();

        // Write to File
        logFile << elapsed << "," << pending << "," << workers << "\n";

        // Visual Dashboard
        std::cout << "\033[H\033[J"; // Clear screen
        std::cout << BOLD << CYAN << "=== THREAD POOL MONITOR ===" << RESET << std::endl;
        std::cout << "Time: " << elapsed << "ms | Pending: " << pending << std::endl;
        
        if(pending == 0) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    logFile.close();
    std::cout << "\n" << GREEN << "Done! Log saved to 'performance_log.csv'" << RESET << std::endl;
    return 0;
}