"""
Adastrea Director Python Helper Utilities

Standardized helper functions for asset operations, error handling, and UClass reflection.
All functions return a consistent result format: {"status": "ok"|"error", "message": str, "details": dict}
"""

import unreal
import os
import traceback


def standardized_result(status="ok", message="", **details):
    """
    Create a standardized result dictionary.
    
    Args:
        status: "ok" or "error"
        message: Human-readable message
        **details: Additional details as keyword arguments
    
    Returns:
        dict with status, message, and details
    """
    return {
        "status": status,
        "message": message,
        "details": details if details else {}
    }


def import_texture(file_path, target_folder="/Game/Textures", asset_name=None):
    """
    Import a texture file as a Texture2D asset.
    
    Args:
        file_path: Full path to image file
        target_folder: Target asset folder (e.g., "/Game/Textures")
        asset_name: Optional name for the asset (uses filename if not provided)
    
    Returns:
        Standardized result dict with asset_path in details
    
    Example:
        >>> result = import_texture("C:/temp/image.png", "/Game/Textures", "MyTexture")
        >>> if result["status"] == "ok":
        >>>     print(f"Imported: {result['details']['asset_path']}")
    """
    try:
        if not os.path.exists(file_path):
            return standardized_result("error", f"File not found: {file_path}")
        
        if not asset_name:
            asset_name = os.path.splitext(os.path.basename(file_path))[0]
        
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        import_task = unreal.AssetImportTask()
        import_task.filename = file_path
        import_task.destination_path = target_folder
        import_task.destination_name = asset_name
        import_task.replace_existing = True
        import_task.automated = True
        import_task.save = True
        
        asset_tools.import_asset_tasks([import_task])
        
        if import_task.imported_object_paths:
            asset_path = import_task.imported_object_paths[0]
            return standardized_result(
                "ok",
                f"Successfully imported texture: {asset_path}",
                asset_path=asset_path,
                local_path=file_path
            )
        else:
            return standardized_result(
                "error",
                f"Import task completed but no asset was created for texture '{file_path}'. "
                "Check the Unreal Editor log for detailed import errors. "
                "Possible causes: unsupported file format, invalid asset path, or corrupted file.",
                file_path=file_path,
                destination_path=target_folder,
                asset_name=asset_name
            )
    
    except Exception as e:
        return standardized_result(
            "error",
            str(e),
            traceback=traceback.format_exc()
        )


def import_static_mesh(file_path, target_folder="/Game/Meshes", asset_name=None):
    """
    Import a 3D model file as a StaticMesh asset.
    
    Args:
        file_path: Full path to mesh file (FBX, OBJ, etc.)
        target_folder: Target asset folder (e.g., "/Game/Meshes")
        asset_name: Optional name for the asset (uses filename if not provided)
    
    Returns:
        Standardized result dict with asset_path in details
    """
    try:
        if not os.path.exists(file_path):
            return standardized_result("error", f"File not found: {file_path}")
        
        if not asset_name:
            asset_name = os.path.splitext(os.path.basename(file_path))[0]
        
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        import_task = unreal.AssetImportTask()
        import_task.filename = file_path
        import_task.destination_path = target_folder
        import_task.destination_name = asset_name
        import_task.replace_existing = True
        import_task.automated = True
        import_task.save = True
        
        asset_tools.import_asset_tasks([import_task])
        
        if import_task.imported_object_paths:
            asset_path = import_task.imported_object_paths[0]
            return standardized_result(
                "ok",
                f"Successfully imported static mesh: {asset_path}",
                asset_path=asset_path,
                local_path=file_path
            )
        else:
            return standardized_result(
                "error",
                f"Import task completed but no asset was created for mesh '{file_path}'. "
                "Check the Unreal Editor log for detailed import errors. "
                "Possible causes: unsupported file format, invalid mesh data, or incorrect import settings.",
                file_path=file_path,
                destination_path=target_folder,
                asset_name=asset_name
            )
    
    except Exception as e:
        return standardized_result(
            "error",
            str(e),
            traceback=traceback.format_exc()
        )


def import_audio(file_path, target_folder="/Game/Audio", asset_name=None):
    """
    Import an audio file as a SoundWave asset.
    
    Args:
        file_path: Full path to audio file (WAV, MP3, etc.)
        target_folder: Target asset folder (e.g., "/Game/Audio")
        asset_name: Optional name for the asset (uses filename if not provided)
    
    Returns:
        Standardized result dict with asset_path in details
    """
    try:
        if not os.path.exists(file_path):
            return standardized_result("error", f"File not found: {file_path}")
        
        if not asset_name:
            asset_name = os.path.splitext(os.path.basename(file_path))[0]
        
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        import_task = unreal.AssetImportTask()
        import_task.filename = file_path
        import_task.destination_path = target_folder
        import_task.destination_name = asset_name
        import_task.replace_existing = True
        import_task.automated = True
        import_task.save = True
        
        asset_tools.import_asset_tasks([import_task])
        
        if import_task.imported_object_paths:
            asset_path = import_task.imported_object_paths[0]
            return standardized_result(
                "ok",
                f"Successfully imported audio: {asset_path}",
                asset_path=asset_path,
                local_path=file_path
            )
        else:
            return standardized_result(
                "error",
                f"Import task completed but no asset was created for audio '{file_path}'. "
                "Check the Unreal Editor log for detailed import errors. "
                "Possible causes: unsupported audio format, invalid sample rate, or corrupted file.",
                file_path=file_path,
                destination_path=target_folder,
                asset_name=asset_name
            )
    
    except Exception as e:
        return standardized_result(
            "error",
            str(e),
            traceback=traceback.format_exc()
        )


def reflect_class(class_name):
    """
    Inspect a UClass and return its properties and functions as JSON.
    Helps AI generate correct code by providing type information.
    
    Args:
        class_name: Full UClass name (e.g., "Actor", "StaticMeshComponent")
    
    Returns:
        Standardized result dict with properties and functions in details
    
    Example:
        >>> result = reflect_class("Actor")
        >>> if result["status"] == "ok":
        >>>     for prop in result["details"]["properties"]:
        >>>         print(f"{prop['name']}: {prop['type']}")
    
    Note:
        Unreal Engine Python API has limited reflection capabilities.
        This function provides basic class information where available.
    """
    try:
        # Try to load the class using proper class loading
        uclass = None
        try:
            # Try loading from Engine module
            uclass = unreal.load_class(None, f"/Script/Engine.{class_name}")
        except Exception:
            # Engine module may not contain the requested class; ignore and try other modules
            pass
        
        if not uclass:
            try:
                # Try loading from CoreUObject module
                uclass = unreal.load_class(None, f"/Script/CoreUObject.{class_name}")
            except Exception:
                # CoreUObject may also not contain the class; final failure is handled below
                pass
        
        if not uclass:
            return standardized_result(
                "error",
                f"Class not found: {class_name}. "
                "Try using the full class path like '/Script/Engine.Actor' or check UE documentation.",
                class_name=class_name
            )
        
        # Note: UE Python API has limited reflection capabilities
        # We can verify the class exists but detailed introspection is not available
        # For detailed class information, refer to:
        # - Unreal Engine C++ API documentation
        # - Unreal Engine Python API documentation
        # - Use unreal.Class methods like get_default_object() for some properties
        
        class_path = uclass.get_path_name() if hasattr(uclass, 'get_path_name') else "unknown"
        
        return standardized_result(
            "ok",
            f"Class found: {class_name}",
            class_name=class_name,
            class_path=class_path,
            note=(
                "UE Python API has limited reflection capabilities. "
                "For property and function details, refer to UE C++ API documentation. "
                f"Class path: {class_path}"
            ),
            recommendation="Use Unreal Engine documentation for detailed class information"
        )
    
    except Exception as e:
        return standardized_result(
            "error",
            str(e),
            traceback=traceback.format_exc()
        )


def create_actor(actor_class, location=None, rotation=None, name=None):
    """
    Create an actor in the current level.
    
    Args:
        actor_class: Actor class name or path (e.g., "StaticMeshActor")
        location: Optional (x, y, z) tuple for actor location
        rotation: Optional (pitch, yaw, roll) tuple for actor rotation
        name: Optional name for the actor
    
    Returns:
        Standardized result dict with actor_name and actor_path in details
    """
    try:
        # Get editor actor subsystem
        editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        
        # Load actor class
        actor_class_obj = unreal.load_class(None, actor_class)
        if not actor_class_obj:
            return standardized_result(
                "error",
                f"Actor class not found: '{actor_class}'. "
                "Expected format: '/Script/Engine.StaticMeshActor' or '/Script/Engine.PointLight'. "
                "Check the class path and ensure it exists in the engine.",
                actor_class=actor_class
            )
        
        # Set location and rotation
        loc = unreal.Vector(location[0], location[1], location[2]) if location else unreal.Vector(0, 0, 0)
        rot = unreal.Rotator(rotation[0], rotation[1], rotation[2]) if rotation else unreal.Rotator(0, 0, 0)
        
        # Spawn actor
        actor = editor_actor_subsystem.spawn_actor_from_class(actor_class_obj, loc, rot)
        
        if not actor:
            return standardized_result("error", "Failed to spawn actor")
        
        # Set name if provided
        if name:
            actor.set_actor_label(name)
        
        return standardized_result(
            "ok",
            f"Successfully created actor: {actor.get_actor_label()}",
            actor_name=actor.get_name(),
            actor_label=actor.get_actor_label(),
            actor_class=actor_class,
            location={"x": loc.x, "y": loc.y, "z": loc.z},
            rotation={"pitch": rot.pitch, "yaw": rot.yaw, "roll": rot.roll}
        )
    
    except Exception as e:
        return standardized_result(
            "error",
            str(e),
            traceback=traceback.format_exc()
        )


def get_selected_actors():
    """
    Get list of currently selected actors.
    
    Returns:
        Standardized result dict with actors list in details
    """
    try:
        editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        selected_actors = editor_actor_subsystem.get_selected_level_actors()
        
        actors_info = []
        for actor in selected_actors:
            actors_info.append({
                "name": actor.get_name(),
                "label": actor.get_actor_label(),
                "class": actor.get_class().get_name(),
                "location": {
                    "x": actor.get_actor_location().x,
                    "y": actor.get_actor_location().y,
                    "z": actor.get_actor_location().z
                }
            })
        
        return standardized_result(
            "ok",
            f"Found {len(actors_info)} selected actor(s)",
            count=len(actors_info),
            actors=actors_info
        )
    
    except Exception as e:
        return standardized_result(
            "error",
            str(e),
            traceback=traceback.format_exc()
        )


# Example usage (for testing)
if __name__ == "__main__":
    print("Adastrea Helper Utilities")
    print("=" * 50)
    print("\nAvailable functions:")
    print("- standardized_result(status, message, **details)")
    print("- import_texture(file_path, target_folder, asset_name)")
    print("- import_static_mesh(file_path, target_folder, asset_name)")
    print("- import_audio(file_path, target_folder, asset_name)")
    print("- reflect_class(class_name)")
    print("- create_actor(actor_class, location, rotation, name)")
    print("- get_selected_actors()")
    print("\nAll functions return: {status, message, details}")
