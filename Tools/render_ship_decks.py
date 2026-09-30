"""Preview renders for the full-deck ship interiors (Tools/build_ship_decks.py).

Same approach as Tools/render_battleship_decks.py: import the EXPORTED FBX
(what Unreal receives), colour slots by name, hide the walk-collision mesh,
put a point light on every light socket from the contract, then render the
contract's eye-height views plus a top-down plan clipped under the ceilings
(Entry = green marker, helm seat trigger = red).

    blender -b --python Tools/render_ship_decks.py -- Ship [Ship ...] [--plan-only] [--no-plan]

Output: Assets/FBX/generated/interior_inside/SM_Int_<Ship>_Decks_*.png
"""
import bpy, os, sys, json, math
from mathutils import Vector

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import render_capital_interiors as rci  # noqa: E402
from prop_materials import PROP_MATS, SCREEN_MATS  # noqa: E402

rci.COL.update({
    "M_Int_Shell": (0.36, 0.38, 0.40), "M_Int_Deck": (0.22, 0.23, 0.25),
    "M_Int_Grate": (0.30, 0.30, 0.29), "M_Int_Hazard": (0.85, 0.62, 0.05),
    "M_Int_Bulkhead": (0.30, 0.33, 0.24), "M_Int_Hatch": (0.12, 0.12, 0.13),
    "M_Int_Stations": (0.26, 0.28, 0.27), "M_Int_Vents": (0.42, 0.22, 0.16),
    "M_Int_Mess": (0.50, 0.50, 0.48), "M_Int_Galley": (0.62, 0.63, 0.62),
    "M_Int_Bunks": (0.34, 0.36, 0.26), "M_Interior_Eng": (0.33, 0.34, 0.33),
    "M_Int_Console": (0.12, 0.40, 0.30),
    "M_Int_Clean": (0.78, 0.80, 0.82), "M_Int_Steel": (0.40, 0.42, 0.45), "M_Int_Dark": (0.07, 0.075, 0.09),
    "M_Int_Paint": (0.80, 0.34, 0.06), "M_Int_AccentBlue": (0.08, 0.20, 0.55), "M_Int_Carpet": (0.36, 0.06, 0.08),
    "M_Int_Wood": (0.40, 0.22, 0.11), "M_Int_Brass": (0.75, 0.55, 0.22), "M_Int_Plant": (0.10, 0.32, 0.08),
})
rci.COL.update({k: v[0] for k, v in PROP_MATS.items()})
EMISSIVE = {
    "M_Prop_Live": ((0.35, 0.75, 1.0), 2.5), "M_Prop_Term": ((0.25, 1.0, 0.45), 2.0),
    "M_Int_Lights": ((0.85, 0.92, 1.0), 6.0), "M_Int_LightsRed": ((1.0, 0.08, 0.04), 8.0),
    "M_Int_LightsAmber": ((1.0, 0.55, 0.12), 8.0), "M_Int_LightsGreen": ((0.25, 1.0, 0.35), 3.0),
    "M_Int_LightsBlue": ((0.3, 0.55, 1.0), 6.0), "M_Int_Console": ((0.2, 0.8, 0.55), 1.2),
    "M_Int_LightsCyan": ((0.2, 0.85, 1.0), 5.0), "M_Int_LightsWarm": ((1.0, 0.72, 0.45), 6.0),
}
LIGHT_COL = {"W": (0.82, 0.9, 1.0), "A": (1.0, 0.62, 0.25), "R": (1.0, 0.12, 0.06),
             "G": (0.35, 1.0, 0.45), "B": (0.4, 0.6, 1.0), "C": (0.3, 0.9, 1.0), "P": (1.0, 0.78, 0.55)}

_orig_make_mat = rci.make_mat


def make_mat(name):
    if name in EMISSIVE:
        c, strength = EMISSIVE[name]
        m = bpy.data.materials.new("PV_" + name)
        m.use_nodes = True
        nt = m.node_tree; nt.nodes.clear()
        out = nt.nodes.new("ShaderNodeOutputMaterial")
        em = nt.nodes.new("ShaderNodeEmission")
        em.inputs["Color"].default_value = (*c, 1); em.inputs["Strength"].default_value = strength
        nt.links.new(em.outputs[0], out.inputs[0])
        return m
    return _orig_make_mat(name)


rci.make_mat = make_mat


def add_fixtures(info):
    """Fixtures are spawned at runtime from X_ sockets, so the deck FBX doesn't
    contain them: draw them here from deck_props at each socket."""
    import deck_kit as dk
    import deck_props as dp
    from mathutils import Matrix
    dk.reset("FxPreview", "military")
    n = 0
    for name, (x, y, z, yaw) in info["sockets_ue_design_cm"].items():
        bits = name.split("_")
        if bits[0] != "X" or bits[1] not in dp.FIXTURES:
            continue
        t = dp.PT("Fx", lights="Fx")
        t.M = dk.Mx(x, -y, z, -yaw)
        dp.FIXTURES[bits[1]][0](t)
        n += 1
    if n:
        ob = dk.K["Fx"].to_object()
        for i, slot in enumerate(ob.material_slots):
            nm = slot.material.name.split(".")[0] if slot.material else "M_Prop_Steel"
            ob.material_slots[i].material = make_mat(nm)
    print("fixtures drawn", n)


def run(ship, names, plan=True, views=True):
    prefix = f"SM_Int_{ship}_Decks"
    objs = rci.setup(prefix)
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
    info = json.load(open(os.path.join(rci.GEN, prefix + "_contract.json")))
    add_fixtures(info)
    for name, (x, y, z, _yaw) in info["sockets_ue_design_cm"].items():
        if not name.startswith("L"):
            continue
        tag, c, radius = name.split("_")[:3]
        bpy.ops.object.light_add(type='POINT', location=(x, -y, z))
        L = bpy.context.active_object
        r = float(radius) * 100.0
        L.data.energy = 2.2 * 4 * math.pi * (r * 0.5) ** 2
        L.data.color = LIGHT_COL.get(c, (1, 1, 1))
        L.data.shadow_soft_size = 20.0
        L.data.use_shadow = True
        try:
            L.data.use_custom_distance = True
            L.data.cutoff_distance = r * 1.3
        except Exception:
            pass
    if views:
        for tag, loc, look, lens in info["views"]:
            if names and tag not in names:
                continue
            c = rci.cam(loc, look, lens=lens)
            rci.render(os.path.join(rci.OUT, f"{prefix}_view_{tag}.png"))
            bpy.data.objects.remove(c)
    if plan:
        ent = info["sockets_ue_design_cm"]["Entry"]; seat = info["sockets_ue_design_cm"]["Seat"]
        pz = info["plan_z"]
        rci.marker_box("TRIGGER", (seat[0], -seat[1], pz - 5), (100, 150, 1.0), (1.0, 0.1, 0.05))
        rci.marker_box("ENTRY", (ent[0], -ent[1], pz - 5), (40, 40, 1.0), (0.1, 1.0, 0.2))
        mn = Vector(info["design_bounds_cm"]["min"]); mx = Vector(info["design_bounds_cm"]["max"])
        sc.render.resolution_x = 2400
        sc.render.resolution_y = max(400, int(2400 * (mx.y - mn.y) / (mx.x - mn.x) * 1.04))
        aspect = sc.render.resolution_x / sc.render.resolution_y
        span = max(mx.x - mn.x, (mx.y - mn.y) * aspect) * 1.03
        top = 5000.0
        rci.cam(((mn.x + mx.x) / 2, (mn.y + mx.y) / 2, top), ((mn.x + mx.x) / 2, (mn.y + mx.y) / 2, 0),
                ortho=span, clip_start=top - pz)
        rci.render(os.path.join(rci.OUT, f"{prefix}_plan.png"))


if __name__ == "__main__":
    args = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    ships = [a for a in args if not a.startswith("--") and a[0].isupper()]
    names = [a for a in args if not a.startswith("--") and not a[0].isupper()]
    for s in ships:
        run(s, names, plan="--no-plan" not in args, views="--plan-only" not in args)
