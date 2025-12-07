#!/usr/bin/env python3
"""Simple ping test for UE IPC server"""

import socket
import json
import time

def simple_ping(port=5555):
    """Send a simple ping and wait for response."""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect(('127.0.0.1', port))
        print(f"✓ Connected to localhost:{port}")
        
        # Send ping
        request = json.dumps({'type': 'ping', 'data': ''}) + '\n'
        print(f"Sending: {request.strip()}")
        sock.sendall(request.encode('utf-8'))
        
        # Wait a bit
        time.sleep(0.5)
        
        # Try to receive
        response = sock.recv(4096)
        if response:
            print(f"✓ Received: {response.decode('utf-8')}")
        else:
            print("✗ No response received")
        
        sock.close()
        return True
    except Exception as e:
        print(f"✗ Error: {e}")
        import traceback
        traceback.print_exc()
        return False

if __name__ == '__main__':
    simple_ping()
