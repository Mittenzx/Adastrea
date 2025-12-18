# Unreal Engine Plugin Connection Guide

**Complete guide to understanding, troubleshooting, and optimizing the Adastrea Director UE plugin connection**

---

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Connection Flow](#connection-flow)
3. [How to Make Connection Perfect](#how-to-make-connection-perfect)
4. [Troubleshooting Guide](#troubleshooting-guide)
5. [Performance Optimization](#performance-optimization)
6. [Advanced Configuration](#advanced-configuration)
7. [Security Best Practices](#security-best-practices)

---

## Architecture Overview

The Adastrea Director plugin uses a **hybrid architecture** with three main components:

```
┌────────────────────────────────────────────────────────────┐
│                  Unreal Engine Editor                      │
│  ┌──────────────────────────────────────────────────────┐ │
│  │           C++ Plugin (AdastreaDirector)              │ │
│  │  ┌────────────────┐  ┌─────────────────────────┐    │ │
│  │  │  UI (Slate)    │  │   IPCClient             │    │ │
│  │  │  - Query Tab   │  │   - Socket connection   │    │ │
│  │  │  - Ingest Tab  │  │   - JSON serialization  │    │ │
│  │  │  - Dashboard   │  │   - Error handling      │    │ │
│  │  └────────┬───────┘  └──────────┬──────────────┘    │ │
│  │           │                     │                     │ │
│  │           └──────────┬──────────┘                     │ │
│  │                      │                                │ │
│  │           ┌──────────▼──────────────────┐            │ │
│  │           │   PythonBridge              │            │ │
│  │           │   - Process management      │            │ │
│  │           │   - Lifecycle control       │            │ │
│  │           │   - Auto-restart logic      │            │ │
│  │           └──────────┬──────────────────┘            │ │
│  └──────────────────────┼─────────────────────────────┘ │
└────────────────────────┼────────────────────────────────┘
                         │ IPC (JSON over TCP Socket)
                         │ Port: 5555 (default for UE plugin)
┌────────────────────────▼────────────────────────────────┐
│             Python Backend (Subprocess)                  │
│  ┌──────────────────────────────────────────────────┐  │
│  │            ipc_server.py                          │  │
│  │  - Socket server on port 5555 (UE plugin)       │  │
│  │  - Request routing and handling                  │  │
│  │  - Performance metrics tracking                  │  │
│  │  - Graceful shutdown                             │  │
│  └───────────┬──────────────────────────────────────┘  │
│              │                                          │
│  ┌───────────▼────────┐  ┌────────────────────────┐   │
│  │  RAG System        │  │  Planning Agents       │   │
│  │  - rag_query.py    │  │  - Goal analysis       │   │
│  │  - rag_ingestion.py│  │  - Task decomposition  │   │
│  └────────────────────┘  └────────────────────────┘   │
│              │                                          │
│  ┌───────────▼────────────────────────────────────┐   │
│  │         ChromaDB (Vector Database)             │   │
│  └────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

### Key Components

#### 1. **C++ Plugin (Unreal Engine Side)**
- **IPCClient**: Manages TCP socket connection to Python backend
- **PythonBridge**: Handles Python process lifecycle (start, stop, restart)
- **Slate UI**: User interface for queries, ingestion, and monitoring

#### 2. **Python Backend (External Process)**
- **IPC Server**: Listens on port 5555 (UE plugin default), routes requests to appropriate handlers
- **RAG System**: Handles document ingestion and query processing
- **Agent System**: Provides planning, profiling, and other AI capabilities

#### 3. **Communication Protocol**
- **Transport**: TCP sockets (localhost only for security)
- **Format**: JSON messages with newline delimiters
- **Port**: 5555 (default for UE plugin; note: standalone GUI may use port 8765)
- **Latency**: < 1ms for simple requests, 100-500ms for RAG queries

---

## Connection Flow

### Startup Sequence

```
1. User opens Adastrea Director panel in UE Editor
   ↓
2. PythonBridge checks if Python backend is running
   ↓
3. If not running:
   a. Launch Python subprocess: python ipc_server.py --port 5555
   b. Wait for socket to become available (timeout: 30 seconds)
   c. Verify connection with ping request
   ↓
4. IPCClient establishes socket connection
   ↓
5. Send initial status request to verify backend is ready
   ↓
6. UI becomes active and ready for user interaction
```

### Request-Response Cycle

```
User action (e.g., "Send Query")
   ↓
1. UI collects input data
   ↓
2. IPCClient serializes to JSON:
   {
     "type": "query",
     "query": "What is Unreal Engine?",
     "conversation_id": "uuid-123"
   }
   ↓
3. Send via TCP socket with newline delimiter
   ↓
4. Python IPC server receives and parses JSON
   ↓
5. Route to appropriate handler (e.g., RAGQueryAgent)
   ↓
6. Process request (may involve LLM API calls, vector DB queries)
   ↓
7. Python serializes response to JSON:
   {
     "status": "success",
     "response": "Unreal Engine is...",
     "sources": [...],
     "processing_time_ms": 234.5
   }
   ↓
8. Send response back via socket
   ↓
9. IPCClient receives and parses response
   ↓
10. UI updates with results
```

### Shutdown Sequence

```
1. User closes UE Editor or plugin panel
   ↓
2. PythonBridge sends shutdown request to Python backend
   ↓
3. Python IPC server closes socket, saves state if needed
   ↓
4. Python process exits gracefully
   ↓
5. IPCClient closes socket connection
   ↓
6. Cleanup complete
```

---

## How to Make Connection Perfect

### 1. ✅ Ensure Prerequisites

**Check Python Installation:**
```bash
# Verify Python version (3.9+ required)
python --version

# Verify Python is in PATH
which python  # Linux/Mac
where python  # Windows

# Test Python can import required modules
python -c "import socket, json, sys; print('OK')"
```

**Check Dependencies:**
```bash
# Navigate to plugin Python directory
cd YourProject/Plugins/AdastreaDirector/Python

# Install dependencies
pip install -r requirements.txt

# Verify ChromaDB installation
python -c "import chromadb; print('ChromaDB OK')"
```

### 2. ✅ Verify Port Availability

**Check if port 5555 is available:**
```bash
# Linux/Mac
netstat -an | grep 5555
lsof -i :5555

# Windows
netstat -an | findstr 5555
```

**If port is in use, change it:**
1. Edit `AdastreaDirector.uplugin` → change port in settings
2. Or use command line: `python ipc_server.py --port 5556`

### 3. ✅ Test Connection Manually

**Test IPC Server Standalone:**
```bash
# Start server manually
cd YourProject/Plugins/AdastreaDirector/Python
python ipc_server.py --port 5555

# In another terminal, test connection
python test_ipc.py 5555

# Expected output:
# ✓ IPC connection test passed
# ✓ Ping latency: <1ms
# ✓ Query test passed
```

### 4. ✅ Enable Debug Logging

**In Unreal Engine:**
1. Edit → Project Settings → Plugins → Adastrea Director
2. Enable "Verbose Logging"
3. Restart UE Editor
4. Check Output Log for connection details

**In Python Backend:**
```bash
# Start server with debug logging
python ipc_server.py --port 5555 --debug

# Or set environment variable
export LOG_LEVEL=DEBUG  # Linux/Mac
set LOG_LEVEL=DEBUG     # Windows
```

### 5. ✅ Verify File Permissions

**Ensure Python files are executable:**
```bash
# Linux/Mac
chmod +x Plugins/AdastreaDirector/Python/*.py

# Verify
ls -l Plugins/AdastreaDirector/Python/ipc_server.py
# Should show: -rwxr-xr-x (executable bits set)
```

### 6. ✅ Configure Firewall

**Allow localhost connections:**
- Port 5555 should allow connections from 127.0.0.1 (localhost only)
- No external network access needed
- Check firewall/antivirus software settings

**Windows Firewall:**
```powershell
# Allow Python through firewall (if needed)
New-NetFirewallRule -DisplayName "Python IPC Server" -Direction Inbound -LocalPort 5555 -Protocol TCP -Action Allow
```

### 7. ✅ Set Up Logging Directory

**Ensure logs directory exists:**
```bash
# Create logs directory in project root
mkdir -p YourProject/Plugins/AdastreaDirector/Python/logs

# Verify writable
touch YourProject/Plugins/AdastreaDirector/Python/logs/test.txt
rm YourProject/Plugins/AdastreaDirector/Python/logs/test.txt
```

### 8. ✅ Optimize Python Environment

**Use Virtual Environment (Recommended):**
```bash
# Create virtual environment
cd YourProject/Plugins/AdastreaDirector/Python
python -m venv venv

# Activate
source venv/bin/activate  # Linux/Mac
venv\Scripts\activate     # Windows

# Install dependencies
pip install -r requirements.txt

# Update plugin to use venv Python
# Edit: Plugins/AdastreaDirector/Source/.../PythonBridge.cpp
# Change: PythonExecutable = "python" 
# To: PythonExecutable = "/absolute/path/to/venv/bin/python"
```

### 9. ✅ Test End-to-End

**Complete connection test:**
```bash
# 1. Start UE Editor with your project
# 2. Open Window → Developer Tools → Adastrea Director
# 3. Check Dashboard tab:
#    - Python Backend: ✅ Running (Green)
#    - IPC Connection: ✅ Connected (Green)
#    - RAG System: ✅ Ready (Green)
# 4. Try a simple query in Query tab
# 5. Check Output Log for any errors
```

---

## Troubleshooting Guide

### Problem 1: "Python backend failed to start"

**Symptoms:**
- Plugin panel shows error message
- Output log: "Failed to launch Python process"
- Dashboard: Python Backend ❌ (Red)

**Solutions:**

1. **Check Python is in PATH:**
   ```bash
   python --version
   # If not found, add Python to system PATH
   ```

2. **Check Python file path:**
   - Open: `Plugins/AdastreaDirector/Source/.../PythonBridge.cpp`
   - Verify: `PythonScriptPath` points to correct location
   - Should be: `<PluginDir>/Python/ipc_server.py`

3. **Check file permissions:**
   ```bash
   chmod +x Plugins/AdastreaDirector/Python/ipc_server.py
   ```

4. **Test manually:**
   ```bash
   cd Plugins/AdastreaDirector/Python
   python ipc_server.py --port 5555
   # Should start without errors
   ```

### Problem 2: "IPC connection timeout"

**Symptoms:**
- Backend starts but connection fails
- Output log: "Timeout waiting for IPC server"
- Dashboard: IPC Connection ❌ (Red)

**Solutions:**

1. **Check port availability:**
   ```bash
   # Kill any process using port 5555
   # Linux/Mac:
   lsof -ti :5555 | xargs kill -9
   # Windows:
   netstat -ano | findstr :5555
   taskkill /PID <pid> /F
   ```

2. **Check firewall settings:**
   - Ensure localhost (127.0.0.1) connections allowed
   - Temporarily disable firewall to test

3. **Increase timeout:**
   - Edit: `PythonBridge.cpp`
   - Change: `ConnectionTimeout = 30.0f`
   - To: `ConnectionTimeout = 60.0f`
   - Rebuild plugin

4. **Check socket binding:**
   ```bash
   # Verify server is listening
   netstat -an | grep 5555
   # Should show: 127.0.0.1:5555 LISTEN
   ```

### Problem 3: "Requests fail with timeout"

**Symptoms:**
- Connection established but queries fail
- Output log: "Request timeout"
- Queries return errors

**Solutions:**

1. **Check Python dependencies:**
   ```bash
   cd Plugins/AdastreaDirector/Python
   pip install -r requirements.txt --upgrade
   ```

2. **Verify ChromaDB:**
   ```bash
   python -c "import chromadb; print(chromadb.__version__)"
   # Should print version number, not error
   ```

3. **Check API keys:**
   - Settings → Enter valid API key for LLM provider
   - Test with: `python test_ipc.py 5555`

4. **Enable verbose logging:**
   ```bash
   python ipc_server.py --port 5555 --debug
   # Watch for error messages
   ```

### Problem 4: "Random disconnections"

**Symptoms:**
- Connection drops intermittently
- Need to restart plugin frequently
- Output log: "Socket error"

**Solutions:**

1. **Check system resources:**
   - Memory usage (should be < 2GB)
   - CPU usage (should be < 50%)
   - Disk space (need > 1GB free)

2. **Disable auto-suspend:**
   - System Settings → Power Management
   - Disable sleep/suspend during development

3. **Increase socket timeout:**
   - Edit: `IPCClient.cpp`
   - Change: `SocketTimeout = 5.0f`
   - To: `SocketTimeout = 30.0f`

4. **Check network interface:**
   ```bash
   # Verify localhost is working
   ping 127.0.0.1
   # Should respond with <1ms latency
   ```

### Problem 5: "Slow response times"

**Symptoms:**
- Queries take > 10 seconds
- UI becomes unresponsive
- Performance degradation over time

**Solutions:**

1. **Check database size:**
   ```bash
   du -sh chroma_db/
   # If > 1GB, consider cleaning old data
   ```

2. **Optimize query parameters:**
   - Settings → Reduce retrieval_k from 6 to 3
   - Reduce fetch_k from 20 to 10

3. **Use faster LLM:**
   - Gemini 1.5 Flash (fastest)
   - GPT-3.5-turbo (fast)
   - Avoid GPT-4 for simple queries

4. **Profile Python backend:**
   ```bash
   python -m cProfile ipc_server.py --port 5555
   # Identify slow functions
   ```

---

## Performance Optimization

### Connection Performance

**Target Metrics:**
- Connection establishment: < 1 second
- Ping request: < 1ms
- Simple query: < 500ms
- RAG query (with LLM): < 3 seconds
- Document ingestion: < 2 seconds per file

### Optimization Strategies

#### 1. **Socket Configuration**

**In C++ (IPCClient.cpp):**
```cpp
// Enable TCP_NODELAY to reduce latency
int flag = 1;
setsockopt(Socket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));

// Increase buffer sizes for large transfers
int bufsize = 262144; // 256KB
setsockopt(Socket, SOL_SOCKET, SO_SNDBUF, (char*)&bufsize, sizeof(int));
setsockopt(Socket, SOL_SOCKET, SO_RCVBUF, (char*)&bufsize, sizeof(int));
```

#### 2. **JSON Serialization**

**Use efficient JSON library:**
```python
# In ipc_server.py, use orjson for faster serialization
import orjson

# Serialize
data = orjson.dumps(response_dict)

# Deserialize
request = orjson.loads(data)
```

#### 3. **Connection Pooling**

**Reuse socket connections:**
```cpp
// Keep socket alive between requests
// Don't close/reopen for each request
KeepSocketAlive = true;
```

#### 4. **Async Processing**

**Use async for long-running operations:**
```python
# In Python backend
async def process_query(query: str) -> dict:
    # Process in background
    result = await query_agent.process_async(query)
    return result
```

#### 5. **Caching**

**Cache frequent queries:**
```python
from functools import lru_cache

@lru_cache(maxsize=50)
def get_query_response(query: str) -> str:
    # Cached responses for identical queries
    return rag_agent.query(query)
```

#### 6. **Batch Processing**

**Process multiple requests together:**
```python
# Instead of multiple single requests
responses = []
for query in queries:
    responses.append(process_query(query))

# Use batch processing
responses = process_queries_batch(queries)
```

---

## Advanced Configuration

### Custom Port Configuration

**Change default port (5555) to custom port:**

1. **In Plugin Settings:**
   ```json
   // AdastreaDirector.uplugin
   {
     "Settings": {
       "IPCPort": 5556
     }
   }
   ```

2. **In Python Server:**
   ```bash
   python ipc_server.py --port 5556
   ```

3. **In C++ Code:**
   ```cpp
   // PythonBridge.cpp
   int32 IPCPort = 5556;
   ```

### Multiple Plugin Instances

**Run multiple UE Editors with different ports:**

```bash
# Editor 1
UE_IPC_PORT=5555 ./UnrealEditor.exe Project1.uproject

# Editor 2
UE_IPC_PORT=5556 ./UnrealEditor.exe Project2.uproject
```

### Remote Connection (Development Only)

**⚠️ WARNING: Only use on trusted networks!**

1. **Modify server to listen on all interfaces:**
   ```python
   # ipc_server.py
   server.bind(('0.0.0.0', 5555))  # Instead of 127.0.0.1
   ```

2. **Configure firewall:**
   ```bash
   # Allow external connections (DANGEROUS!)
   iptables -A INPUT -p tcp --dport 5555 -j ACCEPT
   ```

3. **Update C++ client:**
   ```cpp
   // IPCClient.cpp
   FString ServerAddress = "192.168.1.100";  // Remote IP
   ```

---

## Security Best Practices

### 1. **Localhost Only (Default)**

✅ **DO:** Use 127.0.0.1 for all connections
```python
server.bind(('127.0.0.1', 5555))
```

❌ **DON'T:** Expose to external network
```python
server.bind(('0.0.0.0', 5555))  # VULNERABLE!
```

### 2. **API Key Management**

✅ **DO:** Store keys in environment variables or secure config
```python
api_key = os.getenv('GEMINI_KEY')
```

❌ **DON'T:** Transmit keys over IPC
```python
# NEVER send API keys in IPC messages
request = {"api_key": "secret"}  # BAD!
```

### 3. **Input Validation**

✅ **DO:** Validate all IPC requests
```python
def validate_request(request: dict) -> bool:
    required_fields = ['type']
    return all(field in request for field in required_fields)
```

❌ **DON'T:** Trust user input blindly
```python
eval(request['code'])  # EXTREMELY DANGEROUS!
```

### 4. **Error Handling**

✅ **DO:** Sanitize error messages
```python
try:
    result = process_query(query)
except Exception as e:
    return {"error": "Query failed"}  # Generic message
```

❌ **DON'T:** Expose internal details
```python
except Exception as e:
    return {"error": str(e)}  # May leak sensitive info
```

### 5. **Rate Limiting**

✅ **DO:** Limit request rate
```python
from time import time

last_request = {}
RATE_LIMIT = 10  # requests per second

def check_rate_limit(client_id: str) -> bool:
    now = time()
    if client_id in last_request:
        if now - last_request[client_id] < 1/RATE_LIMIT:
            return False
    last_request[client_id] = now
    return True
```

### 6. **Graceful Shutdown**

✅ **DO:** Clean up resources properly
```python
def shutdown_server():
    # Save state
    save_conversation_history()
    # Close connections
    server.close()
    # Exit cleanly
    sys.exit(0)
```

---

## Testing Connection Quality

### Connection Health Check

```bash
# Run comprehensive connection test
cd Plugins/AdastreaDirector/Python
python test_ipc_performance.py 5555

# Expected output:
# ✓ Connection established: 45ms
# ✓ Ping latency: 0.8ms
# ✓ Simple request: 2.3ms
# ✓ Query request: 234ms
# ✓ Batch processing: 450ms for 10 requests
# ✓ Sustained load: 50 req/s, 0 errors
# 
# Overall: EXCELLENT ✅
```

### Monitoring Dashboard

**Use the plugin's built-in dashboard:**
1. Open Adastrea Director panel
2. Go to Dashboard tab
3. Check status indicators:
   - 🟢 Python Backend: Running
   - 🟢 IPC Connection: Connected
   - 🟢 RAG System: Ready
   - 🟢 LLM Provider: Configured
   - 🟢 Vector Database: 1,234 documents
   - 🟢 System Health: Good

### Stress Testing

```python
# stress_test.py
import socket
import json
import time
from concurrent.futures import ThreadPoolExecutor

def send_request(port: int, request_num: int):
    """Send single request"""
    sock = socket.socket()
    sock.connect(('127.0.0.1', port))
    
    request = {
        "type": "query",
        "query": f"Test query {request_num}"
    }
    
    sock.send(json.dumps(request).encode() + b'\n')
    response = sock.recv(4096).decode()
    sock.close()
    return response

# Run stress test
start = time.time()
with ThreadPoolExecutor(max_workers=10) as executor:
    futures = [executor.submit(send_request, 5555, i) for i in range(100)]
    results = [f.result() for f in futures]
duration = time.time() - start

print(f"Completed 100 requests in {duration:.2f}s")
print(f"Throughput: {100/duration:.2f} req/s")
```

---

## Conclusion

Following this guide ensures:
- ✅ Reliable connection between UE plugin and Python backend
- ✅ Optimal performance (< 1ms latency for simple requests)
- ✅ Robust error handling and automatic recovery
- ✅ Secure localhost-only communication
- ✅ Easy troubleshooting with comprehensive diagnostics

### Quick Checklist for Perfect Connection

- [ ] Python 3.9+ installed and in PATH
- [ ] All dependencies installed (`pip install -r requirements.txt`)
- [ ] Port 5555 available (or custom port configured)
- [ ] Firewall allows localhost connections
- [ ] Python files have execute permissions
- [ ] Logs directory exists and is writable
- [ ] API keys configured in settings
- [ ] Manual test passes (`python test_ipc.py 5555`)
- [ ] Dashboard shows all green indicators
- [ ] Sample query returns results in < 3 seconds

### Support

**For issues or questions:**
- Check [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
- Review [SETUP_GUIDE.md](SETUP_GUIDE.md)
- Check Output Log in UE Editor
- Check Python logs in `Plugins/AdastreaDirector/Python/logs/`
- Open an issue on GitHub

---

**Last Updated:** December 2025  
**Plugin Version:** Weeks 1-8 Complete  
**Compatibility:** UE 5.0+, Python 3.9+
