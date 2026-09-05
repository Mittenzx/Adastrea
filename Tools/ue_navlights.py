"""UE editor Python one-shot: import ALL ships' nav-light FBX as StaticMeshes,
create ONE emissive M_NavLights material, and assign it to each nav mesh slot 0.
Run via:  UnrealEditor.exe <proj>.uproject -ExecutePythonScript=<this file>
Uses unreal.MaterialEditingLibrary (proven recipe from skill reference).
"""
import unreal
import os, glob

GEN = r"C:\Users\akuma\Adastrea\Assets\FBX\generated"
DEST_FOLDER = "/Game/Assets/Ships"
MAT_FOLDER = "/Game/Materials/"

def log(m):
    unreal.log("[assets-nav] " + str(m))

def ensure_folder(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)
        log("made folder " + path)

def main():
    ensure_folder(DEST_FOLDER)
    ensure_folder(MAT_FOLDER)
    AT = unreal.AssetToolsHelpers.get_asset_tools()

    # Find all *Nav.fbx on disk (the single-part nav-light FBX per ship)
    nav_fbxs = glob.glob(os.path.join(GEN, "*_Nav.fbx"))
    log("found %d nav-light FBX on disk" % len(nav_fbxs))
    if not nav_fbxs:
        unreal.log("[assets-nav] NO NAV FBX FOUND - abort")
        return

    # Build one shared import task + emissive material once
    opts = unreal.FbxImportUI()
    opts.set_editor_property("import_mesh", True)
    opts.set_editor_property("import_textures", False)
    opts.set_editor_property("import_materials", False)
    opts.static_mesh_import_data.set_editor_property("import_uniform_scale", 1.0)

    # Create the shared M_NavLights emissive material
    mat_path = MAT_FOLDER + "M_NavLights"
    if unreal.EditorAssetLibrary.does_asset_exist(mat_path):
        unreal.EditorAssetLibrary.delete_asset(mat_path)
    mat = AT.create_asset("M_NavLights", MAT_FOLDER, unreal.Material, unreal.MaterialFactoryNew())
    ED = unreal.MaterialEditingLibrary
    ED.delete_all_material_expressions(mat)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    const = ED.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -400, 0)
    const.set_editor_property("constant", unreal.LinearColor(6.0, 2.0, 0.4, 1.0))  # HDR warm-bright
    ED.connect_material_property(const, "RGB", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    ED.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset(mat_path)
    log("created M_NavLights emissive material")

    imported_ok = 0
    assigned_ok = 0
    for fbx in nav_fbxs:
        stem = os.path.basename(fbx)[:-4]          # e.g. SM_Ship_Fighter_01_Nav
        dest_name = stem
        # import
        task = unreal.AssetImportTask()
        task.filename = fbx
        task.destination_path = DEST_FOLDER
        task.destination_name = dest_name
        task.replace_existing = True
        task.automated = True
        task.save = True
        task.options = opts
        AT.import_asset_tasks([task])
        mesh_path = DEST_FOLDER + "/" + dest_name
        if unreal.EditorAssetLibrary.does_asset_exist(mesh_path):
            imported_ok += 1
            # assign material slot 0
            mesh = unreal.load_asset(mesh_path)
            if mesh is not None:
                mesh.set_material(0, mat)
                unreal.EditorAssetLibrary.save_asset(mesh_path)
                assigned_ok += 1
            else:
                unreal.log("[assets-nav] could not load " + mesh_path)
        else:
            unreal.log("[assets-nav] import FAILED for " + dest_name)

    log("imported=%d assigned=%d  ALL_DONE" % (imported_ok, assigned_ok))

main()