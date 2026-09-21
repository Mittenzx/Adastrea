"""Generates SM_Asteroid_Rock_01..04 (FBX, cm, -Y fwd, Z up) with UCX_ convex collision.
Usage: blender -b --python Tools/generate_asteroid_rocks.py
Output: <repo>/Assets/FBX/generated (same folder convention as generate_adastrea_assets.py)."""
import bpy, bmesh, os, random
from mathutils import Vector, noise

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OUT = os.path.join(REPO, "Assets", "FBX", "generated")
os.makedirs(OUT, exist_ok=True)

# name, seed, subdiv, elongation, noise amp, freq, crater, facets
SPECS = [
    ("SM_Asteroid_Rock_01", 11, 5, (1.0, 0.9, 0.85), 0.28, 1.6, False, 0),
    ("SM_Asteroid_Rock_02", 23, 5, (1.35, 0.8, 0.7), 0.35, 2.2, False, 0),
    ("SM_Asteroid_Rock_03", 37, 5, (1.0, 1.0, 0.9), 0.22, 1.8, True, 0),
    ("SM_Asteroid_Rock_04", 51, 5, (1.15, 0.95, 0.8), 0.10, 1.2, False, 14),
]

def build(spec):
    name, seed, sub, el, amp, freq, crater, facets = spec
    rnd = random.Random(seed)
    bm = bmesh.new()
    bmesh.ops.create_icosphere(bm, subdivisions=sub, radius=1.0)
    off = Vector((rnd.uniform(-50, 50) for _ in range(3)))
    planes = [(Vector((rnd.gauss(0, 1) for _ in range(3))).normalized(), rnd.uniform(0.62, 0.85)) for _ in range(facets)]
    cdir = Vector((0.3, -0.4, 0.85)).normalized()
    for v in bm.verts:
        d = v.co.normalized()
        p = d * freq + off
        r = 1.0 + amp * noise.noise(p) + amp * 0.4 * noise.noise(p * 2.7) \
            + amp * 0.15 * noise.noise(p * 6.1)
        co = d * r
        for n, dist in planes:            # planar cuts -> angular facets
            h = co.dot(n)
            if h > dist:
                co -= n * (h - dist)
        if crater:
            c = d.dot(cdir)
            if c > 0.72:
                t = (c - 0.72) / 0.28     # 0 rim .. 1 centre
                rim = 1 + 0.10 * (1 - abs(t - 0.05) * 6 if t < 0.2 else 0)
                bowl = 0.42 * (1 - (1 - t) ** 2) * (1 if t > 0.15 else t / 0.15)
                co = d * (r * rim - bowl * 1.0)
        v.co = Vector((co.x * el[0], co.y * el[1], co.z * el[2]))
    # normalise so the largest extent is 200 cm, then centre bbox
    mx = max(max(abs(v.co[i]) for v in bm.verts) for i in range(3))
    ext = max((max(v.co[i] for v in bm.verts) - min(v.co[i] for v in bm.verts)) for i in range(3))
    s = 200.0 / ext
    for v in bm.verts:
        v.co *= s
    ctr = sum((v.co for v in bm.verts), Vector()) / len(bm.verts)
    for v in bm.verts:
        v.co -= ctr
    # collision hull
    hull = bm.copy()
    res = bmesh.ops.convex_hull(hull, input=hull.verts)
    bmesh.ops.delete(hull, geom=res["geom_interior"] + res["geom_unused"], context='VERTS')
    me = bpy.data.meshes.new(name); bm.to_mesh(me); bm.free()
    hm = bpy.data.meshes.new("UCX_" + name + "_00"); hull.to_mesh(hm); hull.free()
    ob = bpy.data.objects.new(name, me); ucx = bpy.data.objects.new("UCX_" + name + "_00", hm)
    for o in (ob, ucx):
        bpy.context.scene.collection.objects.link(o)
    return ob, ucx

def export(ob, ucx):
    bpy.ops.object.select_all(action='DESELECT')
    bpy.context.view_layer.objects.active = ob
    ob.select_set(True)
    bpy.ops.object.shade_smooth()
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.uv.smart_project(angle_limit=1.15, island_margin=0.02)
    bpy.ops.object.mode_set(mode='OBJECT')
    mat = bpy.data.materials.new("M_" + ob.name); ob.data.materials.append(mat)
    ucx.select_set(True)
    out = os.path.join(OUT, ob.name + ".fbx")
    bpy.ops.export_scene.fbx(filepath=out, use_selection=True, object_types={'MESH'},
        apply_scale_options='FBX_SCALE_ALL', apply_unit_scale=True,
        axis_forward='-Y', axis_up='Z', mesh_smooth_type='FACE')
    me = ob.data
    tris = sum(len(p.vertices) - 2 for p in me.polygons)
    dims = [round(x, 1) for x in ob.dimensions]
    print(f"EXPORTED {out} tris={tris} dims={dims} ucx_tris={sum(len(p.vertices)-2 for p in ucx.data.polygons)}")

bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.context.scene.unit_settings.system = 'METRIC'
bpy.context.scene.unit_settings.length_unit = 'CENTIMETERS'
for sp in SPECS:
    ob, ucx = build(sp)
    export(ob, ucx)
    ob.hide_set(True); ucx.hide_set(True)
