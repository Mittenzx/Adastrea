"""
Advanced Unreal Engine interaction via Python and AdastreaDirector IPC
"""
import socket
import json

def send_ue_request(request_type, data=None):
    """Send a request to Unreal Engine and return the response"""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)
        sock.connect(('127.0.0.1', 5555))
        
        request = {'type': request_type, 'data': data or {}}
        sock.sendall((json.dumps(request) + '\n').encode('utf-8'))
        
        response = sock.recv(8192).decode('utf-8')
        result = json.loads(response)
        
        sock.close()
        return result
        
    except Exception as e:
        return {'status': 'error', 'error': str(e)}

def main():
    print("=" * 60)
    print("Unreal Engine Python Interaction Test")
    print("=" * 60)
    
    # Test 1: Ping
    print("\n[1] Testing connection (ping)...")
    result = send_ue_request('ping')
    if result.get('status') == 'success':
        print(f"✅ Connected! Response time: {result.get('processing_time_ms', 0):.2f}ms")
    else:
        print(f"❌ Ping failed: {result.get('error', 'Unknown error')}")
        return
    
    # Test 2: Query project info
    print("\n[2] Querying project information...")
    result = send_ue_request('query', {'query': 'project_info'})
    print(f"Status: {result.get('status', 'unknown')}")
    if result.get('status') == 'success':
        print(f"Response: {json.dumps(result, indent=2)}")
    else:
        print(f"Message: {result.get('message', result.get('error', 'No response'))}")
    
    # Test 3: Get metrics
    print("\n[3] Requesting engine metrics...")
    result = send_ue_request('metrics')
    print(f"Status: {result.get('status', 'unknown')}")
    if result.get('data'):
        print(f"Metrics data: {json.dumps(result.get('data'), indent=2)}")
    else:
        print(f"Message: {result.get('message', 'No metrics available')}")
    
    # Test 4: Analyze command
    print("\n[4] Testing analyze command...")
    result = send_ue_request('analyze', {'target': 'performance'})
    print(f"Status: {result.get('status', 'unknown')}")
    print(f"Message: {result.get('message', result.get('error', 'No response'))}")
    
    print("\n" + "=" * 60)
    print("✅ Python can successfully interact with Unreal Engine!")
    print("=" * 60)
    print("\nAvailable IPC Commands:")
    print("  - ping: Test connection")
    print("  - query: Query project/engine data")
    print("  - metrics: Get performance metrics")
    print("  - analyze: Analyze code/performance")
    print("  - plan: Create development plans")
    print("  - run_tests: Execute automated tests")

if __name__ == '__main__':
    main()
