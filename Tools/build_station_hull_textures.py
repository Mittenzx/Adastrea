"""One-shot: generate dedicated hull texture sets for the 5 space station
archetypes that currently use flat placeholder colors with no textures
(Agricultural, Industrial, Research, Luxury, BlackMarket). Mirrors the
T_Station_Hab set already in production -- same gen_texture_set() pipeline,
same 2048px resolution, same _D/_N/_M/_R/_AO/_E(/_SKIN) map convention.

Each station mesh (SM_Station_<Name>_01, built by build_station_<name>() in
generate_adastrea_assets.py) is UV'd with smart_uv(), a world-aligned
triplanar projection tiled every 200cm -- the same technique used for every
ship hull in the fleet. That means these tileable texture sets need no
per-mesh custom UV work; they map cleanly onto any station hull geometry.

Run: blender -b --python Tools/build_station_hull_textures.py
Output: Assets/FBX/generated/Textures/T_Station_<Name>_*.png
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import generate_adastrea_assets as gen

gen.setup_scene()

# Agricultural: hydroponics/farming station -- greenhouse panels, plant-green
# accents, warm amber grow-light emissive (reads as interior grow-lamps
# glowing through greenhouse glass rather than cold sci-fi neon), organic/
# utilitarian paneling (fewer hazard stripes, more cable/irrigation runs).
gen.gen_texture_set("Station_Agricultural",
    {'base': [0.25, 0.55, 0.20], 'accent': [0.14, 0.32, 0.12],
     'emissive': [0.55, 1.0, 0.25],
     'windows': {'cols': 10, 'frac': 0.55, 'cool': [0.5, 0.9, 0.4], 'warm': [1.0, 0.85, 0.3]},
     'grime': True, 'cable': {'runs': 7}, 'detail': True,
     'rough': 0.48, 'metal': 0.75}, 2048, seed=201)

# Industrial: refinery/factory station -- weathered rust-brown metal, exposed
# pipework (dense cable/conduit runs), heavy hazard warning stripes, amber
# warning-light emissive for a heavy-industry read.
gen.gen_texture_set("Station_Industrial",
    {'base': [0.55, 0.32, 0.12], 'accent': [0.30, 0.16, 0.06],
     'emissive': [1.0, 0.55, 0.10],
     'windows': {'cols': 8, 'frac': 0.30, 'cool': [0.4, 0.6, 1.0], 'warm': [1.0, 0.55, 0.15]},
     'grime': True, 'hazard': {'bands': 6}, 'cable': {'runs': 9}, 'detail': True,
     'rough': 0.58, 'metal': 0.90}, 2048, seed=202)

# Research: clean scientific station -- white/blue paneling, tech greeble
# detailing, directional machined bevels for a "precision instrument" read,
# and glowing blue emissive accents for sensor windows/status lights.
gen.gen_texture_set("Station_Research",
    {'base': [0.55, 0.66, 0.70], 'accent': [0.12, 0.50, 0.65],
     'emissive': [0.2, 0.75, 1.0],
     'windows': {'cols': 16, 'frac': 0.60, 'cool': [0.3, 0.75, 1.0], 'warm': [0.6, 0.85, 1.0]},
     'neon': [0.2, 0.75, 1.0], 'neon_thick': 2, 'bevels': 4, 'detail': True,
     'rough': 0.24, 'metal': 0.82}, 2048, seed=203)

# Luxury: upscale resort/spire station -- polished gold/cream paneling, no
# grime/weathering (kept clean and premium), warm amber emissive window
# lighting for a "lit from within at night" penthouse look.
gen.gen_texture_set("Station_Luxury",
    {'base': [0.78, 0.68, 0.38], 'accent': [0.42, 0.34, 0.14],
     'emissive': [1.0, 0.80, 0.45],
     'windows': {'cols': 12, 'frac': 0.70, 'cool': [0.7, 0.7, 0.9], 'warm': [1.0, 0.82, 0.5]},
     'detail': True, 'id_marks': False,
     'rough': 0.16, 'metal': 0.94}, 2048, seed=204)

# BlackMarket: illicit outpost -- dark, grungy, mismatched patched-together
# paneling (heavy grime + dense cable clutter reads as jury-rigged), minimal
# window lighting, dim red emissive accents for a shady/low-visibility feel.
gen.gen_texture_set("Station_BlackMarket",
    {'base': [0.16, 0.10, 0.10], 'accent': [0.28, 0.06, 0.06],
     'emissive': [0.9, 0.10, 0.08],
     'windows': {'cols': 9, 'frac': 0.15, 'cool': [0.3, 0.5, 0.6], 'warm': [0.9, 0.3, 0.15]},
     'neon': [0.9, 0.10, 0.08], 'neon_thick': 2,
     'grime': True, 'cable': {'runs': 10}, 'detail': True,
     'rough': 0.62, 'metal': 0.68}, 2048, seed=205)

print("BUILD_DONE: T_Station_Agricultural_* + T_Station_Industrial_* + "
      "T_Station_Research_* + T_Station_Luxury_* + T_Station_BlackMarket_* texture sets")
