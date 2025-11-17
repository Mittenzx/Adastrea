#!/usr/bin/env python3
"""
IPC Server for Adastrea Director Plugin

This server provides socket-based IPC communication between the Unreal Engine
plugin (C++) and the Python backend (RAG system, planning agents, etc.).

Week 3 Enhancements:
- Performance monitoring and metrics
- Optimized request routing
- Response serialization with timing
- Integration hooks for RAG system

Usage:
    python ipc_server.py --port 5555
"""

import socket
import json
import sys
import argparse
import logging
import threading
import time
from typing import Dict, Any
from collections import defaultdict
from textwrap import dedent

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger('AdastreaIPCServer')


class PerformanceMetrics:
    """Track performance metrics for the IPC server."""
    
    def __init__(self):
        """Initialize metrics tracking."""
        self.request_count = defaultdict(int)
        self.request_times = defaultdict(list)
        self.error_count = defaultdict(int)
        self.total_requests = 0
        self.total_errors = 0
        self._lock = threading.Lock()
    
    def record_request(self, request_type: str, duration: float, success: bool = True):
        """
        Record a request's performance metrics.
        
        Args:
            request_type: Type of request
            duration: Time taken in seconds
            success: Whether request succeeded
        """
        with self._lock:
            self.request_count[request_type] += 1
            self.request_times[request_type].append(duration)
            self.total_requests += 1
            
            if not success:
                self.error_count[request_type] += 1
                self.total_errors += 1
    
    def get_stats(self) -> Dict[str, Any]:
        """Get current performance statistics."""
        with self._lock:
            stats = {
                'total_requests': self.total_requests,
                'total_errors': self.total_errors,
                'by_type': {}
            }
            
            for req_type in self.request_count:
                times = self.request_times[req_type]
                if times:
                    avg_time = sum(times) / len(times)
                    max_time = max(times)
                    min_time = min(times)
                else:
                    avg_time = max_time = min_time = 0
                
                stats['by_type'][req_type] = {
                    'count': self.request_count[req_type],
                    'errors': self.error_count[req_type],
                    'avg_time_ms': avg_time * 1000,
                    'min_time_ms': min_time * 1000,
                    'max_time_ms': max_time * 1000
                }
            
            return stats
    
    def reset(self):
        """Reset all metrics."""
        with self._lock:
            self.request_count.clear()
            self.request_times.clear()
            self.error_count.clear()
            self.total_requests = 0
            self.total_errors = 0


class IPCServer:
    """IPC server for handling requests from the UE plugin with performance monitoring."""

    def __init__(self, host: str = '127.0.0.1', port: int = 5555, enable_metrics: bool = True):
        """
        Initialize the IPC server.
        
        Args:
            host: Host address to bind to (default: localhost)
            port: Port to listen on
            enable_metrics: Enable performance metrics tracking
        """
        self.host = host
        self.port = port
        self.socket = None
        self.running = False
        self.handlers = {}
        self.metrics = PerformanceMetrics() if enable_metrics else None
        
        # Register default handlers
        self._register_default_handlers()

    def _register_default_handlers(self):
        """Register default request handlers."""
        self.register_handler('ping', self._handle_ping)
        self.register_handler('metrics', self._handle_metrics)
        self.register_handler('query', self._handle_query)
        self.register_handler('plan', self._handle_plan)
        self.register_handler('analyze', self._handle_analyze)

    def register_handler(self, request_type: str, handler_func):
        """
        Register a handler function for a request type.
        
        Args:
            request_type: Type of request (e.g., 'query', 'plan')
            handler_func: Function to handle the request
        """
        self.handlers[request_type] = handler_func
        logger.info(f"Registered handler for '{request_type}'")

    def start(self):
        """Start the IPC server."""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.socket.bind((self.host, self.port))
            self.socket.listen(5)
            self.running = True
            
            logger.info(f"IPC Server started on {self.host}:{self.port}")
            logger.info("Waiting for connections from Unreal Engine plugin...")
            
            self._accept_connections()
            
        except Exception as e:
            logger.error(f"Failed to start server: {e}")
            raise
        finally:
            self.stop()

    def stop(self):
        """Stop the IPC server."""
        self.running = False
        if self.socket:
            logger.info("Shutting down IPC server...")
            self.socket.close()
            self.socket = None

    def _accept_connections(self):
        """Accept and handle client connections."""
        while self.running:
            try:
                client_socket, client_address = self.socket.accept()
                logger.info(f"Client connected from {client_address}")
                
                # Handle client in a separate thread
                client_thread = threading.Thread(
                    target=self._handle_client,
                    args=(client_socket, client_address)
                )
                client_thread.daemon = True
                client_thread.start()
                
            except Exception as e:
                if self.running:
                    logger.error(f"Error accepting connection: {e}")

    def _handle_client(self, client_socket: socket.socket, client_address):
        """
        Handle a client connection.
        
        Args:
            client_socket: Client socket
            client_address: Client address tuple
        """
        try:
            while self.running:
                # Receive data (up to 4KB)
                data = client_socket.recv(4096)
                
                if not data:
                    logger.info(f"Client {client_address} disconnected")
                    break
                
                # Decode and parse request
                try:
                    request_str = data.decode('utf-8').strip()
                    request = json.loads(request_str)
                    
                    logger.debug(f"Received request: {request}")
                    
                    # Process request
                    response = self._process_request(request)
                    
                    # Send response
                    response_str = json.dumps(response) + '\n'
                    client_socket.sendall(response_str.encode('utf-8'))
                    
                except json.JSONDecodeError as e:
                    logger.error(f"Invalid JSON received: {e}")
                    error_response = {
                        'status': 'error',
                        'error': 'Invalid JSON format'
                    }
                    client_socket.sendall(
                        (json.dumps(error_response) + '\n').encode('utf-8')
                    )
                except Exception as e:
                    logger.error(f"Error processing request: {e}")
                    error_response = {
                        'status': 'error',
                        'error': str(e)
                    }
                    client_socket.sendall(
                        (json.dumps(error_response) + '\n').encode('utf-8')
                    )
                
        except Exception as e:
            logger.error(f"Error handling client {client_address}: {e}")
        finally:
            client_socket.close()

    def _process_request(self, request: Dict[str, Any]) -> Dict[str, Any]:
        """
        Process a request and return a response with performance tracking.
        
        Args:
            request: Request dictionary with 'type' and 'data' fields
            
        Returns:
            Response dictionary with timing information
        """
        start_time = time.perf_counter()
        request_type = request.get('type', '')
        request_data = request.get('data', '')
        success = False
        
        if not request_type:
            response = {
                'status': 'error',
                'error': 'Missing request type'
            }
            if self.metrics:
                self.metrics.record_request('invalid', time.perf_counter() - start_time, False)
            return response
        
        # Find and call appropriate handler
        handler = self.handlers.get(request_type)
        
        if handler:
            try:
                response = handler(request_data)
                success = response.get('status') == 'success'
            except Exception as e:
                logger.error(f"Handler error for '{request_type}': {e}")
                response = {
                    'status': 'error',
                    'error': f"Handler error: {str(e)}"
                }
                success = False
        else:
            response = {
                'status': 'error',
                'error': f"Unknown request type: {request_type}"
            }
            success = False
        
        # Record metrics
        duration = time.perf_counter() - start_time
        if self.metrics:
            self.metrics.record_request(request_type, duration, success)
        
        # Add timing information to response for performance monitoring
        response['processing_time_ms'] = round(duration * 1000, 2)
        
        return response

    # Default handlers

    def _handle_ping(self, data: str) -> Dict[str, Any]:
        """Handle ping request for health checks."""
        logger.debug("Ping received")
        return {
            'status': 'success',
            'message': 'pong',
            'timestamp': time.time()
        }
    
    def _handle_metrics(self, data: str) -> Dict[str, Any]:
        """
        Handle metrics request to get performance statistics.
        
        Args:
            data: Optional parameter ('reset' to reset metrics)
        """
        if not self.metrics:
            return {
                'status': 'error',
                'error': 'Metrics not enabled'
            }
        
        if data == 'reset':
            self.metrics.reset()
            logger.info("Metrics reset")
            return {
                'status': 'success',
                'message': 'Metrics reset'
            }
        
        stats = self.metrics.get_stats()
        return {
            'status': 'success',
            'metrics': stats
        }

    def _handle_query(self, data: str) -> Dict[str, Any]:
        """
        Handle documentation query request.
        
        TODO: Integrate with actual RAG system
        """
        logger.info(f"Query received: {data}")
        
        # Provide sample responses for testing
        if "unreal engine" in data.lower() or "what is unreal engine" in data.lower():
            result_text = dedent("""
                Unreal Engine is a comprehensive suite of real-time 3D creation tools developed by Epic Games. 

                Key Features:
                • High-fidelity real-time rendering
                • Advanced physics and collision systems
                • Blueprint visual scripting system
                • C++ programming support
                • Cross-platform development (PC, Console, Mobile, VR/AR)
                • Built-in multiplayer and networking
                • Marketplace with thousands of assets
                • Industry-leading graphics capabilities

                Unreal Engine is widely used for:
                - Video game development (AAA and indie games)
                - Film and television production
                - Architectural visualization
                - Automotive design
                - Virtual production and cinematography

                The engine is free to use with a royalty model for commercial products.
            """).strip()
        else:
            result_text = dedent(f"""
                This is a response to your query: "{data}"

                Currently, this is a placeholder response from the Python backend IPC server.

                The query handler is working correctly and can communicate with the Unreal Engine plugin UI.

                To integrate with the full RAG system and planning agents, the actual implementation will be connected in future phases.
            """).strip()
        
        return {
            'status': 'success',
            'result': result_text,
            'sources': []
        }

    def _handle_plan(self, data: str) -> Dict[str, Any]:
        """
        Handle planning request.
        
        TODO: Integrate with planning agents
        """
        logger.info(f"Plan request received: {data}")
        
        # Placeholder response
        return {
            'status': 'success',
            'plan': {
                'goal': data,
                'tasks': [],
                'dependencies': []
            }
        }

    def _handle_analyze(self, data: str) -> Dict[str, Any]:
        """
        Handle goal analysis request.
        
        TODO: Integrate with goal analysis agent
        """
        logger.info(f"Analyze request received: {data}")
        
        # Placeholder response
        return {
            'status': 'success',
            'analysis': {
                'goal': data,
                'complexity': 'medium',
                'estimated_tasks': 5
            }
        }


def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description='Adastrea Director IPC Server'
    )
    parser.add_argument(
        '--port',
        type=int,
        default=5555,
        help='Port to listen on (default: 5555)'
    )
    parser.add_argument(
        '--host',
        type=str,
        default='127.0.0.1',
        help='Host to bind to (default: 127.0.0.1)'
    )
    parser.add_argument(
        '--verbose',
        action='store_true',
        help='Enable verbose logging'
    )
    parser.add_argument(
        '--enable-ue-python',
        action='store_true',
        help='Enable UE Python API integration (requires running in UE)'
    )
    
    args = parser.parse_args()
    
    if args.verbose:
        logger.setLevel(logging.DEBUG)
    
    # Create and start server
    server = IPCServer(host=args.host, port=args.port)
    
    # Register UE Python API handlers if enabled
    if args.enable_ue_python:
        try:
            from ue_python_integration import register_ue_python_handlers
            register_ue_python_handlers(server)
            logger.info("UE Python API integration enabled")
        except ImportError as e:
            logger.warning(f"Could not enable UE Python API: {e}")
        except Exception as e:
            logger.error(f"Error enabling UE Python API: {e}")
    
    try:
        server.start()
    except KeyboardInterrupt:
        logger.info("Received keyboard interrupt, shutting down...")
        server.stop()
    except Exception as e:
        logger.error(f"Server error: {e}")
        sys.exit(1)


if __name__ == '__main__':
    main()
