"""Render generated FBX/interiors with REAL Blender (EEVEE) to PNG.

Earlier EEVEE offline renders silently produced no file — the fix is to call
bpy.ops.render.render(write_still=True) (NOT 'INVOKE_DEFAULT', which only
schedules), and to set the output via scene.render.filepath + image settings.
Falls back to BLENDER_WORKBENCH if EEVEE still writes nothing (Workbench always
renders headless).
"""
import bpy, os, math, sys
from mathutils import Vector

GEN = r"C:\Users\akuma\Adastrea\Assets\FBX\generated"
OUT = os.path.join(GEN, "blender_previews")
os.makedirs(OUT, exist_ok=True)

def clear_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    sc = bpy.context.scene
    sc.unit_settings.system = 'METRIC'
    sc.unit_settings.length_unit = 'CENTIMETERS'

def frame_mesh():
    """Center camera + lights on the active mesh's bounds, return radius."""
    objs = [o for o in bpy.data.objects if o.type == 'MESH']
    if not objs:
        return 50
    vmin = Vector((1e9,1e9,1e9)); vmax = Vector((-1e9,-1e9,-1e9))
    for o in objs:
        for v in o.data.vertices:
            w = o.matrix_world @ v.co
            for i in range(3):
                vmin[i] = min(vmin[i], w[i]); vmax[i] = max(vmax[i], w[i])
    center = (vmin + vmax) * 0.5
    radius = max((vmax - vmin).length * 0.8, 60)
    # camera
    for c in [o for o in bpy.data.objects if o.type == 'CAMERA']:
        bpy.data.objects.remove(c, do_unlink=True)
    bpy.ops.object.camera_add(location=center + Vector((radius, -radius*0.8, radius*0.5)))
    cam = bpy.context.active_object
    bpy.context.scene.camera = cam
    # track to center
    target = bpy.data.objects.new("Target", None)
    target.location = center
    bpy.context.collection.objects.link(target)
    t = cam.constraints.new('TRACK_TO'); t.target = target; t.track_axis='TRACK_NEGATIVE_Z'; t.up_axis='UP_Y'
    # lights: large, well-powered area lights for a readable preview
    for l in [o for o in bpy.data.objects if o.type=='LIGHT']:
        bpy.data.objects.remove(l, do_unlink=True)
    # key light (big soft area from front-top)
    bpy.ops.object.light_add(type='AREA', location=center+Vector((radius*0.5, radius*0.5, radius*0.6)))
    l1=bpy.context.active_object
    l1.scale=(2,2,2)          # grow the light source
    l1.data.size=radius*0.8
    l1.data.energy=8000
    # rim/fill from back-left
    bpy.ops.object.light_add(type='AREA', location=center+Vector((-radius*0.7, -radius*0.6, radius*0.3)))
    l2=bpy.context.active_object
    l2.data.size=radius*0.8
    l2.data.energy=4000; l2.data.color=(0.8,0.85,1.0)
    # a point light to lift the interior dark spots
    bpy.ops.object.light_add(type='POINT', location=center+Vector((0,0,radius*0.8)))
    l3=bpy.context.active_object; l3.data.energy=3000
    return radius

def make_pbr_material(mat_name, tex_prefix, out_png_base):
    """Build a Principled BSDF material wired to the generated PBR textures
    (BaseColor _D, Normal _N, Rougness _R, Metallic _M, AO _AO, Emissive _E).
    Returns the material, or None if no textures found (rare fallback = grey).
    """
    import os as _os
    texbase = os.path.join(GEN, "Textures")
    # build filename candidates: T_<prefix>_D.png etc
    def find(suffix):
        # try a few prefixes (exact, Int_, Freighter_, Ship_Hull_) 
        for cand in (f"T_{tex_prefix}{suffix}",):
            p = os.path.join(texbase, cand + ".png")
            if os.path.exists(p):
                return cand, p
        return None, None

    mat = bpy.data.materials.new(mat_name)
    mat.use_nodes = True
    nt = mat.node_tree
    nt.nodes.clear()
    out = nt.nodes.new('ShaderNodeOutputMaterial')
    out.location = (400, 0)
    bsdf = nt.nodes.new('ShaderNodeBsdfPrincipled')
    bsdf.location = (100, 0)

    def add_tex(png_name, path, label, conn_socket, color_space='sRGB'):
        if not os.path.exists(path):
            return None
        tex = bpy.data.images.load(path, check_existing=True)
        node = nt.nodes.new('ShaderNodeTexImage')
        node.image = tex
        node.label = label
        node.location = (-600, 0)
        try:
            if color_space == 'Non-Color':
                tex.colorspace_settings.name = 'Non-Color'
            else:
                tex.colorspace_settings.name = 'sRGB'
        except Exception:
            pass
        if conn_socket is not None:
            nt.links.new(node.outputs['Color'], conn_socket)
        return node

    # BaseColor
    bn, bp = find('_D')
    if bn:
        inp = bsdf.inputs['Base Color']
        add_tex(bn, bp, 'BaseColor', inp, 'sRGB')
    # Roughness
    rn, rp = find('_R')
    if rn:
        add_tex(rn, rp, 'Roughness', bsdf.inputs['Roughness'], 'Non-Color')
    # Metallic
    mn, mp = find('_M')
    if mn:
        add_tex(mn, mp, 'Metallic', bsdf.inputs['Metallic'], 'Non-Color')
    # Normal
    nn, np_ = find('_N')
    if nn:
        norm_mp = nt.nodes.new('ShaderNodeNormalMap')
        norm_mp.location = (-150, -120)
        nt.links.new(bsdf.inputs['Normal'], norm_mp.outputs['Normal'])
        add_tex(nn, np_, 'Normal', norm_mp.inputs['Color'], 'Non-Color')
    # Emissive -> Emission + strength
    en, ep = find('_E')
    if en:
        em_node = add_tex(en, ep, 'Emissive', None, 'sRGB')   # creates node, no link yet
        if em_node:
            # add emission from the emissive texture
            emit = nt.nodes.new('ShaderNodeEmission')
            emit.location = (100, 260)
            nt.links.new(em_node.outputs['Color'], emit.inputs['Color'])
            # mix emission over the bsdf; Fac driven by emissive brightness
            mix = nt.nodes.new('ShaderNodeMixShader')
            mix.location = (250, 120)
            nt.links.new(bsdf.outputs[0], mix.inputs[1])
            nt.links.new(emit.outputs['Emission'], mix.inputs[2])
            # use emissive texture's luminance as the mix factor
            lum = nt.nodes.new('ShaderNodeSeparateColor')
            lum.location = (0, 320)
            nt.links.new(em_node.outputs['Color'], lum.inputs['Color'])
            # separate to RGB, take max-ish via a math node into Fac
            vals = nt.nodes.new('ShaderNodeMath'); vals.operation='MULTIPLY_ADD'
            vals.inputs[1].default_value = 2.0   # boost
            vals.inputs[2].default_value = 0.0
            nt.links.new(lum.outputs['Red'], vals.inputs[0])
            nt.links.new(vals.outputs[0], mix.inputs['Fac'])
            nt.links.new(mix.outputs[0], out.inputs['Surface'])
        else:
            nt.links.new(bsdf.outputs[0], out.inputs['Surface'])
    else:
        nt.links.new(bsdf.outputs[0], out.inputs['Surface'])

    if not os.listdir(os.path.join(GEN, 'Textures')):
        return None
    return mat


def assign_material(fbx_path, tex_prefix):
    """Assign a textured PBR material to all meshes in the scene."""
    # try to build the textured mat from the interior texture set most relevant
    mat = None
    # guess prefix: interiors -> Int_Wall/Int_Cockpit; else by fbx name
    base = os.path.splitext(os.path.basename(fbx_path))[0]
    candidates = []
    if base.startswith('SM_Int'):
        if 'Cockpit' in base:
            candidates = ['Int_Cockpit', 'Int_Wall', 'Int_Glow']
        else:
            candidates = ['Int_Glow', 'Int_Wall', 'Int_Deck']
    elif 'HabRing' in base or 'Generationship' in base:
        candidates = ['HabRing', 'AsteroidShell', 'Ship_Hull']
    elif 'Freighter' in base:
        candidates = ['Freighter', 'Ship_Hull']
    elif 'Gunship' in base:
        candidates = ['Gunship', 'Ship_Hull']
    elif 'Station' in base or 'Asteroid' in base:
        candidates = ['AsteroidShell', 'Station_Hab', 'Ship_Hull']
    else:
        candidates = ['Ship_Hull', 'Freighter', 'Gunship']

    for pref in candidates:
        mat = make_pbr_material(f"Mat_{pref}", pref, out_png_base=None)
        if mat is not None:
            break
    if mat is None:
        mat = bpy.data.materials.new("PreviewGrey")
        mat.use_nodes = True
    # assign to all mesh objects
    for o in [o for o in bpy.data.objects if o.type == 'MESH']:
        if not o.data.materials:
            o.data.materials.append(mat)
        else:
            o.data.materials[0] = mat
    return mat


def render_fbx(fbx_path, out_png, engine='BLENDER_EEVEE'):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=fbx_path)
    sc = bpy.context.scene
    sc.unit_settings.system='METRIC'; sc.unit_settings.length_unit='CENTIMETERS'

    # assign a real textured PBR material (so it isn't a flat grey blob)
    assign_material(fbx_path, None)

    # brighten world so nothing is pitch black, but keep slight ambient
    if sc.world is None:
        sc.world = bpy.data.worlds.new("W")
    w = sc.world
    w.use_nodes = True
    try:
        bg = w.node_tree.nodes.get("Background")
        if bg:
            bg.inputs[0].default_value = (0.30, 0.32, 0.36, 1.0)
            bg.inputs[1].default_value = 0.4
    except Exception:
        pass

    frame_mesh()
    sc.render.engine = engine
    sc.render.resolution_x=1200; sc.render.resolution_y=825; sc.render.resolution_percentage=100
    sc.render.image_settings.file_format='PNG'
    sc.render.filepath = out_png
    bpy.ops.render.render(write_still=True)
    return os.path.exists(out_png)

if __name__ == "__main__":
    targets = sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
    if not targets:
        # default: all SM_ + SM_Int fbx
        targets = [f for f in os.listdir(GEN) if f.endswith('.fbx')]
    for f in targets:
        p = os.path.join(GEN, f)
        out = os.path.join(OUT, f.replace('.fbx','.png'))
        ok = render_fbx(p, out)
        if not ok:
            # workbench fallback
            ok2 = render_fbx(p, out, engine='BLENDER_WORKBENCH')
            print(f"{f}: eevEE={'OK' if ok else 'none'} workbench={'OK' if ok2 else 'none'} -> {os.path.basename(out)}")
        else:
            print(f"{f}: eevEE OK -> {os.path.basename(out)}")
