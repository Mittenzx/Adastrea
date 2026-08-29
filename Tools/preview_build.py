"""Module-builder PREVIEW — assemble a ship from chosen modules and render it,
so you can see a build before committing. Reuses the generator's module builders
(build_*_part) + hardpoint positions.

Usage (runs inside Blender headless):
  blender -b --python Tools/preview_build.py -- <ship_base> <outname> \
      [engine=ion] [cargo=bulk_tank] [weapon=tri_laser] [sensor=dome] [reactor=fusion_ring]
e.g.:
  blender -b --python Tools/preview_build.py -- SM_Ship_Test small \
      engine=ion weapon=tri_laser reactor=fusion_ring sensor=dome

Outputs: Assets/FBX/generated/<outname>_Assembled.fbx (ready to place) and
         Assets/FBX/generated/previews/<outname>_preview.png
"""
import bpy, os, sys
import importlib.util

GEN = r"C:\Users\akuma\Adastrea\Assets\FBX\generated"
GENDIR = r"C:\Users\akuma\Adastrea\Tools\generate_adastrea_assets.py"

# ---- load the generator module (so we reuse build_*_part, sc, box, etc.) ----
spec = importlib.util.spec_from_file_location("gaa", GENDIR)
gaa = importlib.util.module_from_spec(spec)
# avoid running its __main__ block
sys.modules['gaa'] = gaa
spec.loader.exec_module(gaa)


def build_preview(ship_base, outname, sz, module_choices):
    """Assemble a ship from module choices (variants) + render + export.

    Strategy: each module builder finalizes/joins its own part (and its cleanup
    deletes sibling meshes), so we build+export each part to its own FBX, then
    re-import all part FBX + a fresh carcass and join them for the preview."""
    k = gaa.sc(sz)
    s = gaa.SIZE_CLASSES[sz]
    lx, ly, lz = s['carcass']; locz = s['z']

    # (1) build + export each chosen part FBX in isolation
    MOUNT = {
        'engine': (0, -ly*0.52, locz - 10*k),
        'cargo':  (0, -ly*0.05, locz - 20*k),
        'weapon': (0,  ly*0.34, locz - 25*k),
        'sensor': (0,  ly*0.18, locz + 70*k),
        'reactor':(0, -ly*0.35, locz + 40*k),
    }
    def pick(mtype, variant, twin=True, asym=False):
        if mtype == 'engine':   return gaa.build_engine_part(sz, f"{outname}_Engine", variant=variant)
        if mtype == 'cargo':    return gaa.build_cargo_part(sz, f"{outname}_Cargo", variant=variant)
        if mtype == 'weapon':   return gaa.build_weapon_part(sz, f"{outname}_Weapon", twin=twin, variant=variant)
        if mtype == 'sensor':   return gaa.build_sensor_part(sz, f"{outname}_Sensor", asym=asym, variant=variant)
        if mtype == 'reactor':  return gaa.build_reactor_part(sz, f"{outname}_Reactor", variant=variant)
        return None

    part_fbx = {}
    for mtype, variant in module_choices.items():
        part = pick(mtype, variant)
        if part is None:
            print("  unknown module type:", mtype); continue
        # pick() returns (obj, path) — the builder already exported its part FBX
        path = part[1] if isinstance(part, tuple) else None
        if path:
            part_fbx[mtype] = (os.path.basename(path), MOUNT[mtype])
        # clear scene for the next part
        gaa.clear_scene()

    # (2) fresh scene: build raw carcass + re-import each part FBX at mount pos
    gaa.clear_scene()
    objs = []
    hull = gaa.box("Hull", lx, ly, lz, loc=(0, 0, locz)); gaa.bevel(hull, 6, 2)
    nose = gaa.box("Nose", lx*0.5, ly*0.3, lz*0.55, loc=(0, ly*0.62, locz)); gaa.bevel(nose, 5, 2)
    spine = gaa.box("Spine", lx*0.28, ly*0.85, lz*0.26, loc=(0, -ly*0.05, locz+lz*0.6)); gaa.bevel(spine, 4, 2)
    objs += [hull, nose, spine]
    for mtype, (fname, loc) in part_fbx.items():
        fpath = os.path.join(GEN, fname)
        bpy.ops.import_scene.fbx(filepath=fpath)
        imported = [o for o in bpy.data.objects if o.type=='MESH'][-1]
        imported.location = loc
        objs.append(imported)

    # join into one mesh
    joined = gaa.join(objs, f"{outname}_AssembledGeo")
    gaa.apply_mods(joined)
    gaa.clean_mesh(joined)
    gaa.smart_uv(joined)
    joined.name = f"{outname}_Assembled"
    m = bpy.data.materials.new("M_Assembled"); m.use_nodes = True
    if not joined.data.materials: joined.data.materials.append(m)
    gaa.export_fbx(joined, f"{outname}_Assembled")

    # ---- render via the textured self-lit path (reuse render_ships logic) ----
    sc = bpy.context.scene
    # Cycles-friendly material: lit metallic grey (real lights shade it, so
    # shadows + reflections make module boundaries read; slight emission so it's
    # never pure black).
    mat = bpy.data.materials.new("PV"); mat.use_nodes = True
    try:
        bs = mat.node_tree.nodes['Principled BSDF']
        bs.inputs['Base Color'].default_value = (0.55, 0.58, 0.63, 1.0)
        bs.inputs['Roughness'].default_value = 0.35   # some sheen
        bs.inputs['Metallic'].default_value = 0.9
        try:
            bs.inputs['Emission Color'].default_value = (0.1, 0.11, 0.12, 1.0)
            bs.inputs['Emission Strength'].default_value = 0.4
        except Exception:
            pass
    except Exception:
        pass
    for o in [o for o in bpy.data.objects if o.type=='MESH']:
        o.data.materials.clear(); o.data.materials.append(mat)
    # normalize + frame
    from mathutils import Vector
    vmin = Vector((1e9,)*3); vmax = Vector((-1e9,)*3)
    for o in [o for o in bpy.data.objects if o.type=='MESH']:
        for v in o.data.vertices:
            ww = o.matrix_world @ v.co
            for i in range(3):
                vmin[i]=min(vmin[i],ww[i]); vmax[i]=max(vmax[i],ww[i])
    center=(vmin+vmax)*0.5; diag=(vmax-vmin).length
    target=320.0; sf=target/max(diag,1e-6)
    import bmesh as _bm
    for o in [o for o in bpy.data.objects if o.type=='MESH']:
        me=o.data; b=_bm.new(); b.from_mesh(me)
        for v in b.verts: v.co-=Vector(center); v.co*=sf
        b.to_mesh(me); b.free(); me.update()
    vmin = Vector((1e9,)*3); vmax = Vector((-1e9,)*3)
    for o in [o for o in bpy.data.objects if o.type=='MESH']:
        for v in o.data.vertices:
            ww=o.matrix_world@v.co
            for i in range(3):
                vmin[i]=min(vmin[i],ww[i]); vmax[i]=max(vmax[i],ww[i])
    center=(vmin+vmax)*0.5; diag=(vmax-vmin).length
    dist=max(diag*1.2,100)
    loc=center+Vector((dist,-dist*0.6,dist*0.42))
    bpy.ops.object.camera_add(location=loc)
    cam=bpy.context.active_object; sc.camera=cam; cam.data.lens=35
    d=(center-loc).normalized(); cam.rotation_euler=d.to_track_quat('-Z','Y').to_euler()
    w=sc.world if sc.world else bpy.data.worlds.new("P"); sc.world=w; w.use_nodes=True
    try: w.node_tree.nodes['Background'].inputs[0].default_value=(0.02,0.025,0.04,1.0)
    except Exception: pass
    sc.render.engine = 'CYCLES'
    try:
        sc.cycles.samples = 96
    except Exception:
        pass
    # filmic tone mapping so highlights/shadows look believable
    try:
        sc.view_settings.view_transform = 'Filmic'
    except Exception:
        pass
    # lights so Cycles gives real shading that reads module shapes
    from mathutils import Vector
    cen = Vector((0.0, 0.0, 0.0))
    bpy.ops.object.light_add(type='AREA', location=cen + Vector((300, 300, 400)))
    kl = bpy.context.active_object; kl.data.size = 300; kl.data.energy = 1500
    bpy.ops.object.light_add(type='AREA', location=cen + Vector((-300, -200, 150)))
    fl = bpy.context.active_object; fl.data.size = 300; fl.data.energy = 500; fl.data.color = (0.8, 0.85, 1.0)
    sc.render.resolution_x = 1400; sc.render.resolution_y = 960
    sc.render.resolution_percentage=100
    sc.render.image_settings.file_format='PNG'
    out_png=os.path.join(GEN,"previews",f"{outname}_preview.png")
    sc.render.filepath=out_png
    bpy.ops.render.render(write_still=True)
    print("PREVIEW", outname, "ok" if os.path.exists(out_png) else "FAIL", "->", os.path.basename(out_png))


if __name__ == "__main__":
    argv = sys.argv
    idx = argv.index("--")+1 if "--" in argv else 0
    args = argv[idx:]
    if len(args) < 2:
        print("usage: -- <ship_base> <size_class> [module=variant ...]")
        sys.exit(0)
    ship_base, sz = args[0], args[1]
    outname = f"{ship_base}_PV"
    choices = {}
    for a in args[2:]:
        if '=' in a:
            k, v = a.split('=', 1)
            choices[k] = v
    build_preview(ship_base, outname, sz, choices)