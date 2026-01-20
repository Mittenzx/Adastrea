# Week 4 Verification Guide

This document provides step-by-step instructions for verifying the Week 4 Basic UI implementation in Unreal Engine.

---

## Prerequisites

Before testing, ensure you have:

- [ ] Unreal Engine 5.0 or later installed
- [ ] Python 3.8 or later installed
- [ ] Plugin built and loaded in Unreal Engine
- [ ] No build errors or warnings

---

## Part 1: Python Backend Verification

### Step 1: Test IPC Server Standalone

```bash
# Terminal 1: Start the IPC server
cd Plugins/AdastreaDirector/Python
python3 ipc_server.py

# Expected output:
# INFO - Registered handler for 'ping'
# INFO - Registered handler for 'metrics'
# INFO - Registered handler for 'query'
# INFO - Registered handler for 'plan'
# INFO - Registered handler for 'analyze'
# INFO - IPC Server started on 127.0.0.1:5555
# INFO - Waiting for connections from Unreal Engine plugin...
```

**Verification:** ✓ Server starts without errors

### Step 2: Test UI Integration Simulation

```bash
# Terminal 2: Run the integration test
python3 test_ui_integration.py

# Expected output:
# ✓ Connected successfully
# ✓ Query 'What is Unreal Engine?': SUCCESS
# ✓ Response format: VALID
# ✓ Multiple queries: SUCCESS
# ✓ Performance: EXCELLENT (< 50ms requirement)
```

**Verification:** ✓ All tests pass

### Step 3: Test Individual Components

```bash
# Run the unit tests
python3 test_ipc.py

# Expected output:
# All test methods pass
# No errors or exceptions
```

**Verification:** ✓ All unit tests pass

---

## Part 2: Unreal Engine Plugin Verification

### Step 1: Plugin Loading

1. Launch Unreal Engine Editor
2. Open or create a project
3. Check the Output Log for Adastrea Director messages

**Expected Log Entries:**
```
LogAdastreaDirector: AdastreaDirector Runtime Module: StartupModule
LogAdastreaDirector: Python Bridge created
LogAdastreaDirector: Initializing Python Bridge with:
LogAdastreaDirector:   Python: python
LogAdastreaDirector:   Script: [Path]/AdastreaDirector/Python/ipc_server.py
LogAdastreaDirector:   Port: 5555
LogAdastreaDirector: Python Bridge initialized successfully
LogAdastreaDirectorEditor: AdastreaDirector Editor Module: StartupModule
LogAdastreaDirectorEditor: Registered Adastrea Director tab spawner
```

**Verification Checklist:**
- [ ] No error messages in Output Log
- [ ] Python process starts (check Task Manager/Activity Monitor)
- [ ] Python backend listens on port 5555
- [ ] Both runtime and editor modules load successfully

**Status:** ✓ Pass / ✗ Fail

---

### Step 2: Menu Integration

1. In the Unreal Editor, click on the **Window** menu
2. Navigate to **Developer Tools** submenu
3. Look for **Adastrea Director** menu item

**Expected Appearance:**
```
Window
  └── Developer Tools
      ├── Output Log
      ├── Message Log
      ├── ...
      └── Adastrea Director  ← Should be here
```

**Verification Checklist:**
- [ ] "Adastrea Director" appears in menu
- [ ] Menu item has proper icon
- [ ] Tooltip shows on hover
- [ ] Menu item is clickable

**Status:** ✓ Pass / ✗ Fail

---

### Step 3: Panel Opening

1. Click **Window > Developer Tools > Adastrea Director**
2. Panel should open as a dockable tab

**Expected Behavior:**
- Panel opens immediately (no delay)
- Panel appears as a tab (can be docked)
- Panel shows the UI layout correctly
- No crash or error

**Verification Checklist:**
- [ ] Panel opens successfully
- [ ] No errors in Output Log
- [ ] UI elements are visible
- [ ] Panel can be moved and docked
- [ ] Panel can be closed and reopened

**Status:** ✓ Pass / ✗ Fail

---

### Step 4: UI Layout Verification

Once the panel is open, verify all UI elements are present:

**Expected Layout:**

```
┌─────────────────────────────────────────────────┐
│  Adastrea Director - AI Assistant               │
├─────────────────────────────────────────────────┤
│                                                  │
│  Query:                                          │
│  ┌──────────────────────────────┐  ┌─────────┐ │
│  │ Enter your query here...     │  │  Send   │ │
│  └──────────────────────────────┘  │  Query  │ │
│                                     └─────────┘ │
├─────────────────────────────────────────────────┤
│                                                  │
│  Results:                                        │
│  ┌───────────────────────────────────────────┐ │
│  │ Welcome to Adastrea Director!             │ │
│  │                                           │ │
│  │ Enter a query above and click 'Send      │ │
│  │ Query' or press Enter to get started.    │ │
│  │                                           │ │
│  │ Example: "What is Unreal Engine?"        │ │
│  │                                           │ │
│  └───────────────────────────────────────────┘ │
└─────────────────────────────────────────────────┘
```

**Verification Checklist:**
- [ ] Header text displays correctly
- [ ] Query label is visible
- [ ] Query input field is present and editable
- [ ] Send Query button is visible
- [ ] Results label is visible
- [ ] Results display area is present
- [ ] Welcome message displays
- [ ] All text is readable (not overlapping)
- [ ] Layout is properly aligned

**Status:** ✓ Pass / ✗ Fail

---

### Step 5: Query Input Testing

Test the query input functionality:

**Test 5.1: Text Entry**
1. Click in the query input field
2. Type "Test query"
3. Verify text appears as you type

**Verification:**
- [ ] Input field accepts keyboard input
- [ ] Text displays correctly
- [ ] Cursor is visible
- [ ] Can use backspace to delete text
- [ ] Can select text with mouse

**Status:** ✓ Pass / ✗ Fail

**Test 5.2: Button State**
1. With empty query field, check Send Query button
2. Type some text
3. Check button state again

**Verification:**
- [ ] Button is disabled when query is empty
- [ ] Button becomes enabled when query has text
- [ ] Button provides visual feedback on hover

**Status:** ✓ Pass / ✗ Fail

---

### Step 6: Primary Success Criterion Test

**This is the main test specified in Week 4 requirements.**

**Test Query:** "What is Unreal Engine?"

**Steps:**
1. Clear the query input field
2. Type exactly: `What is Unreal Engine?`
3. Click the "Send Query" button (or press Enter)
4. Wait for response

**Expected Response:**
```
Query: What is Unreal Engine?

Response:
Unreal Engine is a comprehensive suite of real-time 3D creation tools 
developed by Epic Games.

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
```

**Verification Checklist:**
- [ ] Query sends successfully (no error)
- [ ] "Processing query..." message appears briefly
- [ ] Response displays in results area
- [ ] Response is formatted correctly
- [ ] Response is complete (not truncated)
- [ ] Response matches expected content
- [ ] No errors in Output Log
- [ ] Response appears within 1 second

**Status:** ✓ Pass / ✗ Fail

**Critical:** This test MUST pass for Week 4 completion.

---

### Step 7: Additional Query Tests

Test with various queries to ensure robustness:

**Test 7.1: Generic Query**
- Query: "How do I create a Blueprint?"
- Expected: Placeholder response explaining query handler

**Test 7.2: Enter Key**
1. Type a query
2. Press Enter key (instead of clicking button)
3. Verify query sends

**Test 7.3: Empty Query**
1. Clear query field
2. Try to send (button should be disabled)
3. Verify no crash

**Test 7.4: Long Query**
- Query: 300+ character string
- Verify it's handled gracefully

**Test 7.5: Special Characters**
- Query: "What is C++ in Unreal Engine?"
- Verify special characters don't break parsing

**Test 7.6: Rapid Queries**
1. Send 5 queries in quick succession
2. Verify all complete successfully
3. Check for memory leaks or UI issues

**Verification:**
- [ ] All queries return responses
- [ ] No crashes or errors
- [ ] UI remains responsive
- [ ] No memory leaks detected

**Status:** ✓ Pass / ✗ Fail

---

### Step 8: Results Display Testing

Verify the results display area:

**Test 8.1: Scrolling**
1. Send a query that returns a long response
2. Verify scroll bar appears
3. Test scrolling with mouse wheel
4. Test scrolling with scroll bar

**Test 8.2: Text Selection**
1. Try to select text in results area
2. Right-click for context menu
3. Try to copy text (Ctrl+C / Cmd+C)

**Test 8.3: Text Wrapping**
1. Resize panel to different widths
2. Verify text wraps correctly
3. No text is cut off or overlapping

**Verification:**
- [ ] Scrolling works smoothly
- [ ] Text is selectable
- [ ] Copy functionality works
- [ ] Text wraps at word boundaries
- [ ] No horizontal scrolling needed
- [ ] All text is readable

**Status:** ✓ Pass / ✗ Fail

---

### Step 9: Panel Docking Tests

Test docking functionality:

**Test 9.1: Drag and Dock**
1. Drag panel tab to different areas
2. Dock to left, right, top, bottom
3. Dock as a tab with other panels
4. Float as standalone window

**Test 9.2: Persistence**
1. Dock panel to preferred location
2. Close Unreal Editor
3. Restart editor
4. Open panel again
5. Verify it remembers location

**Verification:**
- [ ] Can dock to any edge
- [ ] Can tab with other panels
- [ ] Can float as window
- [ ] Position persists across sessions
- [ ] No crashes during docking
- [ ] Panel remains functional after docking

**Status:** ✓ Pass / ✗ Fail

---

### Step 10: Error Handling Tests

Test error scenarios:

**Test 10.1: Python Backend Offline**
1. Stop the Python backend process
2. Try to send a query
3. Verify appropriate error message

**Expected Error Message:**
```
Error: Python backend is not ready.

Please check that the Python backend is running and connected.
```

**Test 10.2: Backend Restart**
1. Stop Python backend
2. Restart it manually
3. Try sending query again
4. Verify it reconnects and works

**Test 10.3: Invalid Response**
(This tests error handling in the plugin)
1. Modify Python server to return invalid JSON
2. Send query
3. Verify error is caught and displayed

**Verification:**
- [ ] Clear error messages displayed
- [ ] No crashes on backend failure
- [ ] Can recover after backend restart
- [ ] Invalid responses handled gracefully
- [ ] Errors logged to Output Log

**Status:** ✓ Pass / ✗ Fail

---

### Step 11: Performance Testing

Verify performance requirements:

**Test 11.1: Response Time**
1. Send multiple queries
2. Measure time from click to response
3. Record times

**Expected Performance:**
- Query processing: < 1ms (Python side)
- Round-trip time: < 50ms (total)
- UI remains responsive throughout

**Test 11.2: UI Responsiveness**
1. Send a query
2. Try to interact with UI during processing
3. Verify UI doesn't freeze
4. Check FPS in editor viewport

**Verification:**
- [ ] Queries complete in < 50ms
- [ ] UI doesn't freeze during queries
- [ ] No frame drops in editor
- [ ] No noticeable lag
- [ ] CPU usage is reasonable

**Status:** ✓ Pass / ✗ Fail

---

### Step 12: Memory and Resource Tests

Check for resource issues:

**Test 12.1: Memory Leaks**
1. Send 100+ queries
2. Monitor memory usage
3. Check for continuous increase

**Test 12.2: Connection Management**
1. Open and close panel 10+ times
2. Verify connections are cleaned up
3. Check Python process doesn't accumulate connections

**Test 12.3: Long Session**
1. Leave panel open for 30+ minutes
2. Periodically send queries
3. Verify no degradation

**Verification:**
- [ ] No memory leaks detected
- [ ] Connections properly cleaned up
- [ ] No resource exhaustion
- [ ] Python backend remains stable
- [ ] No increase in response times over time

**Status:** ✓ Pass / ✗ Fail

---

## Part 3: Build Verification

### Compilation Check

**Verification Checklist:**
- [ ] Plugin compiles without errors
- [ ] No linker errors
- [ ] No missing dependencies
- [ ] All headers found
- [ ] Module dependencies correct

### Static Analysis

If available, run static analysis:
```bash
# Example with clang-tidy or similar
UnrealBuildTool -Mode=Analyze
```

**Verification:**
- [ ] No critical warnings
- [ ] No null pointer issues
- [ ] No memory management issues

---

## Part 4: Documentation Review

Verify all documentation is complete:

**Required Documents:**
- [ ] WEEK4_COMPLETION.md exists and is complete
- [ ] WEEK4_VERIFICATION.md (this file) is complete
- [ ] README.md updated with Week 4 info
- [ ] Code comments are comprehensive
- [ ] All functions have documentation

---

## Success Criteria Summary

Week 4 is complete when ALL of the following are verified:

### Core Requirements
- [x] Plugin loads without errors
- [x] Python backend starts automatically
- [x] Dockable panel in UE Editor
- [x] Can send query from UI to Python
- [x] Display response in UI
- [x] Can query "What is Unreal Engine?" and get response
- [x] UI is functional and doesn't crash

### Quality Requirements
- [ ] Clean code with no warnings
- [ ] Comprehensive error handling
- [ ] Performance meets requirements (< 50ms)
- [ ] Memory management is sound
- [ ] UI is professional and polished
- [ ] Documentation is complete

---

## Troubleshooting

### Common Issues and Solutions

**Issue 1: Plugin doesn't appear in menu**
- Check that AdastreaDirectorEditor module is enabled
- Verify tab registration in StartupModule()
- Look for errors in Output Log
- Try restarting editor

**Issue 2: Python backend doesn't start**
- Check Python is in system PATH
- Verify ipc_server.py path is correct
- Check port 5555 is not already in use
- Look for Python errors in Output Log

**Issue 3: Queries return errors**
- Verify Python backend is running
- Check IPC connection is established
- Look at Python console for errors
- Verify JSON format is correct

**Issue 4: UI looks broken**
- Check Slate widget construction
- Verify all includes are correct
- Look for Slate layout errors in log
- Try different editor layout

**Issue 5: Build errors**
- Clean and rebuild solution
- Verify all module dependencies in Build.cs
- Check Unreal Engine version compatibility
- Update generated project files

---

## Test Results Template

Copy this template to record your test results:

```
=== Week 4 Verification Test Results ===

Date: ___________
Tester: ___________
UE Version: ___________
OS: ___________

Part 1: Python Backend
[ ] Step 1: IPC Server Standalone
[ ] Step 2: UI Integration Simulation  
[ ] Step 3: Individual Components

Part 2: UE Plugin
[ ] Step 1: Plugin Loading
[ ] Step 2: Menu Integration
[ ] Step 3: Panel Opening
[ ] Step 4: UI Layout
[ ] Step 5: Query Input
[ ] Step 6: Primary Success Test (CRITICAL)
[ ] Step 7: Additional Queries
[ ] Step 8: Results Display
[ ] Step 9: Panel Docking
[ ] Step 10: Error Handling
[ ] Step 11: Performance
[ ] Step 12: Memory/Resources

Part 3: Build
[ ] Compilation
[ ] Static Analysis

Part 4: Documentation
[ ] All docs complete

Overall Status: ___________

Notes:
___________________________________________
___________________________________________
___________________________________________
```

---

## Appendix: Expected Output Logs

### Successful Plugin Load
```
LogAdastreaDirector: AdastreaDirector Runtime Module: StartupModule
LogAdastreaDirector: Initializing Python Bridge...
LogAdastreaDirector: Attempting to connect to 127.0.0.1:5555
LogAdastreaDirector: Successfully connected to 127.0.0.1:5555
LogAdastreaDirector: Python Bridge initialized successfully
LogAdastreaDirectorEditor: AdastreaDirector Editor Module: StartupModule
LogAdastreaDirectorEditor: Registered Adastrea Director tab spawner
LogAdastreaDirectorEditor: Menu extensions registered
```

### Successful Query
```
LogAdastreaDirectorEditor: Spawning Adastrea Director tab
LogAdastreaDirector: Sending request: {"type":"query","data":"What is Unreal Engine?"}
LogAdastreaDirector: Received response: {"status":"success","result":"Unreal Engine is..."}
```

### Python Backend
```
INFO - IPC Server started on 127.0.0.1:5555
INFO - Client connected from ('127.0.0.1', 12345)
INFO - Query received: What is Unreal Engine?
INFO - Client ('127.0.0.1', 12345) disconnected
```

---

**End of Verification Guide**

Once all tests pass, Week 4 is complete and ready for integration into the full system!
