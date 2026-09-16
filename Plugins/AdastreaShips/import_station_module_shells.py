"""One-shot: import the Station Editor's new shared module shells (Standard/
Large/ConnectorThin/SolarArray) + their textures/materials into the plugin.
Read-only for everything else -- does not touch any module Blueprint (that's
Drydock's side, wiring SpaceStationModule subclasses to reference these).

Run via:  UnrealEditor.exe <proj>.uproject -ExecutePythonScript=<this file>
Leaves the editor open when done (quit_editor() has a known UE 5.8 shutdown
crash risk, see wire_corvette_bridge.py/wire_fighter_cabin.py).
"""
import os
import unreal

GEN = r"C:\Users\akuma\Adastrea\Assets\FBX\generated"
TEX_DIR = GEN + r"\Textures"
MESH_DEST = "/AdastreaShips/Meshes/Station"
TEX_FOLDER = "/Game/Textures"
MAT_FOLDER = "/Game/Materials/"

SHELLS = [
    "SM_StationModule_Shell_Standard",
    "SM_StationModule_Shell_Large",
    "SM_StationModule_Shell_ConnectorThin",
    "SM_StationModule_Shell_SolarArray",
]
TEXSETS = [
    ("M_StationModule_Shell", "T_StationModule_Shell"),
    ("M_StationModule_SolarArray", "T_StationModule_SolarArray"),
    ("M_StationModule_Connector", "T_StationModule_Connector"),
    ("M_StationModule_Utility", "T_StationModule_Utility"),
    ("M_StationModule_Defence", "T_StationModule_Defence"),
]
MAPS = ["D", "N", "R", "M", "AO", "E"]


def ensure_dir(p):
    if not unreal.EditorAssetLibrary.does_directory_exist(p):
        unreal.EditorAssetLibrary.make_directory(p)


def import_mesh(name):
    src = os.path.join(GEN, name + ".fbx")
    if not os.path.exists(src):
        print(f"  ! missing {src}")
        return None
    ensure_dir(MESH_DEST)
    task = unreal.AssetImportTask()
    task.filename = src
    task.destination_path = MESH_DEST
    task.destination_name = name
    task.replace_existing = True
    task.automated = True
    task.save = True
    options = unreal.FbxImportUI()
    options.import_mesh = True
    options.import_textures = False
    options.import_materials = False
    sm_data = unreal.FbxStaticMeshImportData()
    sm_data.set_editor_property("import_uniform_scale", 1.0)
    sm_data.set_editor_property("auto_generate_collision", True)
    options.static_mesh_import_data = sm_data
    task.options = options
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    p = MESH_DEST + "/" + name
    ok = unreal.EditorAssetLibrary.does_asset_exist(p)
    print(f"  mesh {name}: {'OK' if ok else 'FAILED'}")
    return unreal.load_asset(p) if ok else None


def reimport_texture(png, dest_folder, name):
    ensure_dir(dest_folder)
    task = unreal.AssetImportTask()
    task.filename = png
    task.destination_path = dest_folder
    task.destination_name = name
    task.replace_existing = True
    task.automated = True
    task.save = True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    p = dest_folder + "/" + name
    return unreal.load_asset(p) if unreal.EditorAssetLibrary.does_asset_exist(p) else None


def build_material(matname, texset):
    tex_assets = {}
    for mapn in MAPS:
        png = os.path.join(TEX_DIR, f"{texset}_{mapn}.png")
        if os.path.exists(png):
            ap = reimport_texture(png, TEX_FOLDER + "/" + texset, f"{texset}_{mapn}")
            if ap:
                tex_assets[mapn] = ap
    at = unreal.AssetToolsHelpers.get_asset_tools()
    mat = unreal.load_asset(MAT_FOLDER + matname)
    if mat is None:
        mat = at.create_asset(matname, MAT_FOLDER, unreal.Material, unreal.MaterialFactoryNew())
    ED = unreal.MaterialEditingLibrary
    ED.delete_all_material_expressions(mat)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    y = 60
    def sample(asset, y_):
        n = ED.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -600, y_)
        n.set_editor_property("texture", asset)
        return n
    def try_c(n, out, prop):
        try:
            ED.connect_material_property(n, out, getattr(unreal.MaterialProperty, prop))
            return True
        except Exception:
            return False
    ok = 0
    if "D" in tex_assets and try_c(sample(tex_assets["D"], y), "RGB", "MP_BASE_COLOR"):
        y += 120; ok += 1
    if "N" in tex_assets and try_c(sample(tex_assets["N"], y), "RGB", "MP_NORMAL"):
        y += 120; ok += 1
    if "R" in tex_assets and try_c(sample(tex_assets["R"], y), "R", "MP_ROUGHNESS"):
        y += 120; ok += 1
    if "M" in tex_assets and try_c(sample(tex_assets["M"], y), "R", "MP_METALLIC"):
        y += 120; ok += 1
    if "E" in tex_assets and try_c(sample(tex_assets["E"], y), "RGB", "MP_EMISSIVE_COLOR"):
        y += 120; ok += 1
    ED.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset(MAT_FOLDER + matname)
    print(f"  material {matname}: {ok} channels from {texset}")


def main():
    print("=== Station module shell import starting ===")
    ok = True
    for name in SHELLS:
        m = import_mesh(name)
        if m is None:
            ok = False
    for matname, texset in TEXSETS:
        build_material(matname, texset)
    print("RESULT_OK" if ok else "RESULT_FAIL")


try:
    main()
except Exception as e:
    import traceback
    unreal.log_error("import_station_module_shells crashed: %s" % e)
    unreal.log_error(traceback.format_exc())
    print("RESULT_FAIL")
