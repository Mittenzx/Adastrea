#!/usr/bin/env python3
"""
Execute FixPIE Script via UnrealMCP TCP
========================================

Runs the FixPIE.py script remotely in Unreal Engine via UnrealMCP TCP connection.
"""

import socket
import json

MCP_HOST = "127.0.0.1"
MCP_PORT = 55557

def send_mcp_command(command_type, params):
    """Send command to UnrealMCP and get response"""
    command = {
        "type": command_type,
        "params": params
    }
    
    command_json = json.dumps(command)
    
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10.0)
        sock.connect((MCP_HOST, MCP_PORT))
        
        # Send command
        sock.sendall(command_json.encode('utf-8'))
        
        # Receive response
        response_data = sock.recv(8192)
        response = response_data.decode('utf-8')
        
        sock.close()
        
        # Parse JSON response
        return json.loads(response)
        
    except Exception as e:
        print(f"Error: {e}")
        return None

# Read the FixPIE.py script
with open(r"c:\Unreal Projects\Adastrea\FixPIE.py", "r", encoding="utf-8") as f:
    fix_pie_code = f.read()

print("=" * 60)
print("  Executing FixPIE.py via UnrealMCP")
print("=" * 60)
print()

# Since UnrealMCP doesn't have execute_python command, we need to ask the user
# to run it manually in Unreal Editor's Python console

print("❌ UnrealMCP doesn't support Python code execution directly.")
print()
print("UnrealMCP is designed for:")
print("  • Spawning actors")
print("  • Creating/modifying Blueprints")
print("  • Editor viewport manipulation")
print("  • Taking screenshots")
print("  • Project settings")
print()
print("For Python script execution, please use Unreal Editor's Python console:")
print()
print("📋 Copy and paste this into Unreal Editor → Output Log → Python:")
print("-" * 60)
print('exec(open(r"c:\\Unreal Projects\\Adastrea\\FixPIE.py").read())')
print("-" * 60)
print()
print("Or I can help add Python execution support to UnrealMCP plugin!")
