"""Import a detailed ship hull + its detail kit (Tools/build_ship_hull_detail.py) into UE.

  1. builds the shared kit materials /Game/Materials/M_ShipDetail_* (slot names in the
     kit FBX bind to them by name) and imports T_ShipDetail_Hazard_D for the hatch trim
  2. reimports the hull (same asset, so its Blueprint, scale and HullMaterialOverride
     stay), checks its bounds did not move, and rebuilds convex collision
  3. imports SM_Ship_<Ship>_Detail next to the ship's other parts and matches its
     build scale to the hull's (fresh part imports come in 1x, some hulls are 0.01x)

ASpaceship::AttachShipDetail attaches /AdastreaShips/Meshes/Ships/<base>_Detail at runtime.

Needs the full editor (StaticMeshEditorSubsystem is None in -run=pythonscript):
  UnrealEditor.exe Adastrea.uproject -ExecutePythonScript="<abs>/Tools/ue_import_ship_detail.py [--ship Corvette_01] [--dry-run] [--quit]"
or remote exec into a running editor.
"""
import os
import sys

import unreal

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import import_art_gap_assets as ia  # noqa: E402

ARGS = sys.argv[1:]
SHIP = ARGS[ARGS.index("--ship") + 1] if "--ship" in ARGS else "Corvette_01"
HULL = {"Corvette_01": "SM_Ship_Corvette_01_Assembled_UniqueUV"}.get(SHIP, "SM_Ship_%s_Assembled" % SHIP)
DETAIL = "SM_Ship_%s_Detail" % SHIP
MAT_DIR = "/Game/Materials"
MEL = unreal.MaterialEditingLibrary
SMES = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)

# name -> (base colour, metallic, roughness, emissive colour * strength or None)
KIT_MATERIALS = {
    "M_ShipDetail_Trim":       ((0.16, 0.16, 0.17), 0.85, 0.36, None),
    "M_ShipDetail_Dark":       ((0.015, 0.015, 0.018), 0.4, 0.7, None),
    "M_ShipDetail_Paint":      ((0.78, 0.78, 0.74), 0.0, 0.55, None),
    "M_ShipDetail_GlowWhite":  ((1.0, 1.0, 1.0), 0.0, 0.3, (1.0, 0.95, 0.85, 20.0)),
    "M_ShipDetail_GlowRed":    ((1.0, 0.0, 0.0), 0.0, 0.3, (1.0, 0.08, 0.05, 20.0)),
    "M_ShipDetail_GlowGreen":  ((0.0, 1.0, 0.0), 0.0, 0.3, (0.1, 1.0, 0.25, 20.0)),
}


def log(m):
    unreal.log("[ship-detail] " + str(m))


def const3(mat, rgb, x, y):
    n = MEL.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, x, y)
    n.set_editor_property("constant", unreal.LinearColor(rgb[0], rgb[1], rgb[2], 1.0))
    return n


def const(mat, v, x, y):
    n = MEL.create_material_expression(mat, unreal.MaterialExpressionConstant, x, y)
    n.set_editor_property("r", v)
    return n


def build_kit_materials():
    for name, (base, metal, rough, emit) in KIT_MATERIALS.items():
        mat, path = ia.get_or_create_material(MAT_DIR, name)
        MEL.delete_all_material_expressions(mat)
        MEL.connect_material_property(const3(mat, base, -400, -200), "", unreal.MaterialProperty.MP_BASE_COLOR)
        MEL.connect_material_property(const(mat, metal, -400, 0), "", unreal.MaterialProperty.MP_METALLIC)
        MEL.connect_material_property(const(mat, rough, -400, 100), "", unreal.MaterialProperty.MP_ROUGHNESS)
        if emit:
            MEL.connect_material_property(const3(mat, [c * emit[3] for c in emit[:3]], -400, 250), "",
                                          unreal.MaterialProperty.MP_EMISSIVE_COLOR)
        MEL.recompile_material(mat)
        unreal.EditorAssetLibrary.save_asset(path, only_if_is_dirty=False)
        log("material " + path)
    tex = ia.import_texture_set("T_ShipDetail_Hazard")
    mat, path = ia.get_or_create_material(MAT_DIR, "M_ShipDetail_Hazard")
    MEL.delete_all_material_expressions(mat)
    uv = MEL.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -900, 0)
    uv.set_editor_property("u_tiling", 2.0)
    uv.set_editor_property("v_tiling", 2.0)
    s = MEL.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -600, 0)
    s.set_editor_property("texture", tex["D"])
    s.set_editor_property("sampler_type", ia.sampler_for_texture(tex["D"]))
    MEL.connect_material_expressions(uv, "", s, "UVs")
    MEL.connect_material_property(s, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    MEL.connect_material_property(const(mat, 0.2, -400, 200), "", unreal.MaterialProperty.MP_METALLIC)
    MEL.connect_material_property(const(mat, 0.55, -400, 300), "", unreal.MaterialProperty.MP_ROUGHNESS)
    MEL.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset(path, only_if_is_dirty=False)
    log("material " + path)


def bounds(path):
    m = unreal.load_asset(path)
    b = m.get_bounds()
    return b.origin, b.box_extent


def fmt(v):
    return "(%.0f, %.0f, %.0f)" % (v.x, v.y, v.z)


def main():
    ia.DRY_RUN = "--dry-run" in ARGS
    hull_path = ia.destination_for(HULL) + "/" + HULL
    det_path = ia.destination_for(DETAIL) + "/" + DETAIL
    o0, e0 = bounds(hull_path)
    log("hull before: origin %s extent %s" % (fmt(o0), fmt(e0)))
    if not ia.DRY_RUN:
        build_kit_materials()
    ia.import_mesh(HULL)
    ia.import_mesh(DETAIL)
    if ia.DRY_RUN:
        return
    o1, e1 = bounds(hull_path)
    log("hull after:  origin %s extent %s" % (fmt(o1), fmt(e1)))
    if max(abs(e1.x - e0.x), abs(e1.y - e0.y), abs(e1.z - e0.z)) > 0.05 * max(e0.x, e0.y, e0.z):
        unreal.log_warning("[ship-detail] hull bounds changed by more than 5% - check the export scale")
    # collision from the new shape, as the redesign import did
    hull = unreal.load_asset(hull_path)
    SMES.remove_collisions(hull)
    SMES.set_convex_decomposition_collisions(hull, 12, 24, 200000)
    unreal.EditorAssetLibrary.save_asset(hull_path, only_if_is_dirty=False)
    log("hull collision: " + ia.collision_summary(hull))
    # match the kit's build scale to the hull's (memory: small hulls are stored at 0.01x)
    det = unreal.load_asset(det_path)
    hs = SMES.get_lod_build_settings(hull, 0).build_scale3d
    ds_settings = SMES.get_lod_build_settings(det, 0)
    if abs(ds_settings.build_scale3d.x - hs.x) > 1e-6:
        ds_settings.build_scale3d = hs
        SMES.set_lod_build_settings(det, 0, ds_settings)
        log("detail build scale -> %s" % hs)
    SMES.remove_collisions(det)
    unreal.EditorAssetLibrary.save_asset(det_path, only_if_is_dirty=False)
    od, ed = bounds(det_path)
    log("detail: origin %s extent %s (hull %s %s)" % (fmt(od), fmt(ed), fmt(o1), fmt(e1)))
    log("DONE hull tris=%d detail tris=%d" % (hull.get_num_triangles(0), det.get_num_triangles(0)))


main()
if "--quit" in ARGS:
    unreal.SystemLibrary.quit_editor()
