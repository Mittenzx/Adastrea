"""Dressing pass for the full-deck interiors: small props and live fixtures.

Tools/build_ship_decks.py calls dress_room(kind, rm) / dress_bay(kind, b) /
dress_bridge(b, kind) / dress_corridor(...) after each space is built. The
placements are written against each builder's layout in Tools/deck_rooms.py
(what's on which wall), so they sit in free spots rather than being scattered
at random. The build's walk check then proves no doorway or path got blocked.

  * prop(...)     static props baked into the room part (deck_props.CATALOG)
  * fixture(...)  runtime fixtures (X_ sockets -> AInteriorFixture): exterior
                  monitors, terminals, alert button, light panel, intercom,
                  coffee station, nav table

Rm frame (see deck_rooms.Rm): u along X, v from the spine wall (door side,
v=0) to the hull (v=D). yaw helpers: facing the room from the spine wall =
rm.yaw(90), from the hull = rm.yaw(-90), from the x0 end = 0, from x1 = 180.
"""
import math

from deck_kit import prop, fixture


def _near(rm, u, pad=90.0):
    return rm.door_u is not None and abs(u - rm.door_u) < rm.door_w / 2 + pad


# =============================================================================
# Rooms
# =============================================================================
def d_bunks(rm):
    p, D = rm.part, rm.D
    prop(p, "Pinboard", rm.x0, rm.y(D * 0.55), 120, 0.0)
    if D > 380:
        prop(p, "JacketHook", rm.x0, rm.y(D * 0.78), 175, 0.0, collide=False)
    prop(p, "DuffelBag", rm.x1 - 75, rm.y(115), 0, 0.0)
    prop(p, "Boots", rm.x1 - 70, rm.y(180), 0, 90.0)
    prop(p, "Guitar", rm.x1 - 12, rm.y(D * 0.52), 0, 180.0)
    if D > 520:
        prop(p, "LaundryBasket", rm.x0 + 60, rm.y(D * 0.3 + 95), 0, 0.0)
    u = (rm.door_u or rm.cx) - rm.door_w / 2 - 30
    fixture("LightSwitch", "-", u, rm.y(0), 125, rm.yaw(90), collide=False)
    prop(p, "PhotoFrame", rm.x1 - 150, rm.y(D - 60), 245 + 5, rm.yaw(-90), collide=False)


def d_galley(rm):
    p, vf = rm.part, rm.D
    prop(p, "CoffeeMachine", rm.x0 + 110, rm.y(vf - 32), 94, rm.yaw(-90))
    prop(p, "Microwave", rm.x0 + 225, rm.y(vf - 38), 94, rm.yaw(-90))
    for k in range(3):
        prop(p, "RationBox", rm.x0 + 420 + k * 26, rm.y(vf - 20), 94, rm.yaw(-90), collide=False)
    prop(p, "WaterDispenser", rm.x0 + 20, rm.y(max(200.0, vf - 150)), 0, 0.0)
    if not _near(rm, rm.x1 - 40, 60):
        prop(p, "TrashBin", rm.x1 - 150, rm.y(max(150.0, vf - 125)), 0, 0.0)
    # things on the table (same maths as rm_galley)
    tl = min(240, rm.L * 0.38)
    du = rm.door_u if rm.door_u is not None else rm.cx
    tu = du + rm.L * 0.27 if du - rm.x0 < rm.x1 - du else du - rm.L * 0.27
    tu = max(rm.x0 + tl / 2 + 60, min(rm.x1 - tl / 2 - 60, tu))
    tv = min(vf - 190, max(170, rm.D * 0.45))
    prop(p, "FoodTray", tu - tl * 0.25, rm.y(tv), 74, 0.0, collide=False)
    prop(p, "Mug", tu + tl * 0.2, rm.y(tv - 15), 74, 30.0, collide=False)
    prop(p, "Datapad", tu + tl * 0.32, rm.y(tv + 12), 74, 15.0, collide=False)


def d_mess(rm):
    p, vf = rm.part, rm.D
    if rm.opts.get("dining"):
        fixture("MonitorWall", "Port", rm.x0, rm.y(vf * 0.5), 170, 0.0, collide=False)
        return
    if vf < 700:
        vm = (vf - 80 + 170) / 2
        n = max(1, int((rm.L - 120) // 330))
        for i in range(n):
            u = rm.x0 + 60 + (i + 0.5) * (rm.L - 120) / n
            prop(p, "Mug", u + 60, rm.y(vm + 18), 74, 20.0 * i, collide=False)
            prop(p, "Bottle", u - 80, rm.y(vm - 20), 74, 0.0, collide=False)
            if i % 2 == 0:
                prop(p, "PlayingCards", u + 20, rm.y(vm - 5), 74, 10.0, collide=False)
        fixture("Coffee", "-", rm.x1 - 40, rm.y(vf - 175), 0, 180.0)
    else:
        prop(p, "VendingMachine", rm.x0 + 72, rm.y(vf - 165), 0, 0.0)
        fixture("Coffee", "-", rm.x1 - 40, rm.y(vf - 180), 0, 180.0)
        sl0 = rm.x0 + rm.L * 0.25
        for k in range(4):
            prop(p, "FoodTray", sl0 + 60 + k * 60, rm.y(vf - 260), 96, 0.0, collide=False)
    fixture("MonitorWall", "Chase", rm.x1, rm.y(max(120.0, vf * 0.35)), 175, 180.0, collide=False)
    prop(p, "Poster", rm.x0, rm.y(max(130.0, vf * 0.35)), 110, 0.0, collide=False)
    if not _near(rm, rm.x0 + 60):
        prop(p, "TrashBin", rm.x0 + 35, rm.y(60), 0, 0.0)


def d_brig(rm):
    p = rm.part
    cd = min(240.0, rm.D - 200)
    vb = rm.D - cd
    du = rm.x0 + 120 if not rm.near_door(rm.x0 + 60, rm.x0 + 260) else rm.x1 - 200
    prop(p, "Clipboard", du - 30, rm.y(vb - 150 - 45), 100, 0.0, collide=False)
    prop(p, "Mug", du + 40, rm.y(vb - 150 - 50), 100, 0.0, collide=False)
    fixture("Intercom", "-", rm.x0, rm.y(vb - 60), 125, 0.0, collide=False)
    prop(p, "SafetyPlacard", rm.x1, rm.y(vb - 70), 150, 180.0, collide=False)


def d_armory(rm):
    p, vf = rm.part, rm.D
    prop(p, "FireExtinguisher", rm.x0, rm.y(150), 0, 0.0, collide=False)
    prop(p, "SafetyPlacard", rm.x0, rm.y(vf * 0.6), 150, 0.0, collide=False)
    prop(p, "WallTools", rm.x0, rm.y(vf * 0.6 + 70), 100, 0.0, collide=False)
    fixture("WallPanel", "Status", rm.x1 - 18, rm.y(150), 0, 180.0, collide=False)


def d_ready(rm):
    p, vf = rm.part, rm.D
    vb = max(200.0, vf * 0.45)
    for k, u in enumerate(rm.spans(rm.x0 + 150, rm.x1 - 150, 120, gap=260, pad=40)):
        prop(p, "Helmet", u + 30, rm.y(vb), 50, 20.0 * k, collide=False)
    prop(p, "OxygenMasks", rm.x0, rm.y(vf * 0.3), 150, 0.0, collide=False)
    prop(p, "SafetyPlacard", rm.x0, rm.y(vf * 0.3 + 70), 150, 0.0, collide=False)


def d_airlock(rm):
    p, vf = rm.part, rm.D
    prop(p, "AlarmBeacon", rm.cx - 120, rm.y(vf - 30), rm.h - 20, 0.0, collide=False)
    prop(p, "OxygenMasks", rm.x0, rm.y(120), 150, 0.0, collide=False)
    fixture("WallPanel", "Status", rm.x1 - 10, rm.y(120), 0, 180.0, collide=False)


def d_workshop(rm):
    p, vf = rm.part, rm.D
    a, b = rm.x0 + 40, rm.x1 - 40
    prop(p, "Toolbox", a + 60, rm.y(vf - 45), 94, rm.yaw(-90), collide=False)
    prop(p, "BatteryPack", b - 80, rm.y(vf - 45), 94, rm.yaw(-90), collide=False)
    prop(p, "PartsBins", rm.x1, rm.y(vf * 0.3), 100, 180.0, collide=False)
    prop(p, "CableSpool", rm.cx + 120, rm.y(vf * 0.3), 0, 0.0)
    if vf > 420:
        prop(p, "ToolChest", rm.x1 - 32, rm.y(vf - 190), 0, 180.0)
    prop(p, "MaintDrone", rm.x0 + 90, rm.y(vf * 0.45 + 120), 0, 30.0)
    prop(p, "HazardCone", rm.cx - 90, rm.y(vf * 0.4 + 60), 0, 0.0)


def d_ecm(rm):
    p = rm.part
    fixture("Terminal", "Comms", rm.x1 - 72 - 20 - 180, rm.y(120), 0, 180.0)
    prop(p, "Headset", rm.x1 - 72 - 20 - 40, rm.y(90), 106, 0.0, collide=False)


def d_ops(rm):
    p, vf = rm.part, rm.D
    for u in rm.spans(rm.x0 + 60, rm.x1 - 60, 240, gap=60, pad=0):
        prop(p, "Datapad", u + 60, rm.y(vf - 72 - 15 - 40), 72, rm.yaw(-90) + 15, collide=False)
        prop(p, "Mug", u + 170, rm.y(vf - 72 - 15 - 35), 72, 0.0, collide=False)
    fixture("MonitorWall", "Bow", rm.x0, rm.y(vf * 0.5), 205, 0.0, collide=False)
    if rm.opts.get("big"):
        fixture("AlertButton", "-", rm.x1 - 60, rm.y(vf * 0.15 + 60), 0, 180.0)


def d_magazine(rm):
    p, vf = rm.part, rm.D
    prop(p, "Canisters", rm.x0 + 40, rm.y(vf * 0.5), 0, 0.0)
    prop(p, "HazardCone", rm.x1 - 60, rm.y(vf * 0.5 - 60), 0, 0.0)
    prop(p, "FireExtinguisher", rm.x0, rm.y(160), 0, 0.0, collide=False)
    prop(p, "AlarmBeacon", rm.x1 - 30, rm.y(vf - 30), rm.h - 20, 0.0, collide=False)


def d_gunnery(rm):
    p, vf = rm.part, rm.D
    n = max(2, int((rm.L - 200) // 280))
    for i in range(n):
        u = rm.x0 + 100 + (i + 0.5) * (rm.L - 200) / n
        prop(p, "Headset", u + 30, rm.y(vf - 87 - 40), 106, 0.0, collide=False)
    fixture("MonitorWall", "Dorsal", rm.x1, rm.y(vf * 0.45), 190, 180.0, collide=False)


def d_medbay(rm):
    p, vf = rm.part, rm.D
    prop(p, "FirstAidKit", rm.x0, rm.y(140), 140, 0.0, collide=False)
    prop(p, "Defibrillator", rm.x0, rm.y(210), 110, 0.0, collide=False)
    beds = rm.spans(rm.x0 + 60, rm.x1 - 180, 100, gap=120, pad=0)[:3]
    for u in beds:
        prop(p, "IVStand", u + 50 - 75, rm.y(vf - 200), 0, 0.0)
    if beds:
        prop(p, "SpecimenJar", beds[0] + 50 + 75, rm.y(vf - 25), 120, 0.0, collide=False)


def d_cryo(rm):
    p = rm.part
    prop(p, "MedCabinet", rm.x0 if rm.door_u and rm.door_u - rm.x0 > rm.x1 - rm.door_u else rm.x1,
         rm.y(120), 120, 0.0 if rm.door_u and rm.door_u - rm.x0 > rm.x1 - rm.door_u else 180.0, collide=False)
    fixture("WallPanel", "Status", rm.x1 - 10 if rm.door_u and rm.door_u - rm.x0 <= rm.x1 - rm.door_u else rm.x0 + 10,
            rm.y(rm.D - 60), 0, 180.0 if rm.door_u and rm.door_u - rm.x0 <= rm.x1 - rm.door_u else 0.0, collide=False)


def d_briefing(rm):
    p, vf = rm.part, rm.D
    ue = rm.x1 - 20
    fixture("MonitorHang", "Chase", ue - 260, rm.y(vf * 0.5), rm.h, 180.0, collide=False)
    prop(p, "Clipboard", ue - 180, rm.y(vf - 120), 116, 180.0, collide=False)
    prop(p, "WallClock", rm.x0, rm.y(vf * 0.5), 200, 0.0, collide=False)


def d_cabins(rm):
    p, vf = rm.part, rm.D
    if rm.opts.get("lux"):
        prop(p, "ChessBoard", rm.x0 + 220, rm.y(vf - 230), 42, 0.0, collide=False)
        prop(p, "DeskLamp", rm.x0 + 45, rm.y(185), 76, 0.0, collide=False)
        prop(p, "BookStack", rm.x0 + 45, rm.y(300), 76, 0.0, collide=False)
        fixture("MonitorWall", "Starboard" if rm.sg < 0 else "Port", rm.x1, rm.y(vf * 0.25), 160, 180.0, collide=False)
        return
    cd = min(300.0, vf - 170)
    vb = vf - cd
    cw = 300.0
    n = max(1, int((rm.L - 20) // cw))
    u0 = rm.x0 + (rm.L - n * cw) / 2
    for i in range(n):
        ua = u0 + i * cw
        prop(p, "DeskLamp", ua + 30, rm.y(vb + 35), 76, 0.0, collide=False)
        prop(p, ("Datapad", "BookStack", "PhotoFrame")[i % 3], ua + 35, rm.y(vb + 85), 76, 0.0, collide=False)
        prop(p, "Boots", ua + 120, rm.y(vb + 45), 0, 0.0, collide=False)
        if i % 2:
            prop(p, "PlantPot", ua + cw - 35, rm.y(vf - 125), 205, 0.0, collide=False)
    prop(p, "FireExtinguisher", rm.x0, rm.y(vb * 0.5), 0, 0.0, collide=False)


def d_lab(rm):
    p, vf = rm.part, rm.D
    a, b = rm.x0 + 40, rm.x1 - 40
    for k in range(int((b - a) // 160)):
        u = a + 60 + k * 160
        prop(p, ("SpecimenJar", "Datapad", "Binders")[k % 3], u + 45, rm.y(vf - 40), 94, rm.yaw(-90), collide=False)
    if vf > 450:
        fixture("WallPanel", "Status", rm.x0 + 10, rm.y(vf * 0.45 + 110), 0, 0.0, collide=False)
    prop(p, "FirstAidKit", rm.x1, rm.y(140), 140, 180.0, collide=False)


def d_storage(rm):
    p, vf = rm.part, rm.D
    if not _near(rm, rm.x0 + 70):
        prop(p, "Barrel", rm.x0 + 70, rm.y(120), 0, 0.0)
    prop(p, "Canisters", rm.cx, rm.y(vf - 30), 0, 90.0 * rm.sg)


def d_ward(rm):
    p, vf = rm.part, rm.D
    prop(p, "MedCabinet", rm.x0, rm.y(max(160.0, vf * 0.45)), 110, 0.0, collide=False)
    prop(p, "FirstAidKit", rm.x1, rm.y(max(160.0, vf * 0.45)), 140, 180.0, collide=False)
    fixture("WallPanel", "Status", rm.x1 - 10, rm.y(max(160.0, vf * 0.45) + 110), 0, 180.0, collide=False)


def d_surgery(rm):
    p, vf = rm.part, rm.D
    su, sv = rm.cx, vf * 0.55
    prop(p, "IVStand", su + 130, rm.y(sv + 30), 0, 0.0)
    prop(p, "Defibrillator", rm.x0, rm.y(sv), 110, 0.0, collide=False)
    prop(p, "MedCabinet", rm.x1, rm.y(sv), 110, 180.0, collide=False)


def d_lounge(rm):
    p, vf = rm.part, rm.D
    n = max(1, int((rm.L - 200) // 420))
    for i in range(n):
        u = rm.x0 + 100 + (i + 0.5) * (rm.L - 200) / n
        prop(p, ("ChessBoard", "PlayingCards")[i % 2], u, rm.y(vf - 190), 42, 0.0, collide=False)
    prop(p, "Radio", rm.x0 + 90, rm.y(260), 110, 0.0, collide=False)
    fixture("MonitorWall", "Starboard" if rm.sg < 0 else "Port", rm.x1, rm.y(vf * 0.4), 170, 180.0, collide=False)


ROOM_DRESS = {
    "bunks": d_bunks, "galley": d_galley, "mess": d_mess, "brig": d_brig, "armory": d_armory,
    "ready": d_ready, "airlock": d_airlock, "workshop": d_workshop, "ecm": d_ecm, "ops": d_ops,
    "magazine": d_magazine, "gunnery": d_gunnery, "medbay": d_medbay, "cryo": d_cryo,
    "briefing": d_briefing, "cabins": d_cabins, "lab": d_lab, "storage": d_storage, "ward": d_ward,
    "surgery": d_surgery, "lounge": d_lounge,
}


def dress_room(kind, rm):
    fn = ROOM_DRESS.get(kind)
    if fn:
        fn(rm)


# =============================================================================
# Bays
# =============================================================================
def d_engine(b):
    p, x0, x1, y0, y1, h = b.part, b.x0 + 15, b.x1 - 15, b.y0 + 15, b.y1 - 15, b.h
    dw = b.door_w / 2
    # forward bulkhead either side of the door: fuse box + junction boxes + terminal
    prop(p, "FuseBox", x1, dw + 330, 110, 180.0, collide=False)
    prop(p, "JunctionBox", x1, dw + 120, 150, 180.0, collide=False)
    prop(p, "JunctionBox", x1, -dw - 120, 150, 180.0, collide=False)
    fixture("Terminal", "Engineering", x1 - 30, -dw - 330 - 150 if b.W > 1100 else -dw - 110, 0, 180.0)
    prop(p, "FireExtinguisher", x1, dw + 60, 0, 180.0, collide=False)
    prop(p, "AlarmBeacon", x1 - 20, 0, h - 30, 0.0, collide=False)
    prop(p, "HazardCone", x1 - 250, dw + 120, 0, 0.0)
    prop(p, "Toolbox", x1 - 300, -dw - 60, 0, 20.0)
    if h >= 560:
        if h < 900:
            prop(p, "GasRack", x0, y0 + 500 if b.W > 1300 else 0.0, 0, 0.0)
        prop(p, "WallFan", x0, y1 - 300, 300, 0.0, collide=False)
        prop(p, "CableSpool", x1 - 200, y1 - 400, 0, 0.0)
    fixture("MonitorWall", "Stern", x1, -dw - 150, 190, 180.0, collide=False)


def d_hold(b):
    p, x0, x1, y0, y1, h = b.part, b.x0 + 15, b.x1 - 15, b.y0 + 15, b.y1 - 15, b.h
    lane = 220.0
    fixture("Terminal", "Cargo", x1 - 30, -lane - 110, 0, 180.0)
    prop(p, "PalletBoxes", x1 - 180, y0 + 90, 0, 0.0)
    prop(p, "Barrel", x0 + 330, -lane - 40, 0, 0.0)
    prop(p, "Barrel", x0 + 395, -lane - 45, 0, 0.0)
    prop(p, "HazardCone", x0 + 520, -lane - 30, 0, 0.0)
    prop(p, "HazardCone", x1 - 420, lane + 40, 0, 0.0)
    prop(p, "FireExtinguisher", x1, lane + 60, 0, 180.0, collide=False)
    fixture("MonitorWall", "Ventral", x1, -lane - 280, 200, 180.0, collide=False)


def d_tankhold(b):
    p, x0, x1 = b.part, b.x0 + 15, b.x1 - 15
    fixture("Terminal", "Engineering", x1 - 30, -150, 0, 180.0)
    prop(p, "ValveWheel", x1, 120, 140, 180.0, collide=False)
    prop(p, "PressureGauge", x1, 60, 170, 180.0, collide=False)
    prop(p, "HazardCone", x0 + 150, 150, 0, 0.0)
    prop(p, "Canisters", x0 + 40, -170, 0, 0.0)


def d_salvage(b):
    p, x0, x1, y0, y1 = b.part, b.x0 + 15, b.x1 - 15, b.y0 + 15, b.y1 - 15
    lane = b.door_w / 2 + 80
    fixture("Terminal", "Cargo", x1 - 30, -lane - 110, 0, 180.0)
    prop(p, "Toolbox", x1 - 330, -lane - 60, 0, 30.0)
    prop(p, "CableSpool", x0 + 120, lane + 70, 0, 0.0)
    prop(p, "HazardCone", x0 + 220, -lane - 20, 0, 0.0)
    fixture("MonitorWall", "Ventral", x0, lane + 130, 200, 0.0, collide=False)


def d_boarding(b):
    p, x0, x1 = b.part, b.x0 + 15, b.x1 - 15
    for x in range(int(x0 + 500), int(x1 - 400), 500):
        prop(p, "Helmet", x - 60, 120, 50, 30.0, collide=False)
        prop(p, "Helmet", x + 70, -120, 50, -40.0, collide=False)
    fixture("AlertButton", "-", x1 - 60, 250, 0, 180.0)
    fixture("MonitorWall", "Bow", x1, -250, 200, 180.0, collide=False)
    prop(p, "OxygenMasks", x0, 200, 150, 0.0, collide=False)


def d_torpedo(b):
    p, x0 = b.part, b.x0 + 15
    prop(p, "Canisters", x0 + 60, b.door_w / 2 + 250, 0, 0.0)
    prop(p, "SafetyPlacard", x0, -b.door_w / 2 - 120, 150, 0.0, collide=False)
    fixture("MonitorWall", "Bow", x0, -b.door_w / 2 - 260, 190, 0.0, collide=False)


def d_astro(b):
    p = b.part
    R_ = min(b.L, b.W) * 0.28
    for k in range(6):
        a = math.radians(360.0 * k / 6 + 30)
        if k % 2 == 0:
            prop(p, "Datapad", b.cx + (R_ - 40) * math.cos(a), (R_ - 40) * math.sin(a), 72, math.degrees(a) + 90, collide=False)
    fixture("Terminal", "Nav", b.x1 - 45, b.W * 0.3, 0, 180.0)
    fixture("MonitorHang", "Dorsal", b.x0 + 200, 0.0, b.h, 0.0, collide=False)


def d_refinery(b):
    p, x0, x1, y0, y1 = b.part, b.x0 + 15, b.x1 - 15, b.y0 + 15, b.y1 - 15
    prop(p, "HazardCone", x0 + 600, -b.W * 0.22 + 120, 0, 0.0)
    prop(p, "Barrel", x1 - 150, -b.W * 0.22 + 360, 0, 0.0)
    prop(p, "GasRack", x0, b.W * 0.25 + 250, 0, 0.0)
    fixture("Terminal", "Cargo", x1 - 30, -b.door_w / 2 - 120, 0, 180.0)


def d_hangar(b):
    p, x0, x1, y0, y1 = b.part, b.x0 + 15, b.x1 - 15, b.y0 + 15, b.y1 - 15
    prop(p, "Toolbox", x0 + 250, -200 - 110, 0, 10.0)
    prop(p, "Barrel", x0 + 120, y1 - 700, 0, 0.0)
    prop(p, "Barrel", x0 + 180, y1 - 760, 0, 0.0)
    prop(p, "HazardCone", x0 + 650, y0 + 250, 0, 0.0)
    prop(p, "HazardCone", x0 + 900, y0 + 250, 0, 0.0)
    fixture("Terminal", "Status", x1 - 260 - 60, -b.door_w / 2 - 130, 0, 180.0)
    fixture("MonitorWall", "Stern", x1, b.door_w / 2 + 220, 200, 180.0, collide=False)


def d_atrium(b):
    p = b.part
    prop(p, "VendingMachine", b.x0 + 15 + 72, b.y0 + 15 + 200 + 400, 0, 0.0)
    fixture("MonitorWall", "Chase", b.x1 - 15, b.y1 - 15 - 280 - 250, 190, 180.0, collide=False)


def d_cryohall(b):
    fixture("WallPanel", "Status", b.x1 - 25, b.door_w / 2 + 150, 0, 180.0, collide=False)
    prop(b.part, "OxygenMasks", b.x0 + 15, -b.door_w / 2 - 150, 150, 0.0, collide=False)


def d_farm(b):
    p = b.part
    fixture("Terminal", "Engineering", b.x0 + 45, b.door_w / 2 + 120, 0, 0.0)
    prop(p, "Toolbox", b.x0 + 150, -b.door_w / 2 - 80, 0, 0.0)


def d_fighter_aft(b):
    pass


def d_fighter_cabin(b):
    p, x0, x1, y0, y1 = b.part, b.x0 + 15, b.x1 - 15, b.y0 + 15, b.y1 - 15
    prop(p, "CoffeeMachine", x0 + 60, y0 + 32, 94, 90.0, collide=False)
    prop(p, "Mug", x0 + 120, y0 + 30, 94, 0.0, collide=False)
    fixture("MonitorWall", "Chase", x0, 60, 165, 0.0, collide=False)
    prop(p, "Helmet", b.cx + 60, y1 - 60, 158, 0.0, collide=False)


BAY_DRESS = {
    "engine": d_engine, "hold": d_hold, "tankhold": d_tankhold, "salvage": d_salvage, "boarding": d_boarding,
    "torpedo": d_torpedo, "astro": d_astro, "refinery": d_refinery, "hangar": d_hangar, "atrium": d_atrium,
    "cryohall": d_cryohall, "farm": d_farm, "fighter_aft": d_fighter_aft, "fighter_cabin": d_fighter_cabin,
}


def dress_bay(kind, b):
    fn = BAY_DRESS.get(kind)
    if fn:
        fn(b)


# =============================================================================
# Bridge + corridors
# =============================================================================
def dress_bridge(b, kind, seat, entry):
    """seat: helm operator position; consoles face aft of the bow windows."""
    p, x0, h, W = b.part, b.x0 + 15, b.h, b.W
    if kind == "cockpit":
        fixture("MonitorWall", "Stern", x0 + 5, -W * 0.25, 150, 0.0, collide=False)
        prop(p, "Mug", seat[0] + 60, seat[1] - W * 0.3, 105, 0.0, collide=False)
        return
    helm_x = seat[0] + 50
    # helm desk top: datapad + mug; headsets on the side stations
    prop(p, "Mug", helm_x + 10, 90, 106, 0.0, collide=False)
    prop(p, "Datapad", helm_x + 15, -95, 106, 10.0, collide=False)
    # monitors hanging aft of the helm row: chase view and stern camera
    if h >= 320:
        fixture("MonitorHang", "Chase", helm_x - 150, -170, h, 180.0, collide=False)
        fixture("MonitorHang", "Stern", helm_x - 150, 170, h, 180.0, collide=False)
    # captain's terminals + alert button
    if kind in ("small", "mid"):
        cap_x = helm_x - 260
        fixture("AlertButton", "-", cap_x + 10, 110, 0, 180.0)
        # nav terminal aft of the captain, clear of the aft-wall screens and door
        fixture("Terminal", "Nav", x0 + 120, -min(300.0, W / 2 - 110), 0, 0.0)
    else:
        r = min(400.0 if kind == "cic" else 560.0, (helm_x - x0 - 580) / 2)
        cap_x = x0 + 60 + 160 + r
        fixture("AlertButton", "-", cap_x - r + 120, 90, 40, 180.0)
        if W * 0.28 + 56 < W / 2 - 180:
            fixture("MapTable", "-", cap_x + r + 20, W * 0.28, 0, 0.0)
    # aft wall: comms panel on the port side, exterior monitor starboard
    if W / 2 - 110 > 250:
        fixture("WallPanel", "Comms", x0 + 60, W / 2 - 110, 0, 0.0, collide=False)
        if kind not in ("small", "mid"):          # the nav terminal has that corner there
            fixture("MonitorWall", "Bow", x0 + 60, -(W / 2 - 110), 200, 0.0, collide=False)
    prop(p, "WallClock", x0, 0, h - 45 if h > 300 else 260, 0.0, collide=False)


def dress_corridor(part, x0, x1, sw, hs, doors, feeds=("Port", "Starboard")):
    """Spine corridor walls: safety kit, speakers, an intercom and an exterior
    monitor per wall, in the gaps between doors. doors: {+1: [x...], -1: [x...]}."""
    hw = sw / 2 - WT_HALF
    kinds = ["FireExtinguisher", "Speaker", "FirstAidKit", "Monitor", "OxygenMasks", "Intercom"]
    for side in (1, -1):
        yaw = -90.0 if side > 0 else 90.0          # back on the wall, facing the corridor
        y = side * hw
        spots = []
        x = x0 + 120
        while x < x1 - 120:
            if not any(abs(x - d) < 150 for d in doors[side]):
                spots.append(x)
            x += 330
        k = 0 if side > 0 else 3
        for x in spots[::2]:
            kind = kinds[k % len(kinds)]
            k += 1
            if kind == "FireExtinguisher":
                prop(part, "FireExtinguisher", x, y, 0, yaw, collide=False)
            elif kind == "Speaker":
                prop(part, "WallSpeaker", x, y, min(225.0, hs - 40), yaw, collide=False)
            elif kind == "FirstAidKit":
                prop(part, "FirstAidKit", x, y, 135, yaw, collide=False)
            elif kind == "OxygenMasks":
                prop(part, "OxygenMasks", x, y, 150, yaw, collide=False)
            elif kind == "Intercom":
                fixture("Intercom", "-", x, y, 120, yaw, collide=False)
            else:
                fixture("MonitorWall", feeds[0] if side > 0 else feeds[1], x, y, 165, yaw, collide=False)
    # lit exit signs over both ends of the spine
    for (xe, yaw) in ((x0 + 16, 0.0), (x1 - 16, 180.0)):
        prop(part, "ExitSign", xe, 0.0, min(hs - 20, 255), yaw, collide=False)


WT_HALF = 15.0
