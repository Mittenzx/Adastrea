"""Render exterior ships with WORKBENCH (reliable, no EEVEE backface-culling
issues) — produces bright, readable ship silhouettes against dark space."""
import bpy, os, sys
from mathutils import Vector
GEN=r"C:\Users\akuma\Adastrea\Assets\FBX\generated"
OUT=os.path.join(GEN,"blender_previews"); os.makedirs(OUT,exist_ok=True)
def render1(target):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=os.path.join(GEN,target))
    objs=[o for o in bpy.data.objects if o.type=='MESH']
    sc=bpy.context.scene
    mat=bpy.data.materials.new("Bright"); mat.use_nodes=True
    bs=mat.node_tree.nodes['Principled BSDF']; bs.inputs['Base Color'].default_value=(0.65,0.7,0.78,1.0)
    for o in objs:
        if not o.data.materials: o.data.materials.append(mat)
    w=sc.world if sc.world else bpy.data.worlds.new("W"); sc.world=w; w.use_nodes=True
    try: w.node_tree.nodes['Background'].inputs[0].default_value=(0.004,0.005,0.01,1.0)
    except Exception: pass
    vmin=Vector((1e9,)*3); vmax=Vector((-1e9,)*3)
    for o in objs:
        for v in o.data.vertices:
            ww=o.matrix_world@v.co
            for i in range(3):
                vmin[i]=min(vmin[i],ww[i]); vmax[i]=max(vmax[i],ww[i])
    center=(vmin+vmax)*0.5
    diag=((vmax[0]-vmin[0])**2+(vmax[1]-vmin[1])**2+(vmax[2]-vmin[2])**2)**0.5
    dist=max(diag*1.1,80)
    bpy.ops.object.camera_add(location=center+Vector((dist,-dist*0.7,dist*0.45)))
    cam=bpy.context.active_object; sc.camera=cam
    tgt=bpy.data.objects.new("T",None); tgt.location=center; sc.collection.objects.link(tgt)
    t=cam.constraints.new('TRACK_TO'); t.target=tgt; t.track_axis='TRACK_NEGATIVE_Z'; t.up_axis='UP_Y'
    cam.data.lens=40
    bpy.ops.object.light_add(type='AREA', location=center+Vector((dist*0.5,dist*0.5,dist*0.7)))
    l=bpy.context.active_object; l.data.size=dist; l.data.energy=10000
    sc.render.engine='BLENDER_WORKBENCH'
    sc.render.resolution_x=1200; sc.render.resolution_y=825; sc.render.resolution_percentage=100
    sc.render.image_settings.file_format='PNG'
    sc.render.filepath=os.path.join(OUT,target.replace('.fbx','.png'))
    bpy.ops.render.render(write_still=True)
    print("OK", target)
for t in sys.argv[sys.argv.index("--")+1:]:
    render1(t)