# Unreal Engine Python API Integration

## Overview

The Adastrea Director plugin now leverages **Unreal Engine's built-in Python API** for direct engine interaction, providing superior performance and more capabilities compared to the Remote Control API alone.

## Architecture

The plugin uses a **hybrid architecture** that combines the best of both worlds:

```
┌──────────────────────────────────────────────────────────┐
│                 Adastrea Director Plugin                 │
├──────────────────────────────────────────────────────────┤
│                                                          │
│  C++ Plugin Shell                                        │
│       ↓                                                  │
│  IPC Communication ←→ External Python (RAG/LLM)         │
│       ↓                                                  │
│  UE Python API ←→ Direct UE Engine Access               │
│                                                          │
└──────────────────────────────────────────────────────────┘
```

### Two Python Environments

1. **External Python** (IPC Server)
   - Runs outside UE in a separate process
   - Handles RAG system (LangChain, ChromaDB)
   - Processes LLM queries (OpenAI, Gemini)
   - Complex data processing and AI tasks
   
2. **UE Python Environment** (This Integration)
   - Runs inside Unreal Engine's Python interpreter
   - Direct access to UE APIs (`import unreal`)
   - Fast operations (no IPC overhead)
   - Editor automation and asset manipulation

## Why UE Python API?

### Benefits

✅ **Direct Engine Access**: No IPC overhead for UE-specific operations  
✅ **Rich API**: Access to all UE subsystems (actors, assets, blueprints, etc.)  
✅ **Better Performance**: Native API calls vs. HTTP/socket communication  
✅ **Editor Automation**: Powerful scripting for repetitive tasks  
✅ **Real-time Operations**: Immediate feedback and results  
✅ **Official Support**: Maintained by Epic Games

### When to Use Each Approach

| Use Case | Recommended Approach | Why |
|----------|---------------------|-----|
| Asset queries | **UE Python API** | Direct access, no serialization |
| Actor spawning/manipulation | **UE Python API** | Native API, better performance |
| Console commands | **UE Python API** | Direct execution |
| RAG/Documentation queries | **External Python** | Needs LangChain/ChromaDB |
| LLM chat/planning | **External Python** | Needs OpenAI/Gemini APIs |
| Code generation | **External Python** | Complex AI processing |
| Performance profiling | **Both** | UE Python for data, External for analysis |

## Features

### 1. Console Commands

Execute any Unreal Engine console command:

```python
from ue_python_api import UEPythonBridge

bridge = UEPythonBridge()

# Performance stats
bridge.execute_console_command("stat fps")
bridge.execute_console_command("stat unit")
bridge.execute_console_command("stat gpu")

# Resolution
bridge.execute_console_command("r.SetRes 1920x1080w")

# Rendering
bridge.execute_console_command("r.ScreenPercentage 100")
```

### 2. Asset Operations

Query and manipulate assets:

```python
# Get selected assets in Content Browser
assets = bridge.get_selected_assets()
for asset in assets:
    print(f"{asset.asset_name} - {asset.asset_class}")
    print(f"  Path: {asset.asset_path}")

# Find all materials
materials = bridge.find_assets_by_class("Material", "/Game/Materials")
print(f"Found {len(materials)} materials")

# Find all static meshes
meshes = bridge.find_assets_by_class("StaticMesh", "/Game")

# Load an asset
material = bridge.load_asset("/Game/Materials/M_MyMaterial")

# Save an asset
bridge.save_asset("/Game/Materials/M_MyMaterial")
```

### 3. Actor Operations

Work with actors in the level:

```python
# Get selected actors
actors = bridge.get_selected_actors()
for actor in actors:
    print(f"{actor.actor_name}: {actor.location}")

# Get all actors of a specific class
mesh_actors = bridge.get_all_actors_of_class("StaticMeshActor")
lights = bridge.get_all_actors_of_class("PointLight")

# Spawn a new actor
actor = bridge.spawn_actor(
    "StaticMeshActor",
    location=(100.0, 200.0, 50.0),
    rotation=(0.0, 0.0, 90.0),
    actor_name="MySpawnedActor"
)

# Delete an actor
bridge.delete_actor("MyActor_123")
```

### 4. Level Operations

Manage levels:

```python
# Get current level info
level_name = bridge.get_current_level_name()
print(f"Current level: {level_name}")

# Load a level
bridge.load_level("/Game/Maps/TestLevel")

# Save current level
bridge.save_current_level()
```

### 5. Editor Utilities

Useful editor functions:

```python
# Get project info
project_dir = bridge.get_project_directory()
engine_version = bridge.get_engine_version()

# Show notification in editor
bridge.show_notification(
    "Operation complete!",
    duration=3.0,
    severity="Success"  # Info, Warning, Error, Success
)

# Log to Output Log
from ue_python_api import LogLevel
bridge.log_message("Processing...", LogLevel.LOG)
bridge.log_message("Warning: Check this!", LogLevel.WARNING)
bridge.log_message("Error occurred!", LogLevel.ERROR)
```

## Usage

### Running Inside Unreal Engine

The UE Python API must be used from within Unreal Engine's Python environment.

#### Method 1: Python Console (Testing)

1. Open UE Editor
2. Go to: **Window → Developer Tools → Python Console**
3. Run commands:

```python
import sys
sys.path.append("Plugins/AdastreaDirector/Python")

from ue_python_api import UEPythonBridge
bridge = UEPythonBridge()

# Test it
print(bridge.get_engine_version())
print(bridge.get_current_level_name())
```

#### Method 2: Python Script File

1. Create a `.py` file in your project
2. Run from Python Console:

```python
execfile("Content/Python/my_script.py")
```

#### Method 3: Via IPC (Recommended)

The plugin automatically integrates UE Python API with the IPC server:

```python
# External Python (or C++) sends IPC request
{
    "type": "ue_console_command",
    "data": {
        "command": "stat fps"
    }
}

# IPC server routes to UE Python API
# Response sent back via IPC
{
    "status": "success",
    "message": "Executed command: stat fps"
}
```

### Available IPC Request Types

When using the IPC integration, the following request types are available:

| Request Type | Description | Example Data |
|-------------|-------------|--------------|
| `ue_console_command` | Execute console command | `{"command": "stat fps"}` |
| `ue_get_selected_assets` | Get selected assets | `{}` |
| `ue_get_selected_actors` | Get selected actors | `{}` |
| `ue_find_assets` | Find assets by class | `{"asset_class": "Material", "path": "/Game"}` |
| `ue_get_all_actors` | Get all actors of class | `{"actor_class": "StaticMeshActor"}` |
| `ue_spawn_actor` | Spawn new actor | `{"actor_class": "StaticMeshActor", "location": [100, 200, 50]}` |
| `ue_get_level_info` | Get level information | `{}` |
| `ue_show_notification` | Show editor notification | `{"message": "Done!", "severity": "Success"}` |

## Examples

### Example 1: Asset Analysis

```python
from ue_python_api import UEPythonBridge

bridge = UEPythonBridge()

# Find all materials that need optimization
materials = bridge.find_assets_by_class("Material", "/Game")
print(f"Analyzing {len(materials)} materials...")

for material in materials:
    # Load and analyze
    mat = bridge.load_asset(material.asset_path)
    if mat:
        print(f"Material: {material.asset_name}")
        # Further analysis here...

bridge.show_notification(
    f"Analysis complete: {len(materials)} materials",
    severity="Success"
)
```

### Example 2: Bulk Actor Operations

```python
from ue_python_api import UEPythonBridge

bridge = UEPythonBridge()

# Get all static mesh actors
actors = bridge.get_all_actors_of_class("StaticMeshActor")
print(f"Found {len(actors)} static mesh actors")

# Process actors in a specific region
for actor in actors:
    x, y, z = actor.location
    
    if x > 1000 and x < 2000:  # Filter by location
        print(f"Processing: {actor.actor_name} at ({x}, {y}, {z})")
        # Modify actor properties here...

bridge.log_message(f"Processed {len(actors)} actors", LogLevel.LOG)
```

### Example 3: Automated Level Setup

```python
from ue_python_api import UEPythonBridge

bridge = UEPythonBridge()

# Spawn a grid of lights
spacing = 500.0
grid_size = 5

for i in range(grid_size):
    for j in range(grid_size):
        location = (i * spacing, j * spacing, 200.0)
        
        light = bridge.spawn_actor(
            "PointLight",
            location=location,
            actor_name=f"GridLight_{i}_{j}"
        )
        
        print(f"Spawned light at {location}")

bridge.show_notification(
    f"Spawned {grid_size * grid_size} lights",
    duration=5.0,
    severity="Success"
)
```

## Integration with IPC Server

The UE Python API is integrated with the IPC server, allowing external Python code (and the C++ plugin) to use UE Python capabilities.

### Server Setup

```python
from ipc_server import IPCServer
from ue_python_integration import register_ue_python_handlers

# Create IPC server
server = IPCServer(host='127.0.0.1', port=5555)

# Register UE Python handlers
register_ue_python_handlers(server)

# Start server
server.start()
```

### Client Usage (from External Python)

```python
import socket
import json

# Connect to IPC server
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('127.0.0.1', 5555))

# Send request to execute console command via UE Python API
request = {
    "type": "ue_console_command",
    "data": {"command": "stat fps"}
}

sock.sendall(json.dumps(request).encode('utf-8'))

# Receive response
response = sock.recv(4096).decode('utf-8')
result = json.loads(response)

print(result)  # {"status": "success", "message": "Executed command: stat fps"}

sock.close()
```

## API Reference

### UEPythonBridge Class

Main class for UE Python API operations.

#### Console & Logging

- `execute_console_command(command: str) -> bool`
- `log_message(message: str, level: LogLevel)`

#### Assets

- `get_selected_assets() -> List[UEAssetInfo]`
- `find_assets_by_class(asset_class: str, path: str) -> List[UEAssetInfo]`
- `load_asset(asset_path: str) -> Optional[Any]`
- `save_asset(asset_path: str) -> bool`

#### Actors

- `get_selected_actors() -> List[UEActorInfo]`
- `get_all_actors_of_class(actor_class: str) -> List[UEActorInfo]`
- `spawn_actor(actor_class, location, rotation, actor_name) -> Optional[Any]`
- `delete_actor(actor_name: str) -> bool`

#### Level & World

- `get_current_level_name() -> str`
- `load_level(level_path: str) -> bool`
- `save_current_level() -> bool`

#### Editor Utilities

- `get_project_directory() -> str`
- `get_engine_version() -> str`
- `show_notification(message, duration, severity)`

### Data Classes

#### UEAssetInfo

- `asset_name: str` - Asset name
- `asset_path: str` - Full asset path
- `asset_class: str` - Asset class name
- `metadata: Dict[str, Any]` - Additional metadata

#### UEActorInfo

- `actor_name: str` - Actor name
- `actor_class: str` - Actor class name
- `location: tuple` - (x, y, z) position
- `rotation: tuple` - (roll, pitch, yaw) rotation
- `scale: tuple` - (x, y, z) scale
- `metadata: Dict[str, Any]` - Additional metadata

### Enums

#### LogLevel

- `LOG` - Regular log message
- `DISPLAY` - Display message (highlighted)
- `WARNING` - Warning message (yellow)
- `ERROR` - Error message (red)

## Testing

### Running the Demo

A comprehensive demo is available:

```python
# In UE Python Console
execfile("examples/ue_python_api_demo.py")
```

The demo shows:
- Basic engine information
- Console command execution
- Asset operations
- Actor operations
- Actor spawning
- Logging examples
- Editor notifications

### Unit Tests

Basic unit tests can be run outside UE (with mock `unreal` module):

```bash
cd Plugins/AdastreaDirector/Python
python -m pytest test_ue_python_api.py
```

## Requirements

### Unreal Engine Setup

1. **Enable Python Plugin**
   - Go to: Edit → Plugins
   - Search for "Python Editor Script Plugin"
   - Enable it and restart UE

2. **Python Version**
   - UE 5.0-5.3: Python 3.9
   - UE 5.4+: Python 3.11
   - Check your UE version's Python version

3. **Plugin Installation**
   - Copy `Plugins/AdastreaDirector` to your project's `Plugins` folder
   - Regenerate project files
   - Build and launch

### Python Dependencies

The UE Python environment is separate from your system Python. The `unreal` module is automatically available when running inside UE.

## Troubleshooting

### "Unreal Python API not available"

**Problem**: Script can't import `unreal` module

**Solutions**:
- Make sure you're running inside UE's Python environment, not system Python
- Check that Python Editor Script Plugin is enabled
- Verify the script is being run from UE Python Console or via `execfile()`

### "Failed to create UE Python bridge"

**Problem**: Bridge initialization fails

**Solutions**:
- Check UE Output Log for detailed error messages
- Verify UE version compatibility
- Ensure project is fully loaded before running script

### Performance Issues

**Problem**: Operations are slow

**Solutions**:
- Use UE Python API for UE-specific operations (not IPC)
- Batch operations when possible
- Check if running in PIE (Play in Editor) mode - may be slower

## Best Practices

1. **Use the Right Tool**
   - UE Python API for editor/engine operations
   - External Python for AI/ML operations

2. **Error Handling**
   - Always check return values
   - Use try/except for robust code
   - Log errors to UE Output Log

3. **Performance**
   - Minimize cross-environment calls
   - Cache results when appropriate
   - Use batch operations

4. **Testing**
   - Test in UE Python Console first
   - Use demo scripts as templates
   - Check UE Output Log for errors

## Future Enhancements

Planned improvements:
- [ ] Blueprint interaction API
- [ ] Animation system access
- [ ] Physics simulation control
- [ ] Landscape manipulation
- [ ] More performance profiling helpers
- [ ] Advanced asset management
- [ ] Automated testing utilities

## References

- [Unreal Engine Python API Documentation](https://docs.unrealengine.com/5.0/en-US/PythonAPI/)
- [Python Editor Script Plugin](https://docs.unrealengine.com/5.0/en-US/scripting-the-unreal-editor-using-python/)
- [Adastrea Director Plugin Documentation](README.md)

## Support

For issues or questions:
1. Check UE Output Log for error messages
2. Review this documentation
3. See examples in `examples/ue_python_api_demo.py`
4. Open an issue on the repository

---

**Last Updated**: November 15, 2025  
**Version**: 1.0.0  
**Status**: Production Ready ✅
