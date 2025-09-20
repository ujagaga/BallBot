#!/usr/bin/env python3

import threading
import cv2
import time
from flask import Flask, Response, request, jsonify
import config
import logging
from logging.handlers import RotatingFileHandler
import argparse
import os
import tcp_server

parser = argparse.ArgumentParser()
parser.add_argument("--mode", default="development", help="Mode to run the server in", required=False)
args = parser.parse_args()

# In dev mode, the logs are printed in console. Any other mode prints logs in file.
execution_mode = args.mode

# Flask server
app = Flask(__name__)

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
        "[%(asctime)s] %(levelname)s [%(name)s.%(funcName)s:%(lineno)d] %(message)s",
        datefmt='%Y-%m-%dT%H:%M:%S'
    )

    if execution_mode == "development":
        console_handler = logging.StreamHandler()
        console_handler.setLevel(logging.DEBUG)  # Ensure handler shows debug
        console_handler.setFormatter(formatter)
        logger_obj.addHandler(console_handler)
    else:
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
        with tcp_server.lock:
            if tcp_server.latest_frame is not None:
                _, jpeg = cv2.imencode('.jpg', tcp_server.latest_frame)
                frame_bytes = jpeg.tobytes()
            else:
                continue

        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + frame_bytes + b'\r\n')


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


@app.route('/api')
def api_control():
    cmd = request.args.get("cmd", "").lower()
    value = request.args.get("val", "1").lower()

    if not tcp_server.esp_client:
        return jsonify({"status": "error", "message": "ESP32 not connected"}), 400

    # Clear old response
    with tcp_server.lock:
        last_ts = tcp_server.text_timestamp
        tcp_server.latest_text = None
        tcp_server.text_timestamp = None

    if cmd == "start":
        tcp_server.esp_client.sendall(b'start\n')
        # message = "Streaming started"
    elif cmd == "stop":
        tcp_server.esp_client.sendall(b'stop\n')
        # message = "Streaming stopped"
    elif cmd == "forward":
        tcp_server.esp_client.sendall(b'fwd\n')
        # message = "Streaming Set direction to forward"
    elif cmd == "reverse":
        tcp_server.esp_client.sendall(b'rev\n')
        # message = "Set direction to reverse"
    elif cmd == "speed":
        tcp_server.esp_client.sendall(f"speed:{value}".encode())
        # message = f"Set speed to: {value}"
    elif cmd == "go":
        # Make sure to send "cmd=go" after setting speed and direction
        tcp_server.esp_client.sendall(f"go:{value}".encode())
        # message = f"Moving for {value} ms"
    elif cmd == "motor1":
        tcp_server.esp_client.sendall(f"m1:{value}".encode())
        # message = f"Moving motor 1 to angle:{value}"
    elif cmd == "motor2":
        tcp_server.esp_client.sendall(f"m2:{value}".encode())
        # message = f"Moving motor 2 to angle:{value}"
    elif cmd == "motor3":
        tcp_server.esp_client.sendall(f"m3:{value}".encode())
        # message = f"Moving motor 3 to angle:{value}"
    elif cmd == "dist":
        tcp_server.esp_client.sendall(b"dist")
        # message = "Measure distance"
    elif cmd == "img_time":
        tcp_server.esp_client.sendall(f"imt:{value}".encode())
        # message = f"Set image capture time in millis to:{value}"
    else:
        return jsonify({"status": "error", "message": f"Unknown command '{cmd}'"}), 400

    esp32_response = get_response(last_ts)
    return jsonify({"status": "ok", "response": f"{esp32_response}"}), 200


if __name__ == "__main__":
    threading.Thread(target=tcp_server.start_server, daemon=True).start()
    setup_logger()
    app.run(host="0.0.0.0", port=config.FLASK_PORT, debug=False, threaded=True)
