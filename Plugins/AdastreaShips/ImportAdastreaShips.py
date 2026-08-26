"""
AdastreaShips — UE 5.8 content import script (run inside the Unreal Editor).

Run once via:  Tools > Execute Python Script... (or `py "Tools/import_adastrea_ships.py"` in the editor's Python console)
This materializes the generated FBX meshes + PNG textures into the plugin's
Content folder as real .uasset assets, and builds the master PBR material +
per-part material instances so the kit is ready to place in the Content Browser.

Source assets must already exist from the generator:
  Assets/FBX/generated/*.fbx   (meshes)
  Assets/FBX/generated/Textures/*.png (T_*_D/_N/_R/_M/_AO/_E)
"""

import os
import unreal

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
PROJECT_DIR = unreal.Paths.project_dir()          # .../Adastrea/
GEN_DIR = os.path.join(PROJECT_DIR, "Assets", "FBX", "generated")
TEX_DIR = os.path.join(GEN_DIR, "Textures")

# Destination content root inside the plugin (relative to /Game)
CONTENT_ROOT = "/AdastreaShips"
MESH_ROOT = CONTENT_ROOT + "/Meshes"
TEX_ROOT = CONTENT_ROOT + "/Textures"
MAT_ROOT = CONTENT_ROOT + "/Materials"


def ensure_dir(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def source_texture_exists(name):
    full = os.path.join(TEX_DIR, name)
    return os.path.exists(full)


def tex_category(prefix):
    """texture-set prefix -> category subfolder (mirrors main()'s routing)."""
    p = prefix.lower()
    if any(k in p for k in ("ship", "freighter", "gunship", "habring", "asteroid")):
        return "Ships"
    if "station" in p:
        return "Station"
    return "Objects"


def texture_asset_path(set_prefix, map_suffix):
    """Return the /AdastreaShips asset path for a texture (e.g. T_HabRing_D)."""
    name = set_prefix + map_suffix
    return TEX_ROOT + "/" + tex_category(set_prefix) + "/" + name


# ---------------------------------------------------------------------------
# Texture import (T_*_D/_N/_R/_M/_AO/_E)
# ---------------------------------------------------------------------------
def import_texture(png_name, dest_path, srgb=True):
    """Import a PNG as a Texture2D into dest_path (folder). Returns asset path."""
    src = os.path.join(TEX_DIR, png_name)
    if not os.path.exists(src):
        print(f"  ! missing texture {png_name}")
        return None
    dest = os.path.join(dest_path, png_name.rsplit(".", 1)[0])
    # Ensure the containing dir exists in the package
    ensure_dir(dest_path)
    task = unreal.AssetImportTask()
    task.filename = src
    task.destination_path = dest_path
    task.destination_name = png_name.rsplit(".", 1)[0]
    task.replace_existing = True
    task.automated = True
    task.save = True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    asset = unreal.load_asset(dest)
    if asset is not None and srgb is not None:
        try:
            asset.set_editor_property("srgb", srgb)
        except Exception as e:
            print(f"  (srgb set skipped for {png_name}: {e})")
    return dest


# ---------------------------------------------------------------------------
# FBX (static mesh) import
# ---------------------------------------------------------------------------
def import_mesh(fbx_name, dest_path):
    """Import one FBX as a StaticMesh into dest_path."""
    src = os.path.join(GEN_DIR, fbx_name + ".fbx")
    if not os.path.exists(src):
        print(f"  ! missing mesh {fbx_name}")
        return None
    ensure_dir(dest_path)
    task = unreal.AssetImportTask()
    task.filename = src
    task.destination_path = dest_path
    task.destination_name = fbx_name
    task.replace_existing = True
    task.automated = True
    task.save = True
    # FBX static-mesh import options (UE 5.x)
    options = unreal.FbxImportUI()
    options.import_as_skeletal = False
    options.import_mesh = True
    options.import_animations = False
    options.import_textures = False
    options.import_materials = False
    options.create_physics_asset = False
    # Use Unreal units; FBX already exported at real-world cm scale
    options.set_editor_property("import_as_skeletal", False)
    # Static Mesh import data
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


# ---------------------------------------------------------------------------
# Master PBR material + instances
# ---------------------------------------------------------------------------
def build_master_material():
    """Create M_Ship_PBR_Master with 6 texture params; returns its path."""
    mat_dir = MAT_ROOT + "/Master"
    ensure_dir(mat_dir)
    mat_path = mat_dir + "/M_Ship_PBR_Master"
    if unreal.EditorAssetLibrary.does_asset_exist(mat_path):
        print("  master material already exists, using it")
        return unreal.load_asset(mat_path)

    factory = unreal.MaterialFactoryNew()
    mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "M_Ship_PBR_Master", mat_dir, unreal.Material, factory)
    if mat is None:
        return None

    ml = unreal.MaterialEditingLibrary
    # texture parameter nodes
    base = ml.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -800, -200)
    base.set_editor_property("parameter_name", "BaseColorMap")
    norm = ml.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -800, -60)
    norm.set_editor_property("parameter_name", "NormalMap")
    rough = ml.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -800, 80)
    rough.set_editor_property("parameter_name", "RoughnessMap")
    metal = ml.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -800, 220)
    metal.set_editor_property("parameter_name", "MetallicMap")
    ao = ml.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -800, 360)
    ao.set_editor_property("parameter_name", "AOMap")
    emiss = ml.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -800, 500)
    emiss.set_editor_property("parameter_name", "EmissiveMap")

    # BaseColor -> main
    ml.connect_material_property(base, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    # Normal -> Normal
    ml.connect_material_property(norm, "RGB", unreal.MaterialProperty.MP_NORMAL)
    # Roughness (R channel, single)
    ml.connect_material_property(rough, "R", unreal.MaterialProperty.MP_ROUGHNESS)
    # Metallic
    ml.connect_material_property(metal, "R", unreal.MaterialProperty.MP_METALLIC)
    # AO
    ml.connect_material_property(ao, "R", unreal.MaterialProperty.MP_AMBIENT_OCCLUSION)
    # Emissive: add emissive color (strength 1, with parameter)
    ml.connect_material_property(emiss, "RGB", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    # default blend/normal settings
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    ml.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset(mat_path)
    print("  created master material", mat_path)
    return mat


def build_material_instances():
    """Create MI_* per part material type using the master."""
    master = build_master_material()
    if master is None:
        print("  ! master material failed; skipping instances")
        return
    # part material -> texture set name
    instances = [
        # (instance name, texture set prefix, emissive intensity)
        ("MI_Ship_Hull",     "T_Ship_Hull_",      1.0),
        ("MI_Freighter",     "T_Freighter_",      1.0),
        ("MI_Gunship",       "T_Gunship_",        1.0),
        ("MI_HabRing",       "T_HabRing_",        1.6),
        ("MI_AsteroidShell", "T_AsteroidShell_",  1.2),
        ("MI_Satellite",     "T_Satellite_",      1.0),
        ("MI_Prop_Crate",    "T_Prop_Crate_",     1.0),
        ("MI_Prop_Tank",     "T_Prop_Tank_",      1.0),
        ("MI_Derelict",      "T_Derelict_",       1.0),
        ("MI_FuelCell",      "T_FuelCell_",       1.0),
    ]
    for name, tex_prefix, emissive_inten in instances:
        inst_dir = MAT_ROOT + "/Instances"
        ensure_dir(inst_dir)
        inst_path = inst_dir + "/" + name
        if unreal.EditorAssetLibrary.does_asset_exist(inst_path):
            print(f"  {name} exists, skip")
            continue
        # Create MI via factory, then parent it to the master
        mi_factory = unreal.MaterialInstanceConstantFactoryNew()
        inst = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            name, inst_dir, unreal.MaterialInstanceConstant, mi_factory)
        if inst is None:
            print(f"  ! failed to create {name}")
            continue
        unreal.MaterialEditingLibrary.set_material_instance_parent(inst, master)

        # assign texture params if source textures exist.
        # tex_prefix ends with "_" (e.g. "T_Ship_Hull_"), map is "_D"/"_N"/...
        set_name = tex_prefix[:-1]               # e.g. "T_Ship_Hull"
        for texmap, param in [("_D", "BaseColorMap"), ("_N", "NormalMap"),
                              ("_R", "RoughnessMap"), ("_M", "MetallicMap"),
                              ("_AO", "AOMap"), ("_E", "EmissiveMap")]:
            if source_texture_exists(set_name + texmap + ".png"):
                tex_path = texture_asset_path(set_name, texmap)
                tex = unreal.load_asset(tex_path)
                if tex is not None:
                    unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(
                        inst, param, tex)
        # emissive scale (if master exposes the param; harmless if unused)
        try:
            unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
                inst, "EmissiveIntensity", emissive_inten)
        except Exception:
            pass
        unreal.EditorAssetLibrary.save_asset(inst_path)
        print(f"  built {name} (emissive {emissive_inten})")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
def main():
    unreal.log("=== AdastreaShips import starting ===")
    if not os.path.isdir(GEN_DIR):
        unreal.log_error(f"Generator output not found at {GEN_DIR}")
        return

    # 1) Textures: import all T_*_?.png, routed into category subfolders
    print("Importing textures...")
    for f in sorted(os.listdir(TEX_DIR)):
        if not (f.endswith(".png") and f.startswith("T_")):
            continue
        # extract set prefix: strip last map-suffix (_D/_N/_R/_M/_AO/_E)
        stem = "_".join(f.split(".")[0].split("_")[:-1])
        import_texture(f, TEX_ROOT + "/" + tex_category(stem), srgb=True)
    print("  done textures")

    # 2) Meshes: import all SM_* FBX
    print("Importing meshes...")
    for f in sorted(os.listdir(GEN_DIR)):
        if f.endswith(".fbx") and f.startswith("SM_"):
            name = f[:-4]
            # route into a subfolder by category
            if name.startswith("SM_Int_") or "_Int_" in name:
                sub = MESH_ROOT + "/Interiors"
            elif "Ship" in name or "Generationship" in name:
                sub = MESH_ROOT + "/Ships"
            elif "Station" in name:
                sub = MESH_ROOT + "/Station"
            else:
                sub = MESH_ROOT + "/Objects"
            ensure_dir(sub)
            import_mesh(name, sub)
    print("  done meshes")

    # 3) Materials
    print("Building materials...")
    build_master_material()
    build_material_instances()
    print("  done materials")

    unreal.log("=== AdastreaShips import complete ===")


if __name__ == "__main__":
    main()
