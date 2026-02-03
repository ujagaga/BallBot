#pragma once
#include <pgmspace.h>

// ---------------- API Wiki Page ----------------
const char api_html[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>BallBot API Documentation</title>
    <style>
        body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif; line-height: 1.6; color: #e1e4e8; background: #0d1117; margin: 0; padding: 20px; }
        .container { max-width: 800px; margin: 0 auto; }
        h1 { color: #58a6ff; border-bottom: 1px solid #30363d; padding-bottom: 10px; }
        h2 { color: #79c0ff; margin-top: 30px; }
        .endpoint { background: #161b22; border: 1px solid #30363d; border-radius: 6px; padding: 15px; margin-bottom: 20px; }
        code { background: rgba(110,118,129,0.4); padding: 0.2em 0.4em; border-radius: 6px; font-family: monospace; color: #ff7b72; }
        .method { font-weight: bold; color: #7ee787; margin-right: 10px; }
        table { width: 100%; border-collapse: collapse; margin-top: 10px; }
        th, td { text-align: left; padding: 8px; border-bottom: 1px solid #30363d; }
        th { color: #8b949e; }
        .example { background: #000; padding: 10px; border-radius: 4px; border-left: 4px solid #238636; overflow-x: auto; }
        a { color: #58a6ff; text-decoration: none; }
        a:hover { text-decoration: underline; }
    </style>
</head>
<body>
    <div class="container">
        <h1>BallBot API Wiki</h1>
        <p>Control the robot via HTTP GET/POST requests. Base URL: <code>http://[device-ip]/</code></p>

        <h2>Core Commands</h2>
        <div class="endpoint">
            <span class="method">GET</span> <code>/api/cmd?action=[action]&timeout=[ms]</code>
            <p>Executes a robot movement or action.</p>
            <table>
                <tr><th>Action</th><th>Description</th></tr>
                <tr><td><code>fwd</code> / <code>rev</code></td><td>Move 10 units forward or backward.</td></tr>
                <tr><td><code>left</code> / <code>right</code></td><td>Increment steering by 20 units.</td></tr>
                <tr><td><code>grab</code> / <code>release</code></td><td>Actuate the ball claw.</td></tr>
                <tr><td><code>stop</code></td><td>Stops all motors and returns system logs.</td></tr>
                <tr><td><code>distance</code></td><td>Returns current sensor reading in plain text.</td></tr>
            </table>
            <p><strong>Example:</strong> <a href="/api/cmd?action=fwd&timeout=1000">/api/cmd?action=fwd&timeout=1000</a></p>
        </div>

        <h2>System & Diagnostics</h2>
        <div class="endpoint">
            <span class="method">GET</span> <code>/api/logs</code>
            <p>Returns the last 5 system log entries as plain text.</p>
        </div>

        <div class="endpoint">
            <span class="method">GET</span> <code>/capture</code>
            <p>Returns a single JPEG frame from the camera. Use <code>?ts=[now]</code> to bypass browser cache.</p>
        </div>

        <h2>Firmware Update</h2>
        <div class="endpoint">
            <span class="method">POST</span> <code>/api/ota</code>
            <p>Triggers an asynchronous Over-The-Air update.</p>
            <p><strong>Body:</strong> The raw URL to the <code>.bin</code> firmware file.</p>
            <p><strong>Header:</strong> <code>X-Log-Callback</code> (Optional) URL to send progress logs.</p>
        </div>

        <p style="text-align: center; margin-top: 50px;"><a href="/">Back to Controller UI</a></p>
    </div>
</body>
</html>
)HTML";
