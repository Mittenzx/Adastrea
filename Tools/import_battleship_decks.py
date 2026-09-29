"""Import the Battleship full-deck interior (Tools/build_battleship_decks.py) into Unreal.

Run inside the editor (remote execution or the Python console), after the C++ with
EShipInteriorFamily::BattleshipDecks is built:

    py "C:/Users/akuma/Adastrea/Tools/import_battleship_decks.py" [--no-bp | --bp-only] [--dry-run]

Steps:
  1. Materials for the new slots: MI_Int_Grate / MI_Int_Hazard / MI_Int_Bulkhead
     (instances of M_IntSurface_Oriented, like the other kit MIs) and the flat
     emissive M_Int_LightsRed / Amber / Green / Blue.
  2. Import every SM_Int_Battleship_Decks_<Part>.fbx (import_art_gap_assets.import_mesh:
     100x FBX, slots bound by name).
  3. Collision part -> complex-as-simple (the avatar walks the authored floors/ramps).
  4. Shell part gets the contract's sockets: Entry, Seat and the L_* light sockets.
  5. BP_Battleship: InteriorShellMesh = the Shell, InteriorFamily = BattleshipDecks.
"""
import json
import os
import sys

import unreal

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import import_art_gap_assets as art  # noqa: E402

PREFIX = "SM_Int_Battleship_Decks"
PARTS = ["Shell", "Spine", "CIC", "Quarters", "Mess", "Medbay", "Briefing", "Armory",
         "Hangar", "Dropship", "Engineering", "Lights", "Collision"]
MESH_DIR = "/AdastreaShips/Meshes/Interiors"
BP_PATH = "/Game/Blueprints/Ships/BP_Battleship"
FBX_SCALE = 100.0      # SM_Int_* kits are exported 100x design cm

SURFACE_MIS = {
    "MI_Int_Grate": (art._all("EngGrate"), {}, None),
    # Hazard set tiles at 100 cm (MATERIAL_MAPPING.md: tiling 2.0)
    "MI_Int_Hazard": (art._all("Hazard"), {"TilingFloor": 2, "TilingWall": 2, "TilingCeiling": 2}, None),
    # olive-drab painted plate for bulkheads, frames, lockers, crates and the dropship
    "MI_Int_Bulkhead": (art._all("ShipWallUpper"), {"NormalStrength": 0.5}, (0.60, 0.66, 0.46)),
}
EMISSIVE = {
    "M_Int_LightsRed": ((1.0, 0.09, 0.04), 12.0),
    "M_Int_LightsAmber": ((1.0, 0.52, 0.12), 9.0),
    "M_Int_LightsGreen": ((0.28, 1.0, 0.38), 3.5),     # CRT phosphor: readable, not blown out
    "M_Int_LightsBlue": ((0.30, 0.55, 1.0), 7.0),
}


def build_emissive(path, rgb, strength):
    folder, name = path.rsplit("/", 1)
    art.log("material %s (flat emissive %s x%.1f)" % (path, rgb, strength))
    if art.DRY_RUN:
        return
    mat, path = art.get_or_create_material(folder, name)
    art.MEL.delete_all_material_expressions(mat)
    col = art.MEL.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -600, 0)
    col.set_editor_property("constant", unreal.LinearColor(rgb[0], rgb[1], rgb[2], 1))
    art.MEL.connect_material_property(col, "", unreal.MaterialProperty.MP_BASE_COLOR)
    em = art.MEL.create_material_expression(mat, unreal.MaterialExpressionMultiply, -300, 100)
    art.MEL.connect_material_expressions(col, "", em, "A")
    em.set_editor_property("const_b", strength)
    art.MEL.connect_material_property(em, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    art.MEL.recompile_material(mat)
    art.EAL.save_asset(path, only_if_is_dirty=False)


def build_materials():
    master = unreal.load_asset(art.INT_MASTER)
    if master is None:
        art.log("! %s missing - run import_art_gap_assets.py --interiors first" % art.INT_MASTER)
        return False
    for name, (sets, scalars, tint) in SURFACE_MIS.items():
        art.make_surface_mi(art.INT_MAT_DIR, name, master, sets, scalars, tint)
    for name, (rgb, strength) in EMISSIVE.items():
        build_emissive(art.INT_MAT_DIR + "/" + name, rgb, strength)
    return True


def set_complex_collision(mesh):
    body = mesh.get_editor_property("body_setup")
    if body is None:
        art.log("! %s has no body setup" % mesh.get_name())
        return
    body.set_editor_property("collision_trace_flag", unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
    art.log("   %s: collision complex-as-simple" % mesh.get_name())


SOCKET_TAG = "BattleshipDecks"


def set_sockets(mesh, sockets):
    # StaticMesh.Sockets is protected in Python: ours carry a tag so a rerun can
    # find and replace them (plus any same-named socket from elsewhere).
    stale = list(mesh.get_sockets_by_tag(SOCKET_TAG))
    for name in sockets:
        s = mesh.find_socket(name)
        if s and s not in stale:
            stale.append(s)
    for s in stale:
        mesh.remove_socket(s)
    for name, (x, y, z, yaw) in sockets.items():
        s = unreal.StaticMeshSocket(mesh)
        s.set_editor_property("socket_name", name)
        s.set_editor_property("tag", SOCKET_TAG)
        s.set_editor_property("relative_location", unreal.Vector(x * FBX_SCALE, y * FBX_SCALE, z * FBX_SCALE))
        s.set_editor_property("relative_rotation", unreal.Rotator(0.0, 0.0, yaw))
        mesh.add_socket(s)
    names = [str(s.get_editor_property("socket_name")) for s in mesh.get_sockets_by_tag(SOCKET_TAG)]
    art.log("   %s: %d sockets (%s ...)" % (mesh.get_name(), len(names), ", ".join(names[:4])))


def wire_bp(shell_path):
    bp = unreal.load_asset(BP_PATH)
    mesh = unreal.load_asset(shell_path)
    fam = getattr(unreal.ShipInteriorFamily, "BATTLESHIP_DECKS", None)
    if not bp or not mesh or fam is None:
        art.log("! BP wiring skipped: bp=%s mesh=%s enum=%s (rebuild the C++?)" % (bool(bp), bool(mesh), fam))
        return False
    cdo = unreal.get_default_object(bp.generated_class())
    before = (cdo.get_editor_property("InteriorShellMesh"), cdo.get_editor_property("InteriorFamily"))
    if art.DRY_RUN:
        art.log("would set %s: %s -> %s / %s" % (BP_PATH, before, shell_path, fam))
        return True
    cdo.set_editor_property("InteriorShellMesh", mesh)
    cdo.set_editor_property("InteriorFamily", fam)
    art.EAL.save_asset(BP_PATH, only_if_is_dirty=False)
    art.log("BP %s: %s -> InteriorShellMesh=%s InteriorFamily=%s" % (BP_PATH, before, mesh.get_name(), fam))
    return True


def main(argv):
    art.DRY_RUN = "--dry-run" in argv
    if "--bp-only" in argv:
        # e.g. after a merge took another branch's BP_Battleship: re-wire it only
        art.log("RESULT_OK" if wire_bp("%s/%s_Shell" % (MESH_DIR, PREFIX)) else "RESULT_FAIL")
        return
    with open(os.path.join(art.GEN_DIR, PREFIX + "_contract.json")) as fh:
        contract = json.load(fh)
    ok = build_materials()
    for part in PARTS:
        ok = art.import_mesh("%s_%s" % (PREFIX, part)) and ok
    if not art.DRY_RUN:
        col = unreal.load_asset("%s/%s_Collision" % (MESH_DIR, PREFIX))
        if col:
            set_complex_collision(col)
            art.EAL.save_asset(col.get_path_name(), only_if_is_dirty=False)
        shell = unreal.load_asset("%s/%s_Shell" % (MESH_DIR, PREFIX))
        if shell:
            set_sockets(shell, contract["sockets_ue_design_cm"])
            art.EAL.save_asset(shell.get_path_name(), only_if_is_dirty=False)
            b = shell.get_bounds().box_extent
            art.log("   Shell extent (%.0f, %.0f, %.0f) -> x0.01 = %.0f x %.0f m" % (
                b.x, b.y, b.z, b.x * 0.02 / 100.0, b.y * 0.02 / 100.0))
    if "--no-bp" not in argv:
        ok = wire_bp("%s/%s_Shell" % (MESH_DIR, PREFIX)) and ok
    art.log("RESULT_OK" if ok else "RESULT_FAIL")


if __name__ == "__main__":
    try:
        main(sys.argv[1:])
    except Exception:
        import traceback
        unreal.log_error("[decks] crashed:\n" + traceback.format_exc())
        unreal.log("[art-gap] RESULT_FAIL")
