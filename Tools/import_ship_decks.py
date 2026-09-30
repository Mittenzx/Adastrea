"""Import the full-deck ship interiors (Tools/build_ship_decks.py) into Unreal.

Run inside the editor (remote execution or the Python console):

    py "C:/Users/akuma/Adastrea/Tools/import_ship_decks.py" [Ship ...] [--no-bp | --bp-only] [--dry-run]

With no ship names, every SM_Int_<Ship>_Decks_contract.json in Assets/FBX/generated
is imported. Steps:
  1. Materials: the Battleship's MI_Int_Grate/Hazard/Bulkhead + coloured emissives
     (Tools/import_battleship_decks.py), plus the per-style slots added for these
     decks: Clean, Steel, Dark, Paint, AccentBlue, Carpet, Wood, Brass, Plant
     (instances of M_IntSurface_Oriented) and the Cyan / Warm emissives.
  2. Import every part FBX the contract lists (slots bind by name).
  3. Collision part -> complex-as-simple.
  4. Shell sockets: Entry, Seat, L_* lights and P_<Part> for every sibling part.
     The P_ sockets are what ASpaceshipInterior mounts from.
  5. The ship blueprint in the contract: InteriorShellMesh = the Shell.
     InteriorFamily is left as it is. The P_ sockets alone switch the interior
     onto the full-deck path.
"""
import glob
import json
import os
import sys

import unreal

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import import_art_gap_assets as art  # noqa: E402
import import_battleship_decks as ibd  # noqa: E402

MESH_DIR = "/AdastreaShips/Meshes/Interiors"
FBX_SCALE = 100.0
SOCKET_TAG = "ShipDecks"

_S = art._all
SURFACE_MIS = {
    # clinical white panelling (science, medical, colony, luxury walls)
    "MI_Int_Clean": ({"Floor": "StnFloor", "WallLo": "StnWall", "WallUp": "StnWallUpper", "Ceil": "StnCeiling"},
                     {}, (0.95, 0.96, 1.0)),
    # bare steel frames for the civilian styles
    "MI_Int_Steel": (_S("ShipWallUpper"), {"NormalStrength": 0.5}, (0.62, 0.65, 0.70)),
    # stealth plate
    "MI_Int_Dark": ({"Floor": "ShipDeck", "WallLo": "ShipWall", "WallUp": "ShipWallUpper", "Ceil": "ShipCeiling"},
                    {}, (0.20, 0.21, 0.25)),
    # safety orange
    "MI_Int_Paint": (_S("ShipWallUpper"), {"NormalStrength": 0.4}, (0.95, 0.40, 0.08)),
    "MI_Int_AccentBlue": (_S("ShipWallUpper"), {"NormalStrength": 0.4}, (0.12, 0.30, 0.75)),
    "MI_Int_Carpet": (_S("HabFloor"), {}, (0.50, 0.10, 0.12)),
    "MI_Int_Wood": (_S("HabWall"), {}, (0.60, 0.36, 0.20)),
    "MI_Int_Brass": (_S("ShipWallUpper"), {"NormalStrength": 0.3}, (0.90, 0.68, 0.32)),
    "MI_Int_Plant": (_S("HabFloor"), {}, (0.20, 0.50, 0.15)),
}
EMISSIVE = {
    "M_Int_LightsCyan": ((0.20, 0.85, 1.0), 5.0),
    "M_Int_LightsWarm": ((1.0, 0.72, 0.45), 7.0),
}


def build_materials():
    if not ibd.build_materials():
        return False
    master = unreal.load_asset(art.INT_MASTER)
    for name, (sets, scalars, tint) in SURFACE_MIS.items():
        art.make_surface_mi(art.INT_MAT_DIR, name, master, sets, scalars, tint)
    for name, (rgb, strength) in EMISSIVE.items():
        ibd.build_emissive(art.INT_MAT_DIR + "/" + name, rgb, strength)
    return True


def set_sockets(mesh, sockets, parts):
    stale = list(mesh.get_sockets_by_tag(SOCKET_TAG))
    wanted = dict(sockets)
    for p in parts:
        if p != "Shell":
            wanted["P_" + p] = [0.0, 0.0, 0.0, 0.0]
    for name in wanted:
        s = mesh.find_socket(name)
        if s and s not in stale:
            stale.append(s)
    for s in stale:
        mesh.remove_socket(s)
    for name, (x, y, z, yaw) in wanted.items():
        s = unreal.StaticMeshSocket(mesh)
        s.set_editor_property("socket_name", name)
        s.set_editor_property("tag", SOCKET_TAG)
        s.set_editor_property("relative_location", unreal.Vector(x * FBX_SCALE, y * FBX_SCALE, z * FBX_SCALE))
        s.set_editor_property("relative_rotation", unreal.Rotator(0.0, 0.0, yaw))
        mesh.add_socket(s)
    n = len(mesh.get_sockets_by_tag(SOCKET_TAG))
    art.log("   %s: %d sockets (%d parts)" % (mesh.get_name(), n, len(parts) - 1))


def wire_bp(bp_path, shell_path):
    bp = unreal.load_asset(bp_path)
    mesh = unreal.load_asset(shell_path)
    if not bp or not mesh:
        art.log("! BP wiring skipped: bp=%s mesh=%s" % (bool(bp), bool(mesh)))
        return False
    cdo = unreal.get_default_object(bp.generated_class())
    before = cdo.get_editor_property("InteriorShellMesh")
    if art.DRY_RUN:
        art.log("would set %s: %s -> %s" % (bp_path, before, shell_path))
        return True
    cdo.set_editor_property("InteriorShellMesh", mesh)
    art.EAL.save_asset(bp_path, only_if_is_dirty=False)
    art.log("BP %s: InteriorShellMesh %s -> %s" % (bp_path, before.get_name() if before else None, mesh.get_name()))
    return True


def contracts(names):
    out = []
    for fp in sorted(glob.glob(os.path.join(art.GEN_DIR, "SM_Int_*_Decks_contract.json"))):
        with open(fp) as fh:
            c = json.load(fh)
        # the Battleship's contract has its own importer (Tools/import_battleship_decks.py);
        # it carries "ship" too now, for Tools/pie_walk_ship_decks.py
        if "ship" not in c or c.get("family") == "BattleshipDecks":
            continue
        if names and c["ship"] not in names:
            continue
        out.append(c)
    return out


def import_ship(c, bp=True, bp_only=False):
    prefix = c["prefix"]
    shell_path = "%s/%s_Shell" % (MESH_DIR, prefix)
    ok = True
    if not bp_only:
        if c.get("problems"):
            art.log("! %s contract has problems: %s" % (prefix, c["problems"][:4]))
        for part in c["parts"]:
            ok = art.import_mesh("%s_%s" % (prefix, part)) and ok
        if not art.DRY_RUN:
            col = unreal.load_asset("%s/%s_Collision" % (MESH_DIR, prefix))
            if col:
                ibd.set_complex_collision(col)
                art.EAL.save_asset(col.get_path_name(), only_if_is_dirty=False)
            shell = unreal.load_asset(shell_path)
            if shell:
                set_sockets(shell, c["sockets_ue_design_cm"], c["parts"])
                art.EAL.save_asset(shell.get_path_name(), only_if_is_dirty=False)
    if bp or bp_only:
        ok = wire_bp(c["bp"], shell_path) and ok
    return ok


def main(argv):
    art.DRY_RUN = "--dry-run" in argv
    names = [a for a in argv if not a.startswith("--")]
    cs = contracts(names)
    if not cs:
        art.log("! no ship-deck contracts matched %s" % names)
        art.log("RESULT_FAIL")
        return
    ok = True
    if "--bp-only" not in argv:
        ok = build_materials()
    for c in cs:
        art.log("=== %s (%s, %.0f x %.0f m)" % (c["ship"], c["style"], c["size_m"][0], c["size_m"][1]))
        ok = import_ship(c, bp="--no-bp" not in argv, bp_only="--bp-only" in argv) and ok
    art.log("RESULT_OK" if ok else "RESULT_FAIL")


if __name__ == "__main__":
    try:
        main(sys.argv[1:])
    except Exception:
        import traceback
        unreal.log_error("[decks] crashed:\n" + traceback.format_exc())
        unreal.log("[art-gap] RESULT_FAIL")
