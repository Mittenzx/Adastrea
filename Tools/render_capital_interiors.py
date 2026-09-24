"""Preview renders for the capital-ship bridge kits built by
Tools/build_capital_interiors.py. Imports the EXPORTED FBX (so it checks what
Unreal will actually receive), colours each material slot by name, puts the
real starfield behind the windows, and renders:
  * <prefix>_plan.png   top-down ortho (ceiling clipped) with the C++ exit
                        trigger box (red) and entry spawn (green) overlaid
  * <prefix>_view_*.png eye-height (165 cm) perspective views

    blender -b --python Tools/render_capital_interiors.py -- SM_Int_Battleship_Bridge [SM_Int_CommandXL_Bridge]

Output: Assets/FBX/generated/interior_inside/
"""
import bpy, os, sys, glob, json, math
from mathutils import Vector

GEN = r"C:\Users\akuma\Adastrea\Assets\FBX\generated"
OUT = os.path.join(GEN, "interior_inside")
STARS = os.path.join(GEN, "T_Starfield.png")
os.makedirs(OUT, exist_ok=True)

COL = {
    "M_Int_Shell": (0.42, 0.44, 0.47), "M_Int_Lights": (0.50, 0.75, 1.00),
    "M_Int_Bunks": (0.80, 0.66, 0.45), "M_Int_Vents": (0.70, 0.25, 0.20),
    "M_Int_Hatch": (0.18, 0.18, 0.20), "M_Int_Deck": (0.30, 0.31, 0.34),
    "M_Int_Console": (0.10, 0.32, 0.36), "M_Int_Stations": (0.22, 0.55, 0.55),
    "M_Int_Viewport": (0.08, 0.10, 0.14),
}


def base_name(n):
    return n.split(".")[0]


def make_mat(name):
    col = COL.get(name, (0.5, 0.5, 0.5))
    m = bpy.data.materials.new("PV_" + name)
    m.use_nodes = True
    nt = m.node_tree; nt.nodes.clear()
    out = nt.nodes.new("ShaderNodeOutputMaterial")
    if name == "M_Int_Lights":
        em = nt.nodes.new("ShaderNodeEmission")
        em.inputs["Color"].default_value = (*col, 1); em.inputs["Strength"].default_value = 2.2
        nt.links.new(em.outputs[0], out.inputs[0])
    elif name == "M_Int_Viewport":
        tr = nt.nodes.new("ShaderNodeBsdfTransparent")
        gl = nt.nodes.new("ShaderNodeBsdfGlossy"); gl.inputs["Roughness"].default_value = 0.05
        mix = nt.nodes.new("ShaderNodeMixShader"); mix.inputs[0].default_value = 0.12
        nt.links.new(tr.outputs[0], mix.inputs[1]); nt.links.new(gl.outputs[0], mix.inputs[2])
        nt.links.new(mix.outputs[0], out.inputs[0])
    else:
        b = nt.nodes.new("ShaderNodeBsdfPrincipled")
        b.inputs["Base Color"].default_value = (*col, 1)
        b.inputs["Roughness"].default_value = 0.55
        b.inputs["Metallic"].default_value = 0.35 if name in ("M_Int_Shell", "M_Int_Hatch") else 0.1
        # weak self-light so unlit corners never go pure black (headless EEVEE)
        em = nt.nodes.new("ShaderNodeEmission")
        em.inputs["Color"].default_value = (*col, 1); em.inputs["Strength"].default_value = 0.12
        add = nt.nodes.new("ShaderNodeAddShader")
        nt.links.new(b.outputs[0], add.inputs[0]); nt.links.new(em.outputs[0], add.inputs[1])
        nt.links.new(add.outputs[0], out.inputs[0])
    return m


def setup(prefix):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    files = sorted(glob.glob(os.path.join(GEN, prefix + "_*.fbx")))
    mats = {}
    objs = []
    for fp in files:
        before = set(bpy.data.objects)
        bpy.ops.import_scene.fbx(filepath=fp)
        for o in set(bpy.data.objects) - before:
            if o.type != 'MESH':
                continue
            for i, slot in enumerate(o.material_slots):
                key = base_name(slot.material.name) if slot.material else "M_Int_Shell"
                if key not in mats:
                    mats[key] = make_mat(key)
                o.material_slots[i].material = mats[key]
            objs.append(o)
    sc = bpy.context.scene
    sc.render.engine = 'BLENDER_EEVEE'
    sc.render.resolution_x, sc.render.resolution_y = 1600, 1000
    sc.view_settings.view_transform = 'AgX'
    sc.view_settings.look = 'AgX - Punchy'
    sc.view_settings.exposure = 0.3
    w = bpy.data.worlds.new("Stars"); sc.world = w; w.use_nodes = True
    nt = w.node_tree
    bg = nt.nodes["Background"]
    env = nt.nodes.new("ShaderNodeTexEnvironment")
    if os.path.exists(STARS):
        env.image = bpy.data.images.load(STARS)
        nt.links.new(env.outputs[0], bg.inputs[0])
    bg.inputs[1].default_value = 1.5
    return objs


def bounds(objs):
    pts = [o.matrix_world @ v.co for o in objs for v in o.data.vertices]
    mn = Vector([min(p[i] for p in pts) for i in range(3)])
    mx = Vector([max(p[i] for p in pts) for i in range(3)])
    return mn, mx


def add_lights(mn, mx, h):
    # a grid of soft area lights under the ceiling approximates the kit's
    # emissive strips + the runtime fill/fixture lights
    nx = 4; ny = 2
    for i in range(nx):
        for j in range(ny):
            x = mn.x + (mx.x - mn.x) * (i + 0.5) / nx
            y = mn.y + (mx.y - mn.y) * (j + 0.5) / ny
            bpy.ops.object.light_add(type='AREA', location=(x, y, h - 30))
            L = bpy.context.active_object
            L.data.energy = 90000; L.data.size = 180; L.data.color = (0.85, 0.92, 1.0)


def marker_box(name, c, ext, color):
    bpy.ops.mesh.primitive_cube_add(size=2.0, location=c)
    o = bpy.context.active_object; o.name = name
    o.scale = ext
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    nt = m.node_tree; nt.nodes.clear()
    out = nt.nodes.new("ShaderNodeOutputMaterial")
    em = nt.nodes.new("ShaderNodeEmission"); em.inputs["Color"].default_value = (*color, 1)
    em.inputs["Strength"].default_value = 3.0
    tr = nt.nodes.new("ShaderNodeBsdfTransparent")
    mix = nt.nodes.new("ShaderNodeMixShader"); mix.inputs[0].default_value = 0.35
    nt.links.new(tr.outputs[0], mix.inputs[1]); nt.links.new(em.outputs[0], mix.inputs[2])
    nt.links.new(mix.outputs[0], out.inputs[0])
    o.data.materials.append(m)
    return o


def cam(loc, look, lens=18, ortho=None, clip_start=5.0):
    bpy.ops.object.camera_add(location=loc)
    c = bpy.context.active_object
    d = (Vector(look) - Vector(loc)).normalized()
    c.rotation_euler = d.to_track_quat('-Z', 'Y').to_euler()
    c.data.lens = lens
    c.data.clip_start = clip_start; c.data.clip_end = 1e6
    if ortho:
        c.data.type = 'ORTHO'; c.data.ortho_scale = ortho
    bpy.context.scene.camera = c
    return c


def render(path):
    bpy.context.scene.render.filepath = path
    bpy.ops.render.render(write_still=True)
    print("RENDER", path, os.path.exists(path))


def run(prefix, views):
    objs = setup(prefix)
    mn, mx = bounds([o for o in objs if o.name.startswith(prefix + "_Shell")])
    h = mx.z
    add_lights(mn, mx, h)
    contract = os.path.join(GEN, prefix + "_contract.json")
    info = json.load(open(contract)) if os.path.exists(contract) else None
    # --- eye-height views first (no markers in them) ---
    for tag, loc, look, lens in views:
        c = cam(loc, look, lens=lens)
        render(os.path.join(OUT, f"{prefix}_view_{tag}.png"))
        bpy.data.objects.remove(c)
    # --- plan with trigger/entry overlay ---
    if info:
        tc = info["exit_trigger_center_design_cm"]; te = info["exit_trigger_extent_design_cm"]
        ec = info["entry_spawn_design_cm"]
        # footprints drawn as slabs at 238 cm (just under the plan clip plane);
        # the trigger's real Z span is reported in the contract JSON
        marker_box("TRIGGER", (tc[0], tc[1], 238.0), (te[0], te[1], 1.0), (1.0, 0.1, 0.05))
        marker_box("ENTRY", (ec[0], ec[1], 238.0), (30, 30, 1.0), (0.1, 1.0, 0.2))
    res = bpy.context.scene.render
    aspect = res.resolution_x / res.resolution_y
    span = max(mx.x - mn.x, (mx.y - mn.y) * aspect) * 1.05
    # clip everything above 245 cm (ceiling, soffits, beams, overhead lights)
    # so the plan shows the walkable floor, furniture and the markers
    cam(((mn.x + mx.x) / 2, (mn.y + mx.y) / 2, h + 200), ((mn.x + mx.x) / 2, (mn.y + mx.y) / 2, 0),
        ortho=span, clip_start=h + 200 - 245)
    render(os.path.join(OUT, f"{prefix}_plan.png"))


VIEWS = {
    "SM_Int_Battleship_Bridge": [
        ("entry_fwd", (-470, 0, 165), (300, 0, 140), 16),      # from the hatch down the deck
        ("cic", (-150, -150, 170), (40, 60, 110), 16),         # across the holotable
        ("bridge_fwd", (150, 150, 170), (560, -60, 150), 16),  # bridge + viewport
        ("helm_back", (330, -255, 175), (-400, 40, 115), 16),  # look aft from the bridge
    ],
    "SM_Int_CommandXL_Bridge": [
        ("entry_fwd", (-400, 0, 170), (400, 0, 170), 16),
        ("dais", (-220, -200, 180), (120, 60, 150), 16),
        ("crew_arc", (60, 160, 170), (470, -120, 160), 16),
        ("bow_back", (420, 220, 190), (-300, -120, 120), 14),
    ],
}

if __name__ == "__main__":
    args = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else list(VIEWS)
    for p in args:
        run(p, VIEWS.get(p, []))
