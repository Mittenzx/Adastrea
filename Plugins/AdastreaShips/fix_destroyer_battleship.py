"""
One-shot: fix two real fleet bugs found by audit_all_ships.py (2026-09-15):
  - BP_Ship_Destroyer's ShipMeshComponent pointed at SM_Ship_Miner_01_Assembled
    (the MINER's mesh) instead of its own already-generated
    SM_Ship_Destroyer_01_Assembled.
  - BP_Battleship's ShipMeshComponent.StaticMesh was None (invisible ship)
    despite SM_Ship_Battleship_01_Assembled existing on disk, generated but
    never wired up.
Also builds the new dedicated M_Destroyer_Hull / M_Battleship_Hull materials
(Tools/build_fleet_hull_textures.py already generated the T_Destroyer_*/
T_Battleship_* source textures) via ue_hull_tex.py, which the rebuilt
Spaceship.cpp::ApplyShipHullMaterial now maps to at runtime by actor name.

Run via:  UnrealEditor.exe <proj>.uproject -ExecutePythonScript=<this file>
Leaves the editor open when done (see wire_corvette_bridge.py's note on why
quit_editor() is no longer called from these one-shots).
"""
import os
import sys
import unreal

PLUGIN_DIR = os.path.join(unreal.Paths.project_dir(), "Plugins", "AdastreaShips")
sys.path.insert(0, PLUGIN_DIR)

MESH_ROOT = "/AdastreaShips/Meshes/Ships"

FIXES = [
    ("/Game/Blueprints/Ships/BP_Ship_Destroyer", f"{MESH_ROOT}/SM_Ship_Destroyer_01_Assembled"),
    ("/Game/Blueprints/Ships/BP_Battleship", f"{MESH_ROOT}/SM_Ship_Battleship_01_Assembled"),
]


def run_script(path, name):
    """Exec a standalone editor-Python script's source and explicitly call
    its main() -- these scripts guard main() behind `if __name__ ==
    "__main__"`, which never fires under exec() with a non-"__main__" name,
    so retrieve and call it directly (same pattern as this project's own
    Plugins/AdastreaShips/run_importer_startup.py)."""
    with open(path, encoding="utf-8") as f:
        code = f.read()
    g = {"__name__": name, "unreal": unreal}
    exec(compile(code, path, "exec"), g)
    main_fn = g.get("main")
    if main_fn:
        main_fn()


def main():
    # 1) Import the mesh assets themselves -- audit found SM_Ship_Destroyer_01_
    # Assembled.fbx and SM_Ship_Battleship_01_Assembled.fbx exist on disk
    # (generated) but were never materialized into the plugin content at all
    # (same gap as SM_Int_Fighter_Cabin was before this session's earlier fix).
    import_path = os.path.normpath(os.path.join(PLUGIN_DIR, "ImportAdastreaShips.py"))
    run_script(import_path, "adastrea_import")

    # 2) Rebuild hull materials from the new T_Destroyer_*/T_Battleship_* textures.
    hull_tex_path = os.path.normpath(os.path.join(PLUGIN_DIR, "..", "..", "Tools", "ue_hull_tex.py"))
    run_script(hull_tex_path, "ue_hull_tex")

    # 3) Fix the two mesh assignments.
    ok = True
    for bp_path, mesh_path in FIXES:
        bp = unreal.load_asset(bp_path)
        if bp is None:
            print(f"FAIL: could not load {bp_path}")
            ok = False
            continue
        mesh = unreal.load_asset(mesh_path)
        if mesh is None:
            print(f"FAIL: could not load {mesh_path}")
            ok = False
            continue
        cdo = unreal.get_default_object(bp.generated_class())
        mesh_comp = cdo.get_editor_property("ship_mesh_component")
        if mesh_comp is None:
            print(f"FAIL: {bp_path} has no ship_mesh_component")
            ok = False
            continue
        mesh_comp.set_editor_property("static_mesh", mesh)
        unreal.EditorAssetLibrary.save_asset(bp_path)
        readback = mesh_comp.static_mesh
        print(f"{bp_path}: static_mesh now = {readback.get_name() if readback else None}")
        if readback is None or readback.get_name() != mesh.get_name():
            ok = False

    print("RESULT_OK" if ok else "RESULT_FAIL")


try:
    main()
except Exception as e:
    import traceback
    unreal.log_error("fix_destroyer_battleship crashed: %s" % e)
    unreal.log_error(traceback.format_exc())
    print("RESULT_FAIL")
