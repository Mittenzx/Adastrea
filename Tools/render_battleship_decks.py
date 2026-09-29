"""Preview renders for the Battleship full-deck interior (Tools/build_battleship_decks.py).

Imports the EXPORTED FBX (what Unreal receives), colours slots by name, hides the
walk-collision mesh, puts a point light on every light socket from the contract,
and renders eye-height views of each zone plus a clipped top-down plan.

    blender -b --python Tools/render_battleship_decks.py -- [view ...] [--plan-only]

Output: Assets/FBX/generated/interior_inside/SM_Int_Battleship_Decks_*.png
"""
import bpy, os, sys, json, math
from mathutils import Vector

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import render_capital_interiors as rci  # noqa: E402

PREFIX = "SM_Int_Battleship_Decks"
rci.COL.update({
    "M_Int_Shell": (0.36, 0.38, 0.40), "M_Int_Deck": (0.22, 0.23, 0.25),
    "M_Int_Grate": (0.30, 0.30, 0.29), "M_Int_Hazard": (0.85, 0.62, 0.05),
    "M_Int_Bulkhead": (0.30, 0.33, 0.24), "M_Int_Hatch": (0.12, 0.12, 0.13),
    "M_Int_Stations": (0.26, 0.28, 0.27), "M_Int_Vents": (0.42, 0.22, 0.16),
    "M_Int_Mess": (0.50, 0.50, 0.48), "M_Int_Galley": (0.62, 0.63, 0.62),
    "M_Int_Bunks": (0.34, 0.36, 0.26), "M_Interior_Eng": (0.33, 0.34, 0.33),
    "M_Int_Console": (0.12, 0.40, 0.30),
})
EMISSIVE = {
    "M_Int_Lights": ((0.85, 0.92, 1.0), 6.0), "M_Int_LightsRed": ((1.0, 0.08, 0.04), 8.0),
    "M_Int_LightsAmber": ((1.0, 0.55, 0.12), 8.0), "M_Int_LightsGreen": ((0.25, 1.0, 0.35), 3.0),
    "M_Int_LightsBlue": ((0.3, 0.55, 1.0), 6.0), "M_Int_Console": ((0.2, 0.8, 0.55), 1.2),
}
LIGHT_COL = {"W": (0.82, 0.9, 1.0), "A": (1.0, 0.62, 0.25), "R": (1.0, 0.12, 0.06),
             "G": (0.35, 1.0, 0.45), "B": (0.4, 0.6, 1.0)}

_orig_make_mat = rci.make_mat


def make_mat(name):
    if name in EMISSIVE:
        col, strength = EMISSIVE[name]
        m = bpy.data.materials.new("PV_" + name)
        m.use_nodes = True
        nt = m.node_tree; nt.nodes.clear()
        out = nt.nodes.new("ShaderNodeOutputMaterial")
        em = nt.nodes.new("ShaderNodeEmission")
        em.inputs["Color"].default_value = (*col, 1); em.inputs["Strength"].default_value = strength
        nt.links.new(em.outputs[0], out.inputs[0])
        return m
    return _orig_make_mat(name)


rci.make_mat = make_mat

VIEWS = [
    ("spine_fwd", (-1850, 0, 165), (3600, 0, 150), 16),
    ("spine_aft", (3450, 60, 165), (-2000, -30, 150), 16),
    ("cic_spawn", (5020, 0, 165), (3700, 0, 120), 16),
    ("cic_fwd", (3850, -650, 210), (5800, 200, 260), 16),
    ("hangar_catwalk", (-2180, -1350, 770), (-3600, 700, 250), 14),
    ("hangar_floor", (-2500, -1500, 165), (-3500, 900, 450), 14),
    ("eng_floor", (-4550, -950, 165), (-5400, 400, 650), 14),
    ("eng_gallery", (-5850, 1000, 670), (-4600, -600, 400), 14),
    ("berths", (-1900, 480, 165), (-100, 1100, 140), 16),
    ("ready_room", (-150, -450, 165), (-1900, -1250, 150), 16),
    ("mess", (1700, 300, 165), (150, 1150, 110), 16),
    ("medbay", (1700, -300, 170), (800, -1100, 90), 16),
    ("briefing", (1900, 1300, 180), (3600, 650, 200), 16),
    ("armory", (1880, -280, 170), (3500, -1150, 140), 16),
]


def run(names, plan=True):
    objs = rci.setup(PREFIX)
    sc = bpy.context.scene
    sc.render.resolution_x, sc.render.resolution_y = 1400, 860
    try:
        sc.eevee.taa_render_samples = 24
    except Exception:
        pass
    sc.world.node_tree.nodes["Background"].inputs[1].default_value = 0.4
    for o in objs:
        if "Collision" in o.name:
            o.hide_render = True
    info = json.load(open(os.path.join(rci.GEN, PREFIX + "_contract.json")))
    for name, (x, y, z, _yaw) in info["sockets_ue_design_cm"].items():
        if not name.startswith("L"):
            continue
        tag, col, radius = name.split("_")[:3]
        bpy.ops.object.light_add(type='POINT', location=(x, -y, z))   # back to Blender y
        L = bpy.context.active_object
        r = float(radius) * 100.0
        L.data.energy = 2.2 * 4 * math.pi * (r * 0.5) ** 2
        L.data.color = LIGHT_COL.get(col, (1, 1, 1))
        L.data.shadow_soft_size = 20.0
        L.data.use_shadow = True
        try:
            L.data.use_custom_distance = True
            L.data.cutoff_distance = r * 1.3
        except Exception:
            pass
    for tag, loc, look, lens in VIEWS:
        if names and tag not in names:
            continue
        c = rci.cam(loc, look, lens=lens)
        rci.render(os.path.join(rci.OUT, f"{PREFIX}_view_{tag}.png"))
        bpy.data.objects.remove(c)
    if plan:
        ent = info["sockets_ue_design_cm"]["Entry"]; seat = info["sockets_ue_design_cm"]["Seat"]
        rci.marker_box("TRIGGER", (seat[0], -seat[1], 395.0), (100, 150, 1.0), (1.0, 0.1, 0.05))
        rci.marker_box("ENTRY", (ent[0], -ent[1], 395.0), (40, 40, 1.0), (0.1, 1.0, 0.2))
        mn = Vector(info["design_bounds_cm"]["min"]); mx = Vector(info["design_bounds_cm"]["max"])
        sc.render.resolution_x, sc.render.resolution_y = 2400, 760
        aspect = sc.render.resolution_x / sc.render.resolution_y
        span = max(mx.x - mn.x, (mx.y - mn.y) * aspect) * 1.03
        top = 3000.0
        rci.cam(((mn.x + mx.x) / 2, (mn.y + mx.y) / 2, top), ((mn.x + mx.x) / 2, (mn.y + mx.y) / 2, 0),
                ortho=span, clip_start=top - 400)
        rci.render(os.path.join(rci.OUT, f"{PREFIX}_plan.png"))


if __name__ == "__main__":
    args = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    names = [a for a in args if not a.startswith("--")]
    run(names, plan="--no-plan" not in args)
