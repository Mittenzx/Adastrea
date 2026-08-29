"""Visualize hardpoints on a ship — render each mount socket as a glowing sphere
at its manifest position. Colored: teal = occupied module, amber = free socket.
Usage: blender -b --python Tools/render_hardpoints.py -- SM_Ship_Fighter_01
Output: Assets/FBX/generated/previews/<ship>_hardpoints.png
"""
import bpy, os, sys, json
from mathutils import Vector

GEN = r"C:\Users\akuma\Adastrea\Assets\FBX\generated"
OUT = os.path.join(GEN, "previews")
os.makedirs(OUT, exist_ok=True)
COL_OCC = (0.1, 0.9, 0.9, 1.0)   # teal = occupied
COL_FREE = (1.0, 0.6, 0.15, 1.0) # amber = free


def make_marker_mat(color, name):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    try:
        bs = mat.node_tree.nodes['Principled BSDF']
        bs.inputs['Base Color'].default_value = color
        try:
            bs.inputs['Emission Color'].default_value = color
            bs.inputs['Emission Strength'].default_value = 2.0
        except Exception:
            pass
    except Exception:
        pass
    return mat


def render_one(ship):
    # ship name without leading dir/extension
    base = ship.replace('.fbx', '')
    hp_path = os.path.join(GEN, f"{base}_hardpoints.json")
    if not os.path.exists(hp_path):
        print("NO_MANIFEST", base); return
    with open(hp_path) as f:
        hp = json.load(f)

    bpy.ops.wm.read_factory_settings(use_empty=True)

    # load the assembled ship (best representation) if present
    asm = os.path.join(GEN, f"{base}_Assembled.fbx")
    carc = os.path.join(GEN, f"{base}_Carcass.fbx")
    src = asm if os.path.exists(asm) else carc
    bpy.ops.import_scene.fbx(filepath=src)
    objs = [o for o in bpy.data.objects if o.type == 'MESH']
    if not objs:
        print("NO_MESH", base); return
    # bright material for the ship
    ship_mat = bpy.data.materials.new("VisShip"); ship_mat.use_nodes = True
    try:
        bs = ship_mat.node_tree.nodes['Principled BSDF']
        bs.inputs['Base Color'].default_value = (0.5, 0.53, 0.6, 1.0)
        # self-light the hull so it's visible (headless EEVEE ignores lights)
        try:
            bs.inputs['Emission Color'].default_value = (0.5, 0.53, 0.6, 1.0)
            bs.inputs['Emission Strength'].default_value = 1.2
        except Exception:
            pass
    except Exception:
        pass
    for o in objs:
        o.data.materials.clear(); o.data.materials.append(ship_mat)

    # ship bounds (before markers — used to size the markers)
    vmin = Vector((1e9,)*3); vmax = Vector((-1e9,)*3)
    for o in objs:
        for v in o.data.vertices:
            ww = o.matrix_world @ v.co
            for i in range(3):
                vmin[i] = min(vmin[i], ww[i]); vmax[i] = max(vmax[i], ww[i])
    ship_diag = (vmax - vmin).length if (vmax[0] - vmin[0] < 1e8) else 340.0

    occ_mat = make_marker_mat(COL_OCC, "HP_Occ")
    free_mat = make_marker_mat(COL_FREE, "HP_Free")
    # marker radius scales with ship size so sockets read clearly on any class
    marker_r = max(ship_diag * 0.045, 12.0)   # ~4.5% of ship diagonal
    markers = []
    for name, spec in hp['hardpoints'].items():
        pos = spec['position']
        occupied = spec.get('occupied', False)
        bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=2, radius=marker_r, location=tuple(pos))
        m = bpy.context.active_object
        m.name = f"HP_{name}"
        m.data.materials.append(occ_mat if occupied else free_mat)
        markers.append(m)

    # normalize scene (ship + markers) to a fixed size + recenter at origin so
    # framing is reliable regardless of ship scale (learned: raw distance makes
    # small ships tiny/offscreen)
    import bmesh as _bm
    all_geo = objs + markers
    vmin = Vector((1e9,)*3); vmax = Vector((-1e9,)*3)
    for o in all_geo:
        for v in o.data.vertices:
            ww = o.matrix_world @ v.co
            for i in range(3):
                vmin[i] = min(vmin[i], ww[i]); vmax[i] = max(vmax[i], ww[i])
    center = (vmin + vmax) * 0.5
    diag = (vmax - vmin).length
    target = 320.0
    sf = target / max(diag, 1e-6)
    for o in all_geo:
        me = o.data
        b = _bm.new(); b.from_mesh(me)
        for v in b.verts:
            v.co -= Vector(center); v.co *= sf
        b.to_mesh(me); b.free(); me.update()

    # now frame the normalized scene
    vmin = Vector((1e9,)*3); vmax = Vector((-1e9,)*3)
    for o in all_geo:
        for v in o.data.vertices:
            ww = o.matrix_world @ v.co
            for i in range(3):
                vmin[i] = min(vmin[i], ww[i]); vmax[i] = max(vmax[i], ww[i])
    center = (vmin + vmax) * 0.5
    diag = (vmax - vmin).length
    dist = max(diag * 1.2, 100)
    loc = center + Vector((dist, -dist*0.6, dist*0.42))
    bpy.ops.object.camera_add(location=loc)
    cam = bpy.context.active_object; bpy.context.scene.camera = cam; cam.data.lens = 35
    d = (center - loc).normalized()
    cam.rotation_euler = d.to_track_quat('-Z', 'Y').to_euler()

    # dark world
    sc = bpy.context.scene
    w = sc.world if sc.world else bpy.data.worlds.new("V"); sc.world = w; w.use_nodes = True
    try:
        w.node_tree.nodes['Background'].inputs[0].default_value = (0.02, 0.025, 0.04, 1.0)
    except Exception:
        pass

    sc.render.engine = 'BLENDER_EEVEE'
    sc.render.resolution_x = 1400; sc.render.resolution_y = 960
    sc.render.resolution_percentage = 100
    sc.render.image_settings.file_format = 'PNG'
    out_png = os.path.join(OUT, f"{base}_hardpoints.png")
    sc.render.filepath = out_png
    bpy.ops.render.render(write_still=True)
    ok = os.path.exists(out_png)
    nocc = sum(1 for s in hp['hardpoints'].values() if s.get('occupied'))
    print("OK" if ok else "FAIL", base, f"({nocc} occ / {len(hp['hardpoints'])} hp)")

    # cleanup markers from this ship for next iteration
    for n in list(bpy.data.objects):
        if n.name.startswith('HP_'):
            bpy.data.objects.remove(n, do_unlink=True)


for s in sys.argv[sys.argv.index("--")+1:]:
    try:
        render_one(s)
    except Exception as e:
        import traceback; traceback.print_exc()
        print("ERROR", s)