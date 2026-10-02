"""Build a hand-authored sector level from Content/Data/Universe/SectorLayouts.json.

Run inside the editor (remote execution): exec this file with the sector id as argument,
e.g. "Tools/build_sector_level.py adastrea_relay". Optional: --dry-run (check only).

Creates the level if it doesn't exist (else opens it), removes everything a previous run placed
(actors tagged SectorKit), then places:
  - the space environment TestLevel uses: star dome, sun (coloured and angled from the system's
    star and the sector's orbit), dim sky light, fixed-exposure post process
  - the sector marker (ASpaceSectorMap with the sector id) and the AI ship populator
  - player start, stations (built module by module through build_level_stations), fields, props
It checks nothing sits within 400 m of where a jump gate will spawn, then saves the level.
Point the sector's "level" in Galaxy.json at the level afterwards (the script prints the path).
See docs/11-TECHNICAL_SPECS/GALAXY_PLAN.md, build order step 3.
"""
import json
import math
import os
import sys

import unreal

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import build_level_stations  # noqa: E402

ROOT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
TAG = "SectorKit"
GATE_DISTANCE = 200000.0     # UJumpGateWorldSubsystem / AJumpGate::AutoGateDistance
GATE_CLEARANCE = 40000.0     # keep content 400 m clear of a gate
PRIME_SUN = 32.0             # TestLevel's sun at orbitRadius 0.32
STATION_CLASS = "/Game/Blueprints/Stations/BP_SpaceStation.BP_SpaceStation_C"
SECTOR_MAP_CLASS = "/Game/DataAssets/Sectors/SpaceSectorMap_Blueprint.SpaceSectorMap_Blueprint_C"
MARKETS = "/Game/DataAssets/Trading/Markets"


def load_json(rel):
    with open(os.path.join(ROOT, rel), encoding="utf-8") as f:
        return json.load(f)


def vec(p):
    return unreal.Vector(float(p[0]), float(p[1]), float(p[2]))


def gate_spots(galaxy, sector_id):
    """Where UJumpGateWorldSubsystem::SpawnGates will put each gate (sector marker at the origin)."""
    sectors, system_of, system_pos = {}, {}, {}
    for s in galaxy["systems"]:
        system_pos[s["id"]] = s["position"]
        for x in s.get("sectors", []):
            sectors[x["id"]] = x
            system_of[x["id"]] = s["id"]
    dests = set(sectors[sector_id].get("gates", []) + sectors[sector_id].get("laneGates", []))
    dests |= {o for o, x in sectors.items() if sector_id in x.get("gates", []) + x.get("laneGates", [])}

    def orbit(x):
        a = math.radians(x.get("orbitAngle", 0.0))
        return (math.cos(a) * x.get("orbitRadius", 0.5), math.sin(a) * x.get("orbitRadius", 0.5))

    spots = {}
    for d in sorted(dests):
        if system_of[d] == system_of[sector_id]:
            a, b = orbit(sectors[d]), orbit(sectors[sector_id])
        else:
            a, b = system_pos[system_of[d]], system_pos[system_of[sector_id]]
        v = (a[0] - b[0], a[1] - b[1])
        n = math.hypot(*v) or 1.0
        spots[d] = (v[0] / n * GATE_DISTANCE, v[1] / n * GATE_DISTANCE, 0.0)
    return spots, sectors[sector_id], system_of[sector_id]


def check_clearance(layout, spots):
    problems = []
    items = [(s["layout"], s["pos"], 15000.0) for s in layout.get("stations", [])]
    items += [(f["label"], f["pos"], float(f.get("radius", 20000))) for f in layout.get("fields", [])]
    items += [(p["label"], p["pos"], 5000.0) for p in layout.get("props", [])]
    for name, pos, radius in items:
        for dest, g in spots.items():
            gap = math.dist(pos, g) - radius
            if gap < GATE_CLEARANCE:
                problems.append(f"{name} is {gap / 100:.0f} m from the gate to {dest} (needs {GATE_CLEARANCE / 100:.0f} m)")
    return problems


def tagged(actor):
    actor.tags = list(actor.tags) + [TAG]
    return actor


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    dry = "--dry-run" in sys.argv
    if not args:
        unreal.log_error("build_sector_level: pass a sector id")
        return
    sector_id = args[0]
    layouts = load_json("Content/Data/Universe/SectorLayouts.json")["sectors"]
    galaxy = load_json("Content/Data/Universe/Galaxy.json")
    layout = layouts.get(sector_id)
    if not layout:
        unreal.log_error(f"build_sector_level: no layout for {sector_id} in SectorLayouts.json")
        return
    spots, sector, system_id = gate_spots(galaxy, sector_id)
    system = next(s for s in galaxy["systems"] if s["id"] == system_id)
    for dest, g in spots.items():
        unreal.log(f"build_sector_level: gate to {dest} will be at ({g[0]:.0f}, {g[1]:.0f})")
    problems = check_clearance(layout, spots)
    for p in problems:
        unreal.log_error("build_sector_level: " + p)
    if problems or dry:
        unreal.log(f"build_sector_level: {'dry run' if dry else 'stopped'}: {len(problems)} problem(s)")
        return

    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    eal = unreal.EditorAssetLibrary
    level = layout["level"]
    if eal.does_asset_exist(level):
        les.load_level(level)
    else:
        les.new_level(level)

    # Remove what a previous run placed (station modules are attached to tagged stations).
    for a in eas.get_all_level_actors():
        if TAG in [str(t) for t in a.tags]:
            for child in a.get_attached_actors():
                eas.destroy_actor(child)
            eas.destroy_actor(a)

    # --- Environment (TestLevel's recipe) ---
    dome = tagged(eas.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, 0)))
    dome.set_actor_label("StarfieldDome")
    dome.static_mesh_component.set_static_mesh(unreal.load_asset("/Game/Meshes/Environment/SM_StarDome_Dense"))
    dome.static_mesh_component.set_material(0, unreal.load_asset("/Game/Materials/M_Starfield_Real"))
    dome.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    dome.static_mesh_component.set_editor_property("cast_shadow", False)
    dome.set_actor_scale3d(unreal.Vector(100000, 100000, 100000))

    sun_cfg = layout.get("sun", {})
    r = max(float(sector.get("orbitRadius", 0.5)), 0.1)
    intensity = float(sun_cfg.get("intensity", max(10.0, min(40.0, PRIME_SUN * 0.32 / r))))
    # The star sits toward the system centre: light travels outward along the sector's orbit bearing.
    yaw = float(sun_cfg.get("yaw", sector.get("orbitAngle", 0.0)))
    pitch = float(sun_cfg.get("pitch", -30.0))
    sun = tagged(eas.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 0), unreal.Rotator(roll=0, pitch=pitch, yaw=yaw)))
    sun.set_actor_label("Sun")
    c = system.get("starColor", [1.0, 0.9, 0.75])
    sun.light_component.set_editor_property("intensity", intensity)
    sun.light_component.set_light_color(unreal.LinearColor(c[0], c[1], c[2], 1.0))
    sky = tagged(eas.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 0)))
    sky.set_actor_label("SkyLight_SpaceFill")
    sky.light_component.set_editor_property("intensity", 0.4)
    ppv = tagged(eas.spawn_actor_from_class(unreal.PostProcessVolume, unreal.Vector(0, 0, 0)))
    ppv.set_actor_label("PPV_SpaceExposure")
    ppv.set_editor_property("unbound", True)
    settings = ppv.get_editor_property("settings")
    settings.set_editor_property("override_auto_exposure_bias", True)
    settings.set_editor_property("auto_exposure_bias", -3.0)
    ppv.set_editor_property("settings", settings)

    # --- Sector marker, AI traffic, player start ---
    marker = tagged(eas.spawn_actor_from_class(unreal.load_class(None, SECTOR_MAP_CLASS), unreal.Vector(0, 0, 0)))
    marker.set_actor_label("SectorMap_" + sector_id)
    marker.set_editor_property("sector_id", sector_id)
    marker.set_editor_property("sector_name", sector["name"])
    marker.set_editor_property("description", sector.get("description", ""))
    tagged(eas.spawn_actor_from_class(unreal.AIShipPopulator, unreal.Vector(0, 0, 0))).set_actor_label("AIShipPopulator")
    start = tagged(eas.spawn_actor_from_class(unreal.PlayerStart, vec(layout.get("playerStart", [0, 0, 3000]))))
    start.set_actor_label("PlayerStart")

    # --- Stations ---
    labels = []
    for st in layout.get("stations", []):
        a = tagged(eas.spawn_actor_from_class(unreal.load_class(None, STATION_CLASS), vec(st["pos"])))
        a.set_actor_label(st["layout"])
        a.set_editor_property("station_name", st["name"])
        if st.get("market"):
            a.set_editor_property("station_market", unreal.load_asset(f"{MARKETS}/{st['market']}"))
        labels.append(st["layout"])
    if labels:
        build_level_stations.main(labels=labels, save=False, dry_run=False)

    # --- Fields ---
    for f in layout.get("fields", []):
        a = tagged(eas.spawn_actor_from_class(unreal.AsteroidField, vec(f["pos"])))
        a.set_actor_label(f["label"])
        a.set_editor_property("asteroid_count", int(f.get("count", 60)))
        a.set_editor_property("field_radius", float(f.get("radius", 25000)))
        a.set_editor_property("vertical_fraction", float(f.get("vertical", 0.35)))
        a.set_editor_property("global_scale", float(f.get("scale", 10)))
        a.set_editor_property("seed", int(f.get("seed", 1337)))
        a.set_editor_property("ores", f["ores"])
        a.resolve_ore_types()
        a.regenerate()
        types = [t.get_name() for t in a.get_editor_property("asteroid_types")]
        unreal.log(f"build_sector_level: field {f['label']}: {a.get_rock_count()} rocks of {types}")

    # --- Props ---
    for p in layout.get("props", []):
        a = tagged(eas.spawn_actor_from_class(unreal.StaticMeshActor, vec(p["pos"]),
                                              unreal.Rotator(roll=p.get("rot", [0, 0, 0])[2], pitch=p.get("rot", [0, 0, 0])[0],
                                                             yaw=p.get("rot", [0, 0, 0])[1])))
        a.set_actor_label(p["label"])
        a.static_mesh_component.set_static_mesh(unreal.load_asset(p["mesh"]))
        a.set_actor_scale3d(unreal.Vector(p.get("scale", 1), p.get("scale", 1), p.get("scale", 1)))
        if p.get("hidden"):
            a.tags = list(a.tags) + ["HiddenPOI"]
        if p.get("poi"):
            a.tags = list(a.tags) + ["POI_" + p["poi"]]

    les.save_current_level()
    unreal.log(f"build_sector_level: built {sector_id} into {level}. Set its \"level\" in Galaxy.json to {level}.")


main()
