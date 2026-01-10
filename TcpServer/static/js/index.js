// ---------------- UTILITY ----------------
const MAX_LOG = 5;
let logBuffer = [];

function getVal(id) {
    return document.getElementById(id).value;
}

function addLog(text) {
    logBuffer.unshift(`[${new Date().toLocaleTimeString()}] ${text}`);
    logBuffer = logBuffer.slice(0, MAX_LOG);
    document.getElementById("log").innerHTML = [...logBuffer].reverse().join("<br>");
}

function sendCmd(cmd, val = null, speed = null, keepDir = null) {
    let url = `/api?cmd=${cmd}`;
    if (val !== null) url += `&val=${val}`;
    if (speed !== null) url += `&speed=${speed}`;
    if (keepDir !== null) url += `&keepDir=${keepDir}`;

    fetch(url)
        .then(r => r.json())
        .then(data => {
            if (data.status === 'ok' && data.response) addLog(data.response);
        })
        .catch(err => addLog(err.toString()));
}

// ---------------- MOTOR CONTROL ----------------
let motorInterval = null;

function startMotor(direction) {
    const speed = parseInt(getVal('motorSpeed')) || 600;
    const keepDir = document.getElementById('keepDir').checked;
    const step = direction === 'up' ? 1 : -1;

    // Send immediately
    sendCmd('distance', step, speed, keepDir);

    // Repeat while holding
    motorInterval = setInterval(() => {
        sendCmd('distance', step, speed, keepDir);
    }, 100); // repeat every 100 ms
}

function stopMotor() {
    if (motorInterval) {
        clearInterval(motorInterval);
        motorInterval = null;
    }
}

// ---------------- STEERING SERVO ----------------
let steerInterval = null;

function startSteer(direction) {
    const increment = direction === 'left' ? -5 : 5;

    // Send immediately
    sendCmd('servoSteer', increment);

    // Repeat while holding
    steerInterval = setInterval(() => {
        sendCmd('servoSteer', increment);
    }, 100);
}

function stopSteer() {
    if (steerInterval) {
        clearInterval(steerInterval);
        steerInterval = null;
    }
}

// ---------------- BUTTON EVENT BINDINGS ----------------
window.addEventListener('DOMContentLoaded', () => {
    // Motor UP/DOWN
    const motorUpBtn = document.querySelector('button[onclick*="moveMotor(\'up\')"]');
    const motorDownBtn = document.querySelector('button[onclick*="moveMotor(\'down\')"]');

    motorUpBtn.addEventListener('mousedown', () => startMotor('up'));
    motorUpBtn.addEventListener('mouseup', stopMotor);
    motorUpBtn.addEventListener('mouseleave', stopMotor);
    motorUpBtn.addEventListener('touchstart', () => startMotor('up'));
    motorUpBtn.addEventListener('touchend', stopMotor);

    motorDownBtn.addEventListener('mousedown', () => startMotor('down'));
    motorDownBtn.addEventListener('mouseup', stopMotor);
    motorDownBtn.addEventListener('mouseleave', stopMotor);
    motorDownBtn.addEventListener('touchstart', () => startMotor('down'));
    motorDownBtn.addEventListener('touchend', stopMotor);

    // Steering LEFT/RIGHT
    const steerLeftBtn = document.querySelector('button[onclick*="steerServo(\'left\')"]');
    const steerRightBtn = document.querySelector('button[onclick*="steerServo(\'right\')"]');

    steerLeftBtn.addEventListener('mousedown', () => startSteer('left'));
    steerLeftBtn.addEventListener('mouseup', stopSteer);
    steerLeftBtn.addEventListener('mouseleave', stopSteer);
    steerLeftBtn.addEventListener('touchstart', () => startSteer('left'));
    steerLeftBtn.addEventListener('touchend', stopSteer);

    steerRightBtn.addEventListener('mousedown', () => startSteer('right'));
    steerRightBtn.addEventListener('mouseup', stopSteer);
    steerRightBtn.addEventListener('mouseleave', stopSteer);
    steerRightBtn.addEventListener('touchstart', () => startSteer('right'));
    steerRightBtn.addEventListener('touchend', stopSteer);
});
