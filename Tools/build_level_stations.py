"""Rebuild the level's space stations from Content/Data/LevelStationLayouts.json.

Runs inside the Unreal Editor (Python remote execution or the Output Log's Python
console) with the level open. For each station in the layout file whose actor
label matches a level ASpaceStation:
  1. destroys the modules currently attached to it,
  2. clears the station actor's own placeholder hull mesh (the core carries it now),
  3. spawns the core and every module where the Station Editor would put them
     (Tools/station_layouts.py), attached to the station.
Then it saves the level. Modules are ordinary level actors, not PlayerBuilt, so the
save system leaves them alone. The station's saved Modules array may still name the
destroyed modules; ASpaceStation::BeginPlay drops those and discovers the new ones.

Usage: exec the file in the editor, optionally with --dry-run to only log.
"""
import os
import sys

import unreal

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import station_layouts  # noqa: E402

DRY_RUN = "--dry-run" in sys.argv


def main():
    stations, cell, errors = station_layouts.load_all()
    if errors:
        for e in errors:
            unreal.log_error("build_level_stations: " + e)
        return

    actor_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    level_stations = {
        a.get_actor_label(): a
        for a in actor_sub.get_all_level_actors()
        if isinstance(a, unreal.SpaceStation)
    }

    for label, modules in stations.items():
        station = level_stations.get(label)
        if station is None:
            unreal.log_warning(f"build_level_stations: no station labelled {label} in this level")
            continue

        old = [m for m in station.get_attached_actors() if isinstance(m, unreal.SpaceStationModule)]
        unreal.log(f"build_level_stations: {label}: replacing {len(old)} module(s) with {len(modules)}")
        if DRY_RUN:
            continue

        for m in old:
            actor_sub.destroy_actor(m)
        for comp in station.get_components_by_class(unreal.StaticMeshComponent):
            comp.set_static_mesh(None)

        origin = station.get_actor_location()
        for m in modules:
            cls = unreal.load_class(None, f"/Script/Adastrea.{m.item}")
            if cls is None:
                unreal.log_error(f"build_level_stations: {label}: no class {m.item}")
                continue
            loc = unreal.Vector(origin.x + m.pos[0], origin.y + m.pos[1], origin.z + m.pos[2])
            rot = unreal.Rotator(roll=0.0, pitch=0.0, yaw=float(m.rot))
            actor = actor_sub.spawn_actor_from_class(cls, loc, rot)
            actor.attach_to_actor(station, "", unreal.AttachmentRule.KEEP_WORLD,
                                  unreal.AttachmentRule.KEEP_WORLD, unreal.AttachmentRule.KEEP_WORLD, False)
            actor.set_actor_label(f"{label}_{m.id}")

    if not DRY_RUN:
        unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
        unreal.log("build_level_stations: level saved")


main()
