# Adastrea-Director Plugin - Architecture Diagram

**Date:** November 18, 2025  
**Status:** Implementation Complete (Weeks 1-6)  
**Version:** 1.0

---

## Table of Contents

1. [Overview](#overview)
2. [High-Level Architecture](#high-level-architecture)
3. [Component Architecture](#component-architecture)
4. [Module Details](#module-details)
5. [Data Flow Diagrams](#data-flow-diagrams)
6. [Communication Protocol](#communication-protocol)
7. [Deployment Architecture](#deployment-architecture)

---

## Overview

The Adastrea-Director Plugin uses a **hybrid architecture** combining:
- **C++ Plugin Shell** - Lightweight integration layer in Unreal Engine
- **Python Backend** - Powerful AI processing in separate subprocess
- **Local Socket IPC** - Fast communication between components

This design achieves optimal balance of:
- ✅ Performance (IPC <1ms latency)
- ✅ Maintainability (95% code reuse)
- ✅ Flexibility (Python ecosystem access)
- ✅ Integration (Native UE experience)

---

## High-Level Architecture

### System Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                       Unreal Engine Editor                           │
│                                                                      │
│  ┌────────────────────────────────────────────────────────────────┐ │
│  │              AdastreaDirector Plugin (C++)                     │ │
│  │                                                                 │ │
│  │  ┌──────────────────────┐        ┌─────────────────────────┐  │ │
│  │  │  Runtime Module      │        │  Editor Module          │  │ │
│  │  │  (AdastreaDirector)  │        │  (AdastreaDirectorEditor│  │ │
│  │  │                      │        │                         │  │ │
│  │  │  Core Services:      │        │  UI Services:           │  │ │
│  │  │  • Python Process    │        │  • Slate Panels         │  │ │
│  │  │    Management        │        │  • Menu Integration     │  │ │
│  │  │  • IPC Client        │        │  • Tab Management       │  │ │
│  │  │  • Bridge API        │        │  • Workspace Docking    │  │ │
│  │  │                      │        │  • Event Handling       │  │ │
│  │  └──────────┬───────────┘        └──────────┬──────────────┘  │ │
│  │             │                               │                  │ │
│  │             └───────────────┬───────────────┘                  │ │
│  │                             │                                  │ │
│  └─────────────────────────────┼──────────────────────────────────┘ │
│                                │                                    │
└────────────────────────────────┼────────────────────────────────────┘
                                 │
                                 │ IPC Protocol
                                 │ (JSON over TCP Sockets)
                                 │ Port: 5555 (configurable)
                                 │ Latency: <1ms
                                 │
                                 ▼
┌─────────────────────────────────────────────────────────────────────┐
│                    Python Backend (Subprocess)                       │
│                                                                      │
│  ┌────────────────────────────────────────────────────────────────┐ │
│  │                  IntegratedIPCServer                           │ │
│  │                                                                 │ │
│  │  ┌────────────┐  ┌────────────┐  ┌────────────┐  ┌─────────┐ │ │
│  │  │  Request   │  │  Handler   │  │  Response  │  │  Error  │ │ │
│  │  │  Router    │→│  Manager   │→│  Serializer│→│  Handler│ │ │
│  │  └────────────┘  └────────────┘  └────────────┘  └─────────┘ │ │
│  │                                                                 │ │
│  │  Request Handlers:                                              │ │
│  │  ├─ ping           Health check                                 │ │
│  │  ├─ query          RAG Q&A                                      │ │
│  │  ├─ ingest         Document ingestion                           │ │
│  │  ├─ db_info        Database stats                               │ │
│  │  ├─ clear_history  Reset conversation                           │ │
│  │  ├─ plan           Task planning (future)                       │ │
│  │  └─ analyze        Goal analysis (future)                       │ │
│  └─────────────────────────────────────────────────────────────────┘ │
│                                                                      │
│  ┌──────────────────────┐              ┌────────────────────────┐   │
│  │  RAGIngestionAgent   │              │   RAGQueryAgent        │   │
│  │                      │              │                        │   │
│  │  • Document Loader   │              │   • Query Processor    │   │
│  │  • Hash Detection    │              │   • Conversation Mgr   │   │
│  │  • Progress Tracking │              │   • Result Caching     │   │
│  │  • Batch Processing  │              │   • Source Tracking    │   │
│  │  • Error Recovery    │              │   • Performance Metrics│   │
│  └──────────┬───────────┘              └────────────┬───────────┘   │
│             │                                       │                │
│             └─────────────────┬─────────────────────┘                │
│                               │                                      │
└───────────────────────────────┼──────────────────────────────────────┘
                                │
                                ▼
                    ┌───────────────────────┐
                    │      ChromaDB         │
                    │  (Vector Database)    │
                    │                       │
                    │  • Collection Manager │
                    │  • Embedding Storage  │
                    │  • Similarity Search  │
                    │  • Persistence Layer  │
                    └───────────────────────┘
```

---

## Component Architecture

### Unreal Engine Plugin Layer

```
┌─────────────────────────────────────────────────────────────────┐
│                    AdastreaDirector Plugin                       │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │              AdastreaDirector (Runtime Module)           │  │
│  ├──────────────────────────────────────────────────────────┤  │
│  │                                                           │  │
│  │  FAdastreaDirectorModule                                 │  │
│  │  ├─ StartupModule()        Initialize plugin             │  │
│  │  ├─ ShutdownModule()       Cleanup resources             │  │
│  │  ├─ GetPythonBridge()      Access bridge API             │  │
│  │  └─ IsPluginReady()        Health check                  │  │
│  │                                                           │  │
│  │  ┌───────────────────────────────────────────────────┐   │  │
│  │  │  FPythonProcessManager                            │   │  │
│  │  ├───────────────────────────────────────────────────┤   │  │
│  │  │  Responsibilities:                                │   │  │
│  │  │  • Launch Python subprocess                       │   │  │
│  │  │  • Monitor subprocess health                      │   │  │
│  │  │  • Handle subprocess crashes                      │   │  │
│  │  │  • Terminate subprocess gracefully               │   │  │
│  │  │                                                   │   │  │
│  │  │  Key Methods:                                     │   │  │
│  │  │  ├─ StartPython()        Launch subprocess        │   │  │
│  │  │  ├─ StopPython()         Terminate subprocess     │   │  │
│  │  │  ├─ IsRunning()          Check status             │   │  │
│  │  │  ├─ Restart()            Restart on failure       │   │  │
│  │  │  └─ GetProcessInfo()     Debug information        │   │  │
│  │  └───────────────────────────────────────────────────┘   │  │
│  │                                                           │  │
│  │  ┌───────────────────────────────────────────────────┐   │  │
│  │  │  FIPCClient                                       │   │  │
│  │  ├───────────────────────────────────────────────────┤   │  │
│  │  │  Responsibilities:                                │   │  │
│  │  │  • Establish TCP socket connections               │   │  │
│  │  │  • Send requests to Python backend                │   │  │
│  │  │  • Receive responses from backend                 │   │  │
│  │  │  • Handle network errors                          │   │  │
│  │  │                                                   │   │  │
│  │  │  Key Methods:                                     │   │  │
│  │  │  ├─ Connect()           Establish connection      │   │  │
│  │  │  ├─ Disconnect()        Close connection          │   │  │
│  │  │  ├─ SendRequest()       Send JSON request         │   │  │
│  │  │  ├─ ReceiveResponse()   Read JSON response        │   │  │
│  │  │  └─ IsConnected()       Check connection status   │   │  │
│  │  │                                                   │   │  │
│  │  │  Performance:                                     │   │  │
│  │  │  • Latency: <1ms per request                      │   │  │
│  │  │  • Throughput: 1000+ requests/second              │   │  │
│  │  └───────────────────────────────────────────────────┘   │  │
│  │                                                           │  │
│  │  ┌───────────────────────────────────────────────────┐   │  │
│  │  │  FPythonBridge                                    │   │  │
│  │  ├───────────────────────────────────────────────────┤   │  │
│  │  │  Responsibilities:                                │   │  │
│  │  │  • High-level Python API                          │   │  │
│  │  │  • Request/response formatting                    │   │  │
│  │  │  • Error handling and recovery                    │   │  │
│  │  │  • Convenience methods                            │   │  │
│  │  │                                                   │   │  │
│  │  │  Key Methods:                                     │   │  │
│  │  │  ├─ Initialize()        Setup bridge              │   │  │
│  │  │  ├─ Shutdown()          Cleanup                   │   │  │
│  │  │  ├─ SendRequest()       High-level API            │   │  │
│  │  │  ├─ Query()             RAG query                 │   │  │
│  │  │  ├─ Ingest()            Start ingestion           │   │  │
│  │  │  └─ IsReady()           Status check              │   │  │
│  │  └───────────────────────────────────────────────────┘   │  │
│  └───────────────────────────────────────────────────────────┘  │
│                                                                  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │         AdastreaDirectorEditor (Editor Module)           │  │
│  ├──────────────────────────────────────────────────────────┤  │
│  │                                                           │  │
│  │  FAdastreaDirectorEditorModule                           │  │
│  │  ├─ StartupModule()        Register UI extensions       │  │
│  │  ├─ ShutdownModule()       Cleanup UI                   │  │
│  │  └─ RegisterMenus()        Add menu items               │  │
│  │                                                           │  │
│  │  ┌───────────────────────────────────────────────────┐   │  │
│  │  │  SAdastreaDirectorPanel (Slate Widget)            │   │  │
│  │  ├───────────────────────────────────────────────────┤   │  │
│  │  │  Responsibilities:                                │   │  │
│  │  │  • Main UI panel                                  │   │  │
│  │  │  • Tab management (Query, Ingestion)              │   │  │
│  │  │  • User input handling                            │   │  │
│  │  │  • Results display                                │   │  │
│  │  │  • Progress tracking                              │   │  │
│  │  │                                                   │   │  │
│  │  │  UI Components:                                   │   │  │
│  │  │  ├─ Query Tab                                     │   │  │
│  │  │  │  ├─ QueryInputBox        Text input            │   │  │
│  │  │  │  ├─ SendButton           Submit query          │   │  │
│  │  │  │  ├─ ClearHistoryButton   Reset context         │   │  │
│  │  │  │  └─ ResultsDisplay       Show response         │   │  │
│  │  │  │                                                │   │  │
│  │  │  └─ Ingestion Tab                                 │   │  │
│  │  │     ├─ DocsPathBox          Select docs folder    │   │  │
│  │  │     ├─ DbPathBox            Select DB path        │   │  │
│  │  │     ├─ StartButton          Begin ingestion       │   │  │
│  │  │     ├─ StopButton           Cancel ingestion      │   │  │
│  │  │     ├─ ProgressBar          Visual progress       │   │  │
│  │  │     ├─ StatusText           Current operation     │   │  │
│  │  │     └─ DetailsText          File being processed  │   │  │
│  │  │                                                   │   │  │
│  │  │  Key Methods:                                     │   │  │
│  │  │  ├─ Construct()          Build UI hierarchy       │   │  │
│  │  │  ├─ Tick()               Update progress          │   │  │
│  │  │  ├─ OnSendQuery()        Handle query submit      │   │  │
│  │  │  ├─ OnStartIngest()      Start ingestion          │   │  │
│  │  │  └─ UpdateProgress()     Refresh progress bar     │   │  │
│  │  └───────────────────────────────────────────────────┘   │  │
│  └───────────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────────────┘
```

### Python Backend Layer

```
┌─────────────────────────────────────────────────────────────────┐
│                     Python Backend                               │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │              IntegratedIPCServer                         │  │
│  ├──────────────────────────────────────────────────────────┤  │
│  │                                                           │  │
│  │  Responsibilities:                                        │  │
│  │  • Accept TCP socket connections                          │  │
│  │  • Parse JSON requests                                    │  │
│  │  • Route to appropriate handlers                          │  │
│  │  • Serialize responses                                    │  │
│  │  • Monitor performance                                    │  │
│  │                                                           │  │
│  │  Request Flow:                                            │  │
│  │  1. Accept connection                                     │  │
│  │  2. Receive JSON data                                     │  │
│  │  3. Parse request type                                    │  │
│  │  4. Call handler                                          │  │
│  │  5. Serialize response                                    │  │
│  │  6. Send response                                         │  │
│  │  7. Close connection                                      │  │
│  │                                                           │  │
│  │  Handlers:                                                │  │
│  │  ├─ handle_ping()          Health check                   │  │
│  │  ├─ handle_query()         RAG Q&A                        │  │
│  │  ├─ handle_ingest()        Document ingestion             │  │
│  │  ├─ handle_db_info()       Database statistics            │  │
│  │  ├─ handle_clear_history() Reset conversation             │  │
│  │  ├─ handle_plan()          Task planning (stub)           │  │
│  │  └─ handle_analyze()       Goal analysis (stub)           │  │
│  └───────────────────────────────────────────────────────────┘  │
│                                                                  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │              RAGIngestionAgent                           │  │
│  ├──────────────────────────────────────────────────────────┤  │
│  │                                                           │  │
│  │  Responsibilities:                                        │  │
│  │  • Load documents from filesystem                         │  │
│  │  • Detect changes via SHA-256 hashing                     │  │
│  │  • Split documents into chunks                            │  │
│  │  • Generate embeddings                                    │  │
│  │  • Store in ChromaDB                                      │  │
│  │  • Track ingestion progress                               │  │
│  │  • Report errors                                          │  │
│  │                                                           │  │
│  │  Supported Formats:                                       │  │
│  │  ├─ Markdown (.md)                                        │  │
│  │  ├─ Text (.txt)                                           │  │
│  │  ├─ PDF (.pdf)                                            │  │
│  │  ├─ Word (.docx)                                          │  │
│  │  ├─ Python (.py)                                          │  │
│  │  ├─ C++ (.cpp, .h)                                        │  │
│  │  └─ C# (.cs)                                              │  │
│  │                                                           │  │
│  │  Processing Pipeline:                                     │  │
│  │  1. Scan directory recursively                            │  │
│  │  2. Calculate file hashes                                 │  │
│  │  3. Compare with stored hashes                            │  │
│  │  4. Load changed files                                    │  │
│  │  5. Split into chunks                                     │  │
│  │  6. Generate embeddings                                   │  │
│  │  7. Upsert to ChromaDB                                    │  │
│  │  8. Update hash database                                  │  │
│  │  9. Report progress                                       │  │
│  │                                                           │  │
│  │  Performance:                                             │  │
│  │  • Throughput: 1-2 files/second                           │  │
│  │  • Hash check: 100 files/second                           │  │
│  │  • Memory: Streaming (low footprint)                      │  │
│  └───────────────────────────────────────────────────────────┘  │
│                                                                  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │              RAGQueryAgent                               │  │
│  ├──────────────────────────────────────────────────────────┤  │
│  │                                                           │  │
│  │  Responsibilities:                                        │  │
│  │  • Process user queries                                   │  │
│  │  • Search ChromaDB for relevant documents                 │  │
│  │  • Maintain conversation context                          │  │
│  │  • Generate responses with LLM                            │  │
│  │  • Cache query results                                    │  │
│  │  • Track source documents                                 │  │
│  │  • Measure performance                                    │  │
│  │                                                           │  │
│  │  Query Pipeline:                                          │  │
│  │  1. Receive query text                                    │  │
│  │  2. Check result cache (LRU, 50 entries)                  │  │
│  │  3. Generate query embedding                              │  │
│  │  4. Search ChromaDB for similar chunks                    │  │
│  │  5. Retrieve top-k relevant documents                     │  │
│  │  6. Build context with conversation history               │  │
│  │  7. Send to LLM for response generation                   │  │
│  │  8. Cache result                                          │  │
│  │  9. Return response with sources                          │  │
│  │                                                           │  │
│  │  Features:                                                │  │
│  │  ├─ Conversation history tracking                         │  │
│  │  ├─ Query result caching (LRU eviction)                   │  │
│  │  ├─ Source document tracking                              │  │
│  │  ├─ Performance metrics                                   │  │
│  │  └─ Context-aware responses                               │  │
│  │                                                           │  │
│  │  Performance:                                             │  │
│  │  • First query: 1-3 seconds                               │  │
│  │  • Cached query: <100ms                                   │  │
│  │  • Token usage: 1000-2000 tokens/query                    │  │
│  └───────────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────────────┘
```

---

## Module Details

### Runtime Module (AdastreaDirector)

**Purpose:** Core plugin functionality available at runtime

**Files:**
```
Source/AdastreaDirector/
├── AdastreaDirector.Build.cs         Build configuration
├── Public/
│   ├── AdastreaDirectorModule.h      Module interface
│   ├── PythonProcessManager.h        Subprocess management
│   ├── IPCClient.h                   IPC communication
│   └── PythonBridge.h                High-level API
└── Private/
    ├── AdastreaDirectorModule.cpp    Module implementation
    ├── PythonProcessManager.cpp      Process lifecycle (~300 lines)
    ├── IPCClient.cpp                 Socket communication (~250 lines)
    └── PythonBridge.cpp              Bridge implementation (~200 lines)
```

**Dependencies:**
- Core
- CoreUObject
- Engine
- Sockets (for IPC)
- Json (for serialization)

**Initialization Flow:**
1. `StartupModule()` called by UE
2. Create `FPythonProcessManager` instance
3. Create `FIPCClient` instance
4. Create `FPythonBridge` instance
5. Start Python subprocess
6. Wait for IPC server ready
7. Test connection with ping
8. Module ready

### Editor Module (AdastreaDirectorEditor)

**Purpose:** Editor-only UI and integration

**Files:**
```
Source/AdastreaDirectorEditor/
├── AdastreaDirectorEditor.Build.cs   Build configuration
├── Public/
│   ├── AdastreaDirectorEditorModule.h Editor module interface
│   └── SAdastreaDirectorPanel.h      Slate panel
└── Private/
    ├── AdastreaDirectorEditorModule.cpp Editor integration
    └── SAdastreaDirectorPanel.cpp    UI implementation (~800 lines)
```

**Dependencies:**
- AdastreaDirector (runtime module)
- Core
- CoreUObject
- Engine
- Slate
- SlateCore
- EditorStyle
- UnrealEd
- WorkspaceMenuStructure

**UI Integration:**
1. `StartupModule()` called by UE
2. Register menu commands
3. Register tab spawner
4. Add to Window > Developer Tools menu
5. Ready for user interaction

---

## Data Flow Diagrams

### Query Flow

```
User Action: Enter Query + Press Enter
         │
         ▼
┌──────────────────────────────────────────┐
│  SAdastreaDirectorPanel::OnSendQuery()   │
│  • Validate input                        │
│  • Get query text                        │
│  • Call SendQueryToPython()              │
└──────────┬───────────────────────────────┘
           │
           ▼
┌──────────────────────────────────────────┐
│  FPythonBridge::SendRequest()            │
│  • Format JSON request                   │
│  • Call IPCClient                        │
└──────────┬───────────────────────────────┘
           │
           ▼
┌──────────────────────────────────────────┐
│  FIPCClient::SendRequest()               │
│  • Connect to localhost:5555             │
│  • Send JSON over socket                 │
│  • Wait for response                     │
└──────────┬───────────────────────────────┘
           │ Network (TCP Socket)
           ▼
┌──────────────────────────────────────────┐
│  Python: IntegratedIPCServer             │
│  • Accept connection                     │
│  • Parse JSON request                    │
│  • Route to handle_query()               │
└──────────┬───────────────────────────────┘
           │
           ▼
┌──────────────────────────────────────────┐
│  Python: RAGQueryAgent.process_query()   │
│  • Check cache                           │
│  • Generate query embedding              │
│  • Search ChromaDB                       │
│  • Retrieve relevant docs                │
│  • Build context                         │
│  • Call LLM                              │
│  • Generate response                     │
└──────────┬───────────────────────────────┘
           │
           ▼
┌──────────────────────────────────────────┐
│  ChromaDB: Similarity Search             │
│  • Compute similarity scores             │
│  • Return top-k chunks                   │
│  • Include metadata                      │
└──────────┬───────────────────────────────┘
           │
           ▼
┌──────────────────────────────────────────┐
│  Python: Serialize Response              │
│  • Create JSON response                  │
│  • Include answer                        │
│  • Include sources                       │
│  • Include timing                        │
│  • Send back over socket                 │
└──────────┬───────────────────────────────┘
           │ Network (TCP Socket)
           ▼
┌──────────────────────────────────────────┐
│  FIPCClient::ReceiveResponse()           │
│  • Read JSON from socket                 │
│  • Disconnect                            │
│  • Return to bridge                      │
└──────────┬───────────────────────────────┘
           │
           ▼
┌──────────────────────────────────────────┐
│  FPythonBridge::SendRequest() (return)   │
│  • Parse JSON response                   │
│  • Extract answer                        │
│  • Return to UI                          │
└──────────┬───────────────────────────────┘
           │
           ▼
┌──────────────────────────────────────────┐
│  SAdastreaDirectorPanel::UpdateResults() │
│  • Format response text                  │
│  • Update ResultsDisplay widget          │
│  • Show processing time                  │
└──────────────────────────────────────────┘

Total Round-Trip Time: 1-3 seconds
  • IPC overhead: <1ms
  • RAG processing: 1-3s
  • UI update: <1ms
```

### Ingestion Flow

```
User Action: Select Folder + Click "Start Ingestion"
         │
         ▼
┌──────────────────────────────────────────┐
│  SAdastreaDirectorPanel::               │
│    OnStartIngestionClicked()            │
│  • Get docs path                        │
│  • Get DB path                          │
│  • Validate paths                       │
│  • Set ingestion flag                   │
│  • Call SendIngestionRequest()          │
└──────────┬───────────────────────────────┘
           │
           ▼
┌──────────────────────────────────────────┐
│  FPythonBridge::SendRequest("ingest")   │
│  • Format JSON with paths               │
│  • Include progress file path           │
│  • Call IPCClient                       │
└──────────┬───────────────────────────────┘
           │
           ▼
┌──────────────────────────────────────────┐
│  FIPCClient::SendRequest()               │
│  • Send ingestion parameters            │
│  • Return immediately (non-blocking)    │
└──────────┬───────────────────────────────┘
           │ Network (TCP Socket)
           ▼
┌──────────────────────────────────────────┐
│  Python: IntegratedIPCServer             │
│  • Route to handle_ingest()             │
└──────────┬───────────────────────────────┘
           │
           ▼
┌──────────────────────────────────────────┐
│  Python: RAGIngestionAgent.ingest()     │
│  • Scan directory                       │
│  • For each file:                       │
│    ├─ Calculate hash                    │
│    ├─ Check if changed                  │
│    ├─ Load if needed                    │
│    ├─ Split into chunks                 │
│    ├─ Generate embeddings               │
│    ├─ Upsert to ChromaDB                │
│    └─ Write progress to file            │◄──┐
└──────────┬───────────────────────────────┘   │
           │                                    │
           │ Writes progress file               │
           │                                    │
           ▼                                    │
  Progress File (JSON)                          │
  {                                             │
    "status": "processing",                     │
    "progress": 45,                             │
    "current_file": "doc.md",                   │
    "stats": {                                  │
      "total": 100,                             │
      "processed": 45                           │
    }                                           │
  }                                             │
           │                                    │
           │ Read by UI Tick()                  │
           ▼                                    │
┌──────────────────────────────────────────┐   │
│  SAdastreaDirectorPanel::Tick()          │   │
│  • Check if ingesting                    │   │
│  • Read progress file                    │   │
│  • Parse JSON                            │   │
│  • Update progress bar                   │   │
│  • Update status text                    │───┘
│  • Update details text                   │
└──────────┬───────────────────────────────┘
           │
           │ (When complete)
           ▼
┌──────────────────────────────────────────┐
│  Python: Send completion response        │
│  • Serialize statistics                  │
│  • Return to C++                         │
└──────────┬───────────────────────────────┘
           │ Network (TCP Socket)
           ▼
┌──────────────────────────────────────────┐
│  UI: Display completion message          │
│  • Show statistics                       │
│  • Reset progress bar                    │
│  • Clear ingestion flag                  │
└──────────────────────────────────────────┘
```

---

## Communication Protocol

### IPC Protocol Specification

**Transport:** TCP Sockets (IPv4)  
**Port:** 5555 (default, configurable)  
**Encoding:** UTF-8  
**Format:** JSON

### Request Structure

```json
{
    "type": "<request_type>",
    "data": "<request_data>"
}
```

**Fields:**
- `type` (string, required): Request type identifier
- `data` (string/object, required): Request-specific data

### Response Structure

```json
{
    "status": "success|error",
    "message": "<optional_message>",
    "result": "<response_data>",
    "processing_time": 0.123
}
```

**Fields:**
- `status` (string, required): "success" or "error"
- `message` (string, optional): Human-readable message
- `result` (any, optional): Response data
- `processing_time` (float, optional): Seconds taken

### Supported Request Types

#### 1. ping
**Purpose:** Health check

**Request:**
```json
{"type": "ping", "data": ""}
```

**Response:**
```json
{
    "status": "success",
    "message": "pong",
    "processing_time": 0.001
}
```

#### 2. query
**Purpose:** RAG Q&A

**Request:**
```json
{
    "type": "query",
    "data": "How do I create a faction?"
}
```

**Response:**
```json
{
    "status": "success",
    "result": {
        "answer": "To create a faction...",
        "sources": ["FactionSetupGuide.md"],
        "cached": false
    },
    "processing_time": 1.234
}
```

#### 3. ingest
**Purpose:** Document ingestion

**Request:**
```json
{
    "type": "ingest",
    "data": {
        "docs_dir": "/path/to/docs",
        "persist_dir": "/path/to/db",
        "progress_file": "/path/to/progress.json",
        "force_reingest": false
    }
}
```

**Response:**
```json
{
    "status": "success",
    "result": {
        "added": 50,
        "updated": 10,
        "skipped": 40,
        "errors": 0
    },
    "processing_time": 123.456
}
```

#### 4. db_info
**Purpose:** Database statistics

**Request:**
```json
{"type": "db_info", "data": ""}
```

**Response:**
```json
{
    "status": "success",
    "result": {
        "collection_name": "adastrea_docs",
        "document_count": 100,
        "persist_directory": "/path/to/db"
    },
    "processing_time": 0.050
}
```

#### 5. clear_history
**Purpose:** Reset conversation

**Request:**
```json
{"type": "clear_history", "data": ""}
```

**Response:**
```json
{
    "status": "success",
    "message": "Conversation history cleared",
    "processing_time": 0.001
}
```

---

## Deployment Architecture

### Development Environment

```
Developer Machine
├── Unreal Engine 5.6+
│   └── Project/
│       ├── Adastrea.uproject
│       └── Plugins/
│           └── AdastreaDirector/
│               ├── Source/ (C++ code)
│               ├── Python/ (Backend scripts)
│               └── Content/ (UI assets)
│
├── Python 3.9+ Installation
│   └── site-packages/
│       ├── chromadb
│       ├── langchain
│       └── (other dependencies)
│
└── Documentation (for ingestion)
    ├── Assets/
    ├── Docs/
    └── README.md
```

### Runtime Architecture

```
┌───────────────────────────────────────────┐
│  Unreal Engine Editor Process             │
│  PID: 12345                               │
│  Memory: ~2-4 GB                          │
│                                           │
│  ┌─────────────────────────────────────┐ │
│  │  AdastreaDirector Plugin            │ │
│  │  • C++ modules loaded              │ │
│  │  • UI integrated                   │ │
│  └─────────────────────────────────────┘ │
└───────────────┬───────────────────────────┘
                │ Spawns
                ▼
┌───────────────────────────────────────────┐
│  Python Subprocess                        │
│  PID: 12346                               │
│  Memory: ~100-500 MB                      │
│                                           │
│  ┌─────────────────────────────────────┐ │
│  │  IPC Server                         │ │
│  │  Port: 5555                         │ │
│  └─────────────────────────────────────┘ │
│                                           │
│  ┌─────────────────────────────────────┐ │
│  │  RAG Agents                         │ │
│  │  • Ingestion                        │ │
│  │  • Query                            │ │
│  └─────────────────────────────────────┘ │
└───────────────┬───────────────────────────┘
                │ Accesses
                ▼
┌───────────────────────────────────────────┐
│  Filesystem                               │
│                                           │
│  ┌─────────────────────────────────────┐ │
│  │  ChromaDB Database                  │ │
│  │  Location: <Project>/chroma_db/     │ │
│  │  Size: ~10-100 MB per 100 docs      │ │
│  └─────────────────────────────────────┘ │
│                                           │
│  ┌─────────────────────────────────────┐ │
│  │  Documentation                      │ │
│  │  Location: <Project>/Assets/        │ │
│  │  Size: ~5-50 MB                     │ │
│  └─────────────────────────────────────┘ │
└───────────────────────────────────────────┘
```

### Resource Requirements

**Unreal Engine Process:**
- CPU: ~10-20% (idle), ~50-70% (active)
- Memory: 2-4 GB
- Disk: Minimal (read-only plugin code)

**Python Subprocess:**
- CPU: ~5% (idle), ~50-100% (processing)
- Memory: 100-500 MB
- Disk: ChromaDB writes, progress files

**Total Overhead:**
- CPU: ~15-25% idle, ~100% peak
- Memory: ~2.5-5 GB total
- Disk: ~10-100 MB for database

---

## Summary

The Adastrea-Director Plugin architecture achieves:

✅ **Clean Separation of Concerns**
- C++ handles UE integration
- Python handles AI processing
- IPC bridges the two

✅ **High Performance**
- IPC latency <1ms
- RAG queries 1-3s
- UI responsive

✅ **Maintainability**
- Modular design
- Clear interfaces
- Comprehensive documentation

✅ **Extensibility**
- Easy to add new request types
- UI components reusable
- Python agents pluggable

This architecture provides a solid foundation for future enhancements including planning agents, code generation, and autonomous monitoring.

---

**Document Status:** ✅ Complete  
**Last Updated:** November 18, 2025  
**Version:** 1.0

For implementation details, see:
- `POC_PLAN.md` - Proof of concept plan
- `RISK_ASSESSMENT.md` - Risk analysis
- `README.md` - Plugin overview
