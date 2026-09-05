"""UE one-shot: rebuild the ship hull M_*_Hull materials from the UPDATED T_* hull
PNGs (richer detail per main feedback). Re-imports each hull set's D/N/R/M/AO/E
texture and recompiles/saves the corresponding M_*_Hull material so UE picks up
the stronger panel relief, roughness and emissive seams.

Targets (material -> texture set from material_map):
  M_Fighter_Hull -> Ship_Hull, M_Freighter_Hull -> Freighter,
  M_Corvette_Hull -> Corvette, M_Gunship_Hull -> Gunship, M_Miner_Hull -> Miner
"""
import unreal, os

GEN = r"C:\Users\akuma\Adastrea\Assets\FBX\generated"
TEX_DIR = GEN + r"\Textures"
TEX_FOLDER = "/Game/Textures"
MAT_FOLDER = "/Game/Materials/"

HULL = [
    ("M_Fighter_Hull", "T_Ship_Hull"),
    ("M_Freighter_Hull", "T_Freighter"),
    ("M_Corvette_Hull", "T_Corvette"),
    ("M_Gunship_Hull", "T_Gunship"),
    ("M_Miner_Hull", "T_Miner"),
]
MAPS = ["D","N","R","M","AO","E"]

def log(m): unreal.log("[hull-tex] " + str(m))
def ensure_folder(p):
    if not unreal.EditorAssetLibrary.does_directory_exist(p):
        unreal.EditorAssetLibrary.make_directory(p)

def reimport_texture(png, dest_folder, name):
    task = unreal.AssetImportTask()
    task.filename = png
    task.destination_path = dest_folder
    task.destination_name = name
    task.replace_existing = True
    task.automated = True
    task.save = True
    task.options = None
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    p = dest_folder + "/" + name
    return unreal.load_asset(p) if unreal.EditorAssetLibrary.does_asset_exist(p) else None

def load_mat_for(matname):
    p = MAT_FOLDER + matname
    return unreal.load_asset(p) if unreal.EditorAssetLibrary.does_asset_exist(p) else None

def rebuild(matname, texset):
    ensure_folder(TEX_FOLDER + "/" + texset)
    tex_assets = {}
    for mapn in MAPS:
        png = os.path.join(TEX_DIR, "%s_%s.png" % (texset, mapn))
        if os.path.exists(png):
            ap = reimport_texture(png, TEX_FOLDER + "/" + texset, "%s_%s" % (texset, mapn))
            if ap: tex_assets[mapn] = ap
    mat = load_mat_for(matname)
    if mat is None:
        # recreate if missing
        at = unreal.AssetToolsHelpers.get_asset_tools()
        mat = at.create_asset(matname, MAT_FOLDER, unreal.Material, unreal.MaterialFactoryNew())
    ED = unreal.MaterialEditingLibrary
    ED.delete_all_material_expressions(mat)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    y=60; ok=0
    def sample(asset, x, y):
        n=ED.create_material_expression(mat, unreal.MaterialExpressionTextureSample, x, y)
        n.set_editor_property("texture", asset); return n
    def try_c(n, out, prop):
        try:
            ED.connect_material_property(n, out, getattr(unreal.MaterialProperty, prop)); return True
        except Exception: return False
    if "D" in tex_assets and try_c(sample(tex_assets["D"],-600,y),"RGB","MP_BASE_COLOR"): y+=120; ok+=1
    if "N" in tex_assets and try_c(sample(tex_assets["N"],-600,y),"RGB","MP_NORMAL"): y+=120; ok+=1
    if "R" in tex_assets and try_c(sample(tex_assets["R"],-600,y),"R","MP_ROUGHNESS"): y+=120; ok+=1
    if "M" in tex_assets and try_c(sample(tex_assets["M"],-600,y),"R","MP_METALLIC"): y+=120; ok+=1
    if "E" in tex_assets and try_c(sample(tex_assets["E"],-600,y),"RGB","MP_EMISSIVE_COLOR"): y+=120; ok+=1
    ED.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset(MAT_FOLDER + matname)
    log("rebuilt %s (%s channels) from %s" % (matname, ok, texset))

for matname, texset in HULL:
    rebuild(matname, texset)
log("HULL_TEX_DONE")