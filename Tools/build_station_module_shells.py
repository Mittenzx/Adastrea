"""One-shot: build ONLY the Station Editor's shared module shells (Standard/
Large/ConnectorThin/SolarArray) + their PBR texture sets, without running the
full generate_adastrea_assets.py pipeline.

Run: blender -b --python Tools/build_station_module_shells.py
Output: Assets/FBX/generated/SM_StationModule_Shell_{Standard,Large,
        ConnectorThin,SolarArray}.fbx
        Assets/FBX/generated/Textures/T_StationModule_{Shell,SolarArray,
        Connector,Utility,Defence}_*.png (5 of the project's 5 station-module
        part_family textures now exist -- Module and Habitation covered by
        M_StationModule_Shell and the pre-existing M_Station_Hab)
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import generate_adastrea_assets as gen

gen.setup_scene()

# Neutral industrial grey-blue -- a sensible default for the "Module" family
# (the majority, 15/20 catalog types) since per-family recoloring is a
# follow-up pass, not blocking these shells from being usable now.
gen.gen_texture_set("StationModule_Shell",
    {'base': [0.36, 0.38, 0.42], 'accent': [0.18, 0.30, 0.40],
     'emissive': [0.25, 0.7, 1.0],
     'windows': {'cols': 10, 'frac': 0.35, 'cool': [0.35, 0.7, 1.0], 'warm': [1.0, 0.6, 0.3]},
     'grime': True, 'hazard': {'bands': 3}, 'cable': {'runs': 4},
     'rough': 0.42, 'metal': 0.9}, 2048, seed=210)

gen.gen_texture_set("StationModule_SolarArray",
    {'base': [0.08, 0.10, 0.22], 'accent': [0.55, 0.58, 0.65],
     'emissive': [0.15, 0.4, 1.0],
     'windows': {'cols': 6, 'frac': 0.1, 'cool': [0.2, 0.5, 1.0]},
     'grime': False, 'hazard': {'bands': 1}, 'rough': 0.2, 'metal': 0.3}, 1024, seed=211)

# Phase 1 per-family texture sets (2026-09-16 cross-session coordination --
# M_StationModule_Shell already covers "Module" (15/20 types) and
# M_Station_Hab already exists for "Habitation"; these 3 fill the remaining
# families: Connector (Corridor), Utility (FuelDepot), Defence (ShieldGenerator
# /Turret -- built anyway despite combat being deferred, since it's cheap
# alongside the other two and the mesh/material system doesn't care that the
# gameplay system consuming it is switched off).

# Connector: plain, clean transit corridor -- pale neutral grey, minimal
# grime/hazard (it's a walkway, not an industrial space), cool strip lighting.
gen.gen_texture_set("StationModule_Connector",
    {'base': [0.52, 0.52, 0.54], 'accent': [0.28, 0.30, 0.34],
     'emissive': [0.3, 0.75, 1.0],
     'windows': {'cols': 8, 'frac': 0.15, 'cool': [0.4, 0.8, 1.0]},
     'grime': False, 'hazard': {'bands': 1}, 'rough': 0.38, 'metal': 0.85}, 2048, seed=212)

# Utility: industrial tank/pipe aesthetic -- dusty brass/tan with
# copper-pipe accents and warm fuel-warning emissive, heavier grime/hazard
# striping (flammable-warning convention) than the general Module shell.
gen.gen_texture_set("StationModule_Utility",
    {'base': [0.40, 0.36, 0.28], 'accent': [0.52, 0.34, 0.15],
     'emissive': [1.0, 0.6, 0.2],
     'windows': {'cols': 6, 'frac': 0.15, 'cool': [0.4, 0.7, 1.0], 'warm': [1.0, 0.6, 0.2]},
     'grime': True, 'hazard': {'bands': 5}, 'cable': {'runs': 5},
     'rough': 0.5, 'metal': 0.8}, 2048, seed=213)

# Defence: dark armored gunmetal with red danger accents/emissive -- built
# per the shared plan even though combat is currently deferred (docs/
# 05-ROADMAP.md), since the asset cost here is negligible and the family
# would otherwise be the one gap in an already-complete 5-family texture set.
gen.gen_texture_set("StationModule_Defence",
    {'base': [0.20, 0.19, 0.21], 'accent': [0.68, 0.14, 0.10],
     'emissive': [1.0, 0.18, 0.12],
     'windows': {'cols': 6, 'frac': 0.1, 'cool': [0.3, 0.5, 1.0], 'warm': [1.0, 0.2, 0.15]},
     'grime': True, 'hazard': {'bands': 4}, 'rough': 0.3, 'metal': 0.95}, 2048, seed=214)

results = gen.build_station_module_shells()
for obj, out in results:
    print(f"BUILD_DONE: {out}")
