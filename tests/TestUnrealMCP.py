#!/usr/bin/env python3
"""
Test UnrealMCP TCP Connection
==============================

Tests the TCP connection to UnrealMCP server running in Unreal Editor.
Port: 55557 (localhost)
"""

import socket
import json
import sys

MCP_HOST = "127.0.0.1"
MCP_PORT = 55557

def send_command(command_type, params=None):
    """Send a command to Unreal MCP server"""
    
    # Create command JSON (UnrealMCP expects "type" not "command")
    command = {
        "type": command_type,
        "params": params or {}
    }
    
    command_json = json.dumps(command)
    
    print(f"Connecting to Unreal MCP at {MCP_HOST}:{MCP_PORT}...")
    
    try:
        # Create socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5.0)  # 5 second timeout
        
        # Connect
        sock.connect((MCP_HOST, MCP_PORT))
        print("✓ Connected!")
        
        # Send command
        print(f"Sending command: {command_type}")
        sock.sendall(command_json.encode('utf-8'))
        
        # Receive response
        response_data = sock.recv(4096)
        response = response_data.decode('utf-8')
        
        print("\n" + "=" * 60)
        print("RESPONSE:")
        print("=" * 60)
        print(response)
        print("=" * 60)
        
        sock.close()
        return True
        
    except socket.timeout:
        print("✗ Connection timed out - is Unreal Editor running?")
        return False
    except ConnectionRefusedError:
        print("✗ Connection refused - UnrealMCP plugin may not be loaded")
        print("  Check Unreal Editor Output Log for 'UnrealMCPBridge: Initializing'")
        return False
    except Exception as e:
        print(f"✗ Error: {e}")
        return False

def test_connection():
    """Test basic connection"""
    print("=" * 60)
    print("  UnrealMCP TCP Connection Test")
    print("=" * 60)
    print()
    
    # Test 1: Ping command
    print("Test 1: Ping UnrealMCP server")
    print("-" * 60)
    success = send_command("ping", {})
    
    if not success:
        print("\n⚠️  Connection failed. Make sure:")
        print("  1. Unreal Editor is running with Adastrea project open")
        print("  2. UnrealMCP plugin is enabled and loaded")
        print("  3. Check Output Log for 'UnrealMCPBridge: Initializing'")
        return False
    
    print("\n✅ UnrealMCP TCP connection working!")
    print("    You can now send Python commands remotely.")
    return True

if __name__ == "__main__":
    test_connection()
