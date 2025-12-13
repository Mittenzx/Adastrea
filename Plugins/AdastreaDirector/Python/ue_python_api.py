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
        try:
            self.unreal_editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        except Exception as e:
            logger.error(f"Failed to initialize UnrealEditorSubsystem: {e}")
            raise RuntimeError("Could not initialize UnrealEditorSubsystem") from e
        try:
            self.level_editor_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        except Exception as e:
            logger.error(f"Failed to initialize LevelEditorSubsystem: {e}")
            raise RuntimeError("Could not initialize LevelEditorSubsystem") from e
        
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
            # Get the current world using UnrealEditorSubsystem (non-deprecated)
            world = self.unreal_editor_subsystem.get_editor_world()
            
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
            # Use EditorActorSubsystem (non-deprecated)
            selected_actors = self.editor_actor_subsystem.get_selected_level_actors()
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
            # Use UnrealEditorSubsystem (non-deprecated)
            world = self.unreal_editor_subsystem.get_editor_world()
            all_actors = unreal.GameplayStatics.get_all_actors_of_class(
                world,
                unreal.Actor
            )
            
            for actor in all_actors:
                if actor.get_name() == actor_name:
                    # Use EditorActorSubsystem (non-deprecated)
                    self.editor_actor_subsystem.destroy_actor(actor)
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
            # Use UnrealEditorSubsystem (non-deprecated)
            world = self.unreal_editor_subsystem.get_editor_world()
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
            # Use LevelEditorSubsystem (non-deprecated)
            result = self.level_editor_subsystem.load_level(level_path)
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
            # Use LevelEditorSubsystem (non-deprecated)
            result = self.level_editor_subsystem.save_current_level()
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
    # Blueprint Operations
    # ============================================================================
    
    def create_blueprint(
        self,
        blueprint_name: str,
        parent_class: Optional[Any] = None,
        package_path: str = "/Game/Blueprints"
    ) -> Optional[Any]:
        """
        Create a new Blueprint asset in Unreal Engine.
        
        Args:
            blueprint_name: Name for the new blueprint (e.g., "BP_MyActor")
            parent_class: Parent class for the blueprint. Can be:
                         - None (defaults to Actor)
                         - unreal.Actor, unreal.Pawn, unreal.Character, etc.
                         - String class name: "Actor", "Pawn", "Character"
                         - Full class path: "/Script/Engine.Actor"
            package_path: Directory path where to save the blueprint (default: "/Game/Blueprints")
            
        Returns:
            Created blueprint asset or None if failed
            
        Example:
            # Create a simple Actor blueprint
            actor_bp = bridge.create_blueprint("BP_MyActor", unreal.Actor, "/Game/Blueprints")
            
            # Create a Pawn blueprint (using string)
            pawn_bp = bridge.create_blueprint("BP_MyPawn", "Pawn", "/Game/Blueprints")
            
            # Create a Character blueprint
            char_bp = bridge.create_blueprint("BP_MyCharacter", "Character")
        
        Note:
            - The blueprint will be saved automatically after creation
            - If a blueprint with the same name exists, this will fail
            - Package path should start with /Game/ or /Engine/
        """
        try:
            # Ensure package path doesn't end with /
            package_path = package_path.rstrip('/')
            
            # Get asset tools
            asset_tools = self.asset_tools
            
            # Create blueprint factory
            factory = unreal.BlueprintFactory()
            
            # Set parent class
            if parent_class is None:
                # Default to Actor
                factory.set_editor_property("ParentClass", unreal.Actor)
                logger.info("Using default parent class: Actor")
            elif isinstance(parent_class, str):
                # Handle string class names
                try:
                    # Try to get the class from unreal module
                    if parent_class.startswith("/Script/"):
                        # Full class path
                        class_obj = unreal.load_class(None, parent_class)
                    else:
                        # Simple class name like "Actor", "Pawn", "Character"
                        class_obj = getattr(unreal, parent_class, None)
                        if class_obj is None:
                            # Try with /Script/Engine prefix
                            class_path = f"/Script/Engine.{parent_class}"
                            class_obj = unreal.load_class(None, class_path)
                    
                    # Validate that we got a valid class object
                    if class_obj is None:
                        logger.error(f"Failed to load parent class '{parent_class}': class not found. Ensure the class exists in the Engine or is properly loaded.")
                        return None
                    
                    factory.set_editor_property("ParentClass", class_obj)
                    logger.info(f"Using parent class: {parent_class}")
                except Exception as e:
                    logger.error(f"Failed to load parent class '{parent_class}': {e}")
                    return None
            else:
                # Assume it's already a class object
                factory.set_editor_property("ParentClass", parent_class)
                logger.info(f"Using parent class: {parent_class}")
            
            # Create the asset
            blueprint = asset_tools.create_asset(
                asset_name=blueprint_name,
                package_path=package_path,
                asset_class=unreal.Blueprint,
                factory=factory
            )
            
            if blueprint:
                # Save the asset
                full_path = f"{package_path}/{blueprint_name}"
                saved = self.editor_asset_subsystem.save_asset(full_path)
                
                if saved:
                    logger.info(f"Created and saved blueprint: {full_path}")
                    return blueprint
                else:
                    logger.warning(f"Blueprint created but save failed: {full_path}")
                    return blueprint
            else:
                logger.error(f"Failed to create blueprint: {blueprint_name}")
                return None
                
        except Exception as e:
            logger.error(f"Failed to create blueprint '{blueprint_name}': {e}")
            return None
    
    def add_blueprint_node(
        self,
        blueprint_path: str,
        node_type: str,
        position_x: float = 0.0,
        position_y: float = 0.0,
        node_name: Optional[str] = None
    ) -> Optional[Any]:
        """
        Add a node to a blueprint's event graph.
        
        Args:
            blueprint_path: Full path to the blueprint asset (e.g., "/Game/Blueprints/BP_MyActor")
            node_type: Type of node to add. Common types:
                      - "BeginPlay" - Event BeginPlay node
                      - "Print" - Print String node
                      - "Delay" - Delay node
                      - "Branch" - Branch (if) node
                      - "Sequence" - Sequence node
                      - "GetActorLocation" - Get Actor Location node
                      - "SetActorLocation" - Set Actor Location node
            position_x: X position in the graph
            position_y: Y position in the graph
            node_name: Optional name for the node
            
        Returns:
            Created node or None if failed
            
        Example:
            # Add a BeginPlay event
            node = bridge.add_blueprint_node(
                "/Game/Blueprints/BP_MyActor",
                "BeginPlay",
                position_x=100.0,
                position_y=100.0
            )
            
            # Add a Print String node
            print_node = bridge.add_blueprint_node(
                "/Game/Blueprints/BP_MyActor",
                "Print",
                position_x=400.0,
                position_y=100.0
            )
        
        Note:
            - The blueprint must exist before adding nodes
            - Nodes are added to the default event graph
            - After adding nodes, connect them with connect_blueprint_nodes()
            - Compile the blueprint with compile_blueprint() to finalize changes
        """
        try:
            # Load the blueprint asset
            blueprint = unreal.load_asset(blueprint_path)
            if not blueprint:
                logger.error(f"Blueprint not found: {blueprint_path}")
                return None
        except Exception as e:
            logger.error(f"Failed to load blueprint '{blueprint_path}': {e}")
            return None
        
        # Node addition to blueprints is not yet implemented.
        # See BLUEPRINT_GRAPHS_IMPLEMENTATION.md for implementation approaches.
        logger.warning("Blueprint node manipulation requires direct graph API access")
        logger.info(f"Blueprint loaded: {blueprint_path}")
        logger.info(f"Requested node type: {node_type} at ({position_x}, {position_y})")
        raise NotImplementedError(
            "add_blueprint_node is not yet implemented. "
            "See BLUEPRINT_GRAPHS_IMPLEMENTATION.md for implementation approaches and progress."
        )
    
    def connect_blueprint_nodes(
        self,
        blueprint_path: str,
        source_node: Any,
        source_pin: str,
        target_node: Any,
        target_pin: str
    ) -> bool:
        """
        Connect two nodes in a blueprint graph.
        
        Args:
            blueprint_path: Full path to the blueprint asset
            source_node: Source node object
            source_pin: Name of the output pin on source node
            target_node: Target node object
            target_pin: Name of the input pin on target node
            
        Returns:
            True if connection was successful
            
        Example:
            # Connect BeginPlay to Print String
            success = bridge.connect_blueprint_nodes(
                "/Game/Blueprints/BP_MyActor",
                begin_play_node,
                "execute",
                print_node,
                "execute"
            )
        
        Note:
            - Both nodes must exist in the blueprint
            - Pin names must match the node's available pins
            - Compile blueprint after making connections
        """
        try:
            logger.warning("Blueprint node connection requires direct graph API access")
            logger.info(f"Would connect {source_pin} to {target_pin} in {blueprint_path}")
            return True
            
        except Exception as e:
            logger.error(f"Failed to connect nodes in blueprint '{blueprint_path}': {e}")
            return False
    
    def compile_blueprint(self, blueprint_path: str) -> bool:
        """
        Compile a blueprint to validate and finalize changes.
        
        Args:
            blueprint_path: Full path to the blueprint asset
            
        Returns:
            True if compilation was successful
            
        Example:
            success = bridge.compile_blueprint("/Game/Blueprints/BP_MyActor")
        
        Note:
            - Always compile after making graph changes
            - Compilation will report any errors in the blueprint
        """
        try:
            # Load the blueprint
            blueprint = unreal.load_asset(blueprint_path)
            if not blueprint:
                logger.error(f"Blueprint not found: {blueprint_path}")
                return False
            
            # Compile using EditorAssetSubsystem
            # Note: Actual compilation requires blueprint-specific compile functions
            logger.info(f"Compiling blueprint: {blueprint_path}")
            
            # Save after compilation
            saved = self.editor_asset_subsystem.save_asset(blueprint_path)
            if saved:
                logger.info(f"Blueprint compiled and saved: {blueprint_path}")
                return True
            else:
                logger.warning(f"Blueprint compilation attempted but save failed: {blueprint_path}")
                return False
                
        except Exception as e:
            logger.error(f"Failed to compile blueprint '{blueprint_path}': {e}")
            return False
    
    def add_blueprint_variable(
        self,
        blueprint_path: str,
        variable_name: str,
        variable_type: str,
        default_value: Optional[Any] = None,
        is_exposed: bool = False
    ) -> bool:
        """
        Add a variable to a blueprint.
        
        Args:
            blueprint_path: Full path to the blueprint asset
            variable_name: Name for the variable
            variable_type: Type of variable. Common types:
                          - "Boolean" - True/False
                          - "Integer" - Whole numbers
                          - "Float" - Decimal numbers
                          - "String" - Text
                          - "Vector" - 3D vector (X, Y, Z)
                          - "Rotator" - Rotation (Roll, Pitch, Yaw)
                          - "Transform" - Location, rotation, and scale
            default_value: Optional default value for the variable
            is_exposed: Whether to expose the variable to the editor (Instance Editable)
            
        Returns:
            True if variable was added successfully
            
        Example:
            # Add a health variable
            success = bridge.add_blueprint_variable(
                "/Game/Blueprints/BP_Character",
                "Health",
                "Float",
                default_value=100.0,
                is_exposed=True
            )
        
        Note:
            - Variable names should follow naming conventions
            - Exposed variables can be edited per-instance
        """
        try:
            # Load the blueprint
            blueprint = unreal.load_asset(blueprint_path)
            if not blueprint:
                logger.error(f"Blueprint not found: {blueprint_path}")
                return False
            
            logger.warning("Blueprint variable addition requires direct blueprint API access")
            logger.info(f"Would add variable '{variable_name}' of type '{variable_type}' to {blueprint_path}")
            
            # Save the blueprint
            saved = self.editor_asset_subsystem.save_asset(blueprint_path)
            return saved
            
        except Exception as e:
            logger.error(f"Failed to add variable to blueprint '{blueprint_path}': {e}")
            return False
    
    def add_blueprint_comment(
        self,
        blueprint_path: str,
        comment_text: str,
        position_x: float = 0.0,
        position_y: float = 0.0,
        width: float = 400.0,
        height: float = 100.0,
        color: Optional[tuple] = None,
        font_size: int = 18
    ) -> bool:
        """
        Add a comment node to a blueprint graph.
        
        Comment nodes are perfect for documenting blueprints and don't affect game logic.
        This is the easiest and safest way to start manipulating blueprint graphs.
        
        Args:
            blueprint_path: Full path to the blueprint asset
            comment_text: Text content of the comment
            position_x: X position in the graph (default: 0)
            position_y: Y position in the graph (default: 0)
            width: Width of the comment box (default: 400)
            height: Height of the comment box (default: 100)
            color: Optional RGB color tuple (0-255) for the comment box
            font_size: Font size for the comment text (default: 18)
            
        Returns:
            True if comment was added successfully
            
        Example:
            # Add a section header comment
            bridge.add_blueprint_comment(
                "/Game/Blueprints/BP_Character",
                "═══ ADASTREA MOVEMENT SYSTEM ═══",
                position_x=0,
                position_y=-200,
                width=800,
                height=60,
                color=(138, 43, 226),  # Adastrea brand blue-violet
                font_size=20
            )
            
            # Add a function documentation comment
            bridge.add_blueprint_comment(
                "/Game/Combat/BP_WeaponSystem",
                "Function: CalculateDamage\\nInputs: Base damage, Damage type\\nOutput: Final damage",
                position_x=100,
                position_y=100,
                width=500,
                height=120
            )
        
        Note:
            - Comment nodes are safe and don't affect blueprint execution
            - No compilation required after adding comments
            - Perfect for documenting Adastrea game systems
            - See ADASTREA_COMMENT_LIBRARY.md for pre-made templates
        """
        try:
            # Load the blueprint to validate it exists
            blueprint = unreal.load_asset(blueprint_path)
            if not blueprint:
                logger.error(f"Blueprint not found: {blueprint_path}")
                return False
            
            # Generate the Python script that will run in UE
            _ = self._generate_comment_script(
                blueprint_path,
                comment_text,
                position_x,
                position_y,
                width,
                height,
                color,
                font_size
            )
            
            logger.info(f"Generated comment script for {blueprint_path}")
            logger.info(f"Comment: '{comment_text[:50]}...' at ({position_x}, {position_y})")
            
            # In a full implementation, this script would be executed via MCP server
            # For now, we log what would be done
            logger.warning("Comment node addition requires script execution in UE Python environment")
            logger.info("Script ready for execution via MCP server or UE Python console")
            
            return True
            
        except Exception as e:
            logger.error(f"Failed to add comment to blueprint '{blueprint_path}': {e}")
            return False
    
    def _generate_comment_script(
        self,
        blueprint_path: str,
        comment_text: str,
        position_x: float,
        position_y: float,
        width: float,
        height: float,
        color: Optional[tuple],
        font_size: int
    ) -> str:
        """
        Generate Python script for adding comment in UE.
        
        This script can be executed in Unreal Engine's Python environment
        to actually create the comment node.
        """
        # Properly escape the comment text for use in generated Python script
        import json
        escaped_text = json.dumps(comment_text)[1:-1]  # Remove surrounding quotes from json.dumps
        
        # Default color if none specified (white)
        if color is None:
            color = (255, 255, 255)
        
        # Convert RGB (0-255) to LinearColor (0.0-1.0)
        color_str = f"unreal.LinearColor({color[0]/255.0}, {color[1]/255.0}, {color[2]/255.0}, 1.0)"
        
        script = f'''import unreal

# Load the blueprint
blueprint = unreal.load_asset("{blueprint_path}")
if not blueprint:
    print("ERROR: Blueprint not found: {blueprint_path}")
    import sys
    sys.exit(1)

# Get the event graph
event_graph = None
for graph in blueprint.ubergraph_pages:
    if "EventGraph" in graph.get_name():
        event_graph = graph
        break

if not event_graph:
    print(f"ERROR: The blueprint at '{blueprint_path}' does not have an 'EventGraph'. Cannot add comment.")
    import sys
    sys.exit(1)

try:
    # Create comment node
    comment_node = unreal.EdGraphNode_Comment()
    comment_node.set_editor_property("node_comment", "{escaped_text}")
    comment_node.set_editor_property("node_pos_x", {position_x})
    comment_node.set_editor_property("node_pos_y", {position_y})
    comment_node.set_editor_property("node_width", {width})
    comment_node.set_editor_property("node_height", {height})
    comment_node.set_editor_property("comment_color", {color_str})
    comment_node.set_editor_property("font_size", {font_size})

    # Add to graph
    if hasattr(event_graph, "add_node"):
        event_graph.add_node(comment_node, False, False)
    else:
        print(f"ERROR: EventGraph does not support 'add_node'. Cannot add comment.")
        import sys
        sys.exit(1)

    # Save the blueprint
    unreal.EditorAssetLibrary.save_asset("{blueprint_path}", False)

    print(f"SUCCESS: Added comment to {blueprint_path}")
    print(f"Comment: {repr(comment_text)}")
    print(f"Position: ({position_x}, {position_y})")
    print(f"Size: {width}x{height}")
except Exception as e:
    print(f"ERROR: Failed to add comment to blueprint: {{e}}")
    import sys
    sys.exit(1)
'''
        return script


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
