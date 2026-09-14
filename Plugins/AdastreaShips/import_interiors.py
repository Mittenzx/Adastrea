"""
Surgical interior import: imports the interior agent's NEW split interior FBX
(SM_Int_*_<Zone> + SM_Int_CommandBridge_* + Alien hold) into the AdastreaShips
plugin and builds the M_Int_* solid-color materials per UE_MATERIAL_REFERENCE.

Run at editor startup via -ExecutePythonScript. Does NOT re-touch ship/station meshes.
"""
import os
import unreal

PROJECT_DIR = unreal.Paths.project_dir()
GEN_DIR = os.path.join(PROJECT_DIR, "Assets", "FBX", "generated")

MESH_DEST = "/AdastreaShips/Meshes/Interiors"

# M_Int_* solid-color materials: (material name, R,G,B) per UE_MATERIAL_REFERENCE
INT_MATERIALS = {
    "M_Int_Shell":    (0.42, 0.44, 0.47),
    "M_Int_Lights":   (0.50, 0.75, 1.00),
    "M_Int_Bunks":    (0.80, 0.66, 0.45),
    "M_Int_Mess":     (0.55, 0.38, 0.25),
    "M_Int_Desks":    (0.25, 0.55, 0.55),
    "M_Int_Galley":   (0.75, 0.78, 0.82),
    "M_Int_Vents":    (0.70, 0.25, 0.20),
    "M_Int_Hatch":    (0.18, 0.18, 0.20),
    "M_Int_Deck":     (0.45, 0.45, 0.48),
    "M_Int_Console":  (0.10, 0.32, 0.36),
    "M_Int_Stations": (0.22, 0.55, 0.55),
    "M_Int_Viewport": (0.08, 0.10, 0.14),
}

def ensure_dir(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)

def import_mesh(fbx_name, dest_path, src_override=None, import_real_materials=False):
    """import_real_materials=True is for BlenderKit-sourced kitbash content
    (Tools/blenderkit_import.py embeds real textures via path_mode=COPY,
    embed_textures=True specifically so this can extract them) -- the
    procedural interior meshes deliberately import WITHOUT materials/textures
    because ApplyInteriorMaterials() re-points their slots to the authored
    M_Int_* kit materials by slot name afterward; a sourced asset has no
    M_Int_* slot names to match, so it needs its own real materials imported
    or it silently keeps UE's default grey WorldGridMaterial."""
    src = src_override or os.path.join(GEN_DIR, fbx_name + ".fbx")
    if not os.path.exists(src):
        print(f"  ! missing mesh {fbx_name}.fbx")
        return None
    ensure_dir(dest_path)
    task = unreal.AssetImportTask()
    task.filename = src
    task.destination_path = dest_path
    task.destination_name = fbx_name
    task.replace_existing = True
    task.automated = True
    task.save = True
    options = unreal.FbxImportUI()
    options.import_as_skeletal = False
    options.import_mesh = True
    options.import_animations = False
    options.import_textures = import_real_materials
    options.import_materials = import_real_materials
    options.create_physics_asset = False
    sm_data = unreal.FbxStaticMeshImportData()
    sm_data.set_editor_property("import_translation", unreal.Vector(0, 0, 0))
    sm_data.set_editor_property("import_rotation", unreal.Rotator(0, 0, 0))
    sm_data.set_editor_property("import_uniform_scale", 1.0)
    sm_data.set_editor_property("combine_meshes", True)
    sm_data.set_editor_property("generate_lightmap_u_vs", True)
    sm_data.set_editor_property("auto_generate_collision", True)
    options.static_mesh_import_data = sm_data
    task.options = options
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return os.path.join(dest_path, fbx_name)

def build_int_materials():
    """Create the solid-color M_Int_* materials under /AdastreaShips/Materials/Interiors."""
    mat_dir = "/AdastreaShips/Materials" + "/Interiors"
    ensure_dir(mat_dir)
    ml = unreal.MaterialEditingLibrary
    for name, (r, g, b) in INT_MATERIALS.items():
        path = mat_dir + "/" + name
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            print(f"  {name} exists, skip")
            continue
        factory = unreal.MaterialFactoryNew()
        mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, mat_dir, unreal.Material, factory)
        if not mat:
            print(f"  ! failed {name}")
            continue
        const = ml.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -400, -100)
        const.set_editor_property("constant", unreal.LinearColor(r, g, b, 1.0))
        ml.connect_material_property(const, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
        ml.recompile_material(mat)
        unreal.EditorAssetLibrary.save_asset(path)
        print(f"  built {name}")

def main():
    unreal.log("=== Surgical interior import starting ===")
    interiors = sorted([f[:-4] for f in os.listdir(GEN_DIR)
                        if f.endswith(".fbx") and f.startswith("SM_Int_")])
    print(f"Found {len(interiors)} SM_Int_ FBX")
    for name in interiors:
        import_mesh(name, MESH_DEST)
    # Sourced (BlenderKit) interior props live in a separate kitbash subfolder
    # (see Tools/blenderkit_import.py) -- import those too.
    kitbash_dir = os.path.join(GEN_DIR, "kitbash")
    if os.path.isdir(kitbash_dir):
        kb_interiors = sorted(f for f in os.listdir(kitbash_dir)
                               if f.endswith(".fbx") and f.startswith("SM_Int_"))
        print(f"Found {len(kb_interiors)} sourced SM_Int_ FBX in kitbash/")
        for f in kb_interiors:
            import_mesh(f[:-4], MESH_DEST, src_override=os.path.join(kitbash_dir, f),
                        import_real_materials=True)
    print("  interior meshes done")
    build_int_materials()
    unreal.log("=== Surgical interior import complete ===")

if __name__ == "__main__":
    main()