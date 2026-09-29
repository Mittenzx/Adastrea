"""
Render the ships the game currently flies with the same clay sheet as
Tools/generate_ship_concepts.py (3/4 hero + top + side ortho), so concepts and
in-game ships can be compared side by side.

Source: the _Assembled FBX each ship Blueprint points at (Assets/FBX/generated),
plus its _Windows mesh (same object space). Hero-generated ships export their
parts in ship space, so when the parts line up with the _Assembled bounds they
are rendered instead to get per-part colours; legacy ships keep their parts in
local space and render as the single hull mesh (+ windows).

Usage: blender -b --python Tools/render_game_ships.py -- [Fighter_01 ...]
Output: Assets/FBX/concepts/renders/current/
"""
import os, sys, glob, importlib.util
import bpy
import numpy as np
from mathutils import Matrix

HERE = os.path.dirname(os.path.abspath(__file__))
_spec = importlib.util.spec_from_file_location("generate_ship_concepts",
                                               os.path.join(HERE, "generate_ship_concepts.py"))
C = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(C)

GEN = os.path.normpath(os.path.join(HERE, "..", "Assets", "FBX", "generated"))
OUT = os.path.join(C.RENDERS, "current")

# Blueprint -> mesh (Content/Blueprints/Ships), deduplicated
SHIPS = [("Fighter_01", "BP_Ship_Fighter"), ("Gunship_02", "BP_Ship_Gunship"),
         ("Escort_01", "BP_Ship_Patrol"), ("Courier_01", "BP_Ship_Science"),
         ("Smuggler_01", "BP_Ship_Frigate / Utility"), ("Trader_01", "BP_Ship_Trading"),
         ("Corvette_01", "BP_Ship_Corvette"), ("Miner_01", "BP_Ship_Mining"),
         ("Freighter_01", "BP_Ship_Freighter"), ("Destroyer_01", "BP_Ship_Destroyer"),
         ("CargoFreighter_01", "BP_Ship_Transport_Genesis"), ("HeavyHauler_01", "BP_Ship_Transport_Behemoth"),
         ("BulkCarrier_01", "BP_Ship_Carrier"), ("Cruiser_01", "BP_Ship_Cruiser"),
         ("Battleship_01", "BP_Battleship / Luxury"), ("CommandXL_01", "BP_CommandXL")]

PART_CLAY = {"Carcass": "Carcass", "Engine": "Engine", "Weapon": "Weapon", "Sensor": "Sensor",
             "Reactor": "Reactor", "Cargo": "Cargo", "Windows": "Windows", "Drill": "Weapon",
             "MiningLaser": "Weapon", "HabRing": "Carcass", "AsteroidShell": "Carcass"}
S = Matrix.Scale(0.01, 4)


def import_mesh(path):
    before = set(bpy.data.objects)
    bpy.ops.import_scene.fbx(filepath=path)
    out = []
    for o in set(bpy.data.objects) - before:
        if o.type != 'MESH':
            bpy.data.objects.remove(o, do_unlink=True)
            continue
        o.data.transform(o.matrix_world)          # bake to ship space
        o.matrix_world = Matrix.Identity(4)
        out.append(o)
    return out


def bounds(objs):
    p = np.array([v.co[:] for o in objs for v in o.data.vertices])
    return p.min(0), p.max(0)


def set_clay(o, key):
    o.data.materials.clear()
    if key.startswith("Nav"):
        nm = key.split("_")[-1]
        o.data.materials.append(C.clay_mat(nm, C.H.NAV_COL[nm][1], 0.3, 0, 12))
    else:
        o.data.materials.append(C.clay_mat(key, *C.CLAY[key]))


def render_ship(ship):
    C.G.setup_scene()
    base = f"SM_Ship_{ship}"
    asm = import_mesh(os.path.join(GEN, base + "_Assembled.fbx"))
    amn, amx = bounds(asm)
    parts = {}
    for f in sorted(glob.glob(os.path.join(GEN, base + "_*.fbx"))):
        suf = os.path.basename(f)[len(base) + 1:-4]
        if suf.startswith("Assembled"):
            continue
        parts[suf] = import_mesh(f)
    body = {k: v for k, v in parts.items() if k != "Windows"}
    use_parts = False
    if body:
        pmn, pmx = bounds([o for v in body.values() for o in v])
        tol = 0.03 * float(np.linalg.norm(amx - amn))
        use_parts = bool(np.all(np.abs(pmn - amn) < tol) and np.all(np.abs(pmx - amx) < tol))
    objs = []
    if use_parts:
        for o in asm:
            bpy.data.objects.remove(o, do_unlink=True)
        for suf, obs in parts.items():
            key = suf if suf.startswith("Nav") else PART_CLAY.get(suf, "Carcass")
            for o in obs:
                set_clay(o, key); objs.append(o)
    else:
        for o in asm:
            set_clay(o, "Carcass"); objs.append(o)
        for suf, obs in parts.items():
            for o in obs:
                if suf == "Windows":
                    set_clay(o, "Windows"); objs.append(o)
                else:
                    bpy.data.objects.remove(o, do_unlink=True)
    for o in objs:
        o.data.transform(S)
    print(f"[game] {ship}: {'per-part colours' if use_parts else 'hull mesh + windows'}")
    return C.render_objs(base, objs, OUT)


def main():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    names = [a for a in argv if not a.startswith("--")] or [s for s, _ in SHIPS]
    os.makedirs(OUT, exist_ok=True)
    heroes = [render_ship(n) for n in names]
    if len(heroes) > 1:
        C.contact_sheet(heroes, os.path.join(OUT, "contact_sheet.png"), cols=4)
    print("GAME_SHIPS_DONE")


if __name__ == "__main__":
    main()
