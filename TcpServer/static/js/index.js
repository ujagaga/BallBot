function getVal(id) {
    return document.getElementById(id).value;
}

function setStatus(text) {
    // Get current time
    const now = new Date();
    const timestamp = now.toLocaleTimeString(); // HH:MM:SS format

    // Update status div
    document.getElementById("status").innerText = `[${timestamp}] Status: ${text}`;
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
                setStatus(data.response || "OK");
            } else {
                setStatus("ERROR: " + data.message);
            }
        })
        .catch(err => {
            setStatus("ERROR: " + err);
        });
}

function measureDistance() {
    fetch("/api?cmd=dist")
        .then(r => r.json())
        .then(data => {
            if (data.status === "ok") {
                document.getElementById("distance_label").innerText =
                    "Distance: " + data.response;
                setStatus("Distance measured");
            } else {
                setStatus("ERROR: " + data.message);
            }
        })
        .catch(err => {
            setStatus("ERROR: " + err);
        });
}