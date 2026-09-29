"""Import the ship redesigns (Tools/generate_ship_redesigns.py) over the in-game ship meshes.

Expects the redesign FBX already copied into Assets/FBX/generated (same names as the
old meshes). For every redesigned ship this reimports all its parts through
import_art_gap_assets (slot -> material binding, plugin destination), plus the four
hull assets the Blueprints load from /Game/Assets/Ships:
  SM_Ship_Cruiser_01_Assembled, SM_Ship_CommandXL_01_Assembled,
  SM_Ship_Corvette_01_Assembled_UniqueUV, SM_Ship_Battleship_01_Assembled_UniqueUV
(the last two keep their names but now hold the redesign with tiled UVs).
Every hull gets fresh convex-decomposition collision from the new shape, and
BP_Ship_Corvette / BP_Battleship get HullMaterialOverride = the tiled class hull,
since the old baked unique-UV materials no longer match the geometry.

Run headless (editor closed):
  UnrealEditor-Cmd.exe Adastrea.uproject -run=pythonscript -script="<abs>/Tools/ue_import_ship_redesigns.py [--dry-run]" -unattended -nosplash -nullrhi
"""
import glob
import os
import sys

import unreal

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import import_art_gap_assets as ia  # noqa: E402

REDO_DIR = os.path.join(ia.PROJECT_DIR, "Assets", "FBX", "concepts", "redo")
SHIPS = ["Gunship_02", "Escort_01", "Courier_01", "Smuggler_01", "Trader_01", "Freighter_01",
         "CargoFreighter_01", "Corvette_01", "Miner_01", "Destroyer_01", "HeavyHauler_01",
         "BulkCarrier_01", "Cruiser_01", "Battleship_01", "CommandXL_01"]
GAME_SHIPS = "/Game/Assets/Ships"
GAME_HULLS = {"SM_Ship_Cruiser_01_Assembled": GAME_SHIPS, "SM_Ship_CommandXL_01_Assembled": GAME_SHIPS}
UNIQUE_UV = ["SM_Ship_Corvette_01_Assembled_UniqueUV", "SM_Ship_Battleship_01_Assembled_UniqueUV"]
BP_OVERRIDES = {"/Game/Blueprints/Ships/BP_Ship_Corvette": "/Game/Materials/M_Corvette_Hull",
                "/Game/Blueprints/Ships/BP_Battleship": "/Game/Materials/M_Battleship_Hull"}
SMES = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)


def log(m):
    unreal.log("[ship-redo] " + str(m))


def import_to(name, dest):
    """ia.import_mesh with a one-off destination override."""
    if dest is None:
        return ia.import_mesh(name)
    ia.DESTINATIONS.insert(0, (name, dest))
    try:
        return ia.import_mesh(name)
    finally:
        ia.DESTINATIONS.pop(0)


def rebuild_collision(path):
    mesh = unreal.load_asset(path)
    before = ia.collision_summary(mesh)
    SMES.remove_collisions(mesh)
    SMES.set_convex_decomposition_collisions(mesh, 12, 24, 200000)
    unreal.EditorAssetLibrary.save_asset(path, only_if_is_dirty=False)
    log("   collision %s: %s -> %s" % (path.split("/")[-1], before, ia.collision_summary(mesh)))


def set_bp_override(bp_path, mat_path):
    bp = unreal.load_asset(bp_path)
    mat = unreal.load_asset(mat_path)
    cdo = unreal.get_default_object(bp.generated_class())
    cdo.set_editor_property("hull_material_override", mat)
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    unreal.EditorAssetLibrary.save_asset(bp_path, only_if_is_dirty=False)
    got = unreal.get_default_object(unreal.load_asset(bp_path).generated_class()).get_editor_property(
        "hull_material_override")
    log("   %s HullMaterialOverride -> %s" % (bp_path.split("/")[-1], got))


def main(argv):
    ia.DRY_RUN = "--dry-run" in argv
    ok, bad = 0, []
    hulls = []
    for ship in SHIPS:
        base = "SM_Ship_" + ship
        names = sorted(os.path.basename(f)[:-4] for f in glob.glob(os.path.join(REDO_DIR, base + "_*.fbx")))
        log("== %s: %d meshes" % (ship, len(names)))
        for n in names:
            dest = GAME_HULLS.get(n)
            if import_to(n, dest):
                ok += 1
                if n.endswith("_Assembled"):
                    hulls.append((dest or ia.destination_for(n)) + "/" + n)
            else:
                bad.append(n)
    for n in UNIQUE_UV:
        if import_to(n, None):
            ok += 1
            hulls.append(ia.destination_for(n) + "/" + n)
        else:
            bad.append(n)
    if not ia.DRY_RUN:
        for h in hulls:
            rebuild_collision(h)
        for bp, mat in BP_OVERRIDES.items():
            set_bp_override(bp, mat)
    log("DONE imported=%d failed=%s" % (ok, bad))


main(sys.argv[1:])
