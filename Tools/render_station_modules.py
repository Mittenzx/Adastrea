"""Render a photoreal preview of every Adastrea station module to station_modules/.

This is a DRIVER: it runs the PROVEN render_scene.py (Blender Cycles photoreal
pipeline) once per module, then copies the produced PNG from scene_renders/ into
station_modules/ (renamed same). It does NOT reimplement rendering — so it keeps
render_scene's correct lighting/normals/texturing.

Usage (host Python, NOT inside Blender):
    python Tools/render_station_modules.py [-- one.fbx ...]
Output: Assets/FBX/generated/station_modules/<name>.png
If no targets given, renders all SM_Station_*.fbx.
"""
import os, sys, glob, subprocess, shutil

HERE = os.path.dirname(os.path.abspath(__file__))
GEN = r"C:\Users\akuma\Adastrea\Assets\FBX\generated"
SCENE = os.path.join(GEN, "scene_renders")
OUT = os.path.join(GEN, "station_modules")
BLENDER = r"C:\Program Files\Blender Foundation\Blender 5.2\blender.exe"
RENDER_SCENE = os.path.join(HERE, "render_scene.py")

os.makedirs(OUT, exist_ok=True)


def render_module(target):
    fbx = os.path.join(GEN, target)
    if not os.path.exists(fbx):
        print("MISSING", target); return False
    # run the proven render_scene for this one module (back camera off for modules
    # so the whole module + margin fits in frame)
    r = subprocess.run([BLENDER, "-b", "--python", RENDER_SCENE, "--", target],
                       capture_output=True, text=True, timeout=400, cwd=HERE)
    name_png = target.replace('.fbx', '.png')
    src = os.path.join(SCENE, name_png)
    if os.path.exists(src):
        shutil.copy(src, os.path.join(OUT, name_png))
        print("OK", target)
        return True
    else:
        tail = (r.stdout + r.stderr).splitlines()[-6:]
        print("FAIL", target, "| scene png missing |", [l for l in tail if 'OK' in l or 'ERR' in l or 'Traceback' in l][:1])
        return False


args = sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
targets = args if args else sorted(os.path.basename(f) for f in glob.glob(os.path.join(GEN, "SM_Station_*.fbx")))
ok = sum(1 for t in targets if render_module(t))
print("DONE_ALL rendered=%d/%d" % (ok, len(targets)))