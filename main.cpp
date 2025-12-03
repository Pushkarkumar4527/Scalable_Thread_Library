#include <iostream>
#include <vector>
#include <chrono>
#include "ThreadPool.h"

// A dummy task that simulates hard work
int complex_calculation(int a, int b) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulate delay
    return a * b;
}

int main() {
    // 1. Initialize Pool with 4 Workers (Scalable to Hardware)
    // You can use std::thread::hardware_concurrency() to detect your CPU cores
    int cores = std::thread::hardware_concurrency();
    std::cout << "Detected " << cores << " cores. Creating Thread Pool..." << std::endl;
    
    ThreadPool pool(cores);

    // 2. Submit 100 tasks (Simulating High Concurrency)
    std::vector<std::future<int>> results;
    
    std::cout << "Submitting 100 tasks..." << std::endl;
    for(int i = 0; i < 100; ++i) {
        results.emplace_back(
            pool.submit(complex_calculation, i, i)
        );
    }

    // 3. Get results
    for(auto && result : results) {
        // .get() waits for the thread to finish and returns the value
        // This proves the "Future" mechanism works
        // std::cout << "Result: " << result.get() << std::endl; 
        result.get(); // Just waiting for completion
    }

    std::cout << "All 100 tasks completed successfully by the pool!" << std::endl;

    return 0;
}