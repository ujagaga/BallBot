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
    </div>

    <!-- RIGHT: Claw -->
    <div class="group stack">
        <button onclick="cmdOnce('grab')">GRAB</button>
        <button onclick="cmdOnce('release')">RELEASE</button>
    </div>
</div>

<script>
let snapTimer = null;
let cmdInterval = null; // Variable to hold the repetition timer

function refreshImage(){
    const img = document.getElementById('cam');
    img.onload = () => {
        if(snapTimer){
            setTimeout(refreshImage, 400);
        }
    };
    img.src = '/capture?ts=' + Date.now();
}

function toggleSnapshots(){
    if(snapTimer){
        snapTimer = null;
    } else {
        snapTimer = true;
        refreshImage();
    }
}

// Function to start repeating a command
function startCmd(action) {
    if (cmdInterval) return; // Prevent multiple timers if button is mashed
    
    // Send the first command immediately
    fetch('/api/cmd?action=' + action);
    
    // Set up the repetition every 300ms
    cmdInterval = setInterval(() => {
        fetch('/api/cmd?action=' + action);
    }, 300);
}

// Function to stop repeating and send a stop command
function stopCmd() {
    if (cmdInterval) {
        clearInterval(cmdInterval);
        cmdInterval = null;
    }
    fetch('/api/cmd?action=stop');
}

function cmdOnce(action){
    fetch('/api/cmd?action=' + action);
}
</script>
</body>
</html>
)HTML";

)rawliteral";