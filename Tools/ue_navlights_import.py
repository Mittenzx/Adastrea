"""UE editor-Python one-shot: import ALL coloured nav-light FBX as StaticMeshes,
create four emissive M_Nav_* materials (green/red/white/amber), and assign each
part's matching material to slot 0. Run via:
  UnrealEditor.exe <proj>.uproject -ExecutePythonScript=<this file>
Uses unreal.AssetTools + unreal.MaterialEditingLibrary (proven recipe).
This is a SELF-CONTAINED one-shot (boot -> run -> exit); do NOT leave an
interactive editor open while this runs.
"""
import unreal
import os, glob

GEN = r"C:\Users\akuma\Adastrea\Assets\FBX\generated"
DEST_FOLDER = "/Game/Assets/Ships"     # nav-light meshes live here (next to ship meshes)
MAT_FOLDER = "/Game/Materials/"

# suffix (as in <ship>_Nav_Nav<Colour>.fbx) -> (material name, emissive colour)
COLOURS = {
    "Green":  ("M_Nav_Green",  (0.0, 1.0, 0.4)),
    "Red":    ("M_Nav_Red",    (1.0, 0.15, 0.15)),
    "White":  ("M_Nav_White",  (1.0, 1.0, 0.95)),
    "Beacon": ("M_Nav_Beacon", (1.0, 0.55, 0.1)),
}

def log(m):
    unreal.log("[nav-import] " + str(m))

def ensure_folder(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)
        log("made folder " + path)

def build_material(at, mat_dir, mat_name, colour, strength=10.0):
    """Create (or reuse) an unlit emissive material with the given colour."""
    path = mat_dir + mat_name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)
    mat = at.create_asset(mat_name, mat_dir, unreal.Material, unreal.MaterialFactoryNew())
    ED = unreal.MaterialEditingLibrary
    ED.delete_all_material_expressions(mat)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    const = ED.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -400, 0)
    # HDR-scaled so it reads as a bright glow
    const.set_editor_property("constant", unreal.LinearColor(colour[0]*strength, colour[1]*strength, colour[2]*strength, 1.0))
    ED.connect_material_property(const, "RGB", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    ED.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset(path)
    log("built material " + mat_name)
    return mat

def main():
    ensure_folder(DEST_FOLDER)
    ensure_folder(MAT_FOLDER)
    at = unreal.AssetToolsHelpers.get_asset_tools()

    # Build the 4 emissive materials once
    mats = {suf: build_material(at, MAT_FOLDER, name, col) for suf, (name, col) in COLOURS.items()}

    # Import FBX options for mesh-only
    opts = unreal.FbxImportUI()
    opts.set_editor_property("import_mesh", True)
    opts.set_editor_property("import_textures", False)
    opts.set_editor_property("import_materials", False)
    opts.static_mesh_import_data.set_editor_property("import_uniform_scale", 1.0)

    # Glob all <ship>_Nav_Nav<Colour>.fbx
    pattern = os.path.join(GEN, "*_Nav_Nav*.fbx")
    fbxs = glob.glob(pattern)
    log("found %d nav-light FBX" % len(fbxs))
    imported = 0
    assigned = 0
    for fbx in fbxs:
        stem = os.path.basename(fbx)[:-4]                 # e.g. SM_Ship_Fighter_01_Nav_NavGreen
        shipname, colour = stem.split("_Nav_Nav")         # -> ("SM_Ship_Fighter_01", "Green")
        colour = colour.split("_")[0]                      # handle any trailing suffix
        if colour not in COLOURS:
            log("SKIP unknown colour " + colour)
            continue
        dest_name = stem
        task = unreal.AssetImportTask()
        task.filename = fbx
        task.destination_path = DEST_FOLDER
        task.destination_name = dest_name
        task.replace_existing = True
        task.automated = True
        task.save = True
        task.options = opts
        at.import_asset_tasks([task])
        mesh_path = DEST_FOLDER + "/" + dest_name
        if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
            imported += 1
            mesh = unreal.load_asset(mesh_path)
            if mesh is not None:
                mesh.set_material(0, mats[colour])
                unreal.EditorAssetLibrary.save_asset(mesh_path)
                assigned += 1
        else:
            log("IMPORT FAILED " + dest_name)
    log("imported=%d assigned=%d  ALL_DONE" % (imported, assigned))

main()