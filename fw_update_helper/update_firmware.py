#!/usr/bin/env python3

import argparse
import http.server
import socket
import socketserver
import threading
import requests
import sys
import os
import time

ESP32_OTA_URL = "http://192.168.4.1/api/ota"
SERVER_PORT = 8000


# ---------------- IP Detection ----------------

def get_local_ap_ip():
    """
    Detect local IP in 192.168.4.0/24
    """
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(("192.168.4.1", 80))
        ip = s.getsockname()[0]
        if ip.startswith("192.168.4."):
            return ip
    except Exception:
        pass
    finally:
        s.close()

    raise RuntimeError("No local IP found in 192.168.4.x subnet")


# ---------------- HTTP Server ----------------

class OTARequestHandler(http.server.BaseHTTPRequestHandler):
    firmware_path = None
    server_instance = None  # Placeholder for the server object

    def do_GET(self):
        if self.path == "/firmware":
            self.serve_firmware()
        else:
            self.send_error(404)

    def do_POST(self):
        if self.path == "/log":
            self.receive_log()
        else:
            self.send_error(404)

    def trigger_shutdown(self):
        # shutdown() must be called from a different thread than serve_forever()
        # Since the handler runs in the server thread, we use a small timer or
        # just call it directly if the server implementation allows.
        print("[HTTP] Initiating server shutdown...")
        threading.Thread(target=self.server_instance.shutdown).start()

    def serve_firmware(self):
        try:
            file_size = os.path.getsize(self.firmware_path)
            self.send_response(200)
            self.send_header("Content-Type", "application/octet-stream")
            self.send_header("Content-Length", str(file_size))
            self.end_headers()

            with open(self.firmware_path, "rb") as f:
                while True:
                    chunk = f.read(1024)
                    if not chunk:
                        break
                    try:
                        self.wfile.write(chunk)
                    except (BrokenPipeError, ConnectionResetError):
                        print("[HTTP] Firmware transfer completed (client closed)")
                        self.trigger_shutdown() # Shutdown here
                        return

            print("[HTTP] Firmware fully sent")
            self.trigger_shutdown() # Shutdown here

        except Exception as e:
            print("[HTTP] Firmware serve failed:", e)

    def receive_log(self):
        length = int(self.headers.get("Content-Length", 0))
        msg = self.rfile.read(length).decode(errors="ignore")

        print("[ESP32]", msg.strip())

        self.send_response(200)
        self.end_headers()


def start_http_server(bind_ip, firmware_path):
    OTARequestHandler.firmware_path = firmware_path
    # Create the server and assign it to the class so the handler can see it
    OTARequestHandler.server_instance = socketserver.TCPServer((bind_ip, SERVER_PORT), OTARequestHandler)
    print(f"[HTTP] Server running at http://{bind_ip}:{SERVER_PORT}")
    OTARequestHandler.server_instance.serve_forever()


# ---------------- OTA Trigger ----------------

def trigger_ota(host_ip):
    firmware_url = f"http://{host_ip}:{SERVER_PORT}/firmware"
    log_url = f"http://{host_ip}:{SERVER_PORT}/log"

    print("[OTA] Triggering OTA")
    print("[OTA] Firmware URL:", firmware_url)
    print("[OTA] Log URL:", log_url)

    response = requests.post(
        ESP32_OTA_URL,
        data=firmware_url,
        headers={
            "X-Log-Callback": log_url
        },
        timeout=5
    )

    print("[OTA] ESP32 response:", response.text)


# ---------------- Main ----------------

def main():
    parser = argparse.ArgumentParser(description="ESP32 OTA push tool")
    parser.add_argument("firmware", help="Path to firmware binary")
    args = parser.parse_args()

    firmware_path = os.path.abspath(args.firmware)

    if not os.path.isfile(firmware_path):
        print("Firmware file not found:", firmware_path)
        sys.exit(1)

    try:
        host_ip = get_local_ap_ip()
    except RuntimeError as e:
        print(e)
        sys.exit(1)

    print("[INFO] Host IP:", host_ip)
    print("[INFO] Firmware:", firmware_path)

    server_thread = threading.Thread(
        target=start_http_server,
        args=(host_ip, firmware_path),
        daemon=True
    )
    server_thread.start()
    time.sleep(1)

    try:
        trigger_ota(host_ip)
    except Exception as e:
        print("[OTA] Failed:", e)
        sys.exit(1)

    print("[INFO] Waiting for transfer to complete...")

    # Instead of while True, wait for the server thread to finish
    while server_thread.is_alive():
        time.sleep(0.5)

    print("[INFO] Server closed. Exiting.")

if __name__ == "__main__":
    main()
