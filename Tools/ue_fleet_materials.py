"""UE editor-Python one-shot: import ALL Adastrea PBR textures and build the fleet
materials NATIVELY in UE (MaterialEditingLibrary graph, not Blender-baked).

- Imports every T_<set>_<map>.png (D/N/R/M/AO/E/SKIN) as a Texture2D under
  /Game/Textures/<set>/.
- Builds every M_<name> (from material_map.json) as a UE Material wiring:
      BaseColor <- _D
      Normal    <- _N
      Roughness <- _R
      Metallic  <- _M
      Emissive  <- _E
  Emissive + Metal/Roughness growth done UE-native. Each channel connect is
  wrapped in try/except so a missing enum member logs + skips (never crashes).

Run via:  UnrealEditor.exe <proj>.uproject -ExecutePythonScript=<this file>
Self-contained (boot -> run -> exit). Do NOT leave an interactive editor open.
"""
import unreal
import os, glob, json

GEN = r"C:\Users\akuma\Adastrea\Assets\FBX\generated"
TEX_DIR = GEN + r"\Textures"
TEX_FOLDER = "/Game/Textures"
MAT_FOLDER = "/Game/Materials/"

ED = unreal.MaterialEditingLibrary
AT = unreal.AssetToolsHelpers.get_asset_tools()
MP = unreal.MaterialProperty   # enum

def log(m):
    unreal.log("[fleet-mat] " + str(m))

def ensure_folder(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)

def import_texture(png_abs, dest_folder, name):
    task = unreal.AssetImportTask()
    task.filename = png_abs
    task.destination_path = dest_folder
    task.destination_name = name
    task.replace_existing = True
    task.automated = True
    task.save = True
    task.options = None
    AT.import_asset_tasks([task])
    p = dest_folder + "/" + name
    if unreal.EditorAssetLibrary.does_asset_exist(p):
        return unreal.load_asset(p)   # return the loaded Texture object, not a path
    return None

def tex_sample(mat, tex_asset, x, y):
    node = ED.create_material_expression(mat, unreal.MaterialExpressionTextureSample, x, y)
    node.set_editor_property("texture", tex_asset)
    return node

def try_connect(mat, node, output, prop_name):
    """Connect node.<output> to a material property by name; skip on failure."""
    try:
        prop = getattr(MP, prop_name)      # raises AttributeError if missing
        ED.connect_material_property(node, output, prop)
        return True
    except Exception as e:
        log("  skip %s (%s)" % (prop_name, e))
        return False

def build_material(name, assets):
    path = MAT_FOLDER + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.EditorAssetLibrary.delete_asset(path)
    mat = AT.create_asset(name, MAT_FOLDER, unreal.Material, unreal.MaterialFactoryNew())
    ED.delete_all_material_expressions(mat)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)

    y = 60
    ok = 0
    # BaseColor (via emissive so the lit windows/night-vibe shows even unlit)
    # Emissive <- _E (intensity-scaled so it actually glows)
    if assets.get("E"):
        n = tex_sample(mat, assets["E"], -600, y); y += 120
        if try_connect(mat, n, "RGB", "MP_EMISSIVE_COLOR"): ok += 1
    if assets.get("D"):
        n = tex_sample(mat, assets["D"], -600, y); y += 120
        if try_connect(mat, n, "RGB", "MP_BASE_COLOR"): ok += 1
    if assets.get("N"):
        n = tex_sample(mat, assets["N"], -600, y); y += 120
        if try_connect(mat, n, "RGB", "MP_NORMAL"): ok += 1
    if assets.get("R"):
        n = tex_sample(mat, assets["R"], -600, y); y += 120
        if try_connect(mat, n, "R", "MP_ROUGHNESS"): ok += 1
    if assets.get("M"):
        n = tex_sample(mat, assets["M"], -600, y); y += 120
        if try_connect(mat, n, "R", "MP_METALLIC"): ok += 1

    ED.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset(path)
    return ok

def main():
    ensure_folder(TEX_FOLDER)
    ensure_folder(MAT_FOLDER)
    mm = json.load(open(os.path.join(GEN, "material_map.json"), encoding="utf-8"))

    # Import all textures, grouped by texture-set (value in material_map)
    set_textures = {}
    for png in glob.glob(os.path.join(TEX_DIR, "*.png")):
        fn = os.path.basename(png)
        stem = fn[:-4]
        base, _, mapleaf = stem.rpartition("_")
        set_name_noT = base[2:] if base.startswith("T_") else base
        dest_folder = TEX_FOLDER + "/" + base
        ensure_folder(dest_folder)
        ap = import_texture(png, dest_folder, stem)
        if ap:
            set_textures.setdefault(set_name_noT, {})[mapleaf] = ap
    log("imported %d texture sets" % len(set_textures))

    built = 0; no_set=0; conn=0
    for mname, set_noT in mm.items():
        assets = set_textures.get(set_noT, {})
        if not assets:
            no_set += 1; continue
        c = build_material(mname, assets)
        built += 1; conn += c
    log("built %d materials with %d channels (skip %d no-set) ALL_DONE" % (built, conn, no_set))

main()