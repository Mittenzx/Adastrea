"""Adastrea interior texture library -- human-scale, tileable PBR sets.

Why this exists
---------------
The first-pass interior sets (T_Int_Wall/Deck/Cockpit/Glow) were produced by the
ship-HULL recipe in generate_adastrea_assets.gen_texture_set(): a dense greeble
grid designed to be seen from 50 m away on a hull. Inside a room, at furniture
scale, that reads as wallpaper. This generator authors each set in real
millimetres instead, so a seam is a seam, a rivet is ~10 mm, a pipe is 80-180 mm,
a button is 40 mm.

Scale contract
--------------
Every set declares its physical tile size (TILE_MM). The interior meshes from
generate_adastrea_assets.smart_uv() are UV'd world-aligned at 200 cm per UV unit,
so a 2000 mm set maps at tiling 1.0 on them; 1000 mm sets (console, hazard trim)
map at tiling 2.0. For UE world-aligned materials use TILE_MM/10 as the world
size in cm. See Assets/Textures/generated/interiors/MATERIAL_MAPPING.md.

smart_uv() mirrors U on -X faces, so nothing here uses readable text or
handed arrows that would look wrong mirrored.

Output (matches Tools/gen_basic_textures.py + generate_adastrea_assets.py):
  Assets/Textures/generated/interiors/T_Int_<Set>_{D,N,R,M,AO,E}.png
      D, E    : sRGB RGB
      N       : DirectX (green-down) tangent normal, flat = (128,128,255)
      R, M, AO: linear greyscale (sRGB OFF in UE)
  plus _MASK on the sets that need one (grating opacity, glass dirt).
  Assets/Textures/generated/interiors/previews/contact_<Set>.png, contact_all.png

Usage:
  python Tools/generate_interior_texture_library.py            # all sets + previews
  python Tools/generate_interior_texture_library.py ShipWall   # just some sets
  python Tools/generate_interior_texture_library.py --half     # half-res quick pass

Deterministic: every set has a fixed seed. Pure numpy + PIL, no Blender needed
(test renders live in Tools/render_interior_texture_tests.py, Blender headless).
"""
import os
import sys
import numpy as np
from PIL import Image, ImageDraw

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, "Assets", "Textures", "generated", "interiors")
PREV = os.path.join(OUT, "previews")

# --------------------------------------------------------------------------
# Shared palette (ship = cool gunmetal + teal ID, station = warm + amber)
# Colours are sRGB display values 0..1.
# --------------------------------------------------------------------------
PAL = dict(
    gunmetal=(0.30, 0.315, 0.335),
    steel=(0.52, 0.53, 0.545),
    bare=(0.66, 0.67, 0.68),
    paint_ship=(0.41, 0.44, 0.48),      # blue-grey bulkhead paint
    paint_dark=(0.19, 0.20, 0.215),
    teal=(0.10, 0.52, 0.56),             # ship ID accent (matches T_Basic_AccentTeal)
    amber=(0.93, 0.58, 0.17),            # station / civilian accent
    hazard=(0.88, 0.66, 0.08),
    black=(0.05, 0.05, 0.055),
    grit=(0.11, 0.115, 0.12),
    cream=(0.70, 0.64, 0.55),
    warm_panel=(0.74, 0.71, 0.66),
    wood=(0.40, 0.25, 0.14),
    brass=(0.72, 0.56, 0.32),
    carpet=(0.33, 0.26, 0.22),
)


# ==========================================================================
# Core helpers (all periodic -> every map tiles seamlessly)
# ==========================================================================
class Canvas:
    """Pixel grid with millimetre coordinates. U right, Z up (row 0 = top)."""

    def __init__(self, size, tile_mm):
        self.S = size
        self.T = float(tile_mm)
        self.mmpp = self.T / size
        y, x = np.mgrid[0:size, 0:size].astype(np.float32)
        self.U = (x + 0.5) * self.mmpp          # 0..T left->right
        self.Z = self.T - (y + 0.5) * self.mmpp  # 0..T bottom->top

    def wrap(self, a, c):
        """Signed periodic offset a-c in (-T/2, T/2]."""
        T = self.T
        return ((a - c + T / 2) % T) - T / 2

    def rep(self, a, period, phase=0.0):
        """Local coordinate in a repeating cell, centred: (-p/2, p/2]."""
        return ((a - phase + period / 2) % period) - period / 2

    def zeros(self):
        return np.zeros((self.S, self.S), np.float32)

    def full(self, v):
        return np.full((self.S, self.S), v, np.float32)

    def rgb(self, c):
        return np.broadcast_to(np.array(c, np.float32), (self.S, self.S, 3)).copy()


def sd_box(px, py, hx, hy, r=0.0):
    qx = np.abs(px) - hx + r
    qy = np.abs(py) - hy + r
    out = np.sqrt(np.maximum(qx, 0) ** 2 + np.maximum(qy, 0) ** 2)
    return out + np.minimum(np.maximum(qx, qy), 0) - r


def sd_circle(px, py, r):
    return np.sqrt(px * px + py * py) - r


def sd_capsule(px, py, half_len, r):
    return np.sqrt(np.maximum(np.abs(px) - half_len, 0) ** 2 + py * py) - r


def fill(sd, aa):
    """1 inside, 0 outside, antialiased over aa mm."""
    return np.clip(0.5 - sd / aa, 0, 1).astype(np.float32)


def bevel(sd, width):
    """0 at the edge rising smoothly to 1 at `width` mm inside."""
    t = np.clip(-sd / width, 0, 1)
    return (t * t * (3 - 2 * t)).astype(np.float32)


def mix(a, b, m):
    if a.ndim == 3 and m.ndim == 2:
        m = m[..., None]
    return a * (1 - m) + b * m


_KCACHE = {}


def blur(img, sigma_px):
    """Periodic gaussian blur via FFT (wraps -> tileable by construction)."""
    if sigma_px <= 0:
        return img
    S = img.shape[0]
    key = (S, round(sigma_px, 3))
    if key not in _KCACHE:
        f = np.fft.fftfreq(S).astype(np.float32)
        g = np.exp(-2 * (np.pi * sigma_px) ** 2 * (f[:, None] ** 2 + f[None, :] ** 2))
        _KCACHE[key] = g.astype(np.float32)
    return np.real(np.fft.ifft2(np.fft.fft2(img) * _KCACHE[key])).astype(np.float32)


def noise(cv, rng, scale_mm, aniso=(1.0, 1.0)):
    """Periodic band-limited noise, ~N(0,1). aniso = stretch factor (u, z): >1 elongates features along that axis."""
    S = cv.S
    w = rng.standard_normal((S, S)).astype(np.float32)
    sig = scale_mm / cv.mmpp
    f = np.fft.fftfreq(S).astype(np.float32)
    fy = f[:, None] * aniso[1] * sig
    fx = f[None, :] * aniso[0] * sig
    g = np.exp(-2 * np.pi ** 2 * (fx ** 2 + fy ** 2))
    n = np.real(np.fft.ifft2(np.fft.fft2(w) * g)).astype(np.float32)
    return (n - n.mean()) / (n.std() + 1e-8)


def fbm(cv, rng, base_mm, octaves=4, aniso=(1.0, 1.0)):
    out = cv.zeros()
    amp, tot, sc = 1.0, 0.0, base_mm
    for _ in range(octaves):
        out += noise(cv, rng, sc, aniso) * amp
        tot += amp
        amp *= 0.5
        sc *= 0.5
    return out / tot


def n01(n, lo=-2.0, hi=2.0):
    return np.clip((n - lo) / (hi - lo), 0, 1).astype(np.float32)


def scratches(cv, rng, count, len_mm, width_mm, dir_bias=None):
    """Periodic fine line segments -> 0..1 mask. dir_bias: angle radians or None."""
    S = cv.S
    m = np.zeros((S, S), np.float32)
    for _ in range(count):
        cx, cz = rng.uniform(0, cv.T, 2)
        ang = rng.uniform(0, np.pi) if dir_bias is None else dir_bias + rng.normal(0, 0.25)
        L = rng.uniform(0.3, 1.0) * len_mm
        wmm = width_mm * rng.uniform(0.6, 1.4)
        ext = int((L / 2 + wmm * 3) / cv.mmpp) + 2
        px0 = int(cx / cv.mmpp)
        py0 = int((cv.T - cz) / cv.mmpp)
        ys = (np.arange(py0 - ext, py0 + ext)) % S
        xs = (np.arange(px0 - ext, px0 + ext)) % S
        ly = (np.arange(-ext, ext) * cv.mmpp)[:, None]
        lx = (np.arange(-ext, ext) * cv.mmpp)[None, :]
        c, s = np.cos(ang), np.sin(ang)
        ru = lx * c - ly * s
        rv = lx * s + ly * c
        taper = np.clip(1 - (np.abs(ru) / (L / 2)) ** 2, 0, 1)
        sd = sd_capsule(ru, rv, L / 2, wmm)
        v = fill(sd, cv.mmpp * 1.2) * taper * rng.uniform(0.4, 1.0)
        sub = m[np.ix_(ys, xs)]
        m[np.ix_(ys, xs)] = np.maximum(sub, v)
    return m


def bolt_field(cv, pts, r_mm, dome_mm=1.2, kind="dome"):
    """Heights + mask for bolts/rivets at a list of (u, z) mm points (periodic)."""
    h = cv.zeros()
    m = cv.zeros()
    for (u, z) in pts:
        du = cv.wrap(cv.U, u)
        dz = cv.wrap(cv.Z, z)
        d2 = du * du + dz * dz
        inside = d2 < (r_mm * 1.6) ** 2
        if not inside.any():
            continue
        d = np.sqrt(d2[inside])
        prof = np.clip(1 - (d / r_mm) ** 2, 0, 1)
        if kind == "dome":
            hh = dome_mm * np.sqrt(prof)
        elif kind == "hex":  # flat hex-ish head with a slot
            hh = dome_mm * np.clip((r_mm - d) / (r_mm * 0.25), 0, 1)
            slot = (np.abs(du[inside]) < r_mm * 0.14) & (d < r_mm * 0.75)
            hh = np.where(slot, hh - dome_mm * 0.8, hh)
        else:  # countersunk: flush with recessed cross
            hh = -0.4 * np.clip((r_mm - d) / r_mm, 0, 1)
            cross = ((np.abs(du[inside]) < r_mm * 0.15) | (np.abs(dz[inside]) < r_mm * 0.15)) & (d < r_mm * 0.7)
            hh = np.where(cross, hh - 0.8, hh)
        h[inside] = np.where(np.abs(hh) > np.abs(h[inside]), hh, h[inside])
        m[inside] = np.maximum(m[inside], np.clip((r_mm - d) / (cv.mmpp * 1.5), 0, 1))
    return h, m


def grid_pts(u0, u1, du, z0, z1, dz):
    us = np.arange(u0, u1 + 1e-3, du) if du else [u0]
    zs = np.arange(z0, z1 + 1e-3, dz) if dz else [z0]
    return [(u, z) for u in us for z in zs]


# ==========================================================================
# Finishing: height -> normal/AO, wear, grime, write
# ==========================================================================
def normal_from_height(cv, h_mm, strength=1.0, micro=None, micro_amp=0.0):
    h = blur(h_mm, 0.6)
    if micro is not None:
        h = h + micro * micro_amp
    k = strength / (2 * cv.mmpp)
    dx = (np.roll(h, -1, 1) - np.roll(h, 1, 1)) * k
    dr = (np.roll(h, -1, 0) - np.roll(h, 1, 0)) * k  # d/d(row), row goes DOWN
    # DirectX tangent space: G stores -Y(up). With rows going down this is -dh/drow.
    nx, ny, nz = -dx, -dr, np.ones_like(dx)
    ln = np.sqrt(nx * nx + ny * ny + nz * nz)
    return np.stack([nx / ln, ny / ln, nz / ln], -1) * 0.5 + 0.5


def cavity_ao(cv, h_mm, radius_mm, depth_mm, strength):
    """Areas lower than their neighbourhood get occluded."""
    hb = blur(h_mm, radius_mm / cv.mmpp)
    occ = np.clip((hb - h_mm) / depth_mm, 0, 1)
    return (1 - occ * strength).astype(np.float32)


def convexity(cv, h_mm, radius_mm):
    return (h_mm - blur(h_mm, radius_mm / cv.mmpp)).astype(np.float32)


def write_set(name, cv, D, H, R, M, AO, E=None, extra=None, nstrength=1.0,
              micro=None, micro_amp=0.0):
    os.makedirs(OUT, exist_ok=True)
    N = normal_from_height(cv, H, nstrength, micro, micro_amp)
    if E is None:
        E = np.zeros_like(D)
    maps = {"D": D, "N": N, "E": E, "R": R, "M": M, "AO": AO}
    if extra:
        maps.update(extra)
    paths = {}
    for suf, arr in maps.items():
        a = np.clip(arr, 0, 1)
        a8 = np.round(a * 255).astype(np.uint8)
        img = Image.fromarray(a8, "RGB" if a8.ndim == 3 else "L")
        p = os.path.join(OUT, f"T_Int_{name}_{suf}.png")
        img.save(p, optimize=False, compress_level=6)
        paths[suf] = p
    return paths, maps


# ==========================================================================
# Texture sets
# ==========================================================================
SETS = {}


def texset(name, tile_mm, size, seed):
    def deco(fn):
        SETS[name] = dict(fn=fn, tile_mm=tile_mm, size=size, seed=seed)
        return fn
    return deco


# --------------------------------------------------------------------------
@texset("ShipDeck", 2000, 2048, 101)
def ship_deck(cv, rng):
    """1 m deck plates: two diamond-tread plates, one anti-slip grit plate,
    one plate with a flush access hatch. 6 mm seams, bolt rows."""
    U, Z = cv.U, cv.Z
    H = cv.zeros()
    col = cv.rgb(PAL["gunmetal"])
    R = cv.full(0.46)
    M = cv.full(1.0)

    # plate seams at 0/1000 in both axes
    su = np.minimum(np.abs(cv.rep(U, 1000.0, 0.0) - 0) , 1e9)
    du = np.abs(cv.rep(U, 1000.0, 500.0))  # distance to seam line u=0,1000
    dz = np.abs(cv.rep(Z, 1000.0, 500.0))
    dseam = np.minimum(500 - du, 500 - dz)  # distance from seam (0 at seam)
    plate_prof = bevel(-(dseam - 3.0), 2.5)  # 0 in 6mm seam, 1 on plate
    H += (plate_prof - 1) * 4.0

    qu = (U // 1000).astype(int)
    qz = (Z // 1000).astype(int)
    quad = qu + 2 * qz   # 0: bottom-left .. 3: top-right
    lu = cv.rep(U, 1000.0, 500.0)  # plate-local, centred
    lz = cv.rep(Z, 1000.0, 500.0)

    # --- diamond tread on quads 0 and 3
    cell = 32.0
    cu = cv.rep(U, cell)
    cz = cv.rep(Z, cell)
    parity = ((np.floor(U / cell) + np.floor(Z / cell)) % 2) * 2 - 1
    a = np.pi / 4 * parity
    ru = cu * np.cos(a) - cz * np.sin(a)
    rz = cu * np.sin(a) + cz * np.cos(a)
    lozenge = bevel(sd_capsule(ru, rz, 8.0, 2.6), 1.6)
    tread_area = ((quad == 0) | (quad == 3)) & (np.maximum(np.abs(lu), np.abs(lz)) < 470)
    tread = lozenge * tread_area
    H += tread * 1.6

    # --- anti-slip grit plate (quad 1): recessed field of dark grit coating
    grit_area = (quad == 1) & (np.maximum(np.abs(lu), np.abs(lz)) < 440)
    grit_edge = bevel(sd_box(lu, lz, 440, 440, 20), 3.0) * (quad == 1)
    gnoise = noise(cv, rng, 0.8)
    H += grit_edge * (0.6 + 0.35 * np.clip(gnoise, -1, 2))
    col = mix(col, cv.rgb((0.17, 0.175, 0.18)) * (0.85 + 0.3 * n01(gnoise)[..., None]), grit_edge)
    R = mix(R, cv.full(0.92), grit_edge)
    M = mix(M, cv.full(0.0), grit_edge)
    # hazard-yellow painted border around the grit field
    border = fill(np.abs(sd_box(lu, lz, 465, 465, 30)) - 9, cv.mmpp * 1.5) * (quad == 1)
    col = mix(col, cv.rgb(PAL["hazard"]) * 0.9, border * 0.85)
    R = mix(R, cv.full(0.62), border)
    M = mix(M, cv.full(0.0), border)

    # --- access hatch on quad 2: 560x720 lid, 4 mm reveal, finger-pull recesses
    hatch = (quad == 2)
    sd_h = sd_box(lu, lz, 280, 360, 18)
    reveal = fill(np.abs(sd_h) - 2.2, cv.mmpp) * hatch
    H -= reveal * 5.0
    for pz in (-230.0, 230.0):
        pull = bevel(sd_capsule(lu, lz - pz, 45, 14), 4) * hatch
        H -= pull * 5.0
    # hatch lid smooth (no tread) with low-profile stiffening ribs
    rib = bevel(np.abs(lu) - 6 + 0 * lz, 2.0) * fill(sd_box(lu, lz, 200, 300, 0), 1) * hatch
    H += rib * 1.2
    hp = [(1000 + 500 + sx * 250, 500 + sz * 330) for sx in (-1, 1) for sz in (-1, 1)]
    hb, hm = bolt_field(cv, hp, 7.0, kind="countersunk")
    H += hb

    # --- bolts along each plate edge every 250 mm, 22 mm in from the seam
    pts = []
    for q0u in (0.0, 1000.0):
        for q0z in (0.0, 1000.0):
            for t in np.arange(125.0, 1000.0, 250.0):
                pts += [(q0u + t, q0z + 22), (q0u + t, q0z + 978),
                        (q0u + 22, q0z + t), (q0u + 978, q0z + t)]
    bb, bm = bolt_field(cv, pts, 6.5, dome_mm=1.8, kind="hex")
    H += bb
    col = mix(col, cv.rgb(PAL["steel"]), bm * 0.7)

    # --- wear: foot traffic polishes a broad lane, tread tops go bright
    traffic = n01(fbm(cv, rng, 700, 3, aniso=(0.35, 1.0)), -1.2, 1.2)
    cvx = convexity(cv, H, 4.0)
    edge = np.clip(cvx / 0.8, 0, 1) * (M > 0.5)
    col = mix(col, cv.rgb(PAL["bare"]), np.clip(edge * 0.8 + tread * traffic * 0.5, 0, 1))
    R = R - tread * 0.12 * traffic - edge * 0.1 + (1 - traffic) * 0.06
    # paint tone variation per plate + large blotches
    tone = 0.035 * fbm(cv, rng, 300, 4)
    col = col * (1 + tone[..., None])
    # grime in seams and cavities
    AO = cavity_ao(cv, H, 10.0, 2.0, 0.75)
    grime = np.clip((1 - AO) * 1.6 + 0.25 * n01(fbm(cv, rng, 120, 4)) - 0.1, 0, 1)
    col = mix(col, col * np.array([0.42, 0.40, 0.37], np.float32), grime * 0.8)
    R = np.clip(R + grime * 0.25, 0.05, 1)
    micro = noise(cv, rng, 0.6)
    return dict(D=col, H=H, R=R, M=M, AO=AO, micro=micro, micro_amp=0.03)


# --------------------------------------------------------------------------
@texset("ShipWall", 2000, 2048, 202)
def ship_wall(cv, rng):
    """1 m wide bulkhead bays: raised kick plate (0-300), recessed painted
    main panel (300-1500) crossed by a teal ID stripe, upper vent/service
    panel (1500-2000). Rivet rows on every seam."""
    U, Z = cv.U, cv.Z
    lu = cv.rep(U, 1000.0, 500.0)  # bay-local, centred
    bay = (U // 1000).astype(int)
    H = cv.zeros()
    col = cv.rgb(PAL["paint_ship"])
    R = cv.full(0.55)
    M = cv.full(0.0)

    # structural seams: vertical at 0/1000, horizontal at 0(=2000), 300, 1500
    dv = 500 - np.abs(lu)
    dh = np.minimum.reduce([np.abs(cv.wrap(Z, 0)), np.abs(Z - 300), np.abs(Z - 1500)])
    dseam = np.minimum(dv, dh)
    H += (bevel(-(dseam - 4.0), 3.0) - 1) * 5.0

    # kick plate: bare dark metal, raised 3 mm, scuffed
    kick = fill(np.maximum(Z - 296, 4 - Z), cv.mmpp)
    H += kick * 3.0 * bevel(-(np.minimum(dv, np.minimum(Z - 4, 296 - Z)) - 2), 4)
    scuff = n01(fbm(cv, rng, 60, 4, aniso=(5.0, 0.6)), 0.2, 2.2) * kick
    col = mix(col, cv.rgb(PAL["paint_dark"]), kick)
    col = mix(col, cv.rgb(PAL["bare"]) * 0.8, scuff * 0.55)
    M = mix(M, cv.full(1.0), kick * 0.85)
    R = mix(R, cv.full(0.5) - scuff * 0.15, kick)

    # main panel: recessed inset 60 mm in from seams, 7 mm deep, 10 mm bevel
    mz = Z - 900.0
    inset = sd_box(lu, mz, 440, 540, 45)
    H -= bevel(inset, 10.0) * 7.0
    # thin raised lip around the inset
    lip = fill(np.abs(inset + 3) - 2.5, cv.mmpp) * 1.0
    H += lip * 1.2

    # teal ID stripe band 1040-1110 across full width (painted, no relief)
    stripe = fill(np.abs(Z - 1075) - 35, cv.mmpp * 1.5)
    col = mix(col, cv.rgb(PAL["teal"]), stripe)
    R = mix(R, cv.full(0.42), stripe)
    # thin white pinstripes above/below
    pin = fill(np.abs(np.abs(Z - 1075) - 47) - 3, cv.mmpp * 1.2)
    col = mix(col, cv.rgb((0.82, 0.83, 0.82)), pin * 0.9)
    # label plate on bay 0: dark plate with light "barcode"/ID ticks (mirror-safe)
    lab = fill(sd_box(lu + 250, Z - 1075, 110, 26, 4), cv.mmpp) * (bay == 0)
    H += lab * 1.0
    col = mix(col, cv.rgb((0.10, 0.11, 0.12)), lab)
    ticks = (np.abs(cv.rep(lu + 250, 14.0)) < 3.2) * fill(sd_box(lu + 250, Z - 1075, 90, 13), 1)
    tick_on = (np.floor((lu + 250) / 14.0) % 3 != 1)
    col = mix(col, cv.rgb((0.85, 0.86, 0.84)), ticks * tick_on * (bay == 0) * 0.9)
    # chevron pair on bay 1 (symmetric, so mirror-safe)
    chev = fill(np.abs(cv.rep(np.abs(lu - 250) * 0.9 + (Z - 1075), 55.0)) - 12, 1)
    chev = chev * fill(sd_box(lu - 250, Z - 1075, 120, 30), 1) * (bay == 1)
    col = mix(col, cv.rgb(PAL["black"]), chev * 0.75)

    # upper band: bay 0 = vent louvre, bay 1 = service hatch
    uz = Z - 1750.0
    vent_sd = sd_box(lu, uz, 330, 160, 12)
    vent = (bay == 0)
    H -= bevel(vent_sd, 6) * 6.0 * vent
    slat_z = cv.rep(Z, 22.0)
    slats = (0.5 + 0.5 * np.cos(slat_z / 22.0 * 2 * np.pi)) * fill(sd_box(lu, uz, 310, 140, 6), 1)
    H += slats * 5.0 * vent
    col = mix(col, cv.rgb(PAL["paint_dark"]), fill(vent_sd, 1) * vent * 0.9)
    hatch_sd = sd_box(lu, uz, 360, 180, 20)
    H -= fill(np.abs(hatch_sd) - 2.0, cv.mmpp) * 5.0 * (bay == 1)
    hp = [(1500 + sx * 330, 1750 + sz * 150) for sx in (-1, 1) for sz in (-1, 1)]
    hb, hm = bolt_field(cv, hp, 8.0, kind="countersunk")
    H += hb
    # small warning glyph square on hatch (hazard triangle-ish block)
    tri = fill(np.maximum(sd_box(lu - 0, uz - 60, 40, 32), (np.abs(lu) * 1.15 + (uz - 60) - 30)), 1) * (bay == 1)
    col = mix(col, cv.rgb(PAL["hazard"]), tri * 0.9)

    # rivet rows: vertical seams every 100 mm, horizontal seams every 100 mm
    pts = []
    for su in (0.0, 1000.0):
        for z in np.arange(50.0, 2000.0, 100.0):
            pts += [(su + 16, z), (su - 16, z)]
    for sz in (300.0, 1500.0):
        for u in np.arange(50.0, 2000.0, 100.0):
            pts += [(u, sz + 16), (u, sz - 16)]
    rb, rm = bolt_field(cv, pts, 5.0, dome_mm=1.6)
    H += rb

    # wear: paint chips on convex edges -> bare metal
    cvx = convexity(cv, H, 5.0)
    chip_n = n01(fbm(cv, rng, 25, 3), 0.0, 1.6)
    chips = np.clip(np.clip(cvx / 1.2, 0, 1) * 1.5 * chip_n - 0.15, 0, 1) * (1 - kick)
    col = mix(col, cv.rgb(PAL["bare"]), chips)
    M = mix(M, cv.full(1.0), chips)
    R = mix(R, cv.full(0.35), chips)
    col = mix(col, cv.rgb(PAL["steel"]), rm * 0.5)
    M = np.maximum(M, rm * 0.8)

    tone = 0.03 * fbm(cv, rng, 400, 4)
    col = col * (1 + tone[..., None])
    AO = cavity_ao(cv, H, 12.0, 2.5, 0.7)
    # vertical grime streaks from seams/vents downwards
    streak = n01(noise(cv, rng, 20, aniso=(0.6, 18.0)), 0.8, 2.6) * n01(fbm(cv, rng, 300, 2), -0.5, 1.5)
    grime = np.clip((1 - AO) * 1.4 + streak * 0.35, 0, 1)
    col = mix(col, col * np.array([0.55, 0.53, 0.50], np.float32), grime * 0.7)
    R = np.clip(R + grime * 0.18 + 0.04 * fbm(cv, rng, 40, 3), 0.05, 1)
    return dict(D=col, H=H, R=R, M=M, AO=AO, micro=noise(cv, rng, 0.8), micro_amp=0.02)


# --------------------------------------------------------------------------
@texset("ShipCeiling", 2000, 2048, 303)
def ship_ceiling(cv, rng):
    """Ceiling: dark structural ribs every 1 m, perforated acoustic panels,
    160 mm recessed light strip running along U through the middle."""
    U, Z = cv.U, cv.Z
    lu = cv.rep(U, 1000.0, 500.0)
    H = cv.zeros()
    col = cv.rgb((0.56, 0.58, 0.61))
    R = cv.full(0.62)
    M = cv.full(0.0)

    # ribs along Z at u = 0/1000, 90 mm wide, protrude 25 mm
    rib_sd = np.abs(cv.rep(U, 1000.0)) - 45
    rib = bevel(rib_sd, 8)
    H += rib * 25
    col = mix(col, cv.rgb(PAL["gunmetal"]), fill(rib_sd, 1))
    M = mix(M, cv.full(1.0), fill(rib_sd, 1))
    R = mix(R, cv.full(0.45), fill(rib_sd, 1))

    # light strip: channel along U at Z = 1000
    dzs = np.abs(Z - 1000)
    chan = bevel(dzs - 110, 6)          # channel 220 wide recessed 30 mm
    H -= chan * 30
    diff = fill(dzs - 80, cv.mmpp * 1.5)  # 160 mm diffuser
    H += diff * 12
    col = mix(col, cv.rgb((0.20, 0.21, 0.22)), chan * (1 - diff))
    col = mix(col, cv.rgb((0.92, 0.95, 0.98)), diff)
    R = mix(R, cv.full(0.3), diff)
    M = mix(M, cv.full(0.0), chan)
    # diffuser has faint segment breaks every 500 mm (LED module joints)
    joints = fill(np.abs(cv.rep(U, 500.0)) - 1.5, cv.mmpp) * diff
    E_mask = diff * (1 - joints * 0.8)
    # soft falloff across the diffuser (brighter in the middle)
    E_mask *= 0.75 + 0.25 * np.cos(np.clip(dzs / 80, 0, 1) * np.pi / 2)

    # panels: rounded 12 mm reveal, panel field between ribs & channel
    pz = np.where(Z > 1000, Z - 1555, Z - 445)
    psd = sd_box(lu, pz, 430, 405, 25)
    H -= fill(np.abs(psd) - 2.5, cv.mmpp) * 4
    # perforation: 24 mm grid, 3 mm holes, inside panel inner field
    hole = sd_circle(cv.rep(U, 24.0), cv.rep(Z, 24.0), 3.0)
    field = fill(sd_box(lu, pz, 380, 355, 20), 1)
    holes = fill(hole, cv.mmpp) * field
    H -= holes * 1.5
    col = mix(col, col * 0.35, holes * 0.85)
    # panel clips
    pts = [(u0 + sx * 400, z0 + sz * 375) for u0 in (500.0, 1500.0) for z0 in (445.0, 1555.0)
           for sx in (-1, 1) for sz in (-1, 1)]
    cb, cm = bolt_field(cv, pts, 6, kind="countersunk")
    H += cb

    AO = cavity_ao(cv, H, 25, 6, 0.75) * cavity_ao(cv, H, 6, 2, 0.4)
    tone = 0.025 * fbm(cv, rng, 350, 4)
    col = col * (1 + tone[..., None])
    grime = np.clip((1 - AO) * 1.1 + 0.15 * n01(fbm(cv, rng, 200, 3)), 0, 1) * (1 - diff)
    col = mix(col, col * 0.6, grime * 0.6)
    E = cv.rgb((0.88, 0.94, 1.0)) * E_mask[..., None]
    return dict(D=col, H=H, R=np.clip(R + grime * 0.1, 0, 1), M=M, AO=AO, E=E,
                micro=noise(cv, rng, 0.8), micro_amp=0.02)


# --------------------------------------------------------------------------
@texset("EngWall", 2000, 2048, 404)
def eng_wall(cv, rng):
    """Engineering wall: dark stiffened plating, colour-coded pipe runs with
    flanges/clamps, conduit bundle, junction box, hazard band at the floor."""
    U, Z = cv.U, cv.Z
    H = cv.zeros()
    col = cv.rgb((0.22, 0.225, 0.235))
    R = cv.full(0.58)
    M = cv.full(0.9)

    # background plating: 500 mm vertical stiffeners, plate seams at 1000
    stiff = bevel(np.abs(cv.rep(U, 500.0)) - 20, 6)
    H += stiff * 10
    seam = fill(np.abs(cv.rep(Z, 1000.0, 500.0)) - 497, cv.mmpp)  # seams at z=0,1000
    H -= seam * 3
    pts = [(u, z) for u in np.arange(250.0, 2000.0, 500.0) for z in np.arange(40.0, 2000.0, 80.0)]
    pts = [(u + s, z) for (u, z) in pts for s in (-0.0,)]
    rb, rm = bolt_field(cv, [(u - 250 + 0, z) for (u, z) in pts], 6.0, dome_mm=2.0)
    H += rb

    # hazard band z = 0..160 (45 deg, 100 mm period -> tiles on 2000 mm)
    band = fill(np.maximum(Z - 160, -Z + 0.0), cv.mmpp)
    diag = cv.rep(U + Z, 100.0)
    yel = fill(np.abs(diag) - 25, cv.mmpp * 1.5)
    hz = mix(cv.rgb(PAL["black"]), cv.rgb(PAL["hazard"]), yel)
    col = mix(col, hz, band)
    R = mix(R, cv.full(0.6), band)
    M = mix(M, cv.full(0.0), band)

    pipe_mask = cv.zeros()
    pipe_col = cv.rgb((0, 0, 0))
    runs = [  # (centre z, radius, colour, metal, rough)
        (1720.0, 90.0, (0.80, 0.62, 0.12), 0.0, 0.5),    # yellow: gas
        (1500.0, 60.0, (0.16, 0.34, 0.60), 0.0, 0.45),   # blue: coolant
        (1350.0, 40.0, (0.62, 0.40, 0.26), 1.0, 0.35),   # bare copper
        (1260.0, 28.0, (0.55, 0.56, 0.57), 1.0, 0.4),    # steel line
    ]
    pr = cv.full(0.0)
    pm = cv.full(0.0)
    for zc, r, c, mt, rg in runs:
        d = np.abs(Z - zc)
        inside = d < r
        prof = np.sqrt(np.clip(1 - (d / r) ** 2, 0, 1))
        h = (r * 0.6 + r * prof) * inside
        # flange/clamp every 500 mm, 40 mm wide, +8% radius
        cu = np.abs(cv.rep(U, 500.0, 125.0))
        clamp = fill(cu - 20, cv.mmpp) * (d < r * 1.12)
        prof_c = np.sqrt(np.clip(1 - (d / (r * 1.12)) ** 2, 0, 1))
        hc = (r * 0.6 + r * 1.12 * prof_c) * clamp
        hh = np.maximum(h, hc)
        m_here = np.maximum(inside.astype(np.float32), clamp)
        H = np.where(m_here > 0, np.maximum(H, hh), H)
        pc = cv.rgb(c)
        # ID tape on painted pipes: white band with a dark stripe, every 1000 mm
        if mt == 0.0:
            tape = fill(np.abs(cv.rep(U, 1000.0, 750.0)) - 45, cv.mmpp) * inside
            pc = mix(pc, cv.rgb((0.85, 0.85, 0.82)), tape)
            pc = mix(pc, cv.rgb(PAL["black"]), tape * fill(np.abs(cv.rep(U, 1000.0, 750.0)) - 12, 1))
        pc = mix(pc, cv.rgb((0.35, 0.36, 0.37)), clamp)
        pipe_col = mix(pipe_col, pc, m_here)
        pipe_mask = np.maximum(pipe_mask, m_here)
        pr = np.where(m_here > 0, np.where(clamp > 0.5, 0.45, rg), pr)
        pm = np.where(m_here > 0, np.where(clamp > 0.5, 1.0, mt), pm)
        # clamp bolts
        H += bolt_field(cv, [(u, zc + r * 1.2) for u in np.arange(125.0, 2000, 500.0)] +
                        [(u, zc - r * 1.2) for u in np.arange(125.0, 2000, 500.0)], 7, 3.0, "hex")[0]
    # shading across the pipe cylinder (fake AO in D for readability at distance)
    col = mix(col, pipe_col, pipe_mask)
    R = mix(R, pr, pipe_mask)
    M = mix(M, pm, pipe_mask)

    # conduit bundle: 3 x 36 mm at z 560/610/660 (grey)
    for zc in (560.0, 610.0, 660.0):
        d = np.abs(Z - zc)
        inside = d < 18
        h = (10 + 18 * np.sqrt(np.clip(1 - (d / 18) ** 2, 0, 1))) * inside
        H = np.where(inside, np.maximum(H, h), H)
        col = mix(col, cv.rgb((0.40, 0.41, 0.42)), inside.astype(np.float32))
        M = np.where(inside, 0.2, M)
        R = np.where(inside, 0.55, R)
    # conduit straps every 400 mm
    strap = fill(np.abs(cv.rep(U, 400.0, 200.0)) - 12, cv.mmpp) * fill(np.abs(Z - 610) - 34, 1)
    H = np.maximum(H, strap * 34)
    col = mix(col, cv.rgb((0.18, 0.18, 0.19)), strap)

    # junction box at u=1500, z=900 + vertical conduit up to the bundle
    jb_sd = sd_box(cv.wrap(U, 1500), Z - 920, 170, 190, 10)
    jb = bevel(jb_sd, 8)
    H = np.maximum(H, jb * 45)
    col = mix(col, cv.rgb((0.33, 0.35, 0.37)), fill(jb_sd, 1))
    M = mix(M, cv.full(0.3), fill(jb_sd, 1))
    R = mix(R, cv.full(0.5), fill(jb_sd, 1))
    lid = fill(np.abs(sd_box(cv.wrap(U, 1500), Z - 920, 150, 170, 6)) - 1.5, cv.mmpp)
    H -= lid * 3
    warn = fill(sd_box(cv.wrap(U, 1500), Z - 990, 70, 45, 5), 1)
    col = mix(col, cv.rgb(PAL["hazard"]), warn)
    warnk = fill(np.maximum(np.abs(cv.wrap(U, 1500)) * 1.3 + (Z - 990) - 25, -(Z - 990) - 22), 1) * warn
    col = mix(col, cv.rgb(PAL["black"]), warnk)
    vc = np.abs(cv.wrap(U, 1500)) < 22
    vz = (Z > 1110) & (Z < 1300) | ((Z > 680) & (Z < 730))
    vmask = (vc & vz).astype(np.float32)
    H = np.where(vmask > 0, np.maximum(H, 12 + 22 * np.sqrt(np.clip(1 - (cv.wrap(U, 1500) / 22) ** 2, 0, 1))), H)
    col = mix(col, cv.rgb((0.40, 0.41, 0.42)), vmask)

    # grime: oil streaks running down from pipes, dust on top surfaces
    AO = cavity_ao(cv, H, 30, 10, 0.8) * cavity_ao(cv, H, 6, 2, 0.4)
    streak = n01(noise(cv, rng, 18, aniso=(0.6, 22.0)), 0.6, 2.5)
    below = np.clip((1320 - Z) / 600, 0, 1) * np.clip(Z / 300, 0, 1)
    oil = np.clip(streak * below * n01(fbm(cv, rng, 250, 2), -0.8, 1.0), 0, 1)
    col = mix(col, col * np.array([0.35, 0.32, 0.28], np.float32), oil * 0.7)
    R = mix(R, cv.full(0.25), oil * 0.5)  # oily = glossier
    cvx = convexity(cv, H, 6.0)
    edge = np.clip(cvx / 3.0, 0, 1)
    col = mix(col, cv.rgb(PAL["bare"]), edge * 0.35 * (1 - band))
    grime = np.clip((1 - AO) * 1.2, 0, 1)
    col = mix(col, col * 0.5, grime * 0.6)
    tone = 0.04 * fbm(cv, rng, 300, 4)
    col = col * (1 + tone[..., None])
    # tiny status LED on junction box
    led = fill(sd_circle(cv.wrap(U, 1500) - 120, Z - 800, 6), cv.mmpp)
    E = cv.rgb((0.1, 1.0, 0.35)) * led[..., None]
    col = mix(col, cv.rgb((0.3, 0.9, 0.45)), led)
    return dict(D=col, H=H, R=np.clip(R + grime * 0.15, 0.05, 1), M=M, AO=AO, E=E,
                micro=noise(cv, rng, 0.8), micro_amp=0.03, nstrength=0.6)


# --------------------------------------------------------------------------
@texset("EngGrate", 2000, 2048, 505)
def eng_grate(cv, rng):
    """Bar grating floor: 1 m panels with solid 60 mm frames, 5 mm bearing
    bars at 35 mm, cross bars at 100 mm, serrated tops. _MASK = opacity
    (1 metal / 0 void) for a Masked material; the void is also painted dark
    so it reads correctly on an opaque material too."""
    U, Z = cv.U, cv.Z
    lu = cv.rep(U, 1000.0, 500.0)
    lz = cv.rep(Z, 1000.0, 500.0)
    frame_sd = -sd_box(lu, lz, 440, 440, 0)  # >0 in the frame band
    frame = fill(frame_sd, cv.mmpp)           # 1 in the frame band (outside the bar field)
    bear = fill(np.abs(cv.rep(U, 2000.0 / 56)) - 2.6, cv.mmpp)  # 35.7 mm pitch, divides the tile
    cross = fill(np.abs(cv.rep(Z, 100.0)) - 2.6, cv.mmpp)
    bars = np.maximum(bear, cross)
    solid = np.maximum(frame, bars)
    # heights: void -40, bars 0 (serrated: 1.2 mm notches every 12 mm), frame 0
    serr = 1.2 * (0.5 + 0.5 * np.cos(Z / 12.5 * 2 * np.pi)) * bear * (1 - frame)
    H = (solid - 1) * 40.0 - serr + cross * 0.3
    H += frame * 1.5
    # frame seam between panels
    seam = fill(np.minimum(500 - np.abs(lu), 500 - np.abs(lz)) - 2.5, cv.mmpp)
    H -= seam * 4
    pts = [(u0 + sx * 470, z0 + sz * 470) for u0 in (500.0, 1500.0) for z0 in (500.0, 1500.0)
           for sx in (-1, 1) for sz in (-1, 1)]
    pts += [(u0 + sx * 470, z0) for u0 in (500.0, 1500.0) for z0 in (500.0, 1500.0) for sx in (-1, 1)]
    pts += [(u0, z0 + sz * 470) for u0 in (500.0, 1500.0) for z0 in (500.0, 1500.0) for sz in (-1, 1)]
    bb, bm = bolt_field(cv, pts, 8, dome_mm=2.5, kind="hex")
    H += bb * frame

    galv = cv.rgb((0.45, 0.46, 0.465))
    spangle = noise(cv, rng, 6)
    col = galv * (1 + 0.05 * spangle[..., None])
    traffic = n01(fbm(cv, rng, 600, 3), -1.0, 1.3)
    col = mix(col, cv.rgb(PAL["bare"]), (1 - serr / 1.3 * 0) * bear * traffic * 0.45)
    # yellow painted frame edge stripe (safety edge marking)
    void_col = cv.rgb((0.025, 0.027, 0.03)) * (1 + 0.3 * n01(fbm(cv, rng, 200, 3))[..., None])
    col = mix(void_col, col, solid)
    R = mix(cv.full(0.9), cv.full(0.5) - traffic * 0.15 * bear, solid)
    M = mix(cv.full(0.0), cv.full(1.0), solid)
    AO = mix(cv.full(0.15), cavity_ao(cv, H * solid, 8, 2, 0.6), solid)
    grime = n01(fbm(cv, rng, 80, 4), 0.4, 2.0) * solid * (1 - traffic * 0.6)
    col = mix(col, col * np.array([0.45, 0.42, 0.38], np.float32), grime * 0.6)
    R = np.clip(R + grime * 0.2, 0.05, 1)
    return dict(D=col, H=H, R=R, M=M, AO=AO, extra={"MASK": solid},
                micro=noise(cv, rng, 0.8), micro_amp=0.03, nstrength=0.5)


# --------------------------------------------------------------------------
@texset("Hazard", 1000, 1024, 606)
def hazard(cv, rng):
    """Hazard trim: 45 deg yellow/black, 200 mm period, chipped + dirty."""
    U, Z = cv.U, cv.Z
    diag = cv.rep(U + Z, 200.0)
    yel = fill(np.abs(diag) - 50, cv.mmpp * 1.5)
    col = mix(cv.rgb(PAL["black"]), cv.rgb(PAL["hazard"]), yel)
    chip = n01(fbm(cv, rng, 30, 4), 0.9, 1.8)
    chip = np.clip(chip * 1.4, 0, 1)
    H = -chip * 0.25 + yel * 0.08
    col = mix(col, cv.rgb(PAL["steel"]), chip)
    R = mix(cv.full(0.58), cv.full(0.38), chip)
    M = mix(cv.full(0.0), cv.full(1.0), chip)
    wear = n01(fbm(cv, rng, 200, 3), -0.5, 1.5)
    col = mix(col, col * np.array([0.5, 0.47, 0.42], np.float32), wear * 0.45)
    sc = scratches(cv, rng, 120, 80, 0.5)
    col = mix(col, cv.rgb(PAL["bare"]), sc * 0.6)
    R = np.clip(R + wear * 0.15 - sc * 0.2, 0.05, 1)
    AO = cavity_ao(cv, H, 4, 0.2, 0.3)
    return dict(D=col, H=H, R=R, M=np.maximum(M, sc * 0.8), AO=AO,
                micro=noise(cv, rng, 0.6), micro_amp=0.03)


# --------------------------------------------------------------------------
@texset("HabWall", 2000, 2048, 707)
def hab_wall(cv, rng):
    """Habitation wall: warm wood-composite wainscot (0-900), brass rail
    (900-960), tufted padded upholstery panels (960-2000)."""
    U, Z = cv.U, cv.Z
    H = cv.zeros()
    col = cv.rgb(PAL["cream"])
    R = cv.full(0.86)
    M = cv.full(0.0)

    # --- wainscot: horizontal laminate boards 150 mm with grain
    wain = fill(Z - 900, cv.mmpp)
    board = np.floor(Z / 150.0)
    grain = noise(cv, rng, 1.5, aniso=(40.0, 1.0)) * 0.6 + noise(cv, rng, 6.0, aniso=(25.0, 1.0))
    wood = cv.rgb(PAL["wood"]) * (1 + 0.12 * np.clip(grain, -2, 2)[..., None])
    bshift = (np.sin(board * 12.9898) * 43758.5453) % 1.0
    wood = wood * (0.92 + 0.14 * bshift[..., None])
    gap = fill(np.abs(cv.rep(Z, 150.0)) - 1.0, cv.mmpp) * wain
    col = mix(col, wood, wain)
    col = mix(col, col * 0.4, gap)
    H += (-gap * 1.0 + grain * 0.04) * wain
    R = mix(R, cv.full(0.52) + 0.06 * grain, wain)
    # vertical panel stiles every 1000 mm on wainscot
    stile = fill(np.abs(cv.rep(U, 1000.0)) - 1.2, cv.mmpp) * wain
    H -= stile * 1.5
    col = mix(col, col * 0.5, stile)

    # --- brass rail
    rz = np.abs(Z - 930)
    rail = rz < 26
    prof = np.sqrt(np.clip(1 - (rz / 26) ** 2, 0, 1))
    H = np.where(rail, 8 + 12 * prof, H)
    brush = noise(cv, rng, 0.8, aniso=(60.0, 1.0))
    col = mix(col, cv.rgb(PAL["brass"]) * (1 + 0.05 * brush[..., None]), rail.astype(np.float32))
    M = np.where(rail, 1.0, M)
    R = np.where(rail, 0.32 + 0.04 * brush, R)
    # rail brackets every 1000 mm
    br = fill(sd_box(cv.rep(U, 1000.0, 500.0), Z - 930, 25, 34, 6), cv.mmpp)
    H = np.maximum(H, br * 10)
    col = mix(col, cv.rgb(PAL["brass"]) * 0.7, br)

    # --- upholstery: tufted cells 250 x 260 mm between 960 and 2000
    up = fill(np.maximum(960 - Z, Z - 1996), cv.mmpp)
    # diamond (Chesterfield-style) tufting: rotated grid, p=250 divides the 2 m tile
    p = 250.0
    ra = cv.rep(U + Z, p)          # along one diagonal
    rb_ = cv.rep(U - Z, p)         # along the other
    cell_d = np.minimum(p / 2 - np.abs(ra), p / 2 - np.abs(rb_)) / np.sqrt(2)  # dist to crease
    puff = np.clip(cell_d / 60, 0, 1)
    puff = np.sin(puff * np.pi / 2) ** 0.7
    ta = cv.rep(U + Z, p, p / 2)
    tb = cv.rep(U - Z, p, p / 2)
    td = np.sqrt(ta * ta + tb * tb) / np.sqrt(2)
    tuft = np.exp(-(td / 40) ** 2)
    pad_h = (17 * puff - 8 * tuft) * up
    button = fill(td - 8, cv.mmpp) * up
    pad_h += button * 5
    H += pad_h
    cu, cz = ra, rb_
    # stitched seams between cells
    stitch = fill(cell_d - 1.0, cv.mmpp) * (tuft < 0.3) * up
    # fabric weave
    weave = (np.sin(U / 2.0 * np.pi) * np.sin(Z / 2.0 * np.pi))  # 4 mm period, divides the tile
    fab = cv.rgb((0.60, 0.50, 0.40)) * (1 + 0.06 * noise(cv, rng, 40)[..., None])
    fab = fab * (0.9 + 0.1 * puff[..., None])  # tuft valleys a touch darker in colour
    col = mix(col, fab, up)
    col = mix(col, cv.rgb((0.30, 0.24, 0.19)), button)
    col = mix(col, col * 0.7, stitch)
    R = mix(R, cv.full(0.9) - weave * 0.03, up)
    # top cap strip of the upholstery
    cap = fill(np.abs(Z - 1998) - 4, cv.mmpp)
    col = mix(col, cv.rgb(PAL["brass"]) * 0.8, cap)
    M = np.maximum(M, cap)

    AO = cavity_ao(cv, H, 40, 10, 0.35) * cavity_ao(cv, H, 5, 1, 0.25)
    grime = np.clip((1 - AO) * 0.8, 0, 1)
    col = mix(col, col * 0.75, grime * 0.5)
    # scuffs on the wainscot bottom
    scuff = n01(fbm(cv, rng, 40, 3, aniso=(5.0, 0.6)), 0.8, 2.2) * np.clip((250 - Z) / 250, 0, 1)
    col = mix(col, col * 0.75, scuff * 0.6)
    tone = 0.02 * fbm(cv, rng, 500, 3)
    col = col * (1 + tone[..., None])
    micro = weave * 0.2 * up + noise(cv, rng, 0.8)
    return dict(D=col, H=H, R=np.clip(R, 0.05, 1), M=M, AO=AO, micro=micro, micro_amp=0.15,
                nstrength=0.55)


# --------------------------------------------------------------------------
@texset("HabFloor", 2000, 2048, 808)
def hab_floor(cv, rng):
    """Habitation floor: 500 mm carpet tiles laid quarter-turn (pile
    direction alternates), warm umber with a subtle heather."""
    U, Z = cv.U, cv.Z
    tu = np.floor(U / 500.0)
    tz = np.floor(Z / 500.0)
    alt = ((tu + tz) % 2).astype(np.float32)
    pile_a = noise(cv, rng, 1.2, aniso=(0.12, 1.0))
    pile_b = noise(cv, rng, 1.2, aniso=(1.0, 0.12))
    pile = mix(pile_a, pile_b, alt)
    heather = noise(cv, rng, 1.5)
    col = cv.rgb(PAL["carpet"]) * (1 + 0.10 * pile[..., None] + 0.06 * heather[..., None])
    # tile brightness shift from pile direction (the classic quarter-turn read)
    col = col * (0.95 + 0.08 * alt[..., None])
    # subtle woven accent line: loop-pile stripe 30 mm from each tile edge
    lu = cv.rep(U, 500.0, 250.0)
    lz = cv.rep(Z, 500.0, 250.0)
    line = fill(np.abs(sd_box(lu, lz, 200, 200, 30)) - 6, cv.mmpp * 1.5)
    col = mix(col, cv.rgb((0.46, 0.34, 0.23)), line * 0.3)
    seam = fill(np.minimum(250 - np.abs(lu), 250 - np.abs(lz)) - 0.8, cv.mmpp)
    H = pile * 0.25 - seam * 1.2 + line * 0.3
    col = mix(col, col * 0.6, seam)
    wear = n01(fbm(cv, rng, 800, 3), -1.0, 1.5)
    col = mix(col, col * np.array([0.82, 0.80, 0.78], np.float32), wear * 0.35)
    R = np.clip(0.92 + 0.04 * pile - wear * 0.05, 0, 1)
    M = cv.zeros()
    AO = np.clip(0.9 + 0.06 * pile - seam * 0.4, 0, 1)
    return dict(D=col, H=H, R=R, M=M, AO=AO, micro=None, micro_amp=0.0, nstrength=0.8)


# --------------------------------------------------------------------------
@texset("Console", 1000, 1024, 909)
def console(cv, rng):
    """Console face (1 m tile): two bezelled screens with emissive UI, a
    button grid with lit keys, toggle row, sliders, screws, label strips."""
    U, Z = cv.U, cv.Z
    H = cv.zeros()
    col = cv.rgb((0.10, 0.108, 0.118))
    R = cv.full(0.68)
    M = cv.full(0.0)
    E = cv.rgb((0, 0, 0))
    # outer panel seam at tile edges
    seam = fill(np.minimum(np.minimum(U, 1000 - U), np.minimum(Z, 1000 - Z)) - 2.5, cv.mmpp)
    H -= seam * 3
    sp = [(u, z) for u in (15.0, 985.0) for z in (15.0, 500.0, 985.0)]
    sb, sm = bolt_field(cv, sp, 5, kind="countersunk")
    H += sb
    col = mix(col, cv.rgb((0.4, 0.41, 0.42)), sm)
    M = np.maximum(M, sm)

    # screens
    for cx in (260.0, 740.0):
        su, sz = U - cx, Z - 760
        bez_sd = sd_box(su, sz, 225, 150, 18)
        scr_sd = sd_box(su, sz, 200, 125, 6)
        H += bevel(bez_sd, 6) * 5
        H -= bevel(scr_sd, 2) * 3
        bez = fill(bez_sd, cv.mmpp) * (1 - fill(scr_sd, cv.mmpp))
        col = mix(col, cv.rgb((0.18, 0.19, 0.2)), bez)
        R = mix(R, cv.full(0.45), bez)
        M = mix(M, cv.full(0.6), bez)
        scr = fill(scr_sd, cv.mmpp)
        col = mix(col, cv.rgb((0.015, 0.02, 0.025)), scr)
        R = mix(R, cv.full(0.06), scr)
        M = mix(M, cv.full(0.0), scr)
        # UI: faint grid + waveform + bar graph (teal / amber)
        grid = (fill(np.abs(cv.rep(su, 40.0)) - 0.6, cv.mmpp) + fill(np.abs(cv.rep(sz, 40.0)) - 0.6, cv.mmpp))
        ui = np.clip(grid, 0, 1) * 0.18
        wave = fill(np.abs(sz - 45 - 35 * np.sin(su / 38.0 + cx) * np.cos(su / 91.0)) - 1.6, cv.mmpp)
        ui = np.maximum(ui, wave * (su < 180) * (su > -180))
        bars = np.zeros_like(U)
        for i in range(9):
            bx = -170 + i * 24
            hgt = 20 + 60 * ((np.sin(i * 1.7 + cx) + 1) / 2)
            bars = np.maximum(bars, fill(sd_box(su - bx, sz + 100 - hgt / 2 * 0 - 0, 8, 0.1, 0) , 1) * 0)
            bars = np.maximum(bars, fill(sd_box(su - bx, sz - (-105 + hgt / 2), 8, hgt / 2), cv.mmpp))
        uicol = cv.rgb((0.15, 0.85, 0.9) if cx < 500 else (0.25, 0.8, 0.95))
        barcol = cv.rgb((1.0, 0.62, 0.15))
        e_scr = uicol * ui[..., None] + barcol * bars[..., None] * 0.9
        e_scr += cv.rgb((0.02, 0.06, 0.07)) * 1.0  # screen backlight
        E = mix(E, e_scr, scr)
        col = mix(col, np.clip(e_scr * 0.6, 0, 1), scr * 0.8)

    # button grid 8 x 3, 40 mm keys on 55 mm pitch, lower-left
    lit_colors = [(1.0, 0.62, 0.15), (0.2, 0.95, 0.45), (0.95, 0.2, 0.15), (0.2, 0.7, 1.0)]
    for i in range(8):
        for j in range(3):
            bu, bz = U - (90 + i * 55), Z - (180 + j * 55)
            bsd = sd_box(bu, bz, 20, 20, 5)
            key = bevel(bsd, 3)
            H += key * 3
            k = fill(bsd, cv.mmpp)
            idx = (i * 7 + j * 3) % 11
            if idx < 4:
                c = lit_colors[idx]
                lens = fill(sd_box(bu, bz, 14, 14, 4), cv.mmpp)
                col = mix(col, cv.rgb(c) * 0.8, lens)
                E = mix(E, cv.rgb(c) * 0.9, lens)
                R = mix(R, cv.full(0.2), lens)
                col = mix(col, cv.rgb((0.22, 0.23, 0.24)), k * (1 - lens))
            else:
                col = mix(col, cv.rgb((0.24, 0.25, 0.26)), k)
                R = mix(R, cv.full(0.5), k)
    # label strip above the keys
    lab = fill(sd_box(U - 282, Z - 318, 200, 10, 2), cv.mmpp)
    col = mix(col, cv.rgb((0.70, 0.71, 0.69)), lab)
    ticks = fill(np.abs(cv.rep(U - 90, 55.0)) - 12, 1) * fill(sd_box(U - 282, Z - 318, 195, 3), 1)
    col = mix(col, cv.rgb((0.1, 0.1, 0.1)), ticks)

    # toggle switches row (right), with guards
    for i in range(6):
        tu, tz = U - (620 + i * 55), Z - 290
        base = fill(sd_circle(tu, tz, 14), cv.mmpp)
        H += bevel(sd_circle(tu, tz, 14), 3) * 2
        lever = bevel(sd_capsule(tz * 0 + tu, tz - 8, 0, 5) , 2)
        lever = bevel(sd_box(tu, tz - 10, 4, 14, 4), 2)
        H += lever * 6
        col = mix(col, cv.rgb((0.55, 0.56, 0.58)), np.maximum(base * 0.6, fill(sd_box(tu, tz - 10, 4, 14, 4), 1)))
        M = np.maximum(M, base * 0.9)
        # status LED above each toggle
        led = fill(sd_circle(tu, tz + 40, 4), cv.mmpp)
        c = (0.2, 1.0, 0.4) if i % 3 else (1.0, 0.6, 0.1)
        E = mix(E, cv.rgb(c), led)
        col = mix(col, cv.rgb(c), led)
    # sliders (right, lower)
    for i in range(4):
        su, sz = U - (640 + i * 80), Z - 150
        slot = fill(sd_box(su, sz, 4, 70, 4), cv.mmpp)
        H -= slot * 4
        col = mix(col, cv.rgb((0.02, 0.02, 0.02)), slot)
        kpos = -40 + 25 * i
        knob = sd_box(su, sz - kpos, 14, 9, 3)
        H += bevel(knob, 2) * 5
        col = mix(col, cv.rgb((0.35, 0.36, 0.37)), fill(knob, cv.mmpp))

    # fine texture: powder-coat speckle, finger grime, faint scratches
    sc = scratches(cv, rng, 80, 60, 0.35)
    col = mix(col, cv.rgb((0.3, 0.3, 0.31)), sc * 0.35)
    R = np.clip(R - sc * 0.15 + 0.03 * noise(cv, rng, 2.0), 0.03, 1)
    AO = cavity_ao(cv, H, 6, 1.5, 0.6)
    col = col * AO[..., None] ** 0.3
    return dict(D=col, H=H, R=R, M=M, AO=AO, E=np.clip(E, 0, 1),
                micro=noise(cv, rng, 0.8), micro_amp=0.04, nstrength=0.8)


# --------------------------------------------------------------------------
@texset("Glass", 2000, 1024, 1010)
def glass(cv, rng):
    """Viewport glass: near-neutral cool tint, wiper-arc-free (tileable)
    smudges, fine scratches. _MASK = dirt/smudge amount (0 clean .. 1 dirty),
    drive Opacity and Roughness from it in a Translucent material."""
    U, Z = cv.U, cv.Z
    smudge = n01(fbm(cv, rng, 250, 5), 0.2, 2.4) ** 1.5
    prints = n01(noise(cv, rng, 18), 1.4, 3.0) * n01(fbm(cv, rng, 400, 2), 0.3, 1.5)
    dust = n01(noise(cv, rng, 1.2), 2.0, 3.5)
    sc = scratches(cv, rng, 260, 160, 0.25)
    sc2 = scratches(cv, rng, 40, 400, 0.15, dir_bias=0.1)  # long cleaning-streak scratches
    scr = np.clip(sc + sc2 * 0.7, 0, 1)
    dirt = np.clip(smudge * 0.7 + prints * 0.6 + dust * 0.45 + scr * 0.35, 0, 1)
    tint = cv.rgb((0.80, 0.88, 0.92))
    dirt_col = cv.rgb((0.55, 0.52, 0.48))
    col = mix(tint, dirt_col, np.clip(dirt * 0.8, 0, 1))
    R = np.clip(0.04 + smudge * 0.25 + prints * 0.25 + scr * 0.3 + dust * 0.2, 0, 1)
    M = cv.zeros()
    AO = cv.full(1.0)
    H = -scr * 0.05 + prints * 0.01
    return dict(D=col, H=H, R=R, M=M, AO=AO, extra={"MASK": dirt},
                micro=None, micro_amp=0.0, nstrength=1.0)


# --------------------------------------------------------------------------
@texset("StnFloor", 2000, 2048, 1111)
def stn_floor(cv, rng):
    """Station concourse floor: polished terrazzo-composite tiles 1000 x 500,
    running bond, 5 mm grout, a brass inlay line once per 2 m."""
    U, Z = cv.U, cv.Z
    row = np.floor(Z / 500.0)
    off = (row % 2) * 500.0
    lu = cv.rep(U - off, 1000.0, 500.0)
    lz = cv.rep(Z, 500.0, 250.0)
    dgr = np.minimum(500 - np.abs(lu), 250 - np.abs(lz))
    grout = fill(dgr - 2.5, cv.mmpp)
    base = cv.rgb((0.60, 0.575, 0.535))
    # per-tile tone
    tid = np.floor((U - off) / 1000.0) * 7 + row * 3
    tt = ((np.sin(tid * 12.9898) * 43758.5453) % 1.0) - 0.5
    col = base * (1 + 0.05 * tt[..., None])
    # terrazzo chips: 3 chip populations
    for sz, thr, c in ((3.0, 2.0, (0.30, 0.28, 0.26)), (5.0, 2.3, (0.82, 0.80, 0.76)),
                       (4.0, 2.6, (0.58, 0.40, 0.28))):
        n = noise(cv, rng, sz)
        chip = fill(-(n - thr) * sz, cv.mmpp * 1.5)
        col = mix(col, cv.rgb(c), chip * 0.9)
    cloud = 0.04 * fbm(cv, rng, 200, 4)
    col = col * (1 + cloud[..., None])
    col = mix(col, cv.rgb((0.20, 0.19, 0.18)), grout)
    # brass inlay at z = 0 (every 2 m)
    inlay = fill(np.abs(cv.wrap(Z, 0.0)) - 6, cv.mmpp)
    col = mix(col, cv.rgb(PAL["brass"]), inlay)
    H = -grout * 1.5 + bevel(-(dgr - 2.5), 2.0) * 0 - inlay * 0.2
    H += 0.02 * noise(cv, rng, 1.0)
    # polish with scuffed traffic paths
    traffic = n01(fbm(cv, rng, 800, 3, aniso=(1.0, 0.4)), -1.0, 1.4)
    sc = scratches(cv, rng, 200, 120, 0.3)
    R = np.clip(0.22 + traffic * 0.18 + sc * 0.2 + grout * 0.6, 0.05, 1)
    M = inlay * 1.0
    R = mix(R, cv.full(0.3), inlay)
    AO = np.clip(1 - grout * 0.5, 0, 1)
    col = mix(col, col * 0.85, traffic[..., None].squeeze(-1) * 0.25)
    return dict(D=col, H=H, R=R, M=M, AO=AO, micro=None, micro_amp=0.0)


# --------------------------------------------------------------------------
@texset("StnWall", 2000, 2048, 1212)
def stn_wall(cv, rng):
    """Station civilian wall: warm off-white composite panels 1000 x 850 with
    rounded 10 mm reveals, amber wayfinding band (1000-1060), warm-grey
    kick base (0-150), perforated speaker/vent grille in alternate bays."""
    U, Z = cv.U, cv.Z
    lu = cv.rep(U, 1000.0, 500.0)
    bay = (U // 1000).astype(int)
    H = cv.zeros()
    col = cv.rgb(PAL["warm_panel"])
    R = cv.full(0.5)
    M = cv.zeros()
    # kick base 0-150, set back 4 mm, satin warm grey
    kick = fill(np.maximum(Z - 150, -Z), cv.mmpp)
    H -= kick * 4
    col = mix(col, cv.rgb((0.36, 0.33, 0.30)), kick)
    R = mix(R, cv.full(0.62), kick)
    # panel reveals
    lower = (Z >= 150) & (Z < 1000)
    pz = np.where(Z >= 1060, Z - 1530, Z - 575)
    phz = np.where(Z >= 1060, 470.0, 425.0)
    psd = sd_box(lu, pz, 495, phz - 5, 30)
    reveal = fill(np.abs(psd) - 5, cv.mmpp) * (1 - kick)
    H -= reveal * 6
    H += bevel(psd, 8) * 1.5 * (1 - kick)  # soft pillowed panel edge
    col = mix(col, cv.rgb((0.25, 0.23, 0.21)), reveal)
    # amber band
    band = fill(np.abs(Z - 1030) - 30, cv.mmpp)
    col = mix(col, cv.rgb(PAL["amber"]), band)
    R = mix(R, cv.full(0.38), band)
    H += band * 1.0
    # wayfinding: soft double chevrons in the band (mirror-symmetric)
    ch = fill(np.abs(cv.rep(np.abs(lu) * 1.0 + np.abs(Z - 1030) * 1.0, 60.0)) - 10, 1)
    ch = ch * fill(sd_box(lu, Z - 1030, 140, 20), 1) * (bay == 0)
    col = mix(col, cv.rgb((0.98, 0.93, 0.85)), ch * 0.85)
    # grille in bay 1, upper panel
    g_area = fill(sd_box(lu, Z - 1700, 260, 110, 30), cv.mmpp) * (bay == 1)
    holes = fill(sd_circle(cv.rep(U, 14.0), cv.rep(Z, 14.0), 3.2), cv.mmpp) * \
        fill(sd_box(lu, Z - 1700, 240, 90, 25), 1) * (bay == 1)
    H -= g_area * 2 + holes * 2
    col = mix(col, cv.rgb((0.62, 0.59, 0.55)), g_area)
    col = mix(col, cv.rgb((0.12, 0.11, 0.10)), holes)
    # warm light slot along the top of the band (emissive, reads as cove light)
    cove = fill(np.abs(Z - 1068) - 3, cv.mmpp)
    E = cv.rgb((1.0, 0.78, 0.5)) * cove[..., None] * 0.9
    col = mix(col, cv.rgb((1.0, 0.9, 0.75)), cove)

    AO = cavity_ao(cv, H, 10, 2, 0.6)
    tone = 0.02 * fbm(cv, rng, 500, 4)
    col = col * (1 + tone[..., None])
    scuff = n01(fbm(cv, rng, 50, 3, aniso=(5.0, 0.6)), 0.8, 2.4) * np.clip((400 - Z) / 250, 0, 1)
    col = mix(col, col * 0.72, scuff * 0.5)
    grime = np.clip((1 - AO) * 1.0, 0, 1)
    col = mix(col, col * 0.7, grime * 0.5)
    R = np.clip(R + 0.03 * noise(cv, rng, 30) + scuff * 0.1, 0.05, 1)
    return dict(D=col, H=H, R=R, M=M, AO=AO, E=E, micro=noise(cv, rng, 1.0), micro_amp=0.015)


# --------------------------------------------------------------------------
@texset("StnCeiling", 2000, 2048, 1313)
def stn_ceiling(cv, rng):
    """Station ceiling: 1 m coffers, each with a warm-white 560 mm diffuser."""
    U, Z = cv.U, cv.Z
    lu = cv.rep(U, 1000.0, 500.0)
    lz = cv.rep(Z, 1000.0, 500.0)
    H = cv.zeros()
    col = cv.rgb((0.78, 0.76, 0.72))
    R = cv.full(0.7)
    M = cv.zeros()
    beam_d = np.minimum(500 - np.abs(lu), 500 - np.abs(lz))
    beam = bevel(-(beam_d - 45), 6)
    H += beam * 40
    cof = sd_box(lu, lz, 440, 440, 60)
    H += bevel(cof, 80) * -30 + 30 * 0
    col = mix(col, cv.rgb((0.68, 0.66, 0.62)), 1 - beam)
    dsd = sd_box(lu, lz, 280, 280, 30)
    diff = fill(dsd, cv.mmpp)
    H += bevel(dsd, 5) * 8
    trim = fill(np.abs(dsd + 0) - 6, cv.mmpp)
    col = mix(col, cv.rgb((0.95, 0.92, 0.86)), diff)
    col = mix(col, cv.rgb(PAL["brass"]) * 0.85, trim)
    M = np.maximum(M, trim)
    R = mix(R, cv.full(0.35), np.maximum(diff, trim))
    glow = diff * (0.8 + 0.2 * np.clip(1 + dsd / 280, 0, 1) * 0 + 0.2 * (1 - np.clip(-dsd / 280, 0, 1)) * 0)
    E = cv.rgb((1.0, 0.9, 0.76)) * glow[..., None]
    AO = cavity_ao(cv, H, 40, 12, 0.6)
    tone = 0.02 * fbm(cv, rng, 500, 3)
    col = col * (1 + tone[..., None])
    return dict(D=col, H=H, R=R, M=M, AO=AO, E=E, micro=noise(cv, rng, 1.0), micro_amp=0.02)


# --------------------------------------------------------------------------
# Upper-wall companions: seamless in BOTH axes, for wall area above the
# floor-anchored 2 m band (station rooms are 3.5-7 m tall).
# --------------------------------------------------------------------------
def _upper_panels(cv, rng, paint, rough, reveal_col, rivets=True, metal_edges=True):
    U, Z = cv.U, cv.Z
    lu = cv.rep(U, 1000.0, 500.0)
    lz = cv.rep(Z, 1000.0, 500.0)
    H = cv.zeros()
    col = cv.rgb(paint)
    R = cv.full(rough)
    M = cv.zeros()
    dseam = np.minimum(500 - np.abs(lu), 500 - np.abs(lz))
    H += (bevel(-(dseam - 4.0), 3.0) - 1) * 5.0
    col = mix(col, cv.rgb(reveal_col), fill(dseam - 4.0, cv.mmpp))
    inset = sd_box(lu, lz, 430, 430, 40)
    H -= bevel(inset, 10.0) * 5.0
    rm = cv.zeros()
    if rivets:
        pts = []
        for t in np.arange(50.0, 2000.0, 100.0):
            for sline in (0.0, 1000.0):
                pts += [(sline + 16, t), (sline - 16, t), (t, sline + 16), (t, sline - 16)]
        rb, rm = bolt_field(cv, pts, 5.0, dome_mm=1.6)
        H += rb
    if metal_edges:
        cvx = convexity(cv, H, 5.0)
        chips = np.clip(np.clip(cvx / 1.2, 0, 1) * 1.5 * n01(fbm(cv, rng, 25, 3), 0.0, 1.6) - 0.15, 0, 1)
        col = mix(col, cv.rgb(PAL["bare"]), chips)
        M = np.maximum(M, chips)
        R = mix(R, cv.full(0.35), chips)
    col = mix(col, cv.rgb(PAL["steel"]), rm * 0.5)
    M = np.maximum(M, rm * 0.8)
    tone = 0.03 * fbm(cv, rng, 400, 4)
    col = col * (1 + tone[..., None])
    AO = cavity_ao(cv, H, 12.0, 2.5, 0.7)
    col = mix(col, col * 0.6, np.clip((1 - AO) * 1.3, 0, 1) * 0.6)
    return col, H, R, M, AO, lu, lz


@texset("ShipWallUpper", 2000, 2048, 1414)
def ship_wall_upper(cv, rng):
    col, H, R, M, AO, lu, lz = _upper_panels(cv, rng, PAL["paint_ship"], 0.55, (0.12, 0.13, 0.14))
    return dict(D=col, H=H, R=R, M=M, AO=AO, micro=noise(cv, rng, 0.8), micro_amp=0.02)


@texset("EngWallUpper", 2000, 2048, 1515)
def eng_wall_upper(cv, rng):
    """Dark stiffened plating + one vertical conduit pair; no pipes/hazard."""
    U, Z = cv.U, cv.Z
    col, H, R, M, AO, lu, lz = _upper_panels(cv, rng, (0.22, 0.225, 0.235), 0.58, (0.08, 0.08, 0.09))
    M = np.maximum(M, 0.85)
    stiff = bevel(np.abs(cv.rep(U, 500.0, 250.0)) - 20, 6)
    H += stiff * 10
    for uc in (1480.0, 1530.0):
        d = np.abs(cv.wrap(U, uc))
        inside = d < 18
        H = np.where(inside, np.maximum(H, 10 + 18 * np.sqrt(np.clip(1 - (d / 18) ** 2, 0, 1))), H)
        col = mix(col, cv.rgb((0.40, 0.41, 0.42)), inside.astype(np.float32))
        R = np.where(inside, 0.55, R)
        M = np.where(inside, 0.2, M)
    strap = fill(np.abs(cv.rep(Z, 400.0)) - 12, cv.mmpp) * fill(np.abs(cv.wrap(U, 1505)) - 50, 1)
    H = np.maximum(H, strap * 34)
    col = mix(col, cv.rgb((0.18, 0.18, 0.19)), strap)
    streak = n01(noise(cv, rng, 18, aniso=(0.6, 22.0)), 0.8, 2.6) * n01(fbm(cv, rng, 300, 2), -0.5, 1.2)
    col = mix(col, col * np.array([0.4, 0.37, 0.33], np.float32), streak * 0.6)
    AO = AO * cavity_ao(cv, H, 25, 8, 0.6)
    return dict(D=col, H=H, R=R, M=M, AO=AO, micro=noise(cv, rng, 0.8), micro_amp=0.03, nstrength=0.6)


@texset("StnWallUpper", 2000, 2048, 1616)
def stn_wall_upper(cv, rng):
    col, H, R, M, AO, lu, lz = _upper_panels(cv, rng, PAL["warm_panel"], 0.5, (0.25, 0.23, 0.21),
                                             rivets=False, metal_edges=False)
    return dict(D=col, H=H, R=R, M=M, AO=AO, micro=noise(cv, rng, 1.0), micro_amp=0.015)


# ==========================================================================
# Previews
# ==========================================================================
def lit_preview(maps, tiles=2, px=512):
    """Cheap in-texture-space shading (key light from upper-left) for review."""
    D = maps["D"]; N = maps["N"]; AO = maps["AO"]; E = maps["E"]
    n = N * 2 - 1
    nx, ny_dx, nz = n[..., 0], n[..., 1], n[..., 2]
    ny = -ny_dx  # DirectX -> up-positive
    L = np.array([-0.45, 0.55, 0.70]); L /= np.linalg.norm(L)
    lam = np.clip(nx * L[0] + ny * L[1] + nz * L[2], 0, 1)
    shade = D * (0.28 + 0.9 * lam[..., None]) * (AO[..., None] ** 0.8) + E * 1.2
    img = np.clip(shade, 0, 1)
    img = np.tile(img, (tiles, tiles, 1))
    im = Image.fromarray((img * 255).astype(np.uint8)).resize((px, px), Image.LANCZOS)
    return im


def contact_sheet(name, maps, info):
    th = 256
    keys = ["D", "N", "R", "M", "AO", "E"] + (["MASK"] if "MASK" in maps else [])
    W = th * len(keys) + 512
    sheet = Image.new("RGB", (W, 512 + 24), (20, 20, 22))
    dr = ImageDraw.Draw(sheet)
    sheet.paste(lit_preview(maps, 2, 512), (0, 24))
    dr.text((4, 4), f"T_Int_{name}  {info}  (lit, 2x2 tiled)", fill=(230, 230, 230))
    for i, k in enumerate(keys):
        a = np.clip(maps[k], 0, 1)
        a8 = (a * 255).astype(np.uint8)
        im = Image.fromarray(a8, "RGB" if a8.ndim == 3 else "L").convert("RGB").resize((th, th), Image.LANCZOS)
        x = 512 + i * th
        sheet.paste(im, (x, 24 + (i % 2) * 0))
        dr.text((x + 4, 4), k, fill=(230, 230, 230))
    # second row: 1:1 crop of D to judge human-scale detail
    crop = Image.fromarray((np.clip(maps["D"], 0, 1) * 255).astype(np.uint8))
    S = crop.size[0]
    cc = crop.crop((0, 0, min(S, th * len(keys)), 256))
    sheet.paste(cc, (512, 24 + th))
    dr.text((516, 24 + th + 4), "1:1 crop of D", fill=(255, 255, 0))
    p = os.path.join(PREV, f"contact_{name}.png")
    sheet.save(p)
    return p


def main(argv):
    half = "--half" in argv
    want = [a for a in argv if not a.startswith("--")]
    os.makedirs(PREV, exist_ok=True)
    thumbs = []
    for name, spec in SETS.items():
        if want and name not in want:
            continue
        size = spec["size"] // (2 if half else 1)
        cv = Canvas(size, spec["tile_mm"])
        rng = np.random.default_rng(spec["seed"])
        out = spec["fn"](cv, rng)
        _, maps = write_set(name, cv, out["D"], out["H"], out["R"], out["M"], out["AO"],
                            out.get("E"), out.get("extra"), out.get("nstrength", 1.0),
                            out.get("micro"), out.get("micro_amp", 0.0))
        info = f"{size}px / {spec['tile_mm']/1000:.0f} m tile ({size/spec['tile_mm']:.2f} px/mm)"
        p = contact_sheet(name, maps, info)
        thumbs.append((name, lit_preview(maps, 2, 384)))
        print(f"ok  T_Int_{name}  {info}  -> {p}", flush=True)
    if thumbs and not want:
        cols = 5
        rows = (len(thumbs) + cols - 1) // cols
        sheet = Image.new("RGB", (cols * 384, rows * 404), (20, 20, 22))
        dr = ImageDraw.Draw(sheet)
        for i, (n, im) in enumerate(thumbs):
            x, y = (i % cols) * 384, (i // cols) * 404
            sheet.paste(im, (x, y + 20))
            dr.text((x + 4, y + 4), f"T_Int_{n}", fill=(230, 230, 230))
        sheet.save(os.path.join(PREV, "contact_all.png"))
        print("contact_all ->", os.path.join(PREV, "contact_all.png"))


if __name__ == "__main__":
    main(sys.argv[1:])
