#pragma once
#include <pgmspace.h>

// ---------------- Index Page ----------------
const char index_html[] PROGMEM = R"HTML(
<!doctype html>
<html>
    <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <title>ESP32 OV2640</title>
<style>
body {
    font-family: Arial,Helvetica,sans-serif;
    background: #181818;
    color: #EFEFEF;
    font-size: 16px
}
h2 {
    font-size: 18px
}
section.main {
    display: flex
}
#menu,section.main {
    flex-direction: column
}
#menu {
    display: none;
    flex-wrap: nowrap;
    min-width: 340px;
    background: #363636;
    padding: 8px;
    border-radius: 4px;
    margin-top: -10px;
    margin-right: 10px;
}
#content {
    display: flex;
    flex-wrap: wrap;
    align-items: stretch
}
figure {
    padding: 0px;
    margin: 0;
    -webkit-margin-before: 0;
    margin-block-start: 0;
    -webkit-margin-after: 0;
    margin-block-end: 0;
    -webkit-margin-start: 0;
    margin-inline-start: 0;
    -webkit-margin-end: 0;
    margin-inline-end: 0
}
figure img {
    display: block;
    width: 100%;
    height: auto;
    border-radius: 4px;
    margin-top: 8px;
}
@media (min-width: 800px) and (orientation:landscape) {
    #content {
        display:flex;
        flex-wrap: nowrap;
        align-items: stretch
    }
    figure img {
        display: block;
        max-width: 100%;
        max-height: calc(100vh - 40px);
        width: auto;
        height: auto
    }

    figure {
        padding: 0 0 0 0px;
        margin: 0;
        -webkit-margin-before: 0;
        margin-block-start: 0;
        -webkit-margin-after: 0;
        margin-block-end: 0;
        -webkit-margin-start: 0;
        margin-inline-start: 0;
        -webkit-margin-end: 0;
        margin-inline-end: 0
    }
}

section#buttons {
    display: flex;
    flex-wrap: nowrap;
    justify-content: space-between
}
#nav-toggle {
    cursor: pointer;
    display: block
}
#nav-toggle-cb {
    outline: 0;
    opacity: 0;
    width: 0;
    height: 0
}
#nav-toggle-cb:checked+#menu {
    display: flex
}
.input-group {
    display: flex;
    flex-wrap: nowrap;
    line-height: 22px;
    margin: 5px 0
}
.input-group>label {
    display: inline-block;
    padding-right: 10px;
    min-width: 47%
}
.input-group input,.input-group select {
    flex-grow: 1
}
.range-max,.range-min {
    display: inline-block;
    padding: 0 5px
}
button, .button {
    display: block;
    margin: 5px;
    padding: 0 12px;
    border: 0;
    line-height: 28px;
    cursor: pointer;
    color: #fff;
    background: #ff3034;
    border-radius: 5px;
    font-size: 16px;
    outline: 0
}
button:hover {
    background: #ff494d
}

button:active {
    background: #f21c21
}
button.disabled {
    cursor: default;
    background: #a0a0a0
}
input[type=range] {
    -webkit-appearance: none;
    width: 100%;
    height: 22px;
    background: #363636;
    cursor: pointer;
    margin: 0
}
input[type=range]:focus {
    outline: 0
}
input[type=range]::-webkit-slider-runnable-track {
    width: 100%;
    height: 2px;
    cursor: pointer;
    background: #EFEFEF;
    border-radius: 0;
    border: 0 solid #EFEFEF
}
input[type=range]::-webkit-slider-thumb {
    border: 1px solid rgba(0,0,30,0);
    height: 22px;
    width: 22px;
    border-radius: 50px;
    background: #ff3034;
    cursor: pointer;
    -webkit-appearance: none;
    margin-top: -11.5px
}
input[type=range]:focus::-webkit-slider-runnable-track {
    background: #EFEFEF
}
input[type=range]::-moz-range-track {
    width: 100%;
    height: 2px;
    cursor: pointer;
    background: #EFEFEF;
    border-radius: 0;
    border: 0 solid #EFEFEF
}
input[type=range]::-moz-range-thumb {
    border: 1px solid rgba(0,0,30,0);
    height: 22px;
    width: 22px;
    border-radius: 50px;
    background: #ff3034;
    cursor: pointer
}
input[type=range]::-ms-track {
    width: 100%;
    height: 2px;
    cursor: pointer;
    background: 0 0;
    border-color: transparent;
    color: transparent
}
input[type=range]::-ms-fill-lower {
    background: #EFEFEF;
    border: 0 solid #EFEFEF;
    border-radius: 0
}
input[type=range]::-ms-fill-upper {
    background: #EFEFEF;
    border: 0 solid #EFEFEF;
    border-radius: 0
}
input[type=range]::-ms-thumb {
    border: 1px solid rgba(0,0,30,0);
    height: 22px;
    width: 22px;
    border-radius: 50px;
    background: #ff3034;
    cursor: pointer;
    height: 2px
}
input[type=range]:focus::-ms-fill-lower {
    background: #EFEFEF
}
input[type=range]:focus::-ms-fill-upper {
    background: #363636
}

.switch {
    display: block;
    position: relative;
    line-height: 22px;
    font-size: 16px;
    height: 22px
}
.switch input {
    outline: 0;
    opacity: 0;
    width: 0;
    height: 0
}

.slider {
    width: 50px;
    height: 22px;
    border-radius: 22px;
    cursor: pointer;
    background-color: grey
}
.slider,.slider:before {
    display: inline-block;
    transition: .4s
}
.slider:before {
    position: relative;
    content: "";
    border-radius: 50%;
    height: 16px;
    width: 16px;
    left: 4px;
    top: 3px;
    background-color: #fff
}

input:checked+.slider {
    background-color: #ff3034
}
input:checked+.slider:before {
    -webkit-transform: translateX(26px);
    transform: translateX(26px)
}

select {
    border: 1px solid #363636;
    font-size: 14px;
    height: 22px;
    outline: 0;
    border-radius: 5px
}

.image-container {
    position: relative;
    min-width: 160px
}

.hidden {
    display: none
}

input[type=text] {
    border: 1px solid #363636;
    font-size: 14px;
    height: 20px;
    margin: 1px;
    outline: 0;
    border-radius: 5px
}

.inline-button {
    line-height: 20px;
    margin: 2px;
    padding: 1px 4px 2px 4px;
}

label.toggle-section-label {
    cursor: pointer;
    display: block
}

input.toggle-section-button {
    outline: 0;
    opacity: 0;
    width: 0;
    height: 0
}

input.toggle-section-button:checked+section.toggle-section {
    display: none
}
#response-log {
    width: 200px;
    height: 100px;
    background: rgba(15, 15, 15, 0.9);
    color: #00ff41;
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
#controls{
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
</style>

</head>
<body>
<section class="main">
<div id="logo">
    <label for="nav-toggle-cb" id="nav-toggle">&#9776;&nbsp;&nbsp;Toggle OV2640 settings</label>
</div>
<div id="content">
    <div id="sidebar">
<input type="checkbox" id="nav-toggle-cb" checked="checked">
<nav id="menu">
    <section id="xclk-section" class="nothidden">
        <div class="input-group" id="set-xclk-group">
            <label for="set-xclk">XCLK MHz</label>
            <div class="text">
                <input id="xclk" type="text" minlength="1" maxlength="2" size="2" value="20">
            </div>
            <button class="inline-button" id="set-xclk">Set</button>
        </div>
    </section>

    <div class="input-group" id="framesize-group">
        <label for="framesize">Resolution</label>
        <select id="framesize" class="default-action">
            <!-- 2MP -->
            <option value="15">UXGA(1600x1200)</option>
            <option value="14">SXGA(1280x1024)</option>
            <option value="13">HD(1280x720)</option>
            <option value="12">XGA(1024x768)</option>
            <option value="11">SVGA(800x600)</option>
            <option value="10">VGA(640x480)</option>
        </select>
    </div>
    <div class="input-group" id="quality-group">
        <label for="quality">Quality</label>
        <div class="range-min">4</div>
        <input type="range" id="quality" min="4" max="63" value="10" class="default-action">
        <div class="range-max">63</div>
    </div>
    <div class="input-group" id="brightness-group">
        <label for="brightness">Brightness</label>
        <div class="range-min">-2</div>
        <input type="range" id="brightness" min="-2" max="2" value="0" class="default-action">
        <div class="range-max">2</div>
    </div>
    <div class="input-group" id="contrast-group">
        <label for="contrast">Contrast</label>
        <div class="range-min">-2</div>
        <input type="range" id="contrast" min="-2" max="2" value="0" class="default-action">
        <div class="range-max">2</div>
    </div>
    <div class="input-group" id="saturation-group">
        <label for="saturation">Saturation</label>
        <div class="range-min">-2</div>
        <input type="range" id="saturation" min="-2" max="2" value="0" class="default-action">
        <div class="range-max">2</div>
    </div>
    <div class="input-group" id="special_effect-group">
        <label for="special_effect">Special Effect</label>
        <select id="special_effect" class="default-action">
            <option value="0" selected="selected">No Effect</option>
            <option value="1">Negative</option>
            <option value="2">Grayscale</option>
            <option value="3">Red Tint</option>
            <option value="4">Green Tint</option>
            <option value="5">Blue Tint</option>
            <option value="6">Sepia</option>
        </select>
    </div>
    <div class="input-group" id="awb-group">
        <label for="awb">AWB</label>
        <div class="switch">
            <input id="awb" type="checkbox" class="default-action" checked="checked">
            <label class="slider" for="awb"></label>
        </div>
    </div>
    <div class="input-group" id="awb_gain-group">
        <label for="awb_gain">AWB Gain</label>
        <div class="switch">
            <input id="awb_gain" type="checkbox" class="default-action" checked="checked">
            <label class="slider" for="awb_gain"></label>
        </div>
    </div>
    <div class="input-group" id="aec-group">
        <label for="aec">AEC SENSOR</label>
        <div class="switch">
            <input id="aec" type="checkbox" class="default-action" checked="checked">
            <label class="slider" for="aec"></label>
        </div>
    </div>
    <div class="input-group" id="aec2-group">
        <label for="aec2">AEC DSP</label>
        <div class="switch">
            <input id="aec2" type="checkbox" class="default-action" checked="checked">
            <label class="slider" for="aec2"></label>
        </div>
    </div>
    <div class="input-group" id="ae_level-group">
        <label for="ae_level">AE Level</label>
        <div class="range-min">-2</div>
        <input type="range" id="ae_level" min="-2" max="2" value="0" class="default-action">
        <div class="range-max">2</div>
    </div>
    <div class="input-group" id="aec_value-group">
        <label for="aec_value">Exposure</label>
        <div class="range-min">0</div>
        <input type="range" id="aec_value" min="0" max="1200" value="204" class="default-action">
        <div class="range-max">1200</div>
    </div>
    <div class="input-group" id="agc-group">
        <label for="agc">AGC</label>
        <div class="switch">
            <input id="agc" type="checkbox" class="default-action" checked="checked">
            <label class="slider" for="agc"></label>
        </div>
    </div>
    <div class="input-group hidden" id="agc_gain-group">
        <label for="agc_gain">Gain</label>
        <div class="range-min">1x</div>
        <input type="range" id="agc_gain" min="0" max="30" value="5" class="default-action">
        <div class="range-max">31x</div>
    </div>
    <div class="input-group" id="gainceiling-group">
        <label for="gainceiling">Gain Ceiling</label>
        <div class="range-min">2x</div>
        <input type="range" id="gainceiling" min="0" max="6" value="0" class="default-action">
        <div class="range-max">128x</div>
    </div>
    <div class="input-group" id="raw_gma-group">
        <label for="raw_gma">Raw GMA</label>
        <div class="switch">
            <input id="raw_gma" type="checkbox" class="default-action" checked="checked">
            <label class="slider" for="raw_gma"></label>
        </div>
    </div>
    <section id="buttons">
        <button id="toggle-stream">Start Stream</button>
    </section>
</nav>
</div>
<figure>
    <div id="stream-container" class="image-container">
        <img id="stream" src="" crossorigin>
    </div>
</figure>
</div>
</section>

<div id="controls">
    <div class="group">
        <div class="joystick">
            <div></div>
            <button id="move-fwd">&#9650;</button>
            <div></div>
            <button id="move-left">&#9664;</button>
            <div></div>
            <button id="move-right">&#9654;</button>
            <div></div>
            <button id="move-rev">&#9660;</button>
            <div></div>
        </div>
    </div>
    <div class="group stack">
        <div id="response-log"></div>
        <button id="get-distance">DISTANCE</button>
    </div>
    <div class="group stack">
        <button id="claw-grab">GRAB</button>
        <button id="claw-release">RELEASE</button>
    </div>
</div>

<script>
document.addEventListener('DOMContentLoaded', function (event) {
  var baseHost = document.location.origin;
  var streamUrl = baseHost + ':81';
  
  let moveTimeout;
  let isMoving = false;

  function logResponse(text) {
    const log = document.getElementById('response-log');
    if(!log) return;
    log.innerHTML += `> ${text}\n`;
    log.scrollTop = log.scrollHeight;
  }

  const show = el => el && el.classList.remove('hidden');
  const hide = el => el && el.classList.add('hidden');

  const sendAction = async (action) => {
    const query = `${baseHost}/command?action=${action}`;
    try {
      const response = await fetch(query);
      const data = await response.text();
      logResponse(`${action}: ${data}`);
    } catch (e) {
      logResponse(`${action}: Error`);
    }
  };

  const moveLoop = async (action) => {
    if (!isMoving) return;
    
    await sendAction(action); 
    
    moveTimeout = setTimeout(() => {
      moveLoop(action); 
    }, 300);
  };

  const startMoving = (action) => {
    if (isMoving) return;
    isMoving = true;
    moveLoop(action);
  };

  const stopMoving = () => {
    isMoving = false;
    clearTimeout(moveTimeout);
    sendAction('stop');
  };

  const attachHoldEvents = (btnId, action) => {
    const btn = document.getElementById(btnId);
    if (!btn) return;
    
    btn.onmousedown = () => startMoving(action);
    btn.onmouseup = stopMoving;
    btn.onmouseleave = stopMoving;
    
    btn.ontouchstart = (e) => { e.preventDefault(); startMoving(action); };
    btn.ontouchend = (e) => { e.preventDefault(); stopMoving(); };
  };

  // Attach Joystick
  attachHoldEvents('move-fwd', 'fwd');
  attachHoldEvents('move-rev', 'rev');
  attachHoldEvents('move-left', 'left');
  attachHoldEvents('move-right', 'right');

  // Single Click Actions
  document.getElementById('get-distance').onclick = () => sendAction('distance');
  document.getElementById('claw-grab').onclick = () => sendAction('grab');
  document.getElementById('claw-release').onclick = () => sendAction('release');

  // --- CAMERA LOGIC ---
  const streamButton = document.getElementById('toggle-stream');
  const view = document.getElementById('stream');
  
  streamButton.onclick = () => {
    if (streamButton.innerHTML === 'Stop Stream') {
      view.src = "";
      streamButton.innerHTML = 'Start Stream';
    } else {
      view.src = `${streamUrl}/stream`;
      show(document.getElementById('stream-container'));
      streamButton.innerHTML = 'Stop Stream';
    }
  };

  function updateConfig(el) {
    let value = (el.type === 'checkbox') ? (el.checked ? 1 : 0) : el.value;
    const query = `${baseHost}/config?var=${el.id}&val=${value}`;
    fetch(query)
      .then(r => console.log(`Config ${el.id} updated`))
      .catch(e => console.log(`Config ${el.id} failed: ${e}`));
  }

  document.querySelectorAll('.default-action').forEach(el => {
    el.onchange = () => updateConfig(el);
  });

  setTimeout(() => { streamButton.click(); }, 2000);
});
</script>
</body>
</html>
)HTML";
