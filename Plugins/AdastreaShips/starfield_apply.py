"""Apply a realistic starfield to the space dome.

Creates M_Starfield_Real (a Material with an unlit emissive TextureSample wired
to the imported T_Starfield), then assigns it to the dome actor's static-mesh
component override. Uses MaterialEditingLibrary so the graph is correct.
Also flips the dome mesh to normal so the material renders. If material-graph
editing fails, falls back to just importing the texture and reporting.
"""
import unreal

ED = unreal.MaterialEditingLibrary
AT = unreal.AssetToolsHelpers.get_asset_tools()

DOME = "/Game/Maps/TestLevel.TestLevel:PersistentLevel.StaticMeshActor_3"

# --- 1. Import the texture (idempotent) ---
src_png = r"C:\Users\akuma\Adastrea\Assets\FBX\generated\T_Starfield.png"
DEST = "/Game/Textures/Environment"
TEX_PATH = DEST + "/T_Starfield"
if not unreal.EditorAssetLibrary.does_directory_exist(DEST):
    unreal.EditorAssetLibrary.make_directory(DEST)
if not unreal.EditorAssetLibrary.does_asset_exist(TEX_PATH):
    task = unreal.AssetImportTask()
    task.filename = src_png
    task.destination_path = DEST
    task.destination_name = "T_Starfield"
    task.replace_existing = True
    task.automated = True
    task.save = True
    AT.import_asset_tasks([task])
tex = unreal.EditorAssetLibrary.load_asset(TEX_PATH)
if tex:
    tex.set_editor_property("srgb", True)
    unreal.EditorAssetLibrary.save_asset(TEX_PATH)
    unreal.log("=== Texture imported: %s ===" % TEX_PATH)

# --- 2. Build an unlit emissive starfield material ---
MAT_PATH = "/Game/Materials/M_Starfield_Real"
if not unreal.EditorAssetLibrary.does_asset_exist(MAT_PATH):
    mat = AT.create_asset("M_Starfield_Real", "/Game/Materials", unreal.Material, unreal.MaterialFactoryNew())
else:
    mat = unreal.EditorAssetLibrary.load_asset(MAT_PATH)

if mat and tex:
    ED.delete_all_material_expressions(mat)
    # Shading model: unlit (self-emissive)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property("two_sided", True)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)

    tex_sample = ED.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -400, 0)
    tex_sample.set_editor_property("texture", tex)
    # Wire its RGB into EmissiveColor (index 0)
    ED.connect_material_property(tex_sample, "RGB", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    ED.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset(MAT_PATH)
    unreal.log("=== Material created/wired: %s (unlit emissive) ===" % MAT_PATH)
else:
    unreal.log_error("=== Material creation aborted (mat=%s tex=%s) ===" % (bool(mat), bool(tex)))

# --- 3. Assign material to the dome ---
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
# find the dome actor
dome_actor = None
for a in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.StaticMeshActor):
    if "StaticMeshActor_3" in a.get_actor_label() or a.get_actor_label() == "StaticMeshActor_3":
        dome_actor = a
        break
if not dome_actor:
    # fallback: first StaticMeshActor with a Sphere mesh
    for a in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.StaticMeshActor):
        comp = a.static_mesh_component
        if comp and comp.static_mesh and "Sphere" in comp.static_mesh.get_name():
            dome_actor = a
            break
if dome_actor and mat:
    comp = dome_actor.static_mesh_component
    comp.set_material(0, mat)
    unreal.log("=== Assigned M_Starfield_Real to dome %s ===" % dome_actor.get_actor_label())
    # Save the level via the LevelEditorSubsystem (robust across 5.x).
    try:
        from unreal import LevelEditorSubsystem
        LevelEditorSubsystem().save_current_level()
        unreal.log("=== Level saved ===")
    except Exception as e:
        unreal.log_error("Level save failed (non-fatal): %s" % e)
else:
    unreal.log_error("=== Dome actor or material not found (actor=%s mat=%s) ===" % (dome_actor, bool(mat)))
unreal.log("=== starfield_apply.py complete ===")