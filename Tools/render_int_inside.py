"""Render INTERIORS from INSIDE with textured EEVEE (self-lit hull/glow).

Reuses the proven self-lit emission approach (headless EEVEE ignores lights, so
materials self-light from their texture) with a camera placed INSIDE the room
looking along the long axis — gives a real "in the room" view with the Int_*
textures visible. Mirrors render_ships material logic.
Usage: blender -b --python Tools/render_int_inside.py -- <fbx> [<fbx> ...]
"""
import bpy, os, sys
from mathutils import Vector

GEN = r"C:\Users\akuma\Adastrea\Assets\FBX\generated"
OUT = os.path.join(GEN, "interior_inside")
os.makedirs(OUT, exist_ok=True)
TEX = os.path.join(GEN, "Textures")


def load_tex(fname):
    p = os.path.join(TEX, fname)
    return p if os.path.exists(p) else None


def make_tex_material(prefixes):
    mat = bpy.data.materials.new("IntPBR")
    mat.use_nodes = True
    nt = mat.node_tree
    nt.nodes.clear()
    out = nt.nodes.new('ShaderNodeOutputMaterial'); out.location=(500,0)
    bsdf = nt.nodes.new('ShaderNodeBsdfPrincipled'); bsdf.location=(200,0)
    bsdf.inputs['Base Color'].default_value=(0.4,0.42,0.45,1.0)
    nt.links.new(bsdf.outputs['BSDF'], out.inputs['Surface'])
    def tex_node(fn, cs='sRGB', x=-500,y=0):
        p=load_tex(fn)
        if not p: return None
        img=bpy.data.images.load(p, check_existing=True)
        try: img.colorspace_settings.name=cs
        except Exception: pass
        n=nt.nodes.new('ShaderNodeTexImage'); n.image=img; n.location=(x,y)
        return n
    for pref in prefixes:
        if load_tex(f"T_{pref}_D.png"):
            base=tex_node(f"T_{pref}_D.png", 'sRGB')
            nt.links.new(base.outputs['Color'], bsdf.inputs['Base Color'])
            # self-light base (texture shows via emission)
            emit=nt.nodes.new('ShaderNodeEmission'); emit.location=(200,250)
            nt.links.new(base.outputs['Color'], emit.inputs['Color'])
            emit.inputs['Strength'].default_value=2.0
            e=tex_node(f"T_{pref}_E.png", 'sRGB', -500, 250)  # glow overlay
            mix0=nt.nodes.new('ShaderNodeMixShader'); mix0.location=(420,60)
            mix0.inputs['Fac'].default_value=0.9
            nt.links.new(bsdf.outputs['BSDF'], mix0.inputs[1])
            nt.links.new(emit.outputs['Emission'], mix0.inputs[2])
            if e:
                neon=nt.nodes.new('ShaderNodeEmission'); neon.location=(200,250)
                nt.links.new(e.outputs['Color'], neon.inputs['Color'])
                neon.inputs['Strength'].default_value=2.5
                mix1=nt.nodes.new('ShaderNodeMixShader'); mix1.location=(560,120)
                lum=nt.nodes.new('ShaderNodeSeparateColor'); lum.location=(100,480)
                nt.links.new(e.outputs['Color'], lum.inputs['Color'])
                val=nt.nodes.new('ShaderNodeMath'); val.operation='MULTIPLY_ADD'
                val.inputs[1].default_value=3.0
                nt.links.new(lum.outputs['Red'], val.inputs[0])
                nt.links.new(val.outputs['Value'], mix1.inputs['Fac'])
                nt.links.new(mix0.outputs['Shader'], mix1.inputs[1])
                nt.links.new(neon.outputs['Emission'], mix1.inputs[2])
                nt.links.new(mix1.outputs['Shader'], out.inputs['Surface'])
            else:
                nt.links.new(mix0.outputs['Shader'], out.inputs['Surface'])
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
    mat,pref=make_tex_material(tex_prefixes(target))
    for o in objs:
        o.data.materials.clear(); o.data.materials.append(mat)

    vmin=Vector((1e9,)*3); vmax=Vector((-1e9,)*3)
    for o in objs:
        for v in o.data.vertices:
            ww=o.matrix_world@v.co
            for i in range(3):
                vmin[i]=min(vmin[i],ww[i]); vmax[i]=max(vmax[i],ww[i])
    center=(vmin+vmax)*0.5
    Lx=vmax[0]-vmin[0]; Ly=vmax[1]-vmin[1]; Lz=vmax[2]-vmin[2]
    # place camera inside, along longest horizontal axis, standing height
    along_x = Lx >= Ly
    eye_h = vmin[2] + 100
    if along_x:
        camloc=Vector((vmin[0]+Lx*0.30, center[1], eye_h))
        look = Vector((vmin[0]+Lx*0.70, center[1], eye_h+5))
    else:
        camloc=Vector((center[0], vmin[1]+Ly*0.30, eye_h))
        look = Vector((center[0], vmin[1]+Ly*0.70, eye_h+5))
    bpy.ops.object.camera_add(location=camloc)
    cam=bpy.context.active_object; sc.camera=cam; cam.data.lens=28
    d=(look-camloc).normalized()
    cam.rotation_euler = d.to_track_quat('-Z','Y').to_euler()

    # ambient self-lit world (dim teal)
    w=sc.world if sc.world else bpy.data.worlds.new("SG"); sc.world=w; w.use_nodes=True
    try:
        w.node_tree.nodes['Background'].inputs[0].default_value=(0.03,0.04,0.06,1.0)
    except Exception: pass

    sc.render.engine='BLENDER_EEVEE'
    sc.render.resolution_x=1400; sc.render.resolution_y=960
    sc.render.resolution_percentage=100
    sc.render.image_settings.file_format='PNG'
    out_png=os.path.join(OUT, target.replace('.fbx','_inside.png'))
    sc.render.filepath=out_png
    bpy.ops.render.render(write_still=True)
    print("OK" if os.path.exists(out_png) else "FAIL", target)


for t in sys.argv[sys.argv.index("--")+1:]:
    try:
        render_one(t)
    except Exception as e:
        import traceback; traceback.print_exc()
        print("ERROR",t)