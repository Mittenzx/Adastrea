"""Make the star texture render as crisp pinpoints: use Nearest filtering and
no mipmap generation so 1-2px stars don't bilinear-blur into blocky squares on
the huge dome. Applies to the T_Starfield textures and the material's sampler.
"""
import unreal

# The star texture that the realistic material uses.
for tpath in ["/Game/Textures/Environment/T_Starfield", "/Game/Textures/T_Starfield"]:
    tex = unreal.EditorAssetLibrary.load_asset(tpath)
    if tex and isinstance(tex, unreal.Texture2D):
        tex.set_editor_property("filter", unreal.TextureFilter.TF_NEAREST)
        tex.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
        tex.set_editor_property("never_stream", True)
        unreal.EditorAssetLibrary.save_asset(tpath)
        unreal.log("=== Set Nearest/no-mip on %s ===" % tpath)
    else:
        unreal.log("texture not found at %s" % tpath)

# Force a recompile of the material so it re-imports the sampler.
mat = unreal.EditorAssetLibrary.load_asset("/Game/Materials/M_Starfield_Real")
if mat:
    from unreal import MaterialEditingLibrary as ED
    ED.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset("/Game/Materials/M_Starfield_Real")
    unreal.log("=== recompiled M_Starfield_Real ===")
unreal.log("=== star_crisp.py complete ===")