"""Close-up PBR material preview of a texture set on a flat tiling plane (Cycles).
Shows the material crisply (panels/bevels/emissive) the way a material-ball preview
works — this is where texture detail reads, unlike whole-ship distant framing.
Usage: blender -b --python _matpreview.py -- T_Fighter_Demo T_Ship_Hull
Output: Assets/FBX/generated/texture_previews/<set>.png
"""
import bpy, os, sys
import numpy as np
from mathutils import Vector

GEN = r"C:\Users\akuma\Adastrea\Assets\FBX\generated"
TEX = os.path.join(GEN, "Textures")
OUT = os.path.join(GEN, "texture_previews")
os.makedirs(OUT, exist_ok=True)

def load_img(fn):
    p = os.path.join(TEX, fn.replace("T_", "T_"))
    # caller passes full name like 'T_Fighter_Demo_D.png'
    p = os.path.join(TEX, fn)
    return p if os.path.exists(p) else None

def make_material(setname):
    mat = bpy.data.materials.new(f"Mat_{setname}")
    mat.use_nodes = True
    nt = mat.node_tree
    nt.nodes.clear()
    out = nt.nodes.new('ShaderNodeOutputMaterial'); out.location=(600,0)
    bsdf = nt.nodes.new('ShaderNodeBsdfPrincipled'); bsdf.location=(250,0)
    nt.links.new(bsdf.outputs['BSDF'], out.inputs['Surface'])
    def t(fn, cs='sRGB', x=-600, y=0):
        p = load_img(fn)
        if not p: return None
        img = bpy.data.images.load(p, check_existing=True)
        try: img.colorspace_settings.name = cs
        except Exception: pass
        n = nt.nodes.new('ShaderNodeTexImage'); n.image=img; n.location=(x,y)
        n.extension = 'REPEAT'
        return n
    # base color
    base = t(f"{setname}_D.png")
    if base:
        nt.links.new(base.outputs['Color'], bsdf.inputs['Base Color'])
    # normal
    nrm = t(f"{setname}_N.png", 'Non-Color', -600, -200)
    if nrm:
        nm = nt.nodes.new('ShaderNodeNormalMap'); nm.location=(0,-200)
        nm.inputs['Strength'].default_value = 1.0
        nt.links.new(nrm.outputs['Color'], nm.inputs['Color'])
        nt.links.new(nm.outputs['Normal'], bsdf.inputs['Normal'])
    # roughness
    r = t(f"{setname}_R.png", 'Non-Color', -600, -400)
    if r: nt.links.new(r.outputs['Color'], bsdf.inputs['Roughness'])
    # metallic
    m = t(f"{setname}_M.png", 'Non-Color', -600, -500)
    if m: nt.links.new(m.outputs['Color'], bsdf.inputs['Metallic'])
    # emissive (luminance keyed MixShader)
    e = t(f"{setname}_E.png", 'sRGB', -600, 250)
    if e:
        emit = nt.nodes.new('ShaderNodeEmission'); emit.location=(100,260)
        nt.links.new(e.outputs['Color'], emit.inputs['Color'])
        emit.inputs['Strength'].default_value = 3.0
        mix = nt.nodes.new('ShaderNodeMixShader'); mix.location=(430,120)
        lum = nt.nodes.new('ShaderNodeSeparateColor'); lum.location=(-150,400)
        nt.links.new(e.outputs['Color'], lum.inputs['Color'])
        val = nt.nodes.new('ShaderNodeMath'); val.operation='MULTIPLY_ADD'; val.inputs[1].default_value=4.0
        nt.links.new(lum.outputs['Red'], val.inputs[0]); nt.links.new(val.outputs['Value'], mix.inputs['Fac'])
        nt.links.new(bsdf.outputs['BSDF'], mix.inputs[1])
        nt.links.new(emit.outputs['Emission'], mix.inputs[2])
        nt.links.new(mix.outputs['Shader'], out.inputs['Surface'])
    return mat

def render(setname):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    # a large tiling plane tilted for perspective
    bpy.ops.mesh.primitive_plane_add(size=10, location=(0,0,0))
    plane = bpy.context.active_object
    plane.rotation_euler = (np.radians(-35), 0, np.radians(25))
    mat = make_material(setname)
    if plane.data.materials:
        plane.data.materials[0]=mat
    else:
        plane.data.materials.append(mat)
    # camera close to a region of the plane
    sc = bpy.context.scene
    sc.render.engine='CYCLES'
    try: sc.cycles.samples=96
    except Exception: pass
    cam_data = bpy.data.cameras.new("C"); cam_data.lens=50
    cam = bpy.data.objects.new("Cam", cam_data); sc.collection.objects.link(cam); sc.camera=cam
    cam.location = (8, -8, 8)
    cam.rotation_euler = (np.radians(55), 0, np.radians(45))
    world = sc.world if sc.world else bpy.data.worlds.new("W"); sc.world=world; world.use_nodes=True
    # ensure a Background + Output world node exist and are linked
    bg = next((n for n in world.node_tree.nodes if n.type == 'BACKGROUND'), None)
    if bg is None:
        bg = world.node_tree.nodes.new('ShaderNodeBackground')
    bg.inputs[0].default_value = (0.08, 0.09, 0.11, 1.0)
    bg.inputs[1].default_value = 1.0
    outW = next((n for n in world.node_tree.nodes if n.type == 'OUTPUT_WORLD'), None)
    if outW is None:
        outW = world.node_tree.nodes.new('ShaderNodeOutputWorld')
    if bg.outputs['Background'].is_linked is False:
        world.node_tree.links.new(bg.outputs['Background'], outW.inputs['Surface'])
    # light
    bpy.ops.object.light_add(type='AREA', location=(10, 10, 12))
    for o in bpy.data.objects:
        if o.type=='LIGHT':
            o.data.energy=60; o.data.size=8
            o.rotation_euler=(np.radians(40),0,np.radians(30))
    sc.render.resolution_x=1400; sc.render.resolution_y=1000
    sc.render.image_settings.file_format='PNG'
    outp=os.path.join(OUT, setname+".png")
    sc.render.filepath=outp
    bpy.ops.render.render(write_still=True)
    print("OK", setname, os.path.exists(outp))

for arg in sys.argv[sys.argv.index("--")+1:]:
    try:
        render(arg)
    except Exception as e:
        import traceback; traceback.print_exc(); print("ERR", arg)