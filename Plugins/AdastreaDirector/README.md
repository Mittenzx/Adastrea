# Adastrea Director - Unreal Engine Plugin

An AI-powered development assistant plugin for Unreal Engine that provides intelligent code assistance, automated planning, performance profiling, and bug detection capabilities.

---

## 🚀 Quick Start

**New to the plugin?** Start here:

👉 **[SETUP_GUIDE.md](SETUP_GUIDE.md)** - Get started in 5 minutes!

This quick guide covers:
- ✅ Installation (3 steps)
- ✅ Basic usage
- ✅ Simple testing
- ✅ Troubleshooting

For comprehensive documentation, continue reading below.

---

## Overview

This plugin integrates the Adastrea Director AI system directly into Unreal Engine, enabling developers to leverage advanced AI capabilities without leaving the editor. The plugin uses a hybrid architecture with a lightweight C++ shell in UE and a powerful Python backend for AI processing.

## Current Status

**Phase 1: Plugin Shell - Weeks 1-4 Complete ✅**

The plugin currently provides:

### Week 1: Project Setup ✅
- ✅ Basic plugin structure and module organization
- ✅ Runtime module for core functionality
- ✅ Editor module for Unreal Editor integration
- ✅ Build system configuration (.uplugin and .Build.cs files)

### Week 2: Python Bridge ✅
- ✅ Python subprocess management (`FPythonProcessManager`)
- ✅ IPC socket communication (`FIPCClient`)
- ✅ High-level bridge interface (`FPythonBridge`)
- ✅ Python IPC server with extensible handlers
- ✅ JSON request/response serialization
- ✅ Error handling and recovery mechanisms

### Week 3: Python Backend IPC ✅
- ✅ Performance monitoring with detailed metrics
- ✅ Optimized request routing (< 1ms latency)
- ✅ Response serialization with processing time
- ✅ Comprehensive performance test suite
- ✅ Integration framework for RAG/Agents
- ✅ Production-ready error handling

### Week 4: Basic UI ✅
- ✅ Main Slate panel (`SAdastreaDirectorPanel`)
- ✅ Editor menu integration (Window > Developer Tools > Adastrea Director)
- ✅ Query input widget with button and Enter key support
- ✅ Results display widget with scrolling and text selection
- ✅ Dockable panel integrated with UE workspace
- ✅ Automatic Python backend initialization
- ✅ End-to-end communication tested (< 1ms latency)

### NEW: UE Python API Integration ✅
- ✅ Direct UE Python API access (`import unreal`)
- ✅ Comprehensive API wrapper (`ue_python_api.py`)
- ✅ IPC integration for hybrid architecture
- ✅ Asset operations (query, load, save)
- ✅ Actor operations (spawn, query, delete)
- ✅ Console command execution
- ✅ Editor utilities (notifications, logging)
- ✅ Complete documentation and examples

### Week 7-8 Features ✅
- ✅ Settings dialog with API key management
- ✅ Keyboard shortcuts (Ctrl+, for Settings)
- ✅ Enhanced error handling with user-friendly messages
- ✅ Conversation history with clear confirmation

### UI Enhancement: Tabbed Interface ✅ (November 2025)
- ✅ Professional tabbed interface with Query, Ingestion, and Dashboard tabs
- ✅ Radio button style tab switching
- ✅ Full access to document ingestion features
- ✅ Progress bar and status updates during ingestion
- ✅ Smooth tab switching with state preservation
- ✅ Dashboard with comprehensive status indicator lights
- ✅ Real-time system health monitoring (6 status indicators)
- ✅ Color-coded status lights for easy troubleshooting
- ✅ Automatic status updates every 0.5 seconds

### C++ Implementation ✅ (December 2025)
- ✅ Native C++ API for Unreal Engine operations
- ✅ StandardResult for consistent error handling
- ✅ UEBridge class - C++ equivalent of Python ue_python_api
- ✅ AssetHelpers class - C++ equivalent of Python adastrea_helpers
- ✅ Blueprint Function Library for easy Blueprint access
- ✅ No Python dependency for core UE operations
- ✅ 10-50x performance improvement over IPC calls
- ✅ Full documentation and examples

👉 **See [CPP_IMPLEMENTATION_GUIDE.md](CPP_IMPLEMENTATION_GUIDE.md)** for complete C++ API documentation

**Coming Soon:**
- Planning agent integration
- Performance profiling UI
- Agent orchestration dashboard

## Quick Start

### Using the AI Assistant Panel

1. Open Unreal Engine Editor with the plugin installed
2. Go to **Window > Developer Tools > Adastrea Director**
3. The AI assistant panel will open as a dockable tab
4. Configure your API keys via the Settings dialog (see below)

#### Query Tab
5. Click the **Query** tab (default view)
6. Type your question in the query input field
7. Click "Send Query" or press Enter
8. View the response in the results area below

**Example Query:** "What is Unreal Engine?"

#### Ingestion Tab
5. Click the **Ingestion** tab
6. Select your documentation folder (Browse button)
7. Configure your database path (Browse button or default)
8. Click "Start Ingestion" to process documents
9. Monitor progress with the real-time progress bar
10. View detailed status messages during processing

#### Dashboard Tab (NEW)
1. Click the **Dashboard** tab to view system status
2. **Status Indicators**: See 6 color-coded lights showing component health:
   - 🟢 Python Process - Backend process running state
   - 🟢 IPC Connection - Socket connection status
   - 🟢 Python Bridge Ready - Overall bridge readiness
   - 🟢 Backend Health - Backend operational status
   - 🟢 Query Processing - Query system availability
   - 🟢 Document Ingestion - Ingestion system status
3. **Quick Status Check**: All green = system healthy!
4. **Detailed Status**: View connection details and PID/Port info
5. **System Logs**: Real-time log output from the backend
6. **Actions**: Refresh Status or Reconnect if needed

**Status Light Colors:**
- 🟢 Green = Working correctly
- 🟡 Yellow = Warning or operation in progress
- 🔴 Red = Error or not functional
- ⚪ Gray = Unknown or checking

**See [STATUS_INDICATORS.md](STATUS_INDICATORS.md) for complete troubleshooting guide.**

The Python backend starts automatically when the plugin loads!

### Settings Dialog (NEW in Week 7-8)

Access the settings dialog in two ways:

1. **Settings Button**: Click the "⚙️ Settings" button in the top-right of the panel
2. **Keyboard Shortcut**: Press `Ctrl+,` (Ctrl + Comma)

The settings dialog allows you to configure:

#### API Keys Section
- **LLM Provider**: Choose between Gemini (Recommended) or OpenAI
- **Gemini API Key**: Secure input for your Gemini API key (masked with • characters)
- **OpenAI API Key**: Secure input for your OpenAI API key (masked with • characters)
- **Embedding Provider**: Choose between HuggingFace (Free) or OpenAI

#### Display Settings Section
- **Default Font Size**: Adjust font size (8-20pt range)
- **Auto-save Settings**: Automatically save settings when clicking Save
- **Show Timestamps**: Display timestamps in conversation history

#### Configuration Storage
Settings are stored in your project's Saved directory:
- Path: `<Project>/Saved/AdastreaDirector/config.ini`
- Format: Simple key=value pairs for easy editing if needed
- **Security Note**: API keys are stored in plaintext. Keep this file secure and ensure it's in your .gitignore
- **Note**: Manual edits to comments may be overwritten when saving from the UI

### Keyboard Shortcuts

- `Enter` - Send query (when focused on query input)
- `Ctrl+,` - Open Settings dialog (requires panel to have focus)
- More shortcuts coming in future updates!

### Tabbed Interface (NEW - November 2025)

The plugin now features a professional tabbed interface that provides access to all functionality:

#### Query Tab (Default)
The Query tab is the main interface for interacting with the AI assistant:
- **Query Input**: Enter your questions or commands
- **Send Query Button**: Submit your query to the AI backend
- **Clear History Button**: Clear conversation history (with confirmation)
- **Results Display**: View AI-generated responses with context from your documentation
- **Features**:
  - Enter key support for quick queries
  - Conversation history maintained across queries
  - Context-aware responses using RAG (Retrieval Augmented Generation)
  - Scrollable results with text selection

#### Ingestion Tab
The Ingestion tab allows you to populate the knowledge base:
- **Documentation Folder**: Select the folder containing your documentation
  - Browse button for easy folder selection
  - Default path: `<Project>/Docs`
  - Supports: Markdown, Python, C++, C#, JSON, YAML, and more
- **Database Path**: Configure where ChromaDB stores the vector database
  - Browse button for path selection
  - Default path: `<Project>/chroma_db`
- **Control Buttons**:
  - Start Ingestion: Begin processing documents
  - Stop: Halt ingestion process
- **Progress Monitoring**:
  - Real-time progress bar (0-100%)
  - Status messages showing current operation
  - Detailed information about files being processed
- **Features**:
  - Incremental ingestion (only processes new/changed files)
  - Hash-based change detection
  - Batch processing for performance
  - Error handling and recovery

#### Tab Switching
- Click any tab button to switch views
- Only one tab can be active at a time
- Tab state is preserved when switching
- Radio button style indicates active tab

**Note**: For detailed technical information about the tabbed interface implementation, see [UI_ENHANCEMENT_TABBED_INTERFACE.md](UI_ENHANCEMENT_TABBED_INTERFACE.md).

### Error Handling

The plugin includes comprehensive error handling:

#### User-Friendly Error Messages
- Clear, actionable error messages for common issues
- No technical jargon - errors explain what went wrong and how to fix it
- Visual indicators (❌, ⚠️, ✓) for different message types

#### Error Recovery
- Graceful handling of network issues
- Automatic retry logic for transient failures
- Clear status updates during operations

#### Common Error Scenarios
1. **Python Backend Not Ready**
   - Message: "Python backend is not ready. Please check that the Python backend is running and connected."
   - Solution: Wait for backend to initialize or restart the editor

2. **Empty Query**
   - Message: "Query cannot be empty."
   - Solution: Enter a question before clicking Send

3. **Invalid Response**
   - Message: "Invalid response format" with details
   - Solution: Check Python backend logs for issues

4. **Clear History Confirmation**
   - Confirmation dialog before clearing history
   - Prevents accidental data loss

### Conversation History

The plugin maintains conversation history for context-aware responses:

- **Persistent Context**: Previous questions and answers are remembered
- **Clear History**: Use the "Clear History" button to start fresh
- **Confirmation Dialog**: Prevents accidental clearing of conversation
- **Backend Integration**: History is managed by the Python backend

### Testing the Python Backend

```bash
# Test the IPC server standalone
cd Plugins/AdastreaDirector/Python
python3 ipc_server.py

# In another terminal, run integration tests
python3 test_ui_integration.py
```

See `WEEK4_VERIFICATION.md` for comprehensive testing procedures.

## Installation

### For Development

1. Copy the entire `Plugins/AdastreaDirector` folder into your Unreal Engine project's `Plugins` directory
2. Regenerate project files (right-click .uproject → Generate Visual Studio project files)
3. Build your project
4. Launch Unreal Engine Editor
5. The plugin will be loaded automatically

### Verifying Installation

1. Open your project in Unreal Engine Editor
2. Go to Edit → Plugins
3. Search for "Adastrea Director"
4. The plugin should appear in the "Developer Tools" category
5. Check the console log for "AdastreaDirector Module: StartupModule" messages

## Architecture

### Hybrid Approach

The plugin uses a hybrid architecture as specified in `PLUGIN_DEVELOPMENT_FEASIBILITY.md`:

```
┌─────────────────────────────────────┐
│  Unreal Engine Editor (C++)        │
│  - Slate UI                         │
│  - Python Bridge                    │
│  - Plugin Module                    │
└──────────────┬──────────────────────┘
               │ Local Socket IPC
               ▼
┌─────────────────────────────────────┐
│  Python Backend (Subprocess)        │
│  - RAG Documentation System         │
│  - Planning Agents                  │
│  - LLM Integration                  │
└─────────────────────────────────────┘
```

### Module Structure

- **AdastreaDirector** (Runtime Module)
  - Core functionality accessible at runtime
  - ✅ Python subprocess management (`FPythonProcessManager`)
  - ✅ IPC communication layer (`FIPCClient`)
  - ✅ High-level bridge (`FPythonBridge`)
  
- **AdastreaDirectorEditor** (Editor Module)
  - Editor-only functionality
  - ✅ Slate UI panel (`SAdastreaDirectorPanel`)
  - ✅ Menu and toolbar integration
  - ✅ Tab spawner registration
  - Asset actions (future)

- **Python/** (Backend Scripts)
  - ✅ `ipc_server.py` - IPC server with request routing
  - ✅ `test_ipc.py` - Unit tests for IPC communication
  - ✅ `test_ipc_performance.py` - Performance validation
  - ✅ `test_ui_integration.py` - End-to-end integration tests
  - ✅ `README.md` - Backend documentation

## File Structure

```
Plugins/AdastreaDirector/
├── AdastreaDirector.uplugin          # Plugin descriptor
├── README.md                         # This file
├── WEEK1_COMPLETION.md               # Week 1 detailed report
├── WEEK2_COMPLETION.md               # Week 2 detailed report
├── Resources/
│   └── Icon128.txt                   # Plugin icon (placeholder)
├── Python/                           # Backend scripts
│   ├── ipc_server.py                 # IPC server
│   ├── test_ipc.py                   # IPC test script
│   └── README.md                     # Backend documentation
├── Source/
│   ├── AdastreaDirector/             # Runtime module
│   │   ├── AdastreaDirector.Build.cs
│   │   ├── Public/
│   │   │   ├── AdastreaDirectorModule.h
│   │   │   ├── PythonProcessManager.h
│   │   │   ├── IPCClient.h
│   │   │   └── PythonBridge.h
│   │   └── Private/
│   │       ├── AdastreaDirectorModule.cpp
│   │       ├── PythonProcessManager.cpp
│   │       ├── IPCClient.cpp
│   │       └── PythonBridge.cpp
│   └── AdastreaDirectorEditor/       # Editor module
│       ├── AdastreaDirectorEditor.Build.cs
│       ├── Public/
│       │   └── AdastreaDirectorEditorModule.h
│       └── Private/
│           └── AdastreaDirectorEditorModule.cpp
└── Content/
    └── UI/
        └── EditorWidgets/            # Future: UI assets
```

## Development Roadmap

### Phase 1: Plugin Shell (Weeks 1-4)

**Week 1: Project Setup** ✅ COMPLETE
- [x] Create plugin folder structure
- [x] Write .uplugin descriptor
- [x] Create build scripts (.Build.cs)
- [x] Set up version control
- [x] Documentation

**Week 2: Python Bridge** ✅ COMPLETE
- [x] Implement subprocess management
- [x] Create IPC socket communication
- [x] Handle Python process lifecycle
- [x] Error handling and recovery
- [x] Python IPC server
- [x] Test scripts and documentation

**Week 3: Python Backend IPC** ✅ COMPLETE
- [x] Create Python IPC server
- [x] Implement request router with performance monitoring
- [x] Add response serialization with timing
- [x] Test communication with plugin
- [x] Performance optimization (< 1ms latency, exceeds 50ms target by 50x)

**Week 4: Basic UI**
- [ ] Create main Slate panel
- [ ] Add to Editor menu
- [ ] Query input widget
- [ ] Results display widget

### Phase 2: RAG Integration (Weeks 5-8)
- Document ingestion UI
- Query system integration
- Settings dialog

### Phase 3: Planning Features (Weeks 9-12)
- Goal analysis UI
- Task decomposition display
- Code generation interface

### Phase 4: Polish & Release (Weeks 13-16)
- Cross-platform testing
- Documentation
- Fab marketplace submission

## Requirements

### Unreal Engine
- Unreal Engine 5.1 or higher
- Supported platforms: Windows, Mac, Linux

### Dependencies
- Python 3.9+ (for backend)
- PythonScriptPlugin (optional, enabled in .uplugin)

### Python Backend
The plugin requires the Adastrea Director Python backend to be available. See the main repository README for setup instructions:
- ChromaDB for vector storage
- LangChain for LLM orchestration
- OpenAI API (or compatible alternative)

## Usage

### Testing the Python Bridge

The Python bridge can be tested independently:

1. **Start the IPC server:**
   ```bash
   cd Plugins/AdastreaDirector/Python
   python ipc_server.py --port 5555
   ```

2. **Run the test script:**
   ```bash
   python test_ipc.py 5555
   ```

This will verify:
- Server starts correctly
- TCP socket communication works
- JSON serialization is working
- All request handlers respond correctly

### C++ Integration (Week 2+)

From C++ code, use the bridge like this:

```cpp
// Get the module
FAdastreaDirectorModule& Module = FModuleManager::LoadModuleChecked<FAdastreaDirectorModule>("AdastreaDirector");

// Get the Python bridge
FPythonBridge* Bridge = Module.GetPythonBridge();

if (Bridge && Bridge->IsReady())
{
    // Send a query
    FString Response;
    if (Bridge->SendRequest(TEXT("query"), TEXT("What is Unreal Engine?"), Response))
    {
        UE_LOG(LogAdastreaDirector, Log, TEXT("Response: %s"), *Response);
    }
}
```

### Python IPC Server

The IPC server supports the following request types:

- **ping** - Health check
  ```json
  {"type": "ping", "data": ""}
  // Response: {"status": "success", "message": "pong"}
  ```

- **query** - Documentation queries
  ```json
  {"type": "query", "data": "Your question here"}
  // Response: {"status": "success", "result": "...", "sources": [...]}
  ```

- **plan** - Task planning
  ```json
  {"type": "plan", "data": "Development goal"}
  // Response: {"status": "success", "plan": {...}}
  ```

- **analyze** - Goal analysis
  ```json
  {"type": "analyze", "data": "Feature description"}
  // Response: {"status": "success", "analysis": {...}}
  ```

See `Python/README.md` for detailed documentation on the IPC protocol.

## Configuration

### Plugin Settings (Future)

Settings will be configurable through:
- Editor Preferences → Plugins → Adastrea Director
- Project Settings → Plugins → Adastrea Director

Planned settings include:
- Python backend path
- IPC port configuration
- API keys (encrypted)
- Model selection
- Performance options

## Building the Plugin

### From Unreal Engine Editor
1. File → Generate Visual Studio Project Files
2. Build → Rebuild Solution (or Build → Build Solution)

### From Command Line (Windows)
```cmd
"C:\Program Files\Epic Games\UE_5.3\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin ^
  -Plugin="C:\Path\To\Project\Plugins\AdastreaDirector\AdastreaDirector.uplugin" ^
  -Package="C:\Output\Path"
```

### From Command Line (Mac/Linux)
```bash
/Path/To/UnrealEngine/Engine/Build/BatchFiles/RunUAT.sh BuildPlugin \
  -Plugin="/Path/To/Project/Plugins/AdastreaDirector/AdastreaDirector.uplugin" \
  -Package="/Output/Path"
```

## Testing

### Manual Testing
1. Load the plugin in UE Editor
2. Check console log for startup messages
3. Verify plugin appears in Edit → Plugins

### Automated Testing (Future)
- Unit tests for C++ components
- Integration tests for Python bridge
- UI tests for Slate panels

## Troubleshooting

### Plugin Doesn't Load
- Check the Output Log for error messages
- Verify all .uplugin and .Build.cs files are correct
- Regenerate project files
- Rebuild the project

### Build Errors
- Ensure Unreal Engine version meets requirements
- Check that all module dependencies are available
- Verify file paths are correct

### Runtime Errors
- Check that Python backend is accessible
- Verify API keys are configured (future)
- Review console logs for specific errors

## Contributing

This plugin is part of the Adastrea Director project. For contribution guidelines, see the main repository:
- [CONTRIBUTING.md](../../CONTRIBUTING.md)
- [ROADMAP.md](../../ROADMAP.md)

## Support

- **Issues**: [GitHub Issues](https://github.com/Mittenzx/Adastrea-Director/issues)
- **Documentation**: [Main README](../../README.md)
- **📖 Remote Connection Types & Actions**: [Comprehensive Guide](../../wiki/Remote-Connection-Types-and-Actions.md) - All connection types, actions, and comparisons
- **Feasibility Study**: [PLUGIN_DEVELOPMENT_FEASIBILITY.md](../../PLUGIN_DEVELOPMENT_FEASIBILITY.md)

## License

[To be determined]

## Credits

- **Project Lead**: Mittenzx
- **Architecture**: Based on PLUGIN_DEVELOPMENT_FEASIBILITY.md
- **Development**: GitHub Copilot Workspace

---

*"Building tomorrow's game development tools, today."*
