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
    # normalize the whole mesh to a fixed scale (~220 units) so all ships render
    # at the size where the small-ship setup is proven to work
    diag=((vmax[0]-vmin[0])**2+(vmax[1]-vmin[1])**2+(vmax[2]-vmin[2])**2)**0.5
    target_size=220.0
    sfactor=target_size/max(diag,1.0)
    if sfactor != 1.0:
        # recenter to origin then scale (keep as one op via bmesh on each obj)
        import bmesh as _bm
        for o in objs:
            me=o.data
            b=_bm.new(); b.from_mesh(me)
            # translate verts about the mesh bbox center
            import mathutils
            bmesh_verts=[v for v in b.verts]
            for v in bmesh_verts: v.co-=mathutils.Vector(center)
            for v in bmesh_verts: v.co*=sfactor
            b.to_mesh(me); b.free(); me.update()
    # now recompute center/bounds post-normalize
    vmin=Vector((1e9,)*3); vmax=Vector((-1e9,)*3)
    for o in objs:
        for v in o.data.vertices:
            ww=o.matrix_world@v.co
            for i in range(3):
                vmin[i]=min(vmin[i],ww[i]); vmax[i]=max(vmax[i],ww[i])
    center=(vmin+vmax)*0.5
    diag=((vmax[0]-vmin[0])**2+(vmax[1]-vmin[1])**2+(vmax[2]-vmin[2])**2)**0.5
    dist=max(diag*1.35,80)
    # manual quaternion aim (Track-to constraint can fail in background mode,
    # leaving the camera at default orientation which misses high-offset ships)
    loc=center+Vector((dist,-dist*0.7,dist*0.45))
    bpy.ops.object.camera_add(location=loc)
    cam=bpy.context.active_object; sc.camera=cam
    d=(center-loc).normalized()
    cam.rotation_euler=d.to_track_quat('-Z','Y').to_euler()
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