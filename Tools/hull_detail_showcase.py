"""SHOWCASE: geometric hull-detailing techniques on ONE ship (Corvette, unique-UV hull).

Each variant layers a detailing technique on top of the current in-game hull
(SM_Ship_Corvette_01_Assembled_UniqueUV, which since the 2026-09-29 redesign
holds the hammerhead hull with tiled UVs, + the tiled T_Corvette_* set as M_Corvette_Hull uses it) and renders it
from fixed cameras so the techniques can be compared side by side:

  base        current hull, untouched
  bevel       chamfered hard edges + hardened normals (edges catch the light)
  plating     hull sliced into armour plates, each inset and raised/recessed
  greebles    scattered surface kit: vents, pipe runs, box clusters, tanks
  functional  purposeful parts only: RCS quads, antenna mast + dish, airlock
              hatches, radiator fins, registration markings, sensor blisters
  hero        bevel + plating + light greebles + functional

Read-only with respect to project assets: nothing is exported or overwritten.

Each variant renders twice: on the current baked texture ("tex") and on a calm
render-time paint ("calm": flat two-tone paint + AO grime + geometric edge wear),
because a busy texture hides geometric detail.

Run: blender -b --python Tools/hull_detail_showcase.py -- --out <dir> [--spp 64] [--res 1280x720]
                                                        [--paint tex,calm] [variant ...]
Output: <out>/<variant>_<paint>_<view>.png
"""
import math
import os
import random
import sys

import bmesh
import bpy
from mathutils import Matrix, Vector

TOOLS = os.path.dirname(os.path.abspath(__file__))
_args = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
# build_unique_hull_textures parses sys.argv at import time for --ship
sys.argv = [sys.argv[0], "--", "preview"]
sys.path.insert(0, TOOLS)
import build_unique_hull_textures as uniq  # noqa: E402

gen = uniq.gen

OPTS = {"out": os.path.join(os.environ.get("TEMP", "."), "hull_detail_showcase"), "spp": "64", "res": "1280x720", "paint": "tex,calm"}
VARIANTS = []
i = 0
while i < len(_args):
    if _args[i].startswith("--"):
        OPTS[_args[i][2:]] = _args[i + 1]
        i += 2
    else:
        VARIANTS.append(_args[i])
        i += 1
ALL = ["base", "bevel", "plating", "greebles", "functional", "hero"]
VARIANTS = VARIANTS or ALL

# bow is +Y, main engine at -Y, dorsal +Z, port -X (as imported with axis_forward='-Y')
VIEWS = {
    "hero":  ((7.6, 3.4, 3.0), (0.0, 0.3, -0.2), 35),
    "close": ((3.0, 1.6, 1.3), (0.7, -0.6, 0.15), 40),
    "bow":   ((2.6, 6.2, 1.5), (0.0, 3.4, 0.0), 40),
    "rear":  ((-4.0, -6.2, 2.0), (0.0, -1.4, 0.0), 40),
}

# material slots on the detail object
TRIM, PAINT, HAZARD, GLOW_W, GLOW_R, GLOW_G, DARK = range(7)


# ----------------------------------------------------------------------------
# scene
# ----------------------------------------------------------------------------
def calm_paint():
    """Render-time stand-in for a calm baked hull: flat two-tone paint, large soft
    tone drift, AO grime in crevices and bare-metal wear on geometric edges."""
    m = principled("CalmPaint", (0.27, 0.25, 0.33), 0.25, 0.5)
    nt = m.node_tree
    p = nt.nodes["Principled BSDF"]
    L = nt.links
    tc = nt.nodes.new("ShaderNodeTexCoord")
    noise = nt.nodes.new("ShaderNodeTexNoise")
    noise.inputs["Scale"].default_value = 0.35
    noise.inputs["Detail"].default_value = 2.0
    L.new(tc.outputs["Object"], noise.inputs["Vector"])
    tone = nt.nodes.new("ShaderNodeMix")
    tone.data_type = 'RGBA'
    tone.inputs["A"].default_value = (0.105, 0.095, 0.135, 1)
    tone.inputs["B"].default_value = (0.13, 0.12, 0.16, 1)
    L.new(noise.outputs["Fac"], tone.inputs["Factor"])
    # two-tone: darker belly below the waterline
    xyz = nt.nodes.new("ShaderNodeSeparateXYZ")
    L.new(tc.outputs["Object"], xyz.inputs[0])
    belly = nt.nodes.new("ShaderNodeMapRange")
    belly.inputs["From Min"].default_value = -0.05
    belly.inputs["From Max"].default_value = -0.12
    L.new(xyz.outputs["Z"], belly.inputs["Value"])
    two = nt.nodes.new("ShaderNodeMix")
    two.data_type = 'RGBA'
    two.inputs["B"].default_value = (0.035, 0.034, 0.042, 1)
    L.new(tone.outputs["Result"], two.inputs["A"])
    L.new(belly.outputs["Result"], two.inputs["Factor"])
    ao = nt.nodes.new("ShaderNodeAmbientOcclusion")
    ao.inputs["Distance"].default_value = 0.12
    grime = nt.nodes.new("ShaderNodeMix")
    grime.data_type = 'RGBA'
    grime.blend_type = 'MULTIPLY'
    L.new(two.outputs["Result"], grime.inputs["A"])
    pw = nt.nodes.new("ShaderNodeMath")
    pw.operation = 'POWER'
    pw.inputs[1].default_value = 0.6
    L.new(ao.outputs["AO"], pw.inputs[0])
    L.new(pw.outputs[0], grime.inputs["B"])
    grime.inputs["Factor"].default_value = 1.0
    bev = nt.nodes.new("ShaderNodeBevel")
    bev.inputs["Radius"].default_value = 0.012
    geo = nt.nodes.new("ShaderNodeNewGeometry")
    dot = nt.nodes.new("ShaderNodeVectorMath")
    dot.operation = 'DOT_PRODUCT'
    L.new(bev.outputs["Normal"], dot.inputs[0])
    L.new(geo.outputs["Normal"], dot.inputs[1])
    edge = nt.nodes.new("ShaderNodeMapRange")
    edge.inputs["From Min"].default_value = 0.985
    edge.inputs["From Max"].default_value = 0.94
    L.new(dot.outputs["Value"], edge.inputs["Value"])
    wear = nt.nodes.new("ShaderNodeMix")
    wear.data_type = 'RGBA'
    wear.inputs["B"].default_value = (0.45, 0.45, 0.47, 1)
    L.new(edge.outputs["Result"], wear.inputs["Factor"])
    L.new(grime.outputs["Result"], wear.inputs["A"])
    L.new(wear.outputs["Result"], p.inputs["Base Color"])
    met = nt.nodes.new("ShaderNodeMapRange")
    met.inputs["To Min"].default_value = 0.25
    met.inputs["To Max"].default_value = 0.9
    L.new(edge.outputs["Result"], met.inputs["Value"])
    L.new(met.outputs["Result"], p.inputs["Metallic"])
    return m


def load_ship(paint="tex"):
    gen.setup_scene()
    bpy.ops.import_scene.fbx(filepath=os.path.join(gen.BASE, "SM_Ship_Corvette_01_Assembled_UniqueUV.fbx"),
                             axis_forward='-Y', axis_up='Z')
    ob = [o for o in bpy.data.objects if o.type == 'MESH'][0]
    s = uniq.m_per_bu_for(ob.dimensions.y)
    ob.scale = (s, s, s)
    ob.location = (0, 0, 0)
    bpy.context.view_layer.objects.active = ob
    ob.select_set(True)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    me = ob.data
    lo = Vector((min(v.co[k] for v in me.vertices) for k in range(3)))
    hi = Vector((max(v.co[k] for v in me.vertices) for k in range(3)))
    c = (lo + hi) / 2
    for v in me.vertices:
        v.co -= c
    me.materials.clear()
    me.materials.append(calm_paint() if paint == "calm" else
                        uniq.make_preview_mat("Hull", "Corvette", True, True))  # in-game M_Corvette_Hull: tiled set on UV0
    for p in me.polygons:
        p.use_smooth = False
    return ob


def principled(name, base, metal, rough, emit=None, strength=0.0):
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    p = m.node_tree.nodes["Principled BSDF"]
    p.inputs["Base Color"].default_value = (*base, 1)
    p.inputs["Metallic"].default_value = metal
    p.inputs["Roughness"].default_value = rough
    if emit:
        p.inputs["Emission Color"].default_value = (*emit, 1)
        p.inputs["Emission Strength"].default_value = strength
    return m


def hazard_material():
    m = principled("Hazard", (0.8, 0.55, 0.05), 0.2, 0.55)
    nt = m.node_tree
    p = nt.nodes["Principled BSDF"]
    tc = nt.nodes.new("ShaderNodeTexCoord")
    wave = nt.nodes.new("ShaderNodeTexWave")
    wave.bands_direction = 'DIAGONAL'
    wave.inputs["Scale"].default_value = 9.0
    ramp = nt.nodes.new("ShaderNodeValToRGB")
    ramp.color_ramp.interpolation = 'CONSTANT'
    ramp.color_ramp.elements[0].color = (0.02, 0.02, 0.02, 1)
    ramp.color_ramp.elements[1].position = 0.5
    ramp.color_ramp.elements[1].color = (0.75, 0.5, 0.04, 1)
    nt.links.new(tc.outputs["Object"], wave.inputs["Vector"])
    nt.links.new(wave.outputs["Fac"], ramp.inputs["Fac"])
    nt.links.new(ramp.outputs["Color"], p.inputs["Base Color"])
    return m


def detail_materials():
    return [
        principled("Trim", (0.16, 0.16, 0.17), 0.85, 0.36),
        principled("Paint", (0.78, 0.78, 0.74), 0.0, 0.55),
        hazard_material(),
        principled("GlowW", (1, 1, 1), 0, 0.3, (1.0, 0.95, 0.85), 25),
        principled("GlowR", (1, 0, 0), 0, 0.3, (1.0, 0.08, 0.05), 25),
        principled("GlowG", (0, 1, 0), 0, 0.3, (0.1, 1.0, 0.25), 25),
        principled("Dark", (0.015, 0.015, 0.018), 0.4, 0.7),
    ]


def setup_render():
    sc = bpy.context.scene
    sc.render.engine = 'CYCLES'
    sc.cycles.samples = int(OPTS["spp"])
    sc.cycles.use_denoising = True
    rx, ry = OPTS["res"].split("x")
    sc.render.resolution_x, sc.render.resolution_y = int(rx), int(ry)
    sc.view_settings.view_transform = 'AgX'
    sc.view_settings.look = 'AgX - Medium High Contrast'
    w = bpy.data.worlds.new("Space")
    sc.world = w
    w.use_nodes = True
    bg = w.node_tree.nodes["Background"]
    bg.inputs["Color"].default_value = (0.06, 0.065, 0.085, 1)
    bg.inputs["Strength"].default_value = 1.0

    def sun(name, energy, color, rot):
        o = bpy.data.objects.new(name, bpy.data.lights.new(name, 'SUN'))
        o.data.energy = energy
        o.data.color = color
        o.data.angle = math.radians(1.5)
        o.rotation_euler = [math.radians(a) for a in rot]
        sc.collection.objects.link(o)
    sun("Key", 5.0, (1.0, 0.96, 0.9), (52, 8, 140))      # raking key from front-right-above
    sun("Rim", 3.0, (0.65, 0.78, 1.0), (115, 0, -25))    # cool rim from behind-below
    sun("Fill", 0.5, (0.8, 0.85, 1.0), (60, 0, -110))
    cam = bpy.data.objects.new("Cam", bpy.data.cameras.new("Cam"))
    cam.data.clip_start = 0.05
    sc.collection.objects.link(cam)
    sc.camera = cam
    return cam


# ----------------------------------------------------------------------------
# hull face survey
# ----------------------------------------------------------------------------
class Face:
    def __init__(self, p, me):
        self.idx = p.index
        self.glazed = False  # hosts window geometry: keep flush, no kit on it
        self.c = p.center.copy()
        self.n = p.normal.copy()
        self.area = p.area
        vs = [me.vertices[k].co for k in p.vertices]
        best, t = -1, Vector((0, 1, 0))
        for a, b in zip(vs, vs[1:] + vs[:1]):
            e = b - a
            if e.length > best:
                best, t = e.length, e.normalized()
        # prefer the edge direction closest to the ship's long axis for a stable frame
        if abs(t.y) < 0.5:
            ty = Vector((0, 1, 0)) - self.n * self.n.y
            if ty.length > 0.3:
                t = ty.normalized()
        self.t = (t - self.n * t.dot(self.n)).normalized()
        self.b = self.n.cross(self.t)
        self.ext_t = max(v.dot(self.t) for v in vs) - min(v.dot(self.t) for v in vs)
        self.ext_b = max(v.dot(self.b) for v in vs) - min(v.dot(self.b) for v in vs)

    def frame(self, du=0.0, dv=0.0, lift=0.0, spin=0.0):
        t = self.t
        b = self.b
        if spin:
            r = Matrix.Rotation(spin, 3, self.n)
            t, b = r @ t, r @ b
        rot = Matrix((t, b, self.n)).transposed().to_4x4()
        return Matrix.Translation(self.c + self.t * du + self.b * dv + self.n * lift) @ rot


def survey(ob):
    me = ob.data
    return [Face(p, me) for p in me.polygons if p.area > 1e-4]


# ----------------------------------------------------------------------------
# kit primitives (all in a face-local frame: X=t, Y=b, Z=normal, origin on hull)
# ----------------------------------------------------------------------------
def _has_uv(bm):
    return bool(bm.loops.layers.uv)


def _assign(bm, verts, mat):
    for f in {f for v in verts for f in v.link_faces}:
        f.material_index = mat


def box(bm, M, sx, sy, sz, mat=TRIM, z0=0.0):
    """Box sitting on the local XY plane (bottom at z0)."""
    m = M @ Matrix.Translation((0, 0, z0 + sz / 2)) @ Matrix.Diagonal((sx, sy, sz, 1))
    r = bmesh.ops.create_cube(bm, size=1.0, matrix=m, calc_uvs=_has_uv(bm))
    _assign(bm, r["verts"], mat)


def cyl(bm, M, r1, h, segs=12, mat=TRIM, r2=None, z0=0.0, axis='Z'):
    rot = {'Z': Matrix(), 'X': Matrix.Rotation(math.pi / 2, 4, 'Y'), 'Y': Matrix.Rotation(-math.pi / 2, 4, 'X')}[axis]
    m = M @ rot @ Matrix.Translation((0, 0, z0 + h / 2))
    r = bmesh.ops.create_cone(bm, cap_ends=True, segments=segs, radius1=r1,
                              radius2=r1 if r2 is None else r2, depth=h, matrix=m, calc_uvs=_has_uv(bm))
    _assign(bm, r["verts"], mat)


def sphere(bm, M, r, mat=TRIM, squash=0.5):
    m = M @ Matrix.Diagonal((1, 1, squash, 1))
    res = bmesh.ops.create_uvsphere(bm, u_segments=14, v_segments=8, radius=r, matrix=m, calc_uvs=_has_uv(bm))
    _assign(bm, res["verts"], mat)


def kit_vent(bm, M, w, h, rnd):
    box(bm, M, w, h, 0.012, DARK)
    box(bm, M, w + 0.02, h + 0.02, 0.006, TRIM)
    n = max(3, int(h / 0.03))
    for k in range(n):
        y = -h / 2 + (k + 0.5) * h / n
        box(bm, M @ Matrix.Translation((0, y, 0.004)) @ Matrix.Rotation(0.5, 4, 'X'), w * 0.94, 0.006, 0.02, TRIM)


def kit_pipes(bm, M, length, rnd):
    count = rnd.choice((2, 3))
    r = rnd.uniform(0.014, 0.022)
    for k in range(count):
        cyl(bm, M @ Matrix.Translation((0, (k - (count - 1) / 2) * r * 2.6, r * 1.1)),
            r, length, 10, TRIM, axis='X', z0=-length / 2)
    for x in range(int(length / 0.25) + 1):
        xx = -length / 2 + x * length / max(1, int(length / 0.25))
        box(bm, M @ Matrix.Translation((xx, 0, 0)), 0.025, count * r * 2.6 + 0.02, r * 2.4, DARK)


def kit_boxes(bm, M, w, h, rnd):
    for _ in range(rnd.randint(2, 4)):
        sx, sy = w * rnd.uniform(0.25, 0.6), h * rnd.uniform(0.25, 0.6)
        x, y = rnd.uniform(-w / 2 + sx / 2, w / 2 - sx / 2), rnd.uniform(-h / 2 + sy / 2, h / 2 - sy / 2)
        box(bm, M @ Matrix.Translation((x, y, 0)), sx, sy, rnd.uniform(0.02, 0.07), TRIM)


def kit_tank(bm, M, w, h, rnd):
    r = min(w, h) * 0.22
    L = max(w, h) * 0.8
    cyl(bm, M @ Matrix.Translation((0, 0, r * 1.05)), r, L, 16, TRIM, axis='X', z0=-L / 2)
    for x in (-L / 2, L / 2):
        sphere(bm, M @ Matrix.Translation((x, 0, r * 1.05)) @ Matrix.Rotation(math.pi / 2, 4, 'Y'), r, TRIM, 0.5)
    for x in (-L / 3, L / 3):
        box(bm, M @ Matrix.Translation((x, 0, 0)), 0.03, r * 2.2, r * 1.2, DARK)


def kit_hatch(bm, M, w, h):
    box(bm, M, w + 0.05, h + 0.05, 0.004, HAZARD)
    box(bm, M, w, h, 0.009, TRIM)
    box(bm, M @ Matrix.Translation((0, 0, 0.009)), w * 0.8, h * 0.08, 0.006, DARK)
    box(bm, M @ Matrix.Translation((w * 0.32, 0, 0.009)), 0.05, 0.03, 0.02, TRIM)
    box(bm, M @ Matrix.Translation((-w / 2 - 0.04, h * 0.35, 0)), 0.025, 0.025, 0.015, GLOW_G)


def kit_rcs(bm, M, out_t):
    """RCS quad: block + three nozzles (outward, up, and fore/aft along out_t)."""
    box(bm, M, 0.16, 0.16, 0.07, TRIM)
    top = M @ Matrix.Translation((0, 0, 0.07))
    cyl(bm, top, 0.022, 0.05, 10, DARK, r2=0.034)                          # outward
    for sgn in (1, -1):
        cyl(bm, M @ Matrix.Translation((0, sgn * 0.08, 0.035)) @ Matrix.Rotation(-sgn * math.pi / 2, 4, 'X'),
            0.018, 0.04, 10, DARK, r2=0.028)                               # up / down
    cyl(bm, M @ Matrix.Translation((out_t * 0.08, 0, 0.035)) @ Matrix.Rotation(out_t * math.pi / 2, 4, 'Y'),
        0.018, 0.04, 10, DARK, r2=0.028)                                   # fore/aft


def kit_antenna(bm, M):
    cyl(bm, M, 0.05, 0.04, 12, TRIM)
    cyl(bm, M, 0.012, 0.55, 8, TRIM, z0=0.04)
    cyl(bm, M @ Matrix.Translation((0, 0, 0.59)), 0.02, 0.02, 8, GLOW_R)
    # small dish on an arm
    arm = M @ Matrix.Translation((0, 0, 0.32))
    box(bm, arm, 0.16, 0.012, 0.012, TRIM)
    dish = arm @ Matrix.Translation((0.09, 0, 0)) @ Matrix.Rotation(math.radians(70), 4, 'Y')
    cyl(bm, dish, 0.012, 0.03, 12, TRIM, r2=0.11)
    for x, hgt in ((0.18, 0.35), (-0.15, 0.28)):
        cyl(bm, M @ Matrix.Translation((x, 0.05, 0)), 0.005, hgt, 6, TRIM)


def kit_radiators(bm, M, length, height, count):
    box(bm, M, length, height * 0.9, 0.02, DARK)
    for k in range(count):
        x = -length / 2 + (k + 0.5) * length / count
        box(bm, M @ Matrix.Translation((x, 0, 0.02)), 0.008, height * 0.85, height * 0.45, TRIM)


def add_text(text, M, size, mat_index, collection):
    cu = bpy.data.curves.new("Reg", 'FONT')
    cu.body = text
    cu.size = size
    cu.align_x = 'CENTER'
    cu.align_y = 'CENTER'
    cu.extrude = 0.0015
    o = bpy.data.objects.new("Reg", cu)
    collection.objects.link(o)
    o.matrix_world = M @ Matrix.Translation((0, 0, 0.004))
    return o


# ----------------------------------------------------------------------------
# techniques
# ----------------------------------------------------------------------------
def apply_bevel(ob, width=0.012):
    for p in ob.data.polygons:
        p.use_smooth = True
    bev = ob.modifiers.new("Bevel", 'BEVEL')
    bev.width = width
    bev.segments = 1
    bev.limit_method = 'ANGLE'
    bev.angle_limit = math.radians(30)
    bev.harden_normals = True
    bev.miter_outer = 'MITER_ARC'
    wn = ob.modifiers.new("WeightedNormal", 'WEIGHTED_NORMAL')
    wn.keep_sharp = True


def apply_plating(ob, rnd, step=0.62, min_area=0.12, flush=()):
    """Slice large hull faces into a plate grid, then inset each plate with a
    varied height (raised / flush / recessed) so the seams are real geometry.
    Faces whose index is in `flush` (e.g. under window geometry) are still
    paneled but stay at their original height."""
    bm = bmesh.new()
    bm.from_mesh(ob.data)
    lay = bm.faces.layers.int.new("plate")
    for f in bm.faces:
        f[lay] = (2 if f.index in flush else 1) if f.calc_area() > min_area else 0

    def geom():
        fs = [f for f in bm.faces if f[lay]]
        es = list({e for f in fs for e in f.edges})
        vs = list({v for f in fs for v in f.verts})
        return vs + es + fs

    lo = Vector((min(v.co[k] for v in bm.verts) for k in range(3)))
    hi = Vector((max(v.co[k] for v in bm.verts) for k in range(3)))
    for axis, s in ((1, step), (0, step * 0.8), (2, step * 0.8)):
        x = lo[axis] + s * rnd.uniform(0.3, 0.7)
        while x < hi[axis]:
            no = Vector((0, 0, 0))
            no[axis] = 1
            co = Vector((0, 0, 0))
            co[axis] = x
            bmesh.ops.bisect_plane(bm, geom=geom(), plane_co=co, plane_no=no, dist=0.002)
            x += s * rnd.uniform(0.7, 1.3)
    plates = [f for f in bm.faces if f[lay] and f.calc_area() > 0.02]
    groups = {0.0: [], 0.02: [], 0.035: [], -0.015: []}
    keys = list(groups)
    for f in plates:
        groups[0.0 if f[lay] == 2 else rnd.choices(keys, weights=(4, 4, 2, 1))[0]].append(f)
    for depth, fs in groups.items():
        if fs:
            bmesh.ops.inset_individual(bm, faces=fs, thickness=0.015, depth=depth, use_even_offset=True)
    bm.faces.layers.int.remove(lay)
    bm.to_mesh(ob.data)
    bm.free()
    return len(plates)


def detail_object(name, mats):
    me = bpy.data.meshes.new(name)
    o = bpy.data.objects.new(name, me)
    bpy.context.scene.collection.objects.link(o)
    for m in mats:
        me.materials.append(m)
    return o


def apply_greebles(faces, bm, rnd, density=0.35):
    placed = 0
    for f in faces:
        if f.area < 0.06 or min(f.ext_t, f.ext_b) < 0.18 or rnd.random() > density:
            continue
        w = min(f.ext_t * 0.55, 0.7)
        h = min(f.ext_b * 0.55, 0.45)
        du = rnd.uniform(-1, 1) * (f.ext_t - w) * 0.35
        dv = rnd.uniform(-1, 1) * (f.ext_b - h) * 0.35
        M = f.frame(du, dv, 0.001)
        kind = rnd.choices(("vent", "pipes", "boxes", "tank"), weights=(3, 2, 4, 1))[0]
        if kind == "vent":
            kit_vent(bm, M, w * 0.7, h * 0.6, rnd)
        elif kind == "pipes":
            kit_pipes(bm, f.frame(0, dv, 0.001), f.ext_t * 0.8, rnd)
        elif kind == "boxes":
            kit_boxes(bm, M, w, h, rnd)
        else:
            kit_tank(bm, M, w, h, rnd)
        placed += 1
    return placed


def apply_functional(faces, bm, coll, ymin, ymax, navlights=True, antenna=True):
    L = ymax - ymin

    def pick(pred, key):
        cand = [f for f in faces if pred(f)]
        return max(cand, key=key) if cand else None

    out = []
    # RCS quads: two per side, at the bow and at the stern
    for side in (1, -1):
        for zone, out_t in (((0.62, 0.85), 1), ((0.08, 0.3), -1)):
            f = pick(lambda f, s=side, z=zone: f.n.x * s > 0.75 and z[0] < (f.c.y - ymin) / L < z[1]
                     and min(f.ext_t, f.ext_b) > 0.2, key=lambda f: f.area)
            if f:
                kit_rcs(bm, f.frame(0, 0, 0.001), out_t * (1 if f.t.y > 0 else -1))
                out.append("rcs")
    # antenna mast on the highest dorsal face forward of midships
    f = pick(lambda f: f.n.z > 0.8 and (f.c.y - ymin) / L > 0.5 and min(f.ext_t, f.ext_b) > 0.15,
             key=lambda f: f.c.z + f.area * 0.1)
    if f and antenna:
        kit_antenna(bm, f.frame(0, 0, 0.001))
        out.append("antenna")
    # airlock hatch + registration number on each flank (largest mid-ship side face)
    for side in (1, -1):
        f = pick(lambda f, s=side: f.n.x * s > 0.85 and 0.35 < (f.c.y - ymin) / L < 0.7,
                 key=lambda f: f.area)
        if f:
            hw, hh = min(0.42, f.ext_t * 0.3), min(0.55, f.ext_b * 0.7)
            kit_hatch(bm, f.frame(-f.ext_t * 0.25, 0, 0.001), hw, hh)
            right = Vector((0, side, 0))
            up = Vector((0, 0, 1))
            rot = Matrix((right, up, f.n)).transposed().to_4x4()
            M = Matrix.Translation(f.c + f.t * f.ext_t * 0.18) @ rot
            add_text("ADS-417", M, min(0.16, f.ext_b * 0.35), PAINT, coll)
            out.append("hatch+reg")
    # radiator fins on the dorsal aft section
    f = pick(lambda f: f.n.z > 0.8 and 0.12 < (f.c.y - ymin) / L < 0.45 and f.ext_t > 0.4,
             key=lambda f: f.area)
    if f:
        kit_radiators(bm, f.frame(0, 0, 0.001), min(f.ext_t * 0.7, 1.4), min(f.ext_b * 0.6, 0.4), 14)
        out.append("radiators")
    # sensor blisters on the ventral bow
    for k, f in enumerate(sorted([f for f in faces if f.n.z < -0.8 and (f.c.y - ymin) / L > 0.55
                                  and min(f.ext_t, f.ext_b) > 0.15], key=lambda f: -f.area)[:2]):
        sphere(bm, f.frame(0, 0, 0.0), 0.09, DARK, 0.6)
        cyl(bm, f.frame(0, 0, 0.0), 0.12, 0.012, 16, TRIM)
        out.append("sensor")
    if not navlights:
        return out
    # nav lights: port red, starboard green, white on the dorsal stern
    xs = sorted(faces, key=lambda f: f.c.x)
    for f, mat in ((xs[0], GLOW_R), (xs[-1], GLOW_G)):
        sphere(bm, f.frame(0, 0, 0.0), 0.035, mat, 0.7)
    f = pick(lambda f: f.n.z > 0.7 and (f.c.y - ymin) / L < 0.15, key=lambda f: f.c.z)
    if f:
        sphere(bm, f.frame(0, 0, 0.0), 0.03, GLOW_W, 0.7)
    out.append("navlights")
    return out


# ----------------------------------------------------------------------------
def build(variant, paint):
    ob = load_ship(paint)
    cam = setup_render()
    rnd = random.Random(417)
    ymin = min(v.co.y for v in ob.data.vertices)
    ymax = max(v.co.y for v in ob.data.vertices)
    faces = survey(ob)  # survey the ORIGINAL hull so kit lands on the base surface
    mats = detail_materials()
    stats = {"variant": variant, "paint": paint}
    if variant in ("plating", "hero"):
        stats["plates"] = apply_plating(ob, rnd)
    if variant in ("bevel", "hero"):
        apply_bevel(ob)
    if variant in ("greebles", "functional", "hero"):
        det = detail_object("Detail", mats)
        bm = bmesh.new()
        if variant in ("greebles", "hero"):
            lift = 0.03 if variant == "hero" else 0.0
            for f in faces:
                f.c = f.c + f.n * lift
            stats["greebles"] = apply_greebles(faces, bm, rnd, 0.45 if variant == "greebles" else 0.16)
        if variant in ("functional", "hero"):
            stats["functional"] = apply_functional(faces, bm, bpy.context.scene.collection, ymin, ymax)
            for o in bpy.data.objects:
                if o.type == 'FONT':
                    o.data.materials.append(mats[PAINT])
        bm.to_mesh(det.data)
        bm.free()
        for p in det.data.polygons:
            p.use_smooth = False
    tris = sum(len(p.vertices) - 2 for o in bpy.data.objects if o.type == 'MESH'
               for p in o.evaluated_get(bpy.context.evaluated_depsgraph_get()).data.polygons)
    stats["tris"] = tris
    os.makedirs(OPTS["out"], exist_ok=True)
    sc = bpy.context.scene
    for vname, (loc, tgt, lens) in VIEWS.items():
        cam.location = loc
        cam.rotation_euler = (Vector(tgt) - Vector(loc)).to_track_quat('-Z', 'Y').to_euler()
        cam.data.lens = lens
        sc.render.filepath = os.path.join(OPTS["out"], f"{variant}_{paint}_{vname}.png")
        bpy.ops.render.render(write_still=True)
    print("SHOWCASE", stats, flush=True)


if __name__ == "__main__":
    for v in VARIANTS:
        for paint in OPTS["paint"].split(","):
            build(v, paint)
