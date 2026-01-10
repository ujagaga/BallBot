const MAX_LOG = 5;
let logBuffer = [];

function getVal(id) {
    return document.getElementById(id).value;
}

function now() {
    return new Date().toLocaleTimeString();
}

function addLog(text) {
    logBuffer.unshift(`[${now()}] ${text}`);
    logBuffer = logBuffer.slice(0, MAX_LOG);

    document.getElementById("log").innerHTML =
        [...logBuffer].reverse().join("<br>");
}

// sendDistance() reads the fields and calls sendCmd
function sendDistance() {
    let distance = parseInt(document.getElementById("distance").value);
    let speed = parseInt(document.getElementById("distance_speed").value);
    let keepDir = document.getElementById("keepDir").value;

    // Pass extra parameters as object
    sendCmd("distance", distance, { speed: speed, keepDir: keepDir });
}

// Modified sendCmd to append extra params
function sendCmd(cmd, val = null, extra = {}) {
    let url = `/api?cmd=${cmd}`;
    if (val !== null) url += `&val=${val}`;
    for (const key in extra) {
        url += `&${key}=${extra[key]}`;
    }

    fetch(url)
        .then(r => r.json())
        .then(data => {
            if (data.status === "ok") {
                if (data.response) addLog(JSON.stringify(data.response));
            } else {
                addLog(data.message);
            }
        })
        .catch(err => {
            addLog(err.toString());
        });
}

