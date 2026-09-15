"""
One-shot: import the new corvette bridge interior meshes (procedural zones +
the BlenderKit-sourced Console) into the plugin, build any missing M_Int_*
kit materials, and point BP_Ship_Corvette at the new bespoke bridge instead
of the shared SM_Int_CommandBridge_* stopgap.

Run via:  UnrealEditor.exe <proj>.uproject -ExecutePythonScript=<this file>
Self-contained (boot -> run -> exit). Do NOT leave an interactive editor open.
"""
import os
import sys
import unreal

sys.path.insert(0, os.path.join(unreal.Paths.project_dir(), "Plugins", "AdastreaShips"))
import import_interiors

BP_PATH = "/Game/Blueprints/Ships/BP_Ship_Corvette"
SHELL_MESH_PATH = "/AdastreaShips/Meshes/Interiors/SM_Int_Corvette_Bridge_Shell"


def main():
    unreal.log("=== Corvette bridge wiring starting ===")

    # 1) Import meshes (procedural zones + kitbash Console) + build M_Int_* materials.
    import_interiors.main()

    # 2) Point BP_Ship_Corvette at the new bespoke bridge.
    bp = unreal.load_asset(BP_PATH)
    if bp is None:
        unreal.log_error(f"Could not load {BP_PATH}")
        return False
    gen_class = bp.generated_class()
    cdo = unreal.get_default_object(gen_class)

    shell_mesh = unreal.load_asset(SHELL_MESH_PATH)
    if shell_mesh is None:
        unreal.log_error(f"Could not load {SHELL_MESH_PATH} -- did the mesh import succeed?")
        return False

    # UE's Python bindings strip the Hungarian "E" prefix from UENUM types
    # (EShipInteriorFamily -> unreal.ShipInteriorFamily) and expose members in
    # SCREAMING_SNAKE_CASE (CorvetteBridge -> CORVETTE_BRIDGE) -- confirmed via
    # Plugins/AdastreaShips/diag_enum.py rather than guessed.
    enum_members = [m for m in dir(unreal.ShipInteriorFamily) if not m.startswith("_")]
    print("ShipInteriorFamily members:", enum_members)
    target = next((m for m in enum_members if "CORVETTE" in m.upper()), None)
    if target is None:
        unreal.log_error("Could not find a CorvetteBridge-like member on ShipInteriorFamily "
                          f"(saw: {enum_members}) -- rebuild may not have picked up the new enum value")
        return False
    family_value = getattr(unreal.ShipInteriorFamily, target)

    cdo.set_editor_property("interior_shell_mesh", shell_mesh)
    cdo.set_editor_property("interior_family", family_value)

    # Verify readback before saving.
    readback_mesh = cdo.get_editor_property("interior_shell_mesh")
    readback_family = cdo.get_editor_property("interior_family")
    print(f"readback: interior_shell_mesh={readback_mesh}  interior_family={readback_family}")

    unreal.EditorAssetLibrary.save_asset(BP_PATH)
    saved_ok = not unreal.EditorAssetLibrary.does_asset_need_load(BP_PATH)
    print(f"saved {BP_PATH}: ok={saved_ok}")

    unreal.log("=== Corvette bridge wiring complete ===")
    return True


try:
    ok = main()
    print("RESULT_OK" if ok else "RESULT_FAIL")
    # Update 2026-09-14: originally this only skipped quit_editor() on
    # failure (see below), on the theory the crash was tied to quitting from
    # a failed script's error path. Turned out wrong -- the fighter cabin
    # wiring script hit the exact same ModeManagerInteractiveToolsContext
    # teardown crash on a SUCCESSFUL run too. So: never call quit_editor()
    # from either path. The actual save already completed synchronously
    # above regardless of what happens next, so this is just noise
    # avoidance, not a data-integrity fix. Leave the editor open; close it
    # manually or let the next launch replace it.
    #
    # (Original reasoning, kept for context: adastrea-1e traced a real crash
    # dump to quit_editor() being called unconditionally in a finally block
    # including on failure -- that was A cause, just not the only one.)
except Exception as e:
    import traceback
    unreal.log_error("Corvette bridge wiring crashed: %s" % e)
    unreal.log_error(traceback.format_exc())
    print("RESULT_FAIL")
