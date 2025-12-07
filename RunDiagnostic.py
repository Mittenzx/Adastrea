#!/usr/bin/env python3
"""
Run DiagnosePIE via UnrealMCP
"""
import socket
import json

def send_python_command(python_code):
    command = {"type": "execute_python", "params": {"code": python_code}}
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(10.0)
    sock.connect(("127.0.0.1", 55557))
    sock.sendall(json.dumps(command).encode('utf-8'))
    response = sock.recv(8192).decode('utf-8')
    sock.close()
    return json.loads(response)

# Run diagnostic
with open(r"c:\Unreal Projects\Adastrea\DiagnosePIE.py", "r", encoding="utf-8") as f:
    code = f.read()

print("Running PIE diagnostic via UnrealMCP...")
print("Check Unreal Editor Output Log for results!")
print("-" * 60)
result = send_python_command(code)
print(json.dumps(result, indent=2))
