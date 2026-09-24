"""Test renders for the interior texture library (Blender headless, Cycles).

Builds simple box rooms at real scale, UV'd world-aligned at 2 m per UV unit
(the same contract as generate_adastrea_assets.smart_uv), and textures them with
Assets/Textures/generated/interiors/T_Int_* so the sets can be judged at human
scale from eye height (1.65 m).

Normal maps are DirectX (green-down); Blender wants OpenGL, so the green channel
is inverted in the shader here -- in Unreal they import as-is (Flip Green OFF).

Usage:
  blender -b --python Tools/render_interior_texture_tests.py            # all rooms
  blender -b --python Tools/render_interior_texture_tests.py -- corridor eng
Outputs: Assets/Textures/generated/interiors/previews/render_<room>.png
"""
import bpy
import bmesh
import math
import os
import sys
from mathutils import Vector

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TEX = os.path.join(ROOT, "Assets", "Textures", "generated", "interiors")
OUT = os.path.join(TEX, "previews")
TILE_M = 2.0


def reset():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    _MATS.clear()
    sc = bpy.context.scene
    sc.render.engine = "CYCLES"
    sc.cycles.device = "CPU"
    sc.cycles.samples = 96
    sc.cycles.use_denoising = True
    sc.render.resolution_x = 1280
    sc.render.resolution_y = 720
    sc.view_settings.view_transform = "AgX"
    sc.view_settings.look = "AgX - Medium High Contrast"
    world = bpy.data.worlds.new("W")
    world.use_nodes = True
    world.node_tree.nodes["Background"].inputs[1].default_value = 0.0
    sc.world = world
    return sc


def img(name, suf, color=False):
    p = os.path.join(TEX, f"T_Int_{name}_{suf}.png")
    if not os.path.exists(p):
        return None
    im = bpy.data.images.load(p, check_existing=True)
    im.colorspace_settings.name = "sRGB" if color else "Non-Color"
    return im


_MATS = {}


def texmat(name, tiling=1.0, emit=0.0, glass=False, masked=False):
    key = (name, tiling, emit, glass, masked)
    if key in _MATS:
        return _MATS[key]
    m = bpy.data.materials.new(f"M_{name}_{tiling}")
    m.use_nodes = True
    nt = m.node_tree
    N = nt.nodes
    L = nt.links
    bsdf = N["Principled BSDF"]
    uv = N.new("ShaderNodeTexCoord")
    mp = N.new("ShaderNodeMapping")
    mp.inputs["Scale"].default_value = (tiling, tiling, 1)
    L.new(uv.outputs["UV"], mp.inputs["Vector"])

    def tex(suf, color=False):
        im = img(name, suf, color)
        if im is None:
            return None
        t = N.new("ShaderNodeTexImage")
        t.image = im
        t.interpolation = "Cubic"
        L.new(mp.outputs["Vector"], t.inputs["Vector"])
        return t

    D, R, M_, AO, E, NM, MK = (tex("D", True), tex("R"), tex("M"), tex("AO"),
                              tex("E", True), tex("N"), tex("MASK"))
    mul = N.new("ShaderNodeMix")
    mul.data_type = "RGBA"
    mul.blend_type = "MULTIPLY"
    mul.inputs["Factor"].default_value = 1.0
    sock = lambda coll, nm: [x for x in coll if x.name == nm and x.type == "RGBA"][0]
    L.new(D.outputs["Color"], sock(mul.inputs, "A"))
    L.new(AO.outputs["Color"], sock(mul.inputs, "B"))
    L.new(sock(mul.outputs, "Result"), bsdf.inputs["Base Color"])
    L.new(R.outputs["Color"], bsdf.inputs["Roughness"])
    L.new(M_.outputs["Color"], bsdf.inputs["Metallic"])
    # DirectX -> OpenGL: invert green
    sep = N.new("ShaderNodeSeparateColor")
    comb = N.new("ShaderNodeCombineColor")
    inv = N.new("ShaderNodeMath")
    inv.operation = "SUBTRACT"
    inv.inputs[0].default_value = 1.0
    L.new(NM.outputs["Color"], sep.inputs["Color"])
    L.new(sep.outputs["Red"], comb.inputs["Red"])
    L.new(sep.outputs["Green"], inv.inputs[1])
    L.new(inv.outputs["Value"], comb.inputs["Green"])
    L.new(sep.outputs["Blue"], comb.inputs["Blue"])
    nmap = N.new("ShaderNodeNormalMap")
    L.new(comb.outputs["Color"], nmap.inputs["Color"])
    L.new(nmap.outputs["Normal"], bsdf.inputs["Normal"])
    if E is not None and emit > 0:
        L.new(E.outputs["Color"], bsdf.inputs["Emission Color"])
        bsdf.inputs["Emission Strength"].default_value = emit
    if glass:
        # suggested UE params mirrored here: tint from D, rough from R, dirt mask -> opacity
        bsdf.inputs["Transmission Weight"].default_value = 1.0
        bsdf.inputs["IOR"].default_value = 1.5
        L.new(D.outputs["Color"], bsdf.inputs["Base Color"])
        opa = N.new("ShaderNodeMapRange")
        opa.inputs["To Min"].default_value = 0.0
        opa.inputs["To Max"].default_value = 0.35
        L.new(MK.outputs["Color"], opa.inputs["Value"])
        # dirt: blend a diffuse dirty layer over the glass
        dirt = N.new("ShaderNodeBsdfPrincipled")
        dirt.inputs["Base Color"].default_value = (0.35, 0.33, 0.30, 1)
        dirt.inputs["Roughness"].default_value = 0.8
        mixs = N.new("ShaderNodeMixShader")
        L.new(opa.outputs["Result"], mixs.inputs["Fac"])
        L.new(bsdf.outputs["BSDF"], mixs.inputs[1])
        L.new(dirt.outputs["BSDF"], mixs.inputs[2])
        L.new(mixs.outputs["Shader"], N["Material Output"].inputs["Surface"])
    if masked and MK is not None:
        L.new(MK.outputs["Color"], bsdf.inputs["Alpha"])
    _MATS[key] = m
    return m


def quad(name, corners, uv_fn, mat):
    """corners: 4 world-space points (CCW seen from the side the normal faces)."""
    me = bpy.data.meshes.new(name)
    bm = bmesh.new()
    vs = [bm.verts.new(c) for c in corners]
    f = bm.faces.new(vs)
    lay = bm.loops.layers.uv.new("UVMap")
    for lp in f.loops:
        lp[lay].uv = uv_fn(lp.vert.co)
    bm.to_mesh(me)
    bm.free()
    ob = bpy.data.objects.new(name, me)
    bpy.context.scene.collection.objects.link(ob)
    me.materials.append(mat)
    return ob


def floor(x0, x1, y0, y1, z, mat, up=True, name="Floor"):
    c = [(x0, y0, z), (x1, y0, z), (x1, y1, z), (x0, y1, z)]
    if not up:
        c = c[::-1]
    return quad(name, [Vector(p) for p in c], lambda v: (v.x / TILE_M, v.y / TILE_M), mat)


def wall(p0, p1, z0, z1, mat, name="Wall", zref=0.0):
    """Wall from p0 to p1 (xy), facing left of the p0->p1 direction.
    V = (z - zref)/2 m so the floor-anchored band starts at the floor."""
    a = Vector((p0[0], p0[1], 0))
    b = Vector((p1[0], p1[1], 0))
    d = (b - a).normalized()
    c = [a + Vector((0, 0, z0)), a + Vector((0, 0, z1)), b + Vector((0, 0, z1)), b + Vector((0, 0, z0))]
    return quad(name, c, lambda v: (((v - a).dot(d)) / TILE_M, (v.z - zref) / TILE_M), mat)


def box_room(L, W, H, floor_m, wall_m, upper_m, ceil_m, band=2.0):
    """Room centred on x=0, spanning y 0..L (camera looks +Y)."""
    hw = W / 2
    floor(-hw, hw, 0, L, 0, floor_m)
    floor(-hw, hw, 0, L, H, ceil_m, up=False, name="Ceiling")
    pts = [((-hw, L), (-hw, 0)), ((hw, 0), (hw, L)), ((-hw, 0), (hw, 0)), ((hw, L), (-hw, L))]
    for i, (p0, p1) in enumerate(pts):
        wall(p0, p1, 0, min(band, H), wall_m, f"Wall{i}")
        if H > band and upper_m is not None:
            wall(p0, p1, band, H, upper_m, f"Upper{i}")


def cube(name, loc, size, mat, uv_scale=TILE_M):
    bpy.ops.mesh.primitive_cube_add(size=1, location=loc)
    ob = bpy.context.active_object
    ob.name = name
    ob.scale = size
    bpy.ops.object.transform_apply(scale=True)
    me = ob.data
    bm = bmesh.new()
    bm.from_mesh(me)
    lay = bm.loops.layers.uv.verify()
    for f in bm.faces:
        n = f.normal
        ax = max(range(3), key=lambda i: abs(n[i]))
        for lp in f.loops:
            v = ob.matrix_world @ lp.vert.co
            if ax == 2:
                lp[lay].uv = (v.x / uv_scale, v.y / uv_scale)
            elif ax == 0:
                lp[lay].uv = (v.y / uv_scale, v.z / uv_scale)
            else:
                lp[lay].uv = (v.x / uv_scale, v.z / uv_scale)
    bm.to_mesh(me)
    bm.free()
    me.materials.append(mat)
    return ob


def area_light(loc, size, power, color=(1, 1, 1), rot=(0, 0, 0), shape="RECTANGLE", sy=None):
    ld = bpy.data.lights.new("A", "AREA")
    ld.energy = power
    ld.color = color
    ld.shape = shape
    ld.size = size
    if sy is not None:
        ld.size_y = sy
    ob = bpy.data.objects.new("A", ld)
    ob.location = loc
    ob.rotation_euler = rot
    bpy.context.scene.collection.objects.link(ob)
    return ob


def point(loc, power, color=(1, 1, 1), r=0.1):
    ld = bpy.data.lights.new("P", "POINT")
    ld.energy = power
    ld.color = color
    ld.shadow_soft_size = r
    ob = bpy.data.objects.new("P", ld)
    ob.location = loc
    bpy.context.scene.collection.objects.link(ob)


def camera(loc, target, lens=18):
    cd = bpy.data.cameras.new("C")
    cd.lens = lens
    ob = bpy.data.objects.new("C", cd)
    ob.location = loc
    d = Vector(target) - Vector(loc)
    ob.rotation_euler = d.to_track_quat("-Z", "Y").to_euler()
    bpy.context.scene.collection.objects.link(ob)
    bpy.context.scene.camera = ob


def render(name):
    os.makedirs(OUT, exist_ok=True)
    p = os.path.join(OUT, f"render_{name}.png")
    bpy.context.scene.render.filepath = p
    bpy.ops.render.render(write_still=True)
    print("RENDERED", p, flush=True)


# ---------------------------------------------------------------- rooms
def room_corridor():
    reset()
    L, W, H = 12.0, 2.4, 2.8
    box_room(L, W, H, texmat("ShipDeck"), texmat("ShipWall"), texmat("ShipWallUpper"),
             texmat("ShipCeiling", emit=6.0))
    # console alcove on the right wall
    cube("ConsoleBody", (1.0, 4.0, 0.5), (0.4, 1.0, 1.0), texmat("ShipWallUpper"))
    ob = cube("ConsoleFace", (0.97, 4.0, 1.04), (0.46, 1.0, 0.03), texmat("Console", tiling=1.0, emit=3.0),
              uv_scale=1.0)
    ob.rotation_euler = (0, math.radians(-18), 0)
    # hazard trim sill at the far bulkhead door
    cube("Sill", (0, 10.0, 0.03), (2.4, 0.3, 0.06), texmat("Hazard", tiling=2.0), uv_scale=TILE_M)
    for y in (1.0, 3.0, 5.0, 7.0, 9.0, 11.0):
        area_light((0, y, H - 0.05), 0.16, 60, (0.88, 0.94, 1.0), shape="RECTANGLE", sy=1.8)
    camera((-0.35, 0.4, 1.65), (0.3, 8.0, 1.2), lens=16)
    render("corridor")


def room_eng():
    reset()
    L, W, H = 10.0, 6.0, 4.5
    # grating floor 0.3 m above a dark sub-floor so the voids read
    floor(-3, 3, 0, L, -0.35, texmat("ShipDeck"), name="SubFloor")
    point((0, 5, -0.2), 40, (1.0, 0.5, 0.2), r=0.3)
    floor(-3, 3, 0, L, 0, texmat("EngGrate", masked=True))
    floor(-3, 3, 0, L, H, texmat("ShipCeiling", emit=5.0), up=False, name="Ceiling")
    for (p0, p1) in (((-3, L), (-3, 0)), ((3, 0), (3, L)), ((3, L), (-3, L))):
        wall(p0, p1, -0.35, 2.0, texmat("EngWall"))
        wall(p0, p1, 2.0, H, texmat("EngWallUpper"))
    # reactor-ish block with hazard trim
    cube("Machine", (0, 6.5, 1.0), (2.0, 1.6, 2.0), texmat("EngWallUpper"))
    cube("MachineTrim", (0, 6.5, 0.06), (2.3, 1.9, 0.12), texmat("Hazard", tiling=2.0))
    for x in (-1.8, 1.8):
        for y in (2.0, 5.0, 8.0):
            area_light((x, y, H - 0.05), 0.16, 80, (1.0, 0.96, 0.88), sy=1.8)
    point((0, 6.5, 2.4), 120, (0.3, 0.8, 1.0), r=0.2)
    camera((-1.9, 0.5, 1.65), (0.8, 7.5, 1.1), lens=16)
    render("engbay")


def room_concourse():
    reset()
    L, W, H = 14.0, 8.0, 4.0
    box_room(L, W, H, texmat("StnFloor"), texmat("StnWall", emit=3.0), texmat("StnWallUpper"),
             texmat("StnCeiling", emit=5.0))
    for x in (-3.0, -1.0, 1.0, 3.0):
        for y in (1.0, 3.0, 5.0, 7.0, 9.0, 11.0, 13.0):
            area_light((x, y, H - 0.05), 0.56, 18, (1.0, 0.9, 0.76), sy=0.56)
    camera((-2.5, 0.5, 1.65), (1.0, 9.0, 1.3), lens=16)
    render("concourse")


def room_hab():
    reset()
    L, W, H = 5.0, 4.0, 2.7
    box_room(L, W, H, texmat("HabFloor"), texmat("HabWall"), texmat("StnWallUpper"),
             texmat("StnCeiling", emit=3.0))
    cube("Bunk", (1.4, 3.2, 0.3), (1.0, 2.0, 0.6), texmat("HabWall"))
    for x, y in ((-1.0, 1.5), (1.0, 1.5), (-1.0, 3.5), (1.0, 3.5)):
        area_light((x, y, H - 0.05), 0.56, 14, (1.0, 0.86, 0.68), sy=0.56)
    point((-1.5, 4.2, 1.2), 25, (1.0, 0.7, 0.4), r=0.15)
    camera((-1.5, 0.3, 1.6), (0.6, 4.6, 1.0), lens=16)
    render("habcabin")


def room_bridge():
    reset()
    L, W, H = 5.0, 5.0, 3.0
    box_room(L, W, H, texmat("ShipDeck"), texmat("ShipWall"), texmat("ShipWallUpper"),
             texmat("ShipCeiling", emit=4.0))
    # knock out the far wall middle and put a viewport there
    for o in [o for o in bpy.data.objects if o.name in ("Wall3", "Upper3")]:
        bpy.data.objects.remove(o)
    wall((2.5, L), (1.5, L), 0, H, texmat("ShipWallUpper"), "JambR")
    wall((-1.5, L), (-2.5, L), 0, H, texmat("ShipWallUpper"), "JambL")
    wall((1.5, L), (-1.5, L), 0, 0.9, texmat("ShipWall"), "SillWall")
    wall((1.5, L), (-1.5, L), 2.6, H, texmat("ShipWallUpper"), "Header")
    g = wall((1.5, L - 0.02), (-1.5, L - 0.02), 0.9, 2.6, texmat("Glass", glass=True), "Viewport")
    # space outside: a planet-ish lit sphere + stars
    bpy.ops.mesh.primitive_uv_sphere_add(radius=30, location=(8, L + 60, -6))
    pl = bpy.context.active_object
    pm = bpy.data.materials.new("Planet")
    pm.use_nodes = True
    pm.node_tree.nodes["Principled BSDF"].inputs["Base Color"].default_value = (0.2, 0.35, 0.6, 1)
    pl.data.materials.append(pm)
    sun = bpy.data.lights.new("S", "SUN")
    sun.energy = 4
    so = bpy.data.objects.new("S", sun)
    so.rotation_euler = (math.radians(60), 0, math.radians(200))
    bpy.context.scene.collection.objects.link(so)
    cube("Console", (0, L - 0.8, 0.5), (2.4, 0.6, 1.0), texmat("ShipWallUpper"))
    ob = cube("ConsoleTop", (0, L - 0.85, 1.02), (2.4, 0.7, 0.04), texmat("Console", emit=3.0), uv_scale=1.0)
    ob.rotation_euler = (math.radians(-15), 0, 0)
    for x in (-1.2, 1.2):
        area_light((x, 2.0, H - 0.05), 0.16, 50, (0.88, 0.94, 1.0), sy=1.8)
    point((0, L - 0.3, 1.6), 20, (0.5, 0.8, 1.0), r=0.5)
    camera((-0.6, 0.6, 1.65), (0.2, L, 1.5), lens=18)
    render("bridge_viewport")


ROOMS = dict(corridor=room_corridor, eng=room_eng, concourse=room_concourse, hab=room_hab,
             bridge=room_bridge)

if __name__ == "__main__":
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    for k, fn in ROOMS.items():
        if not argv or k in argv:
            fn()
