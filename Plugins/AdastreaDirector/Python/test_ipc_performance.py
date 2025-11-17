#!/usr/bin/env python3
"""
Performance test suite for IPC server.

Tests round-trip communication latency and throughput to ensure
the < 50ms requirement is met.

Usage:
    python test_ipc_performance.py [port]
"""

import socket
import json
import sys
import time
import statistics
from typing import List, Tuple, Dict


class IPCPerformanceTester:
    """Test IPC server performance."""
    
    def __init__(self, host: str = '127.0.0.1', port: int = 5555):
        """
        Initialize performance tester.
        
        Args:
            host: Server host
            port: Server port
        """
        self.host = host
        self.port = port
        self.sock = None
    
    def connect(self) -> bool:
        """Connect to the IPC server."""
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((self.host, self.port))
            return True
        except Exception as e:
            print(f"✗ Failed to connect: {e}")
            return False
    
    def disconnect(self):
        """Disconnect from the server."""
        if self.sock:
            self.sock.close()
            self.sock = None
    
    def send_request(self, request_type: str, data: str = '') -> Tuple[Dict, float]:
        """
        Send a request and measure round-trip time.
        
        Args:
            request_type: Type of request
            data: Request data
            
        Returns:
            Tuple of (response dict, latency in ms)
        """
        request = {'type': request_type, 'data': data}
        
        # Measure round-trip time
        start = time.perf_counter()
        self.sock.sendall((json.dumps(request) + '\n').encode('utf-8'))
        response = self.sock.recv(4096).decode('utf-8').strip()
        end = time.perf_counter()
        
        latency_ms = (end - start) * 1000
        response_data = json.loads(response)
        
        return response_data, latency_ms
    
    def test_single_request(self, request_type: str, data: str = '') -> Tuple[bool, float]:
        """
        Test a single request and return success status and latency.
        
        Args:
            request_type: Type of request
            data: Request data
            
        Returns:
            Tuple of (success, latency_ms)
        """
        try:
            response, latency = self.send_request(request_type, data)
            success = response.get('status') == 'success'
            return success, latency
        except Exception as e:
            print(f"✗ Request failed: {e}")
            return False, 0.0
    
    def test_multiple_requests(self, request_type: str, count: int, data: str = '') -> List[float]:
        """
        Test multiple requests and return latencies.
        
        Args:
            request_type: Type of request
            count: Number of requests to send
            data: Request data
            
        Returns:
            List of latencies in ms
        """
        latencies = []
        
        for i in range(count):
            try:
                response, latency = self.send_request(request_type, data)
                if response.get('status') == 'success':
                    latencies.append(latency)
                else:
                    print(f"✗ Request {i+1} failed: {response.get('error')}")
            except Exception as e:
                print(f"✗ Request {i+1} exception: {e}")
        
        return latencies
    
    def run_performance_suite(self) -> bool:
        """
        Run comprehensive performance test suite.
        
        Returns:
            True if all tests pass performance requirements
        """
        print("=" * 70)
        print("IPC Server Performance Test Suite")
        print("=" * 70)
        print(f"Target: Round-trip latency < 50ms\n")
        
        all_passed = True
        
        # Test 1: Single ping request
        print("Test 1: Single Ping Request")
        success, latency = self.test_single_request('ping')
        print(f"  Latency: {latency:.2f}ms")
        passed = success and latency < 50
        print(f"  Status: {'✓ PASS' if passed else '✗ FAIL'}\n")
        all_passed &= passed
        
        # Test 2: Multiple ping requests
        print("Test 2: Multiple Ping Requests (n=100)")
        latencies = self.test_multiple_requests('ping', 100)
        
        if latencies:
            avg_latency = statistics.mean(latencies)
            max_latency = max(latencies)
            min_latency = min(latencies)
            p95_latency = statistics.quantiles(latencies, n=100)[94]  # 95th percentile
            
            print(f"  Average:  {avg_latency:.2f}ms")
            print(f"  Min:      {min_latency:.2f}ms")
            print(f"  Max:      {max_latency:.2f}ms")
            print(f"  P95:      {p95_latency:.2f}ms")
            
            passed = avg_latency < 50 and p95_latency < 50
            print(f"  Status: {'✓ PASS' if passed else '✗ FAIL'}\n")
            all_passed &= passed
        else:
            print("  ✗ No successful requests\n")
            all_passed = False
        
        # Test 3: Query request (with data)
        print("Test 3: Query Request with Data")
        test_query = "What is Unreal Engine?"
        success, latency = self.test_single_request('query', test_query)
        print(f"  Query: '{test_query}'")
        print(f"  Latency: {latency:.2f}ms")
        passed = success and latency < 50
        print(f"  Status: {'✓ PASS' if passed else '✗ FAIL'}\n")
        all_passed &= passed
        
        # Test 4: Multiple query requests
        print("Test 4: Multiple Query Requests (n=50)")
        latencies = self.test_multiple_requests('query', 50, test_query)
        
        if latencies:
            avg_latency = statistics.mean(latencies)
            max_latency = max(latencies)
            p95_latency = statistics.quantiles(latencies, n=100)[94]  # 95th percentile
            
            print(f"  Average:  {avg_latency:.2f}ms")
            print(f"  Max:      {max_latency:.2f}ms")
            print(f"  P95:      {p95_latency:.2f}ms")
            
            passed = avg_latency < 50 and p95_latency < 50
            print(f"  Status: {'✓ PASS' if passed else '✗ FAIL'}\n")
            all_passed &= passed
        else:
            print("  ✗ No successful requests\n")
            all_passed = False
        
        # Test 5: Concurrent requests (sequential for now)
        print("Test 5: Rapid Sequential Requests (n=20)")
        latencies = []
        start_all = time.perf_counter()
        
        for i in range(20):
            success, latency = self.test_single_request('ping')
            if success:
                latencies.append(latency)
        
        end_all = time.perf_counter()
        total_time = (end_all - start_all) * 1000
        
        if latencies:
            throughput = len(latencies) / (total_time / 1000)
            avg_latency = statistics.mean(latencies)
            
            print(f"  Total time:   {total_time:.2f}ms")
            print(f"  Throughput:   {throughput:.2f} req/s")
            print(f"  Avg latency:  {avg_latency:.2f}ms")
            
            passed = avg_latency < 50
            print(f"  Status: {'✓ PASS' if passed else '✗ FAIL'}\n")
            all_passed &= passed
        else:
            print("  ✗ No successful requests\n")
            all_passed = False
        
        # Test 6: Server metrics
        print("Test 6: Server Metrics")
        try:
            response, latency = self.send_request('metrics')
            if response.get('status') == 'success':
                metrics = response.get('metrics', {})
                print(f"  Total requests: {metrics.get('total_requests', 0)}")
                print(f"  Total errors:   {metrics.get('total_errors', 0)}")
                
                by_type = metrics.get('by_type', {})
                for req_type, stats in by_type.items():
                    print(f"  {req_type}: {stats.get('count', 0)} requests, "
                          f"avg {stats.get('avg_time_ms', 0):.2f}ms")
                
                print(f"  Status: ✓ PASS\n")
            else:
                print(f"  ✗ Failed to get metrics\n")
                all_passed = False
        except Exception as e:
            print(f"  ✗ Exception: {e}\n")
            all_passed = False
        
        # Summary
        print("=" * 70)
        if all_passed:
            print("✓ ALL TESTS PASSED - Performance requirements met!")
            print("  Round-trip latency is consistently < 50ms")
        else:
            print("✗ SOME TESTS FAILED - Performance requirements not met")
            print("  Please investigate failures above")
        print("=" * 70)
        
        return all_passed


def main():
    """Main entry point."""
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 5555
    
    print(f"Connecting to IPC server on localhost:{port}...")
    
    tester = IPCPerformanceTester(port=port)
    
    if not tester.connect():
        print("\n✗ Failed to connect to server")
        print(f"  Make sure the IPC server is running:")
        print(f"  python ipc_server.py --port {port}")
        return False
    
    print("✓ Connected successfully\n")
    
    try:
        success = tester.run_performance_suite()
        return success
    except KeyboardInterrupt:
        print("\n\nTest interrupted by user")
        return False
    except Exception as e:
        print(f"\n✗ Test suite failed with exception: {e}")
        import traceback
        traceback.print_exc()
        return False
    finally:
        tester.disconnect()


if __name__ == '__main__':
    success = main()
    sys.exit(0 if success else 1)
