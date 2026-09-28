"""Lookdev render of generated PBR texture sets: a tilted tiling panel and a sphere
under one key light plus a dim sky, in Cycles. Used alongside
Tools/texture_benchmark.py so a score change can be checked by eye.

Run: blender -b --python Tools/render_texture_lookdev.py -- --tex <dir> --out <dir> [--tile 2] T_Freighter T_Int_ShipDeck ...
Output: <out>/<set>.png (640x400)
"""
import math
import os
import sys

import bpy

args = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
opts = {"tex": None, "out": None, "tile": "2"}
names = []
i = 0
while i < len(args):
    if args[i].startswith("--"):
        opts[args[i][2:]] = args[i + 1]
        i += 2
    else:
        names.append(args[i])
        i += 1
TEX, OUT, TILE = opts["tex"], opts["out"], float(opts["tile"])
os.makedirs(OUT, exist_ok=True)


def material(setname):
    mat = bpy.data.materials.new(setname)
    mat.use_nodes = True
    nt = mat.node_tree
    nt.nodes.clear()
    out = nt.nodes.new("ShaderNodeOutputMaterial")
    bsdf = nt.nodes.new("ShaderNodeBsdfPrincipled")
    coord = nt.nodes.new("ShaderNodeTexCoord")
    mapping = nt.nodes.new("ShaderNodeMapping")
    mapping.inputs["Scale"].default_value = (TILE, TILE, 1)
    nt.links.new(coord.outputs["UV"], mapping.inputs["Vector"])

    def tex(suf, cs):
        p = os.path.join(TEX, f"{setname}_{suf}.png")
        if not os.path.exists(p):
            return None
        n = nt.nodes.new("ShaderNodeTexImage")
        n.image = bpy.data.images.load(p, check_existing=True)
        n.image.colorspace_settings.name = cs
        nt.links.new(mapping.outputs["Vector"], n.inputs["Vector"])
        return n

    d = tex("D", "sRGB")
    r = tex("R", "Non-Color")
    m = tex("M", "Non-Color")
    n = tex("N", "Non-Color")
    ao = tex("AO", "Non-Color")
    e = tex("E", "sRGB")
    if d and ao:
        mul = nt.nodes.new("ShaderNodeMix")
        mul.data_type = "RGBA"
        mul.blend_type = "MULTIPLY"
        mul.inputs["Factor"].default_value = 1.0
        nt.links.new(d.outputs["Color"], mul.inputs[6])
        nt.links.new(ao.outputs["Color"], mul.inputs[7])
        nt.links.new(mul.outputs[2], bsdf.inputs["Base Color"])
    elif d:
        nt.links.new(d.outputs["Color"], bsdf.inputs["Base Color"])
    if r:
        nt.links.new(r.outputs["Color"], bsdf.inputs["Roughness"])
    if m:
        nt.links.new(m.outputs["Color"], bsdf.inputs["Metallic"])
    if n:
        # generator writes DirectX (green down): flip G for Blender's OpenGL convention
        sep = nt.nodes.new("ShaderNodeSeparateColor")
        comb = nt.nodes.new("ShaderNodeCombineColor")
        inv = nt.nodes.new("ShaderNodeMath")
        inv.operation = "SUBTRACT"
        inv.inputs[0].default_value = 1.0
        nt.links.new(n.outputs["Color"], sep.inputs["Color"])
        nt.links.new(sep.outputs["Green"], inv.inputs[1])
        nt.links.new(sep.outputs["Red"], comb.inputs["Red"])
        nt.links.new(inv.outputs["Value"], comb.inputs["Green"])
        nt.links.new(sep.outputs["Blue"], comb.inputs["Blue"])
        nm = nt.nodes.new("ShaderNodeNormalMap")
        nt.links.new(comb.outputs["Color"], nm.inputs["Color"])
        nt.links.new(nm.outputs["Normal"], bsdf.inputs["Normal"])
    if e:
        nt.links.new(e.outputs["Color"], bsdf.inputs["Emission Color"])
        bsdf.inputs["Emission Strength"].default_value = 4.0
    nt.links.new(bsdf.outputs["BSDF"], out.inputs["Surface"])
    return mat


def render(setname):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    sc = bpy.context.scene
    mat = material(setname)
    bpy.ops.mesh.primitive_plane_add(size=4, location=(-1.2, 0, 0))
    plane = bpy.context.active_object
    plane.rotation_euler = (math.radians(55), 0, math.radians(20))
    plane.data.materials.append(mat)
    bpy.ops.mesh.primitive_uv_sphere_add(radius=1.1, location=(2.1, 0.3, 0), segments=64, ring_count=32)
    sph = bpy.context.active_object
    bpy.ops.object.shade_smooth()
    sph.data.materials.append(mat)
    cam = bpy.data.objects.new("Cam", bpy.data.cameras.new("Cam"))
    sc.collection.objects.link(cam)
    sc.camera = cam
    cam.location = (0.4, -7.5, 1.6)
    cam.rotation_euler = (math.radians(82), 0, 0)
    cam.data.lens = 42
    sun = bpy.data.objects.new("Sun", bpy.data.lights.new("Sun", "SUN"))
    sun.data.energy = 4.0
    sun.data.angle = math.radians(2)
    sun.rotation_euler = (math.radians(50), math.radians(-25), math.radians(-35))
    sc.collection.objects.link(sun)
    w = bpy.data.worlds.new("W")
    sc.world = w
    w.use_nodes = True
    bg = w.node_tree.nodes.get("Background")
    grad = w.node_tree.nodes.new("ShaderNodeTexGradient")
    ramp = w.node_tree.nodes.new("ShaderNodeValToRGB")
    ramp.color_ramp.elements[0].color = (0.01, 0.012, 0.016, 1)
    ramp.color_ramp.elements[1].color = (0.10, 0.12, 0.16, 1)
    geo = w.node_tree.nodes.new("ShaderNodeTexCoord")
    sepz = w.node_tree.nodes.new("ShaderNodeSeparateXYZ")
    w.node_tree.links.new(geo.outputs["Generated"], sepz.inputs["Vector"])
    w.node_tree.links.new(sepz.outputs["Z"], ramp.inputs["Fac"])
    w.node_tree.links.new(ramp.outputs["Color"], bg.inputs["Color"])
    sc.render.engine = "CYCLES"
    sc.cycles.samples = 32
    sc.cycles.use_denoising = True
    sc.view_settings.view_transform = "AgX"
    sc.render.resolution_x, sc.render.resolution_y = 640, 400
    sc.render.filepath = os.path.join(OUT, setname + ".png")
    bpy.ops.render.render(write_still=True)
    print("LOOKDEV", setname)


for nm in names:
    render(nm)
