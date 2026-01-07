# tcp_server.py
import socket
import struct
import threading
import cv2
import numpy as np
import time
import config
import logging
import sys

latest_frame = None
frame_timestamp = None
frame_count = 0
latest_text = None
text_timestamp = None
esp_client = None
lock = threading.Lock()

logger = logging.getLogger(__name__)

def recv_all(conn, length):
    """Read exactly length bytes from socket or return None on disconnect."""
    buf = b""
    while len(buf) < length:
        packet = conn.recv(length - len(buf))
        if not packet:
            return None
        buf += packet
    return buf


def handle_client(conn, addr):
    global latest_frame, frame_timestamp, esp_client, latest_text, text_timestamp, frame_count

    logger.info(f"ESP32 connected: {addr}")
    conn.settimeout(3.0)

    with lock:
        esp_client = conn

    try:
        while True:
            # === read 1 byte msg_type ===
            msg_type_bytes = conn.recv(1)
            if not msg_type_bytes:
                logger.info("Client disconnected")
                break
            msg_type = msg_type_bytes[0]

            # === read payload length ===
            length_bytes = recv_all(conn, 4)
            if not length_bytes:
                logger.info("Client disconnected (length)")
                break
            payload_len = struct.unpack("I", length_bytes)[0]

            # === read payload ===
            payload = recv_all(conn, payload_len)
            if payload is None:
                logger.info("Client disconnected (payload)")
                break

            # process payload
            if msg_type == 0:  # frame
                npbuf = np.frombuffer(payload, dtype=np.uint8)
                frame = cv2.imdecode(npbuf, cv2.IMREAD_COLOR)
                if frame is not None:
                    with lock:
                        latest_frame = frame
                        frame_timestamp = time.time()
                        frame_count += 1
            elif msg_type == 1:  # text
                message = payload.decode("utf-8", errors="ignore")
                with lock:
                    latest_text = message
                    text_timestamp = time.time()
            elif msg_type == 2:  # debug
                message = payload.decode("utf-8", errors="ignore")
                logger.info(f"DBG: {message}")
            else:
                logger.warning(f"Unknown msg_type: {msg_type}")

    except Exception as e:
        logger.exception(f"Client error: {e}")
    finally:
        with lock:
            if esp_client == conn:
                esp_client = None
        try:
            conn.shutdown(socket.SHUT_RDWR)
        except Exception:
            pass
        conn.close()
        logger.info("ESP32 disconnected")



def start_server(host="0.0.0.0", port=config.TCP_SERVER_PORT):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((host, port))
    s.listen(5)
    logger.info(f"TCP server listening on {host}:{port}")

    while True:
        try:
            conn, addr = s.accept()
            logger.info(f"Incoming connection from {addr}")

            # Spawn a thread to handle this client
            threading.Thread(target=handle_client, args=(conn, addr), daemon=True).start()

        except Exception as e:
            logger.exception(f"Server error: {e}")
