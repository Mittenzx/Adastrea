"""One-off, EDITOR-ONLY (not saved to the level): spawn all 5 station module
shell/material combos as temporary StaticMeshActors near the TestLevel's
PlayerStart so they can be seen in PIE for a visual check. Does NOT save the
level -- these actors exist only in the current in-memory editor world and
will vanish next time the level is loaded fresh, so this is safe to leave
running without polluting the actual level asset.
"""
import unreal

MESH_ROOT = "/AdastreaShips/Meshes/Station"
MAT_ROOT = "/Game/Materials"

# (mesh, material, offset) -- spaced out along X so all 5 are visible/walkable
PLACEMENTS = [
    ("SM_StationModule_Shell_Standard", "M_StationModule_Shell", unreal.Vector(0, 0, 0)),
    ("SM_StationModule_Shell_Standard", "M_StationModule_Connector", unreal.Vector(1000, 0, 0)),
    ("SM_StationModule_Shell_Standard", "M_StationModule_Utility", unreal.Vector(2000, 0, 0)),
    ("SM_StationModule_Shell_Standard", "M_StationModule_Defence", unreal.Vector(3000, 0, 0)),
    ("SM_StationModule_Shell_Large", "M_StationModule_Shell", unreal.Vector(4200, 0, 0)),
    ("SM_StationModule_Shell_ConnectorThin", "M_StationModule_Connector", unreal.Vector(5400, 0, 0)),
    ("SM_StationModule_Shell_SolarArray", "M_StationModule_SolarArray", unreal.Vector(6200, 0, 0)),
]

# PlayerStart-relative base: place near world origin's floor, camera-visible.
BASE = unreal.Vector(2000, 0, 500)


def main():
    subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    spawned = []
    for mesh_name, mat_name, offset in PLACEMENTS:
        mesh = unreal.load_asset(f"{MESH_ROOT}/{mesh_name}")
        mat = unreal.load_asset(f"{MAT_ROOT}/{mat_name}")
        if mesh is None:
            print(f"MISSING mesh {mesh_name}")
            continue
        actor = subsystem.spawn_actor_from_class(unreal.StaticMeshActor, BASE + offset)
        comp = actor.static_mesh_component
        comp.set_static_mesh(mesh)
        if mat is not None:
            comp.set_material(0, mat)
        actor.set_actor_label(f"PreviewShell_{mesh_name}_{mat_name}")
        spawned.append(actor)
        print(f"spawned {mesh_name} + {mat_name} at {BASE + offset}")

    print(f"RESULT_OK spawned={len(spawned)}")

    # Start PIE in-process so the editor doesn't just exit when this script's
    # main() returns (observed: -ExecutePythonScript closes the editor once
    # the script finishes UNLESS something -- like entering PIE -- keeps it
    # busy). Try the current subsystem API first, fall back to the
    # deprecated static one for whichever actually exists in this UE build.
    try:
        les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        les.editor_play_simulate()
        print("PIE_STARTED via LevelEditorSubsystem")
    except Exception as e1:
        try:
            unreal.EditorLevelLibrary.editor_play_simulate()
            print("PIE_STARTED via EditorLevelLibrary (deprecated API)")
        except Exception as e2:
            print(f"PIE_START_FAILED: {e1} / {e2}")


try:
    main()
except Exception as e:
    import traceback
    unreal.log_error("place_station_shells_preview crashed: %s" % e)
    unreal.log_error(traceback.format_exc())
    print("RESULT_FAIL")
