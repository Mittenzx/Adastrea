#!/usr/bin/env python3
"""
Simple test script for the IPC server.

This script demonstrates how to communicate with the IPC server
using the same protocol that the C++ plugin will use.
"""

import socket
import json
import sys


def test_ipc_server(port=5555):
    """Test the IPC server with various requests."""
    
    print(f"Connecting to IPC server on localhost:{port}...")
    
    try:
        # Create socket and connect
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect(('127.0.0.1', port))
        print("✓ Connected successfully\n")
        
        # Test 1: Ping
        print("Test 1: Ping")
        request = {'type': 'ping', 'data': ''}
        sock.sendall((json.dumps(request) + '\n').encode('utf-8'))
        response = sock.recv(4096).decode('utf-8').strip()
        response_data = json.loads(response)
        print(f"  Request:  {request}")
        print(f"  Response: {response_data}")
        assert response_data['status'] == 'success', "Ping failed"
        assert response_data['message'] == 'pong', "Unexpected ping response"
        print("  ✓ Passed\n")
        
        # Test 2: Query
        print("Test 2: Query")
        request = {'type': 'query', 'data': 'What is Unreal Engine?'}
        sock.sendall((json.dumps(request) + '\n').encode('utf-8'))
        response = sock.recv(4096).decode('utf-8').strip()
        response_data = json.loads(response)
        print(f"  Request:  {request}")
        print(f"  Response: {response_data}")
        assert response_data['status'] == 'success', "Query failed"
        assert 'result' in response_data, "No result field"
        print("  ✓ Passed\n")
        
        # Test 3: Plan
        print("Test 3: Plan")
        request = {'type': 'plan', 'data': 'Create a player movement system'}
        sock.sendall((json.dumps(request) + '\n').encode('utf-8'))
        response = sock.recv(4096).decode('utf-8').strip()
        response_data = json.loads(response)
        print(f"  Request:  {request}")
        print(f"  Response: {response_data}")
        assert response_data['status'] == 'success', "Plan failed"
        assert 'plan' in response_data, "No plan field"
        print("  ✓ Passed\n")
        
        # Test 4: Analyze
        print("Test 4: Analyze")
        request = {'type': 'analyze', 'data': 'Add multiplayer networking'}
        sock.sendall((json.dumps(request) + '\n').encode('utf-8'))
        response = sock.recv(4096).decode('utf-8').strip()
        response_data = json.loads(response)
        print(f"  Request:  {request}")
        print(f"  Response: {response_data}")
        assert response_data['status'] == 'success', "Analyze failed"
        assert 'analysis' in response_data, "No analysis field"
        print("  ✓ Passed\n")
        
        # Test 5: Invalid request type
        print("Test 5: Invalid Request Type")
        request = {'type': 'invalid', 'data': 'test'}
        sock.sendall((json.dumps(request) + '\n').encode('utf-8'))
        response = sock.recv(4096).decode('utf-8').strip()
        response_data = json.loads(response)
        print(f"  Request:  {request}")
        print(f"  Response: {response_data}")
        assert response_data['status'] == 'error', "Should return error"
        assert 'Unknown request type' in response_data['error'], "Wrong error message"
        print("  ✓ Passed\n")
        
        sock.close()
        
        print("=" * 60)
        print("All tests passed! ✓")
        print("=" * 60)
        return True
        
    except ConnectionRefusedError:
        print(f"✗ Failed to connect to server on port {port}")
        print("  Make sure the IPC server is running:")
        print(f"  python ipc_server.py --port {port}")
        return False
    except Exception as e:
        print(f"✗ Test failed: {e}")
        import traceback
        traceback.print_exc()
        return False


if __name__ == '__main__':
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 5555
    success = test_ipc_server(port)
    sys.exit(0 if success else 1)
