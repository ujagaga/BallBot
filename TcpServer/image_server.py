#!/usr/bin/env python3
#  image_server.py
import threading
import cv2
import time
from flask import Flask, Response, request, jsonify, render_template, flash
import config
import logging
from logging.handlers import RotatingFileHandler
import argparse
import os
import tcp_server
import json

UPLOAD_FOLDER = os.path.join(os.path.dirname(__file__), 'firmware')
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

parser = argparse.ArgumentParser()
parser.add_argument("--mode", default="development", help="Mode to run the server in", required=False)
args = parser.parse_args()

# In dev mode, the logs are printed in console. Any other mode prints logs in file.
execution_mode = args.mode

# Flask server
app = Flask(__name__)
app.config["SECRET_KEY"] = "9OLWxND4K4_any_random_string123123"
app.config.update(
    SESSION_COOKIE_SECURE=False,
    SESSION_COOKIE_HTTPONLY=True,
    SESSION_COOKIE_SAMESITE='Lax',
)

def setup_logger():
    logger_obj = logging.getLogger()
    # 👇 Set log level based on mode
    if execution_mode == "development":
        logger_obj.setLevel(logging.DEBUG)   # Show everything
    else:
        logger_obj.setLevel(logging.INFO)    # Only info, warning, error, critical

    if logger_obj.hasHandlers():
        logger_obj.handlers.clear()

    formatter = logging.Formatter(
        "[%(asctime)s] %(levelname)s [%(name)s] %(message)s",
        datefmt='%Y-%m-%dT%H:%M:%S'
    )

    if execution_mode == "development":
        console_handler = logging.StreamHandler()
        console_handler.setLevel(logging.DEBUG)  # Ensure handler shows debug
        console_handler.setFormatter(formatter)
        logger_obj.addHandler(console_handler)
    else:
        if config.DEBUG_TO_FILE:
            file_handler = RotatingFileHandler(
                os.path.join(os.path.dirname(__file__), 'app.log'),
                maxBytes=65535,
                backupCount=1
            )
            file_handler.setLevel(logging.INFO)  # Only log info and above
            file_handler.setFormatter(formatter)
            logger_obj.addHandler(file_handler)

    return logger_obj


def get_response(last_timestamp=None):
    start_time = time.time()
    while time.time() - start_time < config.RESPONSE_TIMEOUT:
        with tcp_server.lock:
            if tcp_server.latest_text is not None and (
                last_timestamp is None
                or (tcp_server.text_timestamp and tcp_server.text_timestamp > last_timestamp)
            ):
                message = tcp_server.latest_text
                tcp_server.latest_text = None
                return message
        time.sleep(0.05)
    return None



def generate():
    while True:
        frame_bytes = None

        with tcp_server.lock:
            if tcp_server.latest_frame is not None:
                frame = tcp_server.latest_frame.copy()
            else:
                frame = None

        if frame is not None:
            ok, jpeg = cv2.imencode(".jpg", frame)
            if ok:
                frame_bytes = jpeg.tobytes()

        if frame_bytes:
            yield (
                b"--frame\r\n"
                b"Content-Type: image/jpeg\r\n\r\n"
                + frame_bytes
                + b"\r\n"
            )

        time.sleep(0.05)


@app.route('/video_feed')
def video_feed():
    return Response(generate(), mimetype='multipart/x-mixed-replace; boundary=frame')


@app.route('/frame_info')
def frame_info():
    """Return how old the last frame is in milliseconds and the frame count"""
    with tcp_server.lock:
        if tcp_server.frame_timestamp is None:
            return jsonify({"status": "error", "message": "No frame received yet"}), 404
        age_ms = int((time.time() - tcp_server.frame_timestamp) * 1000)
        return jsonify({"status": "ok", "frame_age_ms": age_ms, "frame_count": tcp_server.frame_count}), 200


@app.route("/upload_fw", methods=["GET", "POST"])
def upload_fw():
    if request.method == "POST":
        if 'file' not in request.files:
            return "No file part", 400
        f = request.files['file']
        if f.filename == '':
            return "No selected file", 400
        if not f.filename.lower().endswith(".bin"):
            return "Only .bin files are allowed", 400

        save_path = os.path.join(UPLOAD_FOLDER, "firmware.bin")
        f.save(save_path)
        flash(f"Firmware uploaded successfully!")

    return render_template("fwupload.html" )


@app.route("/start_update", methods=["POST"])
def start_update():
    firmware_path = os.path.join(UPLOAD_FOLDER, "firmware.bin")
    if not os.path.exists(firmware_path):
        return "No firmware uploaded", 400

    if not tcp_server.esp_client:
        return "ESP32 not connected", 400

    def stream_firmware():
        # Read and send the firmware in chunks
        with open(firmware_path, "rb") as f:
            while chunk := f.read(512):
                tcp_server.esp_client.sendall(chunk)
                yield chunk  # optional: can be used to stream back to browser for progress

    return Response(stream_firmware(), mimetype='application/octet-stream')


@app.route("/update_progress")
def update_progress():
    if not tcp_server.esp_client:
        return "ESP32 not connected", 400

    def event_stream():
        while True:
            if tcp_server.esp_client and tcp_server.esp_client.connected():
                while tcp_server.esp_client.available():
                    line = tcp_server.esp_client.read(1024).decode(errors='ignore')
                    for l in line.splitlines():
                        if l.startswith("PROGRESS:") or l in ("DONE", "ERR update"):
                            yield f"data: {l}\n\n"
            time.sleep(0.05)

    return Response(event_stream(), mimetype="text/event-stream")


@app.route('/api')
def api_control():
    cmd = request.args.get("cmd")
    value = request.args.get("val")
    speed = request.args.get("speed")
    keep_dir = request.args.get("keepDir")
    try:
        increment_val = int(value)
    except ValueError:
        increment_val = 0

    if cmd is None:
        return render_template("api.html")

    cmd = cmd.lower()
    value = (value or "").lower()

    if not tcp_server.esp_client:
        return jsonify({"status": "error", "message": "ESP32 not connected"}), 400

    # Clear old response
    with tcp_server.lock:
        last_ts = tcp_server.text_timestamp
        tcp_server.latest_text = None
        tcp_server.text_timestamp = None

    payload = None

    # ---------- STREAM ----------
    if cmd == "start":
        payload = {"cmd": "stream", "enable": True}
    elif cmd == "stop":
        payload = {"cmd": "stream", "enable": False}

    # ---------- MOTOR ----------
    elif cmd == "distance":
        speed_val = int(speed) if speed else 600
        keep_dir_val = keep_dir.lower() in ("true", "1", "yes") if keep_dir else True
        dist_val = int(value) if value else 0

        payload = {
            "cmd": "motor",
            "distance": dist_val,
            "speed": speed_val,
            "keepDir": keep_dir_val
        }

    # ---------- SERVO ----------
    elif cmd == "servoClaw":
        payload = {"cmd": "servoClawIncrement", "angle": int(increment_val)}
    elif cmd == "servoArm":
        payload = {"cmd": "servoArmIncrement",  "angle": int(increment_val)}
    elif cmd == "servoSteer":
        payload = {"cmd": "servoSteerIncrement", "angle": int(increment_val)}

    # ---------- LIGHT ----------
    elif cmd == "light":
        payload = {"cmd": "light", "value": int(value)}

    else:
        return jsonify({
            "status": "error",
            "message": f"Unknown command '{cmd}'"
        }), 400

    # ---------- SEND JSON ----------
    msg = json.dumps(payload) + "\n"
    tcp_server.esp_client.sendall(msg.encode("utf-8"))

    esp32_response = get_response(last_ts)

    return jsonify({
        "status": "ok",
        "sent": payload,
        "response": esp32_response
    }), 200



@app.route("/")
def index():
    return render_template("index.html")


@app.route("/next_frame")
def next_frame():
    """
    Block until the next frame is received from ESP32,
    then return it as a single JPEG response.
    """
    timeout = config.RESPONSE_TIMEOUT  # seconds

    with tcp_server.frame_condition:
        start_count = tcp_server.frame_count
        start_time = time.time()

        while tcp_server.frame_count == start_count:
            remaining = timeout - (time.time() - start_time)
            if remaining <= 0:
                return jsonify({
                    "status": "timeout",
                    "message": "No new frame received"
                }), 504

            tcp_server.frame_condition.wait(timeout=remaining)

        frame = tcp_server.latest_frame.copy()

    ok, jpeg = cv2.imencode(".jpg", frame)
    if not ok:
        return jsonify({
            "status": "error",
            "message": "Failed to encode frame"
        }), 500

    return Response(
        jpeg.tobytes(),
        mimetype="image/jpeg",
        headers={
            "Cache-Control": "no-store",
            "X-Frame-Count": str(tcp_server.frame_count)
        }
    )



if __name__ == "__main__":
    threading.Thread(target=tcp_server.start_server, daemon=True).start()
    setup_logger()
    app.run(host="0.0.0.0", port=config.FLASK_PORT, debug=False, threaded=True)
