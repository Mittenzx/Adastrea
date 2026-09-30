"""Export the interior prop library (Tools/deck_props.py) as standalone meshes (Blender 5.x).

    blender -b --python Tools/build_interior_props.py -- [--sheet] [--no-export] [Name ...]

  * SM_Prop_<Name>.fbx for every CATALOG prop and SM_Prop_Fx_<Name>.fbx for every
    functional fixture, at 1:1 cm (unlike the 100x SM_Int_* kits), pivot at the
    prop's floor/wall mount point. Assets/FBX/generated/props/
  * props_contract.json: names, placement, bounds, slots.
  * --sheet: contact-sheet renders of the whole catalogue (props_sheet.png) and
    the fixtures (props_fixtures.png) with preview colours from prop_materials.py.
"""
import bpy, os, sys, json, math

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import deck_kit as dk  # noqa: E402
import deck_props as dp  # noqa: E402
import exterior_props as xp  # noqa: E402
from prop_materials import PROP_MATS, SCREEN_MATS, BEACON_MATS  # noqa: E402
from build_capital_interiors import GEN  # noqa: E402

OUT = os.path.join(GEN, "props")
EMISSIVE = {
    "M_Int_Lights": (0.85, 0.92, 1.0), "M_Int_LightsRed": (1.0, 0.08, 0.04), "M_Int_LightsAmber": (1.0, 0.55, 0.12),
    "M_Int_LightsGreen": (0.25, 1.0, 0.35), "M_Int_LightsBlue": (0.3, 0.55, 1.0), "M_Int_LightsCyan": (0.2, 0.85, 1.0),
    "M_Int_LightsWarm": (1.0, 0.72, 0.45),
}


def new_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    sc = bpy.context.scene
    sc.unit_settings.system = 'METRIC'
    sc.unit_settings.scale_length = 0.01          # 1 BU = 1 cm -> FBX comes out 1:1 in UE
    sc.unit_settings.length_unit = 'CENTIMETERS'
    return sc


def build_one(name, fn, prefix):
    dk.reset(f"{prefix}{name}", "military")
    part = f"P_{name}"
    fn(dp.PT(part, lights=part))
    ob = dk.K[part].to_object()
    ob.name = f"{prefix}{name}"
    ob.data.name = ob.name
    for poly in ob.data.polygons:
        poly.use_smooth = False
    dp.fix_screen_uvs(ob)
    for p in list(dk.K):
        if p != part:
            dk.K[p].bm.free()
    return ob


def export(ob):
    os.makedirs(OUT, exist_ok=True)
    path = os.path.join(OUT, ob.name + ".fbx")
    bpy.ops.object.select_all(action='DESELECT')
    ob.select_set(True)
    bpy.context.view_layer.objects.active = ob
    bpy.ops.export_scene.fbx(filepath=path, use_selection=True, object_types={'MESH'},
                             apply_scale_options='FBX_SCALE_ALL', apply_unit_scale=True,
                             axis_forward='-Y', axis_up='Z', mesh_smooth_type='FACE')
    return path


def preview_mat(name):
    m = bpy.data.materials.get("PV_" + name)
    if m:
        return m
    m = bpy.data.materials.new("PV_" + name)
    m.use_nodes = True
    nt = m.node_tree; nt.nodes.clear()
    out = nt.nodes.new("ShaderNodeOutputMaterial")
    if name in EMISSIVE or name in SCREEN_MATS or name in BEACON_MATS:
        c = EMISSIVE.get(name) or (SCREEN_MATS.get(name) or BEACON_MATS[name])[0]
        em = nt.nodes.new("ShaderNodeEmission")
        em.inputs["Color"].default_value = (*c, 1); em.inputs["Strength"].default_value = 3.0
        nt.links.new(em.outputs[0], out.inputs[0])
    elif name == "M_Int_Viewport":
        b = nt.nodes.new("ShaderNodeBsdfPrincipled")
        b.inputs["Base Color"].default_value = (0.6, 0.8, 0.9, 1)
        b.inputs["Roughness"].default_value = 0.05
        b.inputs["Alpha"].default_value = 0.35
        nt.links.new(b.outputs[0], out.inputs[0])
    else:
        c, r, mt = PROP_MATS.get(name, ((0.5, 0.5, 0.5), 0.5, 0.0))
        b = nt.nodes.new("ShaderNodeBsdfPrincipled")
        b.inputs["Base Color"].default_value = (*c, 1)
        b.inputs["Roughness"].default_value = r
        b.inputs["Metallic"].default_value = mt
        nt.links.new(b.outputs[0], out.inputs[0])
    return m


def sheet(items, path, cols, cell, title_scale=1.0):
    """Lay the objects out on a grid (they're already built) and render."""
    sc = bpy.context.scene
    for i, (name, ob, place) in enumerate(items):
        r, c = divmod(i, cols)
        ob.location = (c * cell, -r * cell, 0)
        if place == "wall":
            ob.rotation_euler = (0, 0, math.radians(-90))       # face the camera (-Y)
            ob.location.z = 60 if name not in ("FireExtinguisher", "GasRack", "WallTools") else 0
        elif place == "ceiling":
            ob.rotation_euler = (0, 0, math.radians(-90))
            ob.location.z = 130
        else:
            ob.rotation_euler = (0, 0, math.radians(-90))
            if place == "table":
                bpy.ops.mesh.primitive_cube_add(size=1, location=(c * cell, -r * cell, -10))
                t = bpy.context.active_object
                t.scale = (cell * 0.55, cell * 0.55, 20)
                t.data.materials.append(preview_mat("M_Prop_Steel"))
        for i2, slot in enumerate(ob.material_slots):
            if slot.material:
                ob.material_slots[i2].material = preview_mat(slot.material.name.split(".")[0])
        bpy.ops.object.text_add(location=(c * cell - cell * 0.4, -r * cell - cell * 0.42, 0.5))
        tx = bpy.context.active_object
        tx.data.body = name
        tx.data.size = cell * 0.075 * title_scale
        tx.data.materials.append(preview_mat("M_Prop_White"))
    rows = (len(items) + cols - 1) // cols
    bpy.ops.mesh.primitive_plane_add(size=1, location=((cols - 1) * cell / 2, -(rows - 1) * cell / 2, -20.5))
    fl = bpy.context.active_object
    fl.scale = (cols * cell + cell, rows * cell + cell, 1)
    fl.data.materials.append(preview_mat("M_Prop_Fabric"))
    sc.render.engine = 'BLENDER_EEVEE'
    sc.render.resolution_x = 2400
    sc.render.resolution_y = int(2400 * (rows * cell + cell * 0.3) / (cols * cell + cell * 0.3) * 0.72) + 200
    sc.view_settings.view_transform = 'AgX'
    w = bpy.data.worlds.new("W"); sc.world = w; w.use_nodes = True
    w.node_tree.nodes["Background"].inputs[0].default_value = (0.05, 0.055, 0.065, 1)
    w.node_tree.nodes["Background"].inputs[1].default_value = 1.2
    bpy.ops.object.light_add(type='SUN', rotation=(math.radians(50), math.radians(10), math.radians(-30)))
    bpy.context.active_object.data.energy = 3.5
    cx, cy = (cols - 1) * cell / 2, -(rows - 1) * cell / 2
    bpy.ops.object.camera_add(location=(cx, cy - rows * cell * 0.62, rows * cell * 0.62 + 150))
    cam = bpy.context.active_object
    cam.data.type = 'ORTHO'
    cam.data.ortho_scale = cols * cell + cell * 0.3
    cam.data.clip_end = 1e6
    from mathutils import Vector
    d = (Vector((cx, cy, 0)) - cam.location).normalized()
    cam.rotation_euler = d.to_track_quat('-Z', 'Y').to_euler()
    sc.camera = cam
    sc.render.filepath = path
    bpy.ops.render.render(write_still=True)
    print("RENDER", path, os.path.exists(path))


def main():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    names = [a for a in argv if not a.startswith("--")]
    do_export = "--no-export" not in argv
    contract = {"props": {}, "fixtures": {}, "exterior": {}}
    groups = (("props", dp.CATALOG, "SM_Prop_"), ("fixtures", dp.FIXTURES, "SM_Prop_Fx_"),
              ("exterior", xp.EXTERIOR, "SM_ExtProp_"))
    built = {}
    for key, table, prefix in groups:
        new_scene()
        items = []
        for name, (fn, place) in table.items():
            if names and name not in names:
                continue
            ob = build_one(name, fn, prefix)
            pts = [v.co for v in ob.data.vertices]
            bmin = [round(min(p[i] for p in pts), 1) for i in range(3)]
            bmax = [round(max(p[i] for p in pts), 1) for i in range(3)]
            ob.data.calc_loop_triangles()
            contract[key][name] = {"mesh": ob.name, "placement": place, "bounds_cm": [bmin, bmax],
                                   "tris": len(ob.data.loop_triangles),
                                   "slots": [m.name for m in ob.data.materials]}
            if do_export:
                export(ob)
            items.append((name, ob, place))
        built[key] = items
        if "--sheet" in argv and items:
            cols, cell = {"props": (9, 150.0), "fixtures": (5, 260.0), "exterior": (7, 480.0)}[key]
            sheet(items, os.path.join(OUT, "props_%s.png" % ("sheet" if key == "props" else key)), cols, cell)
    if do_export and not names:
        os.makedirs(OUT, exist_ok=True)
        with open(os.path.join(OUT, "props_contract.json"), "w") as f:
            json.dump(contract, f, indent=1)
    print("PROPS", len(contract["props"]), "FIXTURES", len(contract["fixtures"]), "EXTERIOR", len(contract["exterior"]),
          "tris", sum(v["tris"] for g in contract.values() for v in g.values()))


if __name__ == "__main__":
    main()
