const MAX_LOG = 5;
let logBuffer = [];

function getVal(id) {
    return document.getElementById(id).value;
}

function now() {
    return new Date().toLocaleTimeString();
}

function setStatus(text) {
    document.getElementById("status").innerText =
        `Status [${now()}]: ${text}`;
}

function addLog(text) {
    logBuffer.unshift(`[${now()}] ${text}`);
    logBuffer = logBuffer.slice(0, MAX_LOG);

    document.getElementById("log").innerHTML =
        [...logBuffer].reverse().join("<br>");
}

function sendCmd(cmd, val = null) {
    let url = `/api?cmd=${cmd}`;
    if (val !== null) {
        url += `&val=${val}`;
    }

    fetch(url)
        .then(r => r.json())
        .then(data => {
            if (data.status === "ok") {
                setStatus("OK");
                if (data.response) {
                    addLog(data.response);
                }
            } else {
                setStatus("ERROR");
                addLog(data.message);
            }
        })
        .catch(err => {
            setStatus("ERROR");
            addLog(err.toString());
        });
}
