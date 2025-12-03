#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <iomanip>
#include "ThreadPool.h"

// --- ANSI COLOR CODES FOR VISUALS ---
const std::string RESET = "\033[0m";
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string CYAN = "\033[36m";
const std::string BOLD = "\033[1m";

// Helper to draw a progress bar
void draw_progress_bar(int total, int pending, int active_workers) {
    int completed = total - pending;
    float percentage = (float)completed / total;
    int bar_width = 40;
    int filled_width = (int)(bar_width * percentage);

    // Calculate clear screen codes to keep dashboard static
    std::cout << "\033[H\033[J"; // ANSI code to clear screen and move cursor to top

    std::cout << BOLD << CYAN << "==========================================" << RESET << std::endl;
    std::cout << BOLD << "   SCALABLE THREAD POOL MONITORING SYSTEM   " << RESET << std::endl;
    std::cout << BOLD << CYAN << "==========================================" << RESET << std::endl << std::endl;

    std::cout << "Hardware Cores: " << std::thread::hardware_concurrency() << std::endl;
    std::cout << "Active Workers: " << ((pending > 0) ? GREEN : YELLOW) << active_workers << RESET << std::endl;
    std::cout << "Total Tasks   : " << total << std::endl;
    std::cout << "Tasks Pending : " << ((pending > 50) ? RED : RESET) << pending << RESET << std::endl;
    
    std::cout << std::endl << "Progress: " << (int)(percentage * 100) << "%" << std::endl;
    
    // Draw the bar
    std::cout << "[";
    for (int i = 0; i < bar_width; ++i) {
        if (i < filled_width) std::cout << GREEN << "#" << RESET;
        else std::cout << ".";
    }
    std::cout << "]" << std::endl;
}

// Simulates a heavy task
void heavy_task(int id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

int main() {
    int cores = std::thread::hardware_concurrency();
    ThreadPool pool(cores);
    
    int total_tasks = 500;
    
    // Submit tasks
    for(int i = 0; i < total_tasks; ++i) {
        pool.submit(heavy_task, i);
    }

    // Monitoring Loop (Refreshes every 100ms)
    while(true) {
        size_t pending = pool.get_tasks_queued();
        size_t workers = pool.get_workers_count();
        
        draw_progress_bar(total_tasks, pending, workers);

        if(pending == 0) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << std::endl << BOLD << GREEN << "✔ ALL TASKS COMPLETED SUCCESSFULLY." << RESET << std::endl;
    return 0;
}