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

def render_fbx(fbx_path, out_png, engine='BLENDER_EEVEE'):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=fbx_path)
    sc = bpy.context.scene
    sc.unit_settings.system='METRIC'; sc.unit_settings.length_unit='CENTIMETERS'
    # simple PBR-ish material so it isn't flat (light grey, metallic-ish)
    mat = bpy.data.materials.new("Preview")
    mat.use_nodes=True
    for o in [o for o in bpy.data.objects if o.type=='MESH']:
        if not o.data.materials:
            o.data.materials.append(mat)
    # brighten the world / add ambient so nothing is pitch black
    if sc.world is None:
        sc.world = bpy.data.worlds.new("W")
    w = sc.world
    w.use_nodes = True
    # set world background to a dim mid-grey so dark shots read
    try:
        bg = w.node_tree.nodes.get("Background")
        if bg:
            bg.inputs[0].default_value = (0.28, 0.3, 0.34, 1.0)
            bg.inputs[1].default_value = 0.5
    except Exception:
        pass
    frame_mesh()
    sc.render.engine = engine
    sc.render.resolution_x=1200; sc.render.resolution_y=825; sc.render.resolution_percentage=100
    sc.render.image_settings.file_format='PNG'
    sc.render.filepath = out_png
    # world light
    if sc.world is None:
        sc.world = bpy.data.worlds.new("W")
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
