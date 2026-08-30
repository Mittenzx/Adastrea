"""STUDIO SCENE for rendering ships/stations with lighting — deterministic
software rasterizer (numpy/PIL), ported onto the proven render_pbr.py per-part
core.

Why this works where the old version failed:
- Old version loaded ONE merged `_Assembled` OBJ -> interleaved geometry lost
  per-part UVs and mapped everything to a single texture, and its triangle loop
  used a VERTEX index as a UV index (wrong -> garbled/dark faces).
- This loads each PART OBJ separately (correct per-part UVs), maps the right
  texture set per part, mounts parts at hardpoints, and only then flattens into
  one vertex array with SEPARATE vertex + uv index offsets (render_pbr pattern).

It also builds a proper studio scene: gradient backdrop + floor plane + 3-point
Lambert lighting (key/fill/rim), and self-lits emissive regions.

Usage: python Tools/render_studio.py <ship_or_station_base> [more...]
  e.g. python Tools/render_studio.py SM_Ship_Fighter_01 SM_Station_Habitation_01
Output: Assets/FBX/generated/scene_renders/<base>.png
"""
import os, sys, math
import numpy as np
from PIL import Image

OBJDIR = r"C:\Users\akuma\Adastrea\Assets\FBX\generated\obj"
TEXDIR = r"C:\Users\akuma\Adastrea\Assets\FBX\generated\Textures"
OUTDIR = r"C:\Users\akuma\Adastrea\Assets\FBX\generated\scene_renders"
os.makedirs(OUTDIR, exist_ok=True)

# per-part type -> texture set name (matches generator's texture naming)
PART_TEX = {
    "Carcass": "T_Ship_Hull", "Engine": "T_Ship_Hull", "Cargo": "T_Ship_Hull",
    "Weapon": "T_Ship_Hull", "Sensor": "T_Ship_Hull", "Reactor": "T_Ship_Hull",
    "MiningLaser": "T_Ship_Hull", "Drill": "T_Station_Hab",
    "HabRing": "T_HabRing", "AsteroidShell": "T_AsteroidShell", "Hab": "T_Station_Hab",
}
# whole-ship override reuse: these ship bases have distinct hull textures
SHIP_TEX = {
    "SM_Ship_Freighter_01": "T_Freighter",
    "SM_Ship_Gunship_02": "T_Gunship",
}

# size class -> (lx, ly, lz, locz)  (mirrors SIZE_CLASSES in generator)
SCLS = {
    "small": (250, 340, 95, 55), "medium": (380, 720, 190, 130),
    "corvette": (460, 880, 220, 150), "large": (520, 980, 250, 175),
}
K = {"small": 1.0, "medium": 1.65, "corvette": 2.0, "large": 2.3}
# part type -> (y_frac_of_ly, z_offset_units) mount (mirrors assemble_ship);
# x=0 for all standard mounts
MOUNT_FRAC = {
    "Carcass": (0.0, 0.0), "Engine": (-0.52, -10), "Cargo": (-0.05, -20),
    "Weapon": (0.34, -25), "Sensor": (0.18, 70), "Reactor": (-0.35, 40),
    "MiningLaser": (0.30, -10), "Drill": (-0.30, -15),
    "HabRing": (0.0, 0.0), "AsteroidShell": (0.0, 0.0), "Hab": (0.0, 0.0),
}
PARTS_ORDER = ["Carcass", "HabRing", "AsteroidShell", "Hab",
               "Engine", "Cargo", "Weapon", "Sensor", "Reactor",
               "MiningLaser", "Drill"]


def size_class_of(base):
    b = base.lower()
    if 'corvette' in b or 'miner' in b:
        return 'corvette'
    if 'freighter' in b or 'bulkhauler' in b:
        return 'medium'
    if 'station' in b or 'hab' in b or 'generation' in b:
        return 'large'
    return 'small'  # fighter/gunship


def load_obj(path):
    verts, uvs, faces = [], [], []
    with open(path) as f:
        for line in f:
            p = line.split()
            if not p: continue
            if p[0] == 'v': verts.append((float(p[1]), float(p[2]), float(p[3])))
            elif p[0] == 'vt': uvs.append((float(p[1]), float(p[2])))
            elif p[0] == 'f':
                fidx = [t.split('/') for t in p[1:]]
                tri = []
                for vi, uv, _ in fidx:
                    tri.append((int(vi) - 1, int(uv) - 1 if uv else -1))
                for k in range(1, len(tri) - 1):
                    faces.append((tri[0], tri[k], tri[k + 1]))
    faces = np.array(faces, dtype=int)  # (F,3,2): [corner][v,uv]
    return (np.array(verts, float), np.array(uvs, float), faces)


def load_tex(name):
    d = os.path.join(TEXDIR, name + "_D.png")
    e = os.path.join(TEXDIR, name + "_E.png")
    D = np.asarray(Image.open(d).convert('RGB'), float) / 255.0
    E = np.asarray(Image.open(e).convert('RGB'), float) / 255.0 if os.path.exists(e) else np.zeros_like(D)
    return D, E


def mount_for(base, sz):
    """Return dict {part_type: (x,y,z) mount offset} in world units."""
    lx, ly, lz, locz = SCLS[sz]
    k = K[sz]
    m = {}
    for pt, (yf, zoff) in MOUNT_FRAC.items():
        if pt == "Carcass":
            m[pt] = (0, 0, 0)
        else:
            m[pt] = (0, yf * ly, locz + zoff * k)
    return m


def render_one(base, W=1400, H=1000):
    sz = size_class_of(base)
    mount = mount_for(base, sz)

    # gather per-part geometry with SEPARATE vertex + uv index offsets, per-part tex
    allV, allUV, allFv, allFuv = [], [], [], []
    texset = []      # list of (D, E)
    ftex = []        # per-face texture-set index
    vbase = 0; ubase = 0
    for pt in PARTS_ORDER:
        p = os.path.join(OBJDIR, f"{base}_{pt}.obj")
        if not os.path.exists(p): continue
        V, U, F = load_obj(p)
        # texture set: for Carcass use the ship's hull texture (or per-ship override);
        # for part types use their own tex or fall back to the hull tex.
        if pt == "Carcass":
            tname = SHIP_TEX.get(base, "T_Ship_Hull")
        else:
            tname = PART_TEX.get(pt, "T_Ship_Hull")
        D, E = load_tex(tname)
        ts_idx = len(texset)
        texset.append((D, E))
        # mount offset (x y z)
        off = mount.get(pt, (0, 0, 0))
        V = V + np.array(off)
        allV.append(V)
        allUV.append(U)
        fv = F[:, :, 0] + vbase
        fu = F[:, :, 1] + ubase
        valid_uv = F[:, :, 1] >= 0
        fu = np.where(valid_uv, fu, -1)
        allFv.append(fv)
        allFuv.append(fu)
        ftex.extend([ts_idx] * len(F))
        vbase += len(V)
        ubase += len(U)

    V = np.vstack(allV)
    UV = np.vstack(allUV)
    allFv = np.vstack(allFv)
    allFuv = np.vstack(allFuv)
    ftex = np.array(ftex)
    if len(V) == 0:
        print("NO GEOMETRY", base); return

    # ---- camera framing (turntable 3/4 view around origin) ----
    center = V.mean(0)
    span = (V.max(0) - V.min(0)).max()
    radius = max(span * 1.25, 80)
    az, el = math.radians(42), math.radians(20)
    cam = np.array([center[0] + radius*math.cos(el)*math.sin(az),
                    center[1] + radius*math.cos(el)*math.cos(az),
                    center[2] + radius*math.sin(el)])
    fwd = (center - cam); fwd /= np.linalg.norm(fwd)
    right = np.cross(fwd, np.array([0, 0, 1])); right /= np.linalg.norm(right)
    up = np.cross(right, fwd)
    Rm = np.stack([right, up, -fwd])
    Vc = (V - cam) @ Rm.T
    fov = 42; focal = (H / 2) / math.tan(math.radians(fov / 2))
    zc = -Vc[:, 2]; zc = np.where(zc < 0.1, 0.1, zc)
    xs = Vc[:, 0]*focal/zc + W/2
    ys = -Vc[:, 1]*focal/zc + H/2
    v2d = np.stack([xs, ys], 1)

    # ---- 3-point lighting ----
    L_key = np.array([0.5, -0.55, 0.7]); L_key /= np.linalg.norm(L_key)
    L_fill = np.array([-0.6, 0.3, 0.45]); L_fill /= np.linalg.norm(L_fill)
    L_rim = np.array([-0.35, -0.6, -0.6]); L_rim /= np.linalg.norm(L_rim)
    kk, kf, kr, amb = 1.05, 0.45, 0.35, 0.55

    fb = np.full((H, W, 3), 0.0)
    zb = np.full((H, W), 1e9)

    # ---- backdrop gradient (darker top -> lighter horizon) ----
    for yy in range(H):
        t = yy / H
        g = 0.05 + 0.16 * (1 - t)
        fb[yy, :, :] = (g, g * 1.05, g * 1.1)

    # ---- floor plane (large quad below ship, soft gradient) ----
    fy = V[:, 2].min() - (V[:, 2].max() - V[:, 2].min()) * 0.12
    sp = span * 2.4
    fc = np.array([(center[0]-sp/2, center[1]-sp/2, fy), (center[0]+sp/2, center[1]-sp/2, fy),
                   (center[0]+sp/2, center[1]+sp/2, fy), (center[0]-sp/2, center[1]+sp/2, fy)])
    proj = []
    for c in fc:
        vc = (c - cam) @ Rm.T
        zz = max(-vc[2], 0.1)
        proj.append(np.array([vc[0]*focal/zz + W/2, -vc[1]*focal/zz + H/2, -vc[2]]))
    for i0, i1, i2 in [(0, 1, 2), (0, 2, 3)]:
        p0, p1, p2 = proj[i0][:2], proj[i1][:2], proj[i2][:2]
        e1 = p1-p0; e2 = p2-p0
        area = e1[0]*e2[1] - e1[1]*e2[0]
        if abs(area) < 1e-6: continue
        x0 = max(0, int(min(p0[0], p1[0], p2[0]))); x1 = min(W-1, int(max(p0[0], p1[0], p2[0])))
        y0 = max(0, int(min(p0[1], p1[1], p2[1]))); y1 = min(H-1, int(max(p0[1], p1[1], p2[1])))
        for yy in range(y0, y1+1):
            for xx in range(x0, x1+1):
                a = ((p1[1]-p2[1])*(xx-p2[0]) + (p2[0]-p1[0])*(yy-p2[1])) / area
                b = ((p2[1]-p0[1])*(xx-p2[0]) + (p0[0]-p2[0])*(yy-p2[1])) / area
                c = 1-a-b
                if a < 0 or b < 0 or c < 0: continue
                zv = a*(-proj[i0][2])+b*(-proj[i1][2])+c*(-proj[i2][2])
                if zv > zb[yy, xx]: continue
                zb[yy, xx] = zv
                grad = 0.75 + 0.25*(0.5-c)
                fb[yy, xx] = np.array([0.16, 0.17, 0.19]) * grad

    # ---- raster ships (per-face texture sampled with correct UV indices) ----
    def tri_tex(face, fuvf, D, E):
        i0, i1, i2 = face
        u0i, u1i, u2i = fuvf
        p0, p1, p2 = v2d[i0], v2d[i1], v2d[i2]
        e1 = p1-p0; e2 = p2-p0
        area = e1[0]*e2[1] - e1[1]*e2[0]
        if area >= 0: return  # backface cull
        w0, w1, w2 = V[i0], V[i1], V[i2]
        n = np.cross(w1-w0, w2-w0); nl = np.linalg.norm(n)
        if nl < 1e-9: return
        n = n/nl
        if np.dot(n, (cam-w0)) < 0: n = -n
        lam = amb + kk*max(np.dot(n, L_key), 0) + kf*max(np.dot(n, L_fill), 0) + kr*max(np.dot(n, L_rim), 0)
        lam = min(lam, 1.35)
        got_uv = (u0i >= 0 and u1i >= 0 and u2i >= 0)
        if got_uv:
            u0, u1, u2 = UV[u0i], UV[u1i], UV[u2i]
        th, tw = D.shape[0], D.shape[1]
        x0 = max(0, int(min(p0[0], p1[0], p2[0]))); x1 = min(W-1, int(max(p0[0], p1[0], p2[0])))
        y0 = max(0, int(min(p0[1], p1[1], p2[1]))); y1 = min(H-1, int(max(p0[1], p1[1], p2[1])))
        for yy in range(y0, y1+1):
            for xx in range(x0, x1+1):
                a = ((p1[1]-p2[1])*(xx-p2[0]) + (p2[0]-p1[0])*(yy-p2[1])) / area
                b = ((p2[1]-p0[1])*(xx-p2[0]) + (p0[0]-p2[0])*(yy-p2[1])) / area
                c = 1-a-b
                if a < 0 or b < 0 or c < 0: continue
                zview = a*(-Vc[i0,2])+b*(-Vc[i1,2])+c*(-Vc[i2,2])
                if zview > zb[yy, xx]: continue
                zb[yy, xx] = zview
                if got_uv:
                    uu = u0[0]*a + u1[0]*b + u2[0]*c
                    vv = u0[1]*a + u1[1]*b + u2[1]*c
                    tx = int((uu % 1.0)*tw) % tw; ty = int((vv % 1.0)*th) % th
                    col = D[ty, tx]
                    em = E[ty, tx]
                    if em.max() > 0.25:
                        col = col*lam*0.6 + em*np.array([1.6, 1.6, 1.6])
                    else:
                        col = col*lam
                else:
                    col = np.array([0.6, 0.62, 0.66]) * lam
                fb[yy, xx] = np.clip(col, 0, 1)

    for fi in range(len(allFv)):
        ti = ftex[fi]
        D, E = texset[ti]
        tri_tex(allFv[fi], allFuv[fi], D, E)

    img = Image.fromarray((fb * 255).astype(np.uint8))
    out = os.path.join(OUTDIR, f"{base}.png")
    img.save(out)
    print("OK", base, "tris", len(allFv), "->", os.path.basename(out))


if __name__ == "__main__":
    args = sys.argv[1:] or ["SM_Ship_Fighter_01"]
    for name in args:
        try:
            render_one(name)
        except Exception as e:
            import traceback; traceback.print_exc()
            print("ERROR", name)