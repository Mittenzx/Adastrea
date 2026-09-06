"""Exterior studio render SCENE for ships — CYCLES photoreal with real lighting.

Creates a proper lit scene (floor + HDRI-ish environment + key/rim/fill light
rig), frames the whole ship from outside, applies its PBR textures, and renders
photoreal. This is the "see the ship with lighting" pipeline (headless EEVEE is
flat; Cycles gives true lights/shadows/reflections).

Usage: blender -b --python Tools/render_scene.py -- SM_Ship_Fighter_01_Assembled.fbx [more...]
Output: Assets/FBX/generated/scene_renders/<name>.png  (+ .blend for reuse)
"""
import bpy, os, sys, math
import bmesh
from mathutils import Vector

GEN = r"C:\Users\akuma\Adastrea\Assets\FBX\generated"
OUT = os.path.join(GEN, "scene_renders")
os.makedirs(OUT, exist_ok=True)
TEX = os.path.join(GEN, "Textures")


def tex_prefixes(target):
    b = target.replace('.fbx', '')
    if 'Demo' in b:       return ['Fighter_Demo']  # research demo ship
    if 'Freighter' in b: return ['Freighter', 'Ship_Hull']
    if 'Gunship' in b:   return ['Gunship', 'Ship_Hull']
    if 'Corvette' in b:  return ['Ship_Hull']
    if 'Miner' in b:     return ['Ship_Hull']
    if 'Station' in b:   return ['Station_Hab', 'Ship_Hull']
    if 'Int' in b:
        if 'Cockpit' in b: return ['Int_Cockpit', 'Int_Wall', 'Int_Glow']
        return ['Int_Wall', 'Int_Glow']
    return ['Ship_Hull']


def load_tex(fn):
    p = os.path.join(TEX, fn)
    return p if os.path.exists(p) else None


def make_pbr(prefixes):
    mat = bpy.data.materials.new("ScenePBR")
    mat.use_nodes = True
    nt = mat.node_tree
    nt.nodes.clear()
    out = nt.nodes.new('ShaderNodeOutputMaterial'); out.location = (600, 0)
    bsdf = nt.nodes.new('ShaderNodeBsdfPrincipled'); bsdf.location = (250, 0)
    nt.links.new(bsdf.outputs['BSDF'], out.inputs['Surface'])

    def t(fn, cs='sRGB', x=-600, y=0):
        p = load_tex(fn)
        if not p: return None
        img = bpy.data.images.load(p, check_existing=True)
        try: img.colorspace_settings.name = cs
        except Exception: pass
        n = nt.nodes.new('ShaderNodeTexImage'); n.image = img; n.location = (x, y)
        return n

    for pref in prefixes:
        if load_tex(f"T_{pref}_D.png"):
            base = t(f"T_{pref}_D.png")
            # lift the hull brightness: mix toward a lighter grey so the ship
            # reads as a lit metal body (not a black silhouette against the bg)
            mix_b = nt.nodes.new('ShaderNodeMixRGB')
            mix_b.blend_type = 'MIX'
            mix_b.inputs['Fac'].default_value = 0.65   # 65% toward light grey
            mix_b.inputs['Color2'].default_value = (0.55, 0.57, 0.62, 1.0)  # lighten grey
            nt.links.new(base.outputs['Color'], mix_b.inputs['Color1'])
            nt.links.new(mix_b.outputs['Color'], bsdf.inputs['Base Color'])
            r = t(f"T_{pref}_R.png", 'Non-Color', -600, -300)
            if r: nt.links.new(r.outputs['Color'], bsdf.inputs['Roughness'])
            m = t(f"T_{pref}_M.png", 'Non-Color', -600, -500)
            if m: nt.links.new(m.outputs['Color'], bsdf.inputs['Metallic'])
            n1 = t(f"T_{pref}_N.png", 'Non-Color', -600, -200)
            if n1:
                nm = nt.nodes.new('ShaderNodeNormalMap'); nm.location = (0, -200)
                nt.links.new(n1.outputs['Color'], nm.inputs['Color'])
                nt.links.new(nm.outputs['Normal'], bsdf.inputs['Normal'])
            e = t(f"T_{pref}_E.png", 'sRGB', -600, 250)
            if e:
                emit = nt.nodes.new('ShaderNodeEmission'); emit.location = (100, 260)
                nt.links.new(e.outputs['Color'], emit.inputs['Color'])
                emit.inputs['Strength'].default_value = 2.0
                mix = nt.nodes.new('ShaderNodeMixShader'); mix.location = (430, 120)
                lum = nt.nodes.new('ShaderNodeSeparateColor'); lum.location = (-150, 400)
                nt.links.new(e.outputs['Color'], lum.inputs['Color'])
                val = nt.nodes.new('ShaderNodeMath'); val.operation = 'MULTIPLY_ADD'; val.inputs[1].default_value = 3.0
                nt.links.new(lum.outputs['Red'], val.inputs[0]); nt.links.new(val.outputs['Value'], mix.inputs['Fac'])
                nt.links.new(bsdf.outputs['BSDF'], mix.inputs[1])
                nt.links.new(emit.outputs['Emission'], mix.inputs[2])
                nt.links.new(mix.outputs['Shader'], out.inputs['Surface'])
            return mat
    return mat


def render_one(target):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    fbx = os.path.join(GEN, target)
    if not os.path.exists(fbx):
        print("NO FBX", target); return
    bpy.ops.import_scene.fbx(filepath=fbx)
    objs = [o for o in bpy.data.objects if o.type == 'MESH']
    if not objs:
        print("NO MESH", target); return
    sc = bpy.context.scene
    mat = make_pbr(tex_prefixes(target))
    # Fix any inverted normals so Cycles doesn't render faces as black (the
    # dense-joined assembled meshes often carry inward normals; recalc outward).
    import bmesh as _bm
    for o in objs:
        me = o.data
        b = _bm.new(); b.from_mesh(me)
        for f in b.faces: f.select = True
        bmesh.ops.recalc_face_normals(b, faces=b.faces)
        b.to_mesh(me); b.free(); me.update()
    for o in objs:
        o.data.materials.clear(); o.data.materials.append(mat)

    # bounds
    vmin = Vector((1e9,)*3); vmax = Vector((-1e9,)*3)
    for o in objs:
        for v in o.data.vertices:
            ww = o.matrix_world @ v.co
            for i in range(3):
                vmin[i] = min(vmin[i], ww[i]); vmax[i] = max(vmax[i], ww[i])
    center = (vmin + vmax) * 0.5
    Lx, Ly, Lz = vmax[0]-vmin[0], vmax[1]-vmin[1], vmax[2]-vmin[2]
    diag = (Lx*Lx + Ly*Ly + Lz*Lz) ** 0.5

    # studio floor at ship base
    floor_y = vmin[2] - diag * 0.2
    bpy.ops.mesh.primitive_plane_add(size=diag * 3.5, location=(center[0], center[1], floor_y))
    floor = bpy.context.active_object
    fmat = bpy.data.materials.new("Floor"); fmat.use_nodes = True
    try:
        bs = fmat.node_tree.nodes['Principled BSDF']
        bs.inputs['Base Color'].default_value = (0.18, 0.19, 0.22, 1.0)
        bs.inputs['Roughness'].default_value = 0.4
    except Exception: pass
    floor.data.materials.append(fmat)

    # camera — exterior 3/4 turntable view framing the whole ship
    az, el = math.radians(42), math.radians(18)
    dist = diag * 1.5
    camloc = center + Vector((dist*math.cos(el)*math.sin(az),
                              dist*math.cos(el)*math.cos(az),
                              dist*math.sin(el)))
    bpy.ops.object.camera_add(location=camloc)
    cam = bpy.context.active_object; sc.camera = cam; cam.data.lens = 50
    look = center + Vector((0, 0, diag*0.15))
    d = (look - camloc).normalized()
    cam.rotation_euler = d.to_track_quat('-Z', 'Y').to_euler()
    cam.data.dof.use_dof = True
    cam.data.dof.focus_distance = (camloc - look).length
    cam.data.dof.aperture_fstop = 5.6

    # HDRI-style environment (soft gradient sky + subtle rim)
    w = sc.world if sc.world else bpy.data.worlds.new("SR"); sc.world = w; w.use_nodes = True
    nt = w.node_tree
    for n in list(nt.nodes): nt.nodes.remove(n)
    bg = nt.nodes.new('ShaderNodeBackground')
    bg.inputs[0].default_value = (0.12, 0.13, 0.16, 1.0)   # visible studio backdrop
    bg.inputs[1].default_value = 0.8
    outW = nt.nodes.new('ShaderNodeOutputWorld')
    nt.links.new(bg.outputs['Background'], outW.inputs['Surface'])

    # LIGHT RIG (Cycles uses real lights): key + rim + fill
    # key: warm area from upper-left
    bpy.ops.object.light_add(type='AREA', location=center + Vector((diag*0.9, diag*0.5, diag*0.7)))
    key = bpy.context.active_object; key.data.size = diag; key.data.energy = 1200; key.data.color = (1.0, 0.98, 0.92)
    dk = (center - key.location).normalized()
    key.rotation_euler = dk.to_track_quat('-Z', 'Y').to_euler()
    # rim: cool back-light to separate silhouette from bg
    bpy.ops.object.light_add(type='AREA', location=center + Vector((-diag*0.8, -diag*0.4, diag*0.5)))
    rim = bpy.context.active_object; rim.data.size = diag; rim.data.energy = 1200; rim.data.color = (0.5, 0.6, 1.0)
    dr = (center - rim.location).normalized()
    rim.rotation_euler = dr.to_track_quat('-Z', 'Y').to_euler()
    # fill: low cool bounce from below-front
    bpy.ops.object.light_add(type='AREA', location=center + Vector((0, diag*0.7, diag*0.1)))
    fill = bpy.context.active_object; fill.data.size = diag*0.8; fill.data.energy = 700; fill.data.color = (0.7, 0.8, 1.0)
    df = (center - fill.location).normalized()
    fill.rotation_euler = df.to_track_quat('-Z', 'Y').to_euler()

    sc.render.engine = 'CYCLES'
    try: sc.cycles.device = 'GPU'
    except Exception: pass
    try: sc.cycles.samples = 256
    except Exception:
        try: sc.cycles.samples = 64
        except Exception: pass
    try:
        sc.view_settings.view_transform = 'Filmic'
        sc.view_settings.exposure = 0.5
    except Exception:
        pass
    sc.render.resolution_x = 1600; sc.render.resolution_y = 1100
    sc.render.resolution_percentage = 100
    sc.render.image_settings.file_format = 'PNG'
    sc.render.film_transparent = False
    sc.render.filepath = os.path.join(OUT, target.replace('.fbx', '.png'))
    bpy.ops.render.render(write_still=True)
    try:
        bpy.ops.wm.save_as_mainfile(filepath=os.path.join(OUT, target.replace('.fbx', '.blend')))
    except Exception: pass
    print("OK" if os.path.exists(sc.render.filepath) else "FAIL", target)


for t in sys.argv[sys.argv.index("--")+1:]:
    try:
        render_one(t)
    except Exception as e:
        import traceback; traceback.print_exc()
        print("ERROR", t)