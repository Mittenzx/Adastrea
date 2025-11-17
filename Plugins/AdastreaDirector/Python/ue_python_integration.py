#!/usr/bin/env python3
"""
Unreal Engine Python API Integration for IPC Server

This module extends the IPC server to handle requests that leverage
the Unreal Engine Python API directly, providing better performance
and more capabilities for UE-specific operations.

Architecture:
    IPC Server ← C++ Plugin → UE Editor
         ↓
    UE Python API (this module)
         ↓
    Direct UE Engine Access

This provides the best of both worlds:
- External Python for RAG/LLM (langchain, chromadb, etc.)
- UE Python API for direct engine operations
"""

import logging
from typing import Dict, Any, Optional

# Try to import UE Python API
try:
    from ue_python_api import (
        UEPythonBridge,
        is_running_in_ue,
        get_bridge
    )
    UE_PYTHON_AVAILABLE = True
except ImportError:
    UE_PYTHON_AVAILABLE = False

logger = logging.getLogger('UEPythonIntegration')


class UEPythonIPCHandler:
    """
    Handler for IPC requests that use Unreal Engine Python API.
    
    This class can be registered with the IPC server to handle
    UE-specific operations using the native Python API.
    """
    
    def __init__(self):
        """Initialize the UE Python IPC handler."""
        self.bridge: Optional[UEPythonBridge] = None
        
        if UE_PYTHON_AVAILABLE and is_running_in_ue():
            try:
                self.bridge = get_bridge()
                logger.info("UE Python bridge initialized for IPC handling")
            except Exception as e:
                logger.error(f"Failed to initialize UE Python bridge: {e}")
        else:
            logger.warning("UE Python API not available - handlers will return errors")
    
    def is_available(self) -> bool:
        """Check if UE Python API is available."""
        return self.bridge is not None
    
    # ========================================================================
    # IPC Request Handlers
    # ========================================================================
    
    def handle_console_command(self, data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Handle console command execution request.
        
        Request format:
            {
                "command": "stat fps"
            }
        
        Response format:
            {
                "status": "success",
                "message": "Command executed"
            }
        """
        if not self.bridge:
            return {
                "status": "error",
                "error": "UE Python API not available"
            }
        
        try:
            command = data.get("command", "")
            if not command:
                return {
                    "status": "error",
                    "error": "Missing 'command' field"
                }
            
            success = self.bridge.execute_console_command(command)
            
            if success:
                return {
                    "status": "success",
                    "message": f"Executed command: {command}"
                }
            else:
                return {
                    "status": "error",
                    "error": f"Failed to execute command: {command}"
                }
                
        except Exception as e:
            logger.error(f"Error in handle_console_command: {e}")
            return {
                "status": "error",
                "error": str(e)
            }
    
    def handle_get_selected_assets(self, data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Handle request to get selected assets.
        
        Response format:
            {
                "status": "success",
                "assets": [
                    {
                        "name": "M_Material",
                        "path": "/Game/Materials/M_Material",
                        "class": "Material"
                    },
                    ...
                ]
            }
        """
        if not self.bridge:
            return {
                "status": "error",
                "error": "UE Python API not available"
            }
        
        try:
            assets = self.bridge.get_selected_assets()
            
            return {
                "status": "success",
                "assets": [
                    {
                        "name": asset.asset_name,
                        "path": asset.asset_path,
                        "class": asset.asset_class
                    }
                    for asset in assets
                ],
                "count": len(assets)
            }
            
        except Exception as e:
            logger.error(f"Error in handle_get_selected_assets: {e}")
            return {
                "status": "error",
                "error": str(e)
            }
    
    def handle_get_selected_actors(self, data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Handle request to get selected actors.
        
        Response format:
            {
                "status": "success",
                "actors": [
                    {
                        "name": "StaticMeshActor_1",
                        "class": "StaticMeshActor",
                        "location": [100.0, 200.0, 50.0],
                        "rotation": [0.0, 0.0, 0.0],
                        "scale": [1.0, 1.0, 1.0]
                    },
                    ...
                ]
            }
        """
        if not self.bridge:
            return {
                "status": "error",
                "error": "UE Python API not available"
            }
        
        try:
            actors = self.bridge.get_selected_actors()
            
            return {
                "status": "success",
                "actors": [
                    {
                        "name": actor.actor_name,
                        "class": actor.actor_class,
                        "location": list(actor.location),
                        "rotation": list(actor.rotation),
                        "scale": list(actor.scale)
                    }
                    for actor in actors
                ],
                "count": len(actors)
            }
            
        except Exception as e:
            logger.error(f"Error in handle_get_selected_actors: {e}")
            return {
                "status": "error",
                "error": str(e)
            }
    
    def handle_find_assets(self, data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Handle request to find assets by class.
        
        Request format:
            {
                "asset_class": "Material",
                "path": "/Game/Materials"  # optional
            }
        
        Response format:
            {
                "status": "success",
                "assets": [...],
                "count": 42
            }
        """
        if not self.bridge:
            return {
                "status": "error",
                "error": "UE Python API not available"
            }
        
        try:
            asset_class = data.get("asset_class", "")
            path = data.get("path", "/Game")
            
            if not asset_class:
                return {
                    "status": "error",
                    "error": "Missing 'asset_class' field"
                }
            
            assets = self.bridge.find_assets_by_class(asset_class, path)
            
            return {
                "status": "success",
                "assets": [
                    {
                        "name": asset.asset_name,
                        "path": asset.asset_path,
                        "class": asset.asset_class
                    }
                    for asset in assets
                ],
                "count": len(assets)
            }
            
        except Exception as e:
            logger.error(f"Error in handle_find_assets: {e}")
            return {
                "status": "error",
                "error": str(e)
            }
    
    def handle_get_all_actors(self, data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Handle request to get all actors of a class.
        
        Request format:
            {
                "actor_class": "StaticMeshActor"
            }
        
        Response format:
            {
                "status": "success",
                "actors": [...],
                "count": 15
            }
        """
        if not self.bridge:
            return {
                "status": "error",
                "error": "UE Python API not available"
            }
        
        try:
            actor_class = data.get("actor_class", "")
            
            if not actor_class:
                return {
                    "status": "error",
                    "error": "Missing 'actor_class' field"
                }
            
            actors = self.bridge.get_all_actors_of_class(actor_class)
            
            return {
                "status": "success",
                "actors": [
                    {
                        "name": actor.actor_name,
                        "class": actor.actor_class,
                        "location": list(actor.location),
                        "rotation": list(actor.rotation),
                        "scale": list(actor.scale)
                    }
                    for actor in actors
                ],
                "count": len(actors)
            }
            
        except Exception as e:
            logger.error(f"Error in handle_get_all_actors: {e}")
            return {
                "status": "error",
                "error": str(e)
            }
    
    def handle_spawn_actor(self, data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Handle request to spawn an actor.
        
        Request format:
            {
                "actor_class": "StaticMeshActor",
                "location": [100.0, 200.0, 50.0],  # optional
                "rotation": [0.0, 0.0, 0.0],        # optional
                "name": "MyActor"                   # optional
            }
        
        Response format:
            {
                "status": "success",
                "actor_name": "MyActor_123"
            }
        """
        if not self.bridge:
            return {
                "status": "error",
                "error": "UE Python API not available"
            }
        
        try:
            actor_class = data.get("actor_class", "")
            location_data = data.get("location", [0.0, 0.0, 0.0])
            rotation_data = data.get("rotation", [0.0, 0.0, 0.0])
            actor_name = data.get("name")
            
            if not actor_class:
                return {
                    "status": "error",
                    "error": "Missing 'actor_class' field"
                }
            
            # Validate location and rotation
            if not isinstance(location_data, list) or len(location_data) != 3:
                return {
                    "status": "error",
                    "error": "Location must be a list of exactly 3 numeric values"
                }
            if not isinstance(rotation_data, list) or len(rotation_data) != 3:
                return {
                    "status": "error",
                    "error": "Rotation must be a list of exactly 3 numeric values"
                }
            
            # Validate that values are numeric
            try:
                location = tuple(float(v) for v in location_data)
                rotation = tuple(float(v) for v in rotation_data)
            except (ValueError, TypeError) as e:
                return {
                    "status": "error",
                    "error": f"Location and rotation values must be numeric: {e}"
                }
            
            actor = self.bridge.spawn_actor(
                actor_class,
                location=location,
                rotation=rotation,
                actor_name=actor_name
            )
            
            if actor:
                try:
                    actor_name_str = actor.get_name()
                except Exception:
                    actor_name_str = str(actor)
                return {
                    "status": "success",
                    "actor_name": actor_name_str,
                    "message": "Actor spawned successfully"
                }
            else:
                return {
                    "status": "error",
                    "error": "Failed to spawn actor"
                }
                
        except Exception as e:
            logger.error(f"Error in handle_spawn_actor: {e}")
            return {
                "status": "error",
                "error": str(e)
            }
    
    def handle_get_level_info(self, data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Handle request to get current level information.
        
        Response format:
            {
                "status": "success",
                "level_name": "MyLevel",
                "project_dir": "C:/Projects/MyProject",
                "engine_version": "5.3.0"
            }
        """
        if not self.bridge:
            return {
                "status": "error",
                "error": "UE Python API not available"
            }
        
        try:
            return {
                "status": "success",
                "level_name": self.bridge.get_current_level_name(),
                "project_dir": self.bridge.get_project_directory(),
                "engine_version": self.bridge.get_engine_version()
            }
            
        except Exception as e:
            logger.error(f"Error in handle_get_level_info: {e}")
            return {
                "status": "error",
                "error": str(e)
            }
    
    def handle_show_notification(self, data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Handle request to show a notification in UE editor.
        
        Request format:
            {
                "message": "Operation complete!",
                "duration": 3.0,      # optional, seconds
                "severity": "Success" # optional: Info, Warning, Error, Success
            }
        
        Response format:
            {
                "status": "success",
                "message": "Notification shown"
            }
        """
        if not self.bridge:
            return {
                "status": "error",
                "error": "UE Python API not available"
            }
        
        try:
            message = data.get("message", "")
            duration = data.get("duration", 3.0)
            severity = data.get("severity", "Info")
            
            if not message:
                return {
                    "status": "error",
                    "error": "Missing 'message' field"
                }
            
            self.bridge.show_notification(message, duration, severity)
            
            return {
                "status": "success",
                "message": "Notification shown"
            }
            
        except Exception as e:
            logger.error(f"Error in handle_show_notification: {e}")
            return {
                "status": "error",
                "error": str(e)
            }


def register_ue_python_handlers(ipc_server):
    """
    Register UE Python API handlers with the IPC server.
    
    This function should be called when initializing the IPC server
    to enable UE Python API functionality.
    
    Args:
        ipc_server: IPCServer instance to register handlers with
        
    Example:
        from ipc_server import IPCServer
        from ue_python_integration import register_ue_python_handlers
        
        server = IPCServer()
        register_ue_python_handlers(server)
        server.start()
    """
    handler = UEPythonIPCHandler()
    
    if handler.is_available():
        # Register all UE Python API handlers
        ipc_server.register_handler('ue_console_command', handler.handle_console_command)
        ipc_server.register_handler('ue_get_selected_assets', handler.handle_get_selected_assets)
        ipc_server.register_handler('ue_get_selected_actors', handler.handle_get_selected_actors)
        ipc_server.register_handler('ue_find_assets', handler.handle_find_assets)
        ipc_server.register_handler('ue_get_all_actors', handler.handle_get_all_actors)
        ipc_server.register_handler('ue_spawn_actor', handler.handle_spawn_actor)
        ipc_server.register_handler('ue_get_level_info', handler.handle_get_level_info)
        ipc_server.register_handler('ue_show_notification', handler.handle_show_notification)
        
        logger.info("Registered 8 UE Python API handlers with IPC server")
    else:
        logger.warning("UE Python API not available - handlers not registered")


# ============================================================================
# Convenience Functions
# ============================================================================

def create_ue_python_handler() -> Optional[UEPythonIPCHandler]:
    """
    Create a UE Python IPC handler if available.
    
    Returns:
        UEPythonIPCHandler instance or None if not available
    """
    try:
        handler = UEPythonIPCHandler()
        if handler.is_available():
            return handler
        return None
    except Exception as e:
        logger.error(f"Failed to create UE Python handler: {e}")
        return None


if __name__ == "__main__":
    # Test the handler
    handler = create_ue_python_handler()
    
    if handler:
        print("✓ UE Python IPC handler created successfully")
        print("  Testing handlers...")
        
        # Test level info
        result = handler.handle_get_level_info({})
        print(f"  Level info: {result}")
        
        # Test selected assets
        result = handler.handle_get_selected_assets({})
        print(f"  Selected assets: {result.get('count', 0)}")
        
        # Test selected actors
        result = handler.handle_get_selected_actors({})
        print(f"  Selected actors: {result.get('count', 0)}")
    else:
        print("✗ UE Python API not available")
        print("  This module must be run inside Unreal Engine's Python environment")
