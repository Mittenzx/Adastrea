"""Room, bay and bridge builders for the full-deck ship interiors.

Tools/build_ship_decks.py lays a ship out stern-to-bow as segments and calls
these to furnish each space. Everything draws through deck_kit, so the ship's
style (military / industrial / clean / luxury / stealth / ...) decides the
materials.

Three kinds of space:
  * Rm  - a crew room off a spine corridor. It is side-agnostic: u runs along X
          (x0..x1), v runs from the spine wall (v=0, where the door is) to the
          hull wall (v=D). rm.y(v) maps back to world y and rm.yaw(a) turns a
          local yaw (0 = +u, 90 = toward the hull) into a world yaw.
  * Bay - a full-width space (hold, hangar, engineering...). World x0..x1,
          y0..y1, doors at y ~ 0 on the x0 and/or x1 walls.
  * the bridge, always the bow segment; it writes the Entry/Seat sockets.
Builders keep the door zones clear and add their own walk blockers.
"""
import math

from mathutils import Vector

import deck_kit as dk
from deck_kit import (box, col, solid, prism, extrude, cyl, ring, pipe, seg_box, light, fill, railing,
                      stairs, platform, crt, flatscreen, console, chair, pilot_seat, ceiling_panel,
                      cage_lamp, sconce, flood, crate, container, locker_bank, shelf_rack,
                      bunk_stack, bed, table, sofa, plant, tank, reactor, holo_table, holo_sphere,
                      airlock_door, window_band, ribs, Mx, K, P, R, S, view,
                      VP, PI, BU, GA, GR, LR, LA, LG, LB, LC, LP, LW, CO, CL, PLT, CRP, BRS)
from build_capital_interiors import circle


class Rm:
    def __init__(self, part, x0, x1, yn, yf, h, door_u=None, door_w=200.0, opts=None):
        self.part, self.x0, self.x1, self.yn, self.yf, self.h = part, x0, x1, yn, yf, h
        self.sg = 1 if yf > yn else -1
        self.L = x1 - x0
        self.D = abs(yf - yn)
        self.door_u = door_u
        self.door_w = door_w
        self.opts = opts or {}
        self.cx = (x0 + x1) / 2

    def y(self, v):
        return self.yn + self.sg * v

    def yaw(self, a):
        return self.sg * a

    def B(self, slot, u0, u1, v0, v1, z0, z1, part=None, collide=False):
        box(part or self.part, slot, u0, u1, self.y(v0), self.y(v1), z0, z1)
        if collide:
            col(u0, u1, self.y(v0), self.y(v1), z0, z1)

    def C(self, u0, u1, v0, v1, z0, z1):
        col(u0, u1, self.y(v0), self.y(v1), z0, z1)

    def near_door(self, u0, u1, pad=40.0):
        if self.door_u is None:
            return False
        return u1 > self.door_u - self.door_w / 2 - pad and u0 < self.door_u + self.door_w / 2 + pad

    def spans(self, u0, u1, w, gap=0.0, pad=40.0):
        """Slots of width w along u0..u1 that stay clear of the door."""
        out = []
        u = u0
        while u + w <= u1 + 0.1:
            if not self.near_door(u, u + w, pad):
                out.append(u)
                u += w + gap
            else:
                u += 20
        return out


class Bay:
    def __init__(self, part, x0, x1, y0, y1, h, opts=None, aft_door=True, fwd_door=True, door_w=300.0):
        self.part, self.x0, self.x1, self.y0, self.y1, self.h = part, x0, x1, y0, y1, h
        self.opts = opts or {}
        self.aft_door, self.fwd_door, self.door_w = aft_door, fwd_door, door_w
        self.L, self.W = x1 - x0, y1 - y0
        self.cx, self.cy = (x0 + x1) / 2, (y0 + y1) / 2


# ----------------------------------------------------------------------------
# Common dressing
# ----------------------------------------------------------------------------
def dress(rm, n=None, rad=None, along=True, panel=170.0, extra_fill=0):
    """Ceiling tube panels over the room + fill-light sockets."""
    h = rm.h
    n = n or max(1, int(round(rm.L / 450.0)))
    rows = 1 if rm.D < 520 else 2
    for i in range(n):
        u = rm.x0 + (i + 0.5) * rm.L / n
        for j in range(rows):
            v = rm.D * (j + 1) / (rows + 1)
            ceiling_panel(rm.part, u, rm.y(v), h, 0.0 if along else 90.0, panel)
    nl = max(1, int(round(rm.L / 700.0))) + extra_fill
    for i in range(nl):
        u = rm.x0 + (i + 0.5) * rm.L / nl
        fill(rad or max(6, int(min(10, 3 + max(rm.L / nl, rm.D) / 150))), u, rm.y(rm.D / 2), h - 40)


def wall_panels(rm, v, z0=90, z1=None, every=260.0, part=None):
    """Recessed wall panelling on the end walls (u = x0 / x1) of a room."""
    z1 = z1 or min(rm.h - 60, 230)
    for u, s in ((rm.x0, 1), (rm.x1, -1)):
        for k in range(max(1, int((rm.D - 120) // every))):
            v0 = 60 + k * every
            rm.B("frame", u, u + s * 4, v0 + 10, v0 + every - 10, z0, z1, part=part)


# ----------------------------------------------------------------------------
# Block rooms
# ----------------------------------------------------------------------------
def rm_bunks(rm):
    p = rm.part
    tiers = 3 if rm.h >= 320 else 2
    for u in rm.spans(rm.x0 + 20, rm.x1 - 20, 210, gap=28, pad=0):
        bunk_stack(p, u, u + 210, *sorted((rm.y(rm.D - 94), rm.y(rm.D - 2))), tiers=tiers)
    if rm.D > 560:
        vm = rm.D * 0.52
        for u in rm.spans(rm.x0 + 260, rm.x1 - 260, 210, gap=28, pad=0):
            bunk_stack(p, u, u + 210, *sorted((rm.y(vm - 92), rm.y(vm))), tiers=tiers)
            bunk_stack(p, u, u + 210, *sorted((rm.y(vm), rm.y(vm + 92))), tiers=tiers)
    # lockers on the spine wall either side of the door
    for (a, b) in ((rm.x0 + 10, (rm.door_u or rm.x1) - rm.door_w / 2 - 50),
                   ((rm.door_u or rm.x1) + rm.door_w / 2 + 50, rm.x1 - 10)):
        if b - a > 120:
            locker_bank(p, a, b, rm.y(0), 55, rm.sg)
    crate(p, rm.x0 + 90, rm.y(rm.D * 0.3), 0, 80, 50, 45, lid="trim")
    cage_lamp(p, rm.x1, rm.y(rm.D * 0.4), rm.h - 90, 180.0)
    dress(rm)


def rm_galley(rm):
    """Small-ship galley: back counter, hood, fridge, one table with stools."""
    p = rm.part
    vf = rm.D
    a, b = rm.x0 + 20, rm.x1 - 20
    rm.B(GA, a, b, vf - 70, vf, 0, 90, collide=True)
    rm.B("body", a, b, vf - 74, vf, 90, 94)
    extrude(p, "frame", [(rm.y(vf - 80), 230), (rm.y(vf), 230), (rm.y(vf), 270), (rm.y(vf - 50), 270)],
            'x', a + 60, min(b, a + 260))
    box("Lights", LA, a + 70, min(b, a + 250), *sorted((rm.y(vf - 78), rm.y(vf - 52))), 227, 229)
    for k in range(int((b - a - 200) // 140)):
        uo = a + 30 + k * 140
        rm.B("trim", uo, uo + 110, vf - 72, vf - 70, 12, 80)
        rm.B(VP, uo + 15, uo + 95, vf - 73, vf - 72, 35, 70)
    rm.B(GA, b - 110, b, vf - 80, vf, 0, 220, collide=True)
    rm.B("trim", b - 56, b - 54, vf - 81, vf - 80, 10, 215)
    for k in range(3):
        cyl(p, "body", a + 330 + k * 45, rm.y(vf - 35), 94, 130, 14, 10)
    # table off to the side of the door so the doorway stays clear
    tl = min(240, rm.L * 0.38)
    du = rm.door_u if rm.door_u is not None else rm.cx
    tu = du + rm.L * 0.27 if du - rm.x0 < rm.x1 - du else du - rm.L * 0.27
    tu = max(rm.x0 + tl / 2 + 60, min(rm.x1 - tl / 2 - 60, tu))
    table(p, tu, rm.y(min(vf - 190, max(170, rm.D * 0.45))), tl, 80, seats=4)
    rm.B("trim", rm.x0 + 4, rm.x0 + 8, 60, 60 + min(300, vf - 150), 120, 200)       # menu board
    box("Lights", S["screen"], rm.x0 + 8, rm.x0 + 9, *sorted((rm.y(70), rm.y(50 + min(300, vf - 150)))), 128, 192)
    dress(rm)
    light("A", 5, rm.cx, rm.y(vf - 60), 200)


def rm_mess(rm):
    """Crew mess: bench tables, serving line, galley counter. opts dining=True
    (luxury): round tables with chairs, a bar and plants."""
    p = rm.part
    vf = rm.D
    dining = rm.opts.get("dining")
    rm.B(GA if not dining else "frame", rm.x0 + 60, rm.x1 - 60, vf - 75, vf, 0, 90, collide=True)
    rm.B("body", rm.x0 + 60, rm.x1 - 60, vf - 80, vf, 90, 94)
    if not dining:
        extrude(p, "frame", [(rm.y(vf - 90), 250), (rm.y(vf), 250), (rm.y(vf), 290), (rm.y(vf - 60), 290)],
                'x', rm.x0 + 120, rm.x1 - 120)
        box("Lights", LA, rm.x0 + 130, rm.x1 - 130, *sorted((rm.y(vf - 88), rm.y(vf - 62))), 247, 249)
        if vf < 700:
            vm = (vf - 80 + 170) / 2
            n = max(1, int((rm.L - 120) // 330))
            for i in range(n):
                u = rm.x0 + 60 + (i + 0.5) * (rm.L - 120) / n
                table(p, u, rm.y(vm), 240, 80, seats=6)
                for k in range(3):
                    box(p, GA, u - 100 + k * 80, u - 70 + k * 80, *sorted((rm.y(vm - 30), rm.y(vm - 8))), 74, 76)
                    cyl(p, "body", u - 60 + k * 80, rm.y(vm + 20), 74, 86, 4, 8)
            dress(rm, panel=200)
            return
        sl0, sl1 = rm.x0 + rm.L * 0.25, rm.x1 - rm.L * 0.25
        vs = vf - 260
        rm.B(GA, sl0, sl1, vs - 32, vs + 32, 0, 92, collide=True)
        rm.B("body", sl0, sl1, vs - 36, vs + 36, 92, 96)
        rm.B(VP, sl0 + 20, sl1 - 20, vs - 2, vs + 2, 110, 150)
        for k in range(int((sl1 - sl0) // 150)):
            rm.B("trim", sl0 + 30 + k * 150, sl0 + 140 + k * 150, vs - 20, vs + 20, 88, 93)
        nt = max(1, int((rm.L - 200) // 400))
        for i in range(nt):
            u = rm.x0 + 100 + (i + 0.5) * (rm.L - 200) / nt
            v0, v1 = 190, max(260, vs - 110)
            box(p, "table", u - 50, u + 50, *sorted((rm.y(v0), rm.y(v1))), 72, 78)
            box(p, "trim", u - 44, u + 44, *sorted((rm.y(v0 + 6), rm.y(v1 - 6))), 66, 72)
            for s in (-1, 1):
                box(p, "table", u + s * 70, u + s * 100, *sorted((rm.y(v0), rm.y(v1))), 42, 46)
                box(p, "trim", u + s * 80, u + s * 90, *sorted((rm.y(v0 + 10), rm.y(v1 - 10))), 0, 42)
            for vl in (v0 + 20, v1 - 20):
                box(p, "trim", u - 6, u + 6, *sorted((rm.y(vl - 6), rm.y(vl + 6))), 0, 66)
            for k in range(int((v1 - v0) // 90)):
                vv = v0 + 30 + k * 90
                box(p, GA, u - 40, u - 8, *sorted((rm.y(vv), rm.y(vv + 24))), 78, 80)
                cyl(p, "body", u + 22, rm.y(vv + 12), 78, 90, 4, 8)
            col(u - 102, u + 102, *sorted((rm.y(v0 - 2), rm.y(v1 + 2))), 0, 90)
    else:
        # bar back shelves with bottles
        for z in (120.0, 165.0, 210.0):
            rm.B("frame", rm.x0 + 80, rm.x1 - 80, vf - 22, vf, z, z + 4)
            for k in range(int((rm.L - 180) // 18)):
                if (k * 5) % 7 == 3:
                    continue
                cyl(p, VP if k % 3 else BRS, rm.x0 + 95 + k * 18, rm.y(vf - 11), z + 4, z + 30 + (k % 3) * 4, 4, 6)
        box("Lights", LP, rm.x0 + 80, rm.x1 - 80, *sorted((rm.y(vf - 3), rm.y(vf - 1))), 110, 240)
        if vf >= 620:
            for u in rm.spans(rm.x0 + 120, rm.x1 - 120, 60, gap=40, pad=0):
                cyl(p, "trim", u + 30, rm.y(vf - 130), 0, 72, 4, 8)
                cyl(p, "soft", u + 30, rm.y(vf - 130), 72, 78, 18, 12)
        nt = max(1, int((rm.L - 200) // 330))
        for i in range(nt):
            for j, vv in enumerate((290.0, vf - 330)):
                if j == 1 and vv < 290 + 240:
                    continue
                u = rm.x0 + 100 + (i + 0.5) * (rm.L - 200) / nt
                cyl(p, "table", u, rm.y(vv), 72, 76, 55, 20)
                cyl(p, "trim", u, rm.y(vv), 0, 72, 6, 8)
                cyl(p, "trim", u, rm.y(vv), 0, 3, 30, 12)
                for a in range(4):
                    t = math.radians(a * 90 + 45)
                    chair(p, u + 85 * math.cos(t), rm.y(vv) + 85 * math.sin(t), 0,
                          math.degrees(t), big=False)
                col(u - 110, u + 110, rm.y(vv) - 110, rm.y(vv) + 110, 0, 80)
                box("Lights", LP, u - 4, u + 4, rm.y(vv) - 4, rm.y(vv) + 4, rm.h - 60, rm.h)
                cyl("Lights", LP, u, rm.y(vv), rm.h - 90, rm.h - 60, 22, 12)
        for u in (rm.x0 + 60, rm.x1 - 60):
            plant(p, u, rm.y(120), r=28, h=120)
    dress(rm, panel=200)


def rm_brig(rm):
    p = rm.part
    cd = min(240.0, rm.D - 200)
    vb = rm.D - cd
    cw = 230.0
    n = max(1, int((rm.L - 40) // cw))
    u0 = rm.x0 + (rm.L - n * cw) / 2
    for i in range(n + 1):
        u = u0 + i * cw
        rm.B("frame", u - 8, u + 8, vb, rm.D, 0, rm.h, collide=True)
    for i in range(n):
        ua = u0 + i * cw
        # bars with a gate gap drawn closed
        k = ua + 16
        while k < ua + cw - 12:
            pipe(p, "trim", (k, rm.y(vb), 0), (k, rm.y(vb), 240), 1.8, 6)
            k += 14
        for z in (10.0, 120.0, 238.0):
            pipe(p, "frame", (ua + 8, rm.y(vb), z), (ua + cw - 8, rm.y(vb), z), 3, 8)
        rm.B("accent", ua + cw - 30, ua + cw - 22, vb - 3, vb + 3, 0, 240)
        rm.C(ua, ua + cw, vb - 4, vb + 4, 0, 250)
        rm.B("frame", ua + 20, ua + cw - 60, rm.D - 80, rm.D - 4, 35, 45)          # cot
        rm.B(BU, ua + 24, ua + cw - 64, rm.D - 76, rm.D - 8, 45, 55)
        rm.B("trim", ua + cw - 50, ua + cw - 14, rm.D - 50, rm.D - 4, 0, 42)       # toilet block
        rm.B("body", ua + 80, ua + 120, rm.D - 3, rm.D - 1, 180, 200)
        box("Lights", LW, ua + 84, ua + 116, *sorted((rm.y(rm.D - 4), rm.y(rm.D - 3))), 184, 196)
    # guard desk
    du = rm.x0 + 120 if not rm.near_door(rm.x0 + 60, rm.x0 + 260) else rm.x1 - 200
    console(p, du, rm.y(vb - 150), rm.yaw(-90), n_crt=2)
    for u in (rm.x0, rm.x1):
        cage_lamp(p, u, rm.y(vb - 40), rm.h - 80, 0.0 if u == rm.x0 else 180.0)
    box(p, "accent", rm.x0, rm.x1, *sorted((rm.y(vb - 50), rm.y(vb - 42))), 0, 0.6)
    dress(rm)
    light("R", 4, rm.cx, rm.y(vb - 60), rm.h - 60)


def rifle(part, x, y, yaw):
    M = Mx(x, y, 0, yaw)
    box(part, "frame", -6, 6, -3, 3, 88, 118, M)
    box(part, "trim", -7, 9, -4, 4, 118, 172, M)
    box(part, "frame", 2, 11, -4.5, 4.5, 146, 186, M)
    box(part, "trim", 9, 17, -2.5, 2.5, 128, 146, M)
    pipe(part, "trim", M @ Vector((5, 0, 186)), M @ Vector((5, 0, 204)), 1.6, 6)


def rm_armory(rm):
    p = rm.part
    vf = rm.D
    a, b = rm.x0 + 30, rm.x1 - 30
    rm.B("frame", a, b, vf - 16, vf, 70, 230)
    rm.B("trim", a, b, vf - 30, vf, 70, 84)
    u = a + 30
    while u < b - 20:
        rifle(p, u, rm.y(vf - 24), rm.yaw(90))
        u += 32
    rm.C(a, b, vf - 40, vf, 0, 230)
    # end-wall rack
    ue = rm.x1 - 16
    rm.B("frame", ue - 16, ue + 16, 200, vf - 60, 70, 230)
    v = 230.0
    while v < vf - 80:
        rifle(p, ue - 24, rm.y(v), 180.0)
        v += 32
    rm.C(ue - 40, ue + 16, 200, vf - 60, 0, 230)
    # ammo crates + weapon stand
    for k, (du, dv) in enumerate(((0.3, 0.5), (0.55, 0.45))):
        u0 = rm.x0 + rm.L * du
        for j in range(2 + k % 2):
            crate(p, u0, rm.y(vf * dv), j * 52, 110, 70, 50, yaw=j * 7.0, collide=(j == 0))
    sx, sv = rm.x0 + 110, vf * 0.55
    cyl(p, "trim", sx, rm.y(sv), 0, 8, 45, 12)
    cyl(p, "trim", sx, rm.y(sv), 8, 120, 6, 8)
    box(p, "frame", sx - 25, sx + 25, rm.y(sv) - 15, rm.y(sv) + 15, 120, 180)
    pipe(p, "trim", (sx + 25, rm.y(sv), 160), (sx + 95, rm.y(sv), 162), 6, 10)
    cyl("Collision", CL, sx, rm.y(sv), 0, 190, 60, 8)
    box(p, "accent", rm.x0 + 20, rm.x1 - 20, *sorted((rm.y(vf - 120), rm.y(vf - 112))), 0, 0.6)
    dress(rm)
    light("R", 4, rm.cx, rm.y(vf - 80), rm.h - 60)


def eva_suit(part, x, y, yaw, visor=VP):
    """EVA hardsuit on its hanger, back to local -X."""
    M = Mx(x, y, 0, yaw)
    box(part, "accent", -14, 16, -24, 24, 110, 170, M)
    box(part, "frame", -30, -14, -20, 20, 115, 175, M)                           # life-support pack
    box(part, "trim", -14, 16, -22, 22, 100, 112, M)
    for s in (-1, 1):
        box(part, "accent", -12, 12, s * 24, s * 38, 145, 170, M)
        box(part, "trim", -8, 8, s * 26, s * 36, 95, 146, M)
        box(part, "accent", -10, 10, s * 6, s * 20, 15, 100, M)
        box(part, "trim", -12, 14, s * 5, s * 21, 0, 15, M)
    K[P(part)].ico(R("accent"), tuple(M @ Vector((0, 0, 192))), 19, 1)
    box(part, visor, 16, 20, -12, 12, 184, 200, M)


def rm_ready(rm):
    """Suit-up room: suits on hangers along the hull wall, benches, status
    board. opts eva=True for EVA hardsuits (civil ships)."""
    p = rm.part
    vf = rm.D
    eva = rm.opts.get("eva") or S["accent"] != dk.HZ
    for u in rm.spans(rm.x0 + 30, rm.x1 - 30, 160, gap=26, pad=0):
        um = u + 80
        rm.B("frame", u, u + 160, vf - 10, vf, 0, 230)
        rm.B("trim", u, u + 6, vf - 75, vf, 0, 230)
        rm.B("trim", u + 154, u + 160, vf - 75, vf, 0, 230)
        rm.B("trim", u, u + 160, vf - 75, vf, 230, 238)
        rm.B("body", u + 6, u + 154, vf - 75, vf, 0, 12)
        if eva:
            eva_suit(p, um, rm.y(vf - 42), rm.yaw(-90))
        else:
            box(p, "frame", um - 26, um + 26, *sorted((rm.y(vf - 58), rm.y(vf - 25))), 110, 172)
            box(p, "trim", um - 20, um + 20, *sorted((rm.y(vf - 60), rm.y(vf - 24))), 100, 112)
            for s in (-1, 1):
                box(p, "frame", um + s * 26, um + s * 42, *sorted((rm.y(vf - 56), rm.y(vf - 26))), 150, 176)
                box(p, "trim", um + s * 28, um + s * 38, *sorted((rm.y(vf - 50), rm.y(vf - 32))), 95, 150)
                box(p, "frame", um + s * 6, um + s * 22, *sorted((rm.y(vf - 52), rm.y(vf - 30))), 15, 100)
                box(p, "trim", um + s * 5, um + s * 23, *sorted((rm.y(vf - 60), rm.y(vf - 28))), 0, 15)
            K[P(p)].ico(R("frame"), (um, rm.y(vf - 42), 192), 17, 1)
            box(p, VP, um - 11, um + 11, *sorted((rm.y(vf - 59), rm.y(vf - 57))), 186, 196)
        box("Lights", LA, um - 30, um + 30, *sorted((rm.y(vf - 72), rm.y(vf - 70))), 224, 228)
    rm.C(rm.x0 + 30, rm.x1 - 30, vf - 75, vf, 0, 238)
    vb = max(200.0, vf * 0.45)
    for u in rm.spans(rm.x0 + 150, rm.x1 - 150, 120, gap=260, pad=40):
        rm.B("trim", u, u + 120, vb - 20, vb + 20, 38, 45)
        rm.B("frame", u + 2, u + 118, vb - 18, vb + 18, 45, 50)
        for uu in (u + 20, u + 100):
            rm.B("trim", uu - 5, uu + 5, vb - 12, vb + 12, 0, 38)
        rm.C(u, u + 120, vb - 22, vb + 22, 0, 60)
    ue = rm.x1 - 12
    rm.B("trim", ue, ue + 12, 120, min(vf - 120, 560), 130, 300)
    rm.B(CO, ue - 2, ue, 135, min(vf - 135, 545), 145, 285)
    dress(rm)


def rm_airlock(rm):
    """Airlock: outer hatch in the hull wall, suit lockers, cycling lamps."""
    p = rm.part
    vf = rm.D
    uc = rm.cx
    airlock_door(p, uc, rm.y(vf), rm.yaw(-90), w=150, h=220)
    for s in (-1, 1):
        rm.B("accent", uc + s * 110 - 6, uc + s * 110 + 6, vf - 160, vf, 0, 0.6)
    rm.B("accent", uc - 110, uc + 110, vf - 166, vf - 160, 0, 0.6)
    for k in range(6):
        rm.B(GR, uc - 100, uc + 100, vf - 150 + k * 24, vf - 138 + k * 24, 0, 0.8)
    # suits on the end walls
    for u, yaw in ((rm.x0 + 42, 0.0), (rm.x1 - 42, 180.0)):
        for k, v in enumerate((vf * 0.4, vf * 0.72)):
            if v < 150:
                continue
            eva_suit(p, u, rm.y(v), yaw)
        rm.C(u - 40, u + 40, 120, vf - 40, 0, 210)
    for u in (uc - 150, uc + 150):
        cage_lamp(p, u, rm.y(vf), rm.h - 70, rm.yaw(-90), color=LA)
    rm.B("trim", uc + 110, uc + 150, vf - 6, vf, 110, 170)
    box("Lights", LG, uc + 114, uc + 146, *sorted((rm.y(vf - 7), rm.y(vf - 6))), 130, 162)
    dress(rm, n=1)
    light("A", 4, uc, rm.y(vf - 100), rm.h - 60)


def rm_workshop(rm):
    p = rm.part
    vf = rm.D
    a, b = rm.x0 + 40, rm.x1 - 40
    rm.B("body", a, b, vf - 80, vf, 88, 94)
    for u in (a + 10, b - 10, (a + b) / 2):
        rm.B("trim", u - 5, u + 5, vf - 75, vf - 5, 0, 88)
    rm.C(a, b, vf - 80, vf, 0, 100)
    rm.B("frame", a, b, vf - 6, vf, 120, 260)                                       # pegboard
    for k in range(int((b - a) // 45)):
        rm.B("trim", a + 20 + k * 45, a + 24 + k * 45, vf - 10, vf - 6, 150, 190 + (k % 3) * 18)
    for k in range(3):
        u = a + 100 + k * (b - a - 200) / 2
        rm.B("trim", u - 20, u + 20, vf - 70, vf - 40, 94, 118)
    # lathe
    lu = rm.x0 + 90
    rm.B("accent", lu - 50, lu + 50, vf * 0.45 - 40, vf * 0.45 + 40, 0, 90, collide=True)
    pipe(p, "trim", (lu - 40, rm.y(vf * 0.45), 110), (lu + 40, rm.y(vf * 0.45), 110), 10, 12)
    rm.B("body", lu - 50, lu - 20, vf * 0.45 - 30, vf * 0.45 + 30, 90, 140)
    # engine part on a stand
    su, sv = rm.x1 - 140, vf * 0.45
    cyl(p, "trim", su, rm.y(sv), 0, 60, 10, 8)
    pipe(p, "frame", (su - 70, rm.y(sv), 95), (su + 70, rm.y(sv), 95), 34, 14)
    for k in range(5):
        ring(p, "trim", su - 60 + k * 30, rm.y(sv), 30, 40, 70, 120, 14)
    cyl("Collision", CL, su, rm.y(sv), 0, 130, 75, 8)
    # welding cart, parts shelf
    crate(p, rm.cx, rm.y(vf * 0.4), 0, 70, 50, 80, lid="trim")
    tank(p, rm.cx + 30, rm.y(vf * 0.4) + 10, 10, 90, bands=1, collide=False)
    if rm.D > 450:
        shelf_rack(p, rm.x0 + 30, rm.x0 + 90, *sorted((rm.y(160), rm.y(vf - 120))), h=220)
    dress(rm)
    light("A", 5, rm.cx, rm.y(vf - 80), 200)


def rm_ecm(rm):
    """Electronic-warfare / computer core: rows of server cabinets."""
    p = rm.part
    vf = rm.D
    rows = []
    v = 200.0
    while v + 70 < vf - 30:
        rows.append(v)
        v += 190
    for vr in rows:
        for u in rm.spans(rm.x0 + 60, rm.x1 - 60, 70, gap=2, pad=60):
            rm.B("body", u, u + 68, vr, vr + 70, 0, 215)
            rm.B("trim", u + 2, u + 66, vr - 2, vr, 10, 205)
            for k in range(8):
                c = (LB, LG, S["screen"], LR)[(k + int(u)) % 4]
                box("Lights", c, u + 8 + (k % 4) * 12, u + 14 + (k % 4) * 12,
                    *sorted((rm.y(vr - 3), rm.y(vr - 2))), 40 + (k // 4) * 70, 44 + (k // 4) * 70)
        rm.C(rm.x0 + 60, rm.x1 - 60, vr, vr + 70, 0, 215)
        box(p, "trim", rm.x0 + 60, rm.x1 - 60, *sorted((rm.y(vr + 20), rm.y(vr + 50))), 215, 225)
    for vr in rows:
        pipe(p, "trim", (rm.x0 + 20, rm.y(vr + 35), rm.h - 30), (rm.x1 - 20, rm.y(vr + 35), rm.h - 30), 6, 8)
    console(p, rm.x1 - 72 - 20, rm.y(120), 180.0, n_crt=2, screen=S["screen"])
    dress(rm)
    light("B", 5, rm.cx, rm.y(vf * 0.5), 150)


def rm_ops(rm):
    """Operations / war room: holo table, consoles on the walls, big display.
    opts big=True (war room): long conference table and a wall of screens."""
    p = rm.part
    vf = rm.D
    big = rm.opts.get("big")
    cu, cv = rm.cx, vf * 0.5
    if big:
        table(p, cu, rm.y(cv), rm.L * 0.45, 160, slot="table", seats=0)
        n = int(rm.L * 0.45 // 90)
        for k in range(n):
            u = cu - rm.L * 0.225 + (k + 0.5) * rm.L * 0.45 / n
            for s in (-1, 1):
                chair(p, u, rm.y(cv + s * 125), 0, rm.yaw(90 * s))
        holo_table(p, cu, rm.y(cv), 70, z0=0.0, grid=S["screen"], collide=False)
        col(cu - rm.L * 0.225 - 40, cu + rm.L * 0.225 + 40, *sorted((rm.y(cv - 160), rm.y(cv + 160))), 0, 80)
    else:
        holo_table(p, cu, rm.y(cv), min(170.0, vf * 0.2), grid=S["screen"])
    for u in rm.spans(rm.x0 + 60, rm.x1 - 60, 240, gap=60, pad=0):
        console(p, u + 120, rm.y(vf - 72 - 15), rm.yaw(-90), n_crt=3)
    # big display on an end wall
    ue = rm.x1 - 8
    rm.B("trim", ue - 10, ue, vf * 0.2, vf * 0.8, 110, rm.h - 50)
    rm.B(CO, ue - 12, ue - 10, vf * 0.2 + 15, vf * 0.8 - 15, 125, rm.h - 65)
    for u, yaw in ((rm.x0 + 8, 0.0),):
        for k in range(3):
            crt(p, u + 60, rm.y(vf * 0.3 + k * vf * 0.2), 150, 110, 85, yaw, d=60)
        rm.B("body", u, u + 60, vf * 0.2, vf * 0.8, 0, 150, collide=True)
    dress(rm)
    light(S["fill"] if big else "B", 5, cu, rm.y(cv), 260)


def rm_magazine(rm):
    """Ordnance magazine: missiles on cradles, overhead hoist rail."""
    p = rm.part
    vf = rm.D
    L = min(rm.L - 160, 520)
    for k, vr in enumerate((vf - 60, vf - 180)):
        if vr < 180:
            break
        ua, ub = rm.cx - L / 2, rm.cx + L / 2
        for u in (ua + 30, ub - 30):
            rm.B("frame", u - 15, u + 15, vr - 50, vr + 50, 0, 180)
        for j in range(3):
            z = 40 + j * 55
            pipe(p, "frame", (ua, rm.y(vr), z), (ub - 60, rm.y(vr), z), 20, 12)
            pipe(p, "accent", (ub - 60, rm.y(vr), z), (ub - 10, rm.y(vr), z), 14, 12)
            pipe(p, "trim", (ua - 20, rm.y(vr), z), (ua, rm.y(vr), z), 16, 12)
        rm.C(ua - 20, ub, vr - 55, vr + 55, 0, 200)
    # hoist rail and trolley
    vr = max(200.0, vf * 0.5)
    rm.B("frame", rm.x0 + 20, rm.x1 - 20, vr - 10, vr + 10, rm.h - 40, rm.h)
    rm.B("body", rm.cx - 40, rm.cx + 40, vr - 25, vr + 25, rm.h - 80, rm.h - 40)
    pipe(p, "trim", (rm.cx, rm.y(vr), rm.h - 80), (rm.cx, rm.y(vr), 150), 2, 6)
    rm.B("accent", rm.cx - 20, rm.cx + 20, vr - 20, vr + 20, 130, 150)
    for u in rm.spans(rm.x0 + 40, rm.x1 - 40, 90, gap=30, pad=40)[:4]:
        crate(p, u + 45, rm.y(160), 0, 90, 60, 55)
    rm.B("accent", rm.x0 + 20, rm.x1 - 20, vf - 250, vf - 242, 0, 0.6)
    dress(rm)
    light("R", 4, rm.cx, rm.y(vf * 0.5), rm.h - 60)


def rm_gunnery(rm):
    """Fire control: gunner seats with targeting screens; turret feed trunk."""
    p = rm.part
    vf = rm.D
    n = max(2, int((rm.L - 200) // 280))
    for i in range(n):
        u = rm.x0 + 100 + (i + 0.5) * (rm.L - 200) / n
        console(p, u, rm.y(vf - 87), rm.yaw(-90), n_crt=2, chair_too=False)
        pilot_seat(p, u, rm.y(vf - 190), 0, rm.yaw(90))
        rm.B("trim", u - 30, u + 30, vf - 80, vf - 20, 150, 152)
        crt(p, u, rm.y(vf - 20), 170, 90, 70, rm.yaw(-90), screen=S["screen2"], d=40)
    tu, tv = rm.x0 + 90, vf * 0.45
    cyl(p, "frame", tu, rm.y(tv), 0, rm.h, 60, 16)
    ring(p, "accent", tu, rm.y(tv), 60, 66, 0, 40, 16)
    for z in range(40, int(rm.h) - 30, 30):
        rm.B("trim", tu + 58, tu + 62, tv - 18, tv + 18, z, z + 3)
    cyl("Collision", CL, tu, rm.y(tv), 0, rm.h, 66, 12)
    dress(rm)
    light(S["fill"], 4, rm.cx, rm.y(vf - 150), 200)


def rm_medbay(rm):
    """Small medbay: two beds with lamps and monitors, cabinets, a cryo pod."""
    p = rm.part
    vf = rm.D
    for u in rm.spans(rm.x0 + 60, rm.x1 - 180, 100, gap=120, pad=0)[:3]:
        um = u + 50
        rm.B("trim", um - 40, um + 40, vf - 250, vf - 40, 0, 70)
        rm.B(GA, um - 44, um + 44, vf - 254, vf - 36, 70, 84)
        rm.B(BU, um - 38, um + 38, vf - 90, vf - 44, 84, 94)
        cyl(p, "trim", um, rm.y(vf - 145), rm.h - 70, rm.h, 5, 8)
        cyl(p, "body", um, rm.y(vf - 145), rm.h - 90, rm.h - 70, 55, 16)
        cyl("Lights", LW, um, rm.y(vf - 145), rm.h - 92, rm.h - 90, 45, 16)
        crt(p, um + 75, rm.y(vf - 45), 120, 50, 40, rm.yaw(-90), screen=S["screen"], d=40)
        rm.B("trim", um + 50, um + 100, vf - 40, vf - 10, 0, 120)
        rm.C(um - 46, um + 46, vf - 256, vf, 0, 100)
    # cabinets on the end wall
    ue = rm.x1 - 15
    for k in range(int((vf - 200) // 170)):
        v = 120 + k * 170
        rm.B(GA, ue - 55, ue, v, v + 160, 0, 90)
        rm.B(GA, ue - 35, ue, v, v + 160, 150, 240)
        rm.B(VP, ue - 36, ue - 35, v + 10, v + 150, 160, 230)
    rm.C(ue - 55, ue, 120, vf - 80, 0, 100)
    # cryo pod
    if rm.L > 600 and vf > 360 and not rm.near_door(rm.x0, rm.x0 + 160, 20):
        cu = rm.x0 + 70
        cvp = max(170.0, vf - 380)
        M = Mx(cu, rm.y(cvp + 115), 0, rm.yaw(90))
        box(p, "body", -110, 110, -45, 45, 0, 55, M)
        box(p, BU, -100, 100, -36, 36, 55, 62, M)
        lid = [(38 * math.cos(math.radians(t)), 62 + 34 * math.sin(math.radians(t))) for t in range(0, 181, 20)]
        extrude(p, VP, lid, 'x', -92, 96, M)
        box("Lights", LB, -112, -110, -30, 30, 20, 30, M)
        rm.C(cu - 50, cu + 50, cvp, cvp + 230, 0, 110)
    dress(rm)
    light("W", 5, rm.cx, rm.y(vf - 145), rm.h - 110)


def rm_cryo(rm):
    """Cryo ring: eight hypersleep pods around a control column."""
    p = rm.part
    vf = rm.D
    r1 = min(370.0, min(rm.L, vf) * 0.5 - 60, (vf - 170) / 2)
    r0 = min(150.0, r1 * 0.4)
    cv = vf - 20 - r1
    du = rm.door_u if rm.door_u is not None else rm.cx
    cu = rm.x0 + r1 + 80 if du - rm.x0 > rm.x1 - du else rm.x1 - r1 - 80
    for k in range(8):
        a = 22.5 + 45 * k
        M = Mx(cu, rm.y(cv), 0, a)
        box(p, "body", r0, r1, -45, 45, 0, 55, M)
        box(p, "trim", r0 - 4, r1 + 4, -48, 48, 0, 10, M)
        box(p, BU, r0 + 10, r1 - 10, -36, 36, 55, 62, M)
        lid = [(38 * math.cos(math.radians(t)), 62 + 34 * math.sin(math.radians(t))) for t in range(0, 181, 20)]
        extrude(p, VP, lid, 'x', r0 + 18, r1 - 14, M)
        box(p, "body", r0, r0 + 20, -40, 40, 55, 115, M)
        box("Lights", S["screen"], r0 + 20, r0 + 21, -28, 28, 80, 108, M)
        box("Lights", LB, r1 + 4, r1 + 5, -30, 30, 20, 30, M)
        box("Collision", CL, r0 - 4, r1 + 6, -50, 50, 0, 110, M)
    cyl(p, "body", cu, rm.y(cv), 0, rm.h, r0 * 0.6, 12)
    for z in (80.0, 180.0, 280.0):
        if z < rm.h - 30:
            cyl("Lights", LB, cu, rm.y(cv), z, z + 12, r0 * 0.6 + 2, 12)
    cyl("Collision", CL, cu, rm.y(cv), 0, rm.h, r0 * 0.6 + 15, 12)
    light("B", 7, cu, rm.y(cv), rm.h - 40)
    dress(rm, n=2)


def rm_briefing(rm):
    p = rm.part
    vf = rm.D
    ue = rm.x1 - 20
    vc = vf * 0.5
    rm.B("trim", ue - 20, ue, vc - min(320, vf * 0.4), vc + min(320, vf * 0.4), 60, rm.h - 20)
    rm.B(CO, ue - 22, ue - 20, vc - min(300, vf * 0.4 - 20), vc + min(300, vf * 0.4 - 20), 80, rm.h - 40)
    half = min(320, vf * 0.4)
    rm.C(ue - 22, ue, vc - half, vc + half, 0, rm.h)
    M = Mx(ue - 180, rm.y(vf - 120), 0, 180.0)
    extrude(p, "frame", [(-30, 0), (30, 0), (24, 110), (-36, 118)], 'y', -35, 35, M)
    col(ue - 215, ue - 145, *sorted((rm.y(vf - 160), rm.y(vf - 80))), 0, 120)
    urow = ue - 380
    blocks = ((150.0, vc - 60), (vc + 60, vf - 60)) if vf >= 700 else ((110.0, vf - 50),)
    while urow > rm.x0 + 120:
        for (va, vb) in blocks:
            k = 0
            v = va + 40
            while v < vb - 30:
                chair(p, urow, rm.y(v), 0, 180.0)
                v += 90
                k += 1
            if k:
                col(urow - 35, urow + 35, *sorted((rm.y(va), rm.y(vb))), 0, 100)
        urow -= 160 if rm.L < 800 else 190
    dress(rm, along=False)
    light("G" if S["screen"] == LG else "B", 5, ue - 200, rm.y(vc), 220)


def rm_cabins(rm):
    """Private cabins along the hull wall off a short passage. opts lux=True:
    one or two big suites (double bed, sofa, desk, plants, window)."""
    p = rm.part
    vf = rm.D
    lux = rm.opts.get("lux")
    if lux:
        # one suite: bed against the end wall, lounge by the (hull) window
        bu = rm.x1 - 120
        bed(p, bu, rm.y(vf * 0.55), 180.0, w=180, L=210)       # headboard on the end wall
        for s in (-1, 1):
            rm.B("frame", bu + 60, bu + 110, vf * 0.55 + s * 125 - 25, vf * 0.55 + s * 125 + 25, 0, 55, collide=True)
            cyl(p, "trim", bu + 85, rm.y(vf * 0.55 + s * 125), 55, 90, 6, 8)
            cyl("Lights", LP, bu + 85, rm.y(vf * 0.55 + s * 125), 90, 110, 14, 10)
        sofa(p, rm.x0 + 220, rm.y(vf - 110), 240, 180.0 if rm.sg > 0 else 0.0)
        table(p, rm.x0 + 220, rm.y(vf - 230), 120, 60, h=42, slot="table")
        rm.B("table", rm.x0 + 20, rm.x0 + 80, 160, 330, 72, 76, collide=True)          # desk
        chair(p, rm.x0 + 120, rm.y(245), 0, 180.0)
        flatscreen(p, rm.x0 + 12, rm.y(245), 110, 100, 60, 0.0)
        for u in (rm.x0 + 60, rm.x1 - 60):
            plant(p, u, rm.y(vf - 60), r=26, h=120)
        rm.B(CRP, rm.x0 + 40, rm.x1 - 40, 100, vf - 40, 0, 0.8)
        for u in (rm.x0, rm.x1):
            sconce(p, u + (4 if u == rm.x0 else -4), rm.y(vf * 0.3), 190, 0.0 if u == rm.x0 else 180.0)
        dress(rm, n=2, panel=120)
        fill(6, rm.cx, rm.y(vf * 0.5), rm.h - 60)
        return
    cd = min(300.0, vf - 170)
    vb = vf - cd
    cw = 300.0
    n = max(1, int((rm.L - 20) // cw))
    u0 = rm.x0 + (rm.L - n * cw) / 2
    for i in range(n + 1):
        u = u0 + i * cw
        rm.B("frame", u - 5, u + 5, vb, vf, 0, rm.h, collide=True)
    for i in range(n):
        ua = u0 + i * cw
        dcu = ua + cw - 70
        # front wall with a door opening
        seg_box(p, "wall", (ua, rm.y(vb)), (dcu - 45, rm.y(vb)), 10, 0, rm.h)
        seg_box("Collision", CL, (ua, rm.y(vb)), (dcu - 45, rm.y(vb)), 10, 0, rm.h)
        seg_box(p, "wall", (dcu + 45, rm.y(vb)), (ua + cw, rm.y(vb)), 10, 0, rm.h)
        seg_box("Collision", CL, (dcu + 45, rm.y(vb)), (ua + cw, rm.y(vb)), 10, 0, rm.h)
        seg_box(p, "wall", (dcu - 45, rm.y(vb)), (dcu + 45, rm.y(vb)), 10, 215, rm.h)
        for s in (-1, 1):
            seg_box(p, "frame", (dcu + s * 45 - 5, rm.y(vb)), (dcu + s * 45 + 5, rm.y(vb)), 18, 0, 222)
        seg_box(p, "frame", (dcu - 50, rm.y(vb)), (dcu + 50, rm.y(vb)), 18, 215, 222)
        box("Lights", LG, dcu + 55, dcu + 65, *sorted((rm.y(vb - 10), rm.y(vb - 9))), 140, 150)
        # bed, desk, locker
        bed(p, ua + 20 + 100, rm.y(vf - 55), 0.0, w=90, L=200)
        rm.B("table", ua + 12, ua + 60, vb + 20, vb + 110, 72, 76, collide=True)
        flatscreen(p, ua + 8, rm.y(vb + 65), 110, 70, 45, 0.0)
        rm.B("frame", ua + cw - 60, ua + cw - 8, vf - 110, vf - 4, 0, 205, collide=True)
        box("Lights", LA, ua + 30, ua + 70, *sorted((rm.y(vf - 3), rm.y(vf - 2))), 150, 158)
        ceiling_panel(p, ua + cw / 2, rm.y(vb + cd / 2), rm.h, 0.0, 90)
    # passage: numbered plates by each door
    rm.B("accent", rm.x0 + 20, rm.x1 - 20, vb - 12, vb - 4, 0, 0.6)
    for i in range(n):
        if i % 2 == 0:
            fill(5, u0 + (i + 1) * cw, rm.y(vb + cd / 2), rm.h - 40)
    for i in range(max(1, int(rm.L // 600))):
        u = rm.x0 + (i + 0.5) * rm.L / max(1, int(rm.L // 600))
        ceiling_panel(p, u, rm.y(vb / 2), rm.h, 0.0, 150)
    fill(6, rm.cx, rm.y(vb / 2), rm.h - 40)


def rm_lab(rm):
    """Laboratory: benches with instruments, fume hood, specimen tanks.
    opts bio=True: rows of glowing specimen cylinders and plant trays."""
    p = rm.part
    vf = rm.D
    bio = rm.opts.get("bio")
    a, b = rm.x0 + 40, rm.x1 - 40
    rm.B(GA, a, b, vf - 75, vf, 0, 90, collide=True)
    rm.B("body", a, b, vf - 78, vf, 90, 94)
    for k in range(int((b - a) // 160)):
        u = a + 60 + k * 160
        rm.B("trim", u - 12, u + 12, vf - 50, vf - 30, 94, 100)                        # microscope
        pipe(p, "trim", (u, rm.y(vf - 40), 100), (u, rm.y(vf - 40), 140), 3, 6)
        pipe(p, "body", (u, rm.y(vf - 40), 140), (u, rm.y(vf - 60), 128), 4, 6)
        if k % 2:
            flatscreen(p, u + 50, rm.y(vf - 10), 110, 60, 40, rm.yaw(-90))
    # fume hood
    hu = rm.x1 - 140
    rm.B("body", hu - 90, hu + 90, vf - 90, vf, 94, 240)
    rm.B(VP, hu - 80, hu + 80, vf - 91, vf - 89, 110, 190)
    box("Lights", LW, hu - 80, hu + 80, *sorted((rm.y(vf - 80), rm.y(vf - 60))), 230, 232)
    # island bench / tanks
    vi = max(vf * 0.45, 200.0)
    if bio:
        n = max(1, int((rm.L - 200) // 130))
        for i in range(n):
            uc = rm.x0 + 100 + (i + 0.5) * (rm.L - 200) / n
            cyl(p, "trim", uc, rm.y(vi), 0, 30, 42, 16)
            cyl(p, VP, uc, rm.y(vi), 30, 200, 36, 16)
            cyl("Lights", (LG, LC, LB)[int(uc) % 3], uc, rm.y(vi), 34, 180, 30, 16)
            cyl(p, "trim", uc, rm.y(vi), 200, 225, 42, 16)
            pipe(p, PI, (uc, rm.y(vi), 225), (uc, rm.y(vi), rm.h), 4, 6)
            cyl("Collision", CL, uc, rm.y(vi), 0, 225, 44, 8)
    else:
        L = min(rm.L - 360, 600)
        rm.B(GA, rm.cx - L / 2, rm.cx + L / 2, vi - 50, vi + 50, 0, 90, collide=True)
        rm.B("body", rm.cx - L / 2, rm.cx + L / 2, vi - 54, vi + 54, 90, 94)
        for k in range(int(L // 70)):
            u = rm.cx - L / 2 + 35 + k * 70
            cyl(p, VP, u, rm.y(vi + ((k % 2) * 30 - 15)), 94, 110 + (k % 3) * 8, 6, 8)
        rm.B("trim", rm.cx - L / 2, rm.cx + L / 2, vi - 3, vi + 3, 94, 150)
        rm.B("frame", rm.cx - L / 2, rm.cx + L / 2, vi - 20, vi + 20, 150, 154)
    # sample fridge
    rm.B(GA, rm.x0 + 20, rm.x0 + 90, 140, 260, 0, 200, collide=True)
    rm.B(VP, rm.x0 + 90, rm.x0 + 91, 150, 250, 20, 190)
    dress(rm)
    light("C" if S["screen"] == LC else "W", 5, rm.cx, rm.y(vi), 210)


def rm_storage(rm):
    p = rm.part
    vf = rm.D
    v = 200.0
    while v + 60 < vf - 20:
        for u0, u1 in ((rm.x0 + 40, rm.cx - 80), (rm.cx + 80, rm.x1 - 40)):
            if u1 - u0 > 120 and not rm.near_door(u0, u1, 0) or v > 250:
                shelf_rack(p, u0, u1, *sorted((rm.y(v), rm.y(v + 60))), h=min(260, rm.h - 40))
        v += 190
    for k in range(3):
        crate(p, rm.x1 - 110, rm.y(120 + k * 0), k * 55, 100, 70, 55, yaw=k * 6, collide=(k == 0))
    # pallet jack
    ju, jv = rm.cx, 110.0
    rm.B("accent", ju - 60, ju + 60, jv - 35, jv + 35, 5, 12)
    pipe(p, "trim", (ju + 60, rm.y(jv), 12), (ju + 100, rm.y(jv), 110), 3, 6)
    dress(rm)


def rm_ward(rm):
    """Hospital ward: bed rows with curtain rails, monitors, IV stands.
    opts triage=True: gurneys + a central nurse station."""
    p = rm.part
    vf = rm.D
    triage = rm.opts.get("triage")
    vrows = [vf - 115] + ([200.0] if vf > 620 else [])
    for vr in vrows:
        facing = -90 if vr > vf / 2 else 90
        for u in rm.spans(rm.x0 + 40, rm.x1 - 40, 180, gap=20, pad=40):
            um = u + 90
            if triage:
                M = Mx(um, rm.y(vr), 0, rm.yaw(facing + 90))
                box(p, GA, -35, 35, -95, 95, 70, 80, M)
                box(p, BU, -30, 30, -90, 90, 80, 88, M)
                for sx in (-28, 28):
                    for sy in (-85, 85):
                        c = M @ Vector((sx, sy, 0))
                        pipe(p, "trim", c, c + Vector((0, 0, 70)), 2.5, 6)
                col(um - 40, um + 40, rm.y(vr) - 95, rm.y(vr) + 95, 0, 90)
            else:
                # head against the nearer wall: hull row -> +v, spine row -> -v
                bed(p, um, rm.y(vr), rm.yaw(-90.0 if vr > vf / 2 else 90.0), w=90, L=200)
                # curtain rail around the bed
                pts = [(um - 85, vr - 115), (um - 85, vr + 115), (um + 85, vr + 115), (um + 85, vr - 115)]
                for i in range(3):
                    q0, q1 = pts[i], pts[i + 1]
                    pipe(p, "trim", (q0[0], rm.y(q0[1]), 240), (q1[0], rm.y(q1[1]), 240), 1.5, 6)
                rm.B(BU, um - 86, um - 84, vr - 110, vr + 110, 60, 235)
            pipe(p, "trim", (um + 60, rm.y(vr - 60), 0), (um + 60, rm.y(vr - 60), 190), 1.5, 6)
            rm.B(VP, um + 52, um + 68, vr - 64, vr - 56, 160, 185)
            crt(p, um - 60, rm.y(vr + (100 if vr > vf / 2 else -100)), 130, 40, 32,
                rm.yaw(-90 if vr > vf / 2 else 90), d=30)
    if triage:
        su, sv = rm.cx, vf * 0.5 if vf > 620 else vf * 0.45
        ring(p, GA, su, rm.y(sv), 80, 140, 0, 100, 16, 0, 270)
        ring(p, "body", su, rm.y(sv), 76, 144, 100, 105, 16, 0, 270)
        for a in (30.0, 135.0, 240.0):
            t = math.radians(a)
            crt(p, su + 110 * math.cos(t), rm.y(sv) + 110 * math.sin(t), 105, 50, 40, a + 180, d=36)
        cyl("Collision", CL, su, rm.y(sv), 0, 110, 146, 12)
    dress(rm, extra_fill=1)


def rm_surgery(rm):
    p = rm.part
    vf = rm.D
    su, sv = rm.cx, vf * 0.55
    M = Mx(su, rm.y(sv), 0, 0.0)
    box(p, "trim", -30, 30, -20, 20, 0, 80, M)
    box(p, GA, -100, 100, -32, 32, 80, 92, M)
    box(p, BU, -96, 96, -28, 28, 92, 98, M)
    col(su - 100, su + 100, rm.y(sv) - 34, rm.y(sv) + 34, 0, 100)
    # multi-head surgical lamp
    pipe(p, "trim", (su, rm.y(sv), rm.h), (su, rm.y(sv), rm.h - 60), 5, 8)
    for a in (0.0, 120.0, 240.0):
        t = math.radians(a)
        c = Vector((su + 70 * math.cos(t), rm.y(sv) + 70 * math.sin(t), rm.h - 110))
        pipe(p, "trim", (su, rm.y(sv), rm.h - 60), c + Vector((0, 0, 20)), 3, 6)
        cyl(p, "body", c.x, c.y, c.z, c.z + 20, 30, 16)
        cyl("Lights", LW, c.x, c.y, c.z - 1, c.z, 26, 16)
    # instrument arms and carts
    for s in (-1, 1):
        pipe(p, "trim", (su + s * 140, rm.y(sv - 90), 0), (su + s * 140, rm.y(sv - 90), 150), 4, 8)
        pipe(p, "trim", (su + s * 140, rm.y(sv - 90), 150), (su + s * 70, rm.y(sv - 40), 130), 3, 6)
        rm.B("body", su + s * 70 - 20, su + s * 70 + 20, sv - 55, sv - 25, 125, 132)
        rm.B(GA, su + s * 180 - 30, su + s * 180 + 30, sv + 70, sv + 110, 0, 90, collide=True)
    for k in range(3):
        flatscreen(p, su - 200 + k * 200, rm.y(vf - 8), 140, 140, 80, rm.yaw(-90))
    # glass observation partition near the door
    vp = min(170.0, vf * 0.3)
    for a, b in ((rm.x0 + 20, (rm.door_u or rm.cx) - rm.door_w / 2 - 60), ((rm.door_u or rm.cx) + rm.door_w / 2 + 60, rm.x1 - 20)):
        if b - a > 60:
            rm.B("frame", a, b, vp - 6, vp + 6, 0, 100, collide=True)
            rm.B(VP, a, b, vp - 2, vp + 2, 100, 230)
            rm.B("frame", a, b, vp - 6, vp + 6, 230, 240)
            rm.C(a, b, vp - 6, vp + 6, 0, 240)
    dress(rm)
    light("W", 6, su, rm.y(sv), rm.h - 130, shadow=True)


def rm_lounge(rm):
    """Lounge: sofas around low tables, a bar, plants; window on the hull."""
    p = rm.part
    vf = rm.D
    n = max(1, int((rm.L - 200) // 420))
    for i in range(n):
        u = rm.x0 + 100 + (i + 0.5) * (rm.L - 200) / n
        v = vf - 150
        # sofa back to the hull (it faces local +Y), armchair facing +u
        sofa(p, u, rm.y(v + 55), 220, 180.0 if rm.sg > 0 else 0.0)
        table(p, u, rm.y(v - 40), 120, 60, h=42, slot="table")
        chair(p, u - 110, rm.y(v - 120), 0, 180.0)
    bu = rm.x0 + 60
    rm.B("frame", bu, bu + 60, 160, min(vf - 260, 500), 0, 105, collide=True)
    rm.B("trim", bu - 6, bu + 66, 155, min(vf - 255, 505), 105, 110)
    box("Lights", LP, bu + 60, bu + 61, *sorted((rm.y(170), rm.y(min(vf - 270, 490)))), 20, 30)
    for u in (rm.x1 - 70,):
        plant(p, u, rm.y(140), r=28, h=130)
    if rm.L > 700:
        plant(p, rm.cx, rm.y(vf - 50), r=26, h=110)
    rm.B(CRP if S["floor"] != CRP else "soft", rm.x0 + 60, rm.x1 - 60, vf * 0.35, vf - 20, 0, 0.8)
    dress(rm, panel=140)
    light("P", 6, rm.cx, rm.y(vf - 150), 200)


ROOMS = {
    "bunks": rm_bunks, "galley": rm_galley, "mess": rm_mess, "brig": rm_brig, "armory": rm_armory,
    "ready": rm_ready, "airlock": rm_airlock, "workshop": rm_workshop, "ecm": rm_ecm, "ops": rm_ops,
    "magazine": rm_magazine, "gunnery": rm_gunnery, "medbay": rm_medbay, "cryo": rm_cryo,
    "briefing": rm_briefing, "cabins": rm_cabins, "lab": rm_lab, "storage": rm_storage, "ward": rm_ward,
    "surgery": rm_surgery, "lounge": rm_lounge,
}


# ----------------------------------------------------------------------------
# Bays (full-width spaces)
# ----------------------------------------------------------------------------
def bay_ribs(b, step=420.0, cargo_door=None):
    """Pilasters on both side walls, ceiling trusses across."""
    skip = [cargo_door] if cargo_door else []
    ribs(b.part, (b.x0, b.y1 + 15), (b.x1, b.y1 + 15), -1, b.h, step=step)
    ribs(b.part, (b.x0, b.y0 - 15), (b.x1, b.y0 - 15), 1, b.h, step=step, skip=skip)
    for x in range(int(b.x0 + step / 2), int(b.x1), int(step)):
        box(b.part, "frame", x - 14, x + 14, b.y0, b.y1, b.h - 50, b.h)


def cargo_door(b, xa, xb, side=-1, zh=None):
    """Big shutter on a side wall with chevrons, jamb lamps; drawn closed on
    the inner face of the wall (the wall itself is solid)."""
    p = b.part
    y = b.y0 if side < 0 else b.y1
    s = -side
    zh = zh or min(b.h - 120, 700)
    for (xa_, xb_) in ((xa, (xa + xb) / 2 - 2), ((xa + xb) / 2 + 2, xb)):
        box(p, "trim", xa_, xb_, y, y + s * 22, 0, zh)
        for z in range(50, int(zh), 100):
            box(p, "frame", xa_ + 16, xb_ - 16, y + s * 22, y + s * 34, z, z + 36)
    xm = (xa + xb) / 2
    for z in range(0, int(zh), 80):
        box(p, "accent", xm - 60, xm - 2, y + s * 22, y + s * 25, z, z + 40)
        box(p, "accent", xm + 2, xm + 60, y + s * 22, y + s * 25, z + 40, z + 80)
    box(p, "frame", xa - 50, xb + 50, y, y + s * 46, zh, zh + 70)
    for xj in (xa - 26, xb + 26):
        box(p, "frame", xj - 26, xj + 26, y, y + s * 46, 0, zh + 70)
        cyl(p, "trim", xj, y + s * 70, zh + 80, zh + 100, 22, 12)
        cyl("Lights", LR, xj, y + s * 70, zh + 100, zh + 130, 16, 12)
        light("R", 6, xj, y + s * 140, zh + 90)
    col(xa, xb, *sorted((y, y + s * 35)), 0, zh)


def small_craft(part, x, y, yaw, sc=1.0, slot="frame"):
    """Compact single-seat fighter parked on its gear, nose along local +X."""
    M = Mx(x, y, 0, yaw)
    L = 900 * sc
    body = [(-L * 0.5, 110 * sc), (L * 0.28, 110 * sc), (L * 0.5, 150 * sc), (L * 0.36, 210 * sc),
            (L * 0.05, 240 * sc), (-L * 0.42, 230 * sc), (-L * 0.5, 190 * sc)]
    extrude(part, slot, body, 'y', -80 * sc, 80 * sc, M)
    K[P(part)].slope_panel(R(VP), (L * 0.05, 240 * sc), (L * 0.36, 210 * sc), -55 * sc, 55 * sc,
                           lift=0.8, thick=2.0, inset=0.1, M=M)
    for s in (-1, 1):
        wing = [(-L * 0.35, 80 * sc), (L * 0.1, 80 * sc), (-L * 0.2, 440 * sc), (-L * 0.42, 440 * sc)]
        prism(part, slot, [(u, s * v) for u, v in wing] if s > 0 else [(u, s * v) for u, v in wing[::-1]],
              150 * sc, 168 * sc, M)
        box(part, "accent", -L * 0.4, -L * 0.2, s * 400 * sc, s * 440 * sc, 151 * sc, 167 * sc, M)
        e0 = M @ Vector((-L * 0.52, s * 60 * sc, 175 * sc)); e1 = M @ Vector((-L * 0.2, s * 60 * sc, 175 * sc))
        pipe(part, "trim", e0, e1, 45 * sc, 14)
        box("Lights", LB, -L * 0.53, -L * 0.52, s * 60 * sc - 30 * sc, s * 60 * sc + 30 * sc, 150 * sc, 200 * sc, M)
        pipe(part, "trim", M @ Vector((L * 0.05, s * 300 * sc, 150 * sc)), M @ Vector((L * 0.45, s * 300 * sc, 150 * sc)), 6 * sc, 8)
    extrude(part, slot, [(-L * 0.5, 230 * sc), (-L * 0.3, 230 * sc), (-L * 0.42, 380 * sc), (-L * 0.5, 380 * sc)],
            'y', -6 * sc, 6 * sc, M)
    for (gx, gy) in ((L * 0.3, 0), (-L * 0.25, 90), (-L * 0.25, -90)):
        c0 = M @ Vector((gx, gy * sc, 110 * sc)); c1 = M @ Vector((gx, gy * sc, 8))
        pipe(part, "trim", c0, c1, 7 * sc, 8)
        f = M @ Vector((gx, gy * sc, 0))
        cyl(part, "trim", f.x, f.y, 0, 14 * sc, 20 * sc, 10)
    box("Collision", CL, -L * 0.53, L * 0.5, -90 * sc, 90 * sc, 0, 250 * sc, M)


def bay_engine(b):
    """Engineering. size by height: < 560 twin drive cores along the walls;
    otherwise a reactor with tanks, gallery and (>= 900) a catwalk ring."""
    p = b.part
    x0, x1, y0, y1, h = b.x0, b.x1, b.y0, b.y1, b.h
    box(p, "grate", x0, x1, y0, y1, 0, 0.5)
    ribs(p, (x0, y1 + 15), (x1, y1 + 15), -1, h, step=380)
    ribs(p, (x1, y0 - 15), (x0, y0 - 15), -1, h, step=380)
    if h < 560:
        # twin drive cores: horizontal cylinders along each wall, walkway between
        for s in (-1, 1):
            yc = s * (b.W / 2 - 150)
            r = min(110.0, b.W / 2 - 250)
            pipe(p, "trim", (x0 + 60, yc, 150), (x1 - 150, yc, 150), r, 20)
            for k in range(int((x1 - x0 - 250) // 90)):
                xx = x0 + 100 + k * 90
                pipe(p, "frame", (xx, yc, 150), (xx + 18, yc, 150), r + 10, 20)
            for k, xx in enumerate(range(int(x0 + 200), int(x1 - 200), 300)):
                pipe("Lights", LA if S["fill"] != "B" else LB, (xx, yc, 150), (xx + 30, yc, 150), r + 3, 20)
            box(p, "trim", x0 + 60, x1 - 150, yc - 50, yc + 50, 0, 40)
            col(x0 + 40, x1 - 140, yc - r - 12, yc + r + 12, 0, 150 + r + 12)
            for xx in (x0 + 300, x1 - 400):
                pipe(p, PI, (xx, yc, 150 + r), (xx, yc, h), 12, 10)
            box(p, "accent", x0 + 60, x1 - 150, yc - s * (r + 30) - 4, yc - s * (r + 30) + 4, 0, 0.8)
        console(p, x0 + 72 + 20, 0, 0.0, n_crt=2)
        locker_bank(p, x1 - 400, x1 - 180, y1, 55, -1)
        for s in (-1, 1):
            cage_lamp(p, x1 - 15, s * 120, h - 90, 180.0)
        for k in range(max(1, int(b.L // 450))):
            xx = x0 + (k + 0.5) * b.L / max(1, int(b.L // 450))
            ceiling_panel(p, xx, 0, h, 0.0, 180)
            light("A" if k % 2 else "W", 7, xx, 0, h - 40, shadow=(k == 0))
        return
    r = min(240.0, b.W * 0.1, b.L * 0.14)
    big = h >= 900
    # big rooms keep a 400 cm aft gallery + a 200 cm bridge out to the ring
    rx, ry = (x0 + 600 + r * 2.3 if big else x0 + b.L * 0.42), 0.0
    reactor(p, rx, ry, r, h, color=LA if S["fill"] != "B" else LB)
    ring(p, "accent", rx, ry, r * 1.6 + 45, r * 1.6 + 100, 0, 0.8, 32)
    rz = min(500.0, h * 0.42)
    if big:
        ri, ro = r * 1.38, r * 2.3
        ring(p, "grate", rx, ry, ri, ro, rz - 8, rz, 48)
        ring(p, "trim", rx, ry, ro - 15, ro, rz - 30, rz - 8, 48)
        K["Collision"].annulus(CL, rx, ry, ri - 30, ro, rz - 20, rz, 48)
        K["Collision"].annulus(CL, rx, ry, ri - 30, ri + 15, rz, rz + 130, 48)
        gap = math.degrees(math.asin(115.0 / ro))
        K["Collision"].annulus(CL, rx, ry, ro - 12, ro, rz, rz + 130, 48, -180 + gap, 180 - gap)
        for rr, full in ((ri + 10, True), (ro - 8, False)):
            a0, a1 = (0.0, 360.0) if full else (-180 + gap, 180 - gap)
            pts = circle(rx, ry, rr, 48, a0, a1)
            n = len(pts) if full else len(pts) - 1
            for i in range(n):
                q0 = pts[i]; q1 = pts[(i + 1) % len(pts)]
                pipe(p, "rail", (q0[0], q0[1], rz + 105), (q1[0], q1[1], rz + 105), 3, 6)
                if i % 3 == 0:
                    pipe(p, "trim", (q0[0], q0[1], rz), (q0[0], q0[1], rz + 105), 3, 6)
        for a in (45.0, 135.0, 225.0, 315.0):
            t = math.radians(a)
            q = (rx + (ro - 20) * math.cos(t), ry + (ro - 20) * math.sin(t))
            pipe(p, "trim", (q[0], q[1], 0), (q[0], q[1], rz - 30), 12, 10)
            cyl("Collision", CL, q[0], q[1], 0, rz - 30, 16, 8)
        gx1 = x0 + 400.0
        platform(p, x0 + 15, gx1, y0 + 15, y1 - 15, rz, legs=True,
                 rails=[((gx1, y0 + 15), (gx1, -115)), ((gx1, 115), (gx1, y1 - 400))])
        solid(p, "grate", gx1 - 20, rx - ro + 40, -115, 115, rz - 10, rz)
        railing(p, (gx1, -115), (rx - ro + 20, -115), rz)
        railing(p, (gx1, 115), (rx - ro + 20, 115), rz)
        run = min(1100.0, rz * 2.2)
        stairs(p, gx1 + run, y1 - 15 - 100, run, 200, 0, rz, 180.0, rails=(True, False))
        for y in (y0 + 250, 0.0, y1 - 600):
            Mg = Mx(x0 + 72 + 15, y, rz, 0.0)
            extrude(p, "body", [(-70, 0), (0, 0), (0, 70), (-6, 76), (-44, 104), (-70, 106)], 'y', -110, 110, Mg)
            for k in range(3):
                c = Mg @ Vector((-40, -65 + k * 65, 106))
                crt(p, c.x, c.y, c.z, 56, 44, 0.0, screen=LA if k == 1 else S["screen"], d=38)
            box("Collision", CL, -72, 2, -110, 110, 0, 120, Mg)
        box(p, "trim", x0 + 15, x0 + 35, y0 + 200, y1 - 200, rz + 140, rz + 330)
        box(p, CO, x0 + 35, x0 + 37, y0 + 215, y1 - 215, rz + 155, rz + 315)
    else:
        # gallery on the starboard wall, its stair coming down toward the bow
        gw = 260.0
        gy = y0 + 15 + gw
        run = rz * 1.4
        xg1 = x1 - 15 - run - 40
        platform(p, x0 + 15, xg1, y0 + 15, gy, rz, legs=True, rails=[((x0 + 15, gy), (xg1, gy))])
        stairs(p, xg1 + run, (y0 + 15 + gy) / 2, run, gw - 40, 0, rz, 180.0, rails=(True, False))
        for k in range(max(1, min(2, int((xg1 - x0 - 100) // 300)))):
            console(p, x0 + 200 + k * 300, y0 + 15 + 72, 90.0, n_crt=2, z=rz, chair_too=True)
    # tanks along the port wall
    nt = max(2, int((b.L * 0.7) // 380))
    for k in range(nt):
        tx = x0 + 200 + k * (b.L * 0.75) / nt
        if abs(tx - rx) < r * 1.6 + 200 and b.W < 1800:
            continue
        ty = y1 - 15 - 160 - (230 if big else 0)
        tank(p, tx, ty, 130, min(h - 160, 700))
        q = Vector((tx, ty - 130, 320)); rr_ = Vector((rx, ry, 320))
        d = rr_ - q; d.z = 0
        if d.length > r * 1.6 + 60:
            end = rr_ - d.normalized() * (r * 1.6 + 20)
            pipe(p, PI, q, end, 16, 12)
    for (ex, ey) in ((x0 + 15, 0.0), (rx, y1), (rx, y0), (x1 - 15, 0.0)):
        q = Vector((ex, ey, h - 150)); rr_ = Vector((rx, ry, h - 150))
        d = q - rr_; d.z = 0
        pipe(p, PI, rr_ + d.normalized() * r, q, 30, 14)
    yc_ = b.door_w / 2 + 190
    if yc_ + 100 < b.W / 2 - 330:
        console(p, x1 - 72 - 15, yc_, 180.0, n_crt=2)
    console(p, x1 - 72 - 15, -yc_, 180.0, n_crt=2, screen=S["screen2"])
    for a in (0.0, 90.0, 180.0, 270.0):
        t = math.radians(a + 45)
        light("A" if S["fill"] != "B" else "B", 12 if big else 9,
              rx + (r * 2.8) * math.cos(t), ry + (r * 2.8) * math.sin(t), min(380.0, h * 0.5), shadow=(a == 0.0))
    for y in (y0 + 300, y1 - 300):
        flood(p, x1 - 250, y, h, color=LW)
        light("W", 10 if big else 8, x1 - 250, y, h - 250)
    for (x, y, yaw) in ((x1 - 15, 500, 180.0), (x1 - 15, -500, 180.0), (x0 + 15, y1 - 200, 0.0), (x0 + 15, y0 + 200, 0.0)):
        cage_lamp(p, x, y, min(300.0, h - 80), yaw)
    view("engine", (x1 - 180, y0 + 250, 165), (rx, ry + 100, h * 0.45), 14)


def bay_hold(b):
    """Cargo hold: container stacks either side of a central lane, gantry
    crane, cargo door, catwalk + stairs along one wall, loader."""
    p = b.part
    x0, x1, y0, y1, h = b.x0 + 15, b.x1 - 15, b.y0 + 15, b.y1 - 15, b.h
    lane = 220.0
    cd = (b.x0 + b.L * 0.35, b.x0 + b.L * 0.35 + min(900.0, b.L * 0.35))
    bay_ribs(b, cargo_door=(cd[0] - b.x0 - 80, cd[1] - b.x0 + 80))
    cargo_door(b, cd[0], cd[1], side=-1, zh=min(h - 150, 650))
    box(p, "accent", x0 + 250, x1 - 250, -lane - 10, -lane, 0, 0.6)
    box(p, "accent", x0 + 250, x1 - 250, lane, lane + 10, 0, 0.6)
    for gx in range(int(x0 + 150), int(x1 - 100), 300):
        for gy in (-lane - 150, lane + 150):
            cyl(p, "trim", gx, gy, 0, 1.5, 9, 8)
    # container rows (port side full length, starboard around the door)
    L = 610.0 if b.L > 2000 else 300.0
    tiers = 2 if h >= 700 else 1
    palette = ["accent", "frame", "body", "accent", "trim"]
    k = 0
    for side, yc in ((1, lane + 150 + 122), (-1, -lane - 150 - 122)):
        extra = []
        if b.W > 1500:
            extra = [yc + side * 260]
        for yy in [yc] + extra:
            if abs(yy) + 122 > b.W / 2 - 20:
                continue
            x = x0 + 300
            while x + L < x1 - 350:
                if side < 0 and cd[0] - 150 < x + L and x < cd[1] + 150 and abs(yy) > lane + 400:
                    x += 100
                    continue
                if side < 0 and cd[0] - 150 < x + L and x < cd[1] + 150:
                    x += 100
                    continue
                for t in range(tiers):
                    if t and (k % 3 == 2):
                        break
                    container(p, x + L / 2, yy, t * 267, L - 20, 0.0, slot=palette[k % len(palette)],
                              collide=(t == 0))
                    k += 1
                x += L + 20
    # gantry crane on rails along both walls
    zr = h - 110
    for yy in (y0 + 40, y1 - 40):
        box(p, "frame", x0 + 100, x1 - 100, yy - 25, yy + 25, zr - 20, zr)
        box(p, "trim", x0 + 100, x1 - 100, yy - 6, yy + 6, zr, zr + 20)
    xg = b.x0 + b.L * 0.6
    box(p, "accent", xg - 45, xg + 45, y0 + 40, y1 - 40, zr + 20, zr + 75)
    box(p, "frame", xg - 50, xg + 50, y0 + 40, y1 - 40, zr + 10, zr + 20)
    box(p, "body", xg - 90, xg + 90, -80, 80, zr - 80, zr + 10)
    for s in (-1, 1):
        pipe(p, "trim", (xg + s * 30, 0, zr - 80), (xg + s * 30, 0, 330), 2.5, 6)
    box(p, "accent", xg - 140, xg + 140, -130, 130, 300, 330)
    for s in (-1, 1):
        box(p, "trim", xg + s * 140 - 8, xg + s * 140 + 8, -130, 130, 260, 300)
    # catwalk along the port wall + stairs
    cz = min(420.0, h * 0.5)
    cw = 170.0
    platform(p, x0 + 400, x1 - 20, y1 - cw, y1, cz, legs=True, rails=["y0"])
    run = cz * 2.1
    stairs(p, x0 + 400 - run, y1 - cw / 2, run, cw - 10, 0, cz, 0.0, rails=(True, False))
    box(p, "accent", x0 + 400 - run - 60, x0 + 400 - run, y1 - cw, y1, 0, 0.6)
    console(p, x1 - 20 - 72, y1 - cw / 2, 180.0, n_crt=2, z=cz, chair_too=False)
    # loader exo-frame parked by the aft wall
    lx, ly = x0 + 140, -lane - 60
    box(p, "accent", lx - 50, lx + 50, ly - 60, ly + 60, 0, 30)
    for s in (-1, 1):
        box(p, "trim", lx - 20, lx + 20, ly + s * 40 - 10, ly + s * 40 + 10, 30, 150)
        box(p, "accent", lx - 30, lx + 30, ly + s * 70 - 12, ly + s * 70 + 12, 150, 180)
        box(p, "trim", lx + 30, lx + 110, ly + s * 70 - 8, ly + s * 70 + 8, 160, 170)
        box(p, "trim", lx + 100, lx + 140, ly + s * 70 - 20, ly + s * 70 + 20, 130, 175)
    box(p, "frame", lx - 40, lx + 30, ly - 55, ly + 55, 150, 240)
    pipe(p, "trim", (lx + 30, ly - 55, 240), (lx + 30, ly + 55, 240), 4, 8)
    col(lx - 55, lx + 145, ly - 90, ly + 90, 0, 240)
    # lights
    nf = max(2, int(b.L // 700))
    for k in range(nf):
        x = b.x0 + (k + 0.5) * b.L / nf
        for y in (-lane - 400, lane + 400) if b.W > 1300 else (0.0,):
            flood(p, x, y, h - 50)
        light("A" if S["fill"] == "W" else S["fill"], 14 if h > 700 else 11, x, 0, h - 220, shadow=(k == nf // 2))
    for x in (x0 + 500, x1 - 500):
        cage_lamp(p, x, y1 - cw - 2, cz + 200, -90.0, color=LA)
    view("hold", (x1 - 200, -lane / 2, 165), (x0 + 300, lane, 250), 14)
    view("hold_catwalk", (x1 - 150, y1 - cw / 2, cz + 165), (x0 + 400, y0 + 300, 150), 14)


def bay_tankhold(b):
    """Bulk hold: two rows of big vertical tanks either side of a central
    lane with the manifold, a mid-height gantry between the rows (stair at the
    aft end), pumps and valve stations along the lane."""
    p = b.part
    x0, x1, h = b.x0 + 15, b.x1 - 15, b.h
    bay_ribs(b)
    th = h - 220
    lane = 230.0
    r = min(230.0, (b.W / 2 - lane - 120) / 2)
    gz = min(420.0, th * 0.55)
    run = gz * 1.4
    xs = []
    x = x0 + 300 + run
    while x + 2 * r < x1 - 250:
        xs.append(x + r)
        x += 2 * r + 110
    for side in (-1, 1):
        yc = side * (lane + 80 + r)
        for i, xt in enumerate(xs):
            tank(p, xt, yc, r, th, bands=4)
            cyl("Lights", LG if i % 2 else LA, xt, yc - side * (r + 2), th * 0.6, th * 0.6 + 8, 6, 8)
            pipe(p, PI, (xt, yc - side * r, 80), (xt, side * (lane + 20), 80), 14, 12)
            cyl(p, "accent", xt, side * (lane + 40), 60, 100, 16, 10)
        # manifold along the lane edge
        pipe(p, PI, (x0 + 200, side * (lane + 20), 80), (x1 - 150, side * (lane + 20), 80), 24, 14)
        pipe(p, "trim", (x0 + 200, side * (lane + 20), 150), (x1 - 150, side * (lane + 20), 150), 8, 10)
        col(x0 + 200, x1 - 150, side * (lane + 20) - 30, side * (lane + 20) + 30, 0, 160)
    # gantry down the lane at gz, stair up from the aft end (lane stays walkable under it)
    platform(p, x0 + 300 + run, x1 - 250, -110, 110, gz, legs=True, rails=["y0", "y1", "x1"])
    stairs(p, x0 + 300, 0.0, run, 180, 0, gz, 0.0, rails=(True, True))
    for xt in xs:
        for side in (-1, 1):
            # spur out to each tank's valve deck
            ya, yb = side * 110, side * (lane + 80 + r * 0.35)
            solid(p, "grate", xt - 45, xt + 45, *sorted((ya, yb)), gz - 10, gz)
            railing(p, (xt - 45, ya), (xt - 45, yb), gz)
            railing(p, (xt + 45, ya), (xt + 45, yb), gz)
    # pumps between the tanks, cross-feeds overhead
    for i in range(len(xs) - 1):
        xm = (xs[i] + xs[i + 1]) / 2
        for side in (-1, 1):
            cyl(p, "body", xm, side * (lane - 50), 0, 90, 40, 12)
            pipe(p, "trim", (xm, side * (lane - 50), 90), (xm, side * (lane - 50), 240), 8, 8)
            box(p, "accent", xm - 30, xm + 30, side * (lane - 95), side * (lane - 5), 30, 60)
            col(xm - 45, xm + 45, *sorted((side * (lane - 95), side * (lane - 5))), 0, 100)
        pipe(p, PI, (xm, -(lane - 50), 240), (xm, lane - 50, 240), 8, 8)
    nf = max(2, int(b.L // 800))
    for k in range(nf):
        x = b.x0 + (k + 0.5) * b.L / nf
        flood(p, x, 0, h - 20)
        light("W", 14, x, 0, h - 200, shadow=(k == 0))
        light("A", 8, x, 0, 150)
    view("tanks", (x1 - 150, 0, 165), (x0 + 400, -200, gz * 0.8), 14)
    view("tanks_gantry", (x1 - 350, 0, gz + 165), (x0 + 300, 300, gz - 100), 14)


def bay_salvage(b):
    """Salvage bay: claw crane holding a wreck section over the port half,
    scrap piles, cutting station and sorting bins to starboard; the centre
    lane between the doors stays clear."""
    p = b.part
    x0, x1, y0, y1, h = b.x0 + 15, b.x1 - 15, b.y0 + 15, b.y1 - 15, b.h
    lane = b.door_w / 2 + 80
    cd = (b.cx - b.L * 0.2, b.cx + b.L * 0.2)
    bay_ribs(b, cargo_door=(cd[0] - b.x0 - 80, cd[1] - b.x0 + 80))
    cargo_door(b, cd[0], cd[1], side=-1, zh=min(h - 150, 600))
    # wreck section hanging in the claw over the port half
    wx, wy, wz = b.cx, (lane + y1) / 2 + 20, 200.0
    M = Mx(wx, wy, wz, 18.0)
    wl, ww = min(260.0, b.L * 0.2), min(150.0, (y1 - lane) / 2 - 60)
    box(p, "frame", -wl, wl, -ww, ww, 0, 200, M)
    extrude(p, "trim", [(-ww, 200), (ww, 200), (ww * 0.66, 260), (-ww * 0.4, 250)], 'x', -wl + 10, wl * 0.7, M)
    for k in range(5):
        xx = -wl + 20 + k * (2 * wl - 40) / 4
        box(p, "trim", xx - 8, xx + 8, -ww - 5, ww + 5, 0, 205, M)
    box("Lights", LA, wl - 20, wl + 2, -ww * 0.5, ww * 0.15, 60, 140, M)
    zr = h - 110
    for yy in (y0 + 40, y1 - 40):
        box(p, "frame", x0 + 100, x1 - 100, yy - 25, yy + 25, zr - 20, zr)
    box(p, "accent", wx - 45, wx + 45, y0 + 40, y1 - 40, zr, zr + 55)
    box(p, "body", wx - 80, wx + 80, wy - 70, wy + 70, zr - 90, zr)
    pipe(p, "trim", (wx, wy, zr - 90), (wx, wy, wz + 330), 12, 10)
    for a in (0.0, 120.0, 240.0):
        t = math.radians(a)
        c0 = Vector((wx, wy, wz + 330)); c1 = Vector((wx + 190 * math.cos(t), wy + 120 * math.sin(t), wz + 240))
        pipe(p, "accent", c0, c1, 12, 8)
        pipe(p, "accent", c1, c1 + Vector((0, 0, -120)), 10, 8)
    col(wx - wl - 40, wx + wl + 40, max(lane, wy - ww - 80), min(y1, wy + ww + 80), 0, wz + 260)
    import random
    rnd = random.Random(7)

    def pile(px, py, rx_, ry_):
        for i in range(14):
            sx, sy, sz = rnd.uniform(40, 140), rnd.uniform(30, 110), rnd.uniform(15, 70)
            ox, oy = rnd.uniform(-rx_ + 60, rx_ - 60), rnd.uniform(-ry_ + 50, ry_ - 50)
            zz = max(0.0, 90 - (ox * ox + oy * oy) ** 0.5 * 0.5) * rnd.uniform(0.3, 1.0)
            box(p, ("frame", "trim", "accent", "body")[i % 4], -sx / 2, sx / 2, -sy / 2, sy / 2, 0, sz,
                Mx(px + ox, py + oy, zz, rnd.uniform(0, 90)))
        for i in range(3):
            pipe(p, PI, (px - rx_ + 40 + i * 50, py - ry_ + 40, 10 + i * 20),
                 (px + rx_ - 60 - i * 40, py + ry_ - 40, 40), 14, 10)
        col(px - rx_, px + rx_, py - ry_, py + ry_, 0, 140)
    ry_ = min(160.0, (-lane - y0) / 2 - 20)
    pile(x0 + 320, -lane - 20 - ry_, 200, ry_)
    pile(x1 - 330, (lane + y1) / 2, 180, min(150.0, (y1 - lane) / 2 - 40))
    # cutting station to starboard
    cx_, cy_ = b.cx + b.L * 0.12, -lane - 20 - min(95.0, ry_)
    box(p, "trim", cx_ - 150, cx_ + 150, cy_ - 90, cy_ + 90, 0, 80)
    box(p, GR, cx_ - 150, cx_ + 150, cy_ - 90, cy_ + 90, 80, 85)
    for q in (-1, 1):
        pipe(p, "trim", (cx_ + q * 170, cy_, 0), (cx_ + q * 170, cy_, 200), 8, 8)
        pipe(p, "accent", (cx_ + q * 170, cy_, 200), (cx_ + q * 60, cy_, 150), 6, 8)
        pipe(p, "trim", (cx_ + q * 60, cy_, 150), (cx_ + q * 50, cy_, 100), 3, 6)
        box("Lights", LC, cx_ + q * 50 - 3, cx_ + q * 50 + 3, cy_ - 3, cy_ + 3, 94, 100)
    col(cx_ - 180, cx_ + 180, cy_ - 95, cy_ + 95, 0, 110)
    # sorting bins against the starboard wall, forward of the cargo door
    for k in range(3):
        bx = cd[1] + 120 + k * 230
        if bx + 110 > x1 - 150:
            break
        box(p, ("accent", "frame", "body")[k], bx - 100, bx + 100, y0, y0 + 180, 0, 110)
        box(p, "trim", bx - 104, bx + 104, y0, y0 + 184, 110, 116)
        col(bx - 104, bx + 104, y0, y0 + 184, 0, 116)
    console(p, x1 - 72 - 15, y1 - 200, 180.0, n_crt=2)
    box(p, "accent", x0 + 200, x1 - 200, -lane - 8, -lane, 0, 0.6)
    box(p, "accent", x0 + 200, x1 - 200, lane, lane + 8, 0, 0.6)
    nf = max(2, int(b.L // 600))
    for k in range(nf):
        x = b.x0 + (k + 0.5) * b.L / nf
        flood(p, x, 0, h - 50)
        light("A", 13, x, 0, h - 220, shadow=(k == 0))
    light("C", 4, cx_, cy_, 160)
    view("salvage", (x1 - 150, -80, 165), (wx, wy, wz + 100), 14)


def bay_boarding(b):
    """Boarding bay: breaching pods on launch cradles over floor hatches,
    trooper benches, red launch lamps."""
    p = b.part
    x0, x1, y0, y1, h = b.x0 + 15, b.x1 - 15, b.y0 + 15, b.y1 - 15, b.h
    bay_ribs(b, step=380)
    n = max(2, int((b.L - 600) // 450))
    for i in range(n):
        px = x0 + 350 + (i + 0.5) * (b.L - 700) / n
        for side in (-1, 1):
            py = side * (b.W / 2 - 280)
            # floor hatch ring
            ring(p, "accent", px, py, 130, 150, 0, 0.8, 24)
            cyl(p, "trim", px, py, 0, 0.5, 130, 24)
            # pod: tall capsule with a hatch, on a cradle
            cyl(p, "frame", px, py, 30, 290, 95, 20)
            K[P(p)].ico(R("frame"), (px, py, 290), 95, 2)
            cyl(p, "trim", px, py, 0, 30, 110, 20)
            M = Mx(px, py, 0, 90.0 if side < 0 else -90.0)
            box(p, "trim", 92, 98, -48, 48, 55, 235, M)
            box(p, VP, 96, 100, -20, 20, 190, 220, M)
            box(p, "accent", 97, 99, -48, 48, 45, 55, M)
            for s in (-1, 1):
                pipe(p, "trim", (px + s * 110, py, 0), (px + s * 110, py, h - 60), 10, 10)
            box(p, "frame", px - 130, px + 130, py - 20, py + 20, h - 80, h - 50)
            cyl("Lights", LR, px, py - side * 20, h - 100, h - 80, 14, 10)
            cyl("Collision", CL, px, py, 0, 400, 118, 12)
        light("R", 6, px, 0, h - 120, shadow=(i == 0))
    # benches down the centre lane
    for x in range(int(x0 + 500), int(x1 - 400), 500):
        for s in (-1, 1):
            box(p, "trim", x - 150, x + 150, s * 120 - 22, s * 120 + 22, 38, 45)
            box(p, "frame", x - 148, x + 148, s * 120 - 20, s * 120 + 20, 45, 50)
            col(x - 150, x + 150, s * 120 - 24, s * 120 + 24, 0, 60)
    for k, x in enumerate(range(int(x0 + 400), int(x1 - 200), 600)):
        flood(p, x, 0, h, color=LW if k % 2 else LR)
    fill(10, b.cx, 0, h - 150)
    fill(10, x0 + 300, 0, h - 150)
    locker_bank(p, x1 - 500, x1 - 150, y1, 55, -1)
    view("boarding", (x1 - 150, 0, 165), (x0 + 200, -300, 200), 14)


def bay_torpedo(b):
    """Torpedo room: torpedoes racked on both walls, loading cradle on a
    floor rail to a bank of tube breeches on the forward bulkhead."""
    p = b.part
    x0, x1, y0, y1, h = b.x0 + 15, b.x1 - 15, b.y0 + 15, b.y1 - 15, b.h
    bay_ribs(b, step=380)
    TL = min(560.0, b.L * 0.5)
    for side in (-1, 1):
        yy = side * (b.W / 2 - 110)
        for j in range(3):
            z = 50 + j * 95
            if z > h - 120:
                break
            for q in (-1, 1):
                pipe(p, "frame", (x0 + 200, yy + q * 45, z), (x0 + 200 + TL - 80, yy + q * 45, z), 30, 14)
                pipe(p, "accent", (x0 + 200 + TL - 80, yy + q * 45, z), (x0 + 200 + TL, yy + q * 45, z), 22, 14)
                box(p, "trim", x0 + 190, x0 + 200, yy + q * 45 - 25, yy + q * 45 + 25, z - 25, z + 25)
        for xx in (x0 + 260, x0 + 200 + TL - 100):
            box(p, "trim", xx - 12, xx + 12, yy - 90, yy + 90, 0, 330)
        col(x0 + 180, x0 + 200 + TL, yy - 90, yy + 90, 0, 330)
    # floor rail and loading cradle to the tube breeches
    yr = -(b.door_w / 2 + 170)
    for s in (-1, 1):
        box(p, "trim", x0 + 150, x1 - 110, yr + s * 40 - 5, yr + s * 40 + 5, 0, 4)
    cx_ = b.cx + 150
    box(p, "accent", cx_ - 120, cx_ + 120, yr - 60, yr + 60, 4, 40)
    for q in (-1, 1):
        box(p, "trim", cx_ + q * 100 - 10, cx_ + q * 100 + 10, yr - 50, yr + 50, 40, 80)
    pipe(p, "frame", (cx_ - 200, yr, 80), (cx_ + 150, yr, 80), 30, 14)
    col(cx_ - 210, cx_ + 160, yr - 65, yr + 65, 0, 115)
    for s in (-1, 1):
        for k in range(2):
            yy = s * (b.door_w / 2 + 170 + k * 180)
            if abs(yy) + 60 > b.W / 2 - 200:
                continue
            zz = 110
            pipe(p, "frame", (x1 - 90, yy, zz), (x1, yy, zz), 50, 16)
            pipe(p, "trim", (x1 - 100, yy, zz), (x1 - 90, yy, zz), 58, 16)
            box("Lights", LG if k % 2 else LR, x1 - 102, x1 - 100, yy + 30, yy + 44, zz + 40, zz + 52)
            col(x1 - 100, x1, yy - 60, yy + 60, 0, 170)
    zr = h - 80
    box(p, "frame", x0 + 100, x1 - 100, -12, 12, zr - 20, zr)
    box(p, "body", b.cx - 50, b.cx + 50, -30, 30, zr - 70, zr - 20)
    pipe(p, "trim", (b.cx, 0, zr - 70), (b.cx, 0, 220), 2.5, 6)
    console(p, x0 + 72 + 20, b.door_w / 2 + 160, 0.0, n_crt=2)
    for k, x in enumerate(range(int(x0 + 300), int(x1), 500)):
        ceiling_panel(p, x, 0, h, 0.0, 200)
        light("R" if k % 2 else "W", 8, x, 0, h - 60, shadow=(k == 0))
    view("torpedo", (x0 + 200, -100, 165), (x1, 0, 120), 16)


def bay_astro(b):
    """Astrometrics / observation: holo globe on a sunken floor ring of
    consoles, window bands in both hull walls (built by the layout)."""
    p = b.part
    x0, x1, y0, y1, h = b.x0 + 15, b.x1 - 15, b.y0 + 15, b.y1 - 15, b.h
    cx_, cy_ = b.cx, 0.0
    R_ = min(b.L, b.W) * 0.28
    holo_sphere(p, cx_, cy_, min(160.0, R_ * 0.5), min(260.0, h * 0.5))
    n = 6
    for k in range(n):
        a = 360.0 * k / n + 30
        t = math.radians(a)
        console(p, cx_ + R_ * math.cos(t), cy_ + R_ * math.sin(t), a + 180, n_crt=2, flat=True)
    ring(p, "accent", cx_, cy_, R_ + 90, R_ + 100, 0, 0.8, 48)
    for side in (-1, 1):
        yb = side * (b.W / 2 - 15)
        for x in range(int(x0 + 300), int(x1 - 200), 500):
            box(p, "frame", x - 90, x + 90, yb - side * 60, yb, 0, 45)
            box(p, "soft", x - 86, x + 86, yb - side * 56, yb - side * 4, 45, 50)
            col(x - 90, x + 90, *sorted((yb - side * 60, yb)), 0, 55)
    for k in range(8):
        a = math.radians(k * 45 + 22.5)
        ceiling_panel(p, cx_ + (R_ + 150) * math.cos(a), cy_ + (R_ + 150) * math.sin(a), h, math.degrees(a) + 90, 120)
    light("C" if S["screen"] == LC else "B", 8, cx_, cy_, h - 60)
    fill(9, x0 + 300, 0, h - 60)
    fill(9, x1 - 300, 0, h - 60)
    view("astrometrics", (x1 - 150, y0 + 200, 175), (cx_, cy_, 200), 14)


def bay_refinery(b):
    """Ore processing: hopper/crusher, conveyor, smelter with a glowing
    mouth, ore bins, control booth on a mezzanine."""
    p = b.part
    x0, x1, y0, y1, h = b.x0 + 15, b.x1 - 15, b.y0 + 15, b.y1 - 15, b.h
    bay_ribs(b)
    yc = -b.W * 0.22
    # hopper + crusher
    hx = x0 + 300
    box(p, "accent", hx - 180, hx + 180, yc - 180, yc + 180, 0, 260)
    extrude(p, "frame", [(yc - 250, 520), (yc + 250, 520), (yc + 120, 260), (yc - 120, 260)], 'x', hx - 220, hx + 220)
    for s in (-1, 1):
        pipe(p, "trim", (hx - 100, yc + s * 80, 180), (hx + 100, yc + s * 80, 180), 40, 14)
    col(hx - 220, hx + 220, yc - 250, yc + 250, 0, 520)
    # conveyor to the smelter
    sx = x1 - 420
    zc = 110.0
    box(p, "trim", hx + 180, sx - 200, yc - 55, yc + 55, zc - 10, zc)
    box(p, "body", hx + 180, sx - 200, yc - 48, yc + 48, zc, zc + 4)
    for s in (-1, 1):
        box(p, "accent", hx + 180, sx - 200, yc + s * 55 - 4, yc + s * 55 + 4, zc - 10, zc + 16)
    for x in range(int(hx + 200), int(sx - 200), 160):
        for s in (-1, 1):
            pipe(p, "trim", (x, yc + s * 50, 0), (x, yc + s * 50, zc - 10), 4, 6)
    import random
    rnd = random.Random(3)
    for i in range(40):
        x = rnd.uniform(hx + 200, sx - 220)
        K[P(p)].ico(R("body"), (x, yc + rnd.uniform(-35, 35), zc + 12), rnd.uniform(8, 16), 1)
    col(hx + 180, sx - 200, yc - 60, yc + 60, 0, zc + 30)
    # smelter
    cyl(p, "trim", sx, yc, 0, min(h - 150, 600), 200, 24)
    ring(p, "frame", sx, yc, 196, 215, 150, 175, 24)
    ring(p, "frame", sx, yc, 196, 215, 400, 425, 24)
    box(p, "frame", sx - 230, sx - 190, yc - 80, yc + 80, 80, 200)
    box("Lights", LA, sx - 232, sx - 230, yc - 64, yc + 64, 95, 185)
    pipe(p, PI, (sx, yc, min(h - 150, 600)), (sx, yc, h), 40, 14)
    cyl("Collision", CL, sx, yc, 0, h, 220, 16)
    light("A", 8, sx - 330, yc, 180, shadow=True)
    # ingot pallets + ore bins
    for k in range(3):
        crate(p, sx - 100 + k * 150, yc + 350, 0, 120, 90, 40, body="trim", lid="accent")
    for k in range(3):
        bx = x0 + 250 + k * 320
        by = b.W * 0.25
        box(p, "accent", bx - 140, bx + 140, by - 110, by + 110, 0, 140)
        box(p, "trim", bx - 144, bx + 144, by - 114, by + 114, 140, 148)
        for i in range(10):
            K[P(p)].ico(R("body"), (bx + rnd.uniform(-100, 100), by + rnd.uniform(-70, 70), 145), rnd.uniform(15, 30), 1)
        col(bx - 144, bx + 144, by - 114, by + 114, 0, 150)
    # control booth on a mezzanine along the port wall, stairs up
    mz = min(420.0, h * 0.45)
    bx0, bx1 = x1 - 900, x1 - 20
    platform(p, bx0, bx1, y1 - 320, y1, mz, legs=True, rails=[])
    railing(p, (bx0, y1 - 320), (bx1 - 250, y1 - 320), mz)
    railing(p, (bx0, y1 - 320), (bx0, y1 - 20), mz)
    window_band(p, (bx1 - 250, y1 - 320), (bx1, y1 - 320), mz, mz + 250, mz + 100, mz + 210, mull=120, t=12)
    seg_box(p, "wall", (bx1 - 250, y1 - 320), (bx1 - 250, y1 - 190), 12, mz, mz + 250)
    seg_box("Collision", CL, (bx1 - 250, y1 - 320), (bx1 - 250, y1 - 190), 12, mz, mz + 250)
    box(p, "wall", bx1 - 250, bx1, y1 - 320, y1, mz + 250, mz + 262)
    console(p, bx1 - 125, y1 - 320 + 90, -90.0, n_crt=2, z=mz)
    run = mz * 2.0
    stairs(p, bx0 - run, y1 - 90, run, 150, 0, mz, 0.0, rails=(True, False))
    nf = max(2, int(b.L // 700))
    for k in range(nf):
        x = b.x0 + (k + 0.5) * b.L / nf
        flood(p, x, 0, h - 40)
        light("W", 14, x, 0, h - 200, shadow=(k == 0))
    light("W", 5, bx1 - 125, y1 - 160, mz + 200)
    view("refinery", (x1 - 200, 150, 165), (hx, yc, 250), 14)
    view("refinery_booth", (bx1 - 120, y1 - 100, mz + 165), (hx, yc, 100), 16)


def bay_hangar(b):
    """Hangar: small craft on pads, launch door, catwalk + control booth
    window, floods, tug, ordnance carts."""
    p = b.part
    x0, x1, y0, y1, h = b.x0 + 15, b.x1 - 15, b.y0 + 15, b.y1 - 15, b.h
    n = b.opts.get("craft", 2)
    cd = (b.x0 + b.L * 0.2, b.x0 + b.L * 0.2 + min(1400.0, b.L * 0.55))
    bay_ribs(b, cargo_door=(cd[0] - b.x0 - 80, cd[1] - b.x0 + 80))
    cargo_door(b, cd[0], cd[1], side=-1, zh=min(h - 200, 800))
    # craft sit nose-to-the-door, wingtip to wingtip along X (span 880 * sc)
    avail = b.L - 400 - 260 - 300
    sc = min(b.opts.get("craft_scale", 1.0), avail / (n * 1050.0))
    step = 1050.0 * sc
    xs0 = x0 + 400 + (avail - n * step) / 2 + step / 2
    pads = []
    for i in range(n):
        px = xs0 + i * step
        py = -b.W * 0.08
        pads.append((px, py))
        ring(p, "accent", px, py, 470 * sc, 500 * sc, 0, 0.6, 40)
        small_craft(p, px, py, -90.0, sc=sc)
    for gx in range(int(x0 + 150), int(x1 - 100), 300):
        for gy in range(int(y0 + 150), int(y1 - 100), 300):
            cyl(p, "trim", gx, gy, 0, 1.5, 9, 8)
    # catwalk along the forward wall + stairs down the port wall
    cz = min(600.0, h * 0.5)
    cw = 260.0
    platform(p, x1 - cw, x1, y0 + 400, y1, cz, legs=True, rails=["x0", "y0"])
    run = cz * 1.9
    stairs(p, x1 - cw - run, y1 - 110, run, 200, 0, cz, 0.0, rails=(True, False))
    console(p, x1 - cw + 72, y0 + 700, 0.0, n_crt=2, z=cz)
    # tug + ordnance carts
    tx, ty = x0 + 300, y1 - 400
    box(p, "accent", tx - 200, tx + 200, ty - 110, ty + 110, 30, 110)
    box(p, "frame", tx - 180, tx - 40, ty - 100, ty + 100, 110, 200)
    box(p, VP, tx - 42, tx - 38, ty - 90, ty + 90, 130, 190)
    for sx in (-130, 130):
        for sy in (-1, 1):
            pipe(p, "trim", (tx + sx, ty + sy * 100, 32), (tx + sx, ty + sy * 125, 32), 32, 12)
    col(tx - 205, tx + 205, ty - 130, ty + 130, 0, 200)
    for k in range(2):
        mx, my = x0 + 250, -200 - k * 300
        if my - 60 < y0:
            continue
        box(p, "trim", mx - 150, mx + 150, my - 50, my + 50, 20, 40)
        for j in range(3):
            yy = my - 30 + j * 30
            pipe(p, "frame", (mx - 140, yy, 60), (mx + 120, yy, 60), 12, 10)
            pipe(p, "accent", (mx + 120, yy, 60), (mx + 150, yy, 60), 8, 10)
        col(mx - 155, mx + 155, my - 60, my + 60, 0, 90)
    for zz, r in ((h - 300, 22.0), (h - 240, 14.0)):
        pipe(p, PI, (x0, y1 - 40, zz), (x1 - 300, y1 - 40, zz), r, 12)
    if b.L > 1800:
        locker_bank(p, x0 + 800, x0 + 1300, y1, 60, -1)
    nf = max(2, int(b.L // 800))
    for k in range(nf):
        x = b.x0 + (k + 0.5) * b.L / nf
        for y in (-b.W * 0.25, b.W * 0.25):
            flood(p, x, y, h - 60)
        light("A", 16, x, 0, h - 200, shadow=(k == nf // 2))
    light("W", 6, x1 - cw / 2, 0, cz + 280)
    for y in (y0 + 600, 0.0, y1 - 600):
        cage_lamp(p, x1 - cw - 2, y, cz + 200, 180.0, color=LA)
    view("hangar", (x1 - cw - 200, y1 - 300, 165), (pads[0][0], pads[0][1], 250), 14)
    view("hangar_catwalk", (x1 - cw / 2, y0 + 500, cz + 165), (pads[-1][0], pads[-1][1] + 200, 150), 14)


def bay_atrium(b):
    """Two-storey atrium (luxury): balcony gallery on three sides, grand
    stair, fountain, bar, planters, chandelier ring; windows by the layout."""
    p = b.part
    x0, x1, y0, y1, h = b.x0 + 15, b.x1 - 15, b.y0 + 15, b.y1 - 15, b.h
    gz = min(420.0, h * 0.5)
    gw = 280.0
    # balcony: port side + both ends (the starboard side stays open to the windows)
    platform(p, x0, x1, y1 - gw, y1, gz, legs=False, slot="floor")
    platform(p, x0, x0 + gw, y0 + 200, y1 - gw, gz, legs=False, slot="floor")
    platform(p, x1 - gw, x1, y0 + 200, y1 - gw, gz, legs=False, slot="floor")
    box(p, "frame", x0, x1, y1 - gw - 12, y1 - gw, gz - 60, gz)
    for (a, c) in (((x0 + gw, y1 - gw), (x1 - gw, y1 - gw)), ((x0 + gw, y0 + 200), (x0 + gw, y1 - gw)),
                   ((x1 - gw, y0 + 200), (x1 - gw, y1 - gw)), ((x0, y0 + 200), (x0 + gw, y0 + 200)),
                   ((x1 - gw, y0 + 200), (x1, y0 + 200))):
        seg_box(p, VP, a, c, 3, gz, gz + 100)
        seg_box(p, "rail", a, c, 10, gz + 100, gz + 108)
        seg_box("Collision", CL, a, c, 8, gz, gz + 125)
    for x in range(int(x0 + gw), int(x1 - gw) + 1, 400):
        cyl(p, "trim", x, y1 - gw - 6, 0, gz - 60, 18, 16)
        cyl(p, "rail", x, y1 - gw - 6, 0, 20, 24, 16)
        cyl("Collision", CL, x, y1 - gw - 6, 0, gz, 24, 8)
    for (xa, ya) in ((x0 + gw, y0 + 200), (x1 - gw, y0 + 200)):
        cyl(p, "trim", xa, ya, 0, gz, 18, 16)
        cyl("Collision", CL, xa, ya, 0, gz, 24, 8)
    # grand stair up to the port balcony, centred
    run = gz * 2.1
    stairs(p, b.cx - run / 2, y1 - gw - 170, run, 300, 0, gz, 0.0, rails=(True, True))
    solid(p, "floor", b.cx + run / 2 - 5, b.cx + run / 2 + 170, y1 - gw - 320, y1 - gw, gz - 10, gz)
    seg_box(p, VP, (b.cx + run / 2 + 170, y1 - gw - 320), (b.cx + run / 2 + 170, y1 - gw), 3, gz, gz + 100)
    seg_box("Collision", CL, (b.cx + run / 2 + 170, y1 - gw - 320), (b.cx + run / 2 + 170, y1 - gw), 8, gz, gz + 125)
    # fountain
    fx, fy = b.cx, -b.W * 0.12
    ring(p, "rail", fx, fy, 230, 260, 0, 50, 32)
    cyl("Lights", LC, fx, fy, 20, 30, 232, 32)
    cyl(p, "trim", fx, fy, 0, 150, 30, 16)
    cyl(p, "rail", fx, fy, 150, 160, 90, 16)
    K[P("Lights")].ico(LC, (fx, fy, 200), 30, 2)
    cyl("Collision", CL, fx, fy, 0, 60, 262, 16)
    # bar along the starboard end under the balcony
    bx = x1 - gw - 60
    box(p, "frame", bx - 40, bx + 40, y0 + 350, y0 + 900, 0, 105)
    box(p, "rail", bx - 46, bx + 46, y0 + 345, y0 + 905, 105, 110)
    box("Lights", LP, bx - 41, bx - 40, y0 + 360, y0 + 890, 20, 30)
    col(bx - 46, bx + 46, y0 + 345, y0 + 905, 0, 110)
    for y in range(int(y0 + 380), int(y0 + 880), 90):
        cyl(p, "trim", bx - 90, y, 0, 72, 4, 8)
        cyl(p, "soft", bx - 90, y, 72, 78, 18, 12)
    # seating clusters + planters
    for (sx, sy) in ((x0 + gw + 400, -b.W * 0.25), (x0 + gw + 400, b.W * 0.05), (b.cx + 700, -b.W * 0.3)):
        sofa(p, sx, sy + 90, 240, 180.0)
        sofa(p, sx, sy - 90, 240, 0.0)
        table(p, sx, sy, 120, 60, h=42)
    for x in range(int(x0 + 200), int(x1 - 100), 600):
        plant(p, x, y0 + 110, r=34, h=160)
    # chandelier ring
    cz = h - 120
    ring("Lights", LP, b.cx, 0, 380, 395, cz, cz + 6, 48)
    ring(p, "rail", b.cx, 0, 370, 405, cz + 6, cz + 14, 48)
    for a in range(0, 360, 60):
        t = math.radians(a)
        pipe(p, "trim", (b.cx + 388 * math.cos(t), 388 * math.sin(t), cz + 14), (b.cx, 0, h), 1.5, 6)
    light("P", 14, b.cx, 0, cz - 40, shadow=True)
    for x in (x0 + 400, x1 - 400):
        light("P", 10, x, -b.W * 0.1, gz - 60)
        light("P", 8, x, y1 - gw / 2, gz + 200)
    for x in range(int(x0 + 300), int(x1 - 200), 500):
        sconce(p, x, y1 - 4, gz + 200, 180.0 + 90.0)
    view("atrium", (x0 + gw + 150, y0 + 250, 175), (b.cx + 400, y1 - gw, gz), 14)
    view("atrium_balcony", (x1 - gw - 100, y1 - gw / 2, gz + 170), (x0 + 200, y0 + 200, 150), 14)


def pod_upright(part, x, y, yaw, h=230.0):
    """Upright cryo capsule facing local +X."""
    M = Mx(x, y, 0, yaw)
    cyl(part, "trim", 0, 0, 0, 20, 55, 16, M)
    cyl(part, "body", 0, 0, 20, h, 48, 16, M)
    box(part, VP, 44, 50, -24, 24, 60, h - 50, M)
    box("Lights", LB, 42, 44, -18, 18, 70, h - 60, M)
    box(part, "trim", 46, 52, -30, 30, h - 45, h - 30, M)
    box("Lights", LG, 52, 53, -10, 10, h - 42, h - 34, M)
    cyl(part, "trim", 0, 0, h, h + 20, 40, 16, M)


def bay_cryohall(b):
    """Colony cryo hall: upright pods in rows on the floor and on an upper
    gallery, central aisle, gantry walkway, cold blue light."""
    p = b.part
    x0, x1, y0, y1, h = b.x0 + 15, b.x1 - 15, b.y0 + 15, b.y1 - 15, b.h
    bay_ribs(b, step=400)
    gz = min(420.0, h * 0.47)
    aisle = 200.0
    rows_y = []
    y = aisle + 120
    while y + 60 < b.W / 2 - 345:
        rows_y.append(y)
        y += 260
    for side in (-1, 1):
        for i, ry in enumerate(rows_y):
            yy = side * ry
            for x in range(int(x0 + 250), int(x1 - 250), 120):
                for level, z in ((0, 0.0),):
                    yaw = 90.0 if side * (i % 2 * 2 - 1) > 0 else -90.0
                    pod_upright(p, x, yy, yaw)
            col(x0 + 200, x1 - 200, yy - 60, yy + 60, 0, 250)
    # upper gallery over the outer rows (walkable), stairs at the aft end
    for side in (-1, 1):
        ya, yb = side * (b.W / 2 - 15), side * (b.W / 2 - 330)
        gx1 = x1 - 20 - gz * 2.0 + 5
        platform(p, x0 + 200, gx1, *sorted((ya, yb)), gz, legs=True, rails=[((x0 + 200, yb), (gx1, yb))])
        for x in range(int(x0 + 300), int(gx1 - 60), 120):
            yy = side * (b.W / 2 - 110)
            M = Mx(x, yy, gz, -90.0 * side)
            cyl(p, "trim", 0, 0, 0, 20, 55, 16, M)
            cyl(p, "body", 0, 0, 20, 230, 48, 16, M)
            box(p, VP, 44, 50, -24, 24, 60, 180, M)
            box("Lights", LB, 42, 44, -18, 18, 70, 170, M)
        col(x0 + 250, gx1 - 60, *sorted((side * (b.W / 2 - 20), side * (b.W / 2 - 170))), gz, gz + 250)
    run = gz * 2.0
    # a stair at the forward end of each gallery, climbing aft
    for side in (-1, 1):
        stairs(p, x1 - 20, side * (b.W / 2 - 180), run, 150, 0, gz, 180.0, rails=(True, True))
    # aisle markings + blue floor strips
    for s in (-1, 1):
        box("Lights", LB, x0 + 200, x1 - 200, s * aisle - 3, s * aisle + 3, 0, 0.8)
    nf = max(2, int(b.L // 700))
    for k in range(nf):
        x = b.x0 + (k + 0.5) * b.L / nf
        ceiling_panel(p, x, 0, h, 0.0, 300, color=LB)
        light("B", 14, x, 0, h - 150, shadow=(k == 0))
    fill(12, b.cx, 0, 300)
    view("cryohall", (x1 - 150, 0, 165), (x0 + 300, 0, 200), 14)
    view("cryo_gallery", (x1 - 300, b.W / 2 - 250, gz + 165), (x0 + 400, 0, 120), 14)


def bay_farm(b):
    """Hydroponics hall: multi-level grow racks with lights, irrigation,
    water tanks, central walkway."""
    p = b.part
    x0, x1, y0, y1, h = b.x0 + 15, b.x1 - 15, b.y0 + 15, b.y1 - 15, b.h
    bay_ribs(b)
    aisle = 160.0
    ys = []
    y = aisle + 70
    while y + 70 < b.W / 2 - 80:
        ys.append(y)
        y += 250
    levels = max(2, min(4, int((h - 100) // 110)))
    for side in (-1, 1):
        for ry in ys:
            yy = side * ry
            xa, xb = x0 + 300, x1 - 400
            for (x, yq) in ((xa, yy - 60), (xb, yy - 60), (xa, yy + 60), (xb, yy + 60)):
                box(p, "trim", x - 5, x + 5, yq - 5, yq + 5, 0, levels * 110 + 20)
            for lv in range(levels):
                z = 40 + lv * 110
                box(p, "trim", xa, xb, yy - 62, yy + 62, z, z + 8)
                box(p, "body", xa + 5, xb - 5, yy - 55, yy + 55, z + 8, z + 18)
                n = int((xb - xa) // 60)
                for i in range(n):
                    K[P(p)].ico(R(PLT), (xa + 30 + i * 60, yy + ((i % 2) * 30 - 15), z + 36), 22 + (i * 7 + lv) % 9, 1)
                box("Lights", LP if lv % 2 else LG, xa + 10, xb - 10, yy - 30, yy + 30, z + 100, z + 102)
            pipe(p, PI, (xa, yy, levels * 110 + 30), (xb, yy, levels * 110 + 30), 5, 8)
            col(xa - 5, xb + 5, yy - 65, yy + 65, 0, levels * 110 + 30)
    for s_ in (-1, 1):
        for k in range(2):
            tank(p, x1 - 180 - k * 230, s_ * (b.W / 2 - 170), 90, min(h - 200, 400), bands=2)
    for s in (-1, 1):
        box(p, "accent", x0 + 250, x1 - 250, s * aisle - 4, s * aisle + 4, 0, 0.6)
    nf = max(2, int(b.L // 600))
    for k in range(nf):
        x = b.x0 + (k + 0.5) * b.L / nf
        light("G", 12, x, b.W * 0.2, h - 100)
        light("P", 12, x, -b.W * 0.2, h - 100, shadow=(k == 0))
    fill(10, b.cx, 0, h - 60)
    view("farm", (x1 - 250, 0, 165), (x0 + 300, 60, 150), 14)


def bay_fighter_aft(b):
    """Viper: avionics/engine-access crawl bay with the boarding ladder."""
    p = b.part
    x0, x1, y0, y1, h = b.x0 + 15, b.x1 - 15, b.y0 + 15, b.y1 - 15, b.h
    # engine access panel on the aft wall, glowing drive through a grille
    box(p, "trim", x0, x0 + 20, -110, 110, 30, 190)
    for k in range(7):
        box(p, "frame", x0 + 20, x0 + 26, -100, 100, 40 + k * 21, 50 + k * 21)
    box("Lights", LB, x0 + 2, x0 + 4, -96, 96, 40, 180)
    # avionics racks on both walls
    for s in (-1, 1):
        yw = s * (b.W / 2 - 15)
        box(p, "body", x0 + 40, x1 - 40, yw - s * 55, yw, 0, 200)
        for k in range(int((x1 - x0 - 80) // 40)):
            box("Lights", (LB, LG, LA)[k % 3], x0 + 50 + k * 40, x0 + 60 + k * 40,
                *sorted((yw - s * 56, yw - s * 55)), 120 + (k % 4) * 15, 126 + (k % 4) * 15)
        col(x0 + 40, x1 - 40, *sorted((yw - s * 55, yw)), 0, 200)
    # ladder well up to the dorsal hatch
    lx = b.cx
    for s in (-1, 1):
        pipe(p, "trim", (lx - 30, s * 25, 0), (lx - 30, s * 25, h), 2.5, 6)
    for z in range(30, int(h), 28):
        pipe(p, "trim", (lx - 30, -25, z), (lx - 30, 25, z), 1.8, 6)
    ring(p, "accent", lx, 0, 45, 58, h - 4, h, 20)
    box("Lights", LG, lx + 40, lx + 44, -10, 10, h - 20, h - 10)
    light("B", 4, b.cx, 0, h - 40)
    ceiling_panel(p, b.cx + 60, 0, h, 0.0, 90)


def bay_fighter_cabin(b):
    """Viper: crew nook - fold-down bunk, galley niche, locker, jump seat."""
    p = b.part
    x0, x1, y0, y1, h = b.x0 + 15, b.x1 - 15, b.y0 + 15, b.y1 - 15, b.h
    # bunk in a niche on the port wall
    bed(p, b.cx, y1 - 45, 0.0, w=80, L=195)
    box(p, "frame", b.cx - 100, b.cx + 100, y1 - 90, y1, 150, 158)
    box("Lights", LA, b.cx - 90, b.cx - 60, y1 - 2, y1 - 1, 110, 116)
    # galley niche + locker on the starboard wall
    box(p, GA, x0 + 20, x0 + 150, y0, y0 + 60, 0, 90)
    box(p, "body", x0 + 20, x0 + 150, y0, y0 + 64, 90, 94)
    box(p, GA, x0 + 20, x0 + 150, y0, y0 + 40, 150, 210)
    box(p, VP, x0 + 30, x0 + 140, y0 + 40, y0 + 41, 160, 200)
    col(x0 + 20, x0 + 150, y0, y0 + 64, 0, 100)
    box(p, "frame", x1 - 110, x1 - 20, y0, y0 + 60, 0, 205)
    box(p, "trim", x1 - 66, x1 - 64, y0 + 60, y0 + 62, 10, 200)
    col(x1 - 110, x1 - 20, y0, y0 + 60, 0, 205)
    # jump seat + flight suit
    chair(p, x0 + 230, y0 + 50, 0, -90.0)
    if b.W > 400:
        eva_suit(p, x1 - 50, y1 - 150, 180.0)
    flatscreen(p, x0 + 4, 0, 110, 120, 70, 0.0)
    ceiling_panel(p, b.cx, 0, h, 0.0, 140)
    light("W", 5, b.cx, 0, h - 40)
    view("cabin", (x1 - 20, -40, 160), (x0, 40, 110), 14)


BAYS = {
    "engine": bay_engine, "hold": bay_hold, "tankhold": bay_tankhold, "salvage": bay_salvage,
    "boarding": bay_boarding, "torpedo": bay_torpedo, "astro": bay_astro, "refinery": bay_refinery,
    "hangar": bay_hangar, "atrium": bay_atrium, "cryohall": bay_cryohall, "farm": bay_farm,
    "fighter_aft": bay_fighter_aft, "fighter_cabin": bay_fighter_cabin,
}


# ----------------------------------------------------------------------------
# Bridges (bow segment). The layout builds the walls/windows; this furnishes.
# ----------------------------------------------------------------------------
def bridge(b, kind, nose_x):
    """Furnish the bridge; returns (seat_xy, entry_xy or None, entry_yaw).
    nose_x = x of the bow window line. b.opts["halfw"](x) is the half-width of
    the chamfered plan at x, b.opts["xc"] where the chamfer starts.
    kinds: cockpit (1-2 pilot seats under the canopy), small, mid, cic, flag."""
    p = b.part
    x0, h, W = b.x0 + 15, b.h, b.W
    hw = b.opts["halfw"]
    xc = b.opts["xc"]
    fwd = nose_x - 15
    if kind == "cockpit":
        seats = b.opts.get("seats", 1)
        cx_ = fwd - 150
        cw = hw(fwd - 110) - 25
        extrude(p, "body", [(fwd - 110, 0), (fwd - 12, 0), (fwd - 12, 95), (fwd - 70, 105), (fwd - 110, 95)], 'y',
                -cw, cw)
        nsc = 3 if seats == 1 else 5
        for k in range(nsc):
            yy = -cw + 40 + k * (2 * cw - 80) / (nsc - 1)
            flatscreen(p, fwd - 60, yy, 90, 50, 32, 180.0, screen=(S["screen"], S["screen2"], CO)[k % 3])
        col(fwd - 110, fwd - 12, -cw, cw, 0, 105)
        ys = [0.0] if seats == 1 else [-W * 0.2, W * 0.2]
        for yy in ys:
            pilot_seat(p, cx_, yy, 0, 180.0)
            box("Collision", CL, -30, 46, -42, 42, 0, 150, Mx(cx_, yy, 0, 180.0))
            pipe(p, "trim", (cx_ + 45, yy, 0), (cx_ + 45, yy, 60), 3, 6)       # centre stick
            box(p, "accent", cx_ + 41, cx_ + 49, yy - 4, yy + 4, 60, 80)
        box(p, "body", cx_ - 50, cx_ + 60, -W / 2 + 60, W / 2 - 60, h - 18, h)
        for k in range(10):
            box("Lights", (LG, LA, LR)[k % 3], cx_ - 40 + k * 9, cx_ - 36 + k * 9, -20, 20, h - 19, h - 18)
        light("B" if S["fill"] == "B" else "W", 4, cx_ + 40, 0, h - 40)
        return (cx_, ys[0]), None, 0.0
    # helm row under the bow windows (desk faces aft, operator sits aft of it)
    helm_x = fwd - 100
    console(p, helm_x, 0, 180.0, n_crt=3)
    for k in range(3):
        y = 330 + k * 300
        if y + 85 + 40 > hw(helm_x + 72) - 20:
            break
        for s in (-1, 1):
            console(p, helm_x - 20, s * y, 180.0, n_crt=2, screen=S["screen2"] if s < 0 else None)
    # side-wall stations along the straight part of the hull
    nst = {"small": 1, "mid": 2, "cic": 3, "flag": 4}[kind]
    xa, xb = x0 + 250, min(helm_x - 260, xc - 150)
    if xb - xa > 200:
        nst = max(1, min(nst, int((xb - xa) // 260)))
        for k in range(nst):
            x = xa + (k + 0.5) * (xb - xa) / nst
            for s in (-1, 1):
                console(p, x, s * (W / 2 - 15 - 72), -90.0 * s, n_crt=3 if kind != "small" else 2)
    # captain: chair (small/mid) or a dais with the plot table (cic/flag)
    if kind in ("cic", "flag"):
        r = min(400.0 if kind == "cic" else 560.0, (helm_x - x0 - 580) / 2)
        cap_x = x0 + 60 + 160 + r
        for (z0, z1, rr, slot) in ((0, 20, r, "floor"), (20, 40, r - 60, "grate")):
            pts = circle(cap_x, 0, rr, 8, 22.5, 382.5)[:8]
            prism(p, slot, pts, z0, z1)
            K["Collision"].prism_xy(CL, pts, z0, z1)
        ring(p, "accent", cap_x, 0, r - 25, r, 20, 20.6, 8, 22.5, 382.5)
        holo_table(p, cap_x, 0, min(230.0, r * 0.42), z0=40, grid=S["screen"])
        chair(p, cap_x - (r - 120), 0, 40, 180.0, big=True)
        col(cap_x - r + 80, cap_x - r + 160, -40, 40, 40, 150)
        light("G" if S["screen"] == LG else "C", 6, cap_x, 0, 260)
        ring(p, "body", cap_x, 0, r - 70, r + 20, h - 90, h, 8, 22.5, 382.5)
        ring("Lights", S["lamp"], cap_x, 0, r - 60, r - 20, h - 92, h - 90, 8, 22.5, 382.5)
        for a in range(8):
            t = math.radians(a * 45 + 22.5)
            pipe(p, "trim", (cap_x + r * math.cos(t), r * math.sin(t), h - 90), (cap_x + r * math.cos(t), r * math.sin(t), h), 5, 8)
        if kind == "flag":
            for (px, py) in ((cap_x - r - 110, W * 0.3), (cap_x - r - 110, -W * 0.3),
                             (cap_x + r + 110, W * 0.3), (cap_x + r + 110, -W * 0.3)):
                if cap_x - r - 110 - 45 < x0 + 80:
                    continue
                solid(p, "frame", px - 45, px + 45, py - 45, py + 45, 0, h)
                box(p, "accent", px - 47, px + 47, py - 47, py + 47, 0, 60)
                for k in range(3):
                    pipe(p, PI, (px + 50, py - 20 + k * 20, 0), (px + 50, py - 20 + k * 20, h), 5, 8)
        # beside the dais, well aft of the helm: the seat trigger ignores the avatar
        # within MinDistanceFromEntryToExit (2 m) of its spawn
        entry = (cap_x, -min(r + 90, W / 2 - 250))
    else:
        cap_x = helm_x - 260
        chair(p, cap_x, 0, 0, 180.0, big=True)
        col(cap_x - 40, cap_x + 40, -40, 40, 0, 140)
        if kind == "mid" and cap_x - 450 > x0 + 150:
            holo_table(p, cap_x - 330, 0, 110, grid=S["screen"])
        entry = (cap_x - 120, 0.0)
    # aft wall screens either side of the door
    ncol = 2 if W < 1100 else 3
    for s in (-1, 1):
        for c in range(ncol):
            y = s * (260 + c * 150)
            if abs(y) + 60 > W / 2 - 30:
                continue
            for row in range(2):
                if 150 + row * 110 + 95 > h - 30:
                    continue
                crt(p, x0 + 60, y, 150 + row * 110, 120, 95, 0.0,
                    screen=S["screen"] if (c + row) % 3 else S["screen2"], d=60)
        ya, yb = s * 200, s * min(W / 2 - 60, 200 + ncol * 150)
        box(p, "body", x0, x0 + 60, *sorted((ya, yb)), 0, 150)
        col(x0, x0 + 60, *sorted((ya, yb)), 0, 150)
    # ceiling beams and fixtures
    x = x0 + 250
    while x < fwd - 150:
        wq = hw(x) - 15
        box(p, "frame", x - 18, x + 18, -wq, wq, h - 36, h)
        x += 450.0
    nl = max(1, int((fwd - x0) // 450))
    for k in range(nl):
        xx = x0 + (k + 0.5) * (fwd - x0) / nl
        for yy in ((-W * 0.25, W * 0.25) if W > 800 else (0.0,)):
            if abs(yy) < hw(xx) - 100:
                ceiling_panel(p, xx, yy, h - 36, 90.0, 150)
        fill(9 if W > 1000 else 7, xx, 0, h - 70)
    for s in (-1, 1):
        cage_lamp(p, x0 + 15, s * (W / 2 - 150), h - 70, 0.0)
    light("R" if S["warn"] == LR else "A", 5, x0 + 150, 0, h - 100)
    # CIC/flag spawns beside the dais facing the helm; small/mid face aft into the ship
    return (helm_x - 50, 0.0), entry, (0.0 if kind in ("cic", "flag") else 180.0)
