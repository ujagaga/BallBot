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

def start_server(host="0.0.0.0", port=config.TCP_SERVER_PORT):
    """
    TCP server for ESP32 CAM.
    Handles one client at a time and allows instant reconnection.
    """
    global latest_frame, frame_timestamp, esp_client, latest_text, text_timestamp, frame_count

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((host, port))
    s.listen(1)
    logger.info(f"TCP server listening on {host}:{port}")

    while True:
        try:
            logger.debug("Waiting for ESP32 connection...")
            conn, addr = s.accept()
            logger.info(f"Incoming connection from {addr}")

            # === Close previous client immediately ===
            with lock:
                if esp_client:
                    try:
                        logger.info("Closing previous ESP32 connection")
                        esp_client.shutdown(socket.SHUT_RDWR)
                    except Exception:
                        pass
                    esp_client.close()
                    esp_client = None

                esp_client = conn

            conn.settimeout(3.0)  # Detect dead client quickly

            logger.info(f"ESP32 connected: {addr}")

            while True:
                try:
                    # --- 1 byte: message type ---
                    msg_type_bytes = conn.recv(1)
                    if not msg_type_bytes:
                        logger.info("Client disconnected (msg_type)")
                        break
                    msg_type = msg_type_bytes[0]

                    # --- 4 bytes: payload length ---
                    length_bytes = recv_all(conn, 4)
                    if not length_bytes:
                        logger.info("Client disconnected (length)")
                        break
                    payload_len = struct.unpack("I", length_bytes)[0]

                    # --- Read payload ---
                    payload = recv_all(conn, payload_len)
                    if payload is None:
                        logger.info("Client disconnected (payload)")
                        break

                    # --- Process messages ---
                    if msg_type == 0:
                        # Image frame
                        npbuf = np.frombuffer(payload, dtype=np.uint8)
                        frame = cv2.imdecode(npbuf, cv2.IMREAD_COLOR)
                        if frame is not None:
                            with lock:
                                latest_frame = frame
                                frame_timestamp = time.time()
                                frame_count += 1
                                if frame_count > 1000:
                                    frame_count = 0
                        else:
                            logger.warning("Failed to decode frame")
                    elif msg_type == 1:
                        # Text message
                        message = payload.decode("utf-8", errors="ignore")
                        with lock:
                            latest_text = message
                            text_timestamp = time.time()
                    elif msg_type == 2:
                        # Debug
                        message = payload.decode("utf-8", errors="ignore")
                        logger.info(f"DBG: {message}")
                    else:
                        logger.warning(f"Unknown msg_type: {msg_type}")

                except socket.timeout:
                    continue  # allows next iteration to check for disconnect
                except Exception as e:
                    logger.exception(f"Error while receiving data: {e}")
                    break

        except Exception as exc:
            logger.exception(f"Server error: {exc}")
        finally:
            # Cleanup connection
            with lock:
                if esp_client:
                    try:
                        esp_client.shutdown(socket.SHUT_RDWR)
                    except Exception:
                        pass
                    esp_client.close()
                    esp_client = None
            logger.info("ESP32 disconnected, ready for new connection")
