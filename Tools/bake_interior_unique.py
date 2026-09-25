"""Unique-UV + Cycles-baked PBR set for a walkable interior kit (hero pass).

The interior sibling of Tools/build_unique_hull_textures.py: the same recipe
the ship EXTERIORS get, applied to a room instead of a hull.

  1. visibility cull: every face is ray-tested from its own surface; faces that
     look out of the room (wall backs, slab undersides) or into another solid
     (furniture feet in the deck) are never seen, so they get no texels.
  2. a real unwrap of every visible surface of the kit into shared atlases
     (Smart UV Project -> average island scale -> concave pack), so no two
     square centimetres share texels and nothing repeats.
  3. object-space procedural masks BAKED IN CYCLES from the actual geometry:
     AO, bevel-edge wear, panel seams, per-plate tone, grime, foot traffic,
     livery (dado, gold trim, hazard chevrons) keyed by the original slot.
  4. composed in numpy into _D _N(DirectX) _R _M _AO _E, then the faces that
     were baked are re-slotted onto one material per atlas:
        M_Int_<Tag>_Room  (Shell, Deck, Hatch, Viewport frames)
        M_Int_<Tag>_Kit   (Console + Stations furniture)
     Screens (M_Int_Console), light strips (M_Int_Lights) and glass
     (M_Int_Viewport) keep their shared library slots and UVs.

Called from Tools/build_capital_interiors.py for kits in HERO_KITS, between
contract validation and FBX export. Design units are centimetres.

Options (after `--` on the blender command line):
  --size N      atlas size (default 4096)
  --rebake      ignore the bake cache (%TEMP%/interior_unique_cache/<prefix>)
  --gpu         bake on the GPU if Cycles has one
"""
import os
import sys
import math
import json
import time

import bpy
import bmesh
import numpy as np
from mathutils import Vector
from mathutils.bvhtree import BVHTree

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TEXOUT = os.path.join(ROOT, "Assets", "Textures", "generated", "interiors")
M_PER_BU = 0.01                      # design units are cm

# Atlases: Floor (walkable deck), Room (walls, ceiling, structure), Kit (furniture).
# Original slot -> class id baked into the masks (drives palette + livery).
CLASS = {"M_Int_Shell": 0, "M_Int_Deck": 1, "M_Int_Vents": 2, "M_Int_Trim": 3,
         "M_Int_Hatch": 4, "M_Int_Stations": 5, "M_Int_Command": 6}
UNIQUE_SLOTS = set(CLASS)

# CommandXL livery: the hull is deep navy/indigo plating with gold trim and a
# warm-gold emissive (Tools/regen_ship_hull_textures_v2.py "Command"). Inside,
# the same palette is lifted a little so a lit room still reads.
PALETTE = {
    "wall":     (0.44, 0.46, 0.52),   # upper bulkhead, pale blue-grey
    "dado":     (0.13, 0.15, 0.25),   # lower wall, hull navy
    "ceiling":  (0.33, 0.34, 0.38),
    "floor":    (0.15, 0.16, 0.19),
    "vents":    (0.10, 0.10, 0.12),
    "trim":     (0.66, 0.51, 0.18),   # command gold
    "hatch":    (0.13, 0.13, 0.15),
    "hazard":   (0.88, 0.66, 0.08),
    "stations": (0.15, 0.18, 0.29),
    "command":  (0.07, 0.09, 0.18),
    "bare":     (0.55, 0.56, 0.60),
    "gold_emis": (1.0, 0.78, 0.35),
}


def opt(args, name, default=None, cast=str):
    return cast(args[args.index(name) + 1]) if name in args else default


# ----------------------------------------------------------------------------
# 1. visibility + class attribute
# ----------------------------------------------------------------------------
def build_bvh(objs):
    verts, polys = [], []
    for ob in objs:
        base = len(verts)
        M = ob.matrix_world
        verts.extend(M @ v.co for v in ob.data.vertices)
        polys.extend([base + i for i in p.vertices] for p in ob.data.polygons)
    return BVHTree.FromPolygons(verts, polys, epsilon=0.0)


def face_visibility(ob, tree, eps=0.05):
    """True where a ray off the face (centre or 3/4 toward any vertex) lands on
    a FRONT face, i.e. the face can be seen from somewhere inside the room.
    Escaping rays = facing out of the room; back-face hits = buried."""
    me = ob.data
    vis = []
    for p in me.polygons:
        n = p.normal
        vs = [me.vertices[i].co for i in p.vertices]
        c = p.center
        pts = [c] + [c.lerp(v, 0.9) for v in vs]
        # big faces: a barycentric grid (~12 cm pitch), so a face that is only
        # partly covered (e.g. a deck plate under a console) still counts
        if len(vs) == 3 and p.area > 150.0:
            k = min(24, int(math.ceil(math.sqrt(p.area) / 12.0)))
            a, b, cc = vs
            for i in range(k + 1):
                for j in range(k + 1 - i):
                    u, v = (i + 0.33) / (k + 1), (j + 0.33) / (k + 1)
                    if u + v < 1.0:
                        pts.append(a + (b - a) * u + (cc - a) * v)
        seen = False
        for q in pts:
            hit = tree.ray_cast(q + n * eps, n)
            if hit[0] is not None and hit[1].dot(n) < 0.0:
                seen = True
                break
        vis.append(seen)
    return vis


# ----------------------------------------------------------------------------
# 2. unwrap
# ----------------------------------------------------------------------------
def unwrap_atlas(objs, masks, size, margin_px=4):
    """Pack the masked faces of all objs into one shared 0-1 'Atlas' layer.

    The operators see only temporary copies that contain just the masked
    faces (hide/selection tricks leak unseen faces into the pack), then the
    UVs are copied back face-by-face (bmesh keeps loop order on delete)."""
    tmps = []
    for ob in objs:
        bm = bmesh.new()
        bm.from_mesh(ob.data)
        bm.faces.ensure_lookup_table()
        orig = bm.faces.layers.int.new("orig")
        for f in bm.faces:
            f[orig] = f.index
        # drop unmasked faces AND zero-area slivers (bevel/grid-cut leftovers):
        # smart_project can hand those NaN/garbage UVs that poison the pack
        bmesh.ops.delete(bm, geom=[f for f, m in zip(bm.faces, masks[ob.name])
                                   if not m or f.calc_area() < 1e-3], context='FACES')
        me = bpy.data.meshes.new(ob.name + "_uvtmp")
        bm.to_mesh(me)
        bm.free()
        while me.uv_layers:
            me.uv_layers.remove(me.uv_layers[0])
        me.uv_layers.new(name="Atlas")
        t = bpy.data.objects.new(me.name, me)
        bpy.context.scene.collection.objects.link(t)
        tmps.append(t)
    bpy.ops.object.select_all(action='DESELECT')
    for t in tmps:
        t.select_set(True)
    bpy.context.view_layer.objects.active = tmps[0]
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.context.scene.tool_settings.use_uv_select_sync = True
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.uv.smart_project(angle_limit=math.radians(66), margin_method='SCALED',
                             rotate_method='AXIS_ALIGNED_Y', island_margin=0.0,
                             area_weight=0.0, correct_aspect=True, scale_to_bounds=False)
    bpy.ops.uv.select_all(action='SELECT')
    bpy.ops.uv.average_islands_scale()
    # Blender's packer is heuristic and its result varies run to run (the
    # same kit has landed anywhere from 55% to 90% of the best density), so
    # try a few strategies and keep the one with the most texels per metre.
    bpy.ops.object.mode_set(mode='OBJECT')
    start = {}
    for t in tmps:
        a = _get_uv(t)
        bad = ~np.isfinite(a)
        if bad.any():
            print(f"  {t.name}: {int(bad.sum())} non-finite UV coords from smart_project -> 0", flush=True)
            a[bad] = 0.0
        start[t.name] = a
    best, best_score = None, -1.0
    for shape, rot in (('CONCAVE', 'CARDINAL'), ('CONCAVE', 'ANY'), ('CONVEX', 'ANY'),
                       ('AABB', 'CARDINAL'), ('CONCAVE', 'CARDINAL')):
        for t in tmps:
            _set_uv(t, start[t.name])
        bpy.ops.object.mode_set(mode='EDIT')
        bpy.ops.mesh.select_all(action='SELECT')
        bpy.ops.uv.select_all(action='SELECT')
        bpy.ops.uv.pack_islands(rotate=True, rotate_method=rot, scale=True,
                                margin_method='FRACTION', margin=margin_px / size,
                                shape_method=shape)
        bpy.ops.object.mode_set(mode='OBJECT')
        uvs = [_get_uv(t) for t in tmps]
        valid = all(np.isfinite(a).all() and a.min() >= -1e-4 and a.max() <= 1.0 + 1e-4 for a in uvs)
        score = sum(_uv_area(t) for t in tmps)
        valid = valid and score <= 1.0
        print(f"  pack {shape}/{rot}: uv coverage {score:.3f}{'' if valid else ' (rejected: invalid layout)'}",
              flush=True)
        if valid and score > best_score:
            best, best_score = {t.name: _get_uv(t) for t in tmps}, score
    if best is None:
        raise RuntimeError("every UV pack attempt produced an invalid layout")
    for t in tmps:
        _set_uv(t, best[t.name])
    for ob, t in zip(objs, tmps):
        me = ob.data
        if "Atlas" in me.uv_layers:
            me.uv_layers.remove(me.uv_layers["Atlas"])
        dst = me.uv_layers.new(name="Atlas").data
        src = t.data.uv_layers["Atlas"].data
        orig = t.data.attributes["orig"].data
        for tp in t.data.polygons:
            op = me.polygons[orig[tp.index].value]
            for tl, ol in zip(tp.loop_indices, op.loop_indices):
                dst[ol].uv = src[tl].uv
        bpy.data.objects.remove(t)
        bpy.data.meshes.remove(bpy.data.meshes[ob.name + "_uvtmp"])


def _get_uv(ob):
    a = np.empty(len(ob.data.loops) * 2, dtype=np.float32)
    ob.data.uv_layers["Atlas"].data.foreach_get("uv", a)
    return a


def _set_uv(ob, a):
    ob.data.uv_layers["Atlas"].data.foreach_set("uv", a)


def _uv_area(ob):
    uv = _get_uv(ob).reshape(-1, 2)
    tot = 0.0
    for p in ob.data.polygons:
        q = uv[p.loop_start:p.loop_start + p.loop_total]
        x, y = q[:, 0], q[:, 1]
        tot += 0.5 * abs(float(np.dot(x, np.roll(y, -1)) - np.dot(y, np.roll(x, -1))))
    return tot


def atlas_stats_layer(objs, masks, size, layer):
    dens, wts = [], []
    for ob in objs:
        me = ob.data
        uv = me.uv_layers[layer].data
        for p, m in zip(me.polygons, masks[ob.name]):
            if not m or p.area < 1e-6:
                continue
            pts = [uv[l].uv for l in p.loop_indices]
            a = 0.0
            for i in range(len(pts)):
                x1, y1 = pts[i]
                x2, y2 = pts[(i + 1) % len(pts)]
                a += x1 * y2 - x2 * y1
            a = abs(a) * 0.5
            if a > 0:
                dens.append(math.sqrt(a * size * size / (p.area * M_PER_BU ** 2)))
                wts.append(p.area)
    d, w = np.array(dens), np.array(wts)
    return {"px_per_m_area_weighted_mean": float((d * w).sum() / w.sum()),
            "px_per_m_median": float(np.median(d)), "px_per_m_p5": float(np.percentile(d, 5)),
            "baked_area_m2": float(w.sum() * M_PER_BU ** 2)}


# ----------------------------------------------------------------------------
# 3. bake material (object-space procedural masks, metres)
# ----------------------------------------------------------------------------
class Shader:
    def __init__(self, nt):
        self.nt, self.n = nt, nt.nodes

    def _set(self, node, key, v):
        if v is None:
            return
        if isinstance(v, bpy.types.NodeSocket):
            self.nt.links.new(v, node.inputs[key])
        else:
            node.inputs[key].default_value = v

    def math(self, op, a, b=None, clamp=False, c=None):
        n = self.n.new('ShaderNodeMath')
        n.operation, n.use_clamp = op, clamp
        self._set(n, 0, a); self._set(n, 1, b); self._set(n, 2, c)
        return n.outputs[0]

    def vmath(self, op, a, b=None):
        n = self.n.new('ShaderNodeVectorMath')
        n.operation = op
        self._set(n, 0, a)
        self._set(n, 3 if op == 'SCALE' else 1, b)
        return n.outputs['Value'] if op in ('DOT_PRODUCT', 'LENGTH', 'DISTANCE') else n.outputs['Vector']

    def combine(self, x, y, z):
        n = self.n.new('ShaderNodeCombineXYZ')
        self._set(n, 0, x); self._set(n, 1, y); self._set(n, 2, z)
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

    def brick(self, vec, w, h, mortar, smooth=0.002, offset=0.5, freq=2):
        n = self.n.new('ShaderNodeTexBrick')
        n.offset, n.offset_frequency = offset, freq
        n.squash, n.squash_frequency = 1.0, 99
        n.inputs['Color1'].default_value = (0, 0, 0, 1)
        n.inputs['Color2'].default_value = (1, 1, 1, 1)
        n.inputs['Mortar'].default_value = (0, 0, 0, 1)
        n.inputs['Scale'].default_value = 1.0
        n.inputs['Mortar Size'].default_value = mortar
        n.inputs['Mortar Smooth'].default_value = smooth
        n.inputs['Bias'].default_value = 0.0
        n.inputs['Brick Width'].default_value = w
        n.inputs['Row Height'].default_value = h
        self.nt.links.new(vec, n.inputs['Vector'])
        return n.outputs['Fac'], self.separate(n.outputs['Color'])[0]   # (mortar=1, per-brick rnd)

    def eq(self, a, v):
        return self.math('LESS_THAN', self.math('ABSOLUTE', self.math('SUBTRACT', a, v)), 0.5)

    def band(self, a, lo, hi):
        return self.math('MULTIPLY', self.math('GREATER_THAN', a, lo), self.math('LESS_THAN', a, hi))

    def mix(self, a, b, t):
        """a*(1-t) + b*t for scalar sockets/values."""
        return self.math('ADD', self.math('MULTIPLY', a, self.math('SUBTRACT', 1.0, t)),
                         self.math('MULTIPLY', b, t))


def build_bake_material(room):
    mat = bpy.data.materials.new("BAKE_INT_TMP")
    mat.use_nodes = True
    nt = mat.node_tree
    nt.nodes.clear()
    S = Shader(nt)
    out = nt.nodes.new('ShaderNodeOutputMaterial')
    emit = nt.nodes.new('ShaderNodeEmission')
    nt.links.new(emit.outputs[0], out.inputs[0])

    geo = nt.nodes.new('ShaderNodeNewGeometry')
    P = S.vmath('SCALE', geo.outputs['Position'], M_PER_BU)          # metres
    px, py, pz = S.separate(P)
    nx, ny, nz = S.separate(geo.outputs['True Normal'])
    ax, ay, az = (S.math('ABSOLUTE', v) for v in (nx, ny, nz))
    at = nt.nodes.new('ShaderNodeAttribute')
    at.attribute_type = 'GEOMETRY'
    at.attribute_name = 'cls'
    cls = at.outputs['Fac']

    selX = S.math('MULTIPLY', S.math('GREATER_THAN', ax, ay), S.math('GREATER_THAN', ax, az))
    selY = S.math('MULTIPLY', S.math('SUBTRACT', 1.0, selX), S.math('GREATER_THAN', ay, az))
    selZ = S.math('SUBTRACT', S.math('SUBTRACT', 1.0, selX), selY)

    def planar(fn, off=(0.0, 0.0)):
        ox, oy = off
        c_x = S.combine(S.math('ADD', py, ox), S.math('ADD', pz, oy), 0.0)
        c_y = S.combine(S.math('ADD', px, ox * 1.7 + 0.31), S.math('ADD', pz, oy), 0.0)
        c_z = S.combine(S.math('ADD', px, ox * 0.6 + 0.77), S.math('ADD', py, oy * 1.3), 0.0)
        rs = [fn(c) for c in (c_x, c_y, c_z)]
        return [S.math('ADD', S.math('ADD', S.math('MULTIPLY', rs[0][i], selX),
                                     S.math('MULTIPLY', rs[1][i], selY)),
                       S.math('MULTIPLY', rs[2][i], selZ)) for i in range(len(rs[0]))]

    # --- zones -------------------------------------------------------------
    is_furn = S.math('GREATER_THAN', cls, 4.5)                        # Stations / Command
    is_floor = S.math('MULTIPLY', S.math('GREATER_THAN', nz, 0.6), S.math('LESS_THAN', pz, 0.12))
    is_ceil = S.math('MULTIPLY', S.math('LESS_THAN', nz, -0.6), S.math('GREATER_THAN', pz, 3.2))
    is_wall = S.math('MULTIPLY', S.math('LESS_THAN', az, 0.6), S.math('LESS_THAN', cls, 0.5))
    dado = S.math('MULTIPLY', is_wall, S.math('LESS_THAN', pz, 1.04))

    # --- panel hierarchy (metres, human scale) -------------------------------
    # walls: 1.2 x 0.9 m staggered plates; deck/ceiling: 1.0 m square grid
    def pw(c):
        return S.brick(c, 1.2, 0.9, 0.012, offset=0.5, freq=2)
    gW, rW = planar(pw, (0.13, 0.08))
    def pf(c):
        return S.brick(c, 1.0, 1.0, 0.010, offset=0.0, freq=1)
    gF, rF = planar(pf, (0.0, 0.0))
    # furniture: aligned service panels 0.5 x 0.36 m (a stagger reads as
    # brickwork at chair scale)
    def ps(c):
        return S.brick(c, 0.5, 0.36, 0.004, offset=0.0, freq=1)
    gS, rS = planar(ps, (0.51, 0.09))
    horiz = S.math('GREATER_THAN', az, 0.6)
    gA = S.mix(S.mix(gW, gF, horiz), gS, is_furn)
    plate = S.mix(S.mix(rW, rF, horiz), rS, is_furn)
    # secondary sub-panels only in noise regions (breaks the grid, as on the hulls)
    def pb(c):
        return S.brick(c, 0.4, 0.3, 0.004, offset=0.5, freq=2)
    gB_raw, _ = planar(pb, (0.27, 0.41))
    region = S.math('GREATER_THAN', S.noise(S.vmath('ADD', P, (3.1, 7.7, 1.3)), 0.8, 2.0), 0.5)
    gB = S.math('MULTIPLY', S.math('MULTIPLY', gB_raw, region), S.math('SUBTRACT', 1.0, is_furn))
    # deck grip: diamond tread plate in the aisle + crew-arc walkway plates
    tread = S.math('GREATER_THAN', S.math('MULTIPLY',
                                          S.math('SINE', S.math('MULTIPLY', S.math('ADD', px, py), 120.0)),
                                          S.math('SINE', S.math('MULTIPLY', S.math('SUBTRACT', px, py), 120.0))), 0.55)
    aisle = S.math('MULTIPLY', S.math('LESS_THAN', S.math('ABSOLUTE', py), 0.68),
                   S.math('LESS_THAN', px, -2.18))
    tread = S.math('MULTIPLY', S.math('MULTIPLY', tread, aisle), is_floor)

    # --- noises --------------------------------------------------------------
    drift = S.noise(S.vmath('ADD', P, (11.0, 2.0, 5.0)), 0.35, 4.0, 0.6)
    blot = S.noise(S.vmath('ADD', P, (1.0, 19.0, 3.0)), 1.4, 5.0, 0.6)
    streak_v = S.combine(S.math('MULTIPLY', px, 5.0), S.math('MULTIPLY', py, 5.0),
                         S.math('MULTIPLY', pz, 0.4))          # grime runs DOWN walls
    streak = S.noise(S.vmath('ADD', streak_v, (5.0, 5.0, 0.0)), 1.8, 3.0, 0.55)
    chip = S.noise(S.vmath('ADD', P, (2.0, 3.0, 9.0)), 45.0, 3.0, 0.6)
    micro = S.noise(P, 260.0, 2.0, 0.5)

    # --- foot traffic: aisle, dais approach, helm, crew arc, side banks -------
    dxd = S.math('SUBTRACT', px, -0.40)
    rdais = S.math('SQRT', S.math('ADD', S.math('MULTIPLY', dxd, dxd), S.math('MULTIPLY', py, py)))
    rarc = S.math('SQRT', S.math('ADD', S.math('MULTIPLY', px, px), S.math('MULTIPLY', py, py)))
    t_aisle = S.math('MULTIPLY', S.math('LESS_THAN', S.math('ABSOLUTE', py), 0.9), S.math('LESS_THAN', px, 0.0))
    t_dais = S.band(rdais, 1.75, 2.6)
    t_helm = S.math('MULTIPLY', S.band(px, 1.9, 3.1), S.math('LESS_THAN', S.math('ABSOLUTE', py), 1.3))
    t_arc = S.band(rarc, 2.7, 3.3)
    t_side = S.math('GREATER_THAN', S.math('ABSOLUTE', py), 3.1)
    tr = S.math('MAXIMUM', S.math('MAXIMUM', t_aisle, t_dais), S.math('MAXIMUM', t_helm, S.math('MAXIMUM', t_arc, t_side)))
    tr_n = S.noise(S.vmath('ADD', P, (7.0, 1.0, 2.0)), 2.2, 3.0, 0.6)
    traffic = S.math('MULTIPLY', S.math('MULTIPLY', tr, is_floor),
                     S.math('ADD', 0.35, S.math('MULTIPLY', tr_n, 0.9)), clamp=True)

    # --- livery -------------------------------------------------------------
    # hatch frame + threshold: yellow/black 45deg chevrons
    diag = S.math('FLOORED_MODULO', S.math('MULTIPLY', S.math('ADD', S.math('ADD', px, py), pz), 5.0), 1.0)
    stripes = S.math('LESS_THAN', diag, 0.5)
    hatch_frame = S.math('MULTIPLY', S.eq(cls, 4.0),
                         S.math('MAXIMUM', S.math('GREATER_THAN', pz, 2.44),
                                S.math('GREATER_THAN', S.math('ABSOLUTE', py), 0.83)))
    threshold = S.math('MULTIPLY', S.math('MULTIPLY', is_floor, S.math('LESS_THAN', px, -4.25)),
                       S.math('LESS_THAN', S.math('ABSOLUTE', py), 1.0))
    haz = S.math('MULTIPLY', S.math('MAXIMUM', hatch_frame, threshold), stripes)
    # gold band + top-edge on command furniture (flagship furniture = hull livery)
    cmd = S.eq(cls, 6.0)
    goldband = S.math('MULTIPLY', cmd, S.math('MULTIPLY', S.band(pz, 0.62, 0.65), S.math('LESS_THAN', az, 0.5)))
    # floor guide lines: thin gold line either side of the aisle + dais ring line
    fl1 = S.math('MULTIPLY', aisle, S.band(S.math('ABSOLUTE', py), 0.60, 0.63))
    fl2 = S.band(rdais, 2.20, 2.24)
    floorline = S.math('MULTIPLY', S.math('MAXIMUM', fl1, fl2), is_floor)
    # stencil blocks on the upper wall above the trim, every other plate
    stencil = S.math('MULTIPLY', S.math('MULTIPLY', is_wall, S.band(pz, 1.24, 1.34)),
                     S.math('GREATER_THAN', rW, 0.72))

    # --- convex edges (bevel node) + inside AO; thickness ---------------------
    bev = nt.nodes.new('ShaderNodeBevel')
    bev.inputs['Radius'].default_value = 1.2          # cm = the geometric bevel width
    bev.samples = 8
    nt.links.new(geo.outputs['True Normal'], bev.inputs['Normal'])
    dn = S.vmath('DOT_PRODUCT', bev.outputs['Normal'], geo.outputs['True Normal'])
    edge_b = S.math('MULTIPLY', S.math('SUBTRACT', 1.0, dn), 10.0, clamp=True)
    aoi = nt.nodes.new('ShaderNodeAmbientOcclusion')
    aoi.inside = True
    aoi.samples = 8
    aoi.inputs['Distance'].default_value = 1.5
    nt.links.new(geo.outputs['True Normal'], aoi.inputs['Normal'])
    edge = S.math('MAXIMUM', edge_b, S.math('MULTIPLY', S.math('SUBTRACT', 1.0, aoi.outputs['AO']), 1.4, clamp=True),
                  clamp=True)
    aot = nt.nodes.new('ShaderNodeAmbientOcclusion')
    aot.inside = True
    aot.samples = 8
    aot.inputs['Distance'].default_value = 12.0
    nt.links.new(geo.outputs['True Normal'], aot.inputs['Normal'])
    thick = aot.outputs['AO']

    # rivets: 6 cm pitch, band along wall-plate seams
    def riv(c):
        fac, _ = S.brick(c, 1.2, 0.9, 0.06, offset=0.5, freq=2)
        cx, cy, _ = S.separate(c)
        k = 2 * math.pi / 0.06
        d = S.math('MULTIPLY', S.math('SINE', S.math('MULTIPLY', cx, k)), S.math('SINE', S.math('MULTIPLY', cy, k)))
        return (S.math('MULTIPLY', S.math('GREATER_THAN', d, 0.93), fac),)
    rivets, = planar(riv, (0.13, 0.08))
    rivets = S.math('MULTIPLY', S.math('MULTIPLY', rivets, S.math('SUBTRACT', 1.0, gW)), is_wall)

    def pack(r, g, b):
        cn = nt.nodes.new('ShaderNodeCombineColor')
        for i, v in enumerate((r, g, b)):
            S._set(cn, i, v)
        return cn.outputs[0]

    channels = {
        'P1': pack(gA, gB, plate),
        'P2': pack(edge, thick, rivets),
        'P3': pack(drift, blot, streak),
        'P4': pack(chip, micro, traffic),
        'P5': pack(S.math('DIVIDE', cls, 10.0), haz, tread),
        'P6': pack(dado, is_ceil, is_floor),
        'P7': pack(goldband, floorline, stencil),
    }
    img_node = nt.nodes.new('ShaderNodeTexImage')
    nt.nodes.active = img_node
    return mat, channels, emit, img_node


def bake_atlas(objs, size, cache, rebake, room, ao_samples=64):
    scene = bpy.context.scene
    scene.render.engine = 'CYCLES'
    scene.cycles.use_denoising = False
    scene.render.bake.margin = 12
    scene.render.bake.margin_type = 'EXTEND'
    scene.render.bake.use_clear = True
    if not scene.world:
        scene.world = bpy.data.worlds.new("W")
    scene.world.use_nodes = True
    mat, channels, emit, img_node = build_bake_material(room)
    nt = mat.node_tree
    saved = {}
    for ob in objs:
        saved[ob.name] = list(ob.data.materials)
        for i in range(len(ob.data.materials)):
            ob.data.materials[i] = mat
    bpy.ops.object.select_all(action='DESELECT')
    for ob in objs:
        ob.select_set(True)
    bpy.context.view_layer.objects.active = objs[0]
    res, timings = {}, {}

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
        print(f"BAKE {os.path.basename(cache)}/{name}: {timings[name]}s", flush=True)

    for name, sock in channels.items():
        for l in list(nt.links):
            if l.to_node == emit and l.to_socket.name == 'Color':
                nt.links.remove(l)
        nt.links.new(sock, emit.inputs['Color'])
        do_bake(name, 'EMIT', 24 if name == 'P2' else 4)
    scene.world.light_settings.distance = 0.5 / M_PER_BU        # 50 cm AO rays
    do_bake('AO', 'AO', ao_samples)
    # coverage mask: which texels belong to a baked face (for margin-safe stats)
    for ob in objs:
        for i, m in enumerate(saved[ob.name]):
            ob.data.materials[i] = m
    bpy.data.materials.remove(mat)
    return res, timings


# ----------------------------------------------------------------------------
# 4. compose
# ----------------------------------------------------------------------------
def box_blur(a, r):
    if r <= 0:
        return a
    k = 2 * r + 1
    for ax in (0, 1):
        c = np.cumsum(np.pad(a, [(r + 1, r) if i == ax else (0, 0) for i in range(a.ndim)], mode='edge'),
                      axis=ax, dtype=np.float64)
        hi = [slice(None)] * a.ndim
        lo = [slice(None)] * a.ndim
        hi[ax], lo[ax] = slice(k, None), slice(0, -k)
        a = ((c[tuple(hi)] - c[tuple(lo)]) / k).astype(np.float32)
    return a


def smoothstep(e0, e1, x):
    t = np.clip((x - e0) / (e1 - e0), 0.0, 1.0)
    return t * t * (3 - 2 * t)


def compose(B, px_per_m):
    C = {k: np.array(v, dtype=np.float32) for k, v in PALETTE.items()}
    gA, gB, plate = (B['P1'][..., i] for i in range(3))
    edge, thick, rivet = (B['P2'][..., i] for i in range(3))
    drift, blot, streak = (B['P3'][..., i] for i in range(3))
    chip, micro, traffic = (B['P4'][..., i] for i in range(3))
    clsf, haz, tread = (B['P5'][..., i] for i in range(3))
    dado, ceil, floor = (B['P6'][..., i] for i in range(3))
    goldband, floorline, stencil = (B['P7'][..., i] for i in range(3))
    AOb = B['AO'][..., 0]
    cls = np.rint(clsf * 10.0).astype(np.int8)
    gA, gB = box_blur(gA, 1), box_blur(gB, 1)
    edge = box_blur(edge, 1)
    riv = box_blur(rivet, 1)

    def m(k):
        return (cls == k).astype(np.float32)[..., None]
    wall_m, deck_m, vent_m, trim_m = m(0), m(1), m(2), m(3)
    hatch_m, stat_m, cmd_m = m(4), m(5), m(6)

    # -- base colour by class / zone ---------------------------------------
    shell = C['wall'] * (1 - dado[..., None]) + C['dado'] * dado[..., None]
    shell = shell * (1 - ceil[..., None]) + C['ceiling'] * ceil[..., None]
    shell = shell * (1 - floor[..., None]) + C['floor'] * floor[..., None]
    D = (wall_m * shell + deck_m * C['floor'] + vent_m * C['vents'] + trim_m * C['trim']
         + hatch_m * C['hatch'] + stat_m * C['stations'] + cmd_m * C['command'])
    painted = 1.0 - trim_m[..., 0]

    # -- wear / dirt --------------------------------------------------------
    chip_m = smoothstep(0.42, 0.62, chip + (micro - 0.5) * 0.3)
    size_w = smoothstep(0.30, 0.80, thick)
    wear = smoothstep(0.30, 0.80, np.clip(edge * size_w * (0.25 + 0.55 * chip_m), 0, 1)) * painted
    cavity = 1.0 - AOb
    dirt = np.clip(cavity * 1.3, 0, 1) * (0.30 + 0.70 * smoothstep(0.35, 0.7, blot))
    dirt += 0.35 * smoothstep(0.55, 0.75, streak) * (1 - floor) * (1 - ceil)
    dirt = np.clip(dirt, 0, 1) * (1 - wear)
    scuff = traffic * smoothstep(0.35, 0.65, micro + (chip - 0.5) * 0.6)

    tone = 1.0 + 0.16 * (drift - 0.5) + 0.12 * (plate - 0.5)
    D = D * tone[..., None]
    hue = (drift - 0.5) * 0.06
    D[..., 0] *= (1 + hue)
    D[..., 2] *= (1 - hue)
    D *= (1 - 0.06 * (micro[..., None] - 0.5))
    # livery
    hz = haz[..., None]
    D = D * (1 - hz) + C['hazard'] * hz
    gl = np.clip(goldband + floorline, 0, 1)[..., None]
    D = D * (1 - gl) + C['trim'] * gl
    D = D * (1 - 0.45 * stencil[..., None]) + C['dado'] * 0.9 * (0.45 * stencil[..., None])
    D *= (1 - 0.45 * gA)[..., None] * (1 - 0.30 * gB)[..., None]
    D *= (1 - 0.35 * dirt)[..., None] * np.array([1.0, 0.97, 0.93], dtype=np.float32)
    # worn edges: bare steel on paint, gold shows through on command furniture
    bare = C['bare'][None, None, :] * (1 - cmd_m) + C['trim'] * 0.9 * cmd_m
    D = D * (1 - wear[..., None]) + bare * wear[..., None]
    D = D * (1 - 0.35 * scuff[..., None]) + C['bare'] * 0.6 * (0.35 * scuff[..., None])
    D = D * (1 - 0.5 * riv[..., None]) + C['bare'] * 0.8 * (0.5 * riv[..., None])
    D *= (0.70 + 0.30 * AOb)[..., None]
    D = np.clip(D, 0, 1)

    # -- height -> normal (DirectX, UE convention) ------------------------------
    h = (-0.0030 * gA - 0.0012 * gB + 0.0004 * (plate - 0.5) - 0.0002 * micro
         + 0.0006 * tread + 0.0008 * wear + 0.0006 * riv)
    px = 1.0 / px_per_m
    gy, gx = np.gradient(h, px)
    nrm = np.stack([-gx, gy, np.ones_like(gx)], axis=-1)
    nrm /= np.linalg.norm(nrm, axis=-1, keepdims=True)
    N = nrm * 0.5 + 0.5

    # -- R / M -------------------------------------------------------------------
    rough_cls = (wall_m * 0.55 + deck_m * 0.62 + vent_m * 0.6 + trim_m * 0.28 + hatch_m * 0.5
                 + stat_m * 0.45 + cmd_m * 0.36)[..., 0]
    R = rough_cls + 0.08 * (plate - 0.5) + 0.06 * (micro - 0.5)
    R = R + 0.25 * dirt - 0.18 * scuff + 0.2 * gA
    R = np.where(wear > 0.02, R * (1 - wear) + 0.32 * wear, R)
    R = R * (1 - gl[..., 0]) + 0.28 * gl[..., 0]
    R = np.clip(R, 0.08, 1.0)
    metal_cls = (wall_m * 0.15 + deck_m * 0.55 + vent_m * 0.6 + trim_m * 1.0 + hatch_m * 0.35
                 + stat_m * 0.2 + cmd_m * 0.25)[..., 0]
    M = metal_cls - 0.2 * dirt
    M = M * (1 - wear) + 0.95 * wear
    M = M * (1 - scuff * 0.5) + 0.9 * scuff * 0.5
    M = M * (1 - gl[..., 0]) + 1.0 * gl[..., 0]
    M = M * (1 - haz) + 0.1 * haz
    M = np.clip(M + 0.4 * riv, 0, 1)

    AO = np.clip(AOb * (1 - 0.30 * gA) * (1 - 0.12 * gB), 0, 1)

    # -- E: faint gold seam glow on command furniture + gold floor guide lines --
    E = np.zeros(D.shape, dtype=np.float32)
    E += (gA * cmd_m[..., 0] * smoothstep(0.55, 0.9, plate))[..., None] * C['gold_emis'] * 0.35
    E += floorline[..., None] * C['gold_emis'] * 0.25
    E = np.clip(E, 0, 1)
    return D, N, R, M, AO, E, {"wear_mean": float(wear.mean()), "dirt_mean": float(dirt.mean())}


def write_detail_normal(path, size=1024, seed=7):
    """Tileable micro-surface normal (paint orange-peel + fine scratches) for
    the UE material's close-range detail layer. The unique atlas carries all
    the macro story at ~2 texels/cm; this carries the sub-centimetre texture a
    wall 40 cm from the camera needs. Periodic by construction (FFT noise,
    wrapped scratches). DirectX green."""
    rng = np.random.default_rng(seed)
    f = np.fft.fftfreq(size)
    fr = np.sqrt(f[:, None] ** 2 + f[None, :] ** 2)

    def band(lo, hi):
        spec = np.fft.fft2(rng.standard_normal((size, size)))
        spec *= np.exp(-((fr - (lo + hi) / 2) / ((hi - lo) / 2 + 1e-6)) ** 2)
        a = np.real(np.fft.ifft2(spec))
        return a / (np.abs(a).max() + 1e-9)
    h = 0.55 * band(0.02, 0.06) + 0.30 * band(0.08, 0.2) + 0.15 * band(0.25, 0.45)
    # scratches: short straight wrapped strokes, mostly one direction (wiped panels)
    sc = np.zeros((size, size), dtype=np.float32)
    for _ in range(260):
        x0, y0 = rng.uniform(0, size, 2)
        ang = rng.normal(0.35, 0.5)
        ln = rng.uniform(20, 140)
        t = np.linspace(0, ln, int(ln * 2))
        xs = ((x0 + np.cos(ang) * t) % size).astype(int)
        ys = ((y0 + np.sin(ang) * t) % size).astype(int)
        sc[ys, xs] = np.maximum(sc[ys, xs], rng.uniform(0.3, 1.0))
    h = h * 0.6 - box_blur(sc, 1) * 0.9
    gy = (np.roll(h, -1, 0) - np.roll(h, 1, 0)) * 0.5
    gx = (np.roll(h, -1, 1) - np.roll(h, 1, 1)) * 0.5
    k = 1.6
    nrm = np.stack([-gx * k, gy * k, np.ones_like(h)], axis=-1)
    nrm /= np.linalg.norm(nrm, axis=-1, keepdims=True)
    return write_png(path, (nrm * 0.5 + 0.5).astype(np.float32), False)


def write_png(path, arr, srgb):
    size = arr.shape[0]
    if arr.ndim == 2:
        arr = np.stack([arr] * 3, axis=-1)
    rgba = np.ones((size, size, 4), dtype=np.float32)
    rgba[..., :3] = arr
    name = os.path.basename(path)
    img = bpy.data.images.new(name, size, size, alpha=False)
    img.colorspace_settings.name = 'sRGB' if srgb else 'Non-Color'
    img.pixels.foreach_set(rgba.ravel())
    img.filepath_raw = path
    img.file_format = 'PNG'
    img.save()
    bpy.data.images.remove(img)
    return path


# ----------------------------------------------------------------------------
# 5. atlas assignment, re-slot + finalize UVs
# ----------------------------------------------------------------------------
ROOM_PARTS = ("Shell", "Deck", "Hatch", "Viewport")
KIT_PARTS = ("Console", "Stations")


def atlas_of(part, cls, poly):
    """Which atlas a face is baked into (None = keeps its library slot).
    The walkable floor gets its own atlas: it is the surface seen most and
    closest, so it gets the highest texel density."""
    if cls < 0:
        return None
    if part in KIT_PARTS:
        return "Kit"
    if part not in ROOM_PARTS:
        return None
    if part == "Deck" or cls == CLASS["M_Int_Deck"] or (poly.normal.z > 0.6 and poly.center.z < 12.0):
        return "Floor"
    return "Room"


def finalize(ob, face_atlas, tag):
    """UV0 = atlas UVs on baked faces, triplanar kit UVs elsewhere; baked faces
    move onto M_Int_<tag>_<atlas> and now-unused slots are dropped."""
    me = ob.data
    uv0 = me.uv_layers["UVMap"].data
    layers = {a: me.uv_layers[f"Atlas_{a}"].data for a in set(face_atlas) if a}
    for p, a in zip(me.polygons, face_atlas):
        if a:
            at = layers[a]
            for li in p.loop_indices:
                uv0[li].uv = at[li].uv
    for a in list(layers):
        me.uv_layers.remove(me.uv_layers[f"Atlas_{a}"])
    me.uv_layers.active = me.uv_layers["UVMap"]
    me.uv_layers["UVMap"].active_render = True
    old = [mm.name if mm else "" for mm in me.materials]
    used, new_idx = [], []
    for p, a in zip(me.polygons, face_atlas):
        name = f"M_Int_{tag}_{a}" if a else old[p.material_index]
        if name not in used:
            used.append(name)
        new_idx.append(used.index(name))
    me.materials.clear()
    for n in used:
        me.materials.append(bpy.data.materials.get(n) or bpy.data.materials.new(n))
    for p, i in zip(me.polygons, new_idx):
        p.material_index = i


def bake_kit(room, objs, argv):
    size = opt(argv, '--size', 4096, int)
    rebake = '--rebake' in argv
    tag = room.prefix.replace("SM_Int_", "").replace("_", "")        # CommandXLBridge
    cache_root = os.path.join(os.environ.get('TEMP', '.'), 'interior_unique_cache', f"{room.prefix}_{size}")
    os.makedirs(TEXOUT, exist_ok=True)
    t0 = time.time()
    if '--gpu' in argv:
        try:
            prefs = bpy.context.preferences.addons['cycles'].preferences
            prefs.get_devices()
            bpy.context.scene.cycles.device = 'GPU'
        except Exception as e:     # noqa: BLE001 - fall back to CPU
            print("GPU unavailable, baking on CPU:", e)

    all_obs = list(objs.values())
    tree = build_bvh(all_obs)
    face_atlas, visible, report = {}, {}, {"size": size, "faces": {}}
    for part, ob in objs.items():
        me = ob.data
        names = [mm.name if mm else "" for mm in me.materials]
        cls = [float(CLASS.get(names[p.material_index], -1)) for p in me.polygons]
        me.attributes.new("cls", 'FLOAT', 'FACE').data.foreach_set('value', cls)
        face_atlas[part] = [atlas_of(part, c, p) for c, p in zip(cls, me.polygons)]
        visible[part] = face_visibility(ob, tree)
        report["faces"][part] = {a: sum(1 for x in face_atlas[part] if x == a) for a in ("Room", "Floor", "Kit")}
        report["faces"][part]["visible_baked"] = sum(1 for a, v in zip(face_atlas[part], visible[part]) if a and v)

    for atlas in ("Floor", "Room", "Kit"):
        parts = [p for p in objs if atlas in face_atlas[p]]
        obs = [objs[p] for p in parts]
        seen = {objs[p].name: [a == atlas and v for a, v in zip(face_atlas[p], visible[p])] for p in parts}
        unwrap_atlas(obs, seen, size)
        for p in parts:            # rename this pass's layer; unseen members -> one corner texel
            me = objs[p].data
            lay = me.uv_layers["Atlas"]
            lay.name = f"Atlas_{atlas}"
            at = lay.data
            for poly, a, s in zip(me.polygons, face_atlas[p], seen[objs[p].name]):
                if s:
                    continue
                for li in poly.loop_indices:
                    at[li].uv = (0.0008, 0.0008) if a == atlas else (-4.0, -4.0)
        st = atlas_stats_layer(obs, seen, size, f"Atlas_{atlas}")
        print(f"ATLAS {atlas}", json.dumps(st), flush=True)
        report[atlas] = st
        if '--uv-only' in argv:
            dump = []
            for ob in obs:
                me = ob.data
                uvd = me.uv_layers[f"Atlas_{atlas}"].data
                for poly, s_ in zip(me.polygons, seen[ob.name]):
                    if s_:
                        dump.append([list(uvd[li].uv) for li in poly.loop_indices])
            with open(os.path.join(os.environ.get('TEMP', '.'), f"uvdump_{atlas}.json"), "w") as f:
                json.dump(dump, f)
            continue
        for ob in obs:             # bake writes through the active UV layer
            lay = ob.data.uv_layers[f"Atlas_{atlas}"]
            ob.data.uv_layers.active = lay
            lay.active_render = True
        cache = os.path.join(cache_root, atlas)
        os.makedirs(cache, exist_ok=True)
        B, timings = bake_atlas(obs, size, cache, rebake, room)
        D, N, R, M, AO, E, extra = compose(B, st["px_per_m_area_weighted_mean"])
        outs = {}
        for suf, arr, srgb in (('D', D, True), ('N', N, False), ('R', R, False), ('M', M, False),
                               ('AO', AO, False), ('E', E, True)):
            outs[suf] = write_png(os.path.join(TEXOUT, f"T_Int_{tag}_{atlas}_{suf}.png"), arr, srgb)
        report[atlas].update({**extra, "slot": f"M_Int_{tag}_{atlas}", "bake_s": timings,
                              "textures": [os.path.relpath(x, ROOT) for x in outs.values()]})
    if '--uv-only' not in argv:
        for part, ob in objs.items():
            if any(face_atlas[part]):
                finalize(ob, face_atlas[part], tag)
    for ob in all_obs:
        if "cls" in ob.data.attributes:
            ob.data.attributes.remove(ob.data.attributes["cls"])
    detail = os.path.join(TEXOUT, "T_Int_DetailMicro_N.png")
    if '--uv-only' not in argv and (rebake or not os.path.exists(detail)):
        write_detail_normal(detail)
    report["detail_normal"] = os.path.relpath(detail, ROOT)
    report["total_s"] = round(time.time() - t0, 1)
    print("UNIQUE_BAKE", json.dumps(report, indent=1), flush=True)
    return report
