#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include "ThreadPool.h"
#include "httplib.h" // The new library

// Global stats for the web server to read
std::atomic<int> g_pending{0};
std::atomic<int> g_workers{0};
std::atomic<int> g_total{0};

void heavy_task(int id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

int main() {
    // 1. Setup Thread Pool
    int cores = std::thread::hardware_concurrency();
    ThreadPool pool(cores);
    int total_tasks = 1000;
    g_total = total_tasks;

    // 2. Start the Web Server in a separate thread
    std::thread server_thread([]() {
        httplib::Server svr;

        // Serve the Dashboard HTML
        svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
            std::string html = R"(
                <!DOCTYPE html>
                <html>
                <head>
                    <title>Thread Pool Monitor</title>
                    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
                    <style>
                        body { font-family: sans-serif; background: #1a1a1a; color: white; text-align: center; }
                        canvas { max-width: 95%; height: 600px; margin: 20px auto; }
                        .stat-box { display: inline-block; padding: 20px; background: #333; margin: 10px; border-radius: 10px; }
                        h1 { color: #4CAF50; }
                    </style>
                </head>
                <body>
                    <h1> Scalable Thread Pool Dashboard</h1>
                    <div class="stat-box">
                        <h3>Active Workers</h3>
                        <h2 id="workers" style="color: #FFC107">0</h2>
                    </div>
                    <div class="stat-box">
                        <h3>Pending Tasks</h3>
                        <h2 id="pending" style="color: #2196F3">0</h2>
                    </div>
                    <canvas id="myChart"></canvas>

                    <script>
                        const ctx = document.getElementById('myChart').getContext('2d');
                        const chart = new Chart(ctx, {
                            type: 'line',
                            data: {
                                labels: [],
                                datasets: [{
                                    label: 'Pending Tasks',
                                    borderColor: '#2196F3',
                                    data: [],
                                    fill: false
                                }]
                            },
                            options: { scales: { y: { beginAtZero: true } } }
                        });

                        setInterval(() => {
                            fetch('/stats').then(r => r.json()).then(data => {
                                document.getElementById('workers').innerText = data.workers;
                                document.getElementById('pending').innerText = data.pending;
                                
                                const time = new Date().toLocaleTimeString();
                                if (chart.data.labels.length > 20) {
                                    chart.data.labels.shift();
                                    chart.data.datasets[0].data.shift();
                                }
                                chart.data.labels.push(time);
                                chart.data.datasets[0].data.push(data.pending);
                                chart.update();
                            });
                        }, 500);
                    </script>
                </body>
                </html>
            )";
            res.set_content(html, "text/html");
        });

        // API to send data
        svr.Get("/stats", [](const httplib::Request&, httplib::Response& res) {
            std::string json = "{ \"pending\": " + std::to_string(g_pending) + 
                               ", \"workers\": " + std::to_string(g_workers) + " }";
            res.set_content(json, "application/json");
        });

        std::cout << "Server listening on http://localhost:8080" << std::endl;
        svr.listen("0.0.0.0", 8080);
    });

    // 3. Submit Tasks
    std::this_thread::sleep_for(std::chrono::seconds(2)); // Wait for server to start
    for(int i = 0; i < total_tasks; ++i) {
        pool.submit(heavy_task, i);
    }

    // 4. Update Stats Loop
    while(true) {
        g_pending = pool.get_tasks_queued();
        g_workers = pool.get_workers_count();
        if(g_pending == 0 && g_total > 0) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    server_thread.detach(); // Keep server running for a bit longer if needed
    return 0;
}