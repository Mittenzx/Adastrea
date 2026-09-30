"""Import the interior prop library (Tools/build_interior_props.py) into Unreal.

Run inside the editor (remote execution or the Python console):

    py "C:/Users/akuma/Adastrea/Tools/import_interior_props.py" [Name ...] [--materials-only] [--dry-run]

  1. M_PropSurface: base colour / roughness / metallic parameters with a tiled
     micro-detail normal on the mesh UVs, so it reads the same at 1x (the prop
     assets) and 100x (props baked into the SM_Int_*_Decks parts).
     MI_Prop_<Name> for every slot in Tools/prop_materials.py.
  2. M_PropScreen: an unlit-looking emissive screen that shows a texture
     parameter "Screen" (a render target set at runtime by AInteriorFixture) with
     scanlines and a vignette. Parameters: Tint, Emissive, FlipU.
     MI_Prop_Live (exterior camera feeds) and MI_Prop_Term (console readouts).
  3. SM_Prop_<Name> and SM_Prop_Fx_<Name> from Assets/FBX/generated/props into
     /AdastreaShips/Meshes/Props (slots bind by name).
"""
import json
import os
import sys

import unreal

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import importlib  # noqa: E402
import import_art_gap_assets as art  # noqa: E402
import prop_materials  # noqa: E402
importlib.reload(prop_materials)          # the editor's Python outlives edits to the palette
from prop_materials import PROP_MATS, SCREEN_MATS, BEACON_MATS  # noqa: E402

MAT_DIR = art.INT_MAT_DIR                   # in MATERIAL_SEARCH_DIRS, so slots bind by name
SURFACE = MAT_DIR + "/M_PropSurface"
SCREEN = MAT_DIR + "/M_PropScreen"
PROPS_DIR = os.path.join(art.GEN_DIR, "props")
MESH_DIR = "/AdastreaShips/Meshes/Props"
EXT_MESH_DIR = "/AdastreaShips/Meshes/ExteriorProps"
BEACON = MAT_DIR + "/M_PropBeacon"
MEL = art.MEL
F1 = unreal.CustomMaterialOutputType.CMOT_FLOAT1
F2 = unreal.CustomMaterialOutputType.CMOT_FLOAT2
F3 = unreal.CustomMaterialOutputType.CMOT_FLOAT3


def _vec(mat, name, rgb, x, y):
    n = MEL.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, x, y)
    n.set_editor_property("parameter_name", name)
    n.set_editor_property("default_value", unreal.LinearColor(rgb[0], rgb[1], rgb[2], 1))
    return n


def build_surface_master():
    art.log("material %s (prop surface master)" % SURFACE)
    if art.DRY_RUN:
        return
    mat, path = art.get_or_create_material(MAT_DIR, "M_PropSurface")
    MEL.delete_all_material_expressions(mat)
    col = _vec(mat, "BaseColor", (0.5, 0.5, 0.5), -800, -200)
    MEL.connect_material_property(col, "", unreal.MaterialProperty.MP_BASE_COLOR)
    MEL.connect_material_property(art._scalar(mat, "Roughness", 0.5, -800, 0), "", unreal.MaterialProperty.MP_ROUGHNESS)
    MEL.connect_material_property(art._scalar(mat, "Metallic", 0.0, -800, 100), "", unreal.MaterialProperty.MP_METALLIC)
    det = art.int_tex("DetailMicro", "N")
    if det is None:
        art.import_texture_set("T_Int_DetailMicro", art.INT_TEX_SRC, art.INT_TEX_ROOT)
        det = art.int_tex("DetailMicro", "N")
    if det is not None:
        uv = MEL.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -1400, 300)
        mul = MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -1150, 300)
        MEL.connect_material_expressions(uv, "", mul, "A")
        MEL.connect_material_expressions(art._scalar(mat, "DetailTiling", 16.0, -1400, 420), "", mul, "B")
        tex = MEL.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -900, 300)
        tex.set_editor_property("parameter_name", "DetailNormal")
        tex.set_editor_property("texture", det)
        tex.set_editor_property("sampler_type", art.sampler_for_texture(det))
        MEL.connect_material_expressions(mul, "", tex, "UVs")
        flat = MEL.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -900, 520)
        flat.set_editor_property("constant", unreal.LinearColor(0, 0, 1, 1))
        lerp = MEL.create_material_expression(mat, unreal.MaterialExpressionLinearInterpolate, -500, 400)
        MEL.connect_material_expressions(flat, "", lerp, "A")
        MEL.connect_material_expressions(tex, "RGB", lerp, "B")
        MEL.connect_material_expressions(art._scalar(mat, "DetailStrength", 0.35, -900, 640), "", lerp, "Alpha")
        MEL.connect_material_property(lerp, "", unreal.MaterialProperty.MP_NORMAL)
    MEL.recompile_material(mat)
    art.EAL.save_asset(path, only_if_is_dirty=False)


def build_screen_master():
    art.log("material %s (render-target screen master)" % SCREEN)
    if art.DRY_RUN:
        return
    mat, path = art.get_or_create_material(MAT_DIR, "M_PropScreen")
    MEL.delete_all_material_expressions(mat)
    uv = MEL.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -1600, 0)
    # the FBX importer already turns V top-down, which is what a render target wants
    flip = art._custom(mat, -1350, 0, "return float2(lerp(UV.x, 1.0 - UV.x, FlipU), UV.y);",
                       ["UV", "FlipU"], F2, "screen UV")
    MEL.connect_material_expressions(uv, "", flip, "UV")
    MEL.connect_material_expressions(art._scalar(mat, "FlipU", 0.0, -1600, 120), "", flip, "FlipU")
    tex = MEL.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -1050, 0)
    tex.set_editor_property("parameter_name", "Screen")
    black = unreal.load_asset("/Engine/EngineResources/Black")
    if black:
        tex.set_editor_property("texture", black)
    tex.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_COLOR)
    MEL.connect_material_expressions(flip, "", tex, "UVs")
    em = art._custom(mat, -650, 0,
                     "float scan = 0.86 + 0.14 * step(0.5, frac(UV.y * 180.0)); "
                     "float2 d = UV - 0.5; float vig = saturate(1.0 - dot(d, d) * 1.6); "
                     "float flick = 0.97 + 0.03 * sin(T * 57.0); "
                     "return (C + Idle) * Tint * Strength * scan * vig * flick;",
                     ["C", "UV", "Tint", "Strength", "T", "Idle"], F3, "screen emissive")
    MEL.connect_material_expressions(tex, "RGB", em, "C")
    MEL.connect_material_expressions(uv, "", em, "UV")
    MEL.connect_material_expressions(_vec(mat, "Tint", (0.35, 0.8, 1.0), -1050, 250), "", em, "Tint")
    MEL.connect_material_expressions(art._scalar(mat, "Emissive", 2.5, -1050, 380), "", em, "Strength")
    MEL.connect_material_expressions(MEL.create_material_expression(mat, unreal.MaterialExpressionTime, -1050, 480), "", em, "T")
    MEL.connect_material_expressions(_vec(mat, "IdleGlow", (0.004, 0.006, 0.008), -1050, 560), "", em, "Idle")
    MEL.connect_material_property(em, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    base = MEL.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -650, 300)
    base.set_editor_property("constant", unreal.LinearColor(0.002, 0.002, 0.003, 1))
    MEL.connect_material_property(base, "", unreal.MaterialProperty.MP_BASE_COLOR)
    rough = MEL.create_material_expression(mat, unreal.MaterialExpressionConstant, -650, 400)
    rough.set_editor_property("r", 0.12)
    MEL.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    MEL.recompile_material(mat)
    art.EAL.save_asset(path, only_if_is_dirty=False)


def build_beacon_master():
    """Flat emissive lens: Color x Intensity (both set per light at runtime)."""
    art.log("material %s (exterior light lens)" % BEACON)
    if art.DRY_RUN:
        return
    mat, path = art.get_or_create_material(MAT_DIR, "M_PropBeacon")
    MEL.delete_all_material_expressions(mat)
    col = _vec(mat, "Color", (1.0, 0.95, 0.85), -800, 0)
    mul = MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -500, 0)
    MEL.connect_material_expressions(col, "", mul, "A")
    MEL.connect_material_expressions(art._scalar(mat, "Intensity", 20.0, -800, 200), "", mul, "B")
    MEL.connect_material_property(mul, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    MEL.connect_material_property(col, "", unreal.MaterialProperty.MP_BASE_COLOR)
    MEL.recompile_material(mat)
    art.EAL.save_asset(path, only_if_is_dirty=False)


def make_mi(name, parent, vectors, scalars):
    path = MAT_DIR + "/" + name
    art.log("  MI %s" % name)
    if art.DRY_RUN:
        return
    mi = unreal.load_asset(path) if art.asset_exists(path) else None
    if mi is None:
        mi = art.ASSET_TOOLS.create_asset(name, MAT_DIR, unreal.MaterialInstanceConstant,
                                          unreal.MaterialInstanceConstantFactoryNew())
    MEL.set_material_instance_parent(mi, parent)
    for k, v in vectors.items():
        MEL.set_material_instance_vector_parameter_value(mi, k, unreal.LinearColor(v[0], v[1], v[2], 1.0))
    for k, v in scalars.items():
        MEL.set_material_instance_scalar_parameter_value(mi, k, float(v))
    art.EAL.save_asset(path, only_if_is_dirty=False)


def build_materials():
    build_surface_master()
    build_screen_master()
    build_beacon_master()
    if art.DRY_RUN:
        return True
    surf = unreal.load_asset(SURFACE)
    scr = unreal.load_asset(SCREEN)
    for slot, (rgb, rough, metal) in PROP_MATS.items():
        make_mi("MI_" + slot[2:], surf, {"BaseColor": rgb}, {"Roughness": rough, "Metallic": metal})
    bea = unreal.load_asset(BEACON)
    for slot, (rgb, strength) in BEACON_MATS.items():
        make_mi("MI_" + slot[2:], bea, {"Color": rgb}, {"Intensity": strength})
    black = unreal.load_asset("/Engine/EngineResources/Black")
    for slot, (tint, strength) in SCREEN_MATS.items():
        make_mi("MI_" + slot[2:], scr, {"Tint": tint}, {"Emissive": strength})
        # the runtime binds a render target; the asset itself always shows black
        MEL.set_material_instance_texture_parameter_value(unreal.load_asset(MAT_DIR + "/MI_" + slot[2:]), "Screen", black)
        art.EAL.save_asset(MAT_DIR + "/MI_" + slot[2:], only_if_is_dirty=False)
    return True


def import_props(names):
    with open(os.path.join(PROPS_DIR, "props_contract.json")) as fh:
        contract = json.load(fh)
    # route SM_Prop_* to the Props folder and read the FBX from props/; the editor's
    # Python is long-lived, so put the shared importer module back afterwards
    saved_gen, saved_dest = art.GEN_DIR, list(art.DESTINATIONS)
    art.DESTINATIONS.insert(0, ("SM_Prop_*", MESH_DIR))
    art.DESTINATIONS.insert(0, ("SM_ExtProp_*", EXT_MESH_DIR))
    art.GEN_DIR = PROPS_DIR
    ok = True
    try:
        for group in ("props", "fixtures", "exterior"):
            for name, info in contract[group].items():
                if names and name not in names:
                    continue
                ok = art.import_mesh(info["mesh"]) and ok
    finally:
        art.GEN_DIR = saved_gen
        art.DESTINATIONS[:] = saved_dest
    return ok


def main(argv):
    art.DRY_RUN = "--dry-run" in argv
    names = [a for a in argv if not a.startswith("--")]
    ok = build_materials()
    if "--materials-only" not in argv:
        ok = import_props(names) and ok
    art.log("RESULT_OK" if ok else "RESULT_FAIL")


if __name__ == "__main__":
    try:
        main(sys.argv[1:])
    except Exception:
        import traceback
        unreal.log_error("[props] crashed:\n" + traceback.format_exc())
        unreal.log("[art-gap] RESULT_FAIL")
