import socket
import json

def test_connection():
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect(("127.0.0.1", 55557))
        
        command = {"type": "ping", "params": {}}
        sock.sendall(json.dumps(command).encode('utf-8'))
        
        response = sock.recv(4096).decode('utf-8')
        sock.close()
        
        print("✅ UnrealMCP server is running!")
        print(f"Response: {response}")
        return True
    except ConnectionRefusedError:
        print("❌ Cannot connect - UnrealMCP server not running")
        print("Make sure Unreal Editor is open with the Adastrea project loaded")
        return False
    except socket.timeout:
        print("⏱️ Connection timeout - server may be busy")
        return False
    except Exception as e:
        print(f"❌ Error: {e}")
        return False

if __name__ == "__main__":
    test_connection()
