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

# Globals
latest_frame = None
frame_timestamp = None
frame_count = 0
latest_text = None
text_timestamp = None
esp_client = None
lock = threading.Lock()

logger = logging.getLogger(__name__)


def recv_all(conn, length):
    """Read exactly length bytes from socket, or return None if disconnected"""
    buf = b""
    while len(buf) < length:
        try:
            packet = conn.recv(length - len(buf))
            if not packet:
                return None
            buf += packet
        except socket.timeout:
            continue
        except Exception:
            return None
    return buf


def handle_client(conn, addr):
    """Handle a single ESP32 client in its own thread"""
    global latest_frame, frame_timestamp, frame_count, latest_text, text_timestamp, esp_client

    logger.info(f"ESP32 connected: {addr}")
    conn.settimeout(3.0)

    with lock:
        # Close previous client if exists
        if esp_client and esp_client != conn:
            try:
                esp_client.shutdown(socket.SHUT_RDWR)
            except Exception:
                pass
            esp_client.close()
        esp_client = conn

    try:
        while True:
            # 1 byte: message type
            msg_type_bytes = conn.recv(1)
            if not msg_type_bytes:
                logger.info("Client disconnected (msg_type)")
                break
            msg_type = msg_type_bytes[0]

            # 4 bytes: payload length
            length_bytes = recv_all(conn, 4)
            if not length_bytes:
                logger.info("Client disconnected (length)")
                break
            payload_len = struct.unpack("I", length_bytes)[0]

            # payload
            payload = recv_all(conn, payload_len)
            if payload is None:
                logger.info("Client disconnected (payload)")
                break

            # --- process messages ---
            if msg_type == 0:  # image frame
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

            elif msg_type == 1:  # text message
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
        exc_type, exc_obj, exc_tb = sys.exc_info()
        logger.exception(f"Client error at line {exc_tb.tb_lineno}: {e}")

    finally:
        with lock:
            if esp_client == conn:
                esp_client = None
        try:
            conn.shutdown(socket.SHUT_RDWR)
        except Exception:
            pass
        conn.close()
        logger.info(f"ESP32 disconnected: {addr}")


def start_server(host="0.0.0.0", port=config.TCP_SERVER_PORT):
    """Main TCP server loop: accepts clients and spawns threads for each"""
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((host, port))
    s.listen(5)
    logger.info(f"TCP server listening on {host}:{port}")

    while True:
        try:
            conn, addr = s.accept()
            logger.info(f"Incoming connection from {addr}")

            # Start a new thread for this client
            threading.Thread(target=handle_client, args=(conn, addr), daemon=True).start()

        except Exception as e:
            exc_type, exc_obj, exc_tb = sys.exc_info()
            logger.exception(f"Server accept error at line {exc_tb.tb_lineno}: {e}")
