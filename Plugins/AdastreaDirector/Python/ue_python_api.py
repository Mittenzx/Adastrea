#!/usr/bin/env python3
"""
Unreal Engine Python API Integration Module

This module provides direct access to Unreal Engine's built-in Python API,
enabling seamless interaction with the editor, assets, actors, and other
engine features without going through IPC.

This module is designed to be imported and used WITHIN the Unreal Engine
Python environment, where the 'unreal' module is available.

Key Features:
- Direct UE API access (no IPC overhead)
- Editor scripting automation
- Asset manipulation and queries
- Level and actor operations
- Blueprint interaction
- Performance profiling helpers

Usage:
    # This code runs inside Unreal Engine's Python environment
    import unreal
    from ue_python_api import UEPythonBridge
    
    bridge = UEPythonBridge()
    actors = bridge.get_all_actors_of_class("StaticMeshActor")
    bridge.execute_console_command("stat fps")
"""

import logging
from typing import List, Dict, Any, Optional
from dataclasses import dataclass, field
from enum import Enum

# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger('UEPythonAPI')

# Check if we're running inside Unreal Engine
try:
    import unreal
    UNREAL_AVAILABLE = True
    logger.info("Unreal Python API available")
except ImportError:
    UNREAL_AVAILABLE = False
    logger.warning("Unreal Python API not available - running outside UE environment")
    # Create stub for development/testing outside UE
    class unreal:
        """Stub for development outside Unreal Engine.
        Any access will raise an ImportError with a clear message."""
        def __getattr__(self, name):
            raise ImportError(
                "The 'unreal' Python API is not available outside Unreal Engine. "
                f"Attempted to access attribute '{name}'."
            )
        def __call__(self, *args, **kwargs):
            raise ImportError(
                "The 'unreal' Python API is not available outside Unreal Engine. "
                "Attempted to call the stub 'unreal' object."
            )


@dataclass
class UEAssetInfo:
    """Information about a Unreal Engine asset."""
    asset_name: str
    asset_path: str
    asset_class: str
    asset_size: int = 0
    metadata: Dict[str, Any] = field(default_factory=dict)


@dataclass
class UEActorInfo:
    """Information about a Unreal Engine actor."""
    actor_name: str
    actor_class: str
    location: tuple = (0.0, 0.0, 0.0)
    rotation: tuple = (0.0, 0.0, 0.0)
    scale: tuple = (1.0, 1.0, 1.0)
    metadata: Dict[str, Any] = field(default_factory=dict)


class LogLevel(Enum):
    """Unreal Engine log severity levels."""
    LOG = "Log"
    DISPLAY = "Display"
    WARNING = "Warning"
    ERROR = "Error"


class UEPythonBridge:
    """
    Bridge class for interacting with Unreal Engine via Python API.
    
    This class provides high-level methods for common UE operations:
    - Asset management and queries
    - Actor spawning and manipulation
    - Console command execution
    - Editor scripting utilities
    - Performance profiling helpers
    """
    
    def __init__(self):
        """Initialize the UE Python bridge."""
        if not UNREAL_AVAILABLE:
            raise RuntimeError(
                "Unreal Python API not available. "
                "This module must be run inside Unreal Engine's Python environment."
            )
        
        try:
            self.editor_util = unreal.EditorUtilityLibrary
        except Exception as e:
            logger.error(f"Failed to initialize EditorUtilityLibrary subsystem: {e}")
            raise RuntimeError("Could not initialize EditorUtilityLibrary subsystem") from e
        try:
            self.asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        except Exception as e:
            logger.error(f"Failed to initialize AssetToolsHelpers subsystem: {e}")
            raise RuntimeError("Could not initialize AssetToolsHelpers subsystem") from e
        try:
            self.editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        except Exception as e:
            logger.error(f"Failed to initialize EditorActorSubsystem: {e}")
            raise RuntimeError("Could not initialize EditorActorSubsystem") from e
        try:
            self.editor_asset_subsystem = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
        except Exception as e:
            logger.error(f"Failed to initialize EditorAssetSubsystem: {e}")
            raise RuntimeError("Could not initialize EditorAssetSubsystem") from e
        try:
            self.static_mesh_editor_subsystem = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
        except Exception as e:
            logger.error(f"Failed to initialize StaticMeshEditorSubsystem: {e}")
            raise RuntimeError("Could not initialize StaticMeshEditorSubsystem") from e
        
        logger.info("UE Python Bridge initialized successfully")
    
    # ============================================================================
    # Console and Logging
    # ============================================================================
    
    def execute_console_command(self, command: str) -> bool:
        """
        Execute a console command in Unreal Engine.
        
        Args:
            command: Console command to execute (e.g., "stat fps")
            
        Returns:
            True if command was executed successfully
            
        Example:
            bridge.execute_console_command("stat fps")
            bridge.execute_console_command("r.SetRes 1920x1080w")
        """
        try:
            unreal.SystemLibrary.execute_console_command(
                None,  # world_context_object
                command
            )
            logger.info(f"Executed console command: {command}")
            return True
        except Exception as e:
            logger.error(f"Failed to execute console command '{command}': {e}")
            return False
    
    def log_message(self, message: str, level: LogLevel = LogLevel.LOG):
        """
        Log a message to Unreal Engine's output log.
        
        Args:
            message: Message to log
            level: Log severity level
            
        Example:
            bridge.log_message("Processing complete", LogLevel.DISPLAY)
            bridge.log_message("Warning: Asset not found", LogLevel.WARNING)
        """
        try:
            if level == LogLevel.ERROR:
                unreal.log_error(message)
            elif level == LogLevel.WARNING:
                unreal.log_warning(message)
            else:
                unreal.log(message)
        except Exception as e:
            logger.error(f"Failed to log message: {e}")
    
    # ============================================================================
    # Asset Operations
    # ============================================================================
    
    def get_selected_assets(self) -> List[UEAssetInfo]:
        """
        Get information about currently selected assets in Content Browser.
        
        Returns:
            List of UEAssetInfo objects for selected assets
            
        Example:
            assets = bridge.get_selected_assets()
            for asset in assets:
                print(f"Selected: {asset.asset_name} ({asset.asset_class})")
        """
        try:
            selected_assets = self.editor_util.get_selected_assets()
            asset_infos = []
            
            for asset in selected_assets:
                asset_info = UEAssetInfo(
                    asset_name=asset.get_name(),
                    asset_path=asset.get_path_name(),
                    asset_class=asset.get_class().get_name()
                )
                asset_infos.append(asset_info)
            
            logger.info(f"Retrieved {len(asset_infos)} selected assets")
            return asset_infos
            
        except Exception as e:
            logger.error(f"Failed to get selected assets: {e}")
            return []
    
    def find_assets_by_class(self, asset_class: str, path: str = "/Game") -> List[UEAssetInfo]:
        """
        Find all assets of a specific class in the project.
        
        Args:
            asset_class: Asset class name (e.g., "StaticMesh", "Material")
            path: Root path to search (default: "/Game")
            
        Returns:
            List of UEAssetInfo objects matching the class
            
        Example:
            materials = bridge.find_assets_by_class("Material", "/Game/Materials")
            meshes = bridge.find_assets_by_class("StaticMesh")
        
        Note:
            This function searches recursively through all subdirectories under the
            specified path. For large projects, this can be slow. Consider using a
            more specific path (e.g., "/Game/Materials" instead of "/Game") to
            improve performance when possible.
        """
        try:
            asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
            
            # Create filter
            filter_data = unreal.ARFilter(
                class_names=[asset_class],
                package_paths=[path],
                recursive_paths=True
            )
            
            # Get assets
            assets = asset_registry.get_assets(filter_data)
            asset_infos = []
            
            for asset_data in assets:
                asset_info = UEAssetInfo(
                    asset_name=asset_data.asset_name,
                    asset_path=asset_data.package_name,
                    asset_class=str(asset_data.asset_class)
                )
                asset_infos.append(asset_info)
            
            logger.info(f"Found {len(asset_infos)} assets of class '{asset_class}'")
            return asset_infos
            
        except Exception as e:
            logger.error(f"Failed to find assets by class: {e}")
            return []
    
    def load_asset(self, asset_path: str) -> Optional[Any]:
        """
        Load an asset by its path.
        
        Args:
            asset_path: Full asset path (e.g., "/Game/Materials/M_MyMaterial")
            
        Returns:
            Loaded asset object or None if failed
            
        Example:
            material = bridge.load_asset("/Game/Materials/M_MyMaterial")
            if material:
                print(f"Loaded: {material.get_name()}")
        """
        try:
            asset = unreal.load_asset(asset_path)
            if asset:
                logger.info(f"Loaded asset: {asset_path}")
                return asset
            else:
                logger.warning(f"Asset not found: {asset_path}")
                return None
        except Exception as e:
            logger.error(f"Failed to load asset '{asset_path}': {e}")
            return None
    
    def save_asset(self, asset_path: str) -> bool:
        """
        Save an asset.
        
        Args:
            asset_path: Full asset path to save
            
        Returns:
            True if asset was saved successfully
            
        Example:
            success = bridge.save_asset("/Game/Materials/M_MyMaterial")
        """
        try:
            result = self.editor_asset_subsystem.save_asset(asset_path)
            if result:
                logger.info(f"Saved asset: {asset_path}")
            return result
        except Exception as e:
            logger.error(f"Failed to save asset '{asset_path}': {e}")
            return False
    
    # ============================================================================
    # Actor Operations
    # ============================================================================
    
    def get_all_actors_of_class(self, actor_class: str) -> List[UEActorInfo]:
        """
        Get all actors of a specific class in the current level.
        
        Args:
            actor_class: Actor class name (e.g., "StaticMeshActor", "PointLight") or full class path 
                        (e.g., "/Script/MyGame.MyCustomActor")
            
        Returns:
            List of UEActorInfo objects for matching actors
            
        Example:
            actors = bridge.get_all_actors_of_class("StaticMeshActor")
            actors = bridge.get_all_actors_of_class("/Script/MyGame.MyCustomActor")
            for actor in actors:
                print(f"Actor: {actor.actor_name} at {actor.location}")
        
        Note:
            For custom actor classes (e.g., defined in your game or plugins), provide the full class path.
        """
        try:
            # Get the current world
            world = unreal.EditorLevelLibrary.get_editor_world()
            
            # Determine class path
            if actor_class.startswith("/Script/") or "/" in actor_class or "." in actor_class:
                class_path = actor_class
            else:
                class_path = f"/Script/Engine.{actor_class}"
            
            # Get all actors of the specified class
            actor_class_obj = unreal.load_class(None, class_path)
            actors = unreal.GameplayStatics.get_all_actors_of_class(world, actor_class_obj)
            
            actor_infos = []
            for actor in actors:
                location = actor.get_actor_location()
                rotation = actor.get_actor_rotation()
                scale = actor.get_actor_scale3d()
                
                actor_info = UEActorInfo(
                    actor_name=actor.get_name(),
                    actor_class=actor.get_class().get_name(),
                    location=(location.x, location.y, location.z),
                    rotation=(rotation.roll, rotation.pitch, rotation.yaw),
                    scale=(scale.x, scale.y, scale.z)
                )
                actor_infos.append(actor_info)
            
            logger.info(f"Found {len(actor_infos)} actors of class '{actor_class}'")
            return actor_infos
            
        except Exception as e:
            logger.error(f"Failed to get actors of class '{actor_class}': {e}")
            return []
    
    def get_selected_actors(self) -> List[UEActorInfo]:
        """
        Get information about currently selected actors in the level.
        
        Returns:
            List of UEActorInfo objects for selected actors
            
        Example:
            actors = bridge.get_selected_actors()
            for actor in actors:
                print(f"Selected: {actor.actor_name}")
        """
        try:
            selected_actors = unreal.EditorLevelLibrary.get_selected_level_actors()
            actor_infos = []
            
            for actor in selected_actors:
                location = actor.get_actor_location()
                rotation = actor.get_actor_rotation()
                scale = actor.get_actor_scale3d()
                
                actor_info = UEActorInfo(
                    actor_name=actor.get_name(),
                    actor_class=actor.get_class().get_name(),
                    location=(location.x, location.y, location.z),
                    rotation=(rotation.roll, rotation.pitch, rotation.yaw),
                    scale=(scale.x, scale.y, scale.z)
                )
                actor_infos.append(actor_info)
            
            logger.info(f"Retrieved {len(actor_infos)} selected actors")
            return actor_infos
            
        except Exception as e:
            logger.error(f"Failed to get selected actors: {e}")
            return []
    
    def spawn_actor(
        self,
        actor_class: str,
        location: tuple = (0.0, 0.0, 0.0),
        rotation: tuple = (0.0, 0.0, 0.0),
        actor_name: Optional[str] = None
    ) -> Optional[Any]:
        """
        Spawn a new actor in the current level.
        
        Args:
            actor_class: Class name of actor to spawn (e.g., "StaticMeshActor") or full class path
                        (e.g., "/Script/MyGame.MyCustomActor")
            location: Spawn location (x, y, z)
            rotation: Spawn rotation (roll, pitch, yaw)
            actor_name: Optional name for the actor
            
        Returns:
            Spawned actor object or None if failed
            
        Example:
            actor = bridge.spawn_actor(
                "StaticMeshActor",
                location=(100.0, 200.0, 50.0),
                actor_name="MySpawnedActor"
            )
            actor = bridge.spawn_actor(
                "/Script/MyGame.MyCustomActor",
                location=(100.0, 200.0, 50.0)
            )
        """
        try:
            # Support both short class names and full class paths
            if actor_class.startswith("/Script/") or "/" in actor_class or "." in actor_class:
                class_path = actor_class
            else:
                class_path = f"/Script/Engine.{actor_class}"
            actor_class_obj = unreal.load_class(None, class_path)
            
            spawn_location = unreal.Vector(location[0], location[1], location[2])
            spawn_rotation = unreal.Rotator(rotation[0], rotation[1], rotation[2])
            
            actor = self.editor_actor_subsystem.spawn_actor_from_class(
                actor_class_obj,
                spawn_location,
                spawn_rotation
            )
            
            if actor and actor_name:
                actor.set_actor_label(actor_name)
            
            if actor:
                logger.info(f"Spawned actor: {actor.get_name()} at {location}")
                return actor
            else:
                logger.warning(f"Failed to spawn actor of class '{actor_class}'")
                return None
                
        except Exception as e:
            logger.error(f"Failed to spawn actor: {e}")
            return None
    
    def delete_actor(self, actor_name: str) -> bool:
        """
        Delete an actor from the current level by name.
        
        Args:
            actor_name: Name of the actor to delete
            
        Returns:
            True if actor was deleted successfully
            
        Example:
            success = bridge.delete_actor("MyActor_123")
        """
        try:
            world = unreal.EditorLevelLibrary.get_editor_world()
            all_actors = unreal.GameplayStatics.get_all_actors_of_class(
                world,
                unreal.Actor
            )
            
            for actor in all_actors:
                if actor.get_name() == actor_name:
                    unreal.EditorLevelLibrary.destroy_actor(actor)
                    logger.info(f"Deleted actor: {actor_name}")
                    return True
            
            logger.warning(f"Actor not found: {actor_name}")
            return False
            
        except Exception as e:
            logger.error(f"Failed to delete actor '{actor_name}': {e}")
            return False
    
    # ============================================================================
    # Level and World Operations
    # ============================================================================
    
    def get_current_level_name(self) -> str:
        """
        Get the name of the currently loaded level.
        
        Returns:
            Level name as string
            
        Example:
            level = bridge.get_current_level_name()
            print(f"Current level: {level}")
        """
        try:
            world = unreal.EditorLevelLibrary.get_editor_world()
            level_name = world.get_name()
            logger.info(f"Current level: {level_name}")
            return level_name
        except Exception as e:
            logger.error(f"Failed to get current level name: {e}")
            return ""
    
    def load_level(self, level_path: str) -> bool:
        """
        Load a level by its path.
        
        Args:
            level_path: Path to the level (e.g., "/Game/Maps/MyLevel")
            
        Returns:
            True if level was loaded successfully
            
        Example:
            success = bridge.load_level("/Game/Maps/TestLevel")
        """
        try:
            result = unreal.EditorLevelLibrary.load_level(level_path)
            if result:
                logger.info(f"Loaded level: {level_path}")
            return result
        except Exception as e:
            logger.error(f"Failed to load level '{level_path}': {e}")
            return False
    
    def save_current_level(self) -> bool:
        """
        Save the currently loaded level.
        
        Returns:
            True if level was saved successfully
            
        Example:
            success = bridge.save_current_level()
        """
        try:
            result = unreal.EditorLevelLibrary.save_current_level()
            if result:
                logger.info("Saved current level")
            return result
        except Exception as e:
            logger.error(f"Failed to save current level: {e}")
            return False
    
    # ============================================================================
    # Editor Utilities
    # ============================================================================
    
    def get_project_directory(self) -> str:
        """
        Get the project's root directory path.
        
        Returns:
            Project directory path
            
        Example:
            project_dir = bridge.get_project_directory()
            print(f"Project: {project_dir}")
        """
        try:
            path = unreal.SystemLibrary.get_project_directory()
            logger.info(f"Project directory: {path}")
            return path
        except Exception as e:
            logger.error(f"Failed to get project directory: {e}")
            return ""
    
    def get_engine_version(self) -> str:
        """
        Get the Unreal Engine version.
        
        Returns:
            Engine version string
            
        Example:
            version = bridge.get_engine_version()
            print(f"UE Version: {version}")
        """
        try:
            version = unreal.SystemLibrary.get_engine_version()
            logger.info(f"Engine version: {version}")
            return version
        except Exception as e:
            logger.error(f"Failed to get engine version: {e}")
            return ""
    
    def show_notification(
        self,
        message: str,
        duration: float = 3.0,
        severity: str = "Info"
    ):
        """
        Show a notification in the Unreal Editor.
        
        Args:
            message: Notification message
            duration: How long to show the notification (seconds)
            severity: Notification severity ("Info", "Warning", "Error", "Success")
            
        Example:
            bridge.show_notification("Operation complete!", severity="Success")
        """
        try:
            # Create and show notification
            notification = unreal.NotificationInfo()
            notification.text = unreal.Text(message)
            notification.fade_in_duration = 0.5
            notification.fade_out_duration = 0.5
            notification.expire_duration = duration
            
            # Set severity if supported by Unreal API
            if hasattr(notification, "severity"):
                severity_enum = getattr(unreal.NotificationSeverity, severity.upper(), unreal.NotificationSeverity.INFO)
                notification.severity = severity_enum
            
            unreal.NotificationLibrary.show_notification(notification)
            logger.info(f"Showed notification: {message}")
            
        except Exception as e:
            logger.error(f"Failed to show notification: {e}")


# ============================================================================
# Convenience Functions
# ============================================================================

def is_running_in_ue() -> bool:
    """
    Check if the script is running inside Unreal Engine's Python environment.
    
    Returns:
        True if running in UE, False otherwise
        
    Example:
        if is_running_in_ue():
            # Use UE Python API
            bridge = UEPythonBridge()
        else:
            # Fallback to IPC
            print("Not running in UE environment")
    """
    return UNREAL_AVAILABLE


def get_bridge() -> Optional[UEPythonBridge]:
    """
    Get a UE Python bridge instance if available.
    
    Returns:
        UEPythonBridge instance or None if not in UE environment
        
    Example:
        bridge = get_bridge()
        if bridge:
            bridge.execute_console_command("stat fps")
    """
    if UNREAL_AVAILABLE:
        try:
            return UEPythonBridge()
        except Exception as e:
            logger.error(f"Failed to create UE Python bridge: {e}")
            return None
    return None


# ============================================================================
# Module Initialization
# ============================================================================

if __name__ == "__main__":
    # Basic testing when run directly
    if is_running_in_ue():
        print("Running inside Unreal Engine")
        bridge = UEPythonBridge()
        
        print(f"Engine Version: {bridge.get_engine_version()}")
        print(f"Project Directory: {bridge.get_project_directory()}")
        print(f"Current Level: {bridge.get_current_level_name()}")
        
        # Test getting selected assets
        assets = bridge.get_selected_assets()
        print(f"Selected Assets: {len(assets)}")
        
        # Test getting selected actors
        actors = bridge.get_selected_actors()
        print(f"Selected Actors: {len(actors)}")
    else:
        print("Not running inside Unreal Engine - UE Python API not available")
        print("This module must be imported from within UE's Python environment")
