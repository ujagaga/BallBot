#pragma once
#include <pgmspace.h>

// ---------------- Index Page ----------------
const char index_html[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<title>BallBot Control</title>
<style>
html,body{
    margin:0;
    padding:0;
    width:100%;
    height:100%;
    background:black;
    font-family:Arial,Helvetica,sans-serif;
    overflow:hidden;
}
#cam{
    position:fixed;
    top:0;
    left:0;
    width:100%;
    height:100%;
    object-fit:cover;
    z-index:0;
}
#api-link{
    position:fixed;
    top:10px;
    right:12px;
    z-index:10;
    color:white;
    text-decoration:none;
    font-size:22px;
    background:rgba(0,0,0,0.4);
    border-radius:50%;
    width:32px;
    height:32px;
    text-align:center;
    line-height:32px;
}
#controls{
    position:fixed;
    bottom:0;
    left:0;
    width:100%;
    height:200px;
    background:rgba(0,0,0,0.55);
    display:flex;
    align-items:center;
    justify-content:space-between;
    padding:10px;
    box-sizing:border-box;
    z-index:5;
}
.group{
    display:flex;
    align-items:center;
    justify-content:center;
}
.joystick{
    display:grid;
    grid-template-columns:60px 60px 60px;
    grid-template-rows:60px 60px 60px;
    gap:5px;
}
.joystick button{
    width:60px;
    height:60px;
}
.stack button{
    width:100px;
    height:60px;
}
.center{
    flex-direction:column;    
}
.stack{
    flex-direction:column;
}
button{
    font-size:15px;
    background:rgba(255,255,255,0.85);
    border:none;
    border-radius:8px;
    padding: 5px;
    margin: 5px;
}
button:active{
    background:#ccc;
}
.hidden{visibility:hidden;}
#response-log {
    width: 200px;
    height: 180px; 
    background: rgba(15, 15, 15, 0.9);
    color: #00ff41; /* Classic Matrix/Terminal Green */
    font-family: 'Courier New', monospace;
    font-size: 11px;
    padding: 4px;
    margin: 8px 30px;
    border: 1px solid #333;
    border-radius: 4px;
    overflow-y: scroll;
    white-space: pre-line;
    text-align: left;
    line-height: 1.2;
}
#response-log::-webkit-scrollbar {
    display: none;
}
</style>
</head>
<body>

<img id="cam" src="/capture">
<a id="api-link" href="/api/">?</a>

<div id="controls">
    <!-- LEFT: Joystick -->
    <div class="group">
        <div class="joystick">
            <div></div>
            <button 
                onmousedown="startCmd('fwd')" onmouseup="stopCmd()" 
                ontouchstart="startCmd('fwd')" ontouchend="stopCmd()">FWD</button>
            <div></div>

            <button 
                onmousedown="startCmd('left')" onmouseup="stopCmd()" 
                ontouchstart="startCmd('left')" ontouchend="stopCmd()">LEFT</button>
            <div></div>
            <button 
                onmousedown="startCmd('right')" onmouseup="stopCmd()" 
                ontouchstart="startCmd('right')" ontouchend="stopCmd()">RIGHT</button>

            <div></div>            
            <button 
                onmousedown="startCmd('rev')" onmouseup="stopCmd()" 
                ontouchstart="startCmd('rev')" ontouchend="stopCmd()">REV</button>
            <div></div>
        </div>
        <div id="response-log"></div>
    </div>

    <div class="group stack">
        <button onclick="toggleSnapshots()">STREAM</button>
        <button onclick="cmdOnce('distance')">MEASURE</button>
    </div>

    <div class="group stack">
        <button onclick="cmdOnce('grab')">GRAB</button>
        <button onclick="cmdOnce('release')">RELEASE</button>
    </div>
    
</div>

<script>
let snapTimer = null;
let cmdActive = false; 
const MIN_INTERVAL = 500; 

function logResponse(text) {
    const log = document.getElementById('response-log');
    if(!log) return;
    log.innerHTML += `> ${text}\n`;
    log.scrollTop = log.scrollHeight;
}

function refreshImage() {
    if (!snapTimer) return; // Exit if the stream was turned off

    const img = document.getElementById('cam');
    const startTime = Date.now();

    // 1. Define what happens when the image SUCCESSFULLY arrives
    img.onload = () => {
        console.log(`Image loaded in ${Date.now() - startTime}ms`);
        // Only schedule the next one if we are still in "snapTimer" mode
        if (snapTimer) {
            setTimeout(refreshImage, 1000); // 1 second gap AFTER download finishes
        }
    };

    // 2. Define what happens if the image FAILS (important!)
    img.onerror = () => {
        console.error("Image failed to load. Retrying in 2 seconds...");
        if (snapTimer) {
            setTimeout(refreshImage, 2000); // Wait a bit longer before retrying
        }
    };

    // 3. Trigger the request
    img.src = '/capture?ts=' + Date.now();
}

function toggleSnapshots() {
    // If it was off, turn it on and kick off the first request
    if (!snapTimer) {
        snapTimer = true;
        logResponse("Stream Started");
        refreshImage();
    } else {
        // If it was on, turn it off (the onload check will stop the loop)
        snapTimer = false;
        logResponse("Stream Stopped");
    }
}

async function runCommandLoop(action) {
    if (!cmdActive) return;

    const startTime = Date.now(); // Mark when the request started

    try {
        const response = await fetch('/api/cmd?action=' + action + '&timeout=800');
        const data = await response.text();
        // Optional: logResponse(action + ": " + data);
    } catch (err) {
        console.error("Loop error:", err);
    } finally {
        if (cmdActive) {
            // Calculate how long the request actually took
            const executionTime = Date.now() - startTime;
            const remainingDelay = Math.max(0, MIN_INTERVAL - executionTime);            
            setTimeout(() => runCommandLoop(action), remainingDelay);
        }
    }
}

function startCmd(action) {
    if (cmdActive) return;
    cmdActive = true;
    runCommandLoop(action);
}

function stopCmd() {
    cmdActive = false; 
    fetch('/api/cmd?action=stop')
        .then(r => r.text())
        .then(data => logResponse("Stop: " + data))
        .catch(e => logResponse("Stop Error"));
}

function cmdOnce(action){
    fetch('/api/cmd?action=' + action)
        .then(r => r.text())
        .then(data => logResponse(action.toUpperCase() + ": " + data));
}
</script>
</body>
</html>
)HTML";

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
