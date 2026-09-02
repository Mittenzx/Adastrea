"""Render a hand-authored FBX (Assets/FBX/hand_author/) as a clear studio shot.
Converts the throwaway framing debugger into a reusable tool.
Usage: blender -b --python Tools/hand_render.py -- <ShipName|all>"""
import bpy, os, sys, math
import numpy as np
import mathutils

HAND = r"C:\Users\akuma\Adastrea\Assets\FBX\hand_author"
OBJ = r"C:\Users\akuma\Adastrea\Assets\FBX\generated\silhouettes"
os.makedirs(OBJ, exist_ok=True)

def render_one(name):
    src = os.path.join(HAND, name + ".fbx")
    if not os.path.exists(src):
        print("MISSING", src); return
    bpy.ops.wm.read_factory_settings(use_empty=True)
    for o in list(bpy.data.objects):
        if o.type != 'CAMERA':
            bpy.data.objects.remove(o)
    bpy.ops.import_scene.fbx(filepath=src)
    if "LightMat" not in bpy.data.materials:
        lm = bpy.data.materials.new("LightMat"); lm.use_nodes = True
        lm.node_tree.nodes["Principled BSDF"].inputs["Base Color"].default_value = (0.78, 0.80, 0.82, 1.0)
        lm.node_tree.nodes["Principled BSDF"].inputs["Roughness"].default_value = 0.6
        lm.node_tree.nodes["Principled BSDF"].inputs["Metallic"].default_value = 0.2
    for o in bpy.data.objects:
        if o.type == 'MESH':
            if not o.data.materials: o.data.materials.append(lm)
            else: o.data.materials[0] = lm
    vs = []
    for o in bpy.data.objects:
        if o.type == 'MESH':
            for v in o.data.vertices:
                vs.append(o.matrix_world @ v.co)
    vs = np.array(vs)
    cen = (vs.min(0) + vs.max(0)) / 2
    diag = np.linalg.norm(vs.max(0) - vs.min(0))
    cd = bpy.data.cameras.new("Cam"); cd.lens = 50.0
    cam = bpy.data.objects.new("HandCam", cd)
    bpy.context.collection.objects.link(cam)
    sc = bpy.context.scene; sc.camera = cam
    rad = max(diag * 1.8, 1.0)
    cam.location = (cen[0] + rad*0.5, cen[1] - rad*1.0, cen[2] + rad*0.6)
    direction = mathutils.Vector(cen) - cam.location
    cam.rotation_euler = direction.to_track_quat('-Z', 'Y').to_euler()
    world = sc.world or bpy.data.worlds.new("W"); sc.world = world; world.use_nodes = True
    if "Background" in world.node_tree.nodes:
        world.node_tree.nodes["Background"].inputs[0].default_value = (0.25, 0.27, 0.30, 1.0)
        world.node_tree.nodes["Background"].inputs[1].default_value = 1.0
    if not bpy.data.lights:
        ld = bpy.data.lights.new("Key", type='SUN')
        ln = bpy.data.objects.new("Key", ld); bpy.context.collection.objects.link(ln)
        ln.rotation_euler = (math.radians(50), 0, math.radians(30))
    sc.render.image_settings.file_format = 'PNG'
    sc.render.resolution_x = 1200; sc.render.resolution_y = 900
    sc.render.filepath = os.path.join(OBJ, name + "_hand.png")
    bpy.ops.render.render(write_still=True)
    print("RENDERED", name)

if __name__ == "__main__":
    which = sys.argv[sys.argv.index("--")+1] if "--" in sys.argv else "all"
    if which == "all":
        for f in sorted(os.listdir(HAND)):
            if f.endswith(".fbx"):
                render_one(f[:-4])
    else:
        render_one(which)