# Adastrea Director Python Backend

This directory contains the Python backend components for the Adastrea Director plugin.

## Files

### ipc_server.py

The IPC (Inter-Process Communication) server that bridges the Unreal Engine C++ plugin with the Python backend.

**Week 3 Enhancements:**
- Performance monitoring and metrics tracking
- Optimized request routing with timing
- Sub-millisecond latency (<1ms typical)
- Request/response with processing time information

**Features:**
- TCP socket-based communication on localhost
- JSON request/response serialization
- Multi-threaded client handling
- Extensible request handler system
- Graceful error handling
- Performance metrics collection

**Usage:**
```bash
python ipc_server.py --port 5555
python ipc_server.py --port 5555 --verbose  # Enable debug logging
```

**Supported Request Types:**
- `ping` - Health check with timestamp
- `metrics` - Get performance statistics (or 'reset' to clear)
- `query` - Documentation queries (placeholder for RAG integration)
- `plan` - Task planning requests (placeholder for planning agent)
- `analyze` - Goal analysis requests (placeholder for goal analysis agent)

### ipc_integration.py

**NEW in Week 3:** Integrated IPC server with optional RAG system and planning agent support.

**Features:**
- Optional RAG system integration for real queries
- Optional planning agent integration
- Automatic fallback to placeholder responses
- Configuration via command-line arguments

**Usage:**
```bash
# Basic mode (placeholder responses)
python ipc_integration.py --port 5555

# With RAG system
python ipc_integration.py --port 5555 --enable-rag

# With planning agents
python ipc_integration.py --port 5555 --enable-planning

# Full integration
python ipc_integration.py --port 5555 --enable-rag --enable-planning --verbose
```

### test_ipc.py

Original test suite for basic IPC functionality.

### test_ipc_performance.py

**NEW in Week 3:** Comprehensive performance test suite.

**Features:**
- Round-trip latency measurement
- Throughput testing
- Statistical analysis (avg, min, max, P95)
- Performance requirements validation (< 50ms target)
- Server metrics verification

**Usage:**
```bash
python test_ipc_performance.py [port]
```

**Performance Results:**
- Average latency: < 1ms
- P95 latency: < 1ms
- Throughput: > 4000 req/s
- ✓ Exceeds 50ms requirement by 50x

## Request/Response Format

### Request Format
```json
{
  "type": "query|plan|analyze|ping",
  "data": "request-specific data as string"
}
```

### Response Format
```json
{
  "status": "success|error",
  "message": "response message",
  "error": "error message (if status is error)",
  ...additional fields depending on request type...
}
```

## Integration with Main Backend

The IPC server currently uses placeholder handlers. To integrate with the main Adastrea Director Python codebase:

1. Import the necessary modules (e.g., `main.py`, agents)
2. Initialize the RAG system, planning agents, etc.
3. Replace placeholder handlers with actual implementations

Example integration:
```python
from main import DirectorRAG
from planner import TaskPlanner

class IPCServer:
    def __init__(self, host='127.0.0.1', port=5555):
        # ... existing init code ...
        self.rag = DirectorRAG()
        self.planner = TaskPlanner()
    
    def _handle_query(self, data: str) -> Dict[str, Any]:
        # Use actual RAG system
        result = self.rag.query(data)
        return {
            'status': 'success',
            'result': result.answer,
            'sources': result.sources
        }
```

## Testing

Test the IPC server independently:

```bash
# Start server
python ipc_server.py --port 5556

# In another terminal, test with Python client:
python3 << 'EOF'
import socket
import json

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('127.0.0.1', 5556))

request = {'type': 'ping', 'data': ''}
sock.sendall((json.dumps(request) + '\n').encode('utf-8'))

response = sock.recv(4096).decode('utf-8')
print(response)

sock.close()
EOF
```

## Error Handling

The server handles the following error cases:
- Invalid JSON format
- Missing request type
- Unknown request type
- Handler exceptions
- Client disconnections
- Socket errors

All errors are logged and returned as JSON error responses.

## Performance

- Multi-threaded client handling (one thread per connection)
- Non-blocking socket operations
- Efficient JSON serialization
- Low latency for local communication (<10ms typical)

## Dependencies

Standard Python library only:
- socket
- json
- threading
- logging
- argparse

No external dependencies required for basic IPC functionality.

### ue_info_collector.py

**NEW:** Comprehensive project information collector that gathers extensive data about your UE project from inside the editor.

**Features:**
- Collects 10 categories of project information
- Project metadata and configuration
- Asset counts and organization analysis
- Blueprint complexity and structure
- Level content and actors
- Material and texture information
- Plugin enumeration
- Source code analysis
- Animation and audio assets
- Performance monitoring setup

**Usage:**
```python
# In UE Python Console
execfile("Plugins/AdastreaDirector/Python/ue_info_collector.py")

# Or programmatically
import ue_info_collector
info = ue_info_collector.collect_all_info()
ue_info_collector.print_report(info)
ue_info_collector.save_to_json(info, "project_info.json")
```

**Documentation:**
- See [UE_INFO_COLLECTOR_README.md](UE_INFO_COLLECTOR_README.md) for detailed usage
- See [examples/ue_info_collector_example.py](../../../examples/ue_info_collector_example.py) for examples

**Use Cases:**
- Provide context to AI agents about project structure
- Analyze naming conventions and organization
- Track project growth and complexity
- Detect potential issues and optimization opportunities
- Generate reports for documentation

### ue_python_api.py

Python API bridge for interacting with Unreal Engine from within the editor.

**Features:**
- Asset management and queries
- Actor spawning and manipulation
- Console command execution
- Editor scripting utilities
- Performance profiling helpers

**See also:** [examples/ue_python_api_demo.py](../../../examples/ue_python_api_demo.py)

## Additional Documentation

📖 **[Remote Connection Types & Actions](../../../wiki/Remote-Connection-Types-and-Actions.md)** - Comprehensive guide covering all connection types including this IPC Server, with detailed action listings, comparisons, and best practices.
