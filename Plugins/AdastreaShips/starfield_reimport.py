"""Re-import the 4096 starfield texture (replace) with BILINEAR filtering +
mipmaps, so stars render as soft small glowing points (realistic), not hard
blocky squares. Keep the realistic material wired to it.
"""
import unreal

AT = unreal.AssetToolsHelpers.get_asset_tools()
DEST = "/Game/Textures/Environment"
TEX_PATH = DEST + "/T_Starfield"
src_png = r"C:\Users\akuma\Adastrea\Assets\FBX\generated\T_Starfield.png"

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
    tex.set_editor_property("filter", unreal.TextureFilter.TF_BILINEAR)
    unreal.EditorAssetLibrary.save_asset(TEX_PATH)
    unreal.log("=== Re-imported 4096 starfield (bilinear) at %s ===" % TEX_PATH)

# Recompile material so it picks up the new texture settings
mat = unreal.EditorAssetLibrary.load_asset("/Game/Materials/M_Starfield_Real")
if mat:
    from unreal import MaterialEditingLibrary as ED
    ED.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset("/Game/Materials/M_Starfield_Real")
    unreal.log("=== recompiled M_Starfield_Real ===")
unreal.log("=== starfield_reimport.py complete ===")