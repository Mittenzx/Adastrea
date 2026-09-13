"""Import each FBX and re-export as ASCII OBJ for numeric QA."""
import bpy, os, glob

GENDIR = r"C:\Users\akuma\Adastrea\Assets\FBX\generated"
OBJDIR = os.path.join(GENDIR, "obj")
os.makedirs(OBJDIR, exist_ok=True)

for fbx in sorted(glob.glob(os.path.join(GENDIR, "*.fbx")) +
                   glob.glob(os.path.join(GENDIR, "kitbash", "*.fbx"))):
    base = os.path.splitext(os.path.basename(fbx))[0]
    out = os.path.join(OBJDIR, base + ".obj")
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=fbx)
    bpy.ops.wm.obj_export(
        filepath=out,
        export_selected_objects=False,
        apply_modifiers=True,
        path_mode='STRIP',
        forward_axis='NEGATIVE_Y', up_axis='Z',
        export_uv=True,
        export_materials=False,
    )
    print(f"{base}.obj -> {os.path.getsize(out)} bytes")