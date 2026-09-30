"""Shared kit for the full-deck walkable ship interiors (Blender 5.x).

Used by Tools/build_ship_decks.py (every ship but the Battleship) and modelled on
Tools/build_battleship_decks.py, which stays standalone. The primitives are the
Battleship's: walls split around openings with heavy door frames, window
bands, ribs, railings, grated stairs over smooth ramps, consoles, CRTs, bunks,
lockers and crates. The difference is a per-ship STYLE: each helper asks the
style for a material ROLE ("frame", "accent", "screen", ...) instead of
hard-coding the olive-drab military slots, so a science vessel reads clinical
white/blue, a freighter bare steel with safety orange, the Starliner wood,
brass and carpet, and the stealth frigate near-black with blue strip light.

State is module-level (like the Battleship builder): reset(prefix, style)
before a ship, then P(name) hands out parts lazily. Every visual part may add
walk blockers to "Collision" and emissive bits to "Lights".

Units: design cm, Blender +X = bow, UE y = -Blender y (light() flips).
"""
import math

from mathutils import Vector, Matrix

from build_capital_interiors import Part, circle  # noqa: E402

# ----------------------------------------------------------------------------
# Material slots (UE material names; Tools/import_ship_decks.py makes the new ones)
# ----------------------------------------------------------------------------
SH, DK, CO, ST, LW, VP, HT, PI = ("M_Int_Shell", "M_Int_Deck", "M_Int_Console", "M_Int_Stations",
                                  "M_Int_Lights", "M_Int_Viewport", "M_Int_Hatch", "M_Int_Vents")
BU, ME, GA, EN = "M_Int_Bunks", "M_Int_Mess", "M_Int_Galley", "M_Interior_Eng"
GR, HZ, BH = "M_Int_Grate", "M_Int_Hazard", "M_Int_Bulkhead"
LR, LA, LG, LB = "M_Int_LightsRed", "M_Int_LightsAmber", "M_Int_LightsGreen", "M_Int_LightsBlue"
LC, LP = "M_Int_LightsCyan", "M_Int_LightsWarm"
CLN, STL, DRK, PNT, ABL = "M_Int_Clean", "M_Int_Steel", "M_Int_Dark", "M_Int_Paint", "M_Int_AccentBlue"
CRP, WOD, BRS, PLT = "M_Int_Carpet", "M_Int_Wood", "M_Int_Brass", "M_Int_Plant"
CL = "M_Int_Collision"

# ----------------------------------------------------------------------------
# Styles: role -> slot. fill/lamp = light socket colour letters (W A R G B C P).
# ----------------------------------------------------------------------------
STYLES = {
    # 90s military, like the Battleship: olive bulkheads, hazard yellow, green CRTs
    "military": dict(wall=SH, frame=BH, trim=HT, accent=HZ, floor=DK, grate=GR, body=ST,
                     screen=LG, screen2=LA, lamp=LW, fill="W", warn=LR, rail=HZ, soft=BU, table=ME),
    # law enforcement: fleet grey with blue accents
    "patrol": dict(wall=SH, frame=STL, trim=HT, accent=ABL, floor=DK, grate=GR, body=ST,
                   screen=LB, screen2=LG, lamp=LW, fill="W", warn=LR, rail=ABL, soft=BU, table=ME),
    # commercial/industrial: bare steel, safety orange, amber CRTs
    "industrial": dict(wall=SH, frame=STL, trim=HT, accent=PNT, floor=DK, grate=GR, body=ST,
                       screen=LA, screen2=LG, lamp=LW, fill="W", warn=LA, rail=PNT, soft=BU, table=ME),
    # science / medical / colony: white panels, blue trim, cyan screens
    "clean": dict(wall=CLN, frame=STL, trim=HT, accent=ABL, floor=DK, grate=GR, body=CLN,
                  screen=LC, screen2=LB, lamp=LW, fill="W", warn=LR, rail=ABL, soft=BU, table=GA),
    # stealth: near-black plate, blue strip light
    "stealth": dict(wall=DRK, frame=DRK, trim=HT, accent=LB, floor=DRK, grate=GR, body=HT,
                    screen=LB, screen2=LC, lamp=LB, fill="B", warn=LR, rail=LB, soft=HT, table=DRK),
    # luxury liner: white, wood, brass, carpet, warm light
    "luxury": dict(wall=CLN, frame=WOD, trim=BRS, accent=BRS, floor=CRP, grate=DK, body=WOD,
                   screen=LC, screen2=LP, lamp=LP, fill="P", warn=LA, rail=BRS, soft=CRP, table=WOD),
    # fleet flagship: grey steel, blue accents, cyan/blue plots
    "command": dict(wall=SH, frame=STL, trim=HT, accent=ABL, floor=DK, grate=GR, body=ST,
                    screen=LC, screen2=LB, lamp=LW, fill="W", warn=LR, rail=ABL, soft=BU, table=ME),
}

WT = 30.0            # wall thickness (centred on the wall line)
K = {}
SOCKETS = {}
VIEWS = []
PREFIX = ""
S = dict(STYLES["military"])
PARTS = []


def reset(prefix, style):
    global PREFIX
    PREFIX = prefix
    K.clear(); SOCKETS.clear(); VIEWS.clear(); PARTS.clear()
    S.clear(); S.update(STYLES[style])
    for p in ("Shell", "Lights", "Collision"):
        P(p)


def P(name):
    """Part by name, created on first use (order kept for the export list)."""
    if name not in K:
        K[name] = Part(f"{PREFIX}_{name}")
        PARTS.append(name)
    return name


def R(role):
    """Slot for a style role (or pass a slot straight through)."""
    return S.get(role, role)


def view(tag, loc, look, lens=16):
    VIEWS.append([tag, list(loc), list(look), lens])


# ----------------------------------------------------------------------------
# Primitive helpers
# ----------------------------------------------------------------------------
def Mx(x, y, z=0.0, yaw=0.0):
    return Matrix.Translation(Vector((x, y, z))) @ Matrix.Rotation(math.radians(yaw), 4, 'Z')


def box(part, slot, x0, x1, y0, y1, z0, z1, M=None):
    K[P(part)].box(R(slot), min(x0, x1), max(x0, x1), min(y0, y1), max(y0, y1), min(z0, z1), max(z0, z1), M)


def col(x0, x1, y0, y1, z0, z1, M=None):
    box("Collision", CL, x0, x1, y0, y1, z0, z1, M)


def solid(part, slot, x0, x1, y0, y1, z0, z1, M=None):
    box(part, slot, x0, x1, y0, y1, z0, z1, M)
    col(x0, x1, y0, y1, z0, z1, M)


def prism(part, slot, poly, z0, z1, M=None):
    K[P(part)].prism_xy(R(slot), poly, z0, z1, M)


def extrude(part, slot, prof, axis, a0, a1, M=None):
    """Extrude a 2D profile along `axis`. prof is (u, v): for axis 'x' that is
    (y, z), for 'y' it is (x, z), for 'z' it is (x, y)."""
    M = M or Matrix()
    n = len(prof)

    def Pt(a, u, v):
        if axis == 'x':
            return Vector((a, u, v))
        if axis == 'y':
            return Vector((u, a, v))
        return Vector((u, v, a))
    verts = [M @ Pt(a0, u, v) for u, v in prof] + [M @ Pt(a1, u, v) for u, v in prof]
    faces = [list(range(n))[::-1], list(range(n, 2 * n))]
    for i in range(n):
        j = (i + 1) % n
        faces.append([i, j, n + j, n + i])
    K[P(part)]._faces(verts, faces, R(slot))


def cyl(part, slot, cx, cy, z0, z1, r, segs=16, M=None):
    K[P(part)].prism_xy(R(slot), circle(cx, cy, r, segs), z0, z1, M)


def ring(part, slot, cx, cy, r0, r1, z0, z1, segs=32, a0=0.0, a1=360.0):
    K[P(part)].annulus(R(slot), cx, cy, r0, r1, z0, z1, segs, a0, a1)


def pipe(part, slot, p0, p1, r, segs=10):
    p0 = Vector(p0); p1 = Vector(p1)
    d = p1 - p0
    L = d.length
    if L < 1e-3:
        return
    z = d.normalized()
    a = Vector((0, 0, 1)) if abs(z.z) < 0.9 else Vector((1, 0, 0))
    x = a.cross(z).normalized()
    y = z.cross(x)
    M = Matrix(((x[0], y[0], z[0], p0[0]), (x[1], y[1], z[1], p0[1]),
                (x[2], y[2], z[2], p0[2]), (0, 0, 0, 1)))
    K[P(part)].prism_xy(R(slot), circle(0, 0, r, segs), 0.0, L, M)


def seg_box(part, slot, p0, p1, t, z0, z1):
    """Plan box of thickness t centred on the segment p0->p1."""
    a = Vector((p0[0], p0[1])); b = Vector((p1[0], p1[1]))
    if (b - a).length < 1e-3:
        return
    e = (b - a).normalized()
    n = Vector((-e.y, e.x)) * (t / 2)
    K[P(part)].prism_xy(R(slot), [tuple(a - n), tuple(b - n), tuple(b + n), tuple(a + n)], z0, z1)


def light(color, radius_m, x, y, z, shadow=False):
    """Point-light socket (UE design cm; y flipped here)."""
    n = sum(1 for k in SOCKETS if k.startswith("L"))
    name = "%s_%s_%02d_%03d" % ("LS" if shadow else "L", color, radius_m, n)
    SOCKETS[name] = [round(x, 1), round(-y, 1), round(z, 1), 0.0]


def fill(radius_m, x, y, z, shadow=False):
    light(S["fill"], radius_m, x, y, z, shadow)


# ----------------------------------------------------------------------------
# Architecture
# ----------------------------------------------------------------------------
def wall(part, slot, p0, p1, z0, z1, openings=(), t=WT, frames=True, collide=True):
    """Wall along p0->p1. openings: (s_centre, width, zb, zt) with s measured
    from p0; the wall is split around each and gets a door frame."""
    a = Vector(p0); b = Vector(p1)
    L = (b - a).length
    if L < 1:
        return
    e = (b - a).normalized()
    at = lambda s: tuple(a + e * s)
    cur = 0.0
    ops = sorted(openings)
    for (sc, w, zb, zt) in ops:
        s0, s1 = max(0.0, sc - w / 2), min(L, sc + w / 2)
        if s0 > cur:
            seg_box(part, slot, at(cur), at(s0), t, z0, z1)
            if collide:
                seg_box("Collision", CL, at(cur), at(s0), t, z0, z1)
        if zb > z0:
            seg_box(part, slot, at(s0), at(s1), t, z0, zb)
            if collide:
                seg_box("Collision", CL, at(s0), at(s1), t, z0, zb)
        if zt < z1:
            seg_box(part, slot, at(s0), at(s1), t, zt, z1)
            if collide:
                seg_box("Collision", CL, at(s0), at(s1), t, zt, z1)
        cur = max(cur, s1)
    if cur < L:
        seg_box(part, slot, at(cur), at(L), t, z0, z1)
        if collide:
            seg_box("Collision", CL, at(cur), at(L), t, z0, z1)
    if frames:
        yaw = math.degrees(math.atan2(e.y, e.x))
        for (sc, w, zb, zt) in ops:
            c = a + e * sc
            door_frame(part, c.x, c.y, zb, yaw, w, zt - zb, t)


def door_frame(part, cx, cy, zb, yaw, w, h, t=WT):
    """Door frame: jambs + header proud of both faces, chamfered top corners,
    accent jamb faces, threshold plate, status lamps."""
    M = Mx(cx, cy, zb, yaw)
    d = t / 2 + 10
    jw = 22 if w < 160 else 26
    hw = w / 2
    for s in (-1, 1):
        x0, x1 = (hw, hw + jw) if s > 0 else (-hw - jw, -hw)
        box(part, "frame", x0, x1, -d, d, 0, h + 30, M)
        xi = hw if s > 0 else -hw
        box(part, "accent", xi - 1.5 * s, xi, -d + 2, d - 2, 0, h, M)
        g = min(34.0, hw * 0.4)
        extrude(part, "frame", [(xi, h), (xi - g * s, h), (xi, h - g)], 'y', -d + 3, d - 3, M)
    box(part, "frame", -hw - jw, hw + jw, -d, d, h, h + 30, M)
    box(part, "accent", -hw, hw, -d + 2, d - 2, h - 1.5, h, M)
    box(part, "trim", -hw, hw, -d, d, 0, 0.8, M)
    for s in (-1, 1):
        box(part, "trim", -18, 18, s * d, s * (d + 4), h + 7, h + 23, M)
        box("Lights", LG if s > 0 else S["warn"], -13, 13, s * (d + 4), s * (d + 5.5), h + 10, h + 20, M)


def window_band(part, p0, p1, z0, z1, zw0, zw1, mull=140.0, t=WT, wall_slot="wall"):
    """Wall along p0->p1 with a continuous window zw0..zw1."""
    seg_box(part, wall_slot, p0, p1, t, z0, zw0)
    seg_box(part, wall_slot, p0, p1, t, zw1, z1)
    seg_box("Collision", CL, p0, p1, t, z0, z1)
    a = Vector(p0); b = Vector(p1)
    L = (b - a).length
    e = (b - a).normalized()
    seg_box(part, VP, p0, p1, 4, zw0, zw1)
    n = max(1, int(round(L / mull)))
    for k in range(n + 1):
        c = a + e * (L * k / n)
        seg_box(part, "frame", tuple(c - e * 7), tuple(c + e * 7), t + 16, zw0, zw1)
    seg_box(part, "frame", p0, p1, t + 24, zw0 - 14, zw0)
    seg_box(part, "frame", p0, p1, t + 24, zw1, zw1 + 16)


def ribs(part, p0, p1, side, z1, step=400.0, depth=16.0, w=34.0, skip=(), knee=True, slot="frame"):
    """Vertical pilasters on one face of the wall line p0->p1 (side=+1 = left
    of the direction), knee brace under the ceiling. skip: (s0, s1) ranges."""
    a = Vector(p0); b = Vector(p1)
    L = (b - a).length
    e = (b - a).normalized()
    yaw = math.degrees(math.atan2(e.y, e.x))
    n = int(L // step)
    for k in range(1, n + 1):
        s = k * L / (n + 1)
        c = a + e * s
        if any(s0 - w < s < s1 + w for (s0, s1) in skip):
            continue
        M = Mx(c.x, c.y, 0, yaw)
        y0 = side * WT / 2
        y1 = side * (WT / 2 + depth)
        box(part, slot, -w / 2, w / 2, y0, y1, 0, z1, M)
        box(part, "trim", -w / 2 - 2, w / 2 + 2, y0, side * (WT / 2 + depth + 3), 0, 18, M)
        if knee:
            kd = min(90.0, z1 * 0.18)
            extrude(part, slot, [(y1, z1), (y1 + side * kd, z1), (y1, z1 - kd * 1.4)], 'x', -w / 2 + 6, w / 2 - 6, M)


def railing(part, p0, p1, z, h=105.0, post=160.0, collide=True, kick=True):
    a = Vector((p0[0], p0[1], z)); b = Vector((p1[0], p1[1], z))
    L = (b - a).length
    n = max(1, int(L // post))
    for k in range(n + 1):
        c = a + (b - a) * (k / n)
        pipe(part, "trim", c, c + Vector((0, 0, h)), 3.2, 8)
    up = Vector((0, 0, h))
    pipe(part, "rail", a + up, b + up, 3.0, 8)
    pipe(part, "trim", a + up * 0.55, b + up * 0.55, 2.2, 6)
    if kick:
        seg_box(part, "trim", p0, p1, 2.0, z, z + 12)
    if collide:
        seg_box("Collision", CL, p0, p1, 8.0, z, z + h + 20)


def stairs(part, x0, y0, run, width, z0, z1, yaw, rails=(True, True), collide=True):
    """Open grated stair climbing local +X from (x0, y0) at z0 to z1.
    Visual steps; the collision is a smooth wedge ramp under the nosings."""
    M = Mx(x0, y0, 0, yaw)
    rise = z1 - z0
    n = max(1, int(math.ceil(rise / 20.0)))
    g = run / n
    hw = width / 2
    for i in range(n):
        zt = z0 + rise * (i + 1) / n
        box(part, "grate", i * g - 2, (i + 1) * g + 2, -hw + 6, hw - 6, zt - 4, zt, M)
        box(part, "accent", (i + 1) * g - 4, (i + 1) * g + 2, -hw + 6, hw - 6, zt - 3.5, zt + 0.3, M)
    for s in (-1, 1):
        prof = [(0, z0 - 20), (run, z1 - 20), (run, z1 + 10), (0, z0 + 10)]
        extrude(part, "trim", prof, 'y', s * hw - 6 * (s > 0), s * hw + 6 * (s < 0), M)
        if rails[0 if s < 0 else 1]:
            for k in range(0, 5):
                t = k / 4
                c = M @ Vector((run * t, s * (hw - 3), z0 + rise * t))
                pipe(part, "trim", c, c + Vector((0, 0, 100)), 3.0, 8)
            a = M @ Vector((0, s * (hw - 3), z0 + 100)); b = M @ Vector((run, s * (hw - 3), z1 + 100))
            pipe(part, "rail", a, b, 3.0, 8)
    if collide:
        extrude("Collision", CL, [(0, z0), (run, z1), (run, z0)], 'y', -hw, hw, M)
        for s in (-1, 1):
            if rails[0 if s < 0 else 1]:
                extrude("Collision", CL, [(0, z0), (run, z1), (run, z1 + 125), (0, z0 + 125)], 'y',
                        s * hw - 4, s * hw + 4, M)


def platform(part, x0, x1, y0, y1, z, legs=True, rails=(), slot="grate"):
    """Walkable raised deck (catwalk/mezzanine) at z, with support posts and
    railings on the listed edges ('x0','x1','y0','y1')."""
    solid(part, slot, x0, x1, y0, y1, z - 10, z)
    box(part, "trim", x0, x1, y0, y1, z - 36, z - 10)
    if legs:
        nx = max(1, int((x1 - x0) // 420)); ny = max(1, int((y1 - y0) // 420))
        for i in range(nx + 1):
            for j in range(ny + 1):
                if 0 < i < nx and 0 < j < ny:
                    continue
                x = x0 + 18 + (x1 - x0 - 36) * i / nx
                y = y0 + 18 + (y1 - y0 - 36) * j / ny
                pipe(part, "trim", (x, y, 0), (x, y, z - 36), 8, 10)
                cyl(part, "trim", x, y, 0, 5, 22, 10)
                cyl("Collision", CL, x, y, 0, z - 36, 12, 8)
    edges = {"x0": ((x0, y0), (x0, y1)), "x1": ((x1, y0), (x1, y1)),
             "y0": ((x0, y0), (x1, y0)), "y1": ((x0, y1), (x1, y1))}
    for e in rails:
        if isinstance(e, tuple):
            railing(part, e[0], e[1], z)
        else:
            railing(part, edges[e][0], edges[e][1], z)


# ----------------------------------------------------------------------------
# Props
# ----------------------------------------------------------------------------
def crt(part, x, y, z, w, h, yaw, screen=None, d=None):
    """Chunky monitor facing local +X: tube housing, inset bezel, screen."""
    M = Mx(x, y, z, yaw)
    d = d or w * 0.8
    box(part, "body", -d * 0.45, 0, -w / 2, w / 2, 0, h, M)
    box(part, "body", -d, -d * 0.45, -w * 0.32, w * 0.32, h * 0.12, h * 0.85, M)
    box(part, "trim", 0, 2.5, -w / 2 + 3, w / 2 - 3, 3, h - 3, M)
    box("Lights", screen or S["screen"], 2.5, 3.3, -w / 2 + 7, w / 2 - 7, 8, h - 7, M)
    box(part, "trim", 0, 3, w / 2 - 12, w / 2 - 4, 1, 4, M)


def flatscreen(part, x, y, z, w, h, yaw, screen=None):
    """Thin wall/stand display facing local +X."""
    M = Mx(x, y, z, yaw)
    box(part, "trim", -6, 0, -w / 2, w / 2, 0, h, M)
    box("Lights", screen or S["screen"], 0, 1, -w / 2 + 4, w / 2 - 4, 4, h - 4, M)


def console(part, x, y, yaw, n_crt=2, chair_too=True, collide=True, screen=None, z=0.0, flat=False):
    """Sloped operator desk. Desk occupies local x -70..0, operator at +x."""
    M = Mx(x, y, z, yaw)
    w = 70.0 * n_crt + 30
    extrude(part, "body", [(-70, 0), (0, 0), (0, 70), (-6, 76), (-44, 104), (-70, 106)], 'y', -w / 2, w / 2, M)
    extrude(part, "trim", [(-2, 0), (4, 0), (4, 10), (-2, 10)], 'y', -w / 2, w / 2, M)
    for k in range(n_crt * 2):
        yc = -w / 2 + 20 + k * (w - 40) / max(1, n_crt * 2 - 1)
        pa = Vector((-8, 0, 80)); pb = Vector((-40, 0, 101))
        Mk = M @ Matrix.Translation(Vector((0, yc, 0)))
        prof_i = [(pa.x, pa.z), (pb.x, pb.z), (pb.x - 1.2, pb.z + 1.8), (pa.x - 1.2, pa.z + 1.8)]
        extrude("Lights", CO if k % 2 else S["screen2"] if k % 3 == 0 else S["warn"], prof_i, 'y', -6, 6, Mk)
    for k in range(n_crt):
        yc = -w / 2 + 50 + k * 70
        c = M @ Vector((-40, yc, 106))
        if flat:
            flatscreen(part, c.x, c.y, c.z, 60, 40, yaw, screen=screen)
        else:
            crt(part, c.x, c.y, c.z, 56, 44, yaw, screen=screen, d=38)
    if chair_too:
        c = M @ Vector((50, 0, 0))
        chair(part, c.x, c.y, c.z, yaw)
    if collide:
        box("Collision", CL, -72, 2, -w / 2, w / 2, 0, 120, M)


def chair(part, x, y, z, yaw, big=False):
    """Pedestal crew chair; the sitter faces local -X."""
    M = Mx(x, y, z, yaw)
    s = 1.25 if big else 1.0
    cyl(part, "trim", 0, 0, 0, 5, 26 * s, 10, M)
    cyl(part, "trim", 0, 0, 5, 40, 5 * s, 8, M)
    box(part, "frame", -24 * s, 24 * s, -24 * s, 24 * s, 40, 50, M)
    box(part, "soft", -22 * s, 18 * s, -22 * s, 22 * s, 50, 55, M)
    box(part, "frame", 18 * s, 26 * s, -23 * s, 23 * s, 50, 100 * s + 10, M)
    box(part, "soft", 14 * s, 18 * s, -20 * s, 20 * s, 58, 100 * s + 5, M)
    box(part, "body", 20 * s, 27 * s, -12, 12, 100 * s + 10, 100 * s + 30, M)
    for sy in (-1, 1):
        box(part, "trim", -16 * s, 14 * s, sy * 24 * s, sy * 30 * s, 60, 66, M)
        box(part, "trim", 6 * s, 12 * s, sy * 24 * s, sy * 28 * s, 50, 60, M)


def pilot_seat(part, x, y, z, yaw):
    """Flight seat with high back, side sticks and harness; faces local -X."""
    M = Mx(x, y, z, yaw)
    box(part, "trim", -30, 30, -30, 30, 0, 30, M)
    box(part, "frame", -30, 26, -28, 28, 30, 44, M)
    box(part, "soft", -28, 20, -25, 25, 44, 52, M)
    extrude(part, "frame", [(18, 44), (34, 44), (46, 150), (30, 150)], 'y', -28, 28, M)
    extrude(part, "soft", [(16, 52), (22, 52), (34, 142), (28, 142)], 'y', -24, 24, M)
    for s in (-1, 1):
        box(part, "frame", -20, 20, s * 28, s * 40, 44, 70, M)
        pipe(part, "trim", M @ Vector((-12, s * 34, 70)), M @ Vector((-16, s * 34, 92)), 2.5, 6)
        box(part, "accent", -14, 6, s * 12, s * 16, 52, 140, M)          # harness straps


def ceiling_panel(part, x, y, z, yaw=0.0, length=130.0, color=None):
    """Recessed tube fixture hanging under a ceiling at z."""
    M = Mx(x, y, z, yaw)
    box(part, "body", -length / 2, length / 2, -20, 20, -9, 0, M)
    box("Lights", color or S["lamp"], -length / 2 + 5, length / 2 - 5, -14, 14, -10.5, -9, M)
    for sx in (-1, 1):
        box(part, "trim", sx * length / 2 - 3, sx * length / 2 + 3, -22, 22, -12, 0, M)


def cage_lamp(part, x, y, z, yaw, color=None):
    """Wall-mounted caged lamp facing local +X."""
    M = Mx(x, y, z, yaw)
    box(part, "trim", -1, 4, -9, 9, -9, 9, M)
    box("Lights", color or S["warn"], 4, 12, -6, 6, -6, 6, M)
    for k in (-6, 0, 6):
        box(part, "trim", 4, 14, k - 0.8, k + 0.8, -8, 8, M)


def sconce(part, x, y, z, yaw, color=None):
    """Soft wall uplight facing local +X (luxury/clean interiors)."""
    M = Mx(x, y, z, yaw)
    box(part, "trim", 0, 3, -14, 14, -24, 10, M)
    extrude(part, "trim", [(3, -24), (16, 6), (16, 10), (3, 10)], 'y', -12, 12, M)
    box("Lights", color or S["lamp"], 4, 15, -10, 10, 9, 10.5, M)


def flood(part, x, y, z, color=None):
    """Ceiling floodlamp hanging from z."""
    box(part, "trim", x - 4, x + 4, y - 4, y + 4, z - 60, z)
    box(part, "body", x - 45, x + 45, y - 45, y + 45, z - 95, z - 60)
    box(part, "trim", x - 50, x + 50, y - 50, y + 50, z - 100, z - 95)
    box("Lights", color or LA, x - 38, x + 38, y - 38, y + 38, z - 102, z - 100)


def crate(part, x, y, z, sx, sy, sz, yaw=0.0, lid=None, collide=True, body="frame"):
    M = Mx(x, y, z, yaw)
    box(part, body, -sx / 2, sx / 2, -sy / 2, sy / 2, 0, sz, M)
    for q in (-1, 1):
        box(part, "trim", q * sx / 2 - 3, q * sx / 2 + 3, -sy / 2 - 1.5, sy / 2 + 1.5, 0, sz, M)
    box(part, lid or "accent", -sx / 2 + 4, sx / 2 - 4, -sy / 2 - 0.8, -sy / 2, sz * 0.3, sz * 0.55, M)
    if collide:
        box("Collision", CL, -sx / 2, sx / 2, -sy / 2, sy / 2, 0, sz, Mx(x, y, 0, yaw))


def container(part, x, y, z, L, yaw=0.0, slot=None, collide=True):
    """Corrugated cargo container (L x 244 x 259) on its long axis local X."""
    M = Mx(x, y, z, yaw)
    w, h = 244.0, 259.0
    body = slot or "accent"
    box(part, body, -L / 2, L / 2, -w / 2, w / 2, 0, h, M)
    n = int(L // 30)
    for k in range(1, n):
        xx = -L / 2 + k * L / n
        for s in (-1, 1):
            box(part, body, xx - 5, xx + 5, s * w / 2, s * (w / 2 + 3), 12, h - 12, M)
    for q in (-1, 1):
        box(part, "trim", q * L / 2 - 8, q * L / 2 + 1, -w / 2 - 4, w / 2 + 4, 0, h, M)
        for s in (-1, 1):
            box(part, "trim", q * L / 2 - 16, q * L / 2 + 1, s * w / 2 - 12, s * w / 2 + 4, 0, h, M)
    box(part, "trim", -L / 2, L / 2, -w / 2 - 4, w / 2 + 4, h, h + 8, M)
    for k in (-1, 1):                                                           # door bars
        box(part, "trim", L / 2 + 1, L / 2 + 3, k * 40 - 3, k * 40 + 3, 20, h - 20, M)
    if collide:
        box("Collision", CL, -L / 2 - 2, L / 2 + 4, -w / 2 - 4, w / 2 + 4, 0, h + 8, M)


def locker_bank(part, x0, x1, y_wall, depth, side, h=205.0, w=58.0, collide=True):
    """Row of tall lockers along X, backs on y_wall, doors facing side*Y."""
    n = max(1, int((x1 - x0) // w))
    wx = (x1 - x0) / n
    yf = y_wall + side * depth
    box(part, "frame", x0, x1, y_wall, yf, 0, h)
    for k in range(n):
        xa = x0 + k * wx
        box(part, "trim", xa, xa + 2, yf, yf + side * 1.5, 0, h)
        for v in range(4):
            box(part, "trim", xa + 12, xa + wx - 12, yf, yf + side * 1.2, h - 30 - v * 7, h - 27 - v * 7)
        box(part, "body", xa + wx - 10, xa + wx - 6, yf, yf + side * 4, h * 0.45, h * 0.58)
    box(part, "trim", x0, x1, y_wall, yf + side * 2, h, h + 6)
    if collide:
        col(x0, x1, y_wall, yf, 0, h)


def locker_bank_y(part, y0, y1, x_wall, depth, side, h=205.0, collide=True):
    """locker_bank along Y, backs on x_wall, doors facing side*X."""
    n = max(1, int((y1 - y0) // 58))
    wy = (y1 - y0) / n
    xf = x_wall + side * depth
    box(part, "frame", x_wall, xf, y0, y1, 0, h)
    for k in range(n):
        ya = y0 + k * wy
        box(part, "trim", xf, xf + side * 1.5, ya, ya + 2, 0, h)
        for v in range(4):
            box(part, "trim", xf, xf + side * 1.2, ya + 12, ya + wy - 12, h - 30 - v * 7, h - 27 - v * 7)
        box(part, "body", xf, xf + side * 4, ya + wy - 10, ya + wy - 6, h * 0.45, h * 0.58)
    box(part, "trim", x_wall, xf + side * 2, y0, y1, h, h + 6)
    if collide:
        col(x_wall, xf, y0, y1, 0, h)


def shelf_rack(part, x0, x1, y0, y1, h=240.0, levels=4, goods=True, collide=True):
    """Open steel shelving with boxes on the levels."""
    for (x, y) in ((x0, y0), (x1, y0), (x0, y1), (x1, y1)):
        box(part, "accent", x - 4, x + 4, y - 4, y + 4, 0, h)
    for k in range(levels):
        z = 15 + k * (h - 20) / levels
        box(part, "trim", x0, x1, y0, y1, z, z + 4)
        if goods:
            n = max(1, int((x1 - x0) // 70))
            for i in range(n):
                if (i * 7 + k * 3) % 5 == 4:
                    continue
                xa = x0 + 6 + i * (x1 - x0 - 12) / n
                bw = (x1 - x0 - 12) / n - 8
                bh = 22 + ((i + k) % 3) * 10
                box(part, "frame" if (i + k) % 2 else "body", xa, xa + bw, y0 + 5, y1 - 5, z + 4, z + 4 + bh)
    if collide:
        col(x0 - 4, x1 + 4, y0 - 4, y1 + 4, 0, h)


def bunk_stack(part, x0, x1, y0, y1, tiers=3, collide=True):
    """Rack bunk (2- or 3-high): tube frame, trays, mattresses, reading lamps."""
    top = 245.0 if tiers == 3 else 180.0
    zs = (28.0, 110.0, 192.0) if tiers == 3 else (35.0, 125.0)
    for (x, y) in ((x0 + 3, y0 + 3), (x1 - 3, y0 + 3), (x0 + 3, y1 - 3), (x1 - 3, y1 - 3)):
        pipe(part, "trim", (x, y, 0), (x, y, top), 3.0, 8)
    for z in zs:
        box(part, "body", x0, x1, y0, y1, z, z + 8)
        box(part, BU, x0 + 6, x1 - 6, y0 + 5, y1 - 5, z + 8, z + 22)
        box(part, BU, x1 - 40, x1 - 8, y0 + 12, y1 - 12, z + 22, z + 30)
        box(part, "trim", x0, x1, y0 - 1, y0 + 1, z + 8, z + 30)
        box("Lights", LA, x1 - 30, x1 - 12, y1 - 2, y1 - 1, z + 60, z + 66)
    box(part, "frame", x0 - 2, x0 + 2, y0, y1, 0, top)
    box(part, "frame", x1 - 2, x1 + 2, y0, y1, 0, top)
    box(part, "body", x0, x1, y0, y1, top, top + 5)
    if collide:
        col(x0, x1, y0, y1, 0, top + 5)


def bed(part, x, y, yaw, w=100.0, L=200.0, headboard=True, collide=True):
    """Cabin bed, head at local -X, with a bedside shelf."""
    M = Mx(x, y, 0, yaw)
    box(part, "frame", -L / 2, L / 2, -w / 2, w / 2, 0, 38, M)
    box(part, "soft", -L / 2 + 4, L / 2 - 4, -w / 2 + 4, w / 2 - 4, 38, 56, M)
    box(part, BU, -L / 2 + 8, -L / 2 + 45, -w / 2 + 10, w / 2 - 10, 56, 66, M)        # pillow
    box(part, BU, -L / 2 + 70, L / 2 - 6, -w / 2 + 2, w / 2 - 2, 52, 60, M)           # blanket
    if headboard:
        box(part, "frame", -L / 2 - 8, -L / 2, -w / 2 - 6, w / 2 + 6, 0, 110, M)
        box("Lights", LA, -L / 2 + 0.5, -L / 2 + 1.5, w / 2 - 22, w / 2 - 8, 92, 100, M)
    if collide:
        box("Collision", CL, -L / 2 - 8, L / 2, -w / 2, w / 2, 0, 66, M)


def table(part, x, y, sx, sy, h=74.0, slot="table", seats=0, collide=True):
    """Table with a pedestal, optional stools along the long sides."""
    box(part, slot, x - sx / 2, x + sx / 2, y - sy / 2, y + sy / 2, h - 5, h)
    box(part, "trim", x - sx / 2 + 6, x + sx / 2 - 6, y - sy / 2 + 6, y + sy / 2 - 6, h - 10, h - 5)
    cyl(part, "trim", x, y, 0, h - 10, 7, 8)
    cyl(part, "trim", x, y, 0, 3, min(sx, sy) * 0.35, 10)
    long_x = sx >= sy
    for k in range(seats):
        side = 1 if k % 2 else -1
        i = k // 2
        n = max(1, seats // 2)
        if long_x:
            xs = x - sx / 2 + (i + 0.5) * sx / n
            ys = y + side * (sy / 2 + 28)
        else:
            ys = y - sy / 2 + (i + 0.5) * sy / n
            xs = x + side * (sx / 2 + 28)
        cyl(part, "trim", xs, ys, 0, 42, 4, 8)
        cyl(part, "soft", xs, ys, 42, 48, 18, 12)
    if collide:
        col(x - sx / 2, x + sx / 2, y - sy / 2, y + sy / 2, 0, h)


def sofa(part, x, y, L, yaw, collide=True):
    """Long upholstered bench seat, back at local -Y... facing local +Y."""
    M = Mx(x, y, 0, yaw)
    box(part, "frame", -L / 2, L / 2, -45, 45, 0, 40, M)
    box(part, "soft", -L / 2 + 6, L / 2 - 6, -38, 42, 40, 52, M)
    box(part, "soft", -L / 2 + 6, L / 2 - 6, -45, -25, 40, 95, M)
    for s in (-1, 1):
        box(part, "frame", s * L / 2 - 12 * (s > 0), s * L / 2 + 12 * (s < 0), -45, 45, 0, 64, M)
    if collide:
        box("Collision", CL, -L / 2, L / 2, -45, 45, 0, 70, M)


def plant(part, x, y, z=0.0, r=30.0, h=110.0):
    """Planter with a leafy mass (luxury/colony/science)."""
    cyl(part, "frame", x, y, z, z + 45, r, 12)
    cyl(part, "trim", x, y, z + 45, z + 48, r + 2, 12)
    K[P(part)].ico(R(PLT), (x, y, z + 45 + h * 0.45), h * 0.5, 1)
    K[P(part)].ico(R(PLT), (x + r * 0.5, y - r * 0.3, z + 45 + h * 0.7), h * 0.3, 1)
    col(x - r, x + r, y - r, y + r, 0, z + 60)


def pipe_run(part, x0, x1, y, zs=((255, 8), (280, 6), (304, 4.5))):
    for (zz, r) in zs:
        pipe(part, PI, (x0, y, zz), (x1, y, zz), r, 10)


def tank(part, x, y, r, h, bands=3, collide=True, slot="body"):
    cyl(part, slot, x, y, 0, h, r, 20)
    cyl(part, "trim", x, y, h, h + 30, r * 0.8, 20)
    cyl(part, "trim", x, y, 0, 25, r + 8, 20)
    for k in range(bands):
        zb = h * (k + 1) / (bands + 1)
        ring(part, "frame", x, y, r - 2, r + 6, zb, zb + 12, 20)
    box(part, "accent", x - r * 0.3, x + r * 0.3, y - r - 3, y - r, h * 0.45, h * 0.52)
    if collide:
        cyl("Collision", CL, x, y, 0, h + 30, r + 8, 12)


def reactor(part, x, y, r, h, bands=((0.18, 0.24), (0.5, 0.57), (0.78, 0.84)), color=LA, conduits=8):
    """Reactor column: plinth, core, glowing bands with collars, conduits."""
    cyl(part, "body", x, y, 0, 70, r * 1.6, 32)
    ring(part, "accent", x, y, r * 1.6, r * 1.6 + 45, 0, 0.8, 32)
    cyl(part, "trim", x, y, 70, h, r, 32)
    for (fa, fb) in bands:
        za, zb = h * fa, h * fb
        cyl("Lights", color, x, y, za, zb, r + 3, 32)
        ring(part, "frame", x, y, r - 2, r + 30, za - 16, za, 32)
        ring(part, "frame", x, y, r - 2, r + 30, zb, zb + 16, 32)
    for k in range(conduits):
        t = math.radians(k * 360.0 / conduits + 22.5)
        pipe(part, PI, (x + (r + 50) * math.cos(t), y + (r + 50) * math.sin(t), 70),
             (x + (r + 50) * math.cos(t), y + (r + 50) * math.sin(t), h), max(6.0, r * 0.06), 10)
    cyl("Collision", CL, x, y, 0, h, r * 1.6 + 5, 24)


def holo_table(part, cx, cy, r, z0=0.0, grid=LG, screen=CO, collide=True):
    """Hex plot table with a vector-grid hologram frame above it."""
    cyl(part, "body", cx, cy, z0, z0 + 88, r * 0.7, 6)
    prism(part, "body", circle(cx, cy, r, 6), z0 + 88, z0 + 110)
    prism(part, "trim", circle(cx, cy, r + 6, 6), z0 + 102, z0 + 108)
    K[P("Lights")].prism_xy(screen, circle(cx, cy, r * 0.9, 6), z0 + 110, z0 + 111.2)
    g = r * 0.72
    n = 3
    for k in range(-n, n + 1):
        box("Lights", grid, cx + k * g / n - 0.8, cx + k * g / n + 0.8, cy - g, cy + g, z0 + 150, z0 + 151.5)
        box("Lights", grid, cx - g, cx + g, cy + k * g / n - 0.8, cy + k * g / n + 0.8, z0 + 150, z0 + 151.5)
    for a in range(6):
        t = math.radians(a * 60 + 30)
        pipe(part, "trim", (cx + r * 0.9 * math.cos(t), cy + r * 0.9 * math.sin(t), z0 + 110),
             (cx + g * math.cos(t), cy + g * math.sin(t), z0 + 152), 1.5, 6)
    if collide:
        cyl("Collision", CL, cx, cy, 0, z0 + 150, r + 12, 6)


def holo_sphere(part, cx, cy, r, z, color=LC):
    """Astrometrics globe: latitude/longitude rings of thin light on a pedestal."""
    cyl(part, "body", cx, cy, 0, z - r - 20, 40, 16)
    ring(part, "trim", cx, cy, 30, r * 0.6, z - r - 30, z - r - 20, 24)
    for k in range(-2, 3):
        zz = z + k * r * 0.38
        rr = math.sqrt(max(1.0, r * r - (k * r * 0.38) ** 2))
        ring("Lights", color, cx, cy, rr - 1.2, rr, zz - 0.8, zz + 0.8, 32)
    for a in range(0, 180, 30):
        t = math.radians(a)
        pts = []
        for i in range(17):
            ph = math.radians(-90 + i * 180 / 16)
            pts.append(Vector((cx + r * math.cos(ph) * math.cos(t), cy + r * math.cos(ph) * math.sin(t),
                               z + r * math.sin(ph))))
        for i in range(16):
            pipe("Lights", color, pts[i], pts[i + 1], 0.9, 4)
    K[P("Lights")].ico(color, (cx, cy, z), r * 0.12, 1)
    cyl("Collision", CL, cx, cy, 0, z + r, r + 10, 12)


def airlock_door(part, x, y, yaw, w=160.0, h=220.0):
    """Closed round-cornered pressure hatch set into a wall at (x, y), facing
    local +X, with a wheel, hazard surround and red/green lamp."""
    M = Mx(x, y, 0, yaw)
    box(part, "accent", 16, 18, -w / 2 - 20, w / 2 + 20, 0, h + 20, M)
    box(part, "frame", 18, 30, -w / 2, w / 2, 8, h, M)
    for k in range(8):
        t = math.radians(k * 45)
        c0 = M @ Vector((34, 32 * math.cos(t), h * 0.55 + 32 * math.sin(t)))
        c1 = M @ Vector((34, 0, h * 0.55))
        pipe(part, "trim", c0, c1, 2.0, 6)
    pts = [(32 * math.cos(math.radians(a)), h * 0.55 + 32 * math.sin(math.radians(a))) for a in range(0, 360, 30)]
    for i in range(len(pts)):
        q0, q1 = pts[i], pts[(i + 1) % len(pts)]
        pipe(part, "trim", M @ Vector((34, q0[0], q0[1])), M @ Vector((34, q1[0], q1[1])), 3.0, 6)
    box("Lights", S["warn"], 30, 32, -w / 2 - 14, -w / 2 - 2, h + 2, h + 14, M)
    box(part, "trim", 30, 34, w / 2 - 50, w / 2 - 20, 100, 150, M)
    box("Lights", LG, 34, 35, w / 2 - 46, w / 2 - 24, 128, 146, M)


# ----------------------------------------------------------------------------
# Props and fixtures (Tools/deck_props.py)
# ----------------------------------------------------------------------------
class _Ghost:
    """PT stand-in that only records walk blockers (for runtime-spawned fixtures)."""

    def __init__(self, M):
        self.M = M
        self.collide = True

    def col(self, x0, x1, y0, y1, z0, z1):
        box("Collision", CL, x0, x1, y0, y1, z0, z1, self.M)

    def at(self, x, y, z=0.0, yaw=0.0):
        return _Ghost(self.M @ Mx(x, y, z, yaw))

    def __getattr__(self, name):          # every drawing call is a no-op
        return lambda *a, **k: None


def prop(part, name, x, y, z=0.0, yaw=0.0, collide=True):
    """Bake a catalogue prop (deck_props.CATALOG) into a deck part. Wall props
    sit with their back on (x, y) facing yaw."""
    import deck_props as dp
    fn = dp.CATALOG[name][0]
    fn(dp.PT(P(part), x, y, z, yaw, lights=P("Lights"), collide=collide))


def fixture(name, arg, x, y, z=0.0, yaw=0.0, collide=True):
    """Runtime fixture: an X_<name>_<arg>_<N> socket (UE design cm, y and yaw
    flipped) that ASpaceshipInterior turns into an AInteriorFixture using
    SM_Prop_Fx_<name>; its walk blockers are baked into Collision here."""
    import deck_props as dp
    n = sum(1 for k in SOCKETS if k.startswith("X_"))
    SOCKETS["X_%s_%s_%03d" % (name, arg, n)] = [round(x, 1), round(-y, 1), round(z, 1), round(-yaw, 1)]
    if collide:
        dp.FIXTURES[name][0](_Ghost(Mx(x, y, z, yaw)))
