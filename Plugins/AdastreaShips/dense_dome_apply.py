"""Import the dense star-dome mesh and assign it to the dome actor, preserving
the realistic starfield material (M_Starfield_Real) on it.
"""
import unreal

DEST = "/Game/Meshes/Environment"
MESH_NAME = "SM_StarDome_Dense"
FBX = r"C:\Users\akuma\Adastrea\Assets\FBX\generated\SM_StarDome_Dense.fbx"
DOME_PATH = DEST + "/" + MESH_NAME

# --- Import the FBX as a static mesh ---
if not unreal.EditorAssetLibrary.does_asset_exist(DOME_PATH):
    task = unreal.AssetImportTask()
    task.filename = FBX
    task.destination_path = DEST
    task.destination_name = MESH_NAME
    task.replace_existing = True
    task.automated = True
    task.save = True
    # Static mesh import options
    opts = unreal.FbxImportUI()
    opts.set_editor_property("import_mesh", True)
    opts.set_editor_property("import_textures", False)
    opts.set_editor_property("import_materials", False)
    opts.static_mesh_import_data.set_editor_property("import_translation", unreal.Vector(0,0,0))
    opts.static_mesh_import_data.set_editor_property("import_rotation", unreal.Rotator(0,0,0))
    opts.static_mesh_import_data.set_editor_property("import_uniform_scale", 1.0)
    task.options = opts
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
sm = unreal.EditorAssetLibrary.load_asset(DOME_PATH)
if not sm:
    unreal.log_error("=== mesh import failed ===")
else:
    unreal.log("=== imported dense dome mesh %s ===" % sm.get_name())

# --- Assign to the huge dome actor (scale > 1000) in the level, keep mat ---
mat = unreal.EditorAssetLibrary.load_asset("/Game/Materials/M_Starfield_Real")
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assigned = False
if sm:
    for a in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.StaticMeshActor):
        try:
            scale = a.get_actor_scale3d()
        except Exception:
            continue
        if scale.x > 1000:  # the giant dome
            comp = a.static_mesh_component
            comp.set_static_mesh(sm)
            if mat:
                comp.set_material(0, mat)
            unreal.log("=== Set dense dome mesh on %s (scale %s) ===" % (a.get_actor_label(), scale))
            assigned = True
            break
if not assigned:
    unreal.log_error("=== no huge dome actor found ===")

try:
    from unreal import LevelEditorSubsystem
    LevelEditorSubsystem().save_current_level()
    unreal.log("=== Level saved ===")
except Exception as e:
    unreal.log_error("save: %s" % e)
unreal.log("=== dense_dome_apply.py complete ===")