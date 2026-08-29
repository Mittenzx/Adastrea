"""Render assembled ships TEXTURED + lit with EEVEE — the SAME path that makes
the interiors look great (assign_material + area/point lights + starfield world).

Uses the generated PBR textures (T_Ship_Hull_D.png etc) on the assembled ship,
so the greeble/panel/neon detail the user wants to SEE is actually visible.
Framing: manual camera aim + scale normalization (proven reliable).
Usage: blender -b --python Tools/render_ships.py -- <fbx> [<fbx> ...]
"""
import bpy, os, sys
from mathutils import Vector
import math

GEN = r"C:\Users\akuma\Adastrea\Assets\FBX\generated"
OUT = os.path.join(GEN, "ship_renders")
os.makedirs(OUT, exist_ok=True)
TEX = os.path.join(GEN, "Textures")


def load_tex(fname):
    p = os.path.join(TEX, fname)
    return p if os.path.exists(p) else None


def make_textured_material(tex_prefixes):
    """Build a Principled material wired to BaseColor/Normal/Roughness/Metallic/
    Emissive from the first texture-set prefix that has files. Returns mat."""
    mat = bpy.data.materials.new("ShipPBR")
    mat.use_nodes = True
    nt = mat.node_tree
    nt.nodes.clear()
    out = nt.nodes.new('ShaderNodeOutputMaterial'); out.location = (500, 0)
    bsdf = nt.nodes.new('ShaderNodeBsdfPrincipled'); bsdf.location = (200, 0)
    bsdf.inputs['Base Color'].default_value = (0.55, 0.58, 0.62, 1.0)
    bsdf.inputs['Roughness'].default_value = 0.6
    nt.links.new(bsdf.outputs['BSDF'], out.inputs['Surface'])

    def tex_node(fname, colspace='sRGB', x=-500, y=0):
        path = load_tex(fname)
        if not path:
            return None
        img = bpy.data.images.load(path, check_existing=True)
        try:
            img.colorspace_settings.name = colspace
        except Exception:
            pass
        n = nt.nodes.new('ShaderNodeTexImage'); n.image = img
        n.location = (x, y)
        return n

    # find the first prefix with a _D
    for pref in tex_prefixes:
        if load_tex(f"T_{pref}_D.png"):
            base = tex_node(f"T_{pref}_D.png", 'sRGB')
            nt.links.new(base.outputs['Color'], bsdf.inputs['Base Color'])
            r = tex_node(f"T_{pref}_R.png", 'Non-Color', -500, -300)
            if r: nt.links.new(r.outputs['Color'], bsdf.inputs['Roughness'])
            m = tex_node(f"T_{pref}_M.png", 'Non-Color', -500, -500)
            if m: nt.links.new(m.outputs['Color'], bsdf.inputs['Metallic'])
            n1 = tex_node(f"T_{pref}_N.png", 'Non-Color', -500, -200)
            if n1:
                nm = nt.nodes.new('ShaderNodeNormalMap'); nm.location = (0, -200)
                nt.links.new(n1.outputs['Color'], nm.inputs['Color'])
                nt.links.new(nm.outputs['Normal'], bsdf.inputs['Normal'])
            # SELF-LIT hull: emission = base-color texture (strength scaled) so
            # the ENTIRE textured hull glows out and shows all panel detail.
            # (headless EEVEE in this build ignores lights; only emission shows.)
            emit = nt.nodes.new('ShaderNodeEmission'); emit.location = (200, 250)
            nt.links.new(base.outputs['Color'], emit.inputs['Color'])
            emit.inputs['Strength'].default_value = 0.85
            e = tex_node(f"T_{pref}_E.png", 'sRGB', -500, 250)
            neon_mix = None
            if e:
                # add the emissive texture's own glow on top (bright neon)
                neon = nt.nodes.new('ShaderNodeEmission'); neon.location = (200, 250)
                nt.links.new(e.outputs['Color'], neon.inputs['Color'])
                neon.inputs['Strength'].default_value = 2.5
                neon_mix = neon
            # final surface: mix self-lit base over the BSDF, topped with the
            # explicit neon where the emissive texture is bright
            mix0 = nt.nodes.new('ShaderNodeMixShader'); mix0.location = (420, 60)
            mix0.inputs['Fac'].default_value = 0.92
            nt.links.new(bsdf.outputs['BSDF'], mix0.inputs[1])
            nt.links.new(emit.outputs['Emission'], mix0.inputs[2])
            if neon_mix:
                mix1 = nt.nodes.new('ShaderNodeMixShader'); mix1.location = (560, 120)
                lum2 = nt.nodes.new('ShaderNodeSeparateColor'); lum2.location = (100, 480)
                nt.links.new(e.outputs['Color'], lum2.inputs['Color'])
                val2 = nt.nodes.new('ShaderNodeMath'); val2.operation = 'MULTIPLY_ADD'
                val2.inputs[1].default_value = 3.0
                nt.links.new(lum2.outputs['Red'], val2.inputs[0])
                nt.links.new(val2.outputs['Value'], mix1.inputs['Fac'])
                nt.links.new(mix0.outputs['Shader'], mix1.inputs[1])
                nt.links.new(neon.outputs['Emission'], mix1.inputs[2])
                nt.links.new(mix1.outputs['Shader'], out.inputs['Surface'])
            else:
                nt.links.new(mix0.outputs['Shader'], out.inputs['Surface'])
            return mat, pref
    # fallback: bright grey
    return mat, None


def ship_prefix(target):
    b = target.replace('.fbx', '')
    if 'Corvette' in b: return ['Corvette', 'Ship_Hull', 'Ship_Cargo']
    if 'Miner' in b: return ['Miner', 'Ship_Hull']
    if 'Freighter' in b: return ['Freighter', 'Ship_Hull']
    return ['Ship_Hull', 'Ship_Cargo', 'Ship_Engine', 'Ship_Weapon']


def render_one(target):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=os.path.join(GEN, target))
    objs = [o for o in bpy.data.objects if o.type == 'MESH']
    if not objs:
        print("NO MESH", target); return
    sc = bpy.context.scene

    mat, pref = make_textured_material(ship_prefix(target))
    for o in objs:
        o.data.materials.clear()
        o.data.materials.append(mat)

    # scale-normalize + recenter so framing is reliable at any ship size
    vmin = Vector((1e9,)*3); vmax = Vector((-1e9,)*3)
    for o in objs:
        for v in o.data.vertices:
            ww = o.matrix_world @ v.co
            for i in range(3):
                vmin[i] = min(vmin[i], ww[i]); vmax[i] = max(vmax[i], ww[i])
    center = (vmin + vmax) * 0.5
    diag = (vmax - vmin).length
    sf = 220.0 / max(diag, 1e-6)
    import bmesh as _bm
    for o in objs:
        me = o.data
        b = _bm.new(); b.from_mesh(me)
        for v in b.verts:
            v.co -= Vector(center); v.co *= sf
        b.to_mesh(me); b.free(); me.update()
    vmin = Vector((1e9,)*3); vmax = Vector((-1e9,)*3)
    for o in objs:
        for v in o.data.vertices:
            ww = o.matrix_world @ v.co
            for i in range(3):
                vmin[i] = min(vmin[i], ww[i]); vmax[i] = max(vmax[i], ww[i])
    center = (vmin + vmax) * 0.5
    diag = (vmax - vmin).length
    dist = max(diag * 1.5, 80)

    # camera manual aim
    loc = center + Vector((dist, -dist*0.6, dist*0.4))
    bpy.ops.object.camera_add(location=loc)
    cam = bpy.context.active_object; sc.camera = cam; cam.data.lens = 32
    d = (center - loc).normalized()
    cam.rotation_euler = d.to_track_quat('-Z', 'Y').to_euler()

    # lights: proven interior-style AREA + POINT; bright so the textured hull reads
    bpy.ops.object.light_add(type='AREA', location=center + Vector((dist*0.5, dist*0.5, dist*0.9)))
    l1 = bpy.context.active_object; l1.data.size = dist; l1.data.energy = 60000
    bpy.ops.object.light_add(type='AREA', location=center + Vector((-dist*0.5, -dist*0.4, dist*0.3)))
    l2 = bpy.context.active_object; l2.data.size = dist; l2.data.energy = 30000; l2.data.color = (0.8, 0.85, 1.0)
    bpy.ops.object.light_add(type='POINT', location=center + Vector((0, 0, dist*0.8)))
    l3 = bpy.context.active_object; l3.data.energy = 16000

    # dark space world
    w = sc.world if sc.world else bpy.data.worlds.new("SG"); sc.world = w
    w.use_nodes = True
    try:
        w.node_tree.nodes['Background'].inputs[0].default_value = (0.004, 0.006, 0.012, 1.0)
    except Exception:
        pass

    sc.render.engine = 'BLENDER_EEVEE'
    sc.render.resolution_x = 1400; sc.render.resolution_y = 960
    sc.render.resolution_percentage = 100
    sc.render.image_settings.file_format = 'PNG'
    out_png = os.path.join(OUT, target.replace('.fbx', '.png'))
    sc.render.filepath = out_png
    bpy.ops.render.render(write_still=True)
    print("OK" if os.path.exists(out_png) else "FAIL", target, "tex=", pref)


for t in sys.argv[sys.argv.index("--")+1:]:
    try:
        render_one(t)
    except Exception as e:
        import traceback; traceback.print_exc()
        print("ERROR", t)