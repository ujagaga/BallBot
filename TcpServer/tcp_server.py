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
    global latest_frame, frame_timestamp, esp_client, latest_text, text_timestamp, frame_count

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((host, port))
    s.listen(1)
    logger.debug(f"TCP server listening on {host}:{port}")

    while True:
        logger.debug("Waiting for ESP32 connection...")
        try:
            conn, addr = s.accept()
            esp_client = conn
            logger.info(f"Connected by {addr}")
            conn.settimeout(3.0)  # helps detect abrupt disconnects

            while True:
                try:
                    # === Read 1 byte: message type ===
                    msg_type = conn.recv(1)
                    if not msg_type:
                        logger.debug("Client disconnected")
                        break
                    msg_type = msg_type[0]

                    # === Read 4 bytes: payload length ===
                    length_bytes = recv_all(conn, 4)
                    if not length_bytes:
                        logger.debug("Client disconnected (length)")
                        break
                    payload_len = struct.unpack("I", length_bytes)[0]

                    # === Read payload ===
                    payload = recv_all(conn, payload_len)
                    if payload is None:
                        logger.debug("Client disconnected (payload)")
                        break

                    if msg_type == 0:
                        # --- Image frame ---
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
                        # --- Text message ---
                        message = payload.decode("utf-8", errors="ignore")
                        # logger.info(f"Received text: {message}")
                        with lock:
                            latest_text = message
                            text_timestamp = time.time()
                    elif msg_type == 2:
                        # --- Debug message ---
                        message = payload.decode("utf-8", errors="ignore")
                        logger.info(f"DBG: {message}")
                    else:
                        logger.warning(f"Unknown msg_type: {msg_type}")

                except socket.timeout:
                    continue
                except Exception as e:
                    exc_type, exc_obj, exc_tb = sys.exc_info()
                    logger.exception(f"TCP error at line {exc_tb.tb_lineno}: {e}")
                    break

        except Exception as exc:
            exc_type, exc_obj, exc_tb = sys.exc_info()
            logger.exception(f"Error at line {exc_tb.tb_lineno}: {exc}")
        finally:
            if esp_client:
                esp_client.close()
            esp_client = None
            logger.debug("ESP32 disconnected, waiting for new connection...")
