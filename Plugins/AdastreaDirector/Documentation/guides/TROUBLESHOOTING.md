# Adastrea Director - Troubleshooting Guide

This guide helps you diagnose and fix common issues with the Adastrea Director plugin for Unreal Engine 5.6+.

## Table of Contents
- [Plugin Won't Load](#plugin-wont-load)
- [Initialization Failed Error](#initialization-failed-error)
- [API Key Issues](#api-key-issues)
- [Python Backend Issues](#python-backend-issues)
- [Connection Problems](#connection-problems)
- [Self-Check Failures](#self-check-failures)

---

## Plugin Won't Load

### Symptom
Plugin is not visible in the Window > Developer Tools menu, or Unreal Engine shows an error during startup.

### Solutions

1. **Check Plugin is Enabled**
   - Go to Edit > Plugins
   - Search for "Adastrea Director"
   - Ensure the plugin is checked/enabled
   - Restart Unreal Engine

2. **Check UE Version Compatibility**
   - This plugin requires Unreal Engine 5.6 or later
   - Check your UE version: Help > About Unreal Engine

3. **Check Build Errors**
   - Open Output Log (Window > Developer Tools > Output Log)
   - Look for errors related to "AdastreaDirector"
   - Common issues:
     - Missing dependencies
     - Build configuration problems
     - Module loading failures

---

## Initialization Failed Error

### Symptom
Plugin opens but shows an error screen instead of the main interface.

### Diagnosis
The error screen will show specific failure reasons. Common causes:

1. **API Key Not Configured**
   ```
   Error: Gemini API key is not configured
   ```
   
   **Solution:**
   - Click "Open Settings" button in the error screen
   - Enter your Gemini API key (get one from https://makersuite.google.com/app/apikey)
   - Save settings
   - Restart Unreal Engine

2. **Python Backend Not Running**
   ```
   Error: Python Bridge initialization failed
   ```
   
   **Solution:**
   - Verify Python is installed: Open terminal/command prompt and run `python --version` or `python3 --version`
   - Install required Python packages:
     ```bash
     pip install google-generativeai chromadb langchain
     ```
   - Check Output Log for detailed error messages
   - Ensure backend script exists: `Plugins/AdastreaDirector/Python/ipc_server.py`

3. **Backend Connection Failed**
   ```
   Error: Python backend is not ready
   ```
   
   **Solution:**
   - Check if another application is using port 5555
   - Check firewall settings (allow local connections on port 5555)
   - Review Output Log for network errors

---

## API Key Issues

### Invalid API Key

**Symptom:** Self-check shows "API Key Validation: INVALID"

**Solutions:**

1. **Verify Key Format**
   - Gemini keys typically start with "AIza"
   - OpenAI keys typically start with "sk-"
   - Ensure no extra spaces or characters

2. **Check Key Status**
   - Go to your API provider's dashboard
   - Verify the key hasn't been revoked or expired
   - Check usage limits/quotas

3. **Test Key Manually**
   
   For Gemini:
   ```python
   import google.generativeai as genai
   genai.configure(api_key="YOUR_KEY")
   list(genai.list_models())  # Should not raise error
   ```
   
   For OpenAI:
   ```python
   from openai import OpenAI
   client = OpenAI(api_key="YOUR_KEY")
   list(client.models.list())  # Should not raise error
   ```

### API Key Validation Fails

**Symptom:** Error "Cannot verify API key at this time"

**Solutions:**

1. **Check Network Connection**
   - Ensure you have internet access
   - Try accessing the API provider's website

2. **Check Proxy Settings**
   - If behind a corporate proxy, configure environment variables:
     ```
     HTTP_PROXY=http://proxy.example.com:8080
     HTTPS_PROXY=http://proxy.example.com:8080
     ```

3. **Firewall/VPN Issues**
   - Temporarily disable VPN and retry
   - Check firewall isn't blocking API requests

---

## Python Backend Issues

### Backend Won't Start

**Symptom:** Self-check shows "Python Process: NOT RUNNING"

**Solutions:**

1. **Verify Python Installation**
   ```bash
   python --version  # or python3 --version
   ```
   - Should be Python 3.8 or later

2. **Check Required Packages**
   ```bash
   pip list | grep -E "google-generativeai|chromadb|langchain"
   ```
   
   If missing, install:
   ```bash
   pip install google-generativeai chromadb langchain
   ```

3. **Check Script Permissions**
   - On Mac/Linux, ensure `ipc_server.py` is executable:
     ```bash
     chmod +x Plugins/AdastreaDirector/Python/ipc_server.py
     ```

4. **Check Python Path**
   - Plugin looks for `python` or `python3` in system PATH
   - On Windows, verify Python is in PATH:
     ```cmd
     where python
     ```

### Backend Crashes

**Symptom:** Backend starts but crashes immediately

**Solutions:**

1. **Check Python Logs**
   - Check Output Log in Unreal Engine
   - Look for Python exceptions or stack traces

2. **Test Backend Manually**
   ```bash
   cd Plugins/AdastreaDirector/Python
   python ipc_server.py --port 5555
   ```
   - Should show "IPC Server started on port 5555"
   - If it crashes, check error messages

3. **Check Dependencies Conflicts**
   ```bash
   pip check
   ```
   - Resolve any conflicting packages

---

## Connection Problems

### IPC Connection Failed

**Symptom:** Self-check shows "IPC Connection: NOT CONNECTED"

**Solutions:**

1. **Port Already in Use**
   - Check if port 5555 is in use:
     ```bash
     # Windows
     netstat -ano | findstr :5555
     
     # Mac/Linux
     lsof -i :5555
     ```
   - If in use, close the application or change plugin port

2. **Firewall Blocking**
   - Allow local connections on port 5555
   - Windows: Windows Defender Firewall > Allow an app
   - Mac: System Preferences > Security & Privacy > Firewall

3. **Network Configuration**
   - Ensure localhost (127.0.0.1) is accessible
   - Try pinging: `ping 127.0.0.1`

### Backend Responds But Slow

**Symptom:** Queries take a long time or timeout

**Solutions:**

1. **Check API Quotas**
   - You may have hit rate limits
   - Check your API provider dashboard

2. **Network Latency**
   - API calls require internet access
   - Test network speed

3. **Large Context**
   - If ingested many documents, queries may be slower
   - Consider using smaller document sets for testing

---

## Self-Check Failures

### Running Self-Check

1. Open Adastrea Director panel (Window > Developer Tools > Adastrea Director)
2. Go to "Tests" tab
3. Click "🔍 Self-Check" button
4. Review results

### Common Check Failures

#### [1/8] Runtime Module: NOT LOADED
- Plugin not properly installed
- Rebuild plugin from source
- Check for build errors in Output Log

#### [2/8] Settings Configuration: INVALID
- API key not configured
- Go to Settings and configure API key

#### [3/8] Python Bridge: NOT INITIALIZED
- Python not found or not installed
- Install Python 3.8+

#### [4/8] Python Process: NOT RUNNING
- Backend script failed to start
- Check Python dependencies
- Test backend manually

#### [5/8] IPC Connection: NOT CONNECTED
- Port 5555 blocked or in use
- Check firewall settings
- Verify backend is running

#### [6/8] Backend Health: NOT RESPONDING
- Backend crashed or hung
- Restart Unreal Engine
- Check Python logs

#### [7/8] API Key Validation: INVALID
- Incorrect API key
- API key revoked or expired
- Network/firewall blocking API access

#### [8/8] Query Processing: FAILED
- RAG system not initialized
- Check document ingestion
- Verify ChromaDB is accessible

---

## Advanced Diagnostics

### Enable Verbose Logging

**Note:** Verbose logging is safe - API keys are configured via .env file and never transmitted through the plugin, so they will not appear in logs.

1. Open `DefaultEngine.ini` in your project
2. Add:
   ```ini
   [Core.Log]
   LogAdastreaDirector=Verbose
   LogAdastreaDirectorEditor=Verbose
   ```
3. Restart Unreal Engine
4. Check Output Log for detailed messages

### Check Configuration File

Plugin settings are stored in: `<ProjectDir>/Saved/AdastreaDirector/config.ini`

Example valid configuration:
```ini
# Note: API keys are NOT stored in config.ini - they're in .env file
LLMProvider=gemini
EmbeddingProvider=huggingface
DefaultFontSize=10
AutoSaveSettings=true
ShowTimestamps=true
```

API keys are stored in `.env` file in project root:
```
GEMINI_KEY=AIzaSyXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
# or
GOOGLE_API_KEY=AIzaSyXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
OPENAI_API_KEY=sk-XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
```

### Manual Backend Test

Test backend communication manually:

1. Start backend:
   ```bash
   cd Plugins/AdastreaDirector/Python
   python ipc_server.py --port 5555
   ```

2. In another terminal, test with netcat:
   ```bash
   echo '{"type":"ping","data":""}' | nc localhost 5555
   ```
   
   Should return:
   ```json
   {"status":"success","message":"pong","timestamp":1234567890.0}
   ```

---

## Getting Help

If you're still experiencing issues:

1. **Check Documentation**
   - Read the [README.md](README.md)
   - Review [SETUP_GUIDE.md](SETUP_GUIDE.md)

2. **Review Output Log**
   - Copy relevant error messages
   - Include in your bug report

3. **Run Self-Check**
   - Include self-check results in bug reports
   - Take screenshots of error screens

4. **Report Issues**
   - GitHub Issues: https://github.com/Mittenzx/Adastrea-Director/issues
   - Include:
     - Unreal Engine version
     - Plugin version
     - Operating system
     - Self-check results
     - Relevant log excerpts
     - Steps to reproduce

---

## Quick Reference: Common Fixes

| Problem | Quick Fix |
|---------|-----------|
| Plugin not in menu | Enable in Edit > Plugins, restart UE |
| API key error | Configure in Settings, get key from provider |
| Python not found | Install Python 3.8+, add to PATH |
| Backend won't start | `pip install google-generativeai chromadb langchain` |
| Port 5555 in use | Close other app or change port |
| IPC not connected | Check firewall, allow local connections |
| Slow queries | Check API rate limits, network speed |
| Self-check fails | Run manual tests, check Output Log |

---

**Last Updated:** December 2025  
**Plugin Version:** 1.0.0  
**UE Version:** 5.6+
