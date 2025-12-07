# Status Indicators Guide

## Overview

The Adastrea Director plugin includes comprehensive status indicator "lights" in the Dashboard tab to provide real-time visual feedback on the health of all plugin components. These indicators help quickly diagnose issues and understand the current state of the system.

## Status Light Colors

Each status indicator uses a color-coded system:

- **● Green** - Component is working correctly and operational
- **● Yellow/Orange** - Component has warnings but is still functional
- **● Red** - Component has errors or is not functional
- **● Gray** - Component status is unknown or checking

## Status Indicators

### 1. Python Process
**What it monitors:** The Python backend subprocess

**Possible States:**
- ● **Green**: Process is running with valid PID
- ● **Red**: Process is not running or crashed
- ● **Gray**: Status unknown (checking)

**Common Issues:**
- If red, the Python executable may not be configured correctly in settings
- Check that the Python backend script exists at the configured path

### 2. IPC Connection
**What it monitors:** The TCP socket connection between Unreal Engine and the Python backend

**Possible States:**
- ● **Green**: Socket connected and communicating
- ● **Red**: Socket disconnected or connection failed
- ● **Gray**: Connection status unknown

**Common Issues:**
- If red, verify the Python process is running first
- Check that no firewall is blocking localhost connections
- Default port is 5555 - ensure it's not in use by another process

### 3. Python Bridge Ready
**What it monitors:** Combined readiness of Python process + IPC connection

**Possible States:**
- ● **Green**: Both process and IPC are ready for requests
- ● **Red**: Either process is stopped or IPC is disconnected
- ● **Gray**: Status unknown

**Common Issues:**
- This is the master indicator - if this is red, check Python Process and IPC Connection
- Use the "Reconnect" button to attempt recovery

### 4. Backend Health
**What it monitors:** Overall operational status of the Python backend services

**Possible States:**
- ● **Green**: Backend is responding and operational
- ● **Red**: Backend is not responding or has errors
- ● **Gray**: Health check not performed

**Common Issues:**
- If red, check the System Logs section for Python errors
- May indicate issues with Python dependencies or LLM API configuration

### 5. Query Processing
**What it monitors:** Status of the RAG (Retrieval-Augmented Generation) query system

**Possible States:**
- ● **Green (Active)**: Currently processing a query
- ● **Green (Ready)**: Ready to process queries
- ● **Red**: Query system unavailable (backend not ready)
- ● **Gray**: Status unknown

**Common Issues:**
- Queries cannot be processed if Python Bridge is not ready
- Check Backend Health if this indicator is red

### 6. Document Ingestion
**What it monitors:** Status of the document ingestion system

**Possible States:**
- ● **Yellow (Active X%)**: Currently ingesting documents with progress percentage
- ● **Green (Ready)**: Ready to start ingestion
- ● **Red (Unavailable)**: Ingestion system not available
- ● **Gray**: Status unknown

**Common Issues:**
- Cannot ingest if Python Bridge is not ready
- If stuck in yellow, check the Ingestion tab for detailed progress

## Using the Status Indicators

### Normal Operation
In normal operation, you should see:
- Python Process: ● Green
- IPC Connection: ● Green  
- Python Bridge Ready: ● Green
- Backend Health: ● Green
- Query Processing: ● Green (Ready)
- Document Ingestion: ● Green (Ready)

### Troubleshooting Workflow

1. **Check Status Indicators** - Look for any red or gray indicators
2. **Start from the bottom** - Python Process and IPC Connection must be green first
3. **Check Detailed Status** - Review the "Detailed Status" section for error messages
4. **Review System Logs** - Check logs for specific error messages
5. **Try Reconnect** - Click the "Reconnect" button to attempt recovery
6. **Check Settings** - Verify Python executable and script paths in Settings dialog

### Update Frequency

Status indicators automatically update:
- **Every 0.5 seconds** when viewing the Dashboard tab
- **Immediately** when clicking "Refresh Status"
- **Immediately** when clicking "Reconnect"
- **Immediately** when switching to the Dashboard tab

## Integration Notes

The status indicators integrate with the existing monitoring systems:
- `FPythonBridge::IsReady()` - Overall readiness check
- `FPythonBridge::GetStatus()` - Detailed status string
- `FPythonProcessManager::IsProcessRunning()` - Process monitoring
- `FIPCClient::IsConnected()` - Socket connection monitoring

Status information is gathered without sending additional requests to the Python backend, ensuring minimal performance impact.
