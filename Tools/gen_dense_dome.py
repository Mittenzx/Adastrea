import bpy
import os

# High-segment UV sphere for the starfield dome (equirect UVs)
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete()

bpy.ops.mesh.primitive_uv_sphere_add(segments=128, ring_count=64, radius=1.0, location=(0,0,0))
obj = bpy.context.active_object
obj.name = "StarDome_Dense"

# A UV sphere already has pan/lat equirect UVs baked in 0..1 - keep them.

out_dir = r"C:\Users\akuma\Adastrea\Assets\FBX\generated"
out = os.path.join(out_dir, "SM_StarDome_Dense.fbx")
bpy.ops.export_scene.fbx(
    filepath=out,
    use_selection=True,
    apply_scale_options='FBX_SCALE_UNITS',
    axis_forward='-Z', axis_up='Y',
    mesh_smooth_type='FACE'
)
print("SAVED_DOME", out)