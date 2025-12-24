"""
Test script to interact with Unreal Engine via AdastreaDirector IPC
"""
import socket
import json
import sys

def test_unreal_connection():
    """Test connection to Unreal Engine IPC server"""
    print("Testing Unreal Engine interaction via Python...\n")
    
    try:
        # Connect to AdastreaDirector IPC Server
        print("Attempting to connect to 127.0.0.1:5555...")
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect(('127.0.0.1', 5555))
        
        print("✅ Connected to AdastreaDirector IPC Server!")
        
        # Send ping command (with correct protocol)
        request = {'type': 'ping', 'data': {}}
        print(f"\nSending: {request}")
        sock.sendall((json.dumps(request) + '\n').encode('utf-8'))
        
        # Receive response
        response = sock.recv(4096).decode('utf-8')
        result = json.loads(response)
        
        print(f"Received: {result}")
        print("\n✅ SUCCESS: Can interact with Unreal Engine via Python!")
        
        sock.close()
        return True
        
    except ConnectionRefusedError:
        print("❌ Connection refused")
        print("\nThe Unreal Editor is not currently running.")
        print("To enable Python interaction:")
        print("  1. Open Adastrea.uproject in Unreal Editor")
        print("  2. The AdastreaDirector plugin will start the IPC server")
        print("  3. Run this script again")
        return False
        
    except socket.timeout:
        print("❌ Connection timeout")
        print("Server not responding within 5 seconds")
        return False
        
    except Exception as e:
        print(f"❌ Error: {type(e).__name__}: {e}")
        return False

if __name__ == '__main__':
    success = test_unreal_connection()
    sys.exit(0 if success else 1)
