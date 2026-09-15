"""One-shot: generate dedicated hull texture sets for ships that currently
have none of their own (Destroyer was borrowing Miner's texture, Battleship
fell through to the default Fighter's). Distinct color/detail schemes so
every hull reads as its own ship class, not a reskin.

Run: blender -b --python Tools/build_fleet_hull_textures.py
Output: Assets/FBX/generated/Textures/T_Destroyer_*.png, T_Battleship_*.png
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import generate_adastrea_assets as gen

gen.setup_scene()

# Destroyer: lean warship, cold gunmetal blue-grey with amber warning
# accents (missile-tube/weapons-bay warning-light convention) -- distinct
# from Gunship's dark-grey/red scheme despite both being "warships".
gen.gen_texture_set("Destroyer",
    {'base': [0.30, 0.32, 0.36], 'accent': [0.42, 0.26, 0.08],
     'emissive': [1.0, 0.55, 0.15],
     'windows': {'cols': 12, 'frac': 0.3, 'cool': [0.4, 0.7, 1.0], 'warm': [1.0, 0.6, 0.2]},
     'grime': True, 'hazard': {'bands': 4}, 'cable': {'runs': 4},
     'rough': 0.36, 'metal': 0.95}, 2048, seed=133)

# Battleship: capital-ship scale, near-black tactical gunmetal with
# white/light-grey stripe accents and a cyan command-beacon emissive --
# meant to read as the biggest, most imposing hull in the fleet.
gen.gen_texture_set("Battleship",
    {'base': [0.16, 0.17, 0.20], 'accent': [0.68, 0.70, 0.74],
     'emissive': [0.2, 0.9, 1.0],
     'windows': {'cols': 18, 'frac': 0.55, 'cool': [0.3, 0.75, 1.0], 'warm': [1.0, 0.65, 0.3]},
     'grime': True, 'hazard': {'bands': 5}, 'cable': {'runs': 6},
     'rough': 0.34, 'metal': 0.96}, 2048, seed=144)

print("BUILD_DONE: T_Destroyer_* + T_Battleship_* texture sets")
