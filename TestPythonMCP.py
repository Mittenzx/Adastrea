#!/usr/bin/env python3
"""
Test Python Execution via UnrealMCP
====================================

Tests the new execute_python command via UnrealMCP TCP.
"""

import socket
import json

MCP_HOST = "127.0.0.1"
MCP_PORT = 55557

def send_python_command(python_code):
    """Send Python code to Unreal via MCP"""
    command = {
        "type": "execute_python",
        "params": {
            "code": python_code
        }
    }
    
    command_json = json.dumps(command)
    
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10.0)
        sock.connect((MCP_HOST, MCP_PORT))
        
        # Send
        sock.sendall(command_json.encode('utf-8'))
        
        # Receive
        response_data = sock.recv(8192)
        response = response_data.decode('utf-8')
        
        sock.close()
        
        return json.loads(response)
        
    except Exception as e:
        print(f"❌ Error: {e}")
        return None

print("=" * 60)
print("  Testing Python Execution via UnrealMCP")
print("=" * 60)
print()

# Test 1: Simple print
print("Test 1: Execute simple Python print")
print("-" * 60)
result = send_python_command("import unreal; unreal.log('Hello from MCP!')")
if result:
    print(json.dumps(result, indent=2))
print()

# Test 2: Execute FixPIE.py
print("Test 2: Execute FixPIE.py script")
print("-" * 60)
with open(r"c:\Unreal Projects\Adastrea\FixPIE.py", "r", encoding="utf-8") as f:
    fix_pie_code = f.read()

result = send_python_command(fix_pie_code)
if result:
    print(json.dumps(result, indent=2))
    if result.get("status") == "success":
        print("\n✅ FixPIE.py executed successfully via UnrealMCP!")
        print("   Check Unreal Editor Output Log for results")
print()

print("=" * 60)
