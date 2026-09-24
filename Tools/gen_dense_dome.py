"""Starfield dome mesh (SM_StarDome_Dense) -- low-poly retopo, v2 (2026-09-23).

v1 was a 128x64 UV sphere (16,128 tris) on the theory that more segments
meant crisper stars. They don't: the stars live entirely in the 4096x2048
equirect texture (Tools/gen_starfield.py -> T_Starfield.png, Milky Way band
+ magnitude-skewed dim stars), and the dome is only a carrier for that
texture. What actually matters for crispness is texture resolution plus
sampling, not vertex count.

v2 is a 32x16 UV sphere (960 tris, inside the 1,000 default budget in
Tools/qa_assets.py). Why this is visually equivalent from the dome centre
(the camera always sits at the centre because the dome follows the camera):
  * Each segment spans 11.25 deg. For a flat chord seen from the sphere
    centre, the worst angular error of linear UV interpolation against the
    true equirect angle is about 0.007 deg (atan(0.5*tan(a)) vs 0.5*a for a
    half-angle a of 5.625 deg), far below one texel (360/4096, about 0.088 deg).
  * The dome material is unlit/emissive, so the facet normals never show as
    shading. Normals are still smoothed in case a material reads them.

UV / asset contract (unchanged, so Unreal does a straight reimport):
  * Same Blender primitive (primitive_uv_sphere_add), so the UV layout and
    orientation are identical to v1: U wraps 0..1 around the equator with the
    seam in the same place, V runs 0..1 pole to pole. Only the segment
    density changes.
  * Same object name (StarDome_Dense), radius 1.0 (100 UU after
    FBX_SCALE_UNITS; the level's dome actor supplies the big scale), same
    outward normals, no material slot (the dome component assigns
    M_Starfield_Real in Unreal), and the same export axes and scale options,
    written to the same path.
"""
import bpy
import os

SEGMENTS = 32   # around the equator (U)
RINGS = 16      # pole to pole (V); tris = SEGMENTS*(RINGS-2)*2 + 2*SEGMENTS = 960

bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete()

bpy.ops.mesh.primitive_uv_sphere_add(segments=SEGMENTS, ring_count=RINGS, radius=1.0,
                                     location=(0, 0, 0))
obj = bpy.context.active_object
obj.name = "StarDome_Dense"
obj.data.name = "StarDome_Dense"
# Smooth normals (outward, same winding as v1). The unlit dome doesn't
# strictly need this, but at 32 segments hard facet normals could show if a
# material ever uses them (fresnel, etc.).
for p in obj.data.polygons:
    p.use_smooth = True

# A UV sphere already has pan/lat equirect UVs baked in 0..1, so keep them.

out_dir = r"C:\Users\akuma\Adastrea\Assets\FBX\generated"
out = os.path.join(out_dir, "SM_StarDome_Dense.fbx")
bpy.ops.object.select_all(action='DESELECT')
obj.select_set(True)
bpy.context.view_layer.objects.active = obj
bpy.ops.export_scene.fbx(
    filepath=out,
    use_selection=True,
    apply_scale_options='FBX_SCALE_UNITS',
    axis_forward='-Z', axis_up='Y',
    mesh_smooth_type='FACE'
)
obj.data.calc_loop_triangles()
print("SAVED_DOME", out, "tris", len(obj.data.loop_triangles))
