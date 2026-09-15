"""
One-shot: import the new fighter cabin v3 interior meshes (procedural zones +
the BlenderKit-sourced pilot seat) into the plugin, build any missing
M_Int_* kit materials, and point BP_Ship_Fighter at the new split-zone cabin
instead of the old v2 single-joined mesh.

Run via:  UnrealEditor.exe <proj>.uproject -ExecutePythonScript=<this file>
Self-contained (boot -> run -> exit). Do NOT leave an interactive editor open.
"""
import os
import sys
import unreal

sys.path.insert(0, os.path.join(unreal.Paths.project_dir(), "Plugins", "AdastreaShips"))
import import_interiors

BP_PATH = "/Game/Blueprints/Ships/BP_Ship_Fighter"
SHELL_MESH_PATH = "/AdastreaShips/Meshes/Interiors/SM_Int_Fighter_Cabin_Shell"


def main():
    unreal.log("=== Fighter cabin v3 wiring starting ===")

    # 1) Import meshes (procedural zones + kitbash Console) + build M_Int_* materials.
    import_interiors.main()

    # 2) Point BP_Ship_Fighter at the new split-zone cabin.
    bp = unreal.load_asset(BP_PATH)
    if bp is None:
        unreal.log_error(f"Could not load {BP_PATH}")
        return False
    cdo = unreal.get_default_object(bp.generated_class())

    shell_mesh = unreal.load_asset(SHELL_MESH_PATH)
    if shell_mesh is None:
        unreal.log_error(f"Could not load {SHELL_MESH_PATH} -- did the mesh import succeed?")
        return False

    cdo.set_editor_property("interior_shell_mesh", shell_mesh)
    cdo.set_editor_property("interior_family", unreal.ShipInteriorFamily.FIGHTER)

    readback_mesh = cdo.get_editor_property("interior_shell_mesh")
    readback_family = cdo.get_editor_property("interior_family")
    print(f"readback: interior_shell_mesh={readback_mesh}  interior_family={readback_family}")

    unreal.EditorAssetLibrary.save_asset(BP_PATH)
    print(f"saved {BP_PATH}")

    unreal.log("=== Fighter cabin v3 wiring complete ===")
    return True


try:
    ok = main()
    print("RESULT_OK" if ok else "RESULT_FAIL")
except Exception as e:
    import traceback
    unreal.log_error("Fighter cabin wiring crashed: %s" % e)
    unreal.log_error(traceback.format_exc())
    print("RESULT_FAIL")
# Deliberately no quit_editor() call: confirmed (both here and on the
# corvette bridge script) that UE 5.8 can crash on shutdown via
# quit_editor() regardless of success/failure (a ModeManagerInteractive
# ToolsContext teardown bug, not something in this script's control) --
# the actual save already completed synchronously above, so a crash here
# doesn't lose data, but it's still needless noise. Leave the editor open;
# close it manually or let the next launch replace it.
