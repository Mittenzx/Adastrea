"""Full-deck walkable interiors for every ship except the Battleship (Blender 5.x, headless).

    blender -b --python Tools/build_ship_decks.py -- [Ship ...] [--dry-run] [--list]

Each ship gets its own deck, laid out stern (-X) to bow (+X) as segments at a
scale that fits its class and crew, from the Viper's 11 m cockpit-and-cabin up
to the Sovereign's 100 m flagship deck:

    ("bay",    kind, L, W, H, opts)     full-width space (engineering, hold, hangar, ...)
    ("block",  L, W, H, opts)           spine corridor with rooms either side:
                                        opts port=[(kind, length, opts)], stbd=[...],
                                        spine=corridor width, hs=corridor ceiling
    ("bridge", kind, L, W, H, opts)     bow segment with chamfered window bands

Rooms and bays come from Tools/deck_rooms.py, primitives and styles from
Tools/deck_kit.py (the Battleship's kit, restyled per ship).

Contract with ASpaceshipInterior (generic full-deck path, see SpaceshipInterior.cpp):
  * Parts are SM_Int_<Ship>_Decks_<Part>, exported 100x like every SM_Int_* kit and
    mounted at a fixed 0.01 (1 design cm = 1 uu).
  * The Shell carries sockets (added by Tools/import_ship_decks.py from the
    contract): Entry, Seat, L_<C>_<RR>_<NNN> lights and P_<Part> for every
    other part, which is how the C++ knows what to mount.
  * "Collision" is walk collision only (complex-as-simple, never drawn).
  * Axes: Blender +X = UE +X (bow), UE y = -Blender y. Sockets stored flipped.

The build also runs a walkability check: it rasterises the collision mesh
at body height and flood-fills from Entry, and it reports any room whose floor
the avatar can't reach at deck level (a blocked door, say).
"""
import bpy, math, os, sys, json
from mathutils import Vector

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import deck_kit as dk  # noqa: E402
from deck_kit import (box, col, wall, window_band, seg_box, prism, extrude, pipe, cyl, fill,
                      ceiling_panel, cage_lamp, sconce, K, P, S, SOCKETS, VIEWS, WT, CL, PI, EN,
                      CRP)  # noqa: E402
import deck_rooms as dr  # noqa: E402
import deck_dressing as dd  # noqa: E402
from build_capital_interiors import GEN  # noqa: E402

BP = "/Game/Blueprints/Ships/"

# ----------------------------------------------------------------------------
# Ship specs. Lengths/heights in design cm.
# ----------------------------------------------------------------------------
SHIPS = {
    # --- Viper Interceptor (crew 1-2): cockpit, crew nook, avionics crawl bay ---
    "Fighter": dict(
        bp=BP + "BP_Ship_Fighter", style="command", door=(130, 205), main_door=(130, 205),
        segs=[("bay", "fighter_aft", 300, 300, 225, {}),
              ("bay", "fighter_cabin", 420, 380, 230, {"name": "Cabin"}),
              ("bridge", "cockpit", 400, 300, 220, {"nose": 0.55, "chamfer": 150, "sill": 70, "seats": 1})]),
    # --- Sentinel Patrol Ship (8-16): brig, armory, bunks, galley ---
    "Patrol": dict(
        bp=BP + "BP_Ship_Patrol", style="patrol", door=(140, 220), main_door=(180, 230),
        segs=[("bay", "engine", 520, 900, 420, {}),
              ("block", 1150, 1000, 300, dict(spine=220, hs=260, name="Crew",
                  port=[("bunks", 575, {}), ("galley", 575, {})],
                  stbd=[("brig", 700, {}), ("armory", 450, {})])),
              ("bridge", "small", 760, 820, 320, {"nose": 0.5, "chamfer": 260})]),
    # --- Merchant Trader (8-15, cargo 5000): big container hold ---
    "Trading": dict(
        bp=BP + "BP_Ship_Trading", style="industrial", door=(140, 220), main_door=(240, 250),
        segs=[("bay", "engine", 520, 1000, 420, {}),
              ("bay", "hold", 1500, 1400, 720, {}),
              ("block", 900, 1000, 300, dict(spine=220, hs=260, name="Crew",
                  port=[("bunks", 450, {}), ("galley", 450, {})],
                  stbd=[("airlock", 340, {}), ("workshop", 560, {})])),
              ("bridge", "small", 720, 760, 310, {"nose": 0.55, "chamfer": 220})]),
    # --- Shadowblade Stealth Frigate (12-18): ops room, ECM core ---
    "Frigate": dict(
        bp=BP + "BP_Ship_Frigate", style="stealth", door=(140, 220), main_door=(180, 230),
        segs=[("bay", "engine", 600, 900, 420, {}),
              ("block", 1350, 1000, 300, dict(spine=220, hs=250, name="Ops",
                  port=[("bunks", 500, {}), ("ecm", 850, {})],
                  stbd=[("armory", 450, {}), ("ops", 900, {})])),
              ("bridge", "small", 760, 820, 300, {"nose": 0.4, "chamfer": 300})]),
    # --- Phoenix Salvage Ship (10-20): salvage bay with claw crane ---
    "Utility": dict(
        bp=BP + "BP_Ship_Utility", style="industrial", door=(140, 220), main_door=(240, 250),
        segs=[("bay", "engine", 520, 1000, 420, {}),
              ("bay", "salvage", 1300, 1300, 680, {}),
              ("block", 850, 1000, 300, dict(spine=220, hs=260, name="Crew",
                  port=[("bunks", 425, {}), ("galley", 425, {})],
                  stbd=[("workshop", 850, {})])),
              ("bridge", "small", 720, 820, 320, {"nose": 0.55, "chamfer": 240})]),
    # --- Warhammer Gunship (15-25): magazine, fire control ---
    "Gunship": dict(
        bp=BP + "BP_Ship_Gunship", style="military", door=(160, 230), main_door=(200, 240),
        segs=[("bay", "engine", 600, 1000, 450, {}),
              ("block", 720, 1100, 330, dict(spine=240, hs=280, name="Weapons",
                  port=[("magazine", 720, {})], stbd=[("gunnery", 720, {})])),
              ("block", 760, 1100, 330, dict(spine=240, hs=280, name="Crew",
                  port=[("bunks", 760, {})], stbd=[("galley", 380, {}), ("armory", 380, {})])),
              ("bridge", "mid", 950, 960, 340, {"nose": 0.5, "chamfer": 280})]),
    # --- Raptor Assault Corvette (15-40): boarding bay with breaching pods ---
    "Corvette": dict(
        bp=BP + "BP_Ship_Corvette", style="military", door=(160, 230), main_door=(240, 250),
        segs=[("bay", "engine", 600, 1000, 450, {}),
              ("bay", "boarding", 1100, 1300, 600, {}),
              ("block", 1250, 1100, 330, dict(spine=240, hs=280, name="Crew",
                  port=[("ready", 625, {}), ("bunks", 625, {})],
                  stbd=[("armory", 600, {}), ("medbay", 650, {})])),
              ("bridge", "mid", 950, 960, 340, {"nose": 0.5, "chamfer": 280})]),
    # --- Destroyer (Warhammer-line): torpedo room, CIC ---
    "Destroyer": dict(
        bp=BP + "BP_Ship_Destroyer", style="military", door=(180, 240), main_door=(260, 260),
        segs=[("bay", "engine", 850, 1300, 700, {}),
              ("bay", "torpedo", 950, 1200, 480, {}),
              ("block", 1700, 1300, 350, dict(spine=260, hs=290, name="Crew",
                  port=[("bunks", 650, {}), ("mess", 1050, {})],
                  stbd=[("armory", 520, {}), ("medbay", 560, {}), ("briefing", 620, {})])),
              ("bridge", "cic", 1350, 1300, 450, {"nose": 0.45, "chamfer": 380})]),
    # --- Odyssey Research Vessel (20-45): labs, astrometrics ---
    "Science": dict(
        bp=BP + "BP_Ship_Science", style="clean", door=(160, 230), main_door=(240, 250),
        segs=[("bay", "engine", 720, 1200, 650, {"style": "industrial"}),
              ("block", 1450, 1400, 340, dict(spine=260, hs=300, name="Labs",
                  port=[("cabins", 750, {}), ("mess", 700, {})],
                  stbd=[("lab", 725, {}), ("lab", 725, {"bio": True})])),
              ("bay", "astro", 950, 1400, 560, {"win": "both", "name": "Astrometrics"}),
              ("bridge", "mid", 950, 1000, 340, {"nose": 0.55, "chamfer": 300})]),
    # --- Behemoth Heavy Freighter, container ship ---
    "Freighter": dict(
        bp=BP + "BP_Ship_Freighter", style="industrial", door=(160, 230), main_door=(260, 260),
        segs=[("bay", "engine", 850, 1400, 720, {}),
              ("bay", "hold", 2500, 1900, 820, {}),
              ("block", 950, 1200, 320, dict(spine=240, hs=270, name="Crew",
                  port=[("bunks", 475, {}), ("mess", 475, {})],
                  stbd=[("airlock", 350, {}), ("workshop", 600, {})])),
              ("bridge", "mid", 950, 1000, 340, {"nose": 0.55, "chamfer": 300})]),
    # --- Behemoth hauler: bulk tank hold ---
    "Behemoth": dict(
        bp=BP + "BP_Ship_Transport_Behemoth", style="industrial", door=(160, 230), main_door=(260, 260),
        segs=[("bay", "engine", 900, 1400, 720, {}),
              ("bay", "tankhold", 2400, 1800, 900, {}),
              ("block", 1000, 1200, 320, dict(spine=240, hs=270, name="Crew",
                  port=[("cabins", 600, {}), ("galley", 400, {})],
                  stbd=[("storage", 500, {}), ("medbay", 500, {})])),
              ("bridge", "mid", 950, 1000, 340, {"nose": 0.5, "chamfer": 300})]),
    # --- Excavator Mining Barge (25-50): ore refinery ---
    "Mining": dict(
        bp=BP + "BP_Ship_Mining", style="industrial", door=(160, 230), main_door=(260, 260),
        segs=[("bay", "engine", 850, 1400, 720, {}),
              ("bay", "refinery", 1800, 1800, 820, {}),
              ("block", 1250, 1300, 330, dict(spine=260, hs=280, name="Crew",
                  port=[("bunks", 625, {}), ("mess", 625, {})],
                  stbd=[("ready", 600, {"eva": True}), ("workshop", 650, {})])),
              ("bridge", "mid", 950, 1100, 340, {"nose": 0.5, "chamfer": 320})]),
    # --- Lifeline Medical Cruiser (35-120): wards, surgery, cryo ---
    "Cruiser": dict(
        bp=BP + "BP_Ship_Cruiser", style="clean", door=(200, 240), main_door=(280, 260),
        segs=[("bay", "engine", 850, 1400, 680, {"style": "industrial"}),
              ("block", 1100, 1600, 360, dict(spine=300, hs=310, name="Wards",
                  port=[("ward", 1100, {"triage": True})], stbd=[("cryo", 1100, {})])),
              ("block", 1700, 1600, 360, dict(spine=300, hs=310, name="Medical",
                  port=[("ward", 1050, {}), ("cabins", 650, {})],
                  stbd=[("surgery", 850, {}), ("lab", 850, {})])),
              ("bridge", "mid", 1000, 1100, 360, {"nose": 0.55, "chamfer": 320})]),
    # --- Vanguard Escort Carrier (40-85): fighter hangar ---
    "Carrier": dict(
        bp=BP + "BP_Ship_Carrier", style="military", door=(180, 240), main_door=(300, 270),
        segs=[("bay", "engine", 950, 1600, 820, {}),
              ("bay", "hangar", 2800, 2400, 1000, {"craft": 3}),
              ("block", 1450, 1400, 350, dict(spine=260, hs=290, name="Crew",
                  port=[("briefing", 700, {}), ("bunks", 750, {})],
                  stbd=[("magazine", 700, {}), ("mess", 750, {})])),
              ("bridge", "cic", 1350, 1300, 450, {"nose": 0.45, "chamfer": 380})]),
    # --- Starliner Luxury Cruiser (40-200): suites, atrium, dining ---
    "Luxury": dict(
        bp=BP + "BP_Ship_Luxury", style="luxury", door=(200, 240), main_door=(300, 270),
        segs=[("bay", "engine", 850, 1400, 680, {"style": "industrial"}),
              ("block", 1500, 1600, 340, dict(spine=300, hs=300, name="Suites",
                  port=[("cabins", 750, {"lux": True, "win": True}), ("cabins", 750, {"lux": True, "win": True})],
                  stbd=[("cabins", 750, {"lux": True, "win": True}), ("lounge", 750, {"win": True})])),
              ("bay", "atrium", 1700, 2200, 820, {"win": "y0", "name": "Atrium"}),
              ("block", 900, 1400, 340, dict(spine=300, hs=300, name="Dining",
                  port=[("mess", 900, {"dining": True, "win": True})],
                  stbd=[("mess", 900, {"dining": True, "win": True})])),
              ("bridge", "mid", 1000, 1000, 340, {"nose": 0.55, "chamfer": 300, "style": "command"})]),
    # --- Genesis Colony Ship (80-3000): cryo hall, hydroponics ---
    "Genesis": dict(
        bp=BP + "BP_Ship_Transport_Genesis", style="clean", door=(200, 240), main_door=(320, 280),
        segs=[("bay", "engine", 1800, 2200, 1100, {"style": "industrial"}),
              ("bay", "cryohall", 2600, 2800, 900, {}),
              ("bay", "farm", 1800, 2600, 700, {}),
              ("block", 1500, 1600, 360, dict(spine=300, hs=310, name="Commons",
                  port=[("mess", 800, {}), ("cabins", 700, {})],
                  stbd=[("medbay", 750, {}), ("storage", 750, {})])),
              ("bridge", "cic", 1500, 1500, 460, {"nose": 0.5, "chamfer": 420})]),
    # --- Sovereign Command Cruiser (200-500): fleet flagship ---
    "CommandXL": dict(
        bp=BP + "BP_CommandXL", style="command", door=(200, 240), main_door=(320, 280),
        segs=[("bay", "engine", 1800, 2400, 1200, {}),
              ("bay", "hangar", 2100, 2600, 1000, {"craft": 2, "name": "Hangar"}),
              ("block", 1650, 1800, 400, dict(spine=320, hs=330, name="Crew",
                  port=[("bunks", 850, {}), ("mess", 800, {})],
                  stbd=[("armory", 650, {}), ("medbay", 1000, {})])),
              ("block", 1450, 1800, 400, dict(spine=320, hs=330, name="Staff",
                  port=[("cabins", 750, {"lux": True}), ("briefing", 700, {})],
                  stbd=[("ops", 1450, {"big": True})])),
              ("bridge", "flag", 1750, 2400, 650, {"nose": 0.45, "chamfer": 560})]),
}


# ----------------------------------------------------------------------------
# Layout
# ----------------------------------------------------------------------------
SPACES = []          # (name, x0, x1, y0, y1) for the walk check


class style_scope:
    def __init__(self, name):
        self.name = name

    def __enter__(self):
        self.saved = dict(S)
        if self.name:
            S.clear(); S.update(dk.STYLES[self.name])

    def __exit__(self, *a):
        S.clear(); S.update(self.saved)


def corridor(part, x0, x1, sw, hs, doors):
    """Spine corridor dressing between x0..x1, width sw, ceiling hs.
    doors: {+1: [x...], -1: [x...]}. Ribbed/piped for the working styles,
    smooth panels with cove light for clean/luxury."""
    hw = sw / 2 - WT / 2
    x0, x1 = x0 + 15, x1 - 15
    smooth = S["wall"] == dk.CLN
    box(part, "wall", x0, x1, -hw, hw, hs, hs + 10)
    dw = 110
    near = lambda x, s, pad=0: any(abs(x - d) < dw + pad for d in doors[s])
    if smooth:
        box(part, "soft" if S["floor"] == CRP else "accent", x0, x1, -hw * 0.45, hw * 0.45, 0, 0.6)
        for s in (1, -1):
            box(part, "trim", x0, x1, s * (hw - 4), s * hw, 0, 12)
            box(part, "frame", x0, x1, s * (hw - 3), s * hw, 12, 95)
            box(part, "trim", x0, x1, s * (hw - 6), s * hw, 95, 100)
            box(part, "frame", x0, x1, s * (hw - 22), s * hw, hs - 22, hs)                # cove
            box("Lights", S["lamp"], x0, x1, s * (hw - 23), s * (hw - 22), hs - 20, hs - 8)
        x = x0 + 200
        k = 0
        while x < x1 - 150:
            for s in (1, -1):
                if not near(x, s, 40):
                    box(part, "frame", x - 12, x + 12, s * (hw - 8), s * hw, 0, hs)
                    if S["wall"] == dk.CLN and S["frame"] == dk.WOD:
                        sconce(part, x + 150, s * (hw - 1), 190, -90 if s > 0 else 90)
            cyl("Lights", S["lamp"], x + 150, 0, hs - 1, hs, 14, 12)
            cyl(part, "trim", x + 150, 0, hs - 3, hs, 18, 12)
            if k % 2 == 0:
                fill(7, x + 150, 0, hs - 30)
            x += 400
            k += 1
        return
    cf = min(100.0, sw * 0.28)
    for s in (1, -1):
        extrude(part, "frame", [(s * hw, hs - cf * 0.6), (s * hw, hs), (s * (hw - cf), hs)], 'x', x0, x1)
        box(part, "grate", x0, x1, s * (hw - 55), s * (hw - 12), 0, 0.6)
        box(part, "trim", x0, x1, s * (hw - 12), s * hw, 0, 14)
    box(part, "grate", x0, x1, -hw * 0.3, hw * 0.3, 0, 0.6)
    for s in (1, -1):
        box(part, "accent", x0, x1, s * hw * 0.3, s * (hw * 0.3 + 8), 0, 0.7)
    xs = []
    x = x0 + 150
    while x < x1 - 100:
        xs.append(x)
        x += 300
    for i, xr in enumerate(xs):
        for s in (1, -1):
            if near(xr, s, 60):
                continue
            box(part, "frame", xr - 14, xr + 14, s * (hw - 20), s * hw, 0, hs - cf * 0.6)
            box(part, "accent", xr - 15, xr + 15, s * (hw - 21), s * (hw + 1), 0, 20)
        box(part, "frame", xr - 14, xr + 14, -(hw - cf), hw - cf, hs - 20, hs)
        if i < len(xs) - 1:
            xm = (xr + xs[i + 1]) / 2
            ceiling_panel(part, xm, 0, hs, 0.0, 140)
            if i % 2 == 0:
                fill(7, xm, 0, hs - 30)
    for s in (1, -1):
        for (yy, zz, r) in ((hw - 40, hs - 60, 7), (hw - 60, hs - 38, 5)):
            if zz > 200:
                pipe(part, PI if r > 6 else "trim", (x0, s * yy, zz), (x1, s * yy, zz), r, 10)
    for i in range(len(xs) - 1):
        xa, xb = xs[i] + 15, xs[i + 1] - 15
        xm = (xa + xb) / 2
        for s in (1, -1):
            if any(xa - 110 < d < xb + 110 for d in doors[s]):
                continue
            kind = (i + (s > 0)) % 3
            if kind == 0:
                for v in range(5):
                    box(part, "trim", xm - 40, xm + 40, s * (hw - 2), s * hw, 110 + v * 14, 116 + v * 14)
            elif kind == 1:
                box(part, "body", xm - 20, xm + 20, s * (hw - 9), s * hw, 120, 162)
                box("Lights", S["screen"], xm - 14, xm + 14, s * (hw - 10), s * (hw - 9), 136, 158)
            else:
                cage_lamp(part, xm, s * hw, min(205, hs - 50), -90 if s > 0 else 90)
    for s in (1, -1):
        for d in doors[s]:
            for q in (-1, 1):
                box(part, "accent", d + q * (dw + 20) - 12, d + q * (dw + 20) + 12, s * (hw - 4), s * hw, 30, 190)


def build_ship(key):
    spec = SHIPS[key]
    prefix = f"SM_Int_{key}_Decks"
    dk.reset(prefix, spec["style"])
    SPACES.clear()
    segs = spec["segs"]
    total = sum(s[2] if s[0] != "block" else s[1] for s in segs)
    x = -total / 2
    lay = []
    for s in segs:
        L = s[2] if s[0] != "block" else s[1]
        lay.append((s, x, x + L))
        x += L
    dw, dh = spec["door"]
    mw, mh = spec["main_door"]
    names = {}

    def pname(seg):
        t = seg[0]
        opts = seg[-1]
        base = opts.get("name") or (seg[1].capitalize() if t != "block" else "Crew")
        if t == "bridge":
            base = "Bridge"
        if base in names:
            names[base] += 1
            base = f"{base}{names[base]}"
        else:
            names[base] = 1
        return base

    seg_info = []
    for (seg, x0, x1) in lay:
        t = seg[0]
        W = seg[3] if t != "block" else seg[2]
        H = seg[4] if t != "block" else seg[3]
        seg_info.append(dict(seg=seg, t=t, x0=x0, x1=x1, W=W, H=H, part=pname(seg), opts=seg[-1]))

    sh = "Shell"
    # ---- floors, ceilings, walk floor, side walls --------------------------
    for si in seg_info:
        x0, x1, W, H = si["x0"], si["x1"], si["W"], si["H"]
        with style_scope(si["opts"].get("style")):
            if si["t"] == "bridge":
                o = si["opts"]
                n = o.get("nose", 0.5)
                c = min(o.get("chamfer", 300), (x1 - x0) * 0.6)
                poly = [(x0, -W / 2), (x1 - c, -W / 2), (x1, -W * n / 2), (x1, W * n / 2), (x1 - c, W / 2), (x0, W / 2)]
                prism(sh, "floor", poly, -20, 0)
                prism(sh, "wall", poly, H, H + 20)
                K["Collision"].prism_xy(CL, poly, -20, 0)
                si["poly"] = poly
                si["chamfer"] = c
                sill = o.get("sill", 110)
                top = H - (25 if si["seg"][1] == "cockpit" else 60)
                wall(sh, "wall", (x0, W / 2), (x1 - c, W / 2), 0, H, frames=False)
                wall(sh, "wall", (x1 - c, -W / 2), (x0, -W / 2), 0, H, frames=False)
                for p0, p1 in (((x1 - c, -W / 2), (x1, -W * n / 2)), ((x1, -W * n / 2), (x1, W * n / 2)),
                               ((x1, W * n / 2), (x1 - c, W / 2))):
                    window_band(sh, p0, p1, 0, H, sill, top, mull=120 if W < 900 else 140)
                if H >= 440:
                    for p0, p1 in (((x1 - c, -W / 2), (x1, -W * n / 2)), ((x1, -W * n / 2), (x1, W * n / 2)),
                                   ((x1, W * n / 2), (x1 - c, W / 2))):
                        a = Vector(p0); b = Vector(p1)
                        e = (b - a).normalized(); nn = Vector((-e.y, e.x))
                        Lh = (b - a).length
                        q0 = a + e * (Lh * 0.12) + nn * 24; q1 = a + e * (Lh * 0.88) + nn * 24
                        seg_box(si["part"], "trim", tuple(q0), tuple(q1), 14, top + 30, H - 20)
                        seg_box(si["part"], dk.CO, tuple(q0 + nn * 8 + e * 10), tuple(q1 + nn * 8 - e * 10), 3,
                                top + 40, H - 30)
                SPACES.append((si["part"], x0 + 30, x1 - c, -W / 2 + 30, W / 2 - 30))
                continue
            box(sh, "floor", x0, x1, -W / 2, W / 2, -20, 0)
            box(sh, "wall", x0, x1, -W / 2, W / 2, H, H + 20)
            col(x0, x1, -W / 2, W / 2, -20, 0)
            wslot = EN if (si["t"] == "bay" and si["seg"][1] == "engine") else "wall"
            if si["t"] == "bay":
                win = si["opts"].get("win")
                for side, (p0, p1) in ((1, ((x0, W / 2), (x1, W / 2))), (-1, ((x1, -W / 2), (x0, -W / 2)))):
                    if win == "both" or (win == "y1" and side > 0) or (win == "y0" and side < 0):
                        window_band(sh, p0, p1, 0, H, 120, min(H - 100, 470), mull=180, wall_slot=wslot)
                    else:
                        wall(sh, wslot, p0, p1, 0, H, frames=False)
                SPACES.append((si["part"], x0 + 30, x1 - 30, -W / 2 + 30, W / 2 - 30))

    # ---- transverse walls between segments + stern wall ---------------------
    first = seg_info[0]
    with style_scope(first["opts"].get("style")):
        wall(sh, EN if first["seg"][1] == "engine" else "wall", (first["x0"], -first["W"] / 2),
             (first["x0"], first["W"] / 2), 0, first["H"], frames=False)
    for a, b in zip(seg_info, seg_info[1:]):
        xb = a["x1"]
        Wm = max(a["W"], b["W"])
        Hm = max(a["H"], b["H"])
        if a["t"] == "block" and b["t"] == "block":
            ow = min(a["opts"]["spine"], b["opts"]["spine"]) - WT - 20
            oh = min(a["opts"]["hs"], b["opts"]["hs"]) - 10
        else:
            ow, oh = mw, mh
        with style_scope(b["opts"].get("style")):
            wall(sh, "wall", (xb, -Wm / 2), (xb, Wm / 2), 0, Hm, openings=[(Wm / 2, ow, 0, oh)])

    # ---- blocks: spine walls, dividers, rooms --------------------------------
    for si in seg_info:
        if si["t"] != "block":
            continue
        o = si["opts"]
        x0, x1, W, H = si["x0"], si["x1"], si["W"], si["H"]
        sw, hs = o["spine"], o["hs"]
        part = P(si["part"])
        doors = {1: [], -1: []}
        with style_scope(o.get("style")):
            for side, key_ in ((1, "port"), (-1, "stbd")):
                rooms = o[key_]
                total_r = sum(r[1] for r in rooms)
                scale = (x1 - x0) / total_r
                xr = x0
                spans = []
                for (kind, rl, ropts) in rooms:
                    ra, rb = xr, xr + rl * scale
                    spans.append((kind, ra, rb, ropts))
                    xr = rb
                for i, (kind, ra, rb, ropts) in enumerate(spans):
                    dcx = ropts.get("door_x", (ra + rb) / 2)
                    doors[side].append(dcx)
                    # divider to the next room
                    if i < len(spans) - 1:
                        wall(sh, "wall", (rb, side * sw / 2), (rb, side * W / 2), 0, H, frames=False)
                    # exterior wall piece (window optional)
                    p0, p1 = ((ra, side * W / 2), (rb, side * W / 2)) if side > 0 else ((rb, -W / 2), (ra, -W / 2))
                    if ropts.get("win"):
                        window_band(sh, p0, p1, 0, H, 100, min(H - 60, 240), mull=150)
                    else:
                        wall(sh, "wall", p0, p1, 0, H, frames=False)
                    rm = dr.Rm(part, ra + WT / 2, rb - WT / 2, side * (sw / 2 + WT / 2), side * (W / 2 - WT / 2), H,
                               door_u=dcx, door_w=dw, opts=ropts)
                    dr.ROOMS[kind](rm)
                    dd.dress_room(kind, rm)
                    SPACES.append((f"{si['part']}:{kind}", ra + 40, rb - 40,
                                   *sorted((side * (sw / 2 + 40), side * (W / 2 - 40)))))
                ops = [(d - x0, dw, 0, dh) for d in doors[side]]
                p0, p1 = (x0, side * sw / 2), (x1, side * sw / 2)
                wall(sh, "wall", p0, p1, 0, H, openings=ops)
            corridor(part, x0, x1, sw, hs, doors)
            dd.dress_corridor(part, x0, x1, sw, hs, doors)
            # corridor walls run on above the lowered ceiling to the block height
        SPACES.append((f"{si['part']}:spine", x0 + 30, x1 - 30, -sw / 2 + 40, sw / 2 - 40))

    # ---- bays + bridge -----------------------------------------------------
    for i, si in enumerate(seg_info):
        if si["t"] == "block":
            continue
        with style_scope(si["opts"].get("style")):
            if si["t"] == "bay":
                b = dr.Bay(P(si["part"]), si["x0"], si["x1"], -si["W"] / 2, si["W"] / 2, si["H"], si["opts"],
                           aft_door=i > 0, fwd_door=i < len(seg_info) - 1, door_w=mw)
                dr.BAYS[si["seg"][1]](b)
                dd.dress_bay(si["seg"][1], b)
            else:
                o = dict(si["opts"])
                poly, c = si["poly"], si["chamfer"]
                x0, x1, W = si["x0"], si["x1"], si["W"]
                n = o.get("nose", 0.5)

                def halfw(xq, x0=x0, x1=x1, W=W, c=c, n=n):
                    if xq <= x1 - c:
                        return W / 2
                    t = (xq - (x1 - c)) / c
                    return W / 2 + (W * n / 2 - W / 2) * t
                o["halfw"] = halfw
                o["xc"] = x1 - c
                b = dr.Bay(P(si["part"]), x0, x1, -W / 2, W / 2, si["H"], o, aft_door=True, fwd_door=False, door_w=mw)
                seat, entry, eyaw = dr.bridge(b, si["seg"][1], x1)
                if entry is None:
                    # cockpit too tight to spawn clear of the seat trigger: spawn in
                    # the space behind it, facing the cockpit
                    prev = seg_info[i - 1]
                    entry = ((prev["x0"] + prev["x1"]) / 2, 0.0)
                dd.dress_bridge(b, si["seg"][1], seat, entry)
                SOCKETS["Seat"] = [round(seat[0], 1), round(-seat[1], 1), 100.0, 0.0]
                SOCKETS["Entry"] = [round(entry[0], 1), round(-entry[1], 1), 100.0, eyaw]
                hh = si["H"]
                dk.view("bridge_spawn", (entry[0], entry[1], 165),
                        (x0 if eyaw == 180.0 else x1, 0, 130), 16)
                dk.view("bridge_fwd", (x0 + 120, -W * 0.3, min(210, hh - 40)), (x1, W * 0.1, min(200, hh - 60)), 16)

    # ---- views for every block ---------------------------------------------
    for si in seg_info:
        if si["t"] == "block":
            sw = si["opts"]["spine"]
            dk.view(f"{si['part'].lower()}_spine", (si["x1"] - 120, 0, 165), (si["x0"], 0, 150), 16)
    return dict(prefix=prefix, seg_info=seg_info, spec=spec)


# ----------------------------------------------------------------------------
# Walk check: collision raster at body height + flood fill from Entry
# ----------------------------------------------------------------------------
def walk_check(col_obj, cell=10.0, z_lo=46.0, z_hi=190.0, radius=42.0):   # avatar capsule r42 hh96
    import numpy as np
    me = col_obj.data
    me.calc_loop_triangles()
    V = np.array([v.co[:] for v in me.vertices])
    T = np.array([t.vertices[:] for t in me.loop_triangles])
    mn = V.min(0); mx = V.max(0)
    nx = int((mx[0] - mn[0]) / cell) + 3
    ny = int((mx[1] - mn[1]) / cell) + 3
    blocked = np.zeros((nx, ny), bool)
    for tri in T:
        p = V[tri]
        if p[:, 2].max() < z_lo or p[:, 2].min() > z_hi:
            continue
        e = max(np.linalg.norm(p[0] - p[1]), np.linalg.norm(p[1] - p[2]), np.linalg.norm(p[2] - p[0]))
        n = max(1, int(e / (cell * 0.5)))
        i, j = np.meshgrid(np.arange(n + 1), np.arange(n + 1), indexing="ij")
        m = (i + j) <= n
        a = i[m] / n; b = j[m] / n; c = 1 - a - b
        pts = a[:, None] * p[0] + b[:, None] * p[1] + c[:, None] * p[2]
        pts = pts[(pts[:, 2] >= z_lo) & (pts[:, 2] <= z_hi)]
        if len(pts) == 0:
            continue
        ci = ((pts[:, 0] - mn[0]) / cell).astype(int)
        cj = ((pts[:, 1] - mn[1]) / cell).astype(int)
        blocked[ci, cj] = True
    # capsule clearance
    r = int(math.ceil((radius + 6) / cell))   # 42 cm capsule + margin so routes never shave a jamb
    dil = blocked.copy()
    for di in range(-r, r + 1):
        for dj in range(-r, r + 1):
            if di * di + dj * dj <= r * r:
                dil |= np.roll(np.roll(blocked, di, 0), dj, 1)
    ex, ey = SOCKETS["Entry"][0], -SOCKETS["Entry"][1]
    si, sj = int((ex - mn[0]) / cell), int((ey - mn[1]) / cell)
    seen = np.zeros_like(dil)
    report = {"entry_free": bool(not dil[si, sj])}
    if dil[si, sj]:
        return report
    from collections import deque
    q = deque([(si, sj)])
    seen[si, sj] = True
    while q:
        a, b = q.popleft()
        for da, db in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            u, v = a + da, b + db
            if 0 <= u < nx and 0 <= v < ny and not seen[u, v] and not dil[u, v]:
                seen[u, v] = True
                q.append((u, v))
    rooms = {}
    for (name, x0, x1, y0, y1) in SPACES:
        i0, i1 = int((x0 - mn[0]) / cell), int((x1 - mn[0]) / cell)
        j0, j1 = int((min(y0, y1) - mn[1]) / cell), int((max(y0, y1) - mn[1]) / cell)
        free = ~dil[i0:i1, j0:j1]
        nf = int(free.sum())
        rooms[name] = round(float(seen[i0:i1, j0:j1][free].sum()) / max(1, nf), 2)
    sx, sy = SOCKETS["Seat"][0], -SOCKETS["Seat"][1]
    # the seat itself sits in its chair's blocker; accept any reachable cell within 90 cm
    k = int(90 / cell)
    ci, cj = int((sx - mn[0]) / cell), int((sy - mn[1]) / cell)
    report["seat_reachable"] = bool(seen[max(0, ci - k):ci + k + 1, max(0, cj - k):cj + k + 1].any())
    report["rooms"] = rooms
    report["unreached"] = [n for n, v in rooms.items() if v < 0.5]
    report["route"] = walk_route(seen, dil, mn, cell, (si, sj), (ci, cj), k)
    return report


def walk_route(seen, dil, mn, cell, start, seat, k):
    """Entry -> the most open reachable spot of every space -> beside the helm,
    as a line-of-sight-simplified polyline in UE design cm. The PIE test
    (Tools/pie_walk_ship_decks.py) steers the avatar along it."""
    import numpy as np
    from collections import deque
    nx, ny = seen.shape
    # distance-to-blocker map keeps targets and path corners off the furniture
    clear = np.where(dil, 0, 1).astype(np.int32)
    d = clear * 1000
    for _ in range(12):
        d = np.minimum(d, np.minimum.reduce([np.roll(d, 1, 0) + 1, np.roll(d, -1, 0) + 1,
                                             np.roll(d, 1, 1) + 1, np.roll(d, -1, 1) + 1]))
    goals = []
    for (name, x0, x1, y0, y1) in SPACES:
        i0, i1 = int((x0 - mn[0]) / cell), int((x1 - mn[0]) / cell)
        j0, j1 = int((min(y0, y1) - mn[1]) / cell), int((max(y0, y1) - mn[1]) / cell)
        sub = np.where(seen[i0:i1, j0:j1], d[i0:i1, j0:j1], -1)
        if sub.size and sub.max() > 0:
            a, b = np.unravel_index(int(sub.argmax()), sub.shape)
            goals.append((name, (i0 + a, j0 + b)))
    # beside the helm: the reachable cell nearest the seat
    ci, cj = seat
    best = None
    for a in range(max(0, ci - k), min(nx, ci + k + 1)):
        for b in range(max(0, cj - k), min(ny, cj + k + 1)):
            if seen[a, b]:
                dd = (a - ci) ** 2 + (b - cj) ** 2
                if best is None or dd < best[0]:
                    best = (dd, (a, b))
    if best:
        goals.append(("helm", best[1]))

    def bfs(s, g):
        prev = {s: None}
        q = deque([s])
        while q:
            c = q.popleft()
            if c == g:
                break
            a, b = c
            for da, db in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                u = (a + da, b + db)
                if 0 <= u[0] < nx and 0 <= u[1] < ny and u not in prev and seen[u]:
                    prev[u] = c
                    q.append(u)
        path, c = [], g
        while c is not None:
            path.append(c)
            c = prev.get(c)
        return path[::-1]

    def los(a, b):
        n = max(abs(b[0] - a[0]), abs(b[1] - a[1])) * 2 + 1
        for t in range(n + 1):
            u = round(a[0] + (b[0] - a[0]) * t / n); v = round(a[1] + (b[1] - a[1]) * t / n)
            if not seen[u, v]:     # seen cells are already capsule-clear
                return False
        return True

    route, cur = [], start
    for name, g in goals:
        path = bfs(cur, g)
        if len(path) < 2:
            continue
        pts = [path[0]]
        i = 0
        while i < len(path) - 1:
            j = len(path) - 1
            while j > i + 1 and not los(path[i], path[j]):
                j -= 1
            pts.append(path[j])
            i = j
        for (a, b) in pts[1:]:
            route.append([round(mn[0] + (a + 0.5) * cell, 1), round(-(mn[1] + (b + 0.5) * cell), 1),
                          name if (a, b) == g else ""])
        cur = g
    return route


# ----------------------------------------------------------------------------
# Export + contract
# ----------------------------------------------------------------------------
def export(ob, dry):
    out = os.path.join(GEN, ob.name + ".fbx")
    if dry:
        return out
    bpy.ops.object.select_all(action='DESELECT')
    ob.select_set(True)
    bpy.context.view_layer.objects.active = ob
    bpy.ops.export_scene.fbx(
        filepath=out, use_selection=True, object_types={'MESH'},
        apply_scale_options='FBX_SCALE_ALL', apply_unit_scale=True,
        axis_forward='-Y', axis_up='Z', mesh_smooth_type='FACE',
    )
    return out


def run(key, dry):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    sc = bpy.context.scene
    sc.unit_settings.system = 'METRIC'
    sc.unit_settings.length_unit = 'CENTIMETERS'
    info = build_ship(key)
    prefix = info["prefix"]
    objs = {p: K[p].to_object() for p in dk.PARTS}
    for ob in objs.values():
        for poly in ob.data.polygons:
            poly.use_smooth = False
    problems = []
    shell = objs["Shell"]
    pts = [v.co for v in shell.data.vertices]
    mn = Vector([min(p[i] for p in pts) for i in range(3)])
    mx = Vector([max(p[i] for p in pts) for i in range(3)])
    for name, ob in objs.items():
        for v in ob.data.vertices:
            p = v.co
            if any(p[i] < mn[i] - 1 or p[i] > mx[i] + 1 for i in range(3)):
                problems.append(f"{name}: vertex {tuple(round(c, 1) for c in p)} outside shell bounds")
                break
    for k in ("Entry", "Seat"):
        if k not in SOCKETS:
            problems.append("missing socket " + k)
    if "Entry" in SOCKETS and "Seat" in SOCKETS:
        e, st = SOCKETS["Entry"], SOCKETS["Seat"]
        if math.hypot(e[0] - st[0], e[1] - st[1]) < 300:
            # the seat trigger ignores the avatar within 2 m of its spawn point
            problems.append("entry %.0f cm from the helm seat (< 300)" % math.hypot(e[0] - st[0], e[1] - st[1]))
    wc = walk_check(objs["Collision"])
    if not wc.get("entry_free"):
        problems.append("entry is inside a walk blocker")
    elif not wc.get("seat_reachable"):
        problems.append("helm seat not reachable from entry")
    for n in wc.get("unreached", []):
        problems.append(f"space {n} not reachable at deck level ({wc['rooms'][n]:.0%})")
    tris = {}
    for name, ob in objs.items():
        ob.data.calc_loop_triangles()
        tris[name] = len(ob.data.loop_triangles)
    nl = sum(1 for k in SOCKETS if k.startswith("L"))
    if nl > 64:
        problems.append(f"{nl} light sockets (> 64)")
    contract = {
        "prefix": prefix, "ship": key, "bp": info["spec"]["bp"], "style": info["spec"]["style"],
        "family": "ShipDecks (generic full deck, driven by P_ sockets)",
        "runtime_scale_on_imported_mesh": 0.01,
        "design_bounds_cm": {"min": [round(v, 1) for v in mn], "max": [round(v, 1) for v in mx]},
        "size_m": [round((mx.x - mn.x) / 100, 1), round((mx.y - mn.y) / 100, 1)],
        "parts": dk.PARTS[:],
        "sockets_ue_design_cm": dict(SOCKETS),
        "views": VIEWS[:],
        "plan_z": min((si["opts"]["hs"] if si["t"] == "block" else si["H"]) for si in info["seg_info"]) - 10,
        "light_count": nl,
        "fixture_count": sum(1 for k in SOCKETS if k.startswith("X_")),
        "tris": tris, "tris_total": sum(tris.values()),
        "slots": {n: [m.name for m in ob.data.materials] for n, ob in objs.items()},
        "walk": {k: v for k, v in wc.items()},
        "blender_to_ue_axes": "UE(x, y, z) = Blender(x, -y, z) * 100 * 0.01",
        "problems": problems,
    }
    print("SHIP", key, "size", contract["size_m"], "m  tris", contract["tris_total"], " lights", nl,
          " parts", len(dk.PARTS), " fixtures", contract["fixture_count"])
    for p in dk.PARTS:
        export(objs[p], dry)
    if not dry:
        with open(os.path.join(GEN, prefix + "_contract.json"), "w") as f:
            json.dump(contract, f, indent=1)
    if problems:
        print("CONTRACT_PROBLEMS", key, len(problems), problems[:12])
    return contract


def main():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    dry = "--dry-run" in argv
    if "--list" in argv:
        print(" ".join(SHIPS))
        return
    keys = [a for a in argv if not a.startswith("--")] or list(SHIPS)
    bad = 0
    for key in keys:
        c = run(key, dry)
        bad += bool(c["problems"])
    print("DONE", len(keys), "ships,", bad, "with problems")


if __name__ == "__main__":
    main()
