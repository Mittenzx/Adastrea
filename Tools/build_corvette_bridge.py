"""One-shot: build ONLY the new corvette bridge interior from the shared
generator helpers, without running the full generate_adastrea_assets.py
pipeline (which would regenerate every ship/texture too).

Run: blender -b --python Tools/build_corvette_bridge.py
Output: Assets/FBX/generated/SM_Int_Corvette_Bridge_{Shell,Deck,Stations,Viewport,Lights,Hatch}.fbx

Note: this builds the procedural zones only. The Console zone (command seat +
console) is sourced from BlenderKit separately — see
Tools/blenderkit_fetch.py + Tools/blenderkit_import.py, output expected at
Assets/FBX/generated/kitbash/SM_Int_Corvette_Bridge_Console.fbx.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import generate_adastrea_assets as gen

gen.setup_scene()
results = gen.build_corvette_bridge_interior("SM_Int_Corvette_Bridge")
for obj, out in results:
    print(f"BUILD_DONE: {out}")
