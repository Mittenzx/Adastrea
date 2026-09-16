"""One-shot: build ONLY the Station Editor's shared module shells (Standard/
Large/ConnectorThin/SolarArray) + their PBR texture sets, without running the
full generate_adastrea_assets.py pipeline.

Run: blender -b --python Tools/build_station_module_shells.py
Output: Assets/FBX/generated/SM_StationModule_Shell_{Standard,Large,
        ConnectorThin,SolarArray}.fbx
        Assets/FBX/generated/Textures/T_StationModule_{Shell,SolarArray}_*.png
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

results = gen.build_station_module_shells()
for obj, out in results:
    print(f"BUILD_DONE: {out}")
