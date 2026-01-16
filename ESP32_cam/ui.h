#pragma once
#include <pgmspace.h>

// ---------------- Index Page ----------------
const char index_html[] PROGMEM = R"rawliteral(
#pragma once
#include <pgmspace.h>

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
    width: calc(100vw - 400px);
    height: 52px; /* Fixed height for ~3 rows */
    background: rgba(15, 15, 15, 0.9);
    color: #00ff41; /* Classic Matrix/Terminal Green */
    font-family: 'Courier New', monospace;
    font-size: 11px;
    padding: 4px;
    margin-top: 8px;
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

<a id="api-link" href="/api">?</a>

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
    </div>

    <!-- CENTER: Snapshot -->
    <div class="group center">
        <button onclick="toggleSnapshots()">STREAM</button>
        <div id="response-log">System Ready...</div>
    </div>

    <!-- RIGHT: Claw -->
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

function refreshImage(){
    const img = document.getElementById('cam');
    img.onload = () => { if(snapTimer) setTimeout(refreshImage, 1000); };
    img.src = '/capture?ts=' + Date.now();
}

function toggleSnapshots(){
    snapTimer = !snapTimer;
    if(snapTimer) refreshImage();
    logResponse(snapTimer ? "Stream Started" : "Stream Stopped");
}

/**
 * RECURSIVE LOOP WITH MINIMUM DELAY
 */
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
            
            // Calculate remaining time to hit the 800ms floor
            // If executionTime was 200ms, delay is 600ms.
            // If executionTime was 900ms, delay is 0ms.
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

)rawliteral";