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

import os
import re
import socket
import json
import sys
import argparse
import logging
import threading
import time
from typing import Dict, Any, Optional, Tuple
from collections import defaultdict
from textwrap import dedent

# Add parent directory to path to import main modules (for goal/task agents, llm_config, etc.)
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../../..')))

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger('AdastreaIPCServer')

# Constants
DEFAULT_CHROMA_DB_NAME = "chroma_db"
DEFAULT_COLLECTION_NAME = "adastrea_docs"


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
        
        # MCP server instance (lazy initialization)
        self._mcp_server = None
        
        # Ingestion lock to prevent concurrent ingestion requests
        self._ingestion_lock = threading.Lock()
        self._is_ingesting = False
        
        # Register default handlers
        self._register_default_handlers()

    def _register_default_handlers(self):
        """Register default request handlers."""
        self.register_handler('ping', self._handle_ping)
        self.register_handler('validate_api_key', self._handle_validate_api_key)
        self.register_handler('metrics', self._handle_metrics)
        # Connection testing handlers
        self.register_handler('test_connection', self._handle_test_connection)
        self.register_handler('test_llm_connection', self._handle_test_llm_connection)
        self.register_handler('test_rag_system', self._handle_test_rag_system)
        self.register_handler('test_python_backend', self._handle_test_python_backend)
        self.register_handler('query', self._handle_query)
        self.register_handler('plan', self._handle_plan)
        self.register_handler('analyze', self._handle_analyze)
        self.register_handler('run_tests', self._handle_run_tests)
        # Phase 2 handlers
        self.register_handler('generate_code', self._handle_generate_code)
        self.register_handler('apply_feedback', self._handle_apply_feedback)
        self.register_handler('get_confidence', self._handle_get_confidence)
        # MCP/UE integration handlers
        self.register_handler('mcp_connect', self._handle_mcp_connect)
        self.register_handler('mcp_disconnect', self._handle_mcp_disconnect)
        self.register_handler('mcp_status', self._handle_mcp_status)
        self.register_handler('mcp_execute_python', self._handle_mcp_execute_python)
        self.register_handler('mcp_console_command', self._handle_mcp_console_command)
        self.register_handler('mcp_list_tools', self._handle_mcp_list_tools)
        self.register_handler('mcp_call_tool', self._handle_mcp_call_tool)
        # Log access handlers
        self.register_handler('get_ue_logs', self._handle_get_ue_logs)
        self.register_handler('list_ue_logs', self._handle_list_ue_logs)
        self.register_handler('read_ue_log', self._handle_read_ue_log)
        # RAG ingestion handler
        self.register_handler('ingest', self._handle_ingest)
        self.register_handler('clear_history', self._handle_clear_history)

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
        
        # Clean up MCP server
        if self._mcp_server:
            try:
                self._mcp_server.stop()
                logger.info("MCP server stopped")
            except Exception as e:
                logger.error(f"Error stopping MCP server: {e}")
            finally:
                self._mcp_server = None
        
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

    # Helper methods
    
    def _find_persist_directory(self) -> Optional[str]:
        """
        Find the ChromaDB persist directory.
        
        Checks for the CHROMA_PERSIST_DIRECTORY environment variable first,
        then falls back to checking common locations.
        
        Returns:
            Path to persist directory if found, None otherwise
        """
        # Check environment variable first
        env_persist_dir = os.environ.get('CHROMA_PERSIST_DIRECTORY')
        if env_persist_dir and os.path.exists(env_persist_dir):
            return env_persist_dir
        
        # Check common persist directory locations
        persist_dirs = [
            './chroma_db',
            '../../../chroma_db',
            os.path.join(os.path.dirname(__file__), '..', '..', '..', 'chroma_db'),
        ]
        
        for pd in persist_dirs:
            if os.path.exists(pd):
                return pd
        
        return None
    
    def _validate_path_safety(self, path: str, require_exists: bool = False) -> Tuple[bool, Optional[str]]:
        """
        Validate that a path is safe from path traversal attacks.
        
        Args:
            path: Path to validate
            require_exists: If True, path must exist
            
        Returns:
            Tuple of (is_valid, error_message)
        """
        if not path:
            return False, "Path cannot be empty"
        
        # Convert to absolute path and normalize (handles .., ., //, etc.)
        try:
            abs_path = os.path.abspath(os.path.normpath(path))
        except Exception as e:
            return False, f"Invalid path format: {str(e)}"
        
        # Additional security check: reject if path contains .. after normalization
        # This catches edge cases that might slip through normalization
        if ".." in path:
            return False, "Path contains '..' which is not allowed for security reasons"
        
        # Check if path exists if required
        if require_exists:
            if not os.path.exists(abs_path):
                return False, f"Path does not exist: {path}"
            
            # Check if it's a directory (os.path.isdir returns False for non-existent paths)
            if not os.path.isdir(abs_path):
                return False, f"Path is not a directory: {path}"
        
        return True, None
    
    @staticmethod
    def _extract_code_block_content(text: str) -> str:
        """
        Extract content from markdown code blocks in an LLM response.
        
        Handles markdown code blocks (```json ... ``` or ``` ... ```).
        Falls back to original text if no valid code block is found or extracted content is empty.
        
        Args:
            text: Raw LLM response text
            
        Returns:
            Extracted content from code block, or original text if no valid code block found
        """
        try:
            if '```json' in text:
                parts = text.split('```json')
                if len(parts) > 1:
                    inner_parts = parts[1].split('```')
                    if len(inner_parts) > 0:
                        content = inner_parts[0].strip()
                        if content:  # Only return if non-empty
                            return content
            elif '```' in text:
                parts = text.split('```')
                if len(parts) > 2:  # Need at least 3 parts: before, content, after
                    content = parts[1].strip()
                    if content:  # Only return if non-empty
                        return content
        except (IndexError, AttributeError):
            # Malformed input or missing code block; fallback to returning original text.
            pass
        return text

    # Default handlers

    def _handle_ping(self, data: str) -> Dict[str, Any]:
        """Handle ping request for health checks."""
        logger.debug("Ping received")
        return {
            'status': 'success',
            'message': 'pong',
            'timestamp': time.time()
        }
    
    def _handle_validate_api_key(self, data: str) -> Dict[str, Any]:
        """
        Validate an API key by attempting a simple test request.
        API keys are read from environment variables (.env file).
        
        Args:
            data: JSON string with 'provider' field
            
        Returns:
            Dict with validation result
        """
        logger.info("API key validation requested")
        
        try:
            # Parse request data
            request_data = json.loads(data) if data else {}
            provider = request_data.get('provider', '').lower()
            
            if not provider:
                return {
                    'status': 'error',
                    'error': 'Missing provider in request'
                }
            
            # Get API key from environment variables
            api_key = None
            if provider == 'gemini':
                # Check multiple env variable names for Gemini
                api_key = os.environ.get('GEMINI_KEY') or os.environ.get('GOOGLE_API_KEY')
                if not api_key:
                    return {
                        'status': 'success',
                        'valid': False,
                        'error': 'GEMINI_KEY not found in .env file. Please add GEMINI_KEY=your-api-key to your .env file.',
                        'provider': 'gemini'
                    }
                return self._validate_gemini_key(api_key)
            elif provider == 'openai':
                api_key = os.environ.get('OPENAI_API_KEY')
                if not api_key:
                    return {
                        'status': 'success',
                        'valid': False,
                        'error': 'OPENAI_API_KEY not found in .env file. Please add OPENAI_API_KEY=your-api-key to your .env file.',
                        'provider': 'openai'
                    }
                return self._validate_openai_key(api_key)
            else:
                return {
                    'status': 'error',
                    'error': f'Unsupported provider: {provider}'
                }
                
        except json.JSONDecodeError as e:
            logger.error(f"Failed to parse validation request: {e}")
            return {
                'status': 'error',
                'error': f'Invalid JSON: {str(e)}'
            }
        except Exception as e:
            logger.error(f"Error validating API key: {e}")
            return {
                'status': 'error',
                'error': f'Validation error: {str(e)}'
            }
    
    def _validate_gemini_key(self, api_key: str) -> Dict[str, Any]:
        """
        Validate a Gemini API key by attempting a simple test request.
        
        Args:
            api_key: The Gemini API key to validate
            
        Returns:
            Dict with validation result
        """
        import google.generativeai as genai
        
        try:
            # Configure with the provided key (note: this still sets global state)
            # Unfortunately, genai doesn't provide instance-based API currently
            genai.configure(api_key=api_key)
            
            # Try to list models as a simple validation check
            models = genai.list_models()
            
            # If we can list models, the key is valid
            model_count = sum(1 for _ in models)
            
            return {
                'status': 'success',
                'valid': True,
                'message': f'Gemini API key is valid. Found {model_count} available models.',
                'provider': 'gemini'
            }
            
        except Exception as e:
            error_msg = str(e)
            
            # Sanitize error message to avoid leaking sensitive information
            # Check for common error patterns
            if '401' in error_msg or 'API key not valid' in error_msg or 'INVALID_ARGUMENT' in error_msg:
                return {
                    'status': 'success',
                    'valid': False,
                    'error': 'API key is invalid or has been revoked',
                    'provider': 'gemini'
                }
            elif 'quota' in error_msg.lower():
                return {
                    'status': 'success',
                    'valid': True,
                    'message': 'API key is valid but quota may be exceeded',
                    'provider': 'gemini'
                }
            elif 'network' in error_msg.lower() or 'connection' in error_msg.lower():
                return {
                    'status': 'success',
                    'valid': False,
                    'error': 'Network error - cannot verify API key at this time',
                    'provider': 'gemini'
                }
            else:
                # Log full error for debugging but return sanitized message
                logger.warning(f"Gemini validation error: {error_msg}")
                return {
                    'status': 'success',
                    'valid': False,
                    'error': 'Validation failed due to an unexpected error. Check server logs for details.',
                    'provider': 'gemini'
                }
        # Note: genai library doesn't provide a way to restore previous state
        # The global configuration remains set to the last validated key
        # Multiple validations should be done sequentially to avoid issues
    
    def _validate_openai_key(self, api_key: str) -> Dict[str, Any]:
        """
        Validate an OpenAI API key by attempting a simple test request.
        
        Args:
            api_key: The OpenAI API key to validate
            
        Returns:
            Dict with validation result
        """
        model_count = 0  # Initialize to avoid undefined variable
        
        try:
            # Import OpenAI client (supports both old and new API)
            try:
                # Try new API first (openai >= 1.0.0)
                from openai import OpenAI
                client = OpenAI(api_key=api_key)
                models = client.models.list()
                model_count = len(list(models))
            except (ImportError, AttributeError):
                # Fallback to old API (openai < 1.0.0)
                # Use local state to avoid affecting global openai.api_key
                import openai
                # Save current global state
                old_api_key = getattr(openai, 'api_key', None)
                try:
                    openai.api_key = api_key
                    models = openai.Model.list()
                    model_count = len(models.data)
                finally:
                    # Restore previous state
                    if old_api_key is not None:
                        openai.api_key = old_api_key
                    else:
                        # Clear if it wasn't set before
                        openai.api_key = None
            
            return {
                'status': 'success',
                'valid': True,
                'message': f'OpenAI API key is valid. Found {model_count} available models.',
                'provider': 'openai'
            }
            
        except Exception as e:
            error_msg = str(e)
            
            # Check for common error patterns
            if '401' in error_msg or 'Incorrect API key' in error_msg or 'invalid_api_key' in error_msg:
                return {
                    'status': 'success',
                    'valid': False,
                    'error': 'API key is invalid or has been revoked',
                    'provider': 'openai'
                }
            elif 'quota' in error_msg.lower() or 'rate limit' in error_msg.lower():
                return {
                    'status': 'success',
                    'valid': True,
                    'message': 'API key is valid but rate limit/quota may be exceeded',
                    'provider': 'openai'
                }
            elif 'network' in error_msg.lower() or 'connection' in error_msg.lower():
                return {
                    'status': 'success',
                    'valid': False,
                    'error': 'Network error - cannot verify API key at this time',
                    'provider': 'openai'
                }
            else:
                logger.warning(f"OpenAI validation error: {error_msg}")
                return {
                    'status': 'success',
                    'valid': False,
                    'error': 'Validation failed due to an unexpected error. Check server logs for details.',
                    'provider': 'openai'
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

    def _handle_test_llm_connection(self, data: str) -> Dict[str, Any]:
        """
        Test LLM connection and API key validity.
        
        Returns detailed status about:
        - API key configuration
        - API key validity
        - Connection to LLM provider
        """
        logger.info("Testing LLM connection")
        
        result = {
            'status': 'success',
            'component': 'llm',
            'tests': []
        }
        
        # Test 1: Check if API key is configured
        api_key_configured = False
        api_key_source = None
        provider = os.environ.get('LLM_PROVIDER', 'gemini').lower()
        
        if provider == 'gemini':
            api_key = os.environ.get('GEMINI_KEY') or os.environ.get('GOOGLE_API_KEY')
            api_key_var = 'GEMINI_KEY or GOOGLE_API_KEY'
        else:
            api_key = os.environ.get('OPENAI_API_KEY')
            api_key_var = 'OPENAI_API_KEY'
        
        if api_key:
            api_key_configured = True
            api_key_source = api_key_var
            result['tests'].append({
                'name': 'API Key Configuration',
                'status': 'pass',
                'message': f'API key is configured ({api_key_var})'
            })
        else:
            result['tests'].append({
                'name': 'API Key Configuration',
                'status': 'fail',
                'message': f'No API key found. Please set {api_key_var} in your .env file',
                'solution': f'Add {api_key_var}=your-api-key to .env file in project root'
            })
            result['overall_status'] = 'fail'
            return result
        
        # Test 2: Validate API key with provider
        try:
            if provider == 'gemini':
                validation_result = self._validate_gemini_key(api_key)
            else:
                validation_result = self._validate_openai_key(api_key)
            
            if validation_result.get('valid'):
                result['tests'].append({
                    'name': 'API Key Validity',
                    'status': 'pass',
                    'message': validation_result.get('message', 'API key is valid')
                })
                result['overall_status'] = 'pass'
            else:
                result['tests'].append({
                    'name': 'API Key Validity',
                    'status': 'fail',
                    'message': validation_result.get('error', 'API key validation failed'),
                    'solution': f'Check your API key is correct and has not been revoked. Get a new key from the provider.'
                })
                result['overall_status'] = 'fail'
        except Exception as e:
            result['tests'].append({
                'name': 'API Key Validity',
                'status': 'error',
                'message': f'Error testing API key: {str(e)}',
                'solution': 'Check your internet connection and try again'
            })
            result['overall_status'] = 'error'
        
        return result

    def _handle_test_rag_system(self, data: str) -> Dict[str, Any]:
        """
        Test RAG system availability and document database.
        
        Returns detailed status about:
        - Vector database existence
        - Document count
        - RAG system readiness
        """
        logger.info("Testing RAG system")
        
        result = {
            'status': 'success',
            'component': 'rag',
            'tests': []
        }
        
        # Test 1: Check if persist directory exists
        persist_directory = os.environ.get('CHROMA_PERSIST_DIRECTORY')
        if not persist_directory:
            # Try default location
            project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../..'))
            persist_directory = os.path.join(project_root, DEFAULT_CHROMA_DB_NAME)
        
        if not os.path.exists(persist_directory):
            result['tests'].append({
                'name': 'Vector Database',
                'status': 'fail',
                'message': f'Vector database not found at {persist_directory}',
                'solution': 'Run: python ingest.py --docs-dir <your_docs_folder>'
            })
            result['overall_status'] = 'fail'
            return result
        
        result['tests'].append({
            'name': 'Vector Database',
            'status': 'pass',
            'message': f'Vector database found at {persist_directory}'
        })
        
        # Test 2: Check if documents are ingested
        try:
            import chromadb
            
            # Note: Telemetry is already disabled globally at startup in gui_director.py and ingest.py
            
            client = chromadb.PersistentClient(path=persist_directory)
            collection = client.get_or_create_collection(name=DEFAULT_COLLECTION_NAME)
            doc_count = collection.count()
            
            if doc_count > 0:
                result['tests'].append({
                    'name': 'Document Ingestion',
                    'status': 'pass',
                    'message': f'{doc_count} documents indexed and ready'
                })
                result['overall_status'] = 'pass'
                result['document_count'] = doc_count
            else:
                result['tests'].append({
                    'name': 'Document Ingestion',
                    'status': 'fail',
                    'message': 'No documents have been ingested',
                    'solution': 'Run: python ingest.py --docs-dir <your_docs_folder>'
                })
                result['overall_status'] = 'fail'
        except Exception as e:
            result['tests'].append({
                'name': 'Document Ingestion',
                'status': 'error',
                'message': f'Error checking documents: {str(e)}',
                'solution': 'Try re-running ingestion: python ingest.py --docs-dir <your_docs_folder>'
            })
            result['overall_status'] = 'error'
        
        return result

    def _handle_test_python_backend(self, data: str) -> Dict[str, Any]:
        """
        Test Python backend health and dependencies.
        
        Returns detailed status about:
        - Python version
        - Required dependencies
        - Backend services
        """
        logger.info("Testing Python backend")
        
        result = {
            'status': 'success',
            'component': 'backend',
            'tests': []
        }
        
        # Test 1: Python version
        import sys
        python_version = f"{sys.version_info.major}.{sys.version_info.minor}.{sys.version_info.micro}"
        
        if sys.version_info >= (3, 9):
            result['tests'].append({
                'name': 'Python Version',
                'status': 'pass',
                'message': f'Python {python_version} (compatible)'
            })
        else:
            result['tests'].append({
                'name': 'Python Version',
                'status': 'fail',
                'message': f'Python {python_version} (requires 3.9+)',
                'solution': 'Upgrade to Python 3.9 or higher'
            })
            result['overall_status'] = 'fail'
        
        # Test 2: Critical dependencies
        critical_deps = [
            ('chromadb', 'Vector database'),
            ('langchain', 'LLM framework'),
            ('google.generativeai', 'Gemini SDK')
        ]
        
        all_deps_ok = True
        for module_name, description in critical_deps:
            try:
                __import__(module_name)
                result['tests'].append({
                    'name': f'{description}',
                    'status': 'pass',
                    'message': f'{module_name} is installed'
                })
            except ImportError:
                all_deps_ok = False
                result['tests'].append({
                    'name': f'{description}',
                    'status': 'fail',
                    'message': f'{module_name} is not installed',
                    'solution': f'Run: pip install {module_name}'
                })
        
        if all_deps_ok and 'overall_status' not in result:
            result['overall_status'] = 'pass'
        elif not all_deps_ok:
            result['overall_status'] = 'fail'
        
        # Test 3: IPC Server running (always passes if we're here)
        result['tests'].append({
            'name': 'IPC Server',
            'status': 'pass',
            'message': f'IPC server is running on port {self.port}'
        })
        
        return result

    def _handle_test_connection(self, data: str) -> Dict[str, Any]:
        """
        Test all components of the system.
        
        Runs comprehensive tests on:
        - Python backend
        - LLM connection
        - RAG system
        
        Returns a consolidated report with actionable next steps.
        """
        logger.info("Running comprehensive connection test")
        
        # Run all tests
        backend_result = self._handle_test_python_backend("")
        llm_result = self._handle_test_llm_connection("")
        rag_result = self._handle_test_rag_system("")
        
        # Consolidate results
        result = {
            'status': 'success',
            'components': {
                'backend': backend_result,
                'llm': llm_result,
                'rag': rag_result
            }
        }
        
        # Determine overall status
        all_pass = (
            backend_result.get('overall_status') == 'pass' and
            llm_result.get('overall_status') == 'pass' and
            rag_result.get('overall_status') == 'pass'
        )
        
        if all_pass:
            result['overall_status'] = 'pass'
            result['message'] = '✅ All systems operational! Ready to answer questions.'
        else:
            result['overall_status'] = 'fail'
            
            # Build a specific failure message
            failed_components = []
            if backend_result.get('overall_status') != 'pass':
                failed_components.append('Python Backend')
            if llm_result.get('overall_status') != 'pass':
                failed_components.append('LLM Connection')
            if rag_result.get('overall_status') != 'pass':
                failed_components.append('RAG System')
            
            result['message'] = f'❌ Issues detected: {", ".join(failed_components)}'
            
            # Provide next steps based on failures
            next_steps = []
            
            if llm_result.get('overall_status') != 'pass':
                next_steps.append('1. Configure your API key (see LLM Connection test details)')
            
            if rag_result.get('overall_status') != 'pass':
                next_steps.append('2. Ingest documents: python ingest.py --docs-dir <your_docs_folder>')
            
            if backend_result.get('overall_status') != 'pass':
                next_steps.append('3. Install missing dependencies: pip install -r requirements.txt')
            
            result['next_steps'] = next_steps
        
        return result

    def _handle_query(self, data: str) -> Dict[str, Any]:
        """
        Handle documentation query request using the RAG system.
        
        Attempts to use the RAG system if available. Falls back to LLM-only
        response if RAG database is not initialized.
        
        Environment Variables:
            CHROMA_PERSIST_DIRECTORY: Override the default persist directory path
        """
        logger.info(f"Query received: {data}")
        
        # Try to use the RAG system
        try:
            from rag_query import RAGQueryAgent
            
            # Use helper method to find persist directory
            persist_directory = self._find_persist_directory()
            
            if persist_directory:
                logger.info(f"Using RAG database at: {persist_directory}")
                query_agent = RAGQueryAgent(
                    collection_name=DEFAULT_COLLECTION_NAME,
                    persist_directory=persist_directory
                )
                
                response = query_agent.process_query(data)
                
                return {
                    'status': 'success',
                    'result': response.get('answer', ''),
                    'sources': response.get('source_documents', []),
                    'processing_time': response.get('processing_time', 0),
                    'cached': response.get('cached', False)
                }
            else:
                logger.warning("RAG database not found, using LLM-only mode")
                
        except ImportError as e:
            logger.warning(f"RAG module not available: {e}")
        except ValueError as e:
            # Database is empty or not initialized
            logger.warning(f"RAG database issue: {e}")
        except Exception as e:
            logger.error(f"RAG query error: {e}")
        
        # Fallback: Try direct LLM query if RAG is not available
        try:
            from llm_config import get_llm
            
            llm = get_llm()
            
            prompt = f"""You are the Adastrea Director, an AI assistant specialized in helping with game development in Unreal Engine.

Answer the following question to the best of your ability. Be concise but thorough.

Question: {data}

Answer:"""
            
            response = llm.invoke(prompt)
            result_text = response.content if hasattr(response, 'content') else str(response)
            
            return {
                'status': 'success',
                'result': result_text,
                'sources': [],
                'note': 'Response generated without RAG context. For context-aware answers, please ingest documents first.'
            }
            
        except Exception as e:
            logger.error(f"LLM query error: {e}")
            # Run diagnostic tests to identify the specific issue
            try:
                test_results = self._handle_test_connection("")
                
                if test_results.get('overall_status') == 'fail':
                    # Build a specific error message based on test results
                    error_parts = [f'I received your query: "{data}"', 
                                 '',
                                 'However, I cannot provide a response due to the following issues:',
                                 '']
                    
                    components = test_results.get('components', {})
                    
                    # Check each component and add specific messages
                    if components.get('llm', {}).get('overall_status') != 'pass':
                        error_parts.append('❌ LLM Connection: API key is not configured or invalid')
                        llm_tests = components.get('llm', {}).get('tests', [])
                        for test in llm_tests:
                            if test.get('status') == 'fail' and test.get('solution'):
                                error_parts.append(f"   💡 {test.get('solution')}")
                        error_parts.append('')
                    
                    if components.get('rag', {}).get('overall_status') != 'pass':
                        error_parts.append('❌ RAG System: Document database is not set up')
                        error_parts.append('   💡 Run: python ingest.py --docs-dir <your_docs_folder>')
                        error_parts.append('')
                    
                    if components.get('backend', {}).get('overall_status') != 'pass':
                        error_parts.append('❌ Python Backend: Missing dependencies')
                        error_parts.append('   💡 Run: pip install -r requirements.txt')
                        error_parts.append('')
                    
                    # Add next steps from test results
                    next_steps = test_results.get('next_steps', [])
                    if next_steps:
                        error_parts.append('📝 To fix these issues:')
                        for step in next_steps:
                            error_parts.append(f'   {step}')
                    
                    error_parts.append('')
                    error_parts.append('💡 TIP: Use the "Test Connection" button in the GUI to see detailed diagnostics.')
                    
                    result_text = '\n'.join(error_parts)
                else:
                    # Fallback to generic message if tests pass but query still fails
                    result_text = dedent(f"""
                        I received your query: "{data}"
                        
                        An unexpected error occurred while processing your request: {str(e)}
                        
                        All system components appear to be working. This may be a temporary issue.
                        Please try again, or check the server logs for more details.
                    """).strip()
            except Exception as test_error:
                logger.error(f"Error running diagnostic tests: {test_error}")
                # Ultimate fallback
                result_text = dedent(f"""
                    I received your query: "{data}"

                    However, I'm unable to provide a meaningful response at this time.
                    
                    💡 To diagnose the issue:
                    1. Use the "Test Connection" button in the GUI Status Dashboard
                    2. This will show you exactly which component needs attention:
                       - Python Backend (dependencies)
                       - LLM Connection (API key)
                       - RAG System (document database)
                    
                    For more information, see the README.md file.
                """).strip()
            
            return {
                'status': 'success',
                'result': result_text,
                'sources': [],
                'setup_required': True
            }

    def _handle_plan(self, data: str) -> Dict[str, Any]:
        """
        Handle planning request using the task decomposition agent.
        
        Attempts to use the planning agents if available.
        """
        logger.info(f"Plan request received: {data}")
        
        # Try to use actual planning agents
        try:
            from task_decomposition_agent import TaskDecompositionAgent
            from goal_analysis_agent import GoalAnalysisAgent
            
            # First parse the goal into a Goal object
            goal_agent = GoalAnalysisAgent()
            goal_obj = goal_agent.parse_goal(data)
            
            # Then decompose into tasks (returns a TaskTree)
            task_agent = TaskDecompositionAgent()
            task_tree = task_agent.decompose_goal(goal_obj)
            
            # Extract tasks from TaskTree - convert to serializable format
            all_tasks = task_tree.get_all_tasks()
            tasks_list = []
            for task in all_tasks:
                tasks_list.append({
                    'id': task.id,
                    'name': task.title,
                    'description': task.description,
                    'priority': task.priority.value if hasattr(task.priority, 'value') else str(task.priority),
                    'dependencies': task.dependencies,
                    'estimated_effort': task.estimated_effort or 'unknown'
                })
            
            # Calculate estimated duration from total
            estimated_duration = str(task_tree.total_estimated_duration) if task_tree.total_estimated_duration else 'unknown'
            
            return {
                'status': 'success',
                'plan': {
                    'goal': data,
                    'tasks': tasks_list,
                    'dependencies': [],  # Dependencies are already in tasks
                    'estimated_duration': estimated_duration
                }
            }
            
        except ImportError as e:
            logger.warning(f"Planning agents not available: {e}")
        except Exception as e:
            logger.error(f"Planning error: {e}")
        
        # Fallback: Try direct LLM planning
        try:
            from llm_config import get_llm
            
            llm = get_llm()
            
            prompt = f"""You are the Adastrea Director, an AI assistant specialized in game development planning.

Break down the following goal into a list of actionable tasks for Unreal Engine development.

Goal: {data}

Respond in JSON format with this structure:
{{
    "tasks": [
        {{"id": 1, "name": "Task name", "description": "Brief description", "priority": "high/medium/low"}},
        ...
    ],
    "dependencies": [
        {{"from": 1, "to": 2}}
    ],
    "estimated_duration": "X hours/days"
}}

JSON Response:"""
            
            response = llm.invoke(prompt)
            result_text = response.content if hasattr(response, 'content') else str(response)
            
            # Try to parse as JSON
            try:
                json_str = self._extract_code_block_content(result_text)
                plan_data = json.loads(json_str.strip())
                return {
                    'status': 'success',
                    'plan': {
                        'goal': data,
                        'tasks': plan_data.get('tasks', []),
                        'dependencies': plan_data.get('dependencies', []),
                        'estimated_duration': plan_data.get('estimated_duration', 'unknown')
                    }
                }
            except json.JSONDecodeError:
                # Return raw response if JSON parsing fails
                return {
                    'status': 'success',
                    'plan': {
                        'goal': data,
                        'tasks': [{'id': 1, 'name': 'Generated Plan', 'description': result_text, 'priority': 'medium'}],
                        'dependencies': [],
                        'estimated_duration': 'unknown'
                    }
                }
                
        except Exception as e:
            logger.error(f"LLM planning error: {e}")
        
        # Final fallback
        return {
            'status': 'success',
            'plan': {
                'goal': data,
                'tasks': [],
                'dependencies': []
            },
            'note': 'Planning agents not available. Please configure an LLM API key.'
        }

    def _handle_analyze(self, data: str) -> Dict[str, Any]:
        """
        Handle goal analysis request using the goal analysis agent.
        
        Attempts to use the goal analysis agent if available.
        """
        logger.info(f"Analyze request received: {data}")
        
        # Try to use actual goal analysis agent
        try:
            from goal_analysis_agent import GoalAnalysisAgent
            
            agent = GoalAnalysisAgent()
            goal_obj = agent.parse_goal(data)
            
            # Extract goal_type value properly (it's an enum)
            goal_type_val = goal_obj.goal_type.value if hasattr(goal_obj.goal_type, 'value') else str(goal_obj.goal_type)
            
            # Extract scope information using safe attribute access
            scope_areas = []
            if goal_obj.scope:
                scope_areas = getattr(goal_obj.scope, 'affected_areas', []) or getattr(goal_obj.scope, 'systems', [])
            
            # Extract constraints as list of descriptions
            constraints_list = []
            if goal_obj.constraints:
                for c in goal_obj.constraints:
                    constraints_list.append(c.description if hasattr(c, 'description') else str(c))
            
            # Get complexity from scope
            complexity = 'medium'
            if goal_obj.scope:
                complexity = getattr(goal_obj.scope, 'estimated_complexity', 'medium') or 'medium'
            
            # Base task count for estimation
            BASE_TASK_COUNT = 3
            
            return {
                'status': 'success',
                'analysis': {
                    'goal': data,
                    'goal_type': goal_type_val,
                    'complexity': complexity,
                    'scope': scope_areas,
                    'constraints': constraints_list,
                    'estimated_tasks': len(constraints_list) + BASE_TASK_COUNT
                }
            }
            
        except ImportError as e:
            logger.warning(f"Goal analysis agent not available: {e}")
        except Exception as e:
            logger.error(f"Goal analysis error: {e}")
        
        # Fallback: Try direct LLM analysis
        try:
            from llm_config import get_llm
            
            llm = get_llm()
            
            prompt = f"""You are the Adastrea Director, an AI assistant specialized in analyzing game development goals.

Analyze the following goal and provide a structured assessment for Unreal Engine development.

Goal: {data}

Respond in JSON format with this structure:
{{
    "goal_type": "feature/bugfix/optimization/refactor/documentation",
    "complexity": "low/medium/high",
    "scope": ["list", "of", "affected", "areas"],
    "constraints": ["list", "of", "potential", "constraints"],
    "estimated_tasks": <number>
}}

JSON Response:"""
            
            response = llm.invoke(prompt)
            result_text = response.content if hasattr(response, 'content') else str(response)
            
            # Try to parse as JSON
            try:
                json_str = self._extract_code_block_content(result_text)
                analysis_data = json.loads(json_str.strip())
                return {
                    'status': 'success',
                    'analysis': {
                        'goal': data,
                        'goal_type': analysis_data.get('goal_type', 'unknown'),
                        'complexity': analysis_data.get('complexity', 'medium'),
                        'scope': analysis_data.get('scope', []),
                        'constraints': analysis_data.get('constraints', []),
                        'estimated_tasks': analysis_data.get('estimated_tasks', 5)
                    }
                }
            except json.JSONDecodeError:
                # Return basic analysis if JSON parsing fails
                return {
                    'status': 'success',
                    'analysis': {
                        'goal': data,
                        'goal_type': 'feature',
                        'complexity': 'medium',
                        'scope': [],
                        'constraints': [],
                        'estimated_tasks': 5,
                        'raw_analysis': result_text
                    }
                }
                
        except Exception as e:
            logger.error(f"LLM analysis error: {e}")
        
        # Final fallback
        return {
            'status': 'success',
            'analysis': {
                'goal': data,
                'complexity': 'medium',
                'estimated_tasks': 5
            },
            'note': 'Analysis agent not available. Please configure an LLM API key.'
        }

    def _handle_run_tests(self, data: str) -> Dict[str, Any]:
        """
        Handle run_tests request to execute plugin self-tests.
        
        Args:
            data: Test type to run ('all', 'ipc', 'plugin', 'unit', etc.)
            
        Returns:
            Dict with test results including passed/failed counts
        """
        logger.info(f"Run tests received: {data}")
        
        import subprocess
        
        # Sanitize and validate test_type input
        test_type = data.strip().lower() if data else 'all'
        # Remove any non-alphanumeric characters except underscore
        test_type = re.sub(r'[^a-z0-9_]', '', test_type)
        
        # Map test types to pytest commands
        # Note: 'ipc' focuses on IPC-related tests, 'plugin' includes all plugin Python tests
        test_commands = {
            'all': [sys.executable, '-m', 'pytest', '-v', '--tb=short'],
            'ipc': [sys.executable, '-m', 'pytest', '-v', 'Plugins/AdastreaDirector/Python/test_ipc.py', 
                    'Plugins/AdastreaDirector/Python/test_ipc_performance.py', '--tb=short'],
            'plugin': [sys.executable, '-m', 'pytest', '-v', 'Plugins/AdastreaDirector/Python/', '--tb=short'],
            'unit': [sys.executable, '-m', 'pytest', '-v', '-m', 'unit', '--tb=short'],
            'integration': [sys.executable, '-m', 'pytest', '-v', 'tests/integration/', '--tb=short'],
            'remote': [sys.executable, '-m', 'pytest', '-v', 'tests/remote_control/', '--tb=short'],
        }
        
        if test_type not in test_commands:
            available_types = ', '.join(test_commands.keys())
            return {
                'status': 'error',
                'error': f"Unknown test type. Available: {available_types}"
            }
        
        command = test_commands[test_type]
        
        # Get the project root directory (three levels up from ipc_server.py)
        script_dir = os.path.dirname(os.path.abspath(__file__))
        project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
        
        try:
            # Run pytest and capture output using Popen for better timeout handling
            process = subprocess.Popen(
                command,
                cwd=project_root,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            try:
                stdout, stderr = process.communicate(timeout=300)  # 5 minute timeout
                output = stdout + stderr
                
                passed = 0
                failed = 0
                
                # Look specifically for pytest summary line like "=== 5 passed, 1 failed in 2.45s ==="
                summary_line_pattern = re.compile(r'=+\s*(.*?passed.*?|.*?failed.*?)\s*in\s*[\d\.]+s\s*=+', re.IGNORECASE)
                for line in output.split('\n'):
                    if summary_line_pattern.search(line):
                        # Extract counts for passed and failed
                        passed_match = re.search(r'(\d+)\s+passed', line)
                        failed_match = re.search(r'(\d+)\s+failed', line)
                        if passed_match:
                            passed = int(passed_match.group(1))
                        if failed_match:
                            failed = int(failed_match.group(1))
                        break  # Only parse the first summary line
                
                return {
                    'status': 'success',
                    'result': output,
                    'passed': passed,
                    'failed': failed,
                    'return_code': process.returncode
                }
            except subprocess.TimeoutExpired:
                process.kill()
                # Ensure process resources are cleaned up
                try:
                    process.communicate()
                except Exception:
                    pass
                return {
                    'status': 'error',
                    'error': 'Test execution timed out (5 minute limit)',
                    'passed': 0,
                    'failed': 0
                }
        except Exception as e:
            logger.error(f"Test execution error: {e}")
            return {
                'status': 'error',
                'error': str(e),
                'passed': 0,
                'failed': 0
            }

    def _handle_generate_code(self, data: str) -> Dict[str, Any]:
        """
        Handle code generation request using the code generation agent.
        
        Args:
            data: Goal/task description for code generation
            
        Returns:
            Dict with generated code modifications and confidence scores
        """
        logger.info(f"Code generation request received: {data}")
        
        try:
            from agents.code_generation_agent import CodeGenerationAgent
            from agents.goal_analysis_agent import GoalAnalysisAgent
            from agents.task_decomposition_agent import TaskDecompositionAgent
            
            # First analyze the goal
            goal_agent = GoalAnalysisAgent()
            goal_obj = goal_agent.parse_goal(data)
            
            # Decompose into tasks
            task_agent = TaskDecompositionAgent()
            task_tree = task_agent.decompose_goal(goal_obj)
            
            # Generate code for the first task (or all tasks)
            code_agent = CodeGenerationAgent()
            all_modifications = []
            
            # Get all tasks from the tree
            tasks = task_tree.get_all_tasks()
            
            # Limit number of tasks to generate code for (configurable via environment variable)
            MAX_CODE_GEN_TASKS = int(os.environ.get('MAX_CODE_GEN_TASKS', '3'))
            
            for task in tasks[:MAX_CODE_GEN_TASKS]:
                try:
                    implementation = code_agent.generate_implementation(task)
                    
                    # Extract file modifications with confidence scores
                    for file_mod in implementation.file_modifications:
                        all_modifications.append({
                            'filePath': file_mod.file_path,
                            'modificationType': file_mod.modification_type,
                            'description': file_mod.description,
                            'codeSnippet': file_mod.code_snippet,
                            'lineStart': getattr(file_mod, 'line_start', None),
                            'lineEnd': getattr(file_mod, 'line_end', None),
                            # TODO: Replace with actual confidence calculation based on code quality metrics
                            'confidence': float(os.environ.get('DEFAULT_CODE_MOD_CONFIDENCE', '0.75'))
                        })
                    
                except Exception as e:
                    logger.warning(f"Failed to generate code for task {task.id}: {e}")
                    continue
            
            return {
                'status': 'success',
                'goal': data,
                'file_modifications': all_modifications,
                'total_modifications': len(all_modifications)
            }
            
        except ImportError as e:
            logger.warning(f"Code generation agents not available: {e}")
        except Exception as e:
            logger.error(f"Code generation error: {e}")
        
        # Fallback: Return error status when agents not available
        return {
            'status': 'error',
            'error': 'Code generation agents not available. Please configure agents.',
            'goal': data,
            'file_modifications': [],
            'total_modifications': 0
        }

    def _handle_apply_feedback(self, data: str) -> Dict[str, Any]:
        """
        Handle feedback application for continuous learning.
        
        Args:
            data: JSON string containing feedback item
            
        Returns:
            Dict with confirmation of feedback storage
        """
        logger.info(f"Feedback received")
        
        try:
            feedback_data = json.loads(data)
            
            # Store feedback (could be saved to file, database, etc.)
            # For now, just log it
            logger.info(f"Feedback stored: {feedback_data.get('type')} - {feedback_data.get('id')}")
            
            # In the future, this could:
            # - Update confidence scores based on approval patterns
            # - Train models on user preferences
            # - Adjust code generation strategies
            
            return {
                'status': 'success',
                'message': 'Feedback received and stored',
                'feedback_id': feedback_data.get('id')
            }
            
        except json.JSONDecodeError as e:
            logger.error(f"Invalid feedback JSON: {e}")
            return {
                'status': 'error',
                'error': 'Invalid feedback format'
            }
        except Exception as e:
            logger.error(f"Feedback handling error: {e}")
            return {
                'status': 'error',
                'error': str(e)
            }

    def _handle_get_confidence(self, data: str) -> Dict[str, Any]:
        """
        Handle confidence score request for a proposed change.
        
        Args:
            data: JSON string with change details
            
        Returns:
            Dict with confidence score (0-1)
        """
        logger.info(f"Confidence request received")
        
        try:
            change_data = json.loads(data)
            
            # Calculate confidence based on various factors:
            # - Past approval patterns
            # - Complexity of the change
            # - Similarity to previously approved changes
            # - Code quality metrics
            
            # For now, return a default confidence score
            # In the future, this would use ML models or heuristics
            
            file_path = change_data.get('filePath', '')
            mod_type = change_data.get('modificationType', '')
            
            # Simple heuristic: higher confidence for modifications, lower for creates/deletes
            confidence = 0.8 if mod_type == 'modify' else 0.6
            
            # Adjust based on file type
            if file_path.endswith('.py'):
                confidence += 0.1  # Higher confidence for Python files
            elif file_path.endswith(('.cpp', '.h')):
                confidence -= 0.1  # Lower confidence for C++ files (more complex)
            
            # Ensure within bounds
            confidence = max(0.0, min(1.0, confidence))
            
            return {
                'status': 'success',
                'confidence': confidence,
                'factors': {
                    'modification_type': mod_type,
                    'file_type': file_path.split('.')[-1] if '.' in file_path else 'unknown'
                }
            }
            
        except json.JSONDecodeError as e:
            logger.error(f"Invalid confidence request JSON: {e}")
            return {
                'status': 'error',
                'error': 'Invalid request format'
            }
        except Exception as e:
            logger.error(f"Confidence calculation error: {e}")
            return {
                'status': 'error',
                'error': str(e)
            }
    
    # MCP/UE Integration Handlers
    
    def _get_mcp_server(self):
        """Get or create the MCP server instance."""
        if self._mcp_server is None:
            try:
                mcp_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../..'))
                if mcp_path not in sys.path:
                    sys.path.insert(0, mcp_path)
                from mcp_server import UnrealMCPServer
                self._mcp_server = UnrealMCPServer()
                logger.info("MCP server instance created")
            except ImportError as e:
                logger.error(f"Failed to import MCP server: {e}")
                return None
        return self._mcp_server
    
    def _handle_mcp_connect(self, data: str) -> Dict[str, Any]:
        """
        Handle MCP connection request to Unreal Engine.
        
        Returns connection status and project information if successful.
        """
        logger.info("MCP connect request received")
        
        mcp = self._get_mcp_server()
        if not mcp:
            return {
                'status': 'error',
                'error': (
                    "MCP server not available. To resolve:\n"
                    "- Ensure the 'mcp_server' module is installed and accessible in your Python environment.\n"
                    "- Check that your PYTHONPATH includes the directory containing 'mcp_server.py'.\n"
                    "- You can install the module (if available via pip) with: pip install mcp_server\n"
                    "- For manual installation, place 'mcp_server.py' in your project or site-packages directory.\n"
                    "- See the documentation for more details or contact support if you need help."
                )
            }
        
        try:
            success = mcp.start()
            
            if success:
                # Get project info after successful connection
                project_info = {}
                try:
                    result = mcp.handle_tool_call("editor_project_info", {})
                    if not result.get("isError"):
                        # Extract text from content
                        content = result.get("content", [])
                        if content and content[0].get("type") == "text":
                            project_info = {"info": content[0].get("text", "")}
                except Exception as e:
                    logger.warning(f"Could not get project info: {e}")
                
                return {
                    'status': 'success',
                    'connected': True,
                    'project_info': project_info
                }
            else:
                return {
                    'status': 'error',
                    'error': 'Failed to connect to Unreal Engine. Ensure UE is running with Python Remote Execution enabled.',
                    'connected': False
                }
        except Exception as e:
            logger.error(f"MCP connection error: {e}")
            return {
                'status': 'error',
                'error': str(e),
                'connected': False
            }
    
    def _handle_mcp_disconnect(self, data: str) -> Dict[str, Any]:
        """Handle MCP disconnection request."""
        logger.info("MCP disconnect request received")
        
        if self._mcp_server:
            try:
                self._mcp_server.stop()
                self._mcp_server = None
                return {
                    'status': 'success',
                    'message': 'Disconnected from Unreal Engine'
                }
            except Exception as e:
                logger.error(f"MCP disconnection error: {e}")
                return {
                    'status': 'error',
                    'error': str(e)
                }
        else:
            return {
                'status': 'success',
                'message': 'Not connected'
            }
    
    def _handle_mcp_status(self, data: str) -> Dict[str, Any]:
        """Handle MCP status check request."""
        logger.debug("MCP status check received")
        
        if self._mcp_server:
            try:
                connected = self._mcp_server.is_connected()
                return {
                    'status': 'success',
                    'connected': connected,
                    'server_info': self._mcp_server.get_server_info() if connected else {}
                }
            except Exception as e:
                logger.error(f"MCP status check error: {e}")
                return {
                    'status': 'error',
                    'error': str(e),
                    'connected': False
                }
        else:
            return {
                'status': 'success',
                'connected': False,
                'message': 'MCP server not initialized'
            }
    
    def _handle_mcp_execute_python(self, data: str) -> Dict[str, Any]:
        """
        Handle Python code execution in Unreal Engine via MCP.
        
        Args:
            data: JSON string with 'code' field containing Python code to execute
        """
        logger.info("MCP Python execution request received")
        
        mcp = self._get_mcp_server()
        if not mcp or not mcp.is_connected():
            return {
                'status': 'error',
                'error': 'Not connected to Unreal Engine. Call mcp_connect first.'
            }
        
        try:
            request_data = json.loads(data) if isinstance(data, str) else data
            code = request_data.get('code', '')
            
            if not code:
                return {
                    'status': 'error',
                    'error': 'No code provided'
                }
            
            result = mcp.handle_tool_call("editor_run_python", {"code": code})
            
            return {
                'status': 'success' if not result.get('isError') else 'error',
                'result': result
            }
        except json.JSONDecodeError as e:
            logger.error(f"Invalid JSON in Python execution request: {e}")
            return {
                'status': 'error',
                'error': 'Invalid request format. Expected JSON with "code" field.'
            }
        except Exception as e:
            logger.error(f"Python execution error: {e}")
            return {
                'status': 'error',
                'error': str(e)
            }
    
    def _handle_mcp_console_command(self, data: str) -> Dict[str, Any]:
        """
        Handle console command execution in Unreal Engine via MCP.
        
        Args:
            data: JSON string with 'command' field
        """
        logger.info("MCP console command request received")
        
        mcp = self._get_mcp_server()
        if not mcp or not mcp.is_connected():
            return {
                'status': 'error',
                'error': 'Not connected to Unreal Engine. Call mcp_connect first.'
            }
        
        try:
            request_data = json.loads(data) if isinstance(data, str) else data
            command = request_data.get('command', '')
            
            if not command:
                return {
                    'status': 'error',
                    'error': 'No command provided'
                }
            
            result = mcp.handle_tool_call("editor_console_command", {"command": command})
            
            return {
                'status': 'success' if not result.get('isError') else 'error',
                'result': result
            }
        except json.JSONDecodeError as e:
            logger.error(f"Invalid JSON in console command request: {e}")
            return {
                'status': 'error',
                'error': 'Invalid request format. Expected JSON with "command" field.'
            }
        except Exception as e:
            logger.error(f"Console command error: {e}")
            return {
                'status': 'error',
                'error': str(e)
            }
    
    def _handle_mcp_list_tools(self, data: str) -> Dict[str, Any]:
        """Handle request to list all available MCP tools."""
        logger.debug("MCP list tools request received")
        
        mcp = self._get_mcp_server()
        if not mcp:
            return {
                'status': 'error',
                'error': 'MCP server not available'
            }
        
        try:
            tools = mcp.list_tools()
            return {
                'status': 'success',
                'tools': tools
            }
        except Exception as e:
            logger.error(f"List tools error: {e}")
            return {
                'status': 'error',
                'error': str(e)
            }
    
    def _handle_mcp_call_tool(self, data: str) -> Dict[str, Any]:
        """
        Handle generic MCP tool call.
        
        Args:
            data: JSON string with 'tool' and 'arguments' fields
        """
        logger.info("MCP tool call request received")
        
        mcp = self._get_mcp_server()
        if not mcp or not mcp.is_connected():
            return {
                'status': 'error',
                'error': 'Not connected to Unreal Engine. Call mcp_connect first.'
            }
        
        try:
            request_data = json.loads(data) if isinstance(data, str) else data
            tool_name = request_data.get('tool', '')
            arguments = request_data.get('arguments', {})
            
            if not tool_name:
                return {
                    'status': 'error',
                    'error': 'No tool name provided'
                }
            
            result = mcp.handle_tool_call(tool_name, arguments)
            
            return {
                'status': 'success' if not result.get('isError') else 'error',
                'result': result
            }
        except json.JSONDecodeError as e:
            logger.error(f"Invalid JSON in tool call request: {e}")
            return {
                'status': 'error',
                'error': 'Invalid request format. Expected JSON with "tool" and "arguments" fields.'
            }
        except Exception as e:
            logger.error(f"Tool call error: {e}")
            return {
                'status': 'error',
                'error': str(e)
            }
    
    # Log Access Handlers
    
    def _handle_get_ue_logs(self, data: str) -> Dict[str, Any]:
        """
        Handle request to get recent UE logs.
        
        Args:
            data: JSON string with optional 'limit' field (default: 10)
        """
        logger.debug("Get UE logs request received")
        
        try:
            ue_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../..'))
            if ue_path not in sys.path:
                sys.path.insert(0, ue_path)
            from ue_log_capture import UELogCapture
            
            # Parse request
            try:
                request_data = json.loads(data) if data and isinstance(data, str) else {}
                limit = request_data.get('limit', 10)
            except (json.JSONDecodeError, AttributeError):
                limit = 10
            
            # Get log files
            capture = UELogCapture()
            log_files = capture.list_log_files(limit=limit)
            
            # Format response
            logs = []
            for log_path in log_files:
                try:
                    stat = log_path.stat()
                    logs.append({
                        'filename': log_path.name,
                        'path': str(log_path),
                        'size': stat.st_size,
                        'modified': stat.st_mtime
                    })
                except Exception as e:
                    logger.warning(f"Could not stat log file {log_path}: {e}")
            
            return {
                'status': 'success',
                'logs': logs,
                'count': len(logs)
            }
        except ImportError as e:
            logger.error(f"Could not import ue_log_capture: {e}")
            return {
                'status': 'error',
                'error': 'UE log capture module not available'
            }
        except Exception as e:
            logger.error(f"Get UE logs error: {e}")
            return {
                'status': 'error',
                'error': str(e)
            }
    
    def _handle_list_ue_logs(self, data: str) -> Dict[str, Any]:
        """
        Handle request to list UE log files (alias for get_ue_logs).
        
        Returns list of log files with metadata.
        """
        return self._handle_get_ue_logs(data)
    
    def _handle_read_ue_log(self, data: str) -> Dict[str, Any]:
        """
        Handle request to read a specific UE log file.
        
        Args:
            data: JSON string with 'filename' or 'path' field
        """
        logger.info("Read UE log request received")
        
        try:
            ue_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../..'))
            if ue_path not in sys.path:
                sys.path.insert(0, ue_path)
            from ue_log_capture import UELogCapture
            from pathlib import Path
            
            # Parse request
            request_data = json.loads(data) if isinstance(data, str) else data
            filename = request_data.get('filename', '')
            file_path = request_data.get('path', '')
            max_lines = request_data.get('max_lines', 1000)  # Limit to prevent huge responses
            
            if not filename and not file_path:
                return {
                    'status': 'error',
                    'error': 'No filename or path provided'
                }
            
            capture = UELogCapture()
            
            # Determine the file path
            if file_path:
                log_path = Path(file_path)
            else:
                log_path = capture.log_dir / filename
            
            # Security check: ensure the file is in the logs directory
            # Use is_relative_to for safer path validation (Python 3.9+)
            try:
                log_path = log_path.resolve()
                log_dir = capture.log_dir.resolve()
                
                # Check if log_path is within log_dir
                try:
                    # Python 3.9+ method (safer against symlinks)
                    if not log_path.is_relative_to(log_dir):
                        return {
                            'status': 'error',
                            'error': 'Access denied: File must be in logs directory'
                        }
                except AttributeError:
                    # Fallback for Python < 3.9
                    if not str(log_path).startswith(str(log_dir)):
                        return {
                            'status': 'error',
                            'error': 'Access denied: File must be in logs directory'
                        }
            except Exception as e:
                logger.error(f"Path resolution error: {e}")
                return {
                    'status': 'error',
                    'error': 'Invalid file path'
                }
            
            # Read the log file
            if not log_path.exists():
                return {
                    'status': 'error',
                    'error': f'Log file not found: {log_path.name}'
                }
            
            try:
                with open(log_path, 'r', encoding='utf-8') as f:
                    lines = f.readlines()
                    
                    # Limit lines if necessary
                    if len(lines) > max_lines:
                        lines = lines[-max_lines:]  # Get last N lines
                        truncated = True
                    else:
                        truncated = False
                    
                    content = ''.join(lines)
                
                return {
                    'status': 'success',
                    'filename': log_path.name,
                    'content': content,
                    'line_count': len(lines),
                    'truncated': truncated,
                    'max_lines': max_lines
                }
            except Exception as e:
                logger.error(f"Error reading log file: {e}")
                return {
                    'status': 'error',
                    'error': f'Error reading file: {str(e)}'
                }
        except json.JSONDecodeError as e:
            logger.error(f"Invalid JSON in read log request: {e}")
            return {
                'status': 'error',
                'error': 'Invalid request format. Expected JSON with "filename" or "path" field.'
            }
        except ImportError as e:
            logger.error(f"Could not import ue_log_capture: {e}")
            return {
                'status': 'error',
                'error': 'UE log capture module not available'
            }
        except Exception as e:
            logger.error(f"Read log error: {e}")
            return {
                'status': 'error',
                'error': str(e)
            }
    
    # RAG System Handlers
    
    def _handle_ingest(self, data: str) -> Dict[str, Any]:
        """
        Handle document ingestion request.
        
        Args:
            data: JSON string with ingestion parameters:
                - docs_dir: Directory containing documents to ingest
                - persist_dir: Directory to persist the vector database
                - progress_file: Optional path to file for progress updates
                - force_reingest: Whether to force re-ingestion of all files
                - collection_name: ChromaDB collection name
            
        Returns:
            Dict with ingestion statistics
        """
        logger.info("Ingest request received")
        
        # Check if ingestion is already in progress
        with self._ingestion_lock:
            if self._is_ingesting:
                return {
                    'status': 'error',
                    'error': 'Ingestion is already in progress. Please wait for it to complete.'
                }
            self._is_ingesting = True
        
        try:
            # Parse ingestion parameters
            params = json.loads(data) if isinstance(data, str) else data
            docs_dir = params.get('docs_dir', '')
            progress_file = params.get('progress_file', None)
            force_reingest = params.get('force_reingest', False)
            collection_name = params.get('collection_name', 'adastrea_docs')
            persist_dir = params.get('persist_dir', './chroma_db')
            
            # Validate required parameter
            if not docs_dir:
                return {
                    'status': 'error',
                    'error': 'docs_dir parameter is required and must be a valid directory path'
                }
            
            # Validate path safety for docs_dir
            is_valid, error_msg = self._validate_path_safety(docs_dir, require_exists=True)
            if not is_valid:
                return {
                    'status': 'error',
                    'error': f'Invalid docs_dir: {error_msg}'
                }
            
            # Validate path safety for persist_dir (doesn't need to exist yet)
            is_valid, error_msg = self._validate_path_safety(persist_dir, require_exists=False)
            if not is_valid:
                return {
                    'status': 'error',
                    'error': f'Invalid persist_dir: {error_msg}'
                }
            
            # Validate path safety for progress_file if provided
            if progress_file:
                is_valid, error_msg = self._validate_path_safety(progress_file, require_exists=False)
                if not is_valid:
                    return {
                        'status': 'error',
                        'error': f'Invalid progress_file: {error_msg}'
                    }
            
            # Import ingestion module
            try:
                from rag_ingestion import ingest_documents
            except ImportError as e:
                logger.error(f"Failed to import rag_ingestion module: {e}")
                return {
                    'status': 'error',
                    'error': (
                        f'Ingestion module not available: {str(e)}\n\n'
                        'To resolve this issue:\n'
                        '1. Ensure all Python dependencies are installed (see requirements.txt)\n'
                        '2. Run: pip install -r requirements.txt\n'
                        '3. Restart the Python backend'
                    )
                }
            
            # Log all parameters for debugging
            logger.info(
                f"Starting ingestion: docs_dir={docs_dir}, persist_dir={persist_dir}, "
                f"collection_name={collection_name}, force_reingest={force_reingest}, "
                f"progress_file={progress_file}"
            )
            
            # Start ingestion
            stats = ingest_documents(
                docs_dir=docs_dir,
                collection_name=collection_name,
                persist_dir=persist_dir,
                progress_file=progress_file,
                force_reingest=force_reingest
            )
            
            # Build message from stats
            if isinstance(stats, dict) and all(k in stats for k in ('added', 'updated', 'skipped')):
                # Validate that values are integers
                try:
                    added = int(stats['added'])
                    updated = int(stats['updated'])
                    skipped = int(stats['skipped'])
                    message = (
                        f"Ingestion completed: {added} added, "
                        f"{updated} updated, {skipped} skipped"
                    )
                except (ValueError, TypeError):
                    # Fallback if values aren't numeric
                    message = f"Ingestion completed. Stats: {stats}"
            else:
                # Fallback for unexpected stats structure
                message = f"Ingestion completed. Stats: {stats}"
            
            logger.info(f"Ingestion completed successfully: {stats}")
            return {
                'status': 'success',
                'stats': stats,
                'message': message
            }
            
        except json.JSONDecodeError as e:
            logger.error(f"Invalid JSON in ingest request: {e}")
            return {
                'status': 'error',
                'error': f'Invalid request format: {str(e)}'
            }
        except Exception as e:
            logger.error(f"Ingestion error: {e}", exc_info=True)
            return {
                'status': 'error',
                'error': f'Ingestion failed: {str(e)}'
            }
        finally:
            # Always release the lock
            with self._ingestion_lock:
                self._is_ingesting = False
    
    def _handle_clear_history(self, data: str) -> Dict[str, Any]:
        """
        Handle clear conversation history request.
        
        This clears the conversation history in the RAG query agent if available.
        Returns error if RAG system is not available for more explicit feedback.
        
        Args:
            data: Ignored
            
        Returns:
            Success response or error if RAG system unavailable
        """
        logger.info("Clear history request received")
        
        try:
            # Try to use RAG query agent if available
            from rag_query import RAGQueryAgent
            
            # Use helper method to find persist directory
            persist_directory = self._find_persist_directory()
            
            if not persist_directory:
                logger.warning("No persist directory found for clear_history")
                return {
                    'status': 'error',
                    'error': 'RAG database not found. Please ingest documents first.'
                }
            
            # Use default collection name consistent with ingest handler
            query_agent = RAGQueryAgent(
                collection_name='adastrea_docs',
                persist_directory=persist_directory
            )
            query_agent.clear_conversation_history()
            
            logger.info("Conversation history cleared successfully")
            return {
                'status': 'success',
                'message': 'Conversation history cleared'
            }
            
        except ImportError as e:
            # RAG module not available
            logger.warning(f"RAG query module not available for clear_history: {e}")
            return {
                'status': 'error',
                'error': 'RAG system is not available. Ensure all dependencies are installed.',
                'details': str(e)
            }
        except Exception as e:
            # Other errors during clear history
            logger.error(f"Failed to clear history via RAG agent: {e}", exc_info=True)
            return {
                'status': 'error',
                'error': 'Failed to clear conversation history.',
                'details': str(e)
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
