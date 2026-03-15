import socket
import threading
import time
import sys

# This is a Lightweight UDP Mock Server for the TDD Phase (Before Godot's ENet is fully injected)
# Its purpose is to simulate the latency environment. Test clients can send "REQUEST_ABILITY_X"
# and the server will reply with "AUTHORIZE" or "REJECT" based on the logic profile.


class MPTestServer:
    def __init__(self, host="127.0.0.1", port=7777):
        self.host = host
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind((self.host, self.port))
        self.running = False
        self.clients = set()
        print(f"[MP SERVER MOCK] Bound to UDP {self.host}:{self.port}")

    def handle_packet(self, data, addr):
        msg = data.decode("utf-8").strip()
        print(f"[MP SERVER MOCK] Received from {addr}: {msg}")

        # Simple simulated routing for Ability System operations
        if msg.startswith("PING"):
            self.sock.sendto(b"PONG", addr)
        elif msg.startswith("ACTIVATE_ABILITY"):
            # Mock Server rejects ability activation randomly (simulating state mismatch or cheat)
            import random

            if (
                random.random() < 0.1
            ):  # 10% false prediction / reject rate for test simulation
                print(f"[MP SERVER MOCK] REJECTING activation from {addr}")
                self.sock.sendto(b"SYNC_REJECTED", addr)
            else:
                self.sock.sendto(b"SYNC_APPROVED", addr)
        else:
            self.sock.sendto(b"ACK", addr)

    def listen(self):
        self.running = True
        print("[MP SERVER MOCK] Listening for test clients...")
        self.sock.settimeout(1.0)  # Enable clean shutdown

        while self.running:
            try:
                data, addr = self.sock.recvfrom(1024)
                self.clients.add(addr)
                self.handle_packet(data, addr)
            except socket.timeout:
                continue
            except KeyboardInterrupt:
                self.stop()
            except Exception as e:
                print(f"[MP SERVER MOCK] Error: {e}")

    def stop(self):
        self.running = False
        self.sock.close()
        print("[MP SERVER MOCK] Server Shutdown.")


if __name__ == "__main__":
    server = MPTestServer()
    # If passed a duration argument, auto shutdown
    duration = 0
    if len(sys.argv) > 1:
        duration = int(sys.argv[1])

    t = threading.Thread(target=server.listen)
    t.start()

    if duration > 0:
        print(f"[MP SERVER MOCK] Running for {duration} seconds before auto-closing...")
        time.sleep(duration)
        server.stop()
        t.join()
    else:
        try:
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            server.stop()
            t.join()
