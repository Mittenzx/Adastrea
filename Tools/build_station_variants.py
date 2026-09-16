"""One-shot: build 5 distinct, single-mesh space station hull variants to
replace the placeholder Cylinder every BP_SpaceStation instance currently
shares in TestLevel.

The pre-existing SM_Station_Mining_01 / SM_Station_AsteroidHab_01 assets are
multi-part ship-style assemblies (Carcass/Drill/Reactor/... as separate
StaticMeshes) built for the hardpoint-mounting system ships use -- too much
machinery for BP_SpaceStation's single StaticMeshComponent. These 5 are each
a single joined mesh instead, matching SM_Station_Habitation_01's shape, so
all 6 station archetypes can drop onto BP_SpaceStation the same simple way.

Archetype -> hull mapping (silhouette-driven, each visually distinct):
  Station_TradeHub     -> SM_Station_Habitation_01 (existing, megatower + ring)
  Station_Agricultural -> SM_Station_Agricultural_01 (new, wide rotating hab-ring)
  Station_Research     -> SM_Station_Research_01     (new, spindly dish array)
  Station_Luxury       -> SM_Station_Luxury_01       (new, tapered glass spire)
  Station_Industrial   -> SM_Station_Industrial_01   (new, boxy refinery/tanks)
  Station_BlackMarket  -> SM_Station_BlackMarket_01  (new, asteroid + bolted-on modules)

Run: blender -b --python Tools/build_station_variants.py
Output: Assets/FBX/generated/SM_Station_{Agricultural,Research,Luxury,
        Industrial,BlackMarket}_01.fbx
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import generate_adastrea_assets as gen

gen.setup_scene()

BUILDS = [
    (gen.build_station_agricultural, "SM_Station_Agricultural_01"),
    (gen.build_station_research, "SM_Station_Research_01"),
    (gen.build_station_luxury, "SM_Station_Luxury_01"),
    (gen.build_station_industrial, "SM_Station_Industrial_01"),
    (gen.build_station_blackmarket, "SM_Station_BlackMarket_01"),
]

for build_fn, out_name in BUILDS:
    obj = build_fn()
    out = gen.export_fbx(obj, out_name)
    print(f"BUILD_DONE: {out}")
