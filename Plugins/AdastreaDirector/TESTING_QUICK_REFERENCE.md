# Plugin Testing Quick Reference

**Quick access guide for testing the Adastrea Director plugin**

---

## 👋 First Time Here?

**Welcome!** This is a condensed guide for testing the Adastrea Director plugin.

### What is This Document?

This is the **quick reference** version of our testing documentation. Think of it as a "cheat sheet" that gives you the most important commands and checks without all the detailed explanations.

**Use this if:**
- ✅ You've tested before and need a quick reminder
- ✅ You want to run specific tests quickly
- ✅ You need to find a command fast
- ✅ You're debugging a specific issue

**Use the [full testing checklist](TESTING_CHECKLIST_WEEKS_1_6.md) if:**
- 📚 This is your first time testing the plugin
- 📚 You want detailed explanations of what each test does
- 📚 You need troubleshooting help
- 📚 You're learning about the plugin architecture

### How Testing Works

Testing the plugin involves:
1. **Automated tests** - Scripts you run that check things automatically
2. **Manual tests** - Things you do yourself in Unreal Engine (clicking, typing, observing)
3. **Verification** - Confirming the results match what's expected

Don't worry - we tell you exactly what commands to run and what to look for!

### What You're Testing

The Adastrea Director plugin has several components that need testing:
- **Plugin structure** (Week 1): Files are organized correctly
- **Python connection** (Week 2-3): C++ and Python parts can communicate
- **User interface** (Week 4): UI displays and works properly
- **AI features** (Week 5-6): Document ingestion and question answering work

---

## 📋 Main Testing Document

**[TESTING_CHECKLIST_WEEKS_1_6.md](TESTING_CHECKLIST_WEEKS_1_6.md)** - Comprehensive testing checklist (1594 lines)
- ⭐ **Start here if you're new to testing this plugin**
- Contains detailed explanations, step-by-step procedures, and troubleshooting
- Includes beginner-friendly context and terminology

---

## 🚀 Quick Start Testing

**Before you start:** Make sure you've completed the setup in the [full testing checklist](TESTING_CHECKLIST_WEEKS_1_6.md#testing-prerequisites).

### Prerequisites Check

**What this does:** Verifies you have the required software installed.

```bash
# Check Python version (should show 3.9 or higher)
python --version

# Check where Python is installed (optional, for troubleshooting)
which python             # Mac/Linux
where python             # Windows

# Check if ChromaDB is installed (needed for AI features in Week 5+)
pip list | grep chroma   # Mac/Linux
pip list | findstr chroma  # Windows
```

**What the output means:**
- `python --version` shows something like "Python 3.9.7" or higher → ✅ Good
- `python --version` shows "command not found" or lower version → ❌ Need to install Python
- `pip list` shows chromadb → ✅ Ready for Week 5+ tests
- `pip list` shows nothing → ⚠️ Install later before Week 5 tests

### Run All Quick Tests

**What this does:** Runs the main automated test suite to verify the plugin's Python backend works correctly.

**Time needed:** 2-3 minutes

**Skill level:** Basic (just copy and paste)

**When to use:** After setting up the plugin or making changes to verify everything still works.

```bash
# Step 1: Navigate to the Python backend folder
cd Plugins/AdastreaDirector/Python

# Test 1: IPC Server functionality
# What it tests: Communication between C++ and Python works
python test_ipc.py 5555
# Expected: Should see "All IPC tests passed!" at the end

# Test 2: RAG modules structure
# What it tests: AI document processing modules are set up correctly
python test_rag_modules.py
# Expected: Should see checkmarks (✅) for each module check

# Test 3: IPC performance
# What it tests: Communication is fast enough (should be under 50ms)
python test_ipc_performance.py
# Expected: Should show latency times all under 50ms

# Test 4: UI integration (requires server running)
# What it tests: UI can connect to and use the Python backend
python ipc_server.py --port 5555 &  # Start server in background
SERVER_PID=$!                        # Save process ID for cleanup
python test_ui_integration.py        # Run integration tests
kill $SERVER_PID                     # Stop the server when done
# Expected: Should see "All integration tests passed!"
```

**If tests fail:**
1. Check the error message - it usually tells you what's wrong
2. Verify prerequisites are installed (see above)
3. See the [Troubleshooting section](#common-issues--quick-fixes) below
4. Check the [full testing checklist](TESTING_CHECKLIST_WEEKS_1_6.md#troubleshooting-guide) for detailed help

**Windows users note:**
The last test (Test 4) uses Unix commands. On Windows, use this instead:
```cmd
:: Start server
start /B python ipc_server.py --port 5555
:: Run tests (wait a moment for server to start)
timeout /t 2
python test_ui_integration.py
:: Stop server (manually close the window or use Task Manager)
```

---

## 📊 Testing by Week

**How the weeks are organized:**

The plugin was built over 6 weeks, with each week adding new features. Testing follows the same structure:
- **Weeks 1-2**: Foundation (plugin setup and Python connection)
- **Weeks 3-4**: Communication and UI (data flow and user interface)
- **Weeks 5-6**: AI Features (document processing and question answering)

**Test in order if:**
- This is your first time testing
- You want to understand how everything builds up
- Something isn't working and you need to isolate the problem

**Jump to specific weeks if:**
- You've tested before and need to verify specific features
- You only changed code for a specific week's features
- You're debugging a known issue

| Week | Focus | What Gets Tested | Quick Test Command | Full Docs |
|------|-------|------------------|-------------------|-----------|
| **Week 1** | Project Setup | Files exist, plugin loads | Manual: Load plugin in UE | [Week 1 Tests](#week-1-quick-tests) |
| **Week 2** | Python Bridge | C++ ↔ Python connection | `python test_ipc.py 5555` | [Week 2 Tests](#week-2-quick-tests) |
| **Week 3** | Backend IPC | Communication speed/reliability | `python test_ipc_performance.py` | [Week 3 Tests](#week-3-quick-tests) |
| **Week 4** | Basic UI | Interface displays correctly | Manual: Open panel in UE | [Week 4 Tests](#week-4-quick-tests) |
| **Week 5** | Ingestion | Document processing | `python test_rag_modules.py` | [Week 5 Tests](#week-5-quick-tests) |
| **Week 6** | Query System | Question answering | `python test_ui_integration.py` | [Week 6 Tests](#week-6-quick-tests) |

---

## Week 1 Quick Tests

**Goal:** Verify plugin structure and loading

**What you're checking:** The plugin files are organized correctly and Unreal Engine can recognize and load the plugin.

**Why this matters:** If the basic structure isn't right, nothing else will work. Think of it like checking the foundation before building a house.

**Skill level:** Basic - checking files and opening Unreal Engine

**Time needed:** 5-10 minutes

### Essential Checks
- [ ] Plugin folder structure complete (all expected files and folders exist)
- [ ] `.uplugin` file is valid JSON (configuration file is properly formatted)
- [ ] Plugin appears in UE plugin list (Unreal Engine can see it)
- [ ] Plugin loads without errors (starts up successfully)

### Quick Verification

**Check 1: Verify Folder Structure**

```bash
# Navigate to plugin directory
cd Plugins/AdastreaDirector

# Check source code folders exist
# The '-la' flags mean "list all files (including hidden ones) with detailed information"
ls -la Source/
# Expected: Should see AdastreaDirector/ and AdastreaDirectorEditor/ folders

# Check resources folder exists
ls -la Resources/
# Expected: Should see Icon files

# Windows equivalent:
dir Source\
dir Resources\
```

**Check 2: Validate Configuration File**

```bash
# Check that the .uplugin file is valid JSON format
python -m json.tool AdastreaDirector.uplugin
# Expected: Should display formatted JSON without errors
# If you see an error, the file format is invalid

# What this file does: Tells Unreal Engine about the plugin 
# (name, version, what modules it has, etc.)
```

**Check 3: Test in Unreal Engine (Manual)**

1. Open Unreal Engine Editor
2. Open your test project
3. Go to **Edit → Plugins**
4. Search for "Adastrea Director"
5. ✅ Plugin should appear in the list under "Developer Tools"
6. ✅ Status should show as "Enabled"
7. Check **Window → Developer Tools → Output Log**
8. ✅ Should see: "AdastreaDirector Module: StartupModule"

**If plugin doesn't appear:**
- Check that plugin is copied to YourProject/Plugins/
- Right-click .uproject → Generate Visual Studio Project Files
- Rebuild the project

**Full Details:** [TESTING_CHECKLIST_WEEKS_1_6.md#week-1-project-setup-testing](TESTING_CHECKLIST_WEEKS_1_6.md#week-1-project-setup-testing)

---

## Week 2 Quick Tests

**Goal:** Verify Python Bridge and IPC work

**What you're checking:** The C++ plugin can successfully start and communicate with the Python backend. This is the connection that makes the AI features possible.

**Why this matters:** The plugin's AI capabilities run in Python, so the C++ and Python parts must be able to talk to each other reliably.

**Key concept - IPC (Inter-Process Communication):** 
Think of it like a phone call between two programs. The C++ plugin (in Unreal Engine) needs to "call" the Python backend to request AI processing, and Python needs to "answer" with results.

**Skill level:** Basic - running test scripts

**Time needed:** 2-3 minutes

### Essential Checks
- [ ] Python server starts (the Python backend launches successfully)
- [ ] IPC connection succeeds (C++ can connect to Python)
- [ ] Request/response works (can send data and get responses)
- [ ] Subprocess managed correctly (Python process starts/stops cleanly)

### Quick Test

```bash
# Navigate to Python backend folder
cd Plugins/AdastreaDirector/Python

# Run the IPC test script
python test_ipc.py 5555
# This script:
# 1. Starts a test Python server on port 5555
# 2. Sends various test messages
# 3. Verifies responses are correct
# 4. Cleans up when done
```

**Expected Output:**
```
✅ Server connection successful
✅ Ping test passed
✅ Query test passed
✅ Plan test passed
✅ Analyze test passed
✅ Invalid request handled correctly
✅ Multi-threaded test passed

All IPC tests passed!
```

**What each test means:**
- **Server connection**: Python backend is reachable
- **Ping test**: Basic communication works (like "hello, are you there?")
- **Query/Plan/Analyze**: Different types of AI requests work
- **Invalid request**: Error handling works properly
- **Multi-threaded**: Multiple requests at once work (important for performance)

**If tests fail:**
- Check Python is installed: `python --version`
- Check port 5555 isn't already in use:
  - **Mac/Linux:**
    ```bash
    netstat -an | grep 5555
    ```
  - **Windows:**
    ```cmd
    netstat -an | findstr 5555
    ```
- Make sure you're in the correct directory
- Check firewall isn't blocking localhost connections

**Full Details:** [TESTING_CHECKLIST_WEEKS_1_6.md#week-2-python-bridge-testing](TESTING_CHECKLIST_WEEKS_1_6.md#week-2-python-bridge-testing)

---

## Week 3 Quick Tests

**Goal:** Verify backend performance and reliability

**What you're checking:** The communication between C++ and Python is fast and reliable, even under heavy use. This ensures the plugin will feel responsive and won't slow down Unreal Engine.

**Why this matters:** If communication is slow (like a laggy video call), the plugin will feel sluggish and frustrate users. We need it to be nearly instant.

**Key concept - Performance metrics:**
We measure how long operations take (latency) and how much data can be processed (throughput). Good performance means users don't notice any delay.

**Skill level:** Basic - running test scripts

**Time needed:** 1-2 minutes

### Essential Checks
- [ ] Performance metrics working (the system can track its own speed)
- [ ] All handlers implemented (all types of requests are supported)
- [ ] Latency < 1ms for ping (communication is nearly instant)
- [ ] Concurrent requests handled (multiple requests at once work)

### Quick Test

```bash
# Navigate to Python backend folder
cd Plugins/AdastreaDirector/Python

# Run the performance test script
python test_ipc_performance.py
# This script:
# 1. Sends 100 test requests
# 2. Measures how long each takes
# 3. Calculates average, min, and max times
# 4. Reports if performance meets targets
```

**Expected Output:**
```
Testing IPC Performance...
Sending 100 requests...
Average latency: 0.42ms
Min: 0.15ms, Max: 2.1ms
✅ Performance target met! (target: < 50ms average)
```

**What the numbers mean:**
- **Average latency**: Typical time for one request (should be well under 50ms)
- **Min**: Fastest request (shows best-case performance)
- **Max**: Slowest request (shows worst-case performance)
- Our target is < 50ms average, but we typically achieve < 1ms (50x better!)

**If performance is slow:**
- Close other programs that might be using resources
- Check if antivirus is scanning files
- Ensure computer isn't in power-saving mode
- On Windows, check Windows Defender isn't blocking localhost

**Full Details:** [TESTING_CHECKLIST_WEEKS_1_6.md#week-3-python-backend-ipc-testing](TESTING_CHECKLIST_WEEKS_1_6.md#week-3-python-backend-ipc-testing)

---

## Week 4 Quick Tests

**Goal:** Verify UI loads and communicates

**What you're checking:** The user interface appears in Unreal Engine and you can interact with it to send queries and see results. This is what users will actually use to access the AI features.

**Why this matters:** All the backend functionality is useless if users can't access it! The UI is the "front door" to the plugin's AI capabilities.

**Key concept - Slate UI:**
Slate is Unreal Engine's framework for building editor interfaces. It handles windows, buttons, text boxes, and all the visual elements you interact with.

**Skill level:** Very easy - just clicking and typing in Unreal Engine

**Time needed:** 3-5 minutes

### Essential Checks
- [ ] Panel opens in UE (the Adastrea Director window appears)
- [ ] Query input works (you can type in the text box)
- [ ] Send button functional (clicking the button does something)
- [ ] Results display updates (responses show up in the results area)

### Quick Test - Manual in Unreal Editor

**Step-by-step instructions:**

1. **Open Unreal Engine Editor**
   - Launch Unreal Engine
   - Open your test project

2. **Open the Adastrea Director Panel**
   - Go to the menu bar at the top
   - Click **Window** → **Developer Tools** → **Adastrea Director**
   - ✅ A new dockable panel should appear

3. **Check the Interface**
   - ✅ Should see a text input box at the top
   - ✅ Should see a "Send Query" button
   - ✅ Should see a results display area below
   - ✅ Panel should be dockable (can drag it around)

4. **Test Query Functionality**
   - Click in the text input box
   - Type: "test query"
   - Click the "Send Query" button (or press Enter)
   - ✅ Should see a response appear in the results area
   - ✅ Response should appear within 1-2 seconds

5. **Test Multiple Queries**
   - Send another query
   - ✅ New response should appear below the first one
   - ✅ Should be able to scroll if responses get long

**What a successful test looks like:**
- Panel opens without errors
- Can type in the input box
- Button responds when clicked
- Results show up in the display area
- No freezing or crashes

**If the panel doesn't open:**
- Check Edit → Plugins → search "Adastrea Director" → should be Enabled
- Check Output Log (Window → Output Log) for error messages
- Try restarting Unreal Engine
- Verify Week 1 tests passed (plugin must load correctly first)

**If queries don't work:**
- Check Python backend is running (see Week 2 tests)
- Look for error messages in the Output Log
- Verify Week 2 tests passed (Python connection must work)

**Full Details:** [TESTING_CHECKLIST_WEEKS_1_6.md#week-4-basic-ui-testing](TESTING_CHECKLIST_WEEKS_1_6.md#week-4-basic-ui-testing)

---

## Week 5 Quick Tests

**Goal:** Verify document ingestion works

**What you're checking:** The plugin can read documentation files, process them, and store them in a database so the AI can search through them to answer questions.

**Why this matters:** The AI needs to learn about your project from documentation. Ingestion is like teaching the AI by having it read all your docs.

**Key concepts:**
- **Document ingestion**: Reading files and preparing them for AI processing
- **ChromaDB**: A database that stores documents in a way the AI can search efficiently
- **RAG (Retrieval-Augmented Generation)**: AI technique that searches documents to find relevant information before answering questions

**Skill level:** Basic for automated test; Easy for manual test

**Time needed:** 2-3 minutes for test; 5-30 minutes for real ingestion (depends on document count)

### Essential Checks
- [ ] Ingestion UI functional (the ingestion tab works)
- [ ] Progress bar updates (shows real-time progress)
- [ ] Documents ingested (files are processed successfully)
- [ ] ChromaDB created (database folder is created)

### Quick Automated Test

```bash
# Navigate to Python backend folder
cd Plugins/AdastreaDirector/Python

# Run the RAG modules test
python test_rag_modules.py
# This script:
# 1. Checks that RAG ingestion modules exist
# 2. Verifies they can be imported
# 3. Tests basic functionality
# 4. Doesn't actually ingest documents (that's the manual test)
```

**Expected Output:**
```
Testing RAG Modules...
✅ rag_ingestion module found
✅ rag_query module found
✅ Required classes exist
✅ Basic functionality works
All RAG module tests passed!
```

### Manual Test in Unreal Engine

**This tests the actual document ingestion process:**

1. **Prepare Test Documents** (first time only)
   ```bash
   # Create a test docs folder with some sample files
   mkdir test_docs
   echo "# Test Document 1" > test_docs/doc1.md
   echo "This is a test document for ingestion." >> test_docs/doc1.md
   echo "# Test Document 2" > test_docs/doc2.md
   echo "Another test document with different content." >> test_docs/doc2.md
   ```

2. **Open Adastrea Director in UE**
   - Window → Developer Tools → Adastrea Director

3. **Switch to Ingestion Tab**
   - ✅ Should see "Ingestion" tab at the top of the panel
   - Click it

4. **Select Documents Folder**
   - Click "Browse" button next to "Docs Path"
   - Navigate to your test_docs folder
   - Select it
   - ✅ Path should appear in the text box

5. **Select Database Location**
   - Click "Browse" button next to "DB Path"
   - Choose where to save the database (e.g., create a "test_db" folder)
   - ✅ Path should appear in the text box

6. **Start Ingestion**
   - Click "Start Ingestion" button
   - ✅ Progress bar should start moving
   - ✅ Status label should show "Processing file X of Y"
   - ✅ Progress bar should reach 100%
   - ✅ Status should show "Ingestion complete!"

7. **Verify Results**
   - Check that the database folder was created
   - Check Output Log for any errors
   - Documents are now ready for querying (Week 6)

**What you should see:**
- Progress bar smoothly goes from 0% to 100%
- Status updates with current file being processed
- No error messages
- Completion message when done

**If ingestion fails:**
- Check folder paths are correct and exist
- Verify you have read/write permissions
- Check Output Log for specific error messages
- Make sure ChromaDB is installed: `pip install chromadb`

**Full Details:** [TESTING_CHECKLIST_WEEKS_1_6.md#week-5-document-ingestion-testing](TESTING_CHECKLIST_WEEKS_1_6.md#week-5-document-ingestion-testing)

---

## Week 6 Quick Tests

**Goal:** Verify query system works end-to-end

**What you're checking:** The AI can answer questions based on the documents you ingested, maintain conversation context, and provide relevant, accurate responses.

**Why this matters:** This is the core AI feature! Everything else was building up to this - users asking questions and getting intelligent answers based on their project documentation.

**Key concepts:**
- **Query**: A question you ask the AI
- **Context-aware**: The AI remembers previous questions and provides relevant follow-up answers
- **Conversation history**: Like a chat - each question and answer builds on the previous ones
- **Caching**: Storing answers to common questions so responses are faster next time

**Skill level:** Easy for manual test; Basic for automated test

**Time needed:** 5-10 minutes

### Essential Checks
- [ ] Queries return responses (AI answers your questions)
- [ ] Responses are contextual (answers are relevant to your docs)
- [ ] Conversation history works (AI remembers previous questions)
- [ ] Cache improves performance (repeat questions are instant)

### Quick Automated Test

```bash
# Navigate to Python backend folder
cd Plugins/AdastreaDirector/Python

# Start the IPC server in the background
python ipc_server.py --port 5555 &
# Wait for server to start
sleep 2

# Run UI integration tests
python test_ui_integration.py
# This script:
# 1. Sends test queries to the server
# 2. Verifies responses are received
# 3. Checks conversation history
# 4. Tests cache performance

# Cleanup: Stop the server
pkill -f ipc_server.py  # Mac/Linux
# On Windows, close the command prompt or use Task Manager
```

**Expected Output:**
```
Testing UI Integration...
✅ Server connection successful
✅ Basic query works
✅ Context-aware query works
✅ Conversation history maintained
✅ Cache improves performance
All integration tests passed!
```

### Manual Test in Unreal Engine

**This tests the actual user experience of asking questions:**

**Prerequisites:** You must have completed Week 5 (ingested documents) first!

1. **Open Adastrea Director**
   - Window → Developer Tools → Adastrea Director

2. **Switch to Query Tab**
   - Click the "Query" tab
   - ✅ Should see the query interface

3. **Ask a Simple Question**
   - Type: "What is Unreal Engine?"
   - Click "Send Query" or press Enter
   - ✅ Should receive a response within 1-3 seconds
   - ✅ Response should be coherent and relevant
   - Note: If you haven't ingested UE docs, you might get a generic response

4. **Test Context Awareness**
   - Ask a follow-up: "What are its main features?"
   - ✅ AI should understand "its" refers to Unreal Engine from previous question
   - ✅ Response should be contextually appropriate

5. **Test Conversation History**
   - Ask another related question: "How do I use it?"
   - ✅ AI should still remember the conversation context
   - ✅ Answers should build on previous questions

6. **Test Cache Performance**
   - Repeat the first question: "What is Unreal Engine?"
   - ✅ Response should come back much faster (< 100ms)
   - ✅ Answer should be identical to the first time

7. **Test Clear History**
   - Click "Clear History" button (if available)
   - Ask: "What is it?" (no context)
   - ✅ AI shouldn't know what "it" refers to anymore
   - ✅ Might ask for clarification or give generic response

**What good responses look like:**
- Answer the question directly
- Reference information from your ingested documents
- Are coherent and well-structured
- Take 1-3 seconds for first query, < 100ms for cached queries
- Follow-up questions understand the conversation context

**If queries don't work:**
- Verify documents were ingested (Week 5)
- Check Python backend is running
- Look for errors in Output Log
- Ensure you have an LLM API key configured (check main repo docs)

**If responses are irrelevant:**
- Check that relevant documents were ingested
- Try more specific questions
- Verify the ingested documents actually contain the information

**If responses are slow:**
- First query is always slower (1-3 sec) - this is normal
- Repeat queries should be cached and fast
- Check internet connection (if using cloud LLM)
- Check computer performance (low memory can slow things down)

**Full Details:** [TESTING_CHECKLIST_WEEKS_1_6.md#week-6-query-system-testing](TESTING_CHECKLIST_WEEKS_1_6.md#week-6-query-system-testing)

---

## 🔧 Common Issues & Quick Fixes

**Stuck? Start here!** These are the most common problems and their solutions.

### Issue 1: Plugin Won't Load in Unreal Engine

**Symptoms:**
- Plugin doesn't appear in Edit → Plugins
- Unreal Engine shows compile errors
- Plugin list shows "Failed to load"

**Quick Fixes:**

**Fix A: Regenerate Project Files**
```bash
# Navigate to your project folder
cd /path/to/YourProject

# Windows: Right-click YourProject.uproject → "Generate Visual Studio project files"
# Mac: Either right-click YourProject.uproject → "Generate Xcode project files" (newer UE versions)
#      or run ./GenerateProjectFiles.sh (works on all versions)
# Linux: Run the script
./GenerateProjectFiles.sh
```
**What this does:** Updates project configuration files so Unreal Engine knows about the plugin.

**Fix B: Rebuild the Project**
- Open project in Visual Studio/Xcode
- Build → Clean Solution
- Build → Rebuild Solution
- **What this does:** Recompiles all code from scratch, fixing most build issues.

**Fix C: Check Plugin Location**
```bash
# Plugin must be in the right place:
YourProject/Plugins/AdastreaDirector/AdastreaDirector.uplugin
# Not in a subfolder, not in the wrong project!
```

---

### Issue 2: Python Backend Won't Connect

**Symptoms:**
- UI tests fail with "connection refused"
- Output Log shows "Failed to connect to Python backend"
- Queries don't work

**Quick Fixes:**

**Fix A: Verify Python Installation**
```bash
# Check Python is installed and version is 3.9+
python --version
# Expected: Python 3.9.x or higher

# If "command not found":
# - Install Python from python.org
# - Restart terminal after installing
```

**Fix B: Install Python Dependencies**
```bash
# Navigate to Python backend
cd Plugins/AdastreaDirector/Python

# Install all required packages
pip install -r requirements.txt
# This might take a few minutes

# If you see "pip: command not found":
# Try: pip3 instead of pip
# Or: python -m pip install -r requirements.txt
```

**Fix C: Test Server Manually**
```bash
# Try starting the server yourself
cd Plugins/AdastreaDirector/Python
python ipc_server.py --port 5555

# Should see: "Server listening on localhost:5555"
# If you see an error, read the message - it usually tells you what's wrong

# Common errors:
# "Address already in use" → Port 5555 is busy, try port 5556 instead
#   ⚠️ If you change the port here, you may also need to update the port number 
#   in other places (e.g., plugin config, test scripts, client code) where 5555 
#   is hardcoded, or connections may fail.
# "Module not found" → Install dependencies (Fix B above)
```

**Fix D: Check Firewall**
- Windows: Check Windows Defender isn't blocking Python
- Mac: System Preferences → Security & Privacy → Firewall → Allow Python
- The server only uses localhost (127.0.0.1), no external network access needed

---

### Issue 3: Document Ingestion Fails

**Symptoms:**
- Progress bar doesn't move
- Error message during ingestion
- Database folder not created

**Quick Fixes:**

**Fix A: Install AI Libraries**
```bash
# ChromaDB and LangChain are required for ingestion
pip install chromadb langchain

# If that doesn't work, try:
pip install --upgrade chromadb langchain

# Verify installation:
python -c "import chromadb; print('ChromaDB OK')"
python -c "import langchain; print('LangChain OK')"
```

**Fix B: Check Folder Permissions**
```bash
# Make sure you can read the docs folder
ls -la /path/to/docs/folder
# Should see files listed

# Make sure you can write to DB folder  
# Try creating a test file
touch /path/to/db/folder/test.txt
# If error, you don't have write permission - choose different folder

# Windows equivalent:
dir C:\path\to\docs\folder
echo test > C:\path\to\db\folder\test.txt
```

**Fix C: Test RAG Modules**
```bash
cd Plugins/AdastreaDirector/Python
python test_rag_modules.py
# This verifies ingestion code is working

# If test fails, check error message for specific problem
```

**Fix D: Try Smaller Test**
- Instead of ingesting hundreds of files, try 2-3 files first
- Put test files in a simple folder (no spaces in path)
- Use simple .txt or .md files
- If that works, the issue is with specific files or large volumes

---

### Issue 4: Queries Return Nothing or Errors

**Symptoms:**
- Send query but get no response
- Response says "no documents found"
- Error messages about API keys

**Quick Fixes:**

**Fix A: Verify Database Exists**
```bash
# Check that ingestion created the database
ls -la /path/to/chroma_db/
# Should see database files

# Windows:
dir C:\path\to\chroma_db\

# If folder is empty or doesn't exist:
# You need to complete Week 5 (document ingestion) first!
```

**Fix B: Test RAG System**
```bash
cd Plugins/AdastreaDirector/Python
python test_rag_modules.py
# Verifies RAG components are working
```

**Fix C: Check API Key (if using cloud LLM)**

> ⚠️ **Security Warning:** The commands below will display your API key in plain text. Avoid running these in shared environments or where your terminal output may be logged.

**Recommended: Check if API key is set (without displaying its value)**
```bash
# Linux/Mac (bash/zsh):
[ -z "$GEMINI_KEY" ] && echo "GEMINI_KEY: Not set" || echo "GEMINI_KEY: Set"
[ -z "$OPENAI_API_KEY" ] && echo "OPENAI_API_KEY: Not set" || echo "OPENAI_API_KEY: Set"

# Windows (cmd):
if "%GEMINI_KEY%"=="" (echo GEMINI_KEY: Not set) else (echo GEMINI_KEY: Set)
if "%OPENAI_API_KEY%"=="" (echo OPENAI_API_KEY: Not set) else (echo OPENAI_API_KEY: Set)

# If "Not set", you need to set the API key
# See main repository docs for API key setup
```

**Fix D: Try Basic Query First**
- Instead of complex questions, try: "Hello"
- If that works, the system is functional
- If not, check Output Log for specific errors

---

### Still Stuck?

1. **Check the Output Log** (Window → Developer Tools → Output Log)
   - Error messages usually explain what's wrong
   - Filter for "AdastreaDirector" to see plugin-specific messages

2. **Read the Full Troubleshooting Guide**
   - [Comprehensive troubleshooting](TESTING_CHECKLIST_WEEKS_1_6.md#troubleshooting-guide)
   - Covers advanced issues and platform-specific problems

3. **Ask for Help**
   - Create an issue on [GitHub](https://github.com/Mittenzx/Adastrea-Director/issues)
   - Include: What you were doing, error messages, your OS/UE version
   - Be specific - "doesn't work" doesn't help; error messages do!

4. **Start Over from Week 1**
   - Sometimes it helps to verify each week passes before moving forward
   - Ensures foundation is solid before testing advanced features

---

## 📈 Performance Targets

| Metric | Target | How to Test |
|--------|--------|-------------|
| IPC Latency | < 50ms | `python test_ipc_performance.py` |
| Query Response | < 3s | Send query in UI |
| Cached Query | < 100ms | Repeat same query |
| Ingestion Rate | 1-2 files/s | Ingest 10 files, time it |
| Memory Usage | < 1GB total (peak), avg 500MB | Monitor during ingestion |

---

## 🧪 Test Scripts Reference

Located in: `Plugins/AdastreaDirector/Python/`

| Script | Purpose | Usage |
|--------|---------|-------|
| `test_ipc.py` | IPC functionality | `python test_ipc.py 5555` |
| `test_rag_modules.py` | RAG module structure | `python test_rag_modules.py` |
| `test_ipc_performance.py` | Performance benchmarks | `python test_ipc_performance.py` |
| `test_ui_integration.py` | UI integration | `python test_ui_integration.py` |

---

## 📚 Related Documentation

- **[TESTING_CHECKLIST_WEEKS_1_6.md](TESTING_CHECKLIST_WEEKS_1_6.md)** - Complete testing guide
- **[README.md](README.md)** - Plugin overview
- **[INSTALLATION.md](INSTALLATION.md)** - Installation guide
- **[RAG_INTEGRATION.md](RAG_INTEGRATION.md)** - RAG system details
- **[WEEK1_COMPLETION.md](WEEK1_COMPLETION.md)** - Week 1 report
- **[WEEK2_COMPLETION.md](WEEK2_COMPLETION.md)** - Week 2 report
- **[WEEK3_COMPLETION.md](WEEK3_COMPLETION.md)** - Week 3 report
- **[WEEK4_COMPLETION.md](WEEK4_COMPLETION.md)** - Week 4 report
- **[WEEK5_6_COMPLETION.md](WEEK5_6_COMPLETION.md)** - Weeks 5-6 report

---

## ✅ Full Test Checklist Summary

### Week 1: Project Setup ✅
- [x] Plugin structure valid
- [x] Plugin loads in UE
- [x] Modules initialize
- [x] Cross-platform verified

### Week 2: Python Bridge ✅
- [x] Python subprocess launches
- [x] IPC connection works
- [x] Requests/responses correct
- [x] Error recovery functional

### Week 3: Backend IPC ✅
- [x] Performance metrics working
- [x] All handlers implemented
- [x] Concurrent requests handled
- [x] Optimization targets met

### Week 4: Basic UI ✅
- [x] Panel loads and displays
- [x] Query input functional
- [x] Results display works
- [x] End-to-end communication verified

### Week 5: Document Ingestion ✅
- [x] Ingestion UI functional
- [x] Progress tracking works
- [x] Documents ingested correctly
- [x] Incremental updates work

### Week 6: Query System ✅
- [x] Basic queries work
- [x] Context-aware responses
- [x] Conversation history maintained
- [x] Performance acceptable

### Integration ✅
- [x] End-to-end workflow smooth
- [x] Cross-module integration verified
- [x] Platform compatibility confirmed
- [x] Performance benchmarks met

---

## 🎯 Next Steps

After completing testing for weeks 1-6:

1. **Week 7-8:** Polish & Testing
   - Settings dialog
   - Error handling improvements
   - Additional UI polish
   - Documentation refinement

2. **Week 9-12:** Planning Features (Phase 3)
   - Goal analysis integration
   - Task decomposition
   - Code generation

3. **Week 13-16:** Polish & Release (Phase 4)
   - Fab marketplace preparation
   - Final testing
   - Documentation complete
   - Launch!

---

**Document Version:** 1.0  
**Last Updated:** November 14, 2025  
**Status:** Ready for Use

*For detailed testing procedures, see [TESTING_CHECKLIST_WEEKS_1_6.md](TESTING_CHECKLIST_WEEKS_1_6.md)*
