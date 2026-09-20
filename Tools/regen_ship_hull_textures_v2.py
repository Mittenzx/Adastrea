"""One-shot: quality pass #2 on ship hull textures (2026-09-18).

Context: user reported ship skins look like "simple placeholders" -- flat
albedo, low contrast, no weathering. `gen_texture_set()` in
generate_adastrea_assets.py already got the quality-knob bump (deeper relief
contrast, stronger per-plate tone variance, deeper AO, finer normal grain --
see that file's diffs). This script:

  1. Re-runs every EXISTING dedicated ship-hull texture set through the
     improved generator (same base/accent/emissive palettes -- those were
     already vision-verified per-class identity from prior sessions, so we
     do NOT change the colors, only pick up the sharper generator).
  2. Opts those hull sets into the Phase-3 dirt/streak weathering pass
     locally (`gen.WEATHERING = True` for THIS script's run only -- the
     shared module constant stays False so any other script/run of
     generate_adastrea_assets.py keeps today's "shiny new ships" default).
  3. Adds NEW dedicated hull texture sets for ship classes that were still
     falling back to the generic T_Ship_Hull/T_Fighter_Demo look: Fighter
     (the real SM_Ship_Fighter_01 hides behind M_Fighter_Hull -> Ship_Hull
     today), Carrier, Command, Frigate, Cruiser, Luxury, Patrol, Science,
     Trading, Transport (Behemoth + Genesis), Utility, Mining -- cross
     referenced against Content/DataAssets/Ships/*.uasset.

Run: blender -b --python Tools/regen_ship_hull_textures_v2.py
Output: Assets/FBX/generated/Textures/T_<Class>_*.png (D/N/R/M/AO/E/SKIN)
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import generate_adastrea_assets as gen

gen.setup_scene()
gen.WEATHERING = True  # opt this run into the Phase-3 dirt/streak pass

print("== Re-generating EXISTING dedicated hull sets w/ sharper generator ==")

gen.gen_texture_set("Ship_Hull", {'base': [0.5, 0.56, 0.62], 'accent': [0.1, 0.22, 0.35],
                                   'emissive': [0.2, 0.65, 1.0],
                                   'windows': {'cols': 12, 'frac': 0.4, 'cool': [0.3, 0.7, 1.0], 'warm': [1.0, 0.6, 0.25]},
                                   'cable': {'runs': 4}, 'grime': True, 'hazard': {'bands': 2},
                                   'rough': 0.30, 'metal': 0.97}, 2048, seed=11)

gen.gen_texture_set("Fighter_Demo", {'base': [0.52, 0.56, 0.60], 'accent': [0.12, 0.18, 0.24],
                                      'emissive': [0.25, 0.75, 1.0],
                                      'windows': {'cols': 12, 'frac': 0.45, 'cool': [0.3, 0.7, 1.0], 'warm': [1.0, 0.6, 0.25]},
                                      'bevels': 4, 'grime': True,
                                      'rough': 0.28, 'metal': 0.97}, 2048, seed=9101)

gen.gen_texture_set("Freighter", {'base': [0.45, 0.55, 0.45], 'accent': [0.15, 0.35, 0.18], 'emissive': [0.4, 0.8, 0.3],
                                   'windows': {'cols': 10, 'frac': 0.5, 'cool': [0.3, 0.7, 1.0], 'warm': [0.5, 0.9, 0.4]},
                                   'grime': True, 'hazard': {'bands': 3}, 'rough': 0.45, 'metal': 0.92}, 2048, seed=22)

gen.gen_texture_set("Gunship", {'base': [0.22, 0.23, 0.26], 'accent': [0.4, 0.05, 0.05],
                                 'emissive': [1.0, 0.15, 0.05], 'neon': [1.0, 0.2, 0.1], 'neon_thick': 3,
                                 'windows': {'cols': 16, 'frac': 0.2, 'cool': [0.3, 0.7, 1.0]},
                                 'grime': True, 'hazard': {'bands': 3}, 'rough': 0.40, 'metal': 0.94}, 2048, seed=55)

gen.gen_texture_set("Corvette", {'base': [0.4, 0.34, 0.46], 'accent': [0.25, 0.15, 0.4], 'emissive': [0.6, 0.3, 1.0],
                                  'windows': {'cols': 14, 'frac': 0.45, 'cool': [0.6, 0.5, 1.0], 'warm': [1.0, 0.5, 0.7]},
                                  'grime': True, 'hazard': {'bands': 3}, 'cable': {'runs': 5}, 'rough': 0.32, 'metal': 0.96}, 2048, seed=77)

gen.gen_texture_set("Miner", {'base': [0.55, 0.47, 0.4], 'accent': [0.4, 0.2, 0.1], 'emissive': [1.0, 0.5, 0.1],
                               'windows': {'cols': 10, 'frac': 0.3, 'cool': [0.5, 0.7, 1.0], 'warm': [1.0, 0.6, 0.3]},
                               'drill_accent': True, 'grime': True, 'hazard': {'bands': 4}, 'cable': {'runs': 6},
                               'rough': 0.52, 'metal': 0.88}, 2048, seed=88)

gen.gen_texture_set("Destroyer", {'base': [0.30, 0.32, 0.36], 'accent': [0.42, 0.26, 0.08],
                                   'emissive': [1.0, 0.55, 0.15],
                                   'windows': {'cols': 12, 'frac': 0.3, 'cool': [0.4, 0.7, 1.0], 'warm': [1.0, 0.6, 0.2]},
                                   'grime': True, 'hazard': {'bands': 4}, 'cable': {'runs': 4},
                                   'rough': 0.36, 'metal': 0.95}, 2048, seed=133)

gen.gen_texture_set("Battleship", {'base': [0.16, 0.17, 0.20], 'accent': [0.68, 0.70, 0.74],
                                    'emissive': [0.2, 0.9, 1.0],
                                    'windows': {'cols': 18, 'frac': 0.55, 'cool': [0.3, 0.75, 1.0], 'warm': [1.0, 0.65, 0.3]},
                                    'grime': True, 'hazard': {'bands': 5}, 'cable': {'runs': 6},
                                    'rough': 0.34, 'metal': 0.96}, 2048, seed=144)

print("== Generating NEW dedicated hull sets (were on generic fallback) ==")

# Fighter: the REAL SM_Ship_Fighter_01 (M_Fighter_Hull) currently maps to the
# generic Ship_Hull texture in MAT_TO_TEX -- give it its own sleek interceptor
# identity distinct from both Ship_Hull (generic) and Fighter_Demo (research demo).
gen.gen_texture_set("Fighter", {'base': [0.60, 0.64, 0.68], 'accent': [0.12, 0.34, 0.55],
                                 'emissive': [0.25, 0.8, 1.0],
                                 'windows': {'cols': 10, 'frac': 0.4, 'cool': [0.3, 0.75, 1.0], 'warm': [1.0, 0.6, 0.3]},
                                 'grime': True, 'hazard': {'bands': 2}, 'bevels': 3,
                                 'rough': 0.24, 'metal': 0.96}, 2048, seed=201)

# Carrier (DA_Carrier_VanguardEscort): naval-grey capital hull w/ yellow deck-crew markings
gen.gen_texture_set("Carrier", {'base': [0.34, 0.36, 0.34], 'accent': [0.55, 0.50, 0.15],
                                 'emissive': [1.0, 0.75, 0.2],
                                 'windows': {'cols': 16, 'frac': 0.5, 'cool': [0.3, 0.7, 1.0], 'warm': [1.0, 0.65, 0.25]},
                                 'grime': True, 'hazard': {'bands': 5}, 'cable': {'runs': 6},
                                 'rough': 0.42, 'metal': 0.90}, 2048, seed=202)

# Command (DA_Command_Sovereign): flagship -- deep navy/indigo w/ gold trim
gen.gen_texture_set("Command", {'base': [0.18, 0.20, 0.30], 'accent': [0.55, 0.45, 0.15],
                                 'emissive': [1.0, 0.85, 0.4],
                                 'windows': {'cols': 16, 'frac': 0.6, 'cool': [0.4, 0.7, 1.0], 'warm': [1.0, 0.75, 0.35]},
                                 'grime': True, 'hazard': {'bands': 2}, 'cable': {'runs': 4},
                                 'rough': 0.28, 'metal': 0.97}, 2048, seed=203)

# Frigate (DA_Frigate_ShadowbladeStealth): stealth -- near-black matte, radar-absorbent (low metal)
gen.gen_texture_set("Frigate", {'base': [0.10, 0.10, 0.13], 'accent': [0.25, 0.12, 0.30],
                                 'emissive': [0.35, 0.15, 0.55],
                                 'windows': {'cols': 10, 'frac': 0.18, 'cool': [0.35, 0.2, 0.6]},
                                 'grime': True, 'hazard': {'bands': 1}, 'bevels': 3,
                                 'rough': 0.55, 'metal': 0.55}, 2048, seed=204)

# Cruiser (DA_Cruiser_LifelineMedical): hospital ship -- clean white w/ red-cross accent (NO grime)
gen.gen_texture_set("Cruiser", {'base': [0.72, 0.74, 0.76], 'accent': [0.75, 0.12, 0.12],
                                 'emissive': [0.3, 0.9, 0.5],
                                 'windows': {'cols': 12, 'frac': 0.55, 'cool': [0.3, 0.8, 0.6], 'warm': [1.0, 0.9, 0.85]},
                                 'hazard': {'bands': 2}, 'rough': 0.30, 'metal': 0.70}, 2048, seed=205)

# Luxury (DA_Luxury_Starliner): pearlescent white/gold cruise liner, polished (NO grime)
gen.gen_texture_set("Luxury", {'base': [0.80, 0.78, 0.72], 'accent': [0.65, 0.55, 0.25],
                                'emissive': [1.0, 0.85, 0.55],
                                'windows': {'cols': 20, 'frac': 0.7, 'warm': [1.0, 0.85, 0.55], 'cool': [0.5, 0.75, 1.0]},
                                'rough': 0.18, 'metal': 0.85}, 2048, seed=206)

# Patrol (DA_Patrol_SentinelLaw): law-enforcement -- white/blue livery
gen.gen_texture_set("Patrol", {'base': [0.55, 0.58, 0.62], 'accent': [0.15, 0.35, 0.65],
                                'emissive': [0.3, 0.55, 1.0],
                                'windows': {'cols': 12, 'frac': 0.4, 'cool': [0.3, 0.5, 1.0], 'warm': [1.0, 0.3, 0.25]},
                                'grime': True, 'hazard': {'bands': 3}, 'rough': 0.32, 'metal': 0.90}, 2048, seed=207)

# Science (DA_Science_Odyssey): research vessel -- white/teal, sensor-cabled (NO grime)
gen.gen_texture_set("Science", {'base': [0.58, 0.62, 0.60], 'accent': [0.10, 0.45, 0.40],
                                 'emissive': [0.2, 0.9, 0.85],
                                 'windows': {'cols': 14, 'frac': 0.5, 'cool': [0.3, 0.9, 0.85], 'warm': [1.0, 0.7, 0.4]},
                                 'cable': {'runs': 6}, 'rough': 0.30, 'metal': 0.82}, 2048, seed=208)

# Trading (DA_Trading_Merchant): trade hauler -- warm brass/amber industrial
gen.gen_texture_set("Trading", {'base': [0.42, 0.38, 0.30], 'accent': [0.55, 0.35, 0.10],
                                 'emissive': [1.0, 0.65, 0.2],
                                 'windows': {'cols': 10, 'frac': 0.45, 'warm': [1.0, 0.65, 0.25], 'cool': [0.4, 0.7, 1.0]},
                                 'grime': True, 'hazard': {'bands': 2}, 'rough': 0.40, 'metal': 0.85}, 2048, seed=209)

# Transport_Behemoth (DA_Transport_BehemothFreighter): massive hauler, heavy grey-green industrial
gen.gen_texture_set("Transport_Behemoth", {'base': [0.32, 0.36, 0.30], 'accent': [0.12, 0.28, 0.15],
                                            'emissive': [0.3, 0.9, 0.35],
                                            'windows': {'cols': 8, 'frac': 0.35, 'cool': [0.3, 0.7, 1.0], 'warm': [0.6, 0.9, 0.4]},
                                            'grime': True, 'hazard': {'bands': 4}, 'cable': {'runs': 8},
                                            'rough': 0.50, 'metal': 0.88}, 2048, seed=210)

# Transport_Genesis (DA_Transport_GenesisColony): colony transport -- warm sandy/tan colonial
gen.gen_texture_set("Transport_Genesis", {'base': [0.55, 0.48, 0.38], 'accent': [0.30, 0.22, 0.12],
                                           'emissive': [1.0, 0.75, 0.4],
                                           'windows': {'cols': 14, 'frac': 0.6, 'warm': [1.0, 0.75, 0.45], 'cool': [0.4, 0.7, 1.0]},
                                           'grime': True, 'hazard': {'bands': 2}, 'rough': 0.42, 'metal': 0.80}, 2048, seed=211)

# Utility (DA_Utility_PhoenixSalvage): salvage tug -- rusty orange/black hazard scheme
gen.gen_texture_set("Utility", {'base': [0.40, 0.30, 0.22], 'accent': [0.55, 0.30, 0.05],
                                 'emissive': [1.0, 0.5, 0.15],
                                 'windows': {'cols': 8, 'frac': 0.3, 'warm': [1.0, 0.55, 0.2], 'cool': [0.4, 0.7, 1.0]},
                                 'grime': True, 'hazard': {'bands': 5}, 'cable': {'runs': 6},
                                 'rough': 0.55, 'metal': 0.75}, 2048, seed=212)

# Mining (DA_Mining_Excavator): heavy excavator -- industrial yellow/black
gen.gen_texture_set("Mining", {'base': [0.45, 0.40, 0.15], 'accent': [0.15, 0.12, 0.08],
                                'emissive': [1.0, 0.65, 0.1],
                                'windows': {'cols': 8, 'frac': 0.25, 'warm': [1.0, 0.6, 0.25], 'cool': [0.4, 0.6, 0.9]},
                                'drill_accent': True, 'grime': True, 'hazard': {'bands': 6}, 'cable': {'runs': 8},
                                'rough': 0.55, 'metal': 0.85}, 2048, seed=213)

print("REGEN_DONE: 8 refreshed + 13 new dedicated ship hull texture sets in", gen.TEXDIR)
