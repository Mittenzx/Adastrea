# UE Info Collector - Comprehensive Project Information Tool

## Overview

`ue_info_collector.py` is a comprehensive Python script designed to run inside Unreal Engine to gather extensive information about your game project. This information helps AI agents and developers understand the project structure, assets, and configuration.

## What Information Does It Collect?

The script collects information across 10 major categories:

### 1. Project Information
- Engine version
- Project name and paths
- Current level loaded
- Project directories

### 2. Asset Information
- Total asset count
- Assets by type (Blueprints, Materials, Textures, etc.)
- Assets by location/path
- Naming convention analysis (prefixes/suffixes)
- Asset organization patterns

### 3. Blueprint Information
- Total blueprint count
- Blueprints by parent class
- Actor Blueprints
- Component Blueprints
- Interface Blueprints
- Function Libraries
- Widget Blueprints
- Animation Blueprints
- **NEW: Detailed Blueprint Analysis**
  - Variables per blueprint (names, types, counts)
  - Functions per blueprint (names, counts)
  - Components per blueprint (types, counts)
  - Event graphs and function graphs
  - Complexity scoring
  - Most complex blueprints identification
- **NEW: Deep Blueprint Analysis**
  - Per-blueprint variable analysis
  - Per-blueprint function analysis
  - Per-blueprint component analysis
  - Graph structure analysis
  - Complexity metrics

### 4. Level & Actor Information
- Current level details
- All levels in project
- Actor counts and types
- Lighting information
- Volume information

### 5. Material & Texture Information
- Material count
- Material Instances
- Material Functions
- Texture count by type
- Shader complexity indicators

### 6. Plugin Information
- Project plugins
- Plugin count and names

### 7. Performance Information
- Editor statistics
- Performance monitoring setup
- Instructions for runtime metrics

### 8. Source Code Information
- C++ source presence
- Module list
- Source file counts
- Code structure

### 9. Animation Information
- Skeletal Meshes
- Animation Sequences
- Animation Blueprints
- Skeletons

### 10. Audio Information
- Sound Waves
- Sound Cues
- Sound Classes

## How to Use

### Method 1: Python Console (Interactive)

1. Open Unreal Engine Editor
2. Enable **Python Editor Script Plugin** (if not already enabled)
   - Edit → Plugins → Search "Python"
   - Enable "Python Editor Script Plugin"
   - Restart editor if needed
3. Open Python Console
   - Window → Developer Tools → Output Log
   - Switch to "Python" tab, or
   - Window → Developer Tools → Python Console
4. Run the script:

```python
execfile("Plugins/AdastreaDirector/Python/ue_info_collector.py")
```

### Method 2: Programmatic Use

```python
import sys
sys.path.append("Plugins/AdastreaDirector/Python")

import ue_info_collector

# Collect all information
info = ue_info_collector.collect_all_info()

# Print a summary report
ue_info_collector.print_report(info)

# Save to JSON for agents to read
ue_info_collector.save_to_json(info, "project_info.json")

# Save to Markdown for documentation
ue_info_collector.save_to_markdown(info, "project_info.md")
```

### Method 3: Selective Collection

```python
import ue_info_collector

collector = ue_info_collector.UEInfoCollector()

# Collect only specific information
assets = collector.collect_asset_info()
blueprints = collector.collect_blueprint_info()
levels = collector.collect_level_info()

# Print specific information
print(f"Total Assets: {assets['total_assets']}")
print(f"Total Blueprints: {blueprints['total_blueprints']}")
```

## Output Formats

### Console Output
The script prints a formatted report to the console with key statistics from each category.

### JSON Output
Structured data file saved to `ProjectName/Saved/ue_project_info.json` containing all collected information. Perfect for programmatic access by agents or other tools.

### Markdown Output
Human-readable documentation saved to `ProjectName/Saved/ue_project_info.md` with formatted tables and sections.

## Example Output Structure

```json
{
  "project_info": {
    "collection_time": "2025-01-08T12:00:00",
    "engine_version": {"full": "5.3.0"},
    "project": {
      "name": "MyGame",
      "current_level": "MainLevel"
    }
  },
  "assets": {
    "total_assets": 1250,
    "by_type": {
      "Blueprint": 145,
      "Material": 78,
      "StaticMesh": 234
    }
  },
  "blueprints": {
    "total_blueprints": 145,
    "actor_blueprints": 89,
    "widget_blueprints": 23
  }
  // ... more categories
}
```

## Use Cases for Agents

This information enables agents to:

### 1. **Understand Project Structure**
- Identify project organization patterns
- Understand asset naming conventions
- Map project architecture

### 2. **Analyze Complexity**
- Assess blueprint complexity
- Identify potential performance bottlenecks
- Estimate project scope

### 3. **Make Informed Decisions**
- Choose appropriate naming for new assets
- Follow existing conventions
- Understand technology stack

### 4. **Detect Issues**
- Find naming inconsistencies
- Identify missing assets
- Detect placeholder content

### 5. **Generate Context-Aware Code**
- Reference existing blueprints and classes
- Use project-specific patterns
- Follow established conventions

### 6. **Performance Analysis**
- Count actors and complexity
- Identify optimization opportunities
- Track asset growth

## Integration with Adastrea Director

This script is designed to work seamlessly with the Adastrea Director system:

### Remote Control Integration
```python
# Via Remote Control API
from remote_control.client import RemoteControlClient

client = RemoteControlClient()
result = client.run_python_script(
    script_path="Plugins/AdastreaDirector/Python/ue_info_collector.py"
)
```

### MCP Server Integration
```python
# Via MCP Server
mcp_server.handle_tool_call("editor_run_python", {
    "code": open("Plugins/AdastreaDirector/Python/ue_info_collector.py").read()
})
```

### Agent Integration
Agents can request this information via the UE Data Collector:
```python
from ue_data_collector import UEDataCollector

collector = UEDataCollector(mcp_server=mcp_server)
project_info = await collector.collect_project_info()
```

## Performance Considerations

- **Initial Collection**: Takes 10-30 seconds depending on project size
- **Memory Usage**: Minimal - only collects metadata, not asset data
- **Asset Registry**: Uses UE's Asset Registry for fast queries
- **Lazy Loading**: Doesn't load assets into memory unless needed

## Limitations

- **Editor Only**: Must be run in Editor mode (not PIE or packaged game)
- **Performance Metrics**: Detailed runtime metrics only available during PIE
- **Plugin Enumeration**: Limited to project plugins (not engine plugins)
- **Asset Loading**: Some blueprint details require loading assets (may be slow)

## Troubleshooting

### "Unreal Python API not available"
**Solution**: Make sure you're running the script inside Unreal Engine's Python environment, not standalone Python.

### "Failed to initialize subsystems"
**Solution**: Ensure you're in the Editor (not PIE mode) and a level is loaded.

### Script runs but returns empty data
**Solution**: Check that your project has assets. Try loading a level first.

### Performance is slow
**Solution**: The script analyzes many assets. For large projects (1000+ assets), expect 30-60 seconds.

## Blueprint Screenshot Capabilities

### Can We Screenshot Blueprint Graphs?

**Direct Python API**: ❌ **NO** - The Unreal Python API does not expose blueprint graph screenshot functionality.

### What We CAN Analyze

✅ **Via Python API:**
- Blueprint variables (names, types, counts)
- Blueprint functions (names, counts)
- Blueprint components (types, counts)
- Blueprint graphs (names, types, counts)
- Parent classes and hierarchy
- Complexity metrics (calculated scores)
- Blueprint metadata

### Screenshot Alternatives

While direct Python screenshots aren't possible, here are the alternatives:

#### 1. **Blueprint Screenshot Tool Plugin** (Recommended)
- **GitHub**: https://github.com/Gradess2019/BlueprintScreenshotTool
- **Features**:
  - Full graph capture regardless of size
  - Toolbar button in Blueprint editor
  - Hotkeys (Ctrl+F7 for screenshot, Ctrl+F8 to open directory)
  - Configurable export settings
  - Supports Blueprints, Materials, and other graph editors

#### 2. **Manual Editor Screenshot**
- Use `HighResShot` console command
- Limited to visible viewport area
- Good for quick captures

#### 3. **Copy Blueprint as Text**
- Copy nodes and paste as text
- Use online tools like BlueprintUE to visualize
- Good for sharing logic, not visual screenshots

#### 4. **Custom C++ Solution**
- Requires C++ plugin development
- High complexity
- Full control over rendering

### Recommended Workflow

For comprehensive blueprint understanding:
1. **Use this Python script** for data analysis (variables, functions, complexity)
2. **Use Blueprint Screenshot Tool plugin** for visual documentation
3. **Combine both** for complete blueprint understanding

Example:
```python
import ue_info_collector

collector = ue_info_collector.UEInfoCollector()

# Get detailed analysis
analysis = collector.analyze_blueprint_detailed("/Game/Blueprints/BP_MyActor")

# See what we can and cannot do
screenshot_info = collector.get_blueprint_screenshot_info()
print(screenshot_info["recommendation"])
```

## Advanced Usage

### Custom Filtering
```python
collector = ue_info_collector.UEInfoCollector()

# Only collect materials in a specific folder
# (Requires custom implementation)
```

### Scheduled Collection
```python
# Run periodically to track project growth
import time

while True:
    info = ue_info_collector.collect_all_info()
    ue_info_collector.save_to_json(info, f"info_{int(time.time())}.json")
    time.sleep(3600)  # Collect hourly
```

### Diff Analysis
```python
# Compare project state over time
import json

with open("info_before.json") as f:
    before = json.load(f)
    
with open("info_after.json") as f:
    after = json.load(f)

# Compare asset counts
before_assets = before["assets"]["total_assets"]
after_assets = after["assets"]["total_assets"]
print(f"Asset growth: {after_assets - before_assets} new assets")
```

## Contributing

To add new collection categories:

1. Add a new method to `UEInfoCollector` class
2. Call it from `collect_all_info()`
3. Add output formatting to `print_report()` and `save_to_markdown()`
4. Update this README

## License

Copyright (c) 2025 Mittenzx. All Rights Reserved.

Part of the Adastrea Director project.
