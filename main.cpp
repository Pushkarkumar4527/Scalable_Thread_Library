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
                        body { font-family: 'Segoe UI', Roboto, Helvetica, Arial, sans-serif; background: #0f172a; color: #f8fafc; text-align: center; margin: 0; padding: 20px; }
                        h1 { color: #38bdf8; font-weight: 300; letter-spacing: 1px; margin-bottom: 40px; }
                        
                        /* Layout */
                        .dashboard-grid { display: flex; justify-content: center; gap: 20px; flex-wrap: wrap; margin-bottom: 30px; }
                        .card { background: #1e293b; padding: 20px; border-radius: 12px; min-width: 200px; box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.5); border: 1px solid #334155; }
                        
                        /* Typography */
                        .card h3 { margin: 0; font-size: 0.9rem; color: #94a3b8; text-transform: uppercase; letter-spacing: 1px; }
                        .card h2 { margin: 10px 0 0 0; font-size: 2.5rem; font-weight: 700; }
                        
                        /* Chart Container */
                        .chart-container { background: #1e293b; padding: 20px; border-radius: 12px; border: 1px solid #334155; max-width: 1000px; margin: 0 auto; }
                        canvas { width: 100% !important; height: 400px !important; }

                        /* Status Pulse Animation */
                        .status-badge { display: inline-flex; align-items: center; background: #064e3b; color: #34d399; padding: 5px 12px; border-radius: 20px; font-size: 0.8rem; font-weight: bold; margin-bottom: 20px; }
                        .dot { height: 8px; width: 8px; background-color: #34d399; border-radius: 50%; display: inline-block; margin-right: 8px; animation: pulse 1.5s infinite; }
                        @keyframes pulse { 0% { opacity: 1; } 50% { opacity: 0.5; } 100% { opacity: 1; } }
                    </style>
                </head>
                <body>
                    <div class="status-badge"><span class="dot"></span> SYSTEM ONLINE</div>
                    <h1>Scalable Thread Pool Control Center</h1>
                    
                    <div class="dashboard-grid">
                        <div class="card">
                            <h3>Active Workers</h3>
                            <h2 id="workers" style="color: #fbbf24">0</h2>
                        </div>
                        <div class="card">
                            <h3>Pending Tasks</h3>
                            <h2 id="pending" style="color: #38bdf8">0</h2>
                        </div>
                        <div class="card">
                            <h3>Throughput</h3>
                            <h2 style="color: #4ade80">High</h2>
                        </div>
                    </div>

                    <div class="chart-container">
                        <canvas id="myChart"></canvas>
                    </div>

                    <script>
                        const ctx = document.getElementById('myChart').getContext('2d');
                        
                        // Create a gradient fill
                        const gradient = ctx.createLinearGradient(0, 0, 0, 400);
                        gradient.addColorStop(0, 'rgba(56, 189, 248, 0.5)'); // Light Blue
                        gradient.addColorStop(1, 'rgba(56, 189, 248, 0.0)'); // Transparent

                        const chart = new Chart(ctx, {
                            type: 'line',
                            data: {
                                labels: [],
                                datasets: [{
                                    label: 'Pending Queue Depth',
                                    borderColor: '#38bdf8',
                                    backgroundColor: gradient,
                                    borderWidth: 2,
                                    pointRadius: 0,
                                    data: [],
                                    fill: true,
                                    tension: 0.4 // Smooth curves
                                }]
                            },
                            options: {
                                responsive: true,
                                maintainAspectRatio: false,
                                plugins: { legend: { display: false } },
                                scales: {
                                    x: { grid: { color: '#334155' }, ticks: { color: '#94a3b8' } },
                                    y: { beginAtZero: true, grid: { color: '#334155' }, ticks: { color: '#94a3b8' } }
                                },
                                animation: false // Performance optimization
                            }
                        });

                        setInterval(() => {
                            fetch('/stats').then(r => r.json()).then(data => {
                                document.getElementById('workers').innerText = data.workers;
                                document.getElementById('pending').innerText = data.pending;
                                
                                const time = new Date().toLocaleTimeString();
                                if (chart.data.labels.length > 30) {
                                    chart.data.labels.shift();
                                    chart.data.datasets[0].data.shift();
                                }
                                chart.data.labels.push(time);
                                chart.data.datasets[0].data.push(data.pending);
                                chart.update();
                            });
                        }, 200); // Faster refresh rate
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