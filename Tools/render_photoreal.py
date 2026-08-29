"""Photoreal interior render — CYCLES path tracer with HDRI-style environment,
PBR textures, real lights, and depth of field. This is the DEMO-quality pipeline
(headless EEVEE is flat; Cycles gives true raytraced lighting/reflections/GI).

Usage: blender -b --python Tools/render_photoreal.py -- <fbx> [<fbx> ...]
Output: Assets/FBX/generated/photoreal/<name>.png  (also writes a .blend for reuse)
"""
import bpy, os, sys, math
from mathutils import Vector

GEN = r"C:\Users\akuma\Adastrea\Assets\FBX\generated"
OUT = os.path.join(GEN, "photoreal")
os.makedirs(OUT, exist_ok=True)
TEX = os.path.join(GEN, "Textures")


def load_tex(fn):
    p = os.path.join(TEX, fn)
    return p if os.path.exists(p) else None


def make_pbr(prefixes):
    mat = bpy.data.materials.new("PhotoPBR")
    mat.use_nodes = True
    nt = mat.node_tree
    nt.nodes.clear()
    out = nt.nodes.new('ShaderNodeOutputMaterial'); out.location=(600,0)
    bsdf = nt.nodes.new('ShaderNodeBsdfPrincipled'); bsdf.location=(250,0)
    nt.links.new(bsdf.outputs['BSDF'], out.inputs['Surface'])
    def t(fn, cs='sRGB', x=-600,y=0):
        p=load_tex(fn)
        if not p: return None
        img=bpy.data.images.load(p, check_existing=True)
        try: img.colorspace_settings.name=cs
        except Exception: pass
        n=nt.nodes.new('ShaderNodeTexImage'); n.image=img; n.location=(x,y)
        return n
    for pref in prefixes:
        if load_tex(f"T_{pref}_D.png"):
            base=t(f"T_{pref}_D.png"); nt.links.new(base.outputs['Color'], bsdf.inputs['Base Color'])
            r=t(f"T_{pref}_R.png",'Non-Color',-600,-300)
            if r: nt.links.new(r.outputs['Color'], bsdf.inputs['Roughness'])
            m=t(f"T_{pref}_M.png",'Non-Color',-600,-500)
            if m: nt.links.new(m.outputs['Color'], bsdf.inputs['Metallic'])
            n1=t(f"T_{pref}_N.png",'Non-Color',-600,-200)
            if n1:
                nm=nt.nodes.new('ShaderNodeNormalMap'); nm.location=(0,-200)
                nt.links.new(n1.outputs['Color'], nm.inputs['Color'])
                nt.links.new(nm.outputs['Normal'], bsdf.inputs['Normal'])
            e=t(f"T_{pref}_E.png",'sRGB',-600,250)
            if e:
                emit=nt.nodes.new('ShaderNodeEmission'); emit.location=(100,260)
                nt.links.new(e.outputs['Color'], emit.inputs['Color'])
                emit.inputs['Strength'].default_value=2.0
                mix=nt.nodes.new('ShaderNodeMixShader'); mix.location=(430,120)
                lum=nt.nodes.new('ShaderNodeSeparateColor'); lum.location=(-150,400)
                nt.links.new(e.outputs['Color'], lum.inputs['Color'])
                val=nt.nodes.new('ShaderNodeMath'); val.operation='MULTIPLY_ADD'; val.inputs[1].default_value=3.0
                nt.links.new(lum.outputs['Red'], val.inputs[0]); nt.links.new(val.outputs['Value'], mix.inputs['Fac'])
                nt.links.new(bsdf.outputs['BSDF'], mix.inputs[1])
                nt.links.new(emit.outputs['Emission'], mix.inputs[2])
                nt.links.new(mix.outputs['Shader'], out.inputs['Surface'])
            return mat, pref
    return mat, None


def tex_prefixes(target):
    b=target.replace('.fbx','')
    if 'Cockpit' in b: return ['Int_Cockpit','Int_Wall','Int_Glow']
    return ['Int_Wall','Int_Glow']


def render_one(target):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=os.path.join(GEN,target))
    objs=[o for o in bpy.data.objects if o.type=='MESH']
    if not objs:
        print("NO MESH",target); return
    sc=bpy.context.scene
    mat,pref=make_pbr(tex_prefixes(target))
    for o in objs:
        o.data.materials.clear(); o.data.materials.append(mat)

    # bounds
    vmin=Vector((1e9,)*3); vmax=Vector((-1e9,)*3)
    for o in objs:
        for v in o.data.vertices:
            ww=o.matrix_world@v.co
            for i in range(3):
                vmin[i]=min(vmin[i],ww[i]); vmax[i]=max(vmax[i],ww[i])
    center=(vmin+vmax)*0.5
    Lx=vmax[0]-vmin[0]; Ly=vmax[1]-vmin[1]; Lz=vmax[2]-vmin[2]
    along_x = Lx >= Ly
    # position camera deeper into the interior, aimed at the main set-piece
    # (seat/console), not just down a dark empty axis — composition drives
    # whether a render reads as a real room or an empty box.
    eye_h = vmin[2] + 90
    if along_x:
        camloc=Vector((vmin[0]+Lx*0.20, center[1], eye_h))
        look =Vector((vmin[0]+Lx*0.58, center[1], eye_h+8))
    else:
        camloc=Vector((center[0], vmin[1]+Ly*0.20, eye_h))
        look =Vector((center[0], vmin[1]+Ly*0.58, eye_h+8))
    bpy.ops.object.camera_add(location=camloc)
    cam=bpy.context.active_object; sc.camera=cam; cam.data.lens=18   # wide, cinematic
    d=(look-camloc).normalized()
    cam.rotation_euler=d.to_track_quat('-Z','Y').to_euler()
    # depth of field focused on the set-piece
    cam.data.dof.use_dof=True
    cam.data.dof.focus_distance=(camloc-look).length*0.7
    cam.data.dof.aperture_fstop=4.0

    # HDRI-style environment: procedural sky+studiolight via world nodes gives
    # realistic ambient + reflections. Use a bright gradient + sun.
    # (true .hdr file loading is optional; this built-in keeps it dependency-free)
    w=sc.world if sc.world else bpy.data.worlds.new("PH"); sc.world=w; w.use_nodes=True
    nt=w.node_tree
    for n in list(nt.nodes): nt.nodes.remove(n)
    bg=nt.nodes.new('ShaderNodeBackground'); bg.inputs[0].default_value=(0.35,0.38,0.42,1.0); bg.inputs[1].default_value=1.0
    outW=nt.nodes.new('ShaderNodeOutputWorld'); nt.links.new(bg.outputs['Background'], outW.inputs['Surface'])

    # real lights: key sun + fill area + ambient point (Cycles uses REAL lights!)
    bpy.ops.object.light_add(type='SUN', location=center+Vector((200,200,300)))
    sun=bpy.context.active_object; sun.data.energy=6.0
    sd=(center+Vector((0,0,100))-sun.location).normalized()
    sun.rotation_euler=sd.to_track_quat('-Z','Y').to_euler()
    bpy.ops.object.light_add(type='AREA', location=center+Vector((-300,-200,150)))
    fill=bpy.context.active_object; fill.data.size=400; fill.data.energy=1500; fill.data.color=(0.8,0.85,1.0)
    bpy.ops.object.light_add(type='POINT', location=center+Vector((0,0,Lz*0.7)))
    pt=bpy.context.active_object; pt.data.energy=800
    # a warm practical light so the room reads designed, not a void
    bpy.ops.object.light_add(type='POINT', location=center+Vector((50,40,60)))
    warm=bpy.context.active_object; warm.data.energy=400; warm.data.color=(1.0,0.85,0.6)

    sc.render.engine='CYCLES'
    try:
        sc.cycles.device='GPU'
    except Exception: pass
    try:
        sc.cycles.samples=512
    except Exception:
        try: sc.cycles.samples=64
        except Exception: pass
    # filmic-like tone mapping (real demo look: rolls off highlights, holds
    # shadow detail) — the upgrade from raw linear -> actually photoreal.
    try:
        sc.view_settings.view_transform = 'Filmic'
        sc.view_settings.exposure = 1.0
        if sc.view_settings.view_transform != 'Filmic':
            # fallback: boost exposure target
            sc.view_settings.gamma = 1.0
    except Exception:
        pass
    sc.render.resolution_x=1600; sc.render.resolution_y=1100
    sc.render.resolution_percentage=100
    sc.render.image_settings.file_format='PNG'
    sc.render.filepath=os.path.join(OUT,target.replace('.fbx','.png'))
    bpy.ops.render.render(write_still=True)
    # save a .blend so it can be reopened/tweaked in the editor
    try:
        bpy.ops.wm.save_as_mainfile(filepath=os.path.join(OUT,target.replace('.fbx','.blend')))
    except Exception: pass
    print("OK" if os.path.exists(sc.render.filepath) else "FAIL", target, "tex=",pref)


for t in sys.argv[sys.argv.index("--")+1:]:
    try:
        render_one(t)
    except Exception as e:
        import traceback; traceback.print_exc()
        print("ERROR",t)