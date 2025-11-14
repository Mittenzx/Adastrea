# Adastrea-Director Plugin Development Guide

**Last Updated:** November 14, 2025  
**Status:** Planning Phase  
**Target Release:** Q3 2026

---

## Overview

This guide outlines the development plan for converting Adastrea-Director from an external Python tool into a native Unreal Engine plugin. This integration will provide seamless AI-powered development assistance directly within the Unreal Editor.

---

## Table of Contents

1. [Strategic Vision](#strategic-vision)
2. [Architecture Overview](#architecture-overview)
3. [Development Phases](#development-phases)
4. [Module Structure](#module-structure)
5. [Implementation Details](#implementation-details)
6. [Testing Strategy](#testing-strategy)
7. [Deployment Plan](#deployment-plan)
8. [Maintenance & Updates](#maintenance--updates)

---

## Strategic Vision

### Why a Plugin?

**Current State (External Tool):**
- ✅ Functional Phase 1-2 (RAG + Planning)
- ✅ Production-ready quality
- ⚠️ Requires separate application
- ⚠️ Context switching overhead
- ⚠️ Manual workflow integration

**Future State (Native Plugin):**
- ✅ Seamless editor integration
- ✅ Single-window workflow
- ✅ Direct engine access
- ✅ Better UX for developers
- ✅ Distribution via UE Marketplace

### Strategic Goals

1. **Developer Experience:** Eliminate context switching, integrate AI assistance into natural workflow
2. **Capability Expansion:** Enable Phase 3 autonomous agents with direct engine access
3. **Distribution:** Make tool accessible to wider UE community
4. **Monetization:** Optional premium features via marketplace
5. **Brand Building:** Establish Adastrea ecosystem of tools

### Alignment with Roadmap

| Adastrea Game Phase | Director Plugin Phase | Timeline | Synergy |
|---------------------|----------------------|----------|---------|
| Phase 4: Gameplay & Polish | Plugin Phase 1-2: Foundation & Core | Q2 2026 | PoC during polish phase |
| Phase 5: Content & Beta | Plugin Phase 3: Planning System | Q2 2026 | Planning aids content creation |
| Phase 6: Release | Plugin Phase 4: Autonomous Agents | Q3 2026 | QA automation for release |

---

## Architecture Overview

### High-Level Design

```
┌─────────────────────────────────────────────────────────────┐
│ Unreal Engine Editor                                        │
│                                                             │
│  ┌────────────────────────────────────────────────────────┐ │
│  │ AdastreaDirector Plugin                                │ │
│  │                                                         │ │
│  │  ┌──────────────┐  ┌──────────────┐  ┌─────────────┐ │ │
│  │  │ UI Module    │  │ Core Module  │  │ Python Bridge│ │ │
│  │  │ (Slate)      │  │ (C++)        │  │ (C++)       │ │ │
│  │  └──────────────┘  └──────────────┘  └─────────────┘ │ │
│  └────────────────────────────────────────────────────────┘ │
│                            │ IPC (Sockets)                   │
└────────────────────────────┼───────────────────────────────┘
                             ▼
         ┌───────────────────────────────────────────┐
         │ Python Subprocess                         │
         │  - ChromaDB (RAG)                         │
         │  - LangChain (Agents)                     │
         │  - Gemini API (LLM)                       │
         └───────────────────────────────────────────┘
```

### Module Organization

```
Plugins/AdastreaDirector/
├── AdastreaDirector.uplugin
├── Resources/
│   ├── Icon128.png
│   └── Icon256.png
├── Source/
│   ├── AdastreaDirector/          # Runtime module
│   │   ├── Private/
│   │   │   ├── AdastreaDirectorModule.cpp
│   │   │   ├── PythonSubprocess.cpp
│   │   │   ├── SocketIPC.cpp
│   │   │   └── DirectorConfig.cpp
│   │   ├── Public/
│   │   │   ├── AdastreaDirectorModule.h
│   │   │   ├── PythonSubprocess.h
│   │   │   ├── SocketIPC.h
│   │   │   └── DirectorConfig.h
│   │   └── AdastreaDirector.Build.cs
│   └── AdastreaDirectorEditor/    # Editor-only module
│       ├── Private/
│       │   ├── AdastreaDirectorEditorModule.cpp
│       │   ├── SDirectorMainPanel.cpp
│       │   ├── SDirectorQueryWidget.cpp
│       │   ├── SDirectorPlanningView.cpp
│       │   ├── SDirectorSettingsDialog.cpp
│       │   └── DirectorCommands.cpp
│       ├── Public/
│       │   ├── AdastreaDirectorEditorModule.h
│       │   ├── SDirectorMainPanel.h
│       │   └── DirectorCommands.h
│       └── AdastreaDirectorEditor.Build.cs
├── Content/
│   ├── UI/
│   │   └── Icons/
│   └── Python/
│       ├── director_backend.py
│       ├── ipc_server.py
│       ├── requirements.txt
│       └── setup.py
└── Python/                        # Bundled Python runtime
    ├── windows/
    │   └── python-3.11.x/
    ├── mac/
    │   └── Python.framework/
    └── linux/
        └── python3.11/
```

---

## Development Phases

### Phase 0: Proof of Concept (6 weeks) - Q2 2026

**Goals:**
- Validate technical feasibility
- Test hybrid architecture
- Prove IPC reliability
- Basic UI prototype

**Deliverables:**
- Minimal working plugin
- Python subprocess manager
- Socket communication
- Simple query interface

**Acceptance Criteria:**
- Plugin loads in UE 5.6
- Can start/stop Python subprocess
- Can send query and receive response
- UI panel displays results

**Timeline:**
- Week 1-2: Plugin boilerplate, subprocess manager
- Week 3-4: IPC implementation, basic UI
- Week 5-6: Integration testing, demo

### Phase 1: Foundation (4-6 weeks) - Q2 2026

**Goals:**
- Production-quality plugin structure
- Robust Python integration
- Complete IPC layer
- Editor UI foundation

**Deliverables:**
- Complete plugin modules
- Python lifecycle management
- IPC protocol implementation
- Dockable UI panel

**Acceptance Criteria:**
- Plugin stable on Win/Mac/Linux
- Python subprocess robust
- IPC handles errors gracefully
- UI integrates with UE Editor

**Timeline:**
- Week 1-2: Module structure, build system
- Week 3-4: Python integration, IPC protocol
- Week 5-6: UI framework, testing

### Phase 2: Core Integration (4-6 weeks) - Q2 2026

**Goals:**
- Integrate Phase 1 (RAG system)
- Documentation Q&A working
- Response formatting
- User experience polish

**Deliverables:**
- RAG query interface
- ChromaDB integration
- Response display with formatting
- Query history

**Acceptance Criteria:**
- Can query Adastrea documentation
- Responses accurate and helpful
- Performance < 2 seconds
- UI smooth and responsive

**Timeline:**
- Week 1-2: RAG backend integration
- Week 3-4: Query UI, response display
- Week 5-6: Polish, performance tuning

### Phase 3: Planning System (4-6 weeks) - Q2 2026

**Goals:**
- Integrate Phase 2 (Planning)
- Task decomposition view
- Code generation preview
- File integration

**Deliverables:**
- Planning interface
- Task breakdown display
- Code generation with preview
- Insert code into files

**Acceptance Criteria:**
- Can decompose development goals
- Task breakdown is clear
- Code generation produces valid output
- Can insert code seamlessly

**Timeline:**
- Week 1-2: Planning backend integration
- Week 3-4: Task decomposition UI
- Week 5-6: Code generation, file integration

### Phase 4: Autonomous Agents (8-10 weeks) - Q2-Q3 2026

**Goals:**
- Phase 3 autonomous agents
- Performance profiling
- Bug detection
- Code quality monitoring

**Deliverables:**
- Autonomous agent framework
- Performance profiling agent
- Bug detection agent
- Code quality agent

**Acceptance Criteria:**
- Agents run automatically
- Reports are actionable
- Minimal false positives
- Configurable triggers

**Timeline:**
- Week 1-3: Agent framework
- Week 4-5: Performance profiling
- Week 6-7: Bug detection
- Week 8-10: Code quality, integration

### Phase 5: Polish & Beta (4 weeks) - Q3 2026

**Goals:**
- Beta testing
- Documentation
- Performance optimization
- Marketplace preparation

**Deliverables:**
- Complete documentation
- Tutorial videos
- Beta feedback incorporated
- Marketplace submission

**Acceptance Criteria:**
- Positive beta feedback
- No critical bugs
- Performance acceptable
- Ready for marketplace

**Timeline:**
- Week 1: Documentation, tutorials
- Week 2-3: Beta testing, feedback
- Week 4: Final polish, submission

---

## Module Structure

### Runtime Module (AdastreaDirector)

**Purpose:** Core functionality available at runtime

**Responsibilities:**
- Python subprocess management
- IPC communication layer
- Configuration management
- Logging and telemetry

**Key Classes:**

```cpp
// Python subprocess manager
class FPythonSubprocess {
public:
    bool Start();
    void Stop();
    bool IsRunning();
    FString GetStatus();
};

// Socket-based IPC
class FSocketIPC {
public:
    bool Connect(const FString& Host, int32 Port);
    void Disconnect();
    FString SendRequest(const FString& RequestJson);
    void SetTimeout(float Seconds);
};

// Configuration management
class FDirectorConfig {
public:
    FString GetAPIKey();
    void SetAPIKey(const FString& Key);
    FString GetModelName();
    bool LoadConfig();
    bool SaveConfig();
};
```

### Editor Module (AdastreaDirectorEditor)

**Purpose:** Editor-only UI and tools

**Responsibilities:**
- Slate UI implementation
- Menu/toolbar integration
- Editor commands
- Asset actions

**Key Classes:**

```cpp
// Main dockable panel
class SDirectorMainPanel : public SDockTab {
    SLATE_BEGIN_ARGS(SDirectorMainPanel) {}
    SLATE_END_ARGS()
    
    void Construct(const FArguments& InArgs);
    
private:
    TSharedPtr<SDirectorQueryWidget> QueryWidget;
    TSharedPtr<SDirectorPlanningView> PlanningView;
};

// Query interface
class SDirectorQueryWidget : public SCompoundWidget {
    void OnQuerySubmitted();
    void DisplayResponse(const FString& Response);
};

// Planning view
class SDirectorPlanningView : public SCompoundWidget {
    void DisplayTaskBreakdown(const TArray<FTask>& Tasks);
    void ShowCodePreview(const FString& Code);
};
```

---

## Implementation Details

### Python Subprocess Management

**Lifecycle:**
1. Start on plugin initialization
2. Keep alive during editor session
3. Stop on plugin shutdown or editor close
4. Auto-restart on crash

**Implementation:**
```cpp
bool FPythonSubprocess::Start() {
    FString PythonExePath = GetPythonExecutablePath();
    FString ScriptPath = GetBackendScriptPath();
    
    // Platform-specific process creation
    #if PLATFORM_WINDOWS
        // Use Windows CreateProcess
    #elif PLATFORM_MAC
        // Use macOS posix_spawn
    #elif PLATFORM_LINUX
        // Use Linux fork/exec
    #endif
    
    // Verify process started
    if (!IsProcessRunning(ProcessHandle)) {
        UE_LOG(LogAdastreaDirector, Error, TEXT("Failed to start Python subprocess"));
        return false;
    }
    
    // Wait for IPC server to be ready
    for (int32 i = 0; i < 30; ++i) {
        FPlatformProcess::Sleep(0.1f);
        if (TestIPCConnection()) {
            UE_LOG(LogAdastreaDirector, Log, TEXT("Python backend ready"));
            return true;
        }
    }
    
    return false;
}
```

### IPC Protocol

**Message Format (JSON):**
```json
{
    "type": "query" | "plan" | "config" | "status",
    "id": "unique-request-id",
    "timestamp": 1699999999,
    "payload": {
        // Request-specific data
    }
}
```

**Query Request:**
```json
{
    "type": "query",
    "id": "query-123",
    "timestamp": 1699999999,
    "payload": {
        "question": "How do I create a new faction?",
        "context": {
            "system": "faction",
            "current_file": "/Game/Blueprints/Factions/BP_MyFaction.uasset"
        }
    }
}
```

**Response:**
```json
{
    "id": "query-123",
    "success": true,
    "timestamp": 1699999999,
    "data": {
        "answer": "To create a new faction...",
        "sources": ["Assets/FactionSetupGuide.md"],
        "confidence": 0.95
    }
}
```

### UI Implementation

**Main Panel Layout:**
```cpp
void SDirectorMainPanel::Construct(const FArguments& InArgs) {
    ChildSlot
    [
        SNew(SVerticalBox)
        
        // Header
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            [
                SNew(STextBlock)
                .Text(FText::FromString("Adastrea Director"))
                .Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 16))
            ]
        ]
        
        // Tab switcher
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            [
                SNew(SButton)
                .Text(FText::FromString("Query"))
                .OnClicked(this, &SDirectorMainPanel::OnShowQuery)
            ]
            + SHorizontalBox::Slot()
            [
                SNew(SButton)
                .Text(FText::FromString("Planning"))
                .OnClicked(this, &SDirectorMainPanel::OnShowPlanning)
            ]
        ]
        
        // Content area
        + SVerticalBox::Slot()
        .FillHeight(1.0f)
        [
            SNew(SWidgetSwitcher)
            .WidgetIndex(this, &SDirectorMainPanel::GetActiveTabIndex)
            
            // Query tab
            + SWidgetSwitcher::Slot()
            [
                SAssignNew(QueryWidget, SDirectorQueryWidget)
            ]
            
            // Planning tab
            + SWidgetSwitcher::Slot()
            [
                SAssignNew(PlanningView, SDirectorPlanningView)
            ]
        ]
    ];
}
```

---

## Testing Strategy

### Unit Tests

**C++ Tests:**
- Module initialization
- Python subprocess lifecycle
- IPC communication
- Configuration management

**Python Tests:**
- Backend server functionality
- Request handling
- Error cases
- Performance benchmarks

### Integration Tests

**Plugin Tests:**
- End-to-end query flow
- Planning system integration
- File operations
- Error recovery

**Platform Tests:**
- Windows 10/11
- macOS 12+
- Linux (Ubuntu 22.04+)

### Performance Tests

**Metrics:**
- Query response time < 2s
- Planning response time < 10s
- Memory usage < 500MB
- CPU usage < 10% idle
- UI responsiveness (60 FPS)

### User Acceptance Tests

**Scenarios:**
1. New user installation
2. First-time query
3. Complex planning task
4. Code generation workflow
5. Error recovery
6. Settings configuration

---

## Deployment Plan

### Distribution Methods

1. **GitHub Releases**
   - Open source distribution
   - Free for community
   - Manual installation

2. **UE Marketplace**
   - Commercial distribution
   - Automatic updates
   - Premium features

3. **Epic Games Launcher**
   - Plugin discovery
   - One-click install
   - Integration with UE

### Installation Process

**Automated Installer:**
1. Download plugin package
2. Extract to project `Plugins/` folder
3. Install Python dependencies
4. Configure API key
5. Restart editor
6. Verify installation

**Manual Installation:**
1. Clone/download from GitHub
2. Copy to `Plugins/AdastreaDirector/`
3. Regenerate project files
4. Build plugin
5. Enable in plugin manager
6. Configure settings

### Update Strategy

**Version Scheme:** MAJOR.MINOR.PATCH (e.g., 1.0.0)

**Update Process:**
1. Check for updates on startup
2. Notify user of new version
3. Download in background
4. Apply on editor restart
5. Migrate settings if needed

---

## Maintenance & Updates

### Support Policy

**Community Edition (GitHub):**
- Best-effort support
- Community-driven fixes
- Regular updates aligned with Adastrea game

**Commercial Edition (Marketplace):**
- Dedicated support
- Priority bug fixes
- Premium features
- Long-term maintenance

### Update Cadence

**Minor Updates:** Monthly
- Bug fixes
- Performance improvements
- Small features

**Major Updates:** Quarterly
- New agent types
- Major features
- Architecture improvements

**Security Updates:** As needed
- Critical vulnerabilities
- Dependency updates
- API changes

### Backward Compatibility

**Guarantees:**
- Settings migrate automatically
- Older projects supported
- Deprecation warnings (6 months)
- Migration guides provided

### Monitoring & Telemetry

**Metrics Collected (Optional):**
- Plugin version
- UE version
- Query counts
- Response times
- Error rates

**Privacy:**
- Opt-in only
- No personal data
- No query content
- Local-only storage option

---

## Conclusion

The Adastrea-Director plugin development represents a strategic evolution from external tool to native integration. With clear phases, manageable risks, and strong alignment with the Adastrea game roadmap, this project is positioned for success.

**Next Steps:**
1. Review and approve this development guide
2. Allocate resources for Phase 0 PoC
3. Begin implementation in Q2 2026
4. Regular progress reviews
5. Adjust timeline based on feedback

**Success Metrics:**
- Plugin loads reliably on all platforms
- Positive user feedback (>80% satisfaction)
- Performance targets met
- Adoption by Adastrea team members
- Community engagement

---

**Document Status:** ✅ Complete  
**Approval Required:** Yes  
**Target Start Date:** Q2 2026  
**Estimated Completion:** Q3 2026

For questions or feedback, please create an issue in the [Adastrea GitHub repository](https://github.com/Mittenzx/Adastrea).
