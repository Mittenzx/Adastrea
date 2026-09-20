"""PROTOTYPE: unique-UV + baked texture set for ONE ship (SM_Ship_Corvette_01_Assembled).

Replaces the "one 2048px tile, world-projected every 200 cm" look (smart_uv() +
gen_texture_set() in generate_adastrea_assets.py) with:

  1. a real unwrap of the assembled mesh: Smart UV Project (66 deg) -> average
     island scale (uniform texel density) -> concave island pack into 0-1.
     No overlap, no mirroring, so every square cm of hull has its own texels.
  2. textures BAKED IN CYCLES from the actual mesh + object-space procedural
     shaders (so panel lines, livery, wear are continuous across UV seams and do
     not repeat): AO, convex-edge wear, groove/panel masks, per-plate tone,
     large-scale tone drift, dirt/streaks, livery stripes, lit windows.
     Those masks are composed in numpy into _D _N _R _M _AO _E _SKIN.

Sibling of build_station_hull_textures.py / build_fleet_hull_textures.py; imports
paths, scene setup and the project's FBX export settings from the generator.
Does NOT touch any existing texture or FBX. Outputs:
  Assets/FBX/generated/Textures/T_Corvette_Unique_{D,N,R,M,AO,E,SKIN}.png
  Assets/FBX/generated/SM_Ship_Corvette_01_Assembled_UniqueUV.fbx

Run (build):   blender -b --python Tools/build_unique_hull_textures.py -- build [--size 4096]
Run (preview): blender -b --python Tools/build_unique_hull_textures.py -- preview
Options: --rebake (ignore bake cache), --no-cull, --cache DIR, --dry-run (no writes)
"""
import os
import sys
import math
import time
import json

import bpy
import bmesh
import numpy as np
from mathutils import Vector
from mathutils.bvhtree import BVHTree

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import generate_adastrea_assets as gen

# ----------------------------------------------------------------------------
SHIP = "SM_Ship_Corvette_01"
SRC_FBX = os.path.join(gen.BASE, f"{SHIP}_Assembled.fbx")
OUT_NAME = f"{SHIP}_Assembled_UniqueUV"
TEXSET = "Corvette_Unique"
# In Unreal the Corvette is ~9.3 m long; the FBX mesh is 1439 Blender units long.
GAME_LENGTH_M = 9.3
# Same palette as gen_texture_set("Corvette", ...) in the generator (line ~3259)
VARIANT = {'base': (0.36, 0.28, 0.46), 'accent': (0.25, 0.15, 0.40),
           'emissive': (0.6, 0.3, 1.0), 'rough': 0.50, 'metal': 0.45,   # painted hull: sensible PBR, not a mirror
           
           'win_cool': (0.6, 0.5, 1.0), 'win_warm': (1.0, 0.5, 0.7)}


def argv_after_dashes():
    a = sys.argv
    return a[a.index("--") + 1:] if "--" in a else []


def opt(args, name, default=None, cast=str):
    if name in args:
        return cast(args[args.index(name) + 1])
    return default


# ----------------------------------------------------------------------------
# 1. import + hidden-face cull + unwrap
# ----------------------------------------------------------------------------
def import_source():
    gen.setup_scene()
    bpy.ops.import_scene.fbx(filepath=SRC_FBX, axis_forward='-Y', axis_up='Z')
    ob = [o for o in bpy.data.objects if o.type == 'MESH'][0]
    return ob


def cull_buried_faces(ob, eps=0.05):
    """Delete faces whose sample points are ALL inside another solid (kit-bash
    parts are joined, not booleaned, so a lot of geometry is buried). A face is
    buried at a sample point if a ray shot outward hits a BACK face first.
    Requiring every sample (centre + 3/4-inset vertices) avoids punching holes
    in big hull faces that are only partly covered by a greeble."""
    bm = bmesh.new()
    bm.from_mesh(ob.data)
    bm.faces.ensure_lookup_table()
    tree = BVHTree.FromBMesh(bm)
    kill = []
    for f in bm.faces:
        c = f.calc_center_median()
        n = f.normal
        pts = [c] + [c.lerp(v.co, 0.75) for v in f.verts]
        buried = True
        for p in pts:
            hit = tree.ray_cast(p + n * eps, n)
            if hit[0] is None or hit[1].dot(n) <= 0.0:
                buried = False
                break
        if buried:
            kill.append(f)
    n_before = len(bm.faces)
    bmesh.ops.delete(bm, geom=kill, context='FACES')
    loose = [v for v in bm.verts if not v.link_faces]
    bmesh.ops.delete(bm, geom=loose, context='VERTS')
    bm.to_mesh(ob.data)
    bm.free()
    ob.data.update()
    return n_before, len(kill)


def unwrap(ob, size, margin_px):
    me = ob.data
    while me.uv_layers:
        me.uv_layers.remove(me.uv_layers[0])
    me.uv_layers.new(name="UVMap")
    gen.sel_activate(ob)
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.context.scene.tool_settings.use_uv_select_sync = True
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.uv.smart_project(angle_limit=math.radians(66), margin_method='SCALED',
                             rotate_method='AXIS_ALIGNED_Y', island_margin=0.0,
                             area_weight=0.0, correct_aspect=True, scale_to_bounds=False)
    bpy.ops.uv.select_all(action='SELECT')
    bpy.ops.uv.average_islands_scale()          # equal texel density per island
    bpy.ops.uv.pack_islands(rotate=True, rotate_method='CARDINAL', scale=True,
                            margin_method='FRACTION', margin=margin_px / size,
                            shape_method='CONCAVE')
    bpy.ops.object.mode_set(mode='OBJECT')


def uv_stats(ob, size, m_per_bu):
    """Texel density (px per metre at final in-game scale) + stretch per face."""
    me = ob.data
    uv = me.uv_layers[0].data
    dens, tot_uv = [], 0.0
    ang_err = []
    for p in me.polygons:
        pts = [uv[l].uv.copy() for l in p.loop_indices]
        a = 0.0
        for i in range(len(pts)):
            x1, y1 = pts[i]
            x2, y2 = pts[(i + 1) % len(pts)]
            a += x1 * y2 - x2 * y1
        a = abs(a) * 0.5
        tot_uv += a
        a3 = p.area * m_per_bu ** 2
        if a3 > 1e-9 and a > 0:
            dens.append(math.sqrt(a * size * size / a3))   # px per metre, linear
        # stretch: singular values of the 3D->UV Jacobian of the first triangle
        vs = [me.vertices[i].co for i in p.vertices]
        if len(vs) >= 3:
            e1, e2 = vs[1] - vs[0], vs[2] - vs[0]
            t1 = e1.normalized()
            nrm = e1.cross(e2)
            if nrm.length > 1e-9:
                t2 = nrm.normalized().cross(t1)
                A = np.array([[e1.dot(t1), e2.dot(t1)], [0.0, e2.dot(t2)]])
                U = np.array([[(pts[1] - pts[0])[0], (pts[2] - pts[0])[0]],
                              [(pts[1] - pts[0])[1], (pts[2] - pts[0])[1]]])
                try:
                    sv = np.linalg.svd(U @ np.linalg.inv(A), compute_uv=False)
                    if sv[1] > 1e-12:
                        ang_err.append(sv[0] / sv[1])       # 1.0 = perfect, no stretch
                except np.linalg.LinAlgError:
                    pass
    d = np.array(dens)
    ranked = np.sort(d)
    weights = np.array([p.area for p in me.polygons if p.area * m_per_bu ** 2 > 1e-9])
    return {
        'faces': len(me.polygons),
        'uv_coverage': tot_uv,
        'px_per_m_min': float(ranked[0]), 'px_per_m_p5': float(np.percentile(d, 5)),
        'px_per_m_median': float(np.median(d)), 'px_per_m_p95': float(np.percentile(d, 95)),
        'px_per_m_max': float(ranked[-1]),
        'px_per_m_area_weighted_mean': float((d * weights[:len(d)]).sum() / weights[:len(d)].sum()),
        'stretch_median': float(np.median(ang_err)),
        'stretch_p95': float(np.percentile(ang_err, 95)), 'stretch_max': float(np.max(ang_err)),
        'uv_min': [float(min(x.uv[0] for x in uv)), float(min(x.uv[1] for x in uv))],
        'uv_max': [float(max(x.uv[0] for x in uv)), float(max(x.uv[1] for x in uv))],
    }


# ----------------------------------------------------------------------------
# 2. Cycles bake of object-space procedural masks
# ----------------------------------------------------------------------------
class Shader:
    """Tiny node-builder helper."""
    def __init__(self, nt):
        self.nt = nt
        self.n = nt.nodes

    def _set(self, node, key, v):
        if v is None:
            return
        if isinstance(v, bpy.types.NodeSocket):
            self.nt.links.new(v, node.inputs[key])
        else:
            node.inputs[key].default_value = v

    def math(self, op, a, b=None, clamp=False, c=None):
        n = self.n.new('ShaderNodeMath')
        n.operation = op
        n.use_clamp = clamp
        self._set(n, 0, a)
        self._set(n, 1, b)
        self._set(n, 2, c)
        return n.outputs[0]

    def vmath(self, op, a, b=None):
        n = self.n.new('ShaderNodeVectorMath')
        n.operation = op
        self._set(n, 0, a)
        self._set(n, 3 if op == 'SCALE' else 1, b)
        return n.outputs['Value'] if op in ('DOT_PRODUCT', 'LENGTH', 'DISTANCE') else n.outputs['Vector']

    def combine(self, x, y, z):
        n = self.n.new('ShaderNodeCombineXYZ')
        self._set(n, 0, x)
        self._set(n, 1, y)
        self._set(n, 2, z)
        return n.outputs[0]

    def separate(self, v):
        n = self.n.new('ShaderNodeSeparateXYZ')
        self.nt.links.new(v, n.inputs[0])
        return n.outputs[0], n.outputs[1], n.outputs[2]

    def noise(self, vec, scale, detail=3.0, rough=0.55):
        n = self.n.new('ShaderNodeTexNoise')
        n.noise_dimensions = '3D'
        n.inputs['Scale'].default_value = scale
        n.inputs['Detail'].default_value = detail
        n.inputs['Roughness'].default_value = rough
        self.nt.links.new(vec, n.inputs['Vector'])
        return n.outputs['Fac']

    def brick(self, vec, w, h, mortar, smooth=0.002, offset=0.5, freq=2, squash=1.0,
              squash_freq=99, bias=0.0):
        n = self.n.new('ShaderNodeTexBrick')
        n.offset = offset
        n.offset_frequency = freq
        n.squash = squash
        n.squash_frequency = squash_freq
        n.inputs['Color1'].default_value = (0, 0, 0, 1)
        n.inputs['Color2'].default_value = (1, 1, 1, 1)
        n.inputs['Mortar'].default_value = (0, 0, 0, 1)
        n.inputs['Scale'].default_value = 1.0
        n.inputs['Mortar Size'].default_value = mortar
        n.inputs['Mortar Smooth'].default_value = smooth
        n.inputs['Bias'].default_value = bias
        n.inputs['Brick Width'].default_value = w
        n.inputs['Row Height'].default_value = h
        self.nt.links.new(vec, n.inputs['Vector'])
        # Fac: 1 in MORTAR, 0 inside a brick (verified). Colour R: per-brick random 0..1 (0 in mortar)
        return n.outputs['Fac'], n.outputs['Color']


def build_bake_material(ob, bbox, m_per_bu):
    """Returns (material, channels dict name -> socket, bake image node)."""
    mat = bpy.data.materials.new("BAKE_TMP")
    mat.use_nodes = True
    nt = mat.node_tree
    nt.nodes.clear()
    S = Shader(nt)
    out = nt.nodes.new('ShaderNodeOutputMaterial')
    emit = nt.nodes.new('ShaderNodeEmission')
    nt.links.new(emit.outputs[0], out.inputs[0])

    (xmin, ymin, zmin), (xmax, ymax, zmax) = bbox
    L = (ymax - ymin) * m_per_bu           # ship length in metres
    yc = (ymax + ymin) * 0.5

    tc = nt.nodes.new('ShaderNodeTexCoord')
    geo = nt.nodes.new('ShaderNodeNewGeometry')
    # object space, in metres, ship-centred (x lateral, y fore(+)/aft(-), z up)
    P = S.vmath('SCALE', tc.outputs['Object'], m_per_bu)
    px, py, pz = S.separate(P)
    N = geo.outputs['Normal']
    nx, ny, nz = S.separate(N)
    ax, ay, az = (S.math('ABSOLUTE', v) for v in (nx, ny, nz))

    # hard triplanar selectors (dominant axis of the face normal)
    selX = S.math('MULTIPLY', S.math('GREATER_THAN', ax, ay), S.math('GREATER_THAN', ax, az))
    selY = S.math('MULTIPLY', S.math('SUBTRACT', 1.0, selX), S.math('GREATER_THAN', ay, az))
    selZ = S.math('SUBTRACT', S.math('SUBTRACT', 1.0, selX), selY)

    def planar(fn, off=(0.0, 0.0)):
        """Evaluate fn(vec2) in the 3 axis-aligned planes and select by normal.
        Plane coords keep a consistent metre scale so panel size is uniform."""
        ox, oy = off
        c_x = S.combine(S.math('ADD', py, ox), S.math('ADD', pz, oy), 0.0)          # X-facing
        c_y = S.combine(S.math('ADD', px, ox * 1.7 + 0.31), S.math('ADD', pz, oy), 0.0)  # Y-facing
        c_z = S.combine(S.math('ADD', px, ox * 0.6 + 0.77), S.math('ADD', py, oy * 1.3), 0.0)  # Z-facing
        rs = [fn(c) for c in (c_x, c_y, c_z)]
        # fn returns one or several scalars; select each
        outs = []
        for i in range(len(rs[0])):
            v = S.math('ADD', S.math('ADD', S.math('MULTIPLY', rs[0][i], selX),
                                      S.math('MULTIPLY', rs[1][i], selY)),
                       S.math('MULTIPLY', rs[2][i], selZ))
            outs.append(v)
        return outs

    # --- panel hierarchy ------------------------------------------------
    # A: large primary plates (1.5 x 0.9 m in-game), staggered, deep seams
    def panelA(c):
        fac, col = S.brick(c, 1.5, 0.9, 0.022, smooth=0.003, offset=0.5, freq=2)
        return (fac, S.separate(col)[0])
    gA, rndA = planar(panelA, (0.13, 0.27))
    # B: small sub-panels (0.55 x 0.32 m), thin seams, only in noise-selected
    # regions so the panel scale changes across the hull (breaks the grid look)
    def panelB(c):
        fac, col = S.brick(c, 0.55, 0.32, 0.009, smooth=0.002, offset=0.33, freq=3)
        return (fac, S.separate(col)[0])
    gB_raw, rndB = planar(panelB, (0.51, 0.09))
    region = S.math('GREATER_THAN', S.noise(S.vmath('ADD', P, (3.1, 7.7, 1.3)), 0.45, 2.0), 0.42)
    gB = S.math('MULTIPLY', gB_raw, region)
    plate = S.math('ADD', S.math('MULTIPLY', rndA, S.math('SUBTRACT', 1.0, region)),
                   S.math('MULTIPLY', rndB, region))
    # glow flag: ~22% of primary plates get faint emissive seams
    glow = S.math('GREATER_THAN', rndA, 0.78)

    # --- large-scale tone drift + dirt streak noise ----------------------
    drift = S.noise(S.vmath('ADD', P, (11.0, 2.0, 5.0)), 0.22, 4.0, 0.6)
    blot = S.noise(S.vmath('ADD', P, (1.0, 19.0, 3.0)), 0.9, 5.0, 0.6)
    # streaks run fore/aft (airflow / exhaust smear): squash Y coord
    streak_v = S.combine(S.math('MULTIPLY', px, 4.0), S.math('MULTIPLY', py, 0.35),
                         S.math('MULTIPLY', pz, 4.0))
    streak = S.noise(S.vmath('ADD', streak_v, (5.0, 5.0, 0.0)), 1.6, 3.0, 0.55)
    chip = S.noise(S.vmath('ADD', P, (2.0, 3.0, 9.0)),
                   38.0, 3.0, 0.6)
    micro = S.noise(P, 210.0, 2.0, 0.5)

    # --- livery: object-space, bilaterally symmetric on purpose ----------
    fore = S.math('DIVIDE', S.math('SUBTRACT', py, (yc - 0) * m_per_bu), L)   # -0.5 aft .. +0.5 nose
    ux = S.math('ABSOLUTE', px)
    top = S.math('GREATER_THAN', nz, 0.55)
    # dorsal racing stripe pair, |x| in 0.35..0.62 m, from -0.28 to +0.34 of length
    st_lat = S.math('MULTIPLY', S.math('GREATER_THAN', ux, 0.34), S.math('LESS_THAN', ux, 0.64))
    st_len = S.math('MULTIPLY', S.math('GREATER_THAN', fore, -0.28), S.math('LESS_THAN', fore, 0.34))
    stripe = S.math('MULTIPLY', S.math('MULTIPLY', st_lat, st_len), top)
    # nose tip colour block, angled cut (chevron via |x|)
    nose = S.math('LESS_THAN', S.math('SUBTRACT', S.math('SUBTRACT', 0.5, fore),
                                       S.math('MULTIPLY', ux, 0.12)), 0.085)
    # aft band all round the hull
    aft = S.math('MULTIPLY', S.math('GREATER_THAN', fore, -0.235), S.math('LESS_THAN', fore, -0.185))
    # thin pinstripe next to the dorsal stripe + aft band
    pin_lat = S.math('MULTIPLY', S.math('GREATER_THAN', ux, 0.68), S.math('LESS_THAN', ux, 0.71))
    pin = S.math('MULTIPLY', S.math('MULTIPLY', pin_lat, st_len), top)
    pin2 = S.math('MULTIPLY', S.math('GREATER_THAN', fore, -0.17), S.math('LESS_THAN', fore, -0.16))
    # hazard chevrons across the aft engine bay
    haz_band = S.math('LESS_THAN', fore, -0.41)
    diag = S.math('FLOORED_MODULO', S.math('ADD', S.math('MULTIPLY', ux, 6.0), S.math('MULTIPLY', pz, 6.0)), 1.0)
    haz = S.math('MULTIPLY', S.math('MULTIPLY', haz_band, S.math('LESS_THAN', diag, 0.5)),
                 S.math('GREATER_THAN', S.math('FLOORED_MODULO', S.math('MULTIPLY', py, 0.9), 0.5), 0.2))
    haz = S.math('MULTIPLY', haz, S.math('LESS_THAN', S.math('ABSOLUTE', S.math('SUBTRACT', fore, -0.44)), 0.03))
    liv1 = S.math('MINIMUM', S.math('ADD', S.math('ADD', stripe, nose), aft), 1.0)
    liv2 = S.math('MINIMUM', S.math('ADD', S.math('ADD', pin, pin2), haz), 1.0)

    # --- windows on flanks: row of strip viewports, some lit -------------
    zc = (zmin + zmax) * 0.5 * m_per_bu
    def win(c):
        fac, col = S.brick(c, 0.42, 0.11, 0.045, smooth=0.002, offset=0.5, freq=2)
        return (S.math('SUBTRACT', 1.0, fac), S.separate(col)[0])
    wfac, wrnd = planar(win, (0.0, 0.0))
    side = S.math('GREATER_THAN', S.math('ABSOLUTE', S.separate(geo.outputs['True Normal'])[0]), 0.98)
    wband = S.math('MULTIPLY', S.math('GREATER_THAN', pz, zc - 0.25), S.math('LESS_THAN', pz, zc + 0.55))
    wlen = S.math('MULTIPLY', S.math('GREATER_THAN', fore, -0.32), S.math('LESS_THAN', fore, 0.22))
    wmask = S.math('MULTIPLY', S.math('MULTIPLY', side, wband), wlen)
    win_glass = S.math('MULTIPLY', wmask, wfac)
    win_lit = S.math('MULTIPLY', win_glass, S.math('GREATER_THAN', wrnd, 0.5))
    win_warm = S.math('GREATER_THAN', wrnd, 0.86)

    # --- convex edges: bevel node + inside-AO ----------------------------
    bev = nt.nodes.new('ShaderNodeBevel')
    bev.inputs['Radius'].default_value = 1.8     # Blender units (~2.3 cm in game)
    bev.samples = 8
    nt.links.new(geo.outputs['True Normal'], bev.inputs['Normal'])
    dnode = nt.nodes.new('ShaderNodeVectorMath')
    dnode.operation = 'DOT_PRODUCT'
    nt.links.new(bev.outputs['Normal'], dnode.inputs[0])
    nt.links.new(geo.outputs['True Normal'], dnode.inputs[1])
    edge_b = S.math('MULTIPLY', S.math('SUBTRACT', 1.0, dnode.outputs['Value']), 14.0, clamp=True)
    aoi = nt.nodes.new('ShaderNodeAmbientOcclusion')
    aoi.inside = True
    aoi.samples = 8
    nt.links.new(geo.outputs['True Normal'], aoi.inputs['Normal'])
    aoi.inputs['Distance'].default_value = 1.8
    edge_ao = S.math('SUBTRACT', 1.0, aoi.outputs['AO'])
    edge = S.math('MAXIMUM', edge_b, S.math('MULTIPLY', edge_ao, 1.6, clamp=True), clamp=True)

    # thickness: inside-AO with a long ray; ~1 on chunky parts, low on thin/small ones
    aot = nt.nodes.new('ShaderNodeAmbientOcclusion')
    aot.inside = True
    aot.samples = 8
    aot.inputs['Distance'].default_value = 9.0
    nt.links.new(geo.outputs['True Normal'], aot.inputs['Normal'])
    thick = aot.outputs['AO']
    # rivets: dot grid (7 cm pitch, non-periodic phase per plane) masked to a band beside primary seams
    def riv(c):
        fac, col = S.brick(c, 1.5, 0.9, 0.075, smooth=0.002, offset=0.5, freq=2)
        cx, cy, _ = S.separate(c)
        k = 2 * math.pi / 0.07
        d = S.math('MULTIPLY', S.math('SINE', S.math('MULTIPLY', cx, k)), S.math('SINE', S.math('MULTIPLY', cy, k)))
        dots = S.math('GREATER_THAN', d, 0.93)
        return (S.math('MULTIPLY', dots, fac),)
    rivets, = planar(riv, (0.13, 0.27))
    rivets = S.math('MULTIPLY', rivets, S.math('SUBTRACT', 1.0, S.math('MULTIPLY', gA, 1.0)), clamp=True)

    # --- pack into RGB bakes --------------------------------------------
    def pack(r, g, b):
        cn = nt.nodes.new('ShaderNodeCombineColor')
        for i, v in enumerate((r, g, b)):
            S._set(cn, i, v)
        return cn.outputs[0]

    channels = {
        'P1': pack(gA, gB, plate),
        'P2': pack(edge, liv1, liv2),
        'P3': pack(win_glass, win_lit, S.math('MULTIPLY', win_lit, win_warm)),
        'P4': pack(drift, blot, streak),
        'P5': pack(chip, micro, glow),
        'P6': pack(rivets, thick, 0.0),
    }
    # emit -> output; image node target
    img_node = nt.nodes.new('ShaderNodeTexImage')
    nt.nodes.active = img_node
    return mat, channels, emit, img_node


def bake_all(ob, size, m_per_bu, bbox, cache, rebake, ao_samples=96):
    scene = bpy.context.scene
    scene.render.engine = 'CYCLES'
    scene.cycles.device = 'CPU'
    scene.cycles.use_denoising = False
    scene.render.bake.margin = 16
    scene.render.bake.margin_type = 'EXTEND'
    scene.render.bake.use_clear = True
    w = bpy.data.worlds.new("W")
    scene.world = w
    w.use_nodes = True
    mat, channels, emit, img_node = build_bake_material(ob, bbox, m_per_bu)
    ob.data.materials.clear()
    ob.data.materials.append(mat)
    nt = mat.node_tree
    gen.sel_activate(ob)
    res = {}
    timings = {}

    def do_bake(name, btype, samples):
        f = os.path.join(cache, f"{name}.npy")
        if os.path.exists(f) and not rebake:
            res[name] = np.load(f)
            return
        t0 = time.time()
        img = bpy.data.images.new(f"bake_{name}", size, size, alpha=False, float_buffer=True)
        img.colorspace_settings.name = 'Non-Color'
        img_node.image = img
        nt.nodes.active = img_node
        scene.cycles.samples = samples
        bpy.ops.object.bake(type=btype)
        arr = np.empty(size * size * 4, dtype=np.float32)
        img.pixels.foreach_get(arr)
        arr = arr.reshape(size, size, 4)[..., :3].copy()
        np.save(f, arr)
        res[name] = arr
        bpy.data.images.remove(img)
        timings[name] = round(time.time() - t0, 1)
        print(f"BAKE {name}: {timings[name]}s", flush=True)

    for name, sock in channels.items():
        # link the pack to emission colour
        for l in list(nt.links):
            if l.to_node == emit and l.to_socket.name == 'Color':
                nt.links.remove(l)
        nt.links.new(sock, emit.inputs['Color'])
        nt.links.new(emit.outputs[0], nt.nodes['Material Output'].inputs[0])
        do_bake(name, 'EMIT', 28 if name == 'P2' else 4)
    # true AO from real geometry (native Cycles AO bake, world distance = 0.6 m game)
    w.light_settings.distance = 0.6 / m_per_bu
    do_bake('AO', 'AO', ao_samples)
    return res, timings


# ----------------------------------------------------------------------------
# 3. compose maps
# ----------------------------------------------------------------------------
def box_blur(a, r):
    """Separable box blur via cumulative sums (no scipy)."""
    if r <= 0:
        return a
    k = 2 * r + 1
    for ax in (0, 1):
        c = np.cumsum(np.pad(a, [(r + 1, r) if i == ax else (0, 0) for i in range(a.ndim)], mode='edge'),
                      axis=ax, dtype=np.float64)
        sl_hi = [slice(None)] * a.ndim
        sl_lo = [slice(None)] * a.ndim
        sl_hi[ax] = slice(k, None)
        sl_lo[ax] = slice(0, -k)
        a = ((c[tuple(sl_hi)] - c[tuple(sl_lo)]) / k).astype(np.float32)
    return a


def smoothstep(e0, e1, x):
    t = np.clip((x - e0) / (e1 - e0), 0.0, 1.0)
    return t * t * (3 - 2 * t)


def compose(B, size, px_per_m):
    v = VARIANT
    P1, P2, P3, P4, P5, P6 = (B[k] for k in ('P1', 'P2', 'P3', 'P4', 'P5', 'P6'))
    rivet, thick = P6[..., 0], P6[..., 1]
    AObake = B['AO'][..., 0]
    gA, gB, plate = P1[..., 0], P1[..., 1], P1[..., 2]
    edge, liv1, liv2 = P2[..., 0], P2[..., 1], P2[..., 2]
    wglass, wlit, wwarm = P3[..., 0], P3[..., 1], P3[..., 2]
    drift, blot, streak = P4[..., 0], P4[..., 1], P4[..., 2]
    chip, micro, glow = P5[..., 0], P5[..., 1], P5[..., 2]
    # smooth binary-ish masks slightly so grooves antialias in mips
    gA = box_blur(gA, 1)
    gB = box_blur(gB, 1)
    edge = box_blur(edge, 2)

    base = np.array(v['base'], dtype=np.float32)
    accent = np.array(v['accent'], dtype=np.float32)
    emis = np.array(v['emissive'], dtype=np.float32)

    # -- wear masks --
    chip_m = smoothstep(0.42, 0.62, chip + (micro - 0.5) * 0.3)          # chipped, not uniform
    size_w = smoothstep(0.35, 0.85, thick)                         # tiny parts: little/no edge wear
    wear = np.clip(edge * size_w * (0.30 + 0.60 * chip_m), 0, 1)
    wear = smoothstep(0.25, 0.75, wear)
    cavity = 1.0 - AObake                                                # 0 open .. 1 crevice
    dirt = np.clip(cavity * 1.6, 0, 1) * (0.35 + 0.65 * smoothstep(0.35, 0.7, blot))
    dirt += 0.55 * smoothstep(0.50, 0.72, streak) * (0.5 + 0.5 * cavity)
    dirt = np.clip(dirt, 0, 1) * (1 - wear)

    # -- D --
    tone = 1.0 + 0.30 * (drift - 0.5) + 0.20 * (plate - 0.5)
    D = base[None, None, :] * tone[..., None]
    hue = (drift - 0.5) * 0.10
    D[..., 0] *= (1 + hue)
    D[..., 2] *= (1 - hue)
    D *= (1 - 0.10 * (micro[..., None] - 0.5))                            # fine grain
    l1 = np.array([0.20, 0.10, 0.36], dtype=np.float32)                   # deep violet stripe
    l2 = np.array([0.60, 0.58, 0.66], dtype=np.float32)                   # pale pin/hazard
    D = D * (1 - liv1[..., None]) + (l1 * (1 + 0.15 * (plate[..., None] - 0.5))) * liv1[..., None]
    D = D * (1 - liv2[..., None]) + l2 * liv2[..., None]
    D *= (1 - 0.55 * gA)[..., None] * (1 - 0.35 * gB)[..., None]        # seam darkening
    D *= (1 - 0.45 * dirt)[..., None] * np.array([1.0, 0.96, 0.92], dtype=np.float32)
    bare = np.array([0.50, 0.49, 0.54], dtype=np.float32)
    D = D * (1 - wear[..., None]) + bare * wear[..., None]
    riv = box_blur(rivet, 1)
    D = D * (1 - 0.6 * riv[..., None]) + np.array([0.42, 0.42, 0.46], dtype=np.float32) * (0.6 * riv[..., None])
    D *= (0.65 + 0.35 * AObake)[..., None]                                # light baked contact shading
    # windows
    dark_glass = np.array([0.03, 0.04, 0.06], dtype=np.float32)
    cool = np.array(v['win_cool'], dtype=np.float32)
    warm = np.array(v['win_warm'], dtype=np.float32)
    lit_col = cool[None, None, :] * (1 - wwarm[..., None]) + warm[None, None, :] * wwarm[..., None]
    D = D * (1 - wglass[..., None]) + dark_glass * wglass[..., None]
    D = D * (1 - wlit[..., None]) + (lit_col * 0.55) * wlit[..., None]
    D = np.clip(D, 0, 1)

    # -- height -> normal (DirectX / Y-down, UE convention) --
    h = (-0.006 * gA - 0.0025 * gB + 0.0008 * (plate - 0.5) - 0.0004 * micro
         - 0.0012 * wglass + 0.0015 * wear + 0.0009 * riv)
    px = 1.0 / px_per_m                                  # metres per texel
    gy, gx = np.gradient(h, px)
    nrm = np.stack([-gx, gy, np.ones_like(gx)], axis=-1)  # gy sign flipped => DirectX green
    nrm /= np.linalg.norm(nrm, axis=-1, keepdims=True)
    N = nrm * 0.5 + 0.5

    # -- R / M --
    R = v['rough'] + 0.10 * (plate - 0.5) + 0.06 * (micro - 0.5)
    R = R + 0.30 * dirt
    R = np.where(wear > 0.02, R * (1 - wear) + 0.40 * wear, R)
    R = R + 0.30 * gA
    R = np.where(wglass > 0.5, 0.10, R)
    R = np.clip(R, 0.05, 1.0)
    M = v['metal'] - 0.16 * plate - 0.20 * liv1 - 0.30 * liv2 - 0.25 * dirt - 0.20 * gA
    M = M * (1 - wear) + 0.90 * wear
    M = M + 0.35 * riv
    M = np.where(wglass > 0.5, 0.0, M)
    M = np.clip(M, 0.0, 1.0)

    # -- AO map: baked geometry AO * cavity of grooves --
    AO = np.clip(AObake * (1 - 0.35 * gA) * (1 - 0.15 * gB), 0.0, 1.0)

    # -- E --
    E = np.zeros(D.shape, dtype=np.float32)
    seam_glow = (gA * glow)[..., None] * emis * 0.55
    E += seam_glow
    E += (wlit[..., None] * lit_col * 0.7)
    E = np.clip(E, 0, 1)

    # -- SKIN: 1 = re-skinnable hull panel, 0 = livery/window/emissive --
    fixed = np.clip(np.maximum.reduce([liv1, liv2, wglass, wlit, gA * glow]), 0, 1)
    SKIN = 1.0 - fixed
    return D, N, R, M, AO, E, SKIN, {'wear_frac': float(wear.mean()), 'dirt_mean': float(dirt.mean())}


def write_png(name, arr, srgb):
    size = arr.shape[0]
    if arr.ndim == 2:
        arr = np.stack([arr] * 3, axis=-1)
    rgba = np.ones((size, size, 4), dtype=np.float32)
    rgba[..., :3] = arr
    img = bpy.data.images.new(name, size, size, alpha=False)
    img.colorspace_settings.name = 'sRGB' if srgb else 'Non-Color'
    img.pixels.foreach_set(rgba.ravel())
    img.filepath_raw = os.path.join(gen.TEXDIR, name)
    img.file_format = 'PNG'
    img.save()
    p = img.filepath_raw
    bpy.data.images.remove(img)
    return p


# ----------------------------------------------------------------------------
def cmd_build(args):
    size = opt(args, '--size', 4096, int)
    cache = opt(args, '--cache', os.path.join(os.environ.get('TEMP', '.'), 'unique_hull_cache'))
    os.makedirs(cache, exist_ok=True)
    rebake = '--rebake' in args
    dry = '--dry-run' in args
    t_start = time.time()

    ob = import_source()
    dims = ob.dimensions.copy()
    m_per_bu = GAME_LENGTH_M / dims.y
    print(f"SRC dims BU {tuple(round(d, 1) for d in dims)}  m/BU={m_per_bu:.6f}  faces={len(ob.data.polygons)}")
    src_area = sum(p.area for p in ob.data.polygons) * m_per_bu ** 2
    if '--no-cull' not in args:
        nb, nk = cull_buried_faces(ob)
        print(f"CULL buried faces: {nk}/{nb} removed")
    area = sum(p.area for p in ob.data.polygons) * m_per_bu ** 2
    print(f"AREA game m2: source {src_area:.1f} -> after cull {area:.1f}")
    unwrap(ob, size, margin_px=10)
    st = uv_stats(ob, size, m_per_bu)
    print("UVSTATS", json.dumps(st, indent=1))
    # bbox in BU, object space
    cs = [Vector(c) for c in ob.bound_box]
    bbox = ((min(c.x for c in cs), min(c.y for c in cs), min(c.z for c in cs)),
            (max(c.x for c in cs), max(c.y for c in cs), max(c.z for c in cs)))
    px_per_m = st['px_per_m_area_weighted_mean']
    if dry:
        print("DRY RUN: stopping before bake/export")
        return
    B, timings = bake_all(ob, size, m_per_bu, bbox, cache, rebake)
    t_bake = time.time() - t_start
    D, N, R, M, AO, E, SKIN, extra = compose(B, size, px_per_m)
    outs = {}
    for suf, arr, srgb in (('D', D, True), ('N', N, False), ('R', R, False), ('M', M, False),
                           ('AO', AO, False), ('E', E, True), ('SKIN', SKIN, False)):
        outs[suf] = write_png(f"T_{TEXSET}_{suf}.png", arr, srgb)
    # final material slot named exactly like the source FBX slot so the UE
    # slot -> M_Corvette_Hull assignment is unchanged
    ob.data.materials.clear()
    for old_m in list(bpy.data.materials):          # avoid 'M_Assembled.001' slot name
        bpy.data.materials.remove(old_m)
    m = bpy.data.materials.new("M_Assembled")
    m.use_nodes = True
    ob.data.materials.append(m)
    ob.name = f"{SHIP}_Assembled"
    fbx = gen.export_fbx(ob, OUT_NAME)
    st.update({'texture_files': outs, 'fbx': fbx, 'size': size, 'bake_timings_s': timings,
               'total_s': round(time.time() - t_start, 1), 'src_area_m2': src_area,
               'area_after_cull_m2': area, 'm_per_bu': m_per_bu, **extra})
    with open(os.path.join(cache, 'build_report.json'), 'w') as f:
        json.dump(st, f, indent=1)
    print("BUILD_DONE", json.dumps({k: st[k] for k in ('fbx', 'total_s', 'size')}))


# ----------------------------------------------------------------------------
# 4. preview
# ----------------------------------------------------------------------------
def make_preview_mat(name, texset, use_normal=True, flip_green=False):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    nt = mat.node_tree
    nt.nodes.clear()
    out = nt.nodes.new('ShaderNodeOutputMaterial')
    pr = nt.nodes.new('ShaderNodeBsdfPrincipled')
    nt.links.new(pr.outputs[0], out.inputs[0])

    def tex(suf, cs):
        n = nt.nodes.new('ShaderNodeTexImage')
        n.image = bpy.data.images.load(os.path.join(gen.TEXDIR, f"T_{texset}_{suf}.png"))
        n.image.colorspace_settings.name = cs
        return n
    d = tex('D', 'sRGB')
    nt.links.new(d.outputs['Color'], pr.inputs['Base Color'])
    r = tex('R', 'Non-Color')
    nt.links.new(r.outputs['Color'], pr.inputs['Roughness'])
    m = tex('M', 'Non-Color')
    nt.links.new(m.outputs['Color'], pr.inputs['Metallic'])
    e = tex('E', 'sRGB')
    nt.links.new(e.outputs['Color'], pr.inputs['Emission Color'])
    pr.inputs['Emission Strength'].default_value = 2.0
    if use_normal:
        n = tex('N', 'Non-Color')
        nm = nt.nodes.new('ShaderNodeNormalMap')
        if flip_green:   # DirectX -> OpenGL for Blender
            sep = nt.nodes.new('ShaderNodeSeparateColor')
            com = nt.nodes.new('ShaderNodeCombineColor')
            inv = nt.nodes.new('ShaderNodeMath')
            inv.operation = 'SUBTRACT'
            inv.inputs[0].default_value = 1.0
            nt.links.new(n.outputs['Color'], sep.inputs[0])
            nt.links.new(sep.outputs[0], com.inputs[0])
            nt.links.new(sep.outputs[1], inv.inputs[1])
            nt.links.new(inv.outputs[0], com.inputs[1])
            nt.links.new(sep.outputs[2], com.inputs[2])
            nt.links.new(com.outputs[0], nm.inputs['Color'])
        else:
            nt.links.new(n.outputs['Color'], nm.inputs['Color'])
        nt.links.new(nm.outputs[0], pr.inputs['Normal'])
    return mat


def cmd_preview(args):
    outdir = opt(args, '--out', os.path.join(os.environ.get('TEMP', '.'), 'unique_hull_preview'))
    os.makedirs(outdir, exist_ok=True)
    spp = opt(args, '--spp', 48, int)
    cases = [
        ("tiled", f"{SHIP}_Assembled.fbx", "Corvette", False, False),
        ("unique", f"{OUT_NAME}.fbx", TEXSET, True, True),
    ]
    views = {
        'threequarter': ((12.0, 9.0, 5.5), (0, 0, 0.0)),
        'side': ((14.0, 0.0, 0.7), (0, 0, 0.0)),
        'top': ((0.5, -2.0, 15.0), (0, 0, 0.0)),
        'rear_low': ((-9.0, -9.5, -2.5), (0, -0.5, 0.2)),
        'close_flank': ((3.6, 1.2, 1.6), (0.0, 0.8, 0.2)),
    }
    for tag, fbx, texset, use_n, flip in cases:
        gen.setup_scene()
        bpy.ops.import_scene.fbx(filepath=os.path.join(gen.BASE, fbx), axis_forward='-Y', axis_up='Z')
        ob = [o for o in bpy.data.objects if o.type == 'MESH'][0]
        s = GAME_LENGTH_M / ob.dimensions.y
        ob.scale = (s, s, s)
        ob.location = (0, 0, 0)
        bpy.context.view_layer.update()
        ob.data.materials.clear()
        ob.data.materials.append(make_preview_mat(f"PV_{tag}", texset, use_n, flip))
        sc = bpy.context.scene
        sc.render.engine = 'CYCLES'
        sc.cycles.samples = spp
        sc.cycles.use_denoising = True
        sc.render.resolution_x, sc.render.resolution_y = 1400, 800
        w = bpy.data.worlds.new("W")
        sc.world = w
        w.use_nodes = True
        bg = w.node_tree.nodes['Background']
        bg.inputs['Color'].default_value = (0.30, 0.34, 0.45, 1)
        bg.inputs['Strength'].default_value = 1.0
        sun = bpy.data.objects.new("Sun", bpy.data.lights.new("Sun", 'SUN'))
        sun.data.energy = 5.0
        sun.rotation_euler = (math.radians(50), math.radians(10), math.radians(135))
        sc.collection.objects.link(sun)
        fill = bpy.data.objects.new("Fill", bpy.data.lights.new("Fill", 'SUN'))
        fill.data.energy = 2.0
        fill.data.color = (0.6, 0.7, 1.0)
        fill.rotation_euler = (math.radians(110), 0, math.radians(-40))
        sc.collection.objects.link(fill)
        cam = bpy.data.objects.new("Cam", bpy.data.cameras.new("Cam"))
        cam.data.lens = 50
        sc.collection.objects.link(cam)
        sc.camera = cam
        for vname, (loc, tgt) in views.items():
            cam.location = loc
            d = Vector(tgt) - Vector(loc)
            cam.rotation_euler = d.to_track_quat('-Z', 'Y').to_euler()
            if vname == 'close_flank':
                cam.data.lens = 70
            else:
                cam.data.lens = 50
            sc.render.filepath = os.path.join(outdir, f"{tag}_{vname}.png")
            bpy.ops.render.render(write_still=True)
            print("RENDERED", sc.render.filepath, flush=True)


if __name__ == "__main__":
    a = argv_after_dashes()
    cmd = a[0] if a else 'build'
    {'build': cmd_build, 'preview': cmd_preview}[cmd](a[1:])
