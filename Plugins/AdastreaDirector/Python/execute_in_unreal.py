#!/usr/bin/env python3
"""
Execute Python code in Unreal Engine via IPC

This script sends Python code to Unreal Engine for execution
through the AdastreaDirector IPC server.

Usage:
    python execute_in_unreal.py --file AutoConfigurePIE.py
    python execute_in_unreal.py --code "print('Hello from UE')"
"""

import socket
import json
import sys
import argparse
from pathlib import Path


def send_python_command(code: str, port: int = 5555, timeout: int = 30):
    """
    Send Python code to Unreal Engine for execution.
    
    Args:
        code: Python code to execute
        port: IPC server port
        timeout: Socket timeout in seconds
        
    Returns:
        Response dictionary from UE
    """
    print(f"Connecting to Unreal Engine IPC server on localhost:{port}...")
    
    try:
        # Create socket and connect
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(timeout)
        sock.connect(('127.0.0.1', port))
        print("✓ Connected to Unreal Engine\n")
        
        # Send execute_python command
        request = {
            'type': 'execute_python',
            'data': code
        }
        
        print("Sending Python code to Unreal Engine...")
        sock.sendall((json.dumps(request) + '\n').encode('utf-8'))
        
        # Receive response
        response_data = b''
        while True:
            chunk = sock.recv(4096)
            if not chunk:
                break
            response_data += chunk
            if b'\n' in chunk:
                break
        
        response = json.loads(response_data.decode('utf-8').strip())
        sock.close()
        
        return response
        
    except ConnectionRefusedError:
        print(f"✗ Failed to connect to Unreal Engine IPC server on port {port}")
        print("\nMake sure:")
        print("1. Unreal Editor is running")
        print("2. AdastreaDirector plugin is loaded")
        print("3. IPC server is active (check Output Log)")
        sys.exit(1)
    except socket.timeout:
        print(f"✗ Request timed out after {timeout} seconds")
        sys.exit(1)
    except Exception as e:
        print(f"✗ Error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


def main():
    parser = argparse.ArgumentParser(description='Execute Python code in Unreal Engine')
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument('--file', type=str, help='Python file to execute in UE')
    group.add_argument('--code', type=str, help='Python code string to execute')
    parser.add_argument('--port', type=int, default=5555, help='IPC server port (default: 5555)')
    parser.add_argument('--timeout', type=int, default=30, help='Timeout in seconds (default: 30)')
    
    args = parser.parse_args()
    
    # Get code to execute
    if args.file:
        file_path = Path(args.file)
        if not file_path.exists():
            print(f"✗ File not found: {args.file}")
            sys.exit(1)
        
        print(f"Reading Python file: {args.file}")
        with open(file_path, 'r', encoding='utf-8') as f:
            code = f.read()
    else:
        code = args.code
    
    print(f"\nCode to execute ({len(code)} bytes):")
    print("─" * 60)
    print(code[:500] + ("..." if len(code) > 500 else ""))
    print("─" * 60)
    print()
    
    # Send command
    response = send_python_command(code, args.port, args.timeout)
    
    # Display results
    print("\n" + "=" * 60)
    print("RESPONSE FROM UNREAL ENGINE")
    print("=" * 60)
    
    if response.get('status') == 'success':
        print("✅ Status: SUCCESS")
        if 'result' in response:
            print(f"\nResult:\n{response['result']}")
        if 'output' in response:
            print(f"\nOutput:\n{response['output']}")
        if 'processing_time_ms' in response:
            print(f"\nExecution time: {response['processing_time_ms']:.2f} ms")
    else:
        print("❌ Status: ERROR")
        if 'error' in response:
            print(f"\nError: {response['error']}")
    
    print("=" * 60)


if __name__ == '__main__':
    main()
