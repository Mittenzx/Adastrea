"""One-shot: build ONLY the new fighter cabin interior (v3, split zones) from
the shared generator helpers, without running the full generate_adastrea_
assets.py pipeline (which would regenerate every ship/texture too).

Run: blender -b --python Tools/build_fighter_cabin.py
Output: Assets/FBX/generated/SM_Int_Fighter_Cabin_{Shell,Viewport,Lights}.fbx

Note: this builds the procedural zones only. The Console zone (pilot seat +
console) is sourced from BlenderKit separately -- see
Tools/blenderkit_fetch.py + Tools/blenderkit_import.py, output expected at
Assets/FBX/generated/kitbash/SM_Int_Fighter_Cabin_Console.fbx.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import generate_adastrea_assets as gen

gen.setup_scene()
results = gen.build_fighter_cabin_interior('small', "SM_Int_Fighter_Cabin")
for obj, out in results:
    print(f"BUILD_DONE: {out}")
