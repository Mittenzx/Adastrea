"""Small interior props (Blender 5.x): the dressing library for ship interiors.

Every prop is a function prop(t) that draws into a PT - a local frame at
(x, y, z, yaw) that knows which parts to draw into:

    t.part      visual mesh part
    t.lights    part for emissive bits (== t.part for a standalone prop mesh)
    t.collide   add walk blockers to "Collision" (deck dressing only)

The same functions serve two pipelines:
  * Tools/build_interior_props.py exports each one on its own as SM_Prop_<Name>
    (1:1 cm, pivot on the floor/wall at the origin) for hand placement, and
    the functional fixtures (monitors, consoles, ...) the C++ spawns.
  * Tools/build_ship_decks.py scatters them through the decks (baked into the
    room parts, drawn at the kit's 100x like everything else).

Materials: props use their own M_Prop_* slots (a plain PBR master driven by
the mesh UVs, Tools/import_interior_props.py), because a fire extinguisher is
red on every ship. UVs are the kit's 200 cm triplanar projection in design
units, so they come out the same at 1x and 100x. Screen faces (M_Prop_Live,
M_Prop_Term) get 0..1 UVs in fix_screen_uvs().

Local axes: +X is the prop's front (screens face +X, and a wall prop's back
sits on x=0). +Z is up, and z=0 is the floor, or the mounting height for wall props.
"""
import math

from mathutils import Vector, Matrix

import deck_kit as dk
from deck_kit import K, P, LG, LR, LA, LB, LC, LP, LW, VP, CL
from build_capital_interiors import circle

# prop material slots
RED, WHT, BLK, STE, ALU, RUB = ("M_Prop_RedPaint", "M_Prop_White", "M_Prop_BlackPlastic", "M_Prop_Steel",
                                "M_Prop_Brushed", "M_Prop_Rubber")
YEL, OLV, BLU, ORG, PAP, CRD = ("M_Prop_Yellow", "M_Prop_Olive", "M_Prop_Blue", "M_Prop_Orange",
                                "M_Prop_Paper", "M_Prop_Cardboard")
CER, FAB, COP, SCR, GRN, WOD = ("M_Prop_Ceramic", "M_Prop_Fabric", "M_Prop_Copper", "M_Prop_Screen",
                                "M_Prop_Green", "M_Prop_Wood")
LIVE, TERM = "M_Prop_Live", "M_Prop_Term"       # render-target screens (0..1 UVs)
GLS = VP


class PT:
    def __init__(self, part, x=0.0, y=0.0, z=0.0, yaw=0.0, lights=None, collide=False, s=1.0):
        self.part = part
        self.lights = lights or part
        self.collide = collide
        self.M = dk.Mx(x, y, z, yaw) @ Matrix.Scale(s, 4)
        self.s = s

    def at(self, x, y, z=0.0, yaw=0.0):
        """Child frame (for props built out of props)."""
        t = PT(self.part, lights=self.lights, collide=self.collide)
        t.M = self.M @ dk.Mx(x, y, z, yaw)
        t.s = self.s
        return t

    def w(self, v):
        return self.M @ Vector(v)

    def b(self, slot, x0, x1, y0, y1, z0, z1):
        dk.box(self.part, slot, x0, x1, y0, y1, z0, z1, self.M)

    def c(self, slot, cx, cy, z0, z1, r, segs=12):
        dk.cyl(self.part, slot, cx, cy, z0, z1, r, segs, self.M)

    def ring(self, slot, cx, cy, r0, r1, z0, z1, segs=16):
        verts_o = circle(cx, cy, r1, segs)
        verts_i = circle(cx, cy, r0, segs)
        for i in range(segs):
            a0, a1 = verts_o[i], verts_o[(i + 1) % segs]
            b0, b1 = verts_i[i], verts_i[(i + 1) % segs]
            K[P(self.part)].prism_xy(dk.R(slot), [b0, a0, a1, b1], z0, z1, self.M)

    def p(self, slot, a, b_, r, segs=8):
        dk.pipe(self.part, slot, self.w(a), self.w(b_), r * self.s, segs)

    def e(self, slot, prof, axis, a0, a1):
        dk.extrude(self.part, slot, prof, axis, a0, a1, self.M)

    def sph(self, slot, c, r, sub=1):
        K[P(self.part)].ico(dk.R(slot), tuple(self.w(c)), r * self.s, sub)

    def pl(self, color, a, b_, r, segs=4):
        """Emissive tube into the lights part."""
        dk.pipe(self.lights, color, self.w(a), self.w(b_), r * self.s, segs)

    def L(self, color, x0, x1, y0, y1, z0, z1):
        dk.box(self.lights, color, x0, x1, y0, y1, z0, z1, self.M)

    def Lc(self, color, cx, cy, z0, z1, r, segs=12):
        dk.cyl(self.lights, color, cx, cy, z0, z1, r, segs, self.M)

    def col(self, x0, x1, y0, y1, z0, z1):
        if self.collide:
            dk.box("Collision", CL, x0, x1, y0, y1, z0, z1, self.M)


# =============================================================================
# Galley / mess
# =============================================================================
def mug(t, color=WHT):
    t.c(color, 0, 0, 0, 10, 4.2, 12)
    t.c(BLK, 0, 0, 8.8, 9.2, 3.7, 12)
    t.e(color, [(3.8, 2.5), (6.5, 3.5), (6.5, 7.5), (3.8, 8.5), (3.8, 7.2), (5.3, 6.6), (5.3, 4.3), (3.8, 3.8)],
        'y', -0.8, 0.8)


def food_tray(t):
    t.b(STE, -22, 22, -16, 16, 0, 1.2)
    t.b(STE, -22, 22, -16, -14.8, 1.2, 3)
    t.b(STE, -22, 22, 14.8, 16, 1.2, 3)
    t.c(CER, -9, -3, 1.2, 3.2, 8, 12)
    t.sph(GRN, (-9, -3, 3.5), 4, 1)
    t.b(ORG, 3, 16, -12, -2, 1.2, 4)
    t.c(CER, 10, 8, 1.2, 6, 4.5, 10)
    mug(t.at(15, 9, 1.2), BLU)


def coffee_machine(t):
    """Counter-top brewer with a cup under the spout (faces +X)."""
    t.b(STE, -32, 0, -22, 22, 0, 8)
    t.b(BLK, -32, -14, -22, 22, 8, 52)
    t.b(STE, -32, 4, -22, 22, 52, 62)
    t.b(BLK, -4, 4, -12, 12, 44, 52)
    t.p(ALU, (0, 0, 44), (0, 0, 38), 1.5)
    mug(t.at(-2, 0, 8), WHT)
    t.b(SCR, 3.5, 4.2, 6, 18, 54, 60)
    t.L(LG, 4.2, 4.6, 7, 17, 55, 59)
    t.L(LR, 4.2, 4.6, -18, -15, 56, 58)
    t.c(GLS, -20, 12, 62, 72, 6, 12)
    t.col(-32, 4, -22, 22, 0, 72)


def water_dispenser(t):
    """Floor-standing water cooler (faces +X)."""
    t.b(WHT, -16, 16, -16, 16, 0, 100)
    t.b(BLK, 16, 17, -8, 8, 70, 90)
    t.b(STE, 14, 20, -9, 9, 60, 62)
    t.L(LB, 17, 17.5, -5, -1, 88, 90)
    t.L(LR, 17, 17.5, 1, 5, 88, 90)
    t.c(BLU, 0, 0, 100, 140, 14, 16)
    t.c(BLU, 0, 0, 140, 146, 6, 12)
    t.col(-16, 20, -16, 16, 0, 146)


def vending_machine(t):
    """Snack/drink vendor, glass front with shelves (faces +X)."""
    t.b(RED, -70, 0, -45, 45, 0, 190)
    t.b(BLK, -4, 2, -40, 12, 30, 180)
    t.b(GLS, 2, 3, -38, 10, 32, 178)
    for k in range(5):
        z = 42 + k * 28
        t.b(STE, -60, 0, -38, 10, z, z + 1.5)
        for j in range(5):
            y = -33 + j * 9.5
            if (j + k) % 2:
                t.c(YEL, -20, y + 3, z + 1.5, z + 16, 3.5, 8)        # can
            else:
                t.b(ORG, -25, -15, y, y + 7, z + 1.5, z + 14)        # snack box
    t.b(STE, -4, 3, 16, 38, 110, 150)
    t.b(SCR, 3, 3.5, 18, 36, 136, 148)
    t.L(LG, 3.5, 4, 19, 35, 137, 147)
    for r in range(3):
        for c_ in range(3):
            t.b(ALU, 3, 4.5, 19 + c_ * 6, 23 + c_ * 6, 115 + r * 6, 119 + r * 6)
    t.b(BLK, -2, 3, 16, 38, 20, 40)
    t.L(LW, 0, 0.5, -44, 44, 181, 186)
    t.b(WHT, 0, 0.6, -44, 44, 181, 188)
    t.col(-70, 4, -45, 45, 0, 190)


def bottle(t, color=GRN):
    t.c(color, 0, 0, 0, 20, 3.5, 10)
    t.c(color, 0, 0, 20, 24, 1.4, 8)
    t.c(BLK, 0, 0, 24, 26, 1.6, 8)
    t.b(PAP, 3.3, 3.6, -2, 2, 6, 14)


def ration_box(t):
    t.b(CRD, -10, 10, -6, 6, 0, 26)
    t.b(ORG, -10.2, 10.2, -6.2, 6.2, 16, 22)
    t.b(BLK, -10.3, -2, -6.3, -6, 4, 8)


def microwave(t):
    t.b(STE, -35, 0, -25, 25, 0, 30)
    t.b(GLS, 0, 0.5, -22, 8, 4, 26)
    t.b(BLK, 0, 0.8, 11, 23, 4, 26)
    t.L(LG, 0.8, 1, 13, 21, 20, 24)
    t.col(-35, 1, -25, 25, 0, 30)


def trash_bin(t):
    t.c(STE, 0, 0, 0, 70, 22, 16)
    t.c(BLK, 0, 0, 70, 74, 23, 16)
    t.c(YEL, 0, 0, 30, 36, 22.5, 16)
    t.col(-23, 23, -23, 23, 0, 74)


# =============================================================================
# Office / ops
# =============================================================================
def datapad(t, screen=LB):
    t.b(BLK, -9, 9, -12, 12, 0, 1.0)
    t.b(SCR, -8, 8, -11, 11, 1.0, 1.1)
    t.L(screen, -7, 7, -10, 10, 1.1, 1.2)


def clipboard(t):
    t.b(WOD, -11, 11, -16, 16, 0, 0.6)
    t.b(PAP, -10, 10, -15, 13, 0.6, 1.2)
    t.b(STE, -5, 5, 12, 15.5, 0.6, 2.2)
    for k in range(6):
        t.b(BLK, -8, 4 - (k % 3) * 2, 9 - k * 3.5, 9.5 - k * 3.5, 1.2, 1.3)


def keyboard(t):
    t.e(BLK, [(-8, 0), (8, 0), (8, 1.6), (-8, 3.2)], 'y', -22, 22)
    for r in range(4):
        for c_ in range(11):
            t.b(STE, -6.5 + r * 3.4, -4 + r * 3.4, -20 + c_ * 3.7, -17.5 + c_ * 3.7, 3.0 - r * 0.4, 3.8 - r * 0.4)


def binders(t, n=6):
    cols = [BLU, RED, BLK, YEL, OLV, WHT]
    for k in range(n):
        y = -n * 3.5 + k * 7 + 3.5
        t.b(cols[k % len(cols)], -14, 14, y - 3.2, y + 3.2, 0, 30 - (k % 2) * 2)
        t.b(WHT, 14, 14.3, y - 2, y + 2, 18, 24)
        t.c(STE, 14.2, y, 5, 5.5, 1.2, 6)


def book_stack(t):
    cols = [RED, BLU, OLV, CRD, BLK]
    z = 0.0
    for k in range(5):
        h = 3 + (k % 3)
        t.b(cols[k], -10 + k, 10 - k * 0.5, -7 + (k % 2), 7, z, z + h)
        z += h


def desk_lamp(t):
    t.c(BLK, 0, 0, 0, 2, 8, 14)
    t.p(STE, (0, 0, 2), (-4, 0, 30), 1)
    t.p(STE, (-4, 0, 30), (8, 0, 42), 1)
    t.e(BLK, [(4, 36), (14, 44), (18, 38), (10, 32)], 'y', -5, 5)
    t.L(LP, 10, 16, -4, 4, 33.5, 34.5)


def headset(t):
    t.e(BLK, [(-8, 0), (-8, 10), (-6, 16), (6, 16), (8, 10), (8, 0), (6, 0), (6, 10), (4.5, 14), (-4.5, 14), (-6, 10), (-6, 0)],
        'y', -2, 2)
    for s in (-1, 1):
        t.c(BLK, s * 7.5, 0, 0, 3, 4.5, 12)
        t.c(FAB, s * 7.5, 0, 3, 4, 4, 12)
    t.p(BLK, (7.5, 0, 3), (4, 8, 1), 0.6)


def portable_crt(t):
    t.b(OLV, -24, 0, -18, 18, 0, 30)
    t.b(OLV, -38, -24, -12, 12, 4, 26)
    t.b(BLK, 0, 1.5, -15, 15, 3, 27)
    t.L(LG, 1.5, 2, -12, 12, 6, 24)
    t.p(STE, (-20, -16, 30), (-20, 16, 30), 1.2)
    t.col(-38, 2, -18, 18, 0, 32)


def radio(t):
    t.b(OLV, -12, 0, -16, 16, 0, 20)
    t.b(BLK, 0, 0.8, -14, -2, 3, 17)
    for k in range(5):
        t.b(STE, 0.8, 1.2, -13, -3, 4 + k * 3, 5 + k * 3)
    t.b(SCR, 0, 0.6, 2, 13, 10, 16)
    t.L(LA, 0.6, 0.9, 3, 12, 11, 15)
    t.p(STE, (-6, 12, 20), (-8, 12, 60), 0.5)


def pinboard(t):
    """Wall cork board with notes and a schematic (back on x=0, faces +X)."""
    t.b(WOD, 0, 2, -45, 45, 0, 60)
    t.b(CRD, 2, 2.4, -42, 42, 3, 57)
    notes = [(-35, 42, WHT), (-18, 30, YEL), (-30, 15, PAP), (5, 44, PAP), (20, 25, YEL), (32, 40, WHT), (12, 10, BLU)]
    for (y, z, c_) in notes:
        t.b(c_, 2.4, 2.8, y - 6, y + 6, z - 7, z + 7)
        t.b(RED, 2.8, 3.4, y - 0.7, y + 0.7, z + 5, z + 6.4)          # pin
    t.b(BLU, 2.4, 2.8, -12, 6, 38, 54)
    t.b(WHT, 2.8, 2.9, -10, 4, 40, 52)


def wall_clock(t):
    """Wall clock with hands (back on x=0, faces +X)."""
    M = t.M @ Matrix.Rotation(math.radians(90), 4, 'Y')
    for (slot, r, x0, x1) in ((BLK, 15, 0, 3), (WHT, 13.5, 3, 3.3)):
        K[P(t.part)].prism_xy(dk.R(slot), circle(0, 0, r, 20), x0, x1, M @ Matrix.Rotation(math.radians(0), 4, 'Z'))
    for k in range(12):
        a = math.radians(k * 30)
        t.b(BLK, 3.3, 3.5, 11 * math.cos(a) - 0.5, 11 * math.cos(a) + 0.5, 11 * math.sin(a) - 1.2, 11 * math.sin(a) + 1.2)
    t.e(BLK, [(-0.6, 0), (0.6, 0), (0.3, 8), (-0.3, 8)], 'x', 3.5, 3.8)
    t.e(RED, [(-0.4, 0), (0.4, 0), (4.5, -9), (4.0, -9.4)], 'x', 3.8, 4.0)


def pen_cup(t):
    t.c(STE, 0, 0, 0, 10, 3.5, 12)
    for k, c_ in enumerate((RED, BLU, BLK, YEL)):
        a = math.radians(k * 90 + 20)
        t.p(c_, (1.5 * math.cos(a), 1.5 * math.sin(a), 2), (2.5 * math.cos(a), 2.5 * math.sin(a), 15), 0.45, 6)


def photo_frame(t):
    t.e(BLK, [(0, 0), (1, 0), (-3, 14), (-4, 14)], 'y', -6, 6)
    t.b(WOD, -0.5, 1, -8, 8, 0, 13)
    t.b(BLU, 1, 1.2, -6.5, 6.5, 1.5, 11.5)
    t.b(GRN, 1.2, 1.3, -6.5, 6.5, 1.5, 5)


def poster(t, color=BLU):
    t.b(ALU, 0, 1, -32, 32, 0, 90)
    t.b(color, 1, 1.2, -30, 30, 2, 88)
    t.b(WHT, 1.2, 1.3, -26, 26, 60, 80)
    t.b(YEL, 1.2, 1.3, -26, 10, 10, 18)
    t.b(BLK, 1.2, 1.3, -18, 18, 25, 55)


def chess_board(t):
    t.b(WOD, -18, 18, -18, 18, 0, 2)
    for i in range(8):
        for j in range(8):
            if (i + j) % 2:
                t.b(BLK, -16 + i * 4, -12 + i * 4, -16 + j * 4, -12 + j * 4, 2, 2.2)
    for k in range(10):
        x = -14 + (k % 8) * 4
        y = -14 if k < 8 else 10
        t.c(WHT if k % 3 else BLK, x, y + (k // 8) * 4, 2.2, 5 + (k % 2) * 2, 1.4, 8)


def playing_cards(t):
    """A dealt hand fanned across the table plus the deck."""
    t.b(WHT, -3.2, 3.2, -4.5, 4.5, 0, 1.2)
    t.b(RED, -3.2, 3.2, -4.5, 4.5, 1.2, 1.3)
    for k in range(5):
        c_ = t.at(k * 4 + 6, (k % 2) * 3, 0, k * 17)
        c_.b(WHT, -3.2, 3.2, -4.5, 4.5, 0, 0.2)


# =============================================================================
# Safety
# =============================================================================
def fire_extinguisher(t):
    """Wall bracket + red extinguisher (back on x=0, faces +X, z=0 = floor)."""
    t.b(STE, 0, 2, -8, 8, 40, 95)
    t.b(RED, 0.5, 2.2, -10, 10, 96, 110)
    t.b(WHT, 2.2, 2.4, -8, 8, 99, 107)
    t.c(RED, 11, 0, 45, 92, 8.5, 14)
    t.sph(RED, (11, 0, 92), 8.5, 1)
    t.c(BLK, 11, 0, 98, 104, 2.4, 8)
    t.b(BLK, 6, 18, -2, 2, 103, 106)
    t.p(BLK, (13, 5, 100), (16, 12, 60), 1.2)
    t.b(STE, 2, 6, -9, 9, 60, 63)
    t.b(STE, 2, 6, -9, 9, 80, 83)
    t.b(WHT, 19.4, 19.6, -4, 4, 60, 76)


def first_aid(t):
    """Wall first-aid cabinet (back on x=0, faces +X, z = mounting height)."""
    t.b(WHT, 0, 14, -22, 22, 0, 34)
    t.b(STE, 14, 14.5, -21, 21, 1, 33)
    t.b(RED, 14.5, 14.8, -3, 3, 8, 26)
    t.b(RED, 14.5, 14.8, -9, 9, 14, 20)
    t.b(BLK, 14.5, 16, 18, 20, 12, 22)


def oxygen_masks(t):
    """Emergency O2 locker with a drop-down mask (back on x=0, faces +X)."""
    t.b(YEL, 0, 10, -18, 18, 0, 30)
    t.b(BLK, 10, 10.4, -16, 16, 20, 28)
    t.b(YEL, 10, 10.3, -16, 16, 2, 18)
    t.p(RUB, (8, 5, 0), (10, 6, -30), 0.5)
    t.c(YEL, 10, 6, -36, -30, 4.5, 10)
    t.L(LG, 10.4, 10.6, -14, -8, 23, 25)


def hazard_cone(t):
    t.b(BLK, -17, 17, -17, 17, 0, 3)
    K[P(t.part)].prism_xy(dk.R(ORG), circle(0, 0, 13, 12), 3, 3.01, t.M)
    for k in range(10):
        z0 = 3 + k * 6
        r0 = 13 - k * 1.15
        t.c(ORG if k not in (4, 6) else WHT, 0, 0, z0, z0 + 6, r0, 12)
    t.col(-17, 17, -17, 17, 0, 60)


def exit_sign(t):
    """Lit overhead sign (back on x=0, faces +X)."""
    t.b(STE, 0, 8, -26, 26, 0, 16)
    t.L(LG, 8, 8.4, -23, 23, 2, 14)
    t.b(WHT, 8.4, 8.5, -16, -6, 5, 11)
    t.e(WHT, [(-2, 5), (6, 8), (-2, 11)], 'x', 8.4, 8.5)


def alarm_beacon(t):
    """Wall/ceiling-mount rotating beacon (base at z=0, dome up)."""
    t.c(BLK, 0, 0, 0, 5, 8, 14)
    t.Lc(LR, 0, 0, 5, 15, 6.5, 14)
    t.sph(GLS, (0, 0, 15), 6.5, 1)
    t.c(STE, 0, 0, 5, 15, 7, 6)


def safety_placard(t, color=YEL):
    t.b(color, 0, 0.5, -20, 20, 0, 28)
    t.b(BLK, 0.5, 0.6, -16, 16, 20, 24)
    t.e(BLK, [(-8, 4), (8, 4), (0, 17)], 'x', 0.5, 0.6)
    t.b(color, 0.6, 0.7, -1, 1, 8, 13)


# =============================================================================
# Engineering
# =============================================================================
def toolbox(t, color=RED):
    t.b(color, -22, 22, -11, 11, 0, 16)
    t.b(color, -22, 22, -11, 11, 16.5, 22)
    t.b(STE, -22.2, 22.2, -11.2, 11.2, 16, 16.5)
    t.p(BLK, (-10, 0, 22), (10, 0, 22), 1.4)
    for s in (-1, 1):
        t.p(BLK, (s * 10, 0, 22), (s * 10, 0, 26), 1.2)
    t.b(STE, 21.5, 23, -4, 4, 12, 18)


def tool_chest(t):
    """Rolling drawer cabinet (faces +X)."""
    t.b(RED, -30, 30, -45, 45, 8, 100)
    for k in range(6):
        z = 14 + k * 14
        t.b(STE, 30, 31.5, -40, 40, z + 10, z + 12)
        t.b(RED, 30, 30.6, -44, 44, z, z + 13)
    t.b(STE, -30, 30, -45, 45, 100, 103)
    for (x, y) in ((-24, -38), (-24, 38), (24, -38), (24, 38)):
        t.c(BLK, x, y, 0, 8, 4, 8)
    toolbox(t.at(0, 0, 103), BLU)
    t.col(-30, 32, -45, 45, 0, 125)


def gas_rack(t):
    """Three gas bottles chained in a floor rack (back on x=0, faces +X)."""
    t.b(STE, 0, 4, -40, 40, 0, 150)
    for k, c_ in enumerate((GRN, BLU, ORG)):
        y = -26 + k * 26
        t.c(c_, 14, y, 0, 125, 11, 14)
        t.sph(c_, (14, y, 125), 11, 1)
        t.c(STE, 14, y, 134, 140, 3, 8)
        t.b(BLK, 12, 16, y - 6, y + 6, 138, 142)
        t.b(WHT, 24.8, 25.2, y - 6, y + 6, 70, 100)
    for z in (60.0, 110.0):
        t.b(STE, 4, 27, -40, -38, z, z + 3)
        t.b(STE, 4, 27, 38, 40, z, z + 3)
        t.p(STE, (27, -40, z + 1.5), (27, 40, z + 1.5), 0.8, 6)
    t.col(0, 27, -40, 40, 0, 150)


def cable_spool(t):
    t.c(WOD, 0, 0, 0, 4, 40, 16)
    t.c(WOD, 0, 0, 56, 60, 40, 16)
    t.c(WOD, 0, 0, 4, 56, 12, 12)
    t.c(BLK, 0, 0, 6, 54, 30, 16)
    for k in range(5):
        t.c(ORG if k % 2 else BLK, 0, 0, 8 + k * 9.5, 16 + k * 9.5, 31, 16)
    t.col(-40, 40, -40, 40, 0, 60)


def junction_box(t):
    """Wall junction box with conduit and a hazard label (back on x=0)."""
    t.b(STE, 0, 12, -18, 18, 0, 34)
    t.b(STE, 12, 12.6, -17, 17, 1, 33)
    t.b(YEL, 12.6, 12.8, -10, 10, 20, 30)
    t.e(BLK, [(-4, 21.5), (4, 21.5), (0, 28.5)], 'x', 12.8, 12.9)
    for s in (-1, 1):
        t.c(STE, 6, s * 9, 34, 60, 2.2, 8)
    t.c(STE, 6, 0, -30, 0, 2.5, 8)
    t.L(LG, 12.6, 12.9, 12, 15, 4, 7)


def fuse_box(t):
    """Breaker panel with levers and status lights (back on x=0)."""
    t.b(STE, 0, 10, -30, 30, 0, 60)
    t.b(BLK, 10, 10.5, -27, 27, 3, 57)
    for r in range(3):
        for c_ in range(6):
            y = -22 + c_ * 8.8
            z = 10 + r * 16
            t.b(STE, 10.5, 12, y - 2.5, y + 2.5, z, z + 8)
            t.b(RED if (r + c_) % 5 == 0 else BLK, 12, 16, y - 1.2, y + 1.2, z + (5 if (r * 3 + c_) % 2 else 1), z + (7 if (r * 3 + c_) % 2 else 3))
            t.L(LG if (r + c_) % 5 else LR, 10.5, 10.8, y - 1, y + 1, z + 9.5, z + 11)
    t.b(YEL, 10.5, 10.8, -27, 27, 52, 56)


def valve_wheel(t):
    """Pipe stub with a hand wheel (pipe along Y, wheel facing +X)."""
    t.p(COP, (0, -40, 0), (0, 40, 0), 6, 12)
    t.p(STE, (0, -9, 0), (0, 9, 0), 9, 12)                         # valve body
    t.p(STE, (0, 0, 0), (16, 0, 0), 2, 8)
    M = t.M @ dk.Mx(18, 0, 0) @ Matrix.Rotation(math.radians(90), 4, 'Y')
    pts = circle(0, 0, 13, 16)
    for i in range(16):
        a = pts[i]; b_ = pts[(i + 1) % 16]
        dk.pipe(t.part, RED, M @ Vector((a[0], a[1], 0)), M @ Vector((b_[0], b_[1], 0)), 1.4, 6)
    for k in range(4):
        a = math.radians(k * 90 + 45)
        dk.pipe(t.part, RED, M @ Vector((0, 0, 0)), M @ Vector((13 * math.cos(a), 13 * math.sin(a), 0)), 1.0, 6)


def pressure_gauge(t):
    t.p(COP, (0, 0, 0), (8, 0, 0), 1.5, 8)
    M = t.M @ dk.Mx(8, 0, 0) @ Matrix.Rotation(math.radians(90), 4, 'Y')
    K[P(t.part)].prism_xy(dk.R(STE), circle(0, 0, 7, 16), 0, 3, M)
    K[P(t.part)].prism_xy(dk.R(WHT), circle(0, 0, 6, 16), -0.2, 0, M)
    t.b(RED, 7.7, 7.8, -0.4, 4.0, -0.4, 0.4)                       # needle


def wall_tools(t):
    """Pegboard strip with hanging wrenches and a hammer (back on x=0)."""
    t.b(STE, 0, 1.5, -40, 40, 0, 40)
    for k in range(5):
        y = -32 + k * 11
        L_ = 18 + k * 3
        t.b(ALU, 1.5, 3, y - 1.2, y + 1.2, 32 - L_, 32)
        t.b(ALU, 1.5, 3, y - 3, y + 3, 30, 35)                     # wrench jaw
    t.b(WOD, 1.5, 4, 22, 25, 5, 33)
    t.b(STE, 1.5, 5, 18, 30, 33, 37)


def parts_bin(t):
    """Stack of open parts bins (back on x=0, faces +X)."""
    cols = (BLU, YEL, RED)
    for r in range(3):
        for c_ in range(4):
            y = -30 + c_ * 20
            z = r * 14
            t.e(cols[(r + c_) % 3], [(0, z), (18, z), (18, z + 8), (14, z + 12), (0, z + 12)], 'y', y - 9, y + 9)
            t.b(WHT, 16, 16.3, y - 5, y + 5, z + 2, z + 6)
            t.sph(STE, (8, y, z + 11), 3, 1)
    t.col(0, 18, -40, 40, 0, 42)


def battery_pack(t):
    t.b(OLV, -20, 20, -14, 14, 0, 30)
    t.b(BLK, -20.3, 20.3, -14.3, 14.3, 24, 26)
    for s in (-1, 1):
        t.c(RED if s > 0 else BLK, s * 12, 0, 30, 34, 2.5, 8)
    t.b(YEL, 20, 20.3, -8, 8, 8, 18)
    t.L(LG, 20.3, 20.5, -6, -2, 20, 22)


def maint_drone(t):
    """Small hover maintenance drone on a charging stand."""
    t.c(STE, 0, 0, 0, 4, 22, 14)
    t.c(BLK, 0, 0, 4, 40, 3, 8)
    t.sph(YEL, (0, 0, 52), 12, 2)
    t.Lc(LB, 0, 0, 47, 48, 12.2, 16)
    for a in range(4):
        r = math.radians(a * 90 + 45)
        c_ = (22 * math.cos(r), 22 * math.sin(r), 52)
        t.p(BLK, (0, 0, 52), c_, 1.4)
        t.c(BLK, c_[0], c_[1], 50, 53, 8, 12)
    t.b(BLK, 10, 13, -3, 3, 50, 55)
    t.L(LR, 13, 13.3, -1.5, 1.5, 51.5, 53.5)
    t.col(-30, 30, -30, 30, 0, 60)


# =============================================================================
# Crew
# =============================================================================
def helmet(t, color=WHT):
    t.sph(color, (0, 0, 13), 13, 2)
    t.b(BLK, -12, 14, -14, 14, 0, 4)
    t.e(SCR, [(7, 8), (13.5, 10), (13.5, 20), (6, 22)], 'y', -9, 9)
    t.b(color, -3, 3, -14, 14, 22, 26)


def duffel_bag(t, color=OLV):
    M = t.M @ Matrix.Rotation(math.radians(90), 4, 'X')
    K[P(t.part)].prism_xy(dk.R(color), circle(0, 14, 14, 14), -30, 30, M)
    for s in (-1, 1):
        t.p(BLK, (s * 8, -14, 26), (s * 8, 14, 26), 1)
    t.p(BLK, (-8, 0, 28), (8, 0, 28), 1.3)
    t.b(BLK, -30.5, 30.5, -2, 2, 12, 16)


def boots(t):
    for s in (-1, 1):
        t.b(BLK, -12, 16, s * 6 - 5, s * 6 + 5, 0, 4)
        t.b(BLK, -12, 2, s * 6 - 4.5, s * 6 + 4.5, 4, 26)
        t.e(BLK, [(0, 4), (14, 4), (14, 8), (2, 12)], 'y', s * 6 - 4.5, s * 6 + 4.5)


def wall_hook_jacket(t, color=OLV):
    """Coat hook with a hanging jacket (back on x=0, hook at z=0)."""
    t.b(STE, 0, 2, -3, 3, -3, 3)
    t.p(STE, (2, 0, 0), (7, 0, 2), 0.8)
    t.e(color, [(2, -2), (14, -8), (15, -70), (1, -70), (1, -4)], 'y', -22, 22)
    for s in (-1, 1):
        t.e(color, [(2, -8), (12, -12), (12, -55), (3, -52)], 'y', s * 22, s * 28)
    t.b(BLK, 14.5, 15, -1, 1, -65, -12)
    t.b(YEL, 15, 15.2, 10, 18, -30, -26)


def guitar(t):
    """Acoustic guitar leaning on a wall (lean back toward -X)."""
    M = t.M @ Matrix.Rotation(math.radians(-12), 4, 'Y')
    for (cz, r) in ((22, 19), (48, 15)):
        K[P(t.part)].prism_xy(dk.R(WOD), circle(0, 0, r, 16), -5, 5, M @ dk.Mx(0, 0, cz) @ Matrix.Rotation(math.radians(90), 4, 'Y'))
    K[P(t.part)].prism_xy(dk.R(BLK), circle(0, 0, 5, 12), 5, 5.3, M @ dk.Mx(0, 0, 38) @ Matrix.Rotation(math.radians(90), 4, 'Y'))
    dk.box(t.part, WOD, -2, 2, -3, 3, 62, 100, M)
    dk.box(t.part, BLK, -2.5, 2.5, -4, 4, 100, 112, M)
    for k in range(6):
        dk.box(t.part, STE, 5.2, 5.3, -2.5 + k, -2.3 + k, 20, 102, M)


def dumbbells(t):
    for s in (-1, 1):
        y = s * 12
        t.p(STE, (-14, y, 6), (14, y, 6), 1.5)
        t.p(BLK, (-14, y, 6), (-9, y, 6), 6, 10)
        t.p(BLK, (9, y, 6), (14, y, 6), 6, 10)


def plant_pot(t, h=40.0):
    t.c(CER, 0, 0, 0, 16, 10, 14)
    t.c(CRD, 0, 0, 14.5, 15, 9, 14)
    for k, (dx, dy, r) in enumerate(((0, 0, 9), (4, -3, 7), (-4, 3, 6), (1, 4, 5))):
        t.sph(GRN, (dx, dy, 16 + h * 0.3 + k * 4), r, 1)


def laundry_basket(t):
    t.b(BLU, -22, 22, -16, 16, 0, 2)
    for s in (-1, 1):
        t.b(BLU, -22, 22, s * 16 - 1, s * 16 + 1, 2, 34)
        t.b(BLU, s * 22 - 1, s * 22 + 1, -16, 16, 2, 34)
    for k in range(4):
        t.b(BLK, -20, 20, -17.2, -16.8, 8 + k * 6, 10 + k * 6)
    t.sph(FAB, (0, 0, 28), 15, 1)
    t.sph(WHT, (8, 4, 32), 10, 1)


# =============================================================================
# Medical
# =============================================================================
def iv_stand(t):
    for a in range(5):
        r = math.radians(a * 72)
        t.p(STE, (0, 0, 8), (24 * math.cos(r), 24 * math.sin(r), 2), 1.2)
        t.c(BLK, 24 * math.cos(r), 24 * math.sin(r), 0, 3, 3, 8)
    t.p(STE, (0, 0, 8), (0, 0, 190), 1.2)
    t.p(STE, (-14, 0, 188), (14, 0, 188), 0.8)
    for s in (-1, 1):
        t.b(GLS, s * 12 - 5, s * 12 + 5, -2, 2, 160, 184)
        t.p(GLS, (s * 12, 0, 160), (s * 4, 0, 60), 0.4)
    t.col(-24, 24, -24, 24, 0, 60)


def med_cabinet(t):
    """Glass-front wall cabinet with supplies (back on x=0, faces +X)."""
    t.b(WHT, 0, 30, -40, 40, 0, 70)
    t.b(GLS, 30, 30.5, -38, 38, 2, 68)
    for k in range(3):
        z = 4 + k * 22
        t.b(WHT, 2, 29, -38, 38, z, z + 1)
        for j in range(7):
            y = -33 + j * 10
            (t.c(WHT if j % 2 else BLU, 15, y, z + 1, z + 12 + (j % 3) * 3, 3.5, 10) if (j + k) % 3 else
             t.b(RED if k == 1 else ORG, 8, 22, y - 3.5, y + 3.5, z + 1, z + 9))
    t.b(RED, 30.5, 30.8, -3, 3, 60, 66)


def specimen_jar(t, glow=LC):
    t.c(STE, 0, 0, 0, 4, 9, 12)
    t.c(GLS, 0, 0, 4, 34, 8, 12)
    t.Lc(glow, 0, 0, 5, 26, 7, 12)
    t.sph(GRN, (0, 0, 15), 4, 1)
    t.c(STE, 0, 0, 34, 38, 9, 12)


def defibrillator(t):
    t.b(ORG, 0, 12, -16, 16, 0, 30)
    t.b(SCR, 12, 12.3, -10, 10, 14, 26)
    t.L(LG, 12.3, 12.5, -8, 8, 16, 24)
    for s in (-1, 1):
        t.b(BLK, 12, 16, s * 10 - 4, s * 10 + 4, 3, 10)


# =============================================================================
# Cargo
# =============================================================================
def barrel(t, color=BLU):
    t.c(color, 0, 0, 0, 88, 29, 18)
    for z in (0.0, 29.0, 58.0, 86.0):
        t.c(STE, 0, 0, z, z + 2.5, 30, 18)
    t.c(color, 0, 0, 88, 89, 27, 18)
    t.c(BLK, 12, 0, 89, 90.5, 3, 8)
    t.b(WHT, 28.6, 29.8, -8, 8, 35, 55)
    t.col(-30, 30, -30, 30, 0, 90)


def small_crate(t, color=OLV):
    t.b(color, -25, 25, -20, 20, 0, 36)
    for s in (-1, 1):
        t.b(STE, s * 25 - 2, s * 25 + 2, -21, 21, 0, 36)
        t.b(BLK, -12, 12, s * 20 - 0.3, s * 20 + 0.3, 20, 28)
    t.b(STE, -26, 26, -21, 21, 34, 36)
    t.b(YEL, -8, 8, -20.4, -20, 8, 14)
    t.col(-26, 26, -21, 21, 0, 36)


def pallet_boxes(t):
    t.b(WOD, -60, 60, -50, 50, 0, 3)
    for y in (-45, 0, 45):
        t.b(WOD, -60, 60, y - 5, y + 5, 3, 13)
    t.b(WOD, -60, 60, -50, 50, 13, 16)
    for i in range(2):
        for j in range(2):
            h = 40 - ((i + j) % 2) * 8
            t.b(CRD, -58 + i * 59, -1 + i * 59, -48 + j * 49, -1 + j * 49, 16, 16 + h)
            t.b(YEL, -58 + i * 59, -1 + i * 59, -48 + j * 49 + 20, -48 + j * 49 + 27, 16 + h, 16.3 + h)
    t.b(CRD, -30, 30, -25, 25, 56, 86)
    for s in (-1, 1):
        t.b(BLK, -61, 61, s * 30 - 2, s * 30 + 2, 16, 87)
    t.col(-60, 60, -50, 50, 0, 87)


def canister_row(t, n=4):
    for k in range(n):
        y = -n * 12 + 12 + k * 24
        t.b(YEL if k % 2 else RED, -8, 8, y - 10, y + 10, 0, 34)
        t.c(BLK, 0, y - 5, 34, 38, 2.5, 8)
        t.b(BLK, -6, 6, y - 2, y + 2, 36, 38)


# =============================================================================
# Tech / wall
# =============================================================================
def wall_speaker(t):
    t.b(BLK, 0, 12, -14, 14, 0, 22)
    t.b(STE, 12, 12.5, -12, 12, 2, 20)
    for k in range(5):
        t.b(BLK, 12.5, 12.8, -10, 10, 4 + k * 3.5, 5.5 + k * 3.5)


def wall_fan(t):
    """Wall vent fan in a housing (back on x=0, faces +X)."""
    t.b(STE, 0, 10, -26, 26, 0, 52)
    M = t.M @ dk.Mx(10, 0, 26) @ Matrix.Rotation(math.radians(90), 4, 'Y')
    K[P(t.part)].prism_xy(dk.R(BLK), circle(0, 0, 21, 20), 0, 1, M)
    for k in range(5):
        a = math.radians(k * 72)
        dk.extrude(t.part, STE, [(0, 0), (18 * math.cos(a), 18 * math.sin(a)), (18 * math.cos(a + 0.5), 18 * math.sin(a + 0.5))],
                   'z', 1, 3, M)
    for k in range(-4, 5):
        dk.box(t.part, STE, 10.5, 12, -23, 23, 26 + k * 5 - 0.6, 26 + k * 5 + 0.6, t.M)


def intercom_panel(t):
    """Static wall intercom (back on x=0, faces +X)."""
    t.b(STE, 0, 6, -12, 12, 0, 34)
    t.b(BLK, 6, 6.3, -9, 9, 20, 30)
    for k in range(4):
        t.b(STE, 6.3, 6.6, -8, 8, 21 + k * 2.2, 22 + k * 2.2)
    t.b(RED, 6, 9, -4, 4, 8, 14)
    t.L(LG, 6.3, 6.6, 6, 9, 4, 7)


def server_cabinet(t):
    """Short electronics rack with blinking status lights (faces +X)."""
    t.b(BLK, -60, 0, -30, 30, 0, 120)
    t.b(STE, -1, 0.5, -29, 29, 2, 118)
    for r in range(8):
        z = 8 + r * 13.5
        t.b(BLK, 0.5, 1.5, -27, 27, z, z + 11)
        for k in range(6):
            t.L((LG, LB, LA, LG, LR, LG)[(r + k) % 6], 1.5, 1.8, -24 + k * 4, -22 + k * 4, z + 4, z + 6)
    t.col(-60, 2, -30, 30, 0, 120)


def holo_projector(t, color=LC):
    """Small table-top hologram emitter with a floating wire globe."""
    t.c(STE, 0, 0, 0, 6, 12, 16)
    t.Lc(color, 0, 0, 6, 7, 8, 16)
    for a in range(0, 180, 45):
        pts = []
        for i in range(13):
            ph = math.radians(-90 + i * 15)
            pts.append((10 * math.cos(ph) * math.cos(math.radians(a)), 10 * math.cos(ph) * math.sin(math.radians(a)),
                        24 + 10 * math.sin(ph)))
        for i in range(12):
            t.pl(color, pts[i], pts[i + 1], 0.35)


# =============================================================================
# Functional fixtures (exported as SM_Prop_Fx_*; ASpaceshipInterior spawns them
# from X_ sockets and drives their screens)
# =============================================================================
def fx_monitor_wall(t):
    """Wall-mounted exterior monitor: 16:9 screen (M_Prop_Live) on an arm
    bracket, back on x=0, screen centre at z=0."""
    t.b(STE, 0, 4, -10, 10, -14, 14)
    t.p(STE, (4, 0, 0), (10, 0, 0), 3)
    t.b(BLK, 10, 16, -58, 58, -35, 35)
    t.b(STE, 10, 16.5, -58, 58, 33, 35)
    t.b(LIVE, 16, 16.4, -54, 54, -30.4, 30.4)
    t.b(BLK, 16, 17, -58, 58, -35, -31)
    t.L(LR, 17, 17.3, 50, 54, -34, -32)
    t.b(WHT, 17, 17.1, -54, -30, -34, -32)


def fx_monitor_hang(t):
    """Ceiling-hung monitor (z=0 is the ceiling, screen faces +X)."""
    t.p(STE, (0, -30, 0), (0, -30, -45), 1.5)
    t.p(STE, (0, 30, 0), (0, 30, -45), 1.5)
    t.b(BLK, -6, 0, -50, 50, -105, -45)
    t.b(LIVE, 0, 0.4, -46, 46, -101, -49)


def fx_monitor_desk(t):
    """Desk monitor on a stand (z=0 is the desk top, screen faces +X)."""
    t.b(STE, -12, 4, -12, 12, 0, 1.5)
    t.p(STE, (-6, 0, 1.5), (-6, 0, 22), 2.2)
    t.e(BLK, [(-8, 18), (0, 18), (-2, 54), (-10, 54)], 'y', -32, 32)
    t.e(LIVE, [(0, 20), (0.4, 20), (-1.6, 52), (-2, 52)], 'y', -29, 29)


def fx_terminal(t):
    """Standing console terminal: pedestal, keyboard shelf, angled screen
    (M_Prop_Term). The operator stands at +X."""
    t.b(STE, -30, 10, -30, 30, 0, 8)
    t.e(BLK, [(-30, 8), (-2, 8), (-2, 80), (-30, 80)], 'y', -26, 26)
    t.e(STE, [(-10, 88), (26, 92), (26, 96), (-10, 94)], 'y', -28, 28)          # keyboard shelf
    keyboard(t.at(12, 0, 92.5))
    t.e(BLK, [(-26, 94), (-10, 94), (4, 150), (-12, 154)], 'y', -30, 30)
    t.e(TERM, [(-9.6, 97), (-9.0, 97), (3.4, 147), (2.8, 147)], 'y', -27, 27)
    t.L(LG, 0, 3, 24, 27, 90, 92)
    t.L(LA, 0, 3, -27, -24, 90, 92)
    t.b(YEL, -30, 10, -30, 30, 0, 1)
    t.col(-30, 26, -30, 30, 0, 150)


def fx_wall_panel(t):
    """Wall-mounted console: screen (M_Prop_Term) over a button deck
    (back on x=0, screen centre ~160 cm)."""
    t.b(STE, 0, 8, -40, 40, 100, 200)
    t.e(BLK, [(8, 104), (22, 108), (22, 116), (8, 118)], 'y', -38, 38)
    for r in range(2):
        for c_ in range(8):
            t.b((RED, YEL, BLU, WHT)[(r + c_) % 4], 12 + r * 4, 15 + r * 4, -32 + c_ * 8, -27 + c_ * 8, 110.5 + r * 2, 112 + r * 2)
    t.b(BLK, 8, 10, -36, 36, 124, 194)
    t.b(TERM, 10, 10.4, -33, 33, 127, 191)
    t.L(LG, 8, 8.4, 34, 38, 196, 198)


def fx_alert_button(t):
    """Red-alert pedestal: mushroom button under a flip cover."""
    t.c(STE, 0, 0, 0, 5, 20, 14)
    t.c(BLK, 0, 0, 5, 95, 7, 12)
    t.e(YEL, [(-14, 95), (14, 95), (10, 108), (-10, 108)], 'y', -14, 14)
    t.b(BLK, -12, 12, -12, 12, 108, 109)
    t.c(RED, 0, 0, 109, 113, 4, 12)
    t.Lc(LR, 0, 0, 113, 117, 7, 16)
    t.e(GLS, [(-12, 109), (10, 109), (-12, 124)], 'y', -12, 12)
    t.col(-20, 20, -20, 20, 0, 110)


def fx_light_switch(t):
    """Wall lighting panel with a big rocker (back on x=0)."""
    t.b(STE, 0, 4, -8, 8, 0, 20)
    t.b(WHT, 4, 6, -4, 4, 4, 16)
    t.b(BLK, 4, 4.3, -7, 7, 17, 19)
    t.L(LA, 4.3, 4.5, -5, 5, 17.5, 18.5)


def fx_intercom(t):
    """Interactive intercom: speaker grille, call button, tiny screen."""
    intercom_panel(t)
    t.b(BLK, 6, 6.3, -9, 9, 12, 17)
    t.b(TERM, 6.3, 6.5, -8, 8, 12.5, 16.5)


def fx_coffee(t):
    """Interactive coffee station: brewer on a counter with cups."""
    t.b(STE, -40, 0, -45, 45, 0, 88)
    t.b(WHT, -40, 2, -46, 46, 88, 92)
    coffee_machine(t.at(-4, -12, 92))
    for k in range(4):
        mug(t.at(-20 + (k % 2) * 8, 22 + (k // 2) * 9, 92), (WHT, BLU, RED, OLV)[k])
    t.b(BLK, -40, 0, 30, 44, 92, 120)
    t.col(-40, 2, -46, 46, 0, 120)


def fx_map_table(t):
    """Round nav table with a holo globe (the operator can stand anywhere)."""
    t.c(STE, 0, 0, 0, 80, 30, 16)
    t.c(BLK, 0, 0, 80, 90, 55, 24)
    t.c(TERM, 0, 0, 90, 90.3, 50, 24)
    t.ring(STE, 0, 0, 50, 56, 90, 93, 24)
    holo_projector(t.at(0, 0, 90))
    t.col(-56, 56, -56, 56, 0, 95)


# -----------------------------------------------------------------------------
# Catalogue: name -> (function, placement, footprint note)
#   placement: "floor" (stands on z=0), "table" (on a surface), "wall" (back on x=0)
# -----------------------------------------------------------------------------
CATALOG = {
    # galley / mess
    "Mug": (mug, "table"), "FoodTray": (food_tray, "table"), "CoffeeMachine": (coffee_machine, "table"),
    "WaterDispenser": (water_dispenser, "floor"), "VendingMachine": (vending_machine, "floor"),
    "Bottle": (bottle, "table"), "RationBox": (ration_box, "table"), "Microwave": (microwave, "table"),
    "TrashBin": (trash_bin, "floor"),
    # office / ops
    "Datapad": (datapad, "table"), "Clipboard": (clipboard, "table"), "Keyboard": (keyboard, "table"),
    "Binders": (binders, "table"), "BookStack": (book_stack, "table"), "DeskLamp": (desk_lamp, "table"),
    "Headset": (headset, "table"), "PortableCRT": (portable_crt, "table"), "Radio": (radio, "table"),
    "Pinboard": (pinboard, "wall"), "WallClock": (wall_clock, "wall"), "PenCup": (pen_cup, "table"),
    "PhotoFrame": (photo_frame, "table"), "Poster": (poster, "wall"), "ChessBoard": (chess_board, "table"),
    "PlayingCards": (playing_cards, "table"),
    # safety
    "FireExtinguisher": (fire_extinguisher, "wall"), "FirstAidKit": (first_aid, "wall"),
    "OxygenMasks": (oxygen_masks, "wall"), "HazardCone": (hazard_cone, "floor"), "ExitSign": (exit_sign, "wall"),
    "AlarmBeacon": (alarm_beacon, "wall"), "SafetyPlacard": (safety_placard, "wall"),
    # engineering
    "Toolbox": (toolbox, "floor"), "ToolChest": (tool_chest, "floor"), "GasRack": (gas_rack, "wall"),
    "CableSpool": (cable_spool, "floor"), "JunctionBox": (junction_box, "wall"), "FuseBox": (fuse_box, "wall"),
    "ValveWheel": (valve_wheel, "wall"), "PressureGauge": (pressure_gauge, "wall"), "WallTools": (wall_tools, "wall"),
    "PartsBins": (parts_bin, "wall"), "BatteryPack": (battery_pack, "floor"), "MaintDrone": (maint_drone, "floor"),
    # crew
    "Helmet": (helmet, "table"), "DuffelBag": (duffel_bag, "floor"), "Boots": (boots, "floor"),
    "JacketHook": (wall_hook_jacket, "wall"), "Guitar": (guitar, "floor"), "Dumbbells": (dumbbells, "floor"),
    "PlantPot": (plant_pot, "table"), "LaundryBasket": (laundry_basket, "floor"),
    # medical
    "IVStand": (iv_stand, "floor"), "MedCabinet": (med_cabinet, "wall"), "SpecimenJar": (specimen_jar, "table"),
    "Defibrillator": (defibrillator, "wall"),
    # cargo
    "Barrel": (barrel, "floor"), "SmallCrate": (small_crate, "floor"), "PalletBoxes": (pallet_boxes, "floor"),
    "Canisters": (canister_row, "floor"),
    # tech / wall
    "WallSpeaker": (wall_speaker, "wall"), "WallFan": (wall_fan, "wall"), "Intercom": (intercom_panel, "wall"),
    "ServerCabinet": (server_cabinet, "floor"), "HoloProjector": (holo_projector, "table"),
}

# Functional fixtures: name -> (function, placement). The C++ picks the mesh
# SM_Prop_Fx_<Name> by fixture kind (see ASpaceshipInterior::SpawnFixtures).
FIXTURES = {
    "MonitorWall": (fx_monitor_wall, "wall"), "MonitorHang": (fx_monitor_hang, "ceiling"),
    "MonitorDesk": (fx_monitor_desk, "table"), "Terminal": (fx_terminal, "floor"),
    "WallPanel": (fx_wall_panel, "wall"), "AlertButton": (fx_alert_button, "floor"),
    "LightSwitch": (fx_light_switch, "wall"), "Intercom": (fx_intercom, "wall"),
    "Coffee": (fx_coffee, "floor"), "MapTable": (fx_map_table, "floor"),
}

SCREEN_SLOTS = (LIVE, TERM)


def fix_screen_uvs(ob):
    """Screen faces get 0..1 UVs across the screen (u along the face's
    horizontal, v bottom->top) so a render target fills them. Screens are thin
    slabs: only the faces pointing the same way as the biggest screen face are
    the display; the back and edges are handed to black plastic."""
    import bmesh
    me = ob.data
    names = [m.name.split(".")[0] if m else "" for m in me.materials]
    idx = [i for i, n in enumerate(names) if n in SCREEN_SLOTS]
    if not idx:
        return
    if BLK not in names:
        mat = bpy_material(BLK)
        me.materials.append(mat)
        names.append(BLK)
    blk = names.index(BLK)
    bm = bmesh.new()
    bm.from_mesh(me)
    uv = bm.loops.layers.uv.active
    for mi in idx:
        faces = [f for f in bm.faces if f.material_index == mi]
        if not faces:
            continue
        # display direction = normal of the largest coplanar group (triangulated quads come in pairs)
        big = max(faces, key=lambda f: f.calc_area())
        n = big.normal.copy()
        front = [f for f in faces if f.normal.dot(n) > 0.95]
        for f in faces:
            if f not in front:
                f.material_index = blk
        up = Vector((0, 0, 1))
        if abs(n.dot(up)) > 0.9:            # flat (table) screen: "up" is local +X
            up = Vector((1, 0, 0))
        right = up.cross(n).normalized()
        upv = n.cross(right).normalized()
        pts = [(l.vert.co.dot(right), l.vert.co.dot(upv)) for f in front for l in f.loops]
        u0, u1 = min(p[0] for p in pts), max(p[0] for p in pts)
        v0, v1 = min(p[1] for p in pts), max(p[1] for p in pts)
        for f in front:
            for l in f.loops:
                co = l.vert.co
                l[uv].uv = ((co.dot(right) - u0) / max(1e-4, u1 - u0), (co.dot(upv) - v0) / max(1e-4, v1 - v0))
    bm.to_mesh(me)
    bm.free()


def bpy_material(name):
    import bpy
    return bpy.data.materials.get(name) or bpy.data.materials.new(name)
