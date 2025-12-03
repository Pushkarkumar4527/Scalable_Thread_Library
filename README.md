# Scalable Thread Management Library

**Course:** CSE 316 (Operating Systems)  
**Project:** Scalable Thread Management Library (High-Performance Thread Pool)  

## Project Overview
This project implements a high-performance **Thread Pool** in C++ designed to handle thousands of concurrent tasks efficiently. Instead of creating and destroying a thread for every task (which causes system overhead), this library maintains a fixed pool of worker threads that reuse resources, ensuring scalability on limited hardware.

## Modules Implemented
The project is divided into 3 core modules:
1.  **Module 1: Task Scheduler & Queue** - A thread-safe storage buffer that manages incoming tasks using Mutex locks.
2.  **Module 2: Worker Thread Engine** - A pool of persistent threads that continuously fetch and execute tasks.
3.  **Module 3: Monitoring & Control** - A synchronization layer (using Condition Variables) and a status dashboard to track active workers and pending tasks in real-time.

## Key Features
* **Scalability:** Automatically detects hardware cores and scales the thread pool size.
* **Thread Safety:** Uses `std::mutex` and `std::unique_lock` to prevent race conditions.
* **Non-Blocking Submission:** Users can submit tasks asynchronously using `std::future`.
* **Live Dashboard:** Displays real-time metrics (Pending Tasks, Active Workers) during execution.

## How to Compile and Run
**Requirements:** C++17 or later, Linux/MinGW (with POSIX threads).

1.  **Compile:**
    ```bash
    g++ -std=c++17 main.cpp -o main
    ```
2.  **Run:**
    ```bash
    ./main
    ```

## Project Status
* [x] Module 1 Completed (Queue)
* [x] Module 2 Completed (Worker Engine)
* [x] Module 3 Completed (Monitoring)
* [x] Final Integration Testing Completed

## Future Enhancements
* **Priority Scheduling:** Implementing a priority queue to handle critical tasks first.
* **Work Stealing:** Allowing idle threads to steal tasks from busy ones.

# Authors  
| NAME | REG. NO. |
|---|:---:|
| Pushkar Kumar | 12411187 | 
| Shivam Singh | 12419111 | 
| Prachi Sharma | 12419896 |