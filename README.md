# Scalable Thread Management Library (High-Performance Thread Pool)

**Course:** CSE 316 (Operating Systems)  
**Project:** Academic Task-2 (Thread Management Library)  

## 📌 Project Overview
This project implements a high-performance **Thread Pool** in C++ designed to handle thousands of concurrent tasks efficiently. Instead of creating and destroying a thread for every task (which causes system overhead), this library maintains a fixed pool of worker threads that reuse resources, ensuring scalability on limited hardware.

Uniquely, this project includes a **Real-Time Web Dashboard** hosted on an embedded HTTP server, allowing users to visualize the thread pool's performance (Active Workers, Queue Depth) via a modern web browser.

## 🚀 Key Features
* **Scalability:** Automatically detects hardware cores and scales the thread pool size.
* **Thread Safety:** Uses `std::mutex` and `std::unique_lock` to prevent race conditions.
* **Web-Based Dashboard:** A live GUI hosted on `localhost:8080` featuring:
    * Gradient Area Charts (using Chart.js).
    * Real-time counters for Active Workers & Pending Tasks.
    * "System Online" status pulse animation.
* **JSON API:** Exposes internal system metrics via a `/stats` endpoint.

## 🛠️ Modules Implemented
The project is architected into 3 core modules:
1.  **Module 1: Task Scheduler & Queue** - A thread-safe storage buffer that manages incoming tasks using Mutex locks.
2.  **Module 2: Worker Thread Engine** - A pool of persistent threads that continuously fetch and execute tasks.
3.  **Module 3: Monitoring & Control** - An embedded HTTP server (`httplib`) that serves a live HTML5 dashboard to monitor system health.

## 💻 Tech Stack
* **Language:** C++17
* **Concurrency:** `<thread>`, `<mutex>`, `<condition_variable>`, `<atomic>`
* **Web Server:** `httplib.h` (Single-header C++ HTTP Library)
* **Frontend:** HTML5, CSS3 (Dark Mode), JavaScript (Chart.js)

## ⚙️ How to Compile and Run
**Requirements:** Linux environment (or GitHub Codespaces) with G++ compiler.

1.  **Compile the code:**
    ```bash
    g++ -std=c++17 main.cpp -o main
    ```
2.  **Run the application:**
    ```bash
    ./main
    ```
3.  **Open the Dashboard:**
    * The terminal will show: `Server listening on http://localhost:8080`
    * Open your web browser and go to: `http://localhost:8080`
    * Watch the live graph as tasks are processed!

## 📊 Project Status
* [x] Module 1 Completed (Thread-Safe Queue)
* [x] Module 2 Completed (Worker Engine)
* [x] Module 3 Completed (Web GUI & JSON API)
* [x] Final Documentation Updated

# Authors  
| NAME | REG. NO. |
|---|:---:|
| Pushkar Kumar | 12411187 | 
| Shivam Singh | 12419111 | 
| Prachi Sharma | 12419896 |