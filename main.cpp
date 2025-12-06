#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include <string>
#include "ThreadPool.h"
#include "httplib.h" 

// --- GLOBAL STATS ---
std::atomic<int> g_pending{0};
std::atomic<int> g_workers{0};
std::atomic<int> g_total{0};
std::atomic<int> g_completed{0}; 
std::atomic<int> g_task_delay{50}; // Default 50ms delay
auto g_start_time = std::chrono::steady_clock::now(); 

// Simulated heavy task with VARIABLE speed
void heavy_task(int id) {
    // We use the atomic g_task_delay to control speed dynamically
    std::this_thread::sleep_for(std::chrono::milliseconds(g_task_delay.load()));
    g_completed++; 
}

int main() {
    // 1. Setup
    int cores = std::thread::hardware_concurrency();
    ThreadPool pool(cores);
    int total_tasks = 5000; 
    g_total = total_tasks;

    // 2. Start Web Server
    std::thread server_thread([&pool]() {
        httplib::Server svr;

        // Serve the Dashboard HTML
        svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
            std::string html = R"HTML(
                <!DOCTYPE html>
                <html>
                <head>
                    <title>NEXUS // Thread Command</title>
                    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
                    <link href="https://fonts.googleapis.com/css2?family=Rajdhani:wght@400;600;700&family=JetBrains+Mono:wght@400;700&display=swap" rel="stylesheet">
                    <style>
                        :root { 
                            --bg: #050505; 
                            --panel: rgba(20, 20, 30, 0.6); 
                            --accent: #00f3ff; 
                            --accent-glow: rgba(0, 243, 255, 0.4);
                            --danger: #ff0055; 
                            --success: #00ff9d;
                            --text: #e0e0e0;
                        }
                        
                        body { 
                            background-color: var(--bg); 
                            color: var(--text); 
                            font-family: 'Rajdhani', sans-serif; 
                            margin: 0; padding: 20px; 
                            height: 100vh; overflow: hidden; 
                            box-sizing: border-box;
                        }

                        /* ANIMATED BACKGROUND */
                        #particles { position: absolute; top: 0; left: 0; width: 100%; height: 100%; z-index: -1; opacity: 0.3; }

                        /* HEADER */
                        header { 
                            display: flex; justify-content: space-between; align-items: center; 
                            border-bottom: 1px solid var(--accent); padding-bottom: 15px; margin-bottom: 20px; 
                            text-shadow: 0 0 10px var(--accent-glow);
                        }
                        h1 { margin: 0; font-weight: 700; letter-spacing: 2px; font-size: 2rem; color: var(--accent); }
                        .status { font-family: 'JetBrains Mono', monospace; font-size: 0.8rem; color: var(--success); display: flex; align-items: center; gap: 10px; }
                        .blink { width: 8px; height: 8px; background: var(--success); border-radius: 50%; box-shadow: 0 0 8px var(--success); animation: pulse 1s infinite; }
                        
                        @keyframes pulse { 0% { opacity: 1; } 50% { opacity: 0.2; } 100% { opacity: 1; } }

                        /* GRID LAYOUT */
                        .grid { display: grid; grid-template-columns: 320px 1fr; gap: 20px; height: calc(100% - 80px); }
                        
                        /* CARDS */
                        .card { 
                            background: var(--panel); 
                            border: 1px solid rgba(255, 255, 255, 0.1); 
                            border-left: 2px solid var(--accent);
                            backdrop-filter: blur(10px); 
                            padding: 20px; border-radius: 4px; 
                            display: flex; flex-direction: column;
                            box-shadow: 0 10px 30px rgba(0,0,0,0.5);
                        }
                        .card-title { font-family: 'JetBrains Mono', monospace; font-size: 0.7rem; color: var(--accent); letter-spacing: 1px; margin-bottom: 10px; text-transform: uppercase; }

                        /* SIDEBAR CONTROLS */
                        .sidebar { display: flex; flex-direction: column; gap: 20px; }
                        
                        input[type=range] { width: 100%; accent-color: var(--accent); margin: 15px 0; }
                        
                        .btn-chaos { 
                            background: transparent; border: 1px solid var(--danger); color: var(--danger); 
                            font-family: 'JetBrains Mono', monospace; padding: 15px; 
                            font-weight: bold; cursor: pointer; transition: 0.3s; 
                            text-transform: uppercase; letter-spacing: 2px;
                            box-shadow: 0 0 10px rgba(255, 0, 85, 0.2);
                        }
                        .btn-chaos:hover { background: var(--danger); color: #fff; box-shadow: 0 0 20px var(--danger); }
                        .btn-chaos:active { transform: scale(0.98); }

                        /* METRICS ROW */
                        .metrics { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 15px; margin-bottom: 20px; }
                        .metric-val { font-size: 3rem; font-weight: 700; line-height: 1; }
                        
                        /* LOGS */
                        .terminal { 
                            background: rgba(0,0,0,0.5); font-family: 'JetBrains Mono', monospace; 
                            font-size: 0.8rem; color: #aaa; padding: 10px; flex-grow: 1; 
                            overflow-y: auto; border: 1px solid #333;
                        }
                        .log-line { margin-bottom: 5px; border-left: 2px solid #333; padding-left: 8px; }
                        .log-line.warn { border-left-color: var(--danger); color: #ff8888; }
                        .log-line.success { border-left-color: var(--success); color: #ccffdd; }

                        /* TOAST NOTIFICATION */
                        #toast {
                            visibility: hidden; min-width: 250px; background-color: #333; color: #fff; text-align: center;
                            border-left: 4px solid var(--accent); border-radius: 2px; padding: 16px; position: fixed; z-index: 100;
                            left: 50%; bottom: 30px; transform: translateX(-50%); font-family: 'JetBrains Mono';
                            box-shadow: 0 5px 15px rgba(0,0,0,0.5); opacity: 0; transition: opacity 0.5s, bottom 0.5s;
                        }
                        #toast.show { visibility: visible; opacity: 1; bottom: 50px; }

                        /* CHART CONTAINER */
                        .chart-wrapper { flex-grow: 1; position: relative; min-height: 0; }
                    </style>
                </head>
                <body>
                    <canvas id="particles"></canvas>

                    <header>
                        <div>
                            <h1>Thread Manager</h1>
                            <div style="font-size: 0.8rem; color: #666; letter-spacing: 3px;">THREAD POOL ORCHESTRATOR</div>
                        </div>
                        <div class="status"><div class="blink"></div> SYSTEM ONLINE</div>
                    </header>

                    <div class="grid">
                        <div class="sidebar">
                            <div class="card">
                                <div class="card-title">Flux Control</div>
                                <label style="font-size: 0.9rem; color: #ccc;">Cycle Delay: <span id="speedDisplay" style="color: var(--accent)">50</span>ms</label>
                                <input type="range" min="1" max="200" value="50" oninput="updateSpeed(this.value)">
                                <div style="display: flex; justify-content: space-between; font-size: 0.7rem; color: #666;">
                                    <span>FAST</span> <span>SLOW</span>
                                </div>
                            </div>

                            <div class="card">
                                <div class="card-title">Stress Test</div>
                                <button class="btn-chaos" onclick="injectChaos()">INJECT LOAD</button>
                                <p style="font-size: 0.7rem; color: #666; text-align: center; margin-top: 10px;">Warning: High CPU Usage</p>
                            </div>

                            <div class="card" style="flex-grow: 1;">
                                <div class="card-title">System Logs</div>
                                <div class="terminal" id="console">
                                    <div class="log-line">> Initializing Nexus Core...</div>
                                    <div class="log-line success">> Connection Established.</div>
                                </div>
                            </div>
                        </div>

                        <div style="display: flex; flex-direction: column;">
                            <div class="metrics">
                                <div class="card">
                                    <div class="card-title">Active Workers</div>
                                    <div class="metric-val" id="workers" style="color: #ffcc00">0</div>
                                </div>
                                <div class="card">
                                    <div class="card-title">Pending Queue</div>
                                    <div class="metric-val" id="pending" style="color: var(--accent)">0</div>
                                </div>
                                <div class="card">
                                    <div class="card-title">Completion</div>
                                    <div class="metric-val" id="percent" style="color: var(--success)">0%</div>
                                </div>
                            </div>

                            <div class="card" style="flex-grow: 1;">
                                <div class="card-title">Real-Time Throughput Analysis</div>
                                <div class="chart-wrapper">
                                    <canvas id="mainChart"></canvas>
                                </div>
                            </div>
                        </div>
                    </div>

                    <div id="toast">Notification Message</div>

                    <script>
                        // --- PARTICLE BACKGROUND ---
                        const canvas = document.getElementById('particles');
                        const ctxPart = canvas.getContext('2d');
                        canvas.width = window.innerWidth; canvas.height = window.innerHeight;
                        const particles = [];
                        for(let i=0; i<50; i++) particles.push({x: Math.random()*canvas.width, y: Math.random()*canvas.height, vx: (Math.random()-0.5), vy: (Math.random()-0.5)});
                        
                        function animateParticles() {
                            ctxPart.clearRect(0, 0, canvas.width, canvas.height);
                            ctxPart.fillStyle = 'rgba(0, 243, 255, 0.5)';
                            ctxPart.strokeStyle = 'rgba(0, 243, 255, 0.1)';
                            
                            particles.forEach((p, index) => {
                                p.x += p.vx; p.y += p.vy;
                                if(p.x < 0 || p.x > canvas.width) p.vx *= -1;
                                if(p.y < 0 || p.y > canvas.height) p.vy *= -1;
                                ctxPart.beginPath(); ctxPart.arc(p.x, p.y, 2, 0, Math.PI*2); ctxPart.fill();
                                
                                // Connect lines
                                for(let j=index+1; j<particles.length; j++) {
                                    const p2 = particles[j];
                                    const dist = Math.hypot(p.x-p2.x, p.y-p2.y);
                                    if(dist < 150) {
                                        ctxPart.beginPath(); ctxPart.moveTo(p.x, p.y); ctxPart.lineTo(p2.x, p2.y); ctxPart.stroke();
                                    }
                                }
                            });
                            requestAnimationFrame(animateParticles);
                        }
                        animateParticles();

                        // --- CHART ---
                        Chart.defaults.color = '#555';
                        Chart.defaults.borderColor = 'rgba(255,255,255,0.05)';
                        const ctxChart = document.getElementById('mainChart').getContext('2d');
                        const gradient = ctxChart.createLinearGradient(0, 0, 0, 400);
                        gradient.addColorStop(0, 'rgba(0, 243, 255, 0.2)');
                        gradient.addColorStop(1, 'rgba(0, 243, 255, 0)');

                        const mainChart = new Chart(ctxChart, {
                            type: 'line',
                            data: {
                                labels: [],
                                datasets: [{
                                    label: 'Pending Tasks',
                                    data: [],
                                    borderColor: '#00f3ff',
                                    backgroundColor: gradient,
                                    borderWidth: 2,
                                    fill: true,
                                    tension: 0.4,
                                    pointRadius: 0
                                }]
                            },
                            options: {
                                responsive: true, maintainAspectRatio: false,
                                plugins: { legend: { display: false } },
                                scales: { x: { grid: { display: false } } },
                                animation: false
                            }
                        });

                        // --- LOGIC ---
                        function showToast(msg) {
                            const x = document.getElementById("toast");
                            x.innerText = msg;
                            x.className = "show";
                            setTimeout(function(){ x.className = x.className.replace("show", ""); }, 3000);
                        }

                        function log(msg, type='') {
                            const box = document.getElementById('console');
                            const div = document.createElement('div');
                            div.className = 'log-line ' + type;
                            div.innerText = `> ${msg}`;
                            box.prepend(div);
                            if(box.children.length > 15) box.lastChild.remove();
                        }

                        function updateSpeed(val) {
                            document.getElementById('speedDisplay').innerText = val;
                            fetch('/set_speed?val=' + val);
                        }

                        function injectChaos() {
                            fetch('/inject').then(() => {
                                showToast("CHAOS INITIATED: +1000 TASKS");
                                log("Injecting High Load Payload...", "warn");
                            });
                        }

                        let lastCompleted = 0;

                        setInterval(() => {
                            fetch('/stats').then(r => r.json()).then(data => {
                                document.getElementById('workers').innerText = data.workers;
                                document.getElementById('pending').innerText = data.pending;
                                
                                const total = data.total;
                                const pct = total > 0 ? Math.round(((total - data.pending)/total)*100) : 0;
                                document.getElementById('percent').innerText = pct + "%";

                                // Chart
                                const time = new Date().toLocaleTimeString().split(' ')[0];
                                if (mainChart.data.labels.length > 60) {
                                    mainChart.data.labels.shift();
                                    mainChart.data.datasets[0].data.shift();
                                }
                                mainChart.data.labels.push(time);
                                mainChart.data.datasets[0].data.push(data.pending);
                                mainChart.update();

                                // Logs
                                const diff = data.completed - lastCompleted;
                                if(diff > 0) {
                                    if(diff > 5) log(`Processed Batch: ${diff} units`, "success");
                                }
                                lastCompleted = data.completed;
                            });
                        }, 500);
                    </script>
                </body>
                </html>
            )HTML"; 
            res.set_content(html, "text/html");
        });

        // API: Get Stats
        svr.Get("/stats", [](const httplib::Request&, httplib::Response& res) {
            std::string json = "{ \"pending\": " + std::to_string(g_pending) + 
                               ", \"workers\": " + std::to_string(g_workers) + 
                               ", \"completed\": " + std::to_string(g_completed) + 
                               ", \"total\": " + std::to_string(g_total) + " }";
            res.set_content(json, "application/json");
        });

        // API: Inject Chaos
        svr.Get("/inject", [&pool](const httplib::Request&, httplib::Response& res) {
            for(int i = 0; i < 1000; ++i) pool.submit(heavy_task, i);
            g_total += 1000; 
            res.set_content("OK", "text/plain");
        });

        // API: Set Speed
        svr.Get("/set_speed", [](const httplib::Request& req, httplib::Response& res) {
            if (req.has_param("val")) {
                int val = std::stoi(req.get_param_value("val"));
                g_task_delay = val;
            }
            res.set_content("OK", "text/plain");
        });

        std::cout << "Server listening on http://localhost:8080" << std::endl;
        svr.listen("0.0.0.0", 8080);
    });

    // 3. Submit Initial Tasks
    std::this_thread::sleep_for(std::chrono::seconds(2)); 
    for(int i = 0; i < total_tasks; ++i) {
        pool.submit(heavy_task, i);
    }

    // 4. Monitoring Loop
   // 4. Monitoring Loop
    while(true) {
        g_pending = pool.get_tasks_queued();
        
        // Check if queue is empty AND the initial load is finished
        if(g_pending == 0 && g_total > 0) {
             std::cout << "\n[NEXUS] ALL TASKS COMPLETE. SYSTEM SHUTDOWN IN 3 SECONDS..." << std::endl;
             std::this_thread::sleep_for(std::chrono::seconds(3));
             break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "[NEXUS] OFFLINE." << std::endl;
    server_thread.detach(); 
    return 0;
}