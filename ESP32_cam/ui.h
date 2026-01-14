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
            <button onmousedown="cmd('fwd')" onmouseup="stop()">FWD</button>
            <div></div>

            <button onmousedown="cmd('left')" onmouseup="stop()">LEFT</button>
            <div></div>
            <button onmousedown="cmd('right')" onmouseup="stop()">RIGHT</button>

            <div></div>
            <button onmousedown="cmd('rev')" onmouseup="stop()">REV</button>
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
let snapTimer=null;

function refreshImage(){
    const img=document.getElementById('cam');
    img.onload=()=> {
        if(snapTimer){
            setTimeout(refreshImage,200);
        }
    };
    img.src='/capture?ts='+Date.now();
}

function toggleSnapshots(){
    if(snapTimer){
        snapTimer=null;
    }else{
        snapTimer=true;
        refreshImage();
    }
}

function cmd(action){
    fetch('/api/cmd?action='+action);
}
function stop(){
    fetch('/api/cmd?action=stop');
}
function cmdOnce(action){
    fetch('/api/cmd?action='+action);
}
</script>
</body>
</html>
)HTML";

)rawliteral";