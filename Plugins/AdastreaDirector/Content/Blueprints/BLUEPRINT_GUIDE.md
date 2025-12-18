# Blueprint Integration Guide

This guide explains how to create and use Blueprints with the Adastrea Director plugin.

## Overview

While the Adastrea Director plugin primarily provides an editor UI panel for AI assistance, you can also integrate its functionality into your Blueprints for automation and custom workflows.

## Creating Custom Integration Blueprints

### Blueprint Template: AI Query Actor

**Purpose**: A Blueprint actor that can query the AI assistant and respond to results.

**To Create:**

1. **Create a new Blueprint Actor**
   - Right-click in Content Browser → Blueprint Class → Actor
   - Name it `BP_AdastreaQuery`

2. **Add Custom Events**
   - `SubmitQuery`: Takes a String input for the query
   - `OnQueryComplete`: Fires when results are ready
   - `DisplayResult`: Shows the result to the user

3. **Implementation Notes**
   - Use the plugin's UI panel directly for most use cases
   - For programmatic access, consider using the Python IPC layer
   - The plugin's backend automatically handles query processing

### Example: Context-Aware Help System

**Use Case**: Display relevant documentation when a player enters a specific area.

**Blueprint Structure:**
```
Event BeginPlay
  → Get Game Instance
  → Cast to Your Game Instance
  → Store Reference

Event ActorBeginOverlap
  → Get Actor Class Name
  → Format Query String: "What is [ActorName]?"
  → [Future: Call AI Assistant]
  → Display Help Widget
```

**Current Implementation:**
Since the plugin focuses on editor-time assistance, runtime integration is a future enhancement. For now:
- Use the plugin during development to generate documentation
- Export results to data tables for runtime access
- Create pre-generated help content based on AI responses

## Best Practices

### 1. Editor-Time Usage (Current)

The plugin excels at **editor-time assistance**:
- Use the UI panel while working in the editor
- Query documentation as you develop
- Generate code snippets and implementation plans
- Get answers to Unreal Engine questions

### 2. Future Runtime Integration

For runtime gameplay integration (planned for future phases):
- Query AI during gameplay for dynamic content
- Generate procedural dialogue or quest descriptions
- Provide in-game help systems
- Adaptive tutorial systems

### 3. Automation Workflows

Use Blueprints with the plugin for:
- **Editor Utility Widgets**: Create custom tools that leverage AI assistance
- **Automated Testing**: Generate test cases and validation scripts
- **Content Validation**: Check assets against project standards
- **Documentation Generation**: Auto-generate documentation from assets

## Blueprint Examples

### Example 1: Editor Utility Widget for AI Queries

**Purpose**: A custom editor tool window that queries the AI.

**Setup:**
1. Create a new Editor Utility Widget
2. Add a text input field for queries
3. Add a button to submit
4. Add a text block for results
5. Use the plugin's backend via Python bridge

**Implementation Path:**
```
[Text Input] → [Submit Button]
                     ↓
           [Call Python Bridge]
                     ↓
          [Display in Text Block]
```

### Example 2: Asset Documentation Helper

**Purpose**: Generate documentation for selected assets.

**Workflow:**
1. Select assets in Content Browser
2. Click custom toolbar button
3. Plugin analyzes asset types and contents
4. Generates comprehensive documentation
5. Exports to Markdown or adds as metadata

### Example 3: Code Review Assistant

**Purpose**: Review Blueprint graphs for common issues.

**Features:**
- Analyzes Blueprint complexity
- Identifies potential performance bottlenecks
- Suggests optimization strategies
- Provides best practice recommendations

## Python Bridge Integration

For advanced users who want programmatic access:

### Accessing the Python Backend

The plugin uses a Python backend accessible via IPC:

```cpp
// C++ Example (for plugin developers)
#include "PythonBridge.h"

void UYourClass::QueryAI(const FString& Query)
{
    FPythonBridge* Bridge = FPythonBridge::Get();
    if (Bridge && Bridge->IsConnected())
    {
        Bridge->SendRequest(Query, [this](const FString& Response)
        {
            // Handle response
            OnAIResponseReceived(Response);
        });
    }
}
```

**Note**: Direct C++ integration requires plugin source code access and recompilation.

### Blueprint Library Functions (Future)

Planned Blueprint functions for future releases:

- `Query AI Assistant`: Submit a query and get a response
- `Update Knowledge Base`: Add documentation at runtime
- `Get System Status`: Check backend health
- `Execute Planning Task`: Run the planning agent
- `Generate Content`: Create procedural content with AI

## Integration Patterns

### Pattern 1: Development Assistant

**Best for**: Editor-time development help
**Access via**: Plugin UI panel
**Complexity**: Low
**Current Status**: ✅ Available

### Pattern 2: Editor Automation

**Best for**: Custom editor tools and utilities
**Access via**: Editor Utility Widgets + Python Bridge
**Complexity**: Medium
**Current Status**: 🔧 Requires custom development

### Pattern 3: Runtime Integration

**Best for**: In-game AI assistance and content generation
**Access via**: Blueprint functions (planned)
**Complexity**: High
**Current Status**: 🔮 Future feature

## Troubleshooting

### Blueprint Node Not Found
- Most plugin functionality is currently UI-focused
- Direct Blueprint nodes are planned for future releases
- Use Editor Utility Widgets for custom tools

### IPC Connection Issues
- Ensure Python backend is running (automatic when plugin loads)
- Check plugin status indicators in the Dashboard tab
- See troubleshooting guide in SETUP_GUIDE.md

### Performance Considerations
- IPC latency is < 1ms for optimal performance
- Backend processes queries asynchronously
- UI updates happen on the game thread

## Next Steps

1. **Explore the UI Panel**: Open Window → Developer Tools → Adastrea Director
2. **Try Example Queries**: Use the Query tab to ask about your project
3. **Ingest Documentation**: Add your project docs in the Ingestion tab
4. **Monitor Status**: Check the Dashboard tab for system health
5. **Read Documentation**: See the plugin README for comprehensive guides

## Additional Resources

- **Plugin Setup**: `SETUP_GUIDE.md`
- **Python API**: `UE_PYTHON_API.md`
- **Main Documentation**: Project Wiki
- **Support**: GitHub Issues

## Contributing

Have ideas for Blueprint integration features? We welcome contributions!
See `CONTRIBUTING.md` in the root project directory.

---

**Note**: This plugin is under active development. Blueprint integration features are being continuously improved. Check the CHANGELOG.md for the latest updates.
