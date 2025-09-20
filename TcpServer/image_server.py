#!/usr/bin/env python3

import socket
import struct
import threading
import cv2
import numpy as np
import time
from flask import Flask, Response, request, jsonify

latest_frame = None
frame_timestamp = None  # timestamp of the last frame in seconds
lock = threading.Lock()
tcp_server_port = 5000
flask_port = 8000

esp_client = None  # store TCP connection to ESP32

def tcp_server(host="0.0.0.0", port=tcp_server_port):
    global latest_frame, frame_timestamp, esp_client

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((host, port))
    s.listen(1)
    print(f"TCP server listening on {host}:{port}")

    while True:  # outer loop: accept new clients forever
        print("Waiting for ESP32 connection...")
        try:
            conn, addr = s.accept()
            esp_client = conn
            print("Connected by", addr)
            conn.settimeout(1.0)  # optional: timeout to check for abrupt disconnect

            while True:  # inner loop: read frames from this client
                try:
                    # Read 4 bytes for frame length
                    data = conn.recv(4)
                    if not data or len(data) < 4:
                        print("Client disconnected")
                        break

                    frame_len = struct.unpack("I", data)[0]

                    # Read full frame
                    buf = b""
                    while len(buf) < frame_len:
                        packet = conn.recv(frame_len - len(buf))
                        if not packet:
                            print("Client disconnected during frame")
                            break
                        buf += packet

                    if len(buf) != frame_len:
                        break  # incomplete frame, assume disconnect

                    # Decode JPEG
                    npbuf = np.frombuffer(buf, dtype=np.uint8)
                    frame = cv2.imdecode(npbuf, cv2.IMREAD_COLOR)

                    # Store latest frame and timestamp
                    with lock:
                        latest_frame = frame
                        frame_timestamp = time.time()

                except socket.timeout:
                    continue
                except Exception as e:
                    print("TCP error:", e)
                    break

        except Exception as e:
            print("Accept error:", e)
        finally:
            if esp_client:
                esp_client.close()
            esp_client = None
            print("ESP32 disconnected, waiting for new connection...")


# Flask server
app = Flask(__name__)

def generate():
    global latest_frame
    while True:
        with lock:
            if latest_frame is not None:
                _, jpeg = cv2.imencode('.jpg', latest_frame)
                frame_bytes = jpeg.tobytes()
            else:
                continue

        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + frame_bytes + b'\r\n')


@app.route('/video_feed')
def video_feed():
    return Response(generate(),
                    mimetype='multipart/x-mixed-replace; boundary=frame')


@app.route('/last_frame_age')
def last_frame_age():
    """Return how old the last frame is in milliseconds"""
    with lock:
        if frame_timestamp is None:
            return jsonify({"status": "error", "message": "No frame received yet"}), 404
        age_ms = int((time.time() - frame_timestamp) * 1000)
        return jsonify({"status": "ok", "age_ms": age_ms}), 200


@app.route('/api')
def api_control():
    global esp_client
    cmd = request.args.get("cmd", "").lower()

    if not esp_client:
        return jsonify({"status": "error", "message": "ESP32 not connected"}), 400

    if cmd == "start":
        esp_client.sendall(b'start\n')
        return jsonify({"status": "ok", "message": "Streaming started"}), 200
    elif cmd == "stop":
        esp_client.sendall(b'stop\n')
        return jsonify({"status": "ok", "message": "Streaming stopped"}), 200
    else:
        return jsonify({"status": "error", "message": f"Unknown command '{cmd}'"}), 400


if __name__ == "__main__":
    threading.Thread(target=tcp_server, daemon=True).start()
    app.run(host="0.0.0.0", port=flask_port, debug=False, threaded=True)
