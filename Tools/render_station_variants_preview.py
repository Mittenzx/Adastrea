"""One-off: render a quick orbiting-camera thumbnail of each new station hull
FBX for a visual sanity check before importing into UE. Not part of the
regular pipeline; safe to delete after use.
"""
import math
import os
import sys

import bpy

GEN = r"C:\Users\akuma\Adastrea\Assets\FBX\generated"
OUT_DIR = r"C:\Users\akuma\Adastrea\Assets\FBX\generated\_preview"
os.makedirs(OUT_DIR, exist_ok=True)

NAMES = ["SM_Station_BlackMarket_01"]


def render_one(name):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.render.engine = 'BLENDER_EEVEE'
    scene.render.resolution_x = 512
    scene.render.resolution_y = 512

    fbx = os.path.join(GEN, name + ".fbx")
    bpy.ops.import_scene.fbx(filepath=fbx)

    meshes = [o for o in bpy.context.scene.objects if o.type == 'MESH']
    if not meshes:
        print(f"NO_MESH: {name}")
        return

    # bounding box across all imported meshes
    min_c = [1e9, 1e9, 1e9]
    max_c = [-1e9, -1e9, -1e9]
    for o in meshes:
        for corner in o.bound_box:
            wc = o.matrix_world @ __import__('mathutils').Vector(corner)
            for i in range(3):
                min_c[i] = min(min_c[i], wc[i])
                max_c[i] = max(max_c[i], wc[i])
    center = [(min_c[i] + max_c[i]) / 2 for i in range(3)]
    radius = max(max_c[i] - min_c[i] for i in range(3)) or 100

    dist = radius * 1.8
    cam_loc = (center[0] + dist * 0.7, center[1] - dist * 0.9, center[2] + dist * 0.5)
    cam_data = bpy.data.cameras.new("Cam")
    cam_data.clip_start = 1.0
    cam_data.clip_end = 1000000.0
    cam = bpy.data.objects.new("Cam", cam_data)
    bpy.context.collection.objects.link(cam)
    bpy.context.scene.camera = cam
    cam.location = cam_loc
    direction = __import__('mathutils').Vector(center) - __import__('mathutils').Vector(cam_loc)
    cam.rotation_euler = direction.to_track_quat('-Z', 'Y').to_euler()

    sun_data = bpy.data.lights.new("Sun", type='SUN')
    sun_data.energy = 3.0
    sun = bpy.data.objects.new("Sun", sun_data)
    bpy.context.collection.objects.link(sun)
    sun.rotation_euler = (math.radians(55), 0, math.radians(35))

    world = bpy.data.worlds.new("World")
    world.use_nodes = True
    bpy.context.scene.world = world
    bg = world.node_tree.nodes.get("Background")
    if bg:
        bg.inputs[0].default_value = (0.02, 0.02, 0.03, 1.0)

    out_path = os.path.join(OUT_DIR, name + ".png")
    scene.render.filepath = out_path
    bpy.ops.render.render(write_still=True)
    print(f"RENDERED: {out_path}")


for n in NAMES:
    render_one(n)
