# Unreal MCP Integration Guide

## What is Unreal MCP?

Unreal MCP (Model Context Protocol) enables AI assistants to remotely control and automate Unreal Engine workflows. This allows for:
- Remote Blueprint configuration
- Automated asset creation
- Batch operations
- Editor automation via natural language

---

## Setup Unreal MCP

### Prerequisites
- Unreal Engine 5.5+ (UE 5.6 recommended)
- Python 3.9+ enabled in Unreal
- MCP-compatible AI client (Claude Desktop, Cursor, Windsurf, etc.)

### Installation

1. **Install Unreal MCP Plugin**
   ```bash
   # Clone the Unreal MCP repository
   git clone https://github.com/chongdashu/unreal-mcp.git
   
   # Copy plugin to project
   cp -r unreal-mcp/UnrealMCP "c:/Unreal Projects/Adastrea/Plugins/"
   ```

2. **Enable Python in Unreal**
   - **Edit** → **Plugins**
   - Search: "Python Editor Script Plugin"
   - Check **Enabled**
   - Restart Unreal Editor

3. **Configure MCP Server**
   
   Create/edit your MCP configuration file:
   
   **For Claude Desktop** (`%APPDATA%\Claude\claude_desktop_config.json`):
   ```json
   {
     "mcpServers": {
       "unreal": {
         "command": "npx",
         "args": [
           "-y",
           "@chongdashu/unreal-mcp-server"
         ]
       }
     }
   }
   ```
   
   **For Cursor** (`.cursor/mcp.json` in project root):
   ```json
   {
     "mcpServers": {
       "unreal": {
         "command": "npx",
         "args": [
           "-y",
           "@chongdashu/unreal-mcp-server"
         ]
       }
     }
   }
   ```

4. **Start Unreal Editor**
   - Open Adastrea.uproject
   - Verify Python is enabled (check Output Log for Python initialization)

---

## Using Unreal MCP for PIE Configuration

### Method 1: Via AI Assistant (Recommended)

If your AI assistant has Unreal MCP configured, simply ask:

```
"Execute ConfigurePIE_UnrealMCP.py in Unreal Engine"
```

The AI will:
1. Load the Python script
2. Execute it in Unreal Engine via MCP
3. Report the results

### Method 2: Manual Execution via MCP

1. Ensure Unreal Editor is running with Adastrea project open
2. Use MCP client to send command:
   ```python
   execute_python_file("c:/Unreal Projects/Adastrea/ConfigurePIE_UnrealMCP.py")
   ```

### Method 3: Direct in Unreal (No MCP Required)

1. **Unreal Editor** → **Window** → **Developer Tools** → **Output Log**
2. Change console from **Cmd** to **Python**
3. Copy contents of `ConfigurePIE_UnrealMCP.py`
4. Paste into Python console
5. Press **Enter**

---

## Available Automation Scripts

### 1. `ConfigurePIE_UnrealMCP.py`
**Purpose**: Configure PIE (Play In Editor) setup for spaceship gameplay

**What it does**:
- ✅ Sets BP_TestGameMode's Default Pawn Class
- ✅ Sets BP_TestGameMode's Player Controller Class
- ✅ Sets BP_PlayerController's HUD Widget Class
- ✅ Disables mouse cursor in BP_PlayerController
- ✅ Verifies TestLevel has lighting and Player Start

**What it CAN'T do**:
- ❌ Add components to BP_PlayerShip (manual step required)
- ❌ Modify component hierarchies
- ❌ Edit widget layouts

**Usage**:
```bash
# Via MCP
execute_python_file("c:/Unreal Projects/Adastrea/ConfigurePIE_UnrealMCP.py")

# Or direct in UE Python console
execfile(r"c:\Unreal Projects\Adastrea\ConfigurePIE_UnrealMCP.py")
```

### 2. `AutoConfigurePIE.py` (Legacy)
**Purpose**: Same as above but designed for manual execution

**Usage**:
```bash
# In Unreal Editor
Tools → Execute Python Script → Select AutoConfigurePIE.py
```

---

## Common Unreal MCP Commands

### Execute Python Code
```python
# Execute string
execute_python("print('Hello from Unreal!')")

# Execute file
execute_python_file("/path/to/script.py")
```

### Actor Management
```python
# Create actor
create_actor("StaticMeshActor", location=(0, 0, 100))

# Delete actor
delete_actor(actor_reference)

# Get all actors of type
get_actors_of_class("StaticMeshActor")
```

### Asset Operations
```python
# Load asset
load_asset("/Game/Blueprints/BP_PlayerShip")

# Save asset
save_asset(asset_reference)

# Get asset list
list_assets("/Game/Blueprints/Ships")
```

### Blueprint Operations
```python
# Open Blueprint
open_blueprint("/Game/Blueprints/BP_PlayerShip")

# Compile Blueprint
compile_blueprint(blueprint_reference)

# Get Blueprint properties
get_blueprint_properties(blueprint_reference)
```

### Editor Control
```python
# Focus viewport on actor
focus_viewport(actor_reference)

# Set camera position
set_camera(location=(0, 0, 500), rotation=(0, -45, 0))

# Take screenshot
take_screenshot("MyScreenshot.png")

# Execute console command
execute_console_command("stat fps")
```

---

## Troubleshooting

### "MCP server not responding"
**Solution**:
1. Check Unreal Editor is running
2. Verify Python plugin is enabled
3. Check MCP configuration in your AI client
4. Restart both Unreal Editor and AI client

### "Failed to execute Python"
**Solution**:
1. Check Output Log for Python errors
2. Verify file paths use forward slashes: `/` not `\`
3. Ensure script has valid Python syntax
4. Try executing a simple test: `execute_python("print('test')")`

### "Cannot load asset"
**Solution**:
1. Verify asset path is correct (use Content Browser path)
2. Asset path format: `/Game/Folder/AssetName`
3. Don't include file extension (.uasset)
4. Check asset actually exists in Content Browser

### "Blueprint not compiling"
**Solution**:
1. Open Blueprint manually and check for errors
2. Verify all required properties are set
3. Check parent class is correct
4. Look for missing references in Blueprint

---

## Best Practices

### 1. Always Verify Assets Exist
```python
asset = unreal.load_asset("/Game/Blueprints/BP_MyAsset")
if not asset:
    unreal.log_error("Asset not found!")
    return False
```

### 2. Use Try-Except for Error Handling
```python
try:
    # Your code
    asset = unreal.load_asset("/Game/Blueprints/BP_Test")
except Exception as e:
    unreal.log_error(f"Error: {e}")
    return False
```

### 3. Log Progress
```python
unreal.log("Starting configuration...")
unreal.log("✓ Step 1 complete")
unreal.log_error("✗ Step 2 failed")
```

### 4. Save Assets After Modification
```python
asset = unreal.load_asset("/Game/Blueprints/BP_Test")
# Modify asset...
unreal.EditorAssetLibrary.save_loaded_asset(asset)
```

### 5. Batch Operations Efficiently
```python
# Load all assets first
assets = [unreal.load_asset(path) for path in asset_paths]

# Process in batch
for asset in assets:
    # Modify asset
    pass

# Save all at once
for asset in assets:
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
```

---

## Example Workflows

### Workflow 1: Spawn Test Actors
```python
# Create 10 cubes in a line
for i in range(10):
    location = (i * 200, 0, 100)
    actor = create_actor("StaticMeshActor", location=location)
    set_property(actor, "Mesh", "/Engine/BasicShapes/Cube")
```

### Workflow 2: Batch Configure Blueprints
```python
blueprints = [
    "/Game/Blueprints/BP_Ship1",
    "/Game/Blueprints/BP_Ship2",
    "/Game/Blueprints/BP_Ship3"
]

for bp_path in blueprints:
    bp = unreal.load_asset(bp_path)
    if bp:
        default_obj = unreal.get_default_object(bp)
        default_obj.set_editor_property("MaxSpeed", 3000.0)
        unreal.EditorAssetLibrary.save_loaded_asset(bp)
```

### Workflow 3: Level Setup
```python
# Load level
unreal.EditorLevelLibrary.load_level("/Game/Maps/TestLevel")

# Add lighting if missing
actors = unreal.EditorLevelLibrary.get_all_level_actors()
has_light = any("Light" in a.get_class().get_name() for a in actors)

if not has_light:
    create_actor("DirectionalLight", location=(0, 0, 500))

# Add Player Start if missing
has_start = any("PlayerStart" in a.get_class().get_name() for a in actors)

if not has_start:
    create_actor("PlayerStart", location=(0, 0, 100))

# Save level
unreal.EditorLevelLibrary.save_current_level()
```

---

## Resources

### Official Documentation
- **Unreal MCP GitHub**: https://github.com/chongdashu/unreal-mcp
- **MCP Protocol**: https://modelcontextprotocol.io/
- **Unreal Python API**: https://docs.unrealengine.com/en-US/ProductionPipelines/ScriptingAndAutomation/Python/

### Adastrea Documentation
- **PIE_FIX_CHECKLIST.md** - Setup checklist
- **QUICK_PIE_FIX.md** - Quick troubleshooting
- **VISUAL_BLUEPRINT_SETUP.md** - Visual guide
- **PIE_FIX_SUMMARY.md** - Technical details

### Community
- **MCP Database**: https://mcpdb.org/mcps/unreal-mcp
- **Unreal Engine Forums**: Python scripting section
- **Discord**: Unreal Engine Python community

---

## Security Considerations

⚠️ **Important Security Notes**:

1. **Only run trusted Python scripts** - Scripts have full editor access
2. **Review scripts before execution** - Check what they modify
3. **Backup your project** - Use version control (Git)
4. **Don't expose MCP publicly** - Keep it on localhost only
5. **Validate inputs** - Especially user-provided scripts

---

**Last Updated**: 2025-11-29  
**Unreal MCP Version**: Compatible with UE 5.5+  
**Adastrea Project Version**: 1.0  
**Status**: Production Ready
