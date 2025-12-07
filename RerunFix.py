import socket
import json

def send_python_command(code):
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)
        sock.connect(("127.0.0.1", 55557))
        
        command = {
            "type": "execute_python",
            "params": {"code": code}
        }
        
        sock.sendall(json.dumps(command).encode('utf-8'))
        response = sock.recv(8192).decode('utf-8')
        sock.close()
        
        return json.loads(response)
    except Exception as e:
        print(f"Error: {e}")
        return None

# Read and execute FixPIE.py
with open(r"c:\Unreal Projects\Adastrea\FixPIE.py", "r") as f:
    fix_script = f.read()

print("📡 Sending FixPIE.py to Unreal Engine...")
result = send_python_command(fix_script)

if result and result.get("status") == "success":
    print("✅ FixPIE.py executed successfully!")
    print("Check Unreal Editor Output Log for detailed results")
else:
    print(f"❌ Execution failed: {result}")
