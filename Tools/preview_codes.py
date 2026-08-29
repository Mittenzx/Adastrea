"""Software renderer for the MODULE-BUILDER PREVIEW — deterministic, no render
engine quirks. Loads each chosen module's part OBJ, color-fills its triangles
with a solid accent color (engine blue/cargo green/weapon red/sensor amber/
reactor magenta, hull grey) + diffuse shading, composited by depth.

Output: Assets/FBX/generated/previews/<outname>_codes_preview.png

Usage: python Tools/preview_codes.py <outname>
(e.g. SM_Ship_Test_PV — expects <outname>_<Part>.obj in Assets/FBX/generated/obj/)
"""
import os, sys, math
import numpy as np
from PIL import Image

OBJDIR = r"C:\Users\akuma\Adastrea\Assets\FBX\generated\obj"
OUTDIR = r"C:\Users\akuma\Adastrea\Assets\FBX\generated\previews"

# module type -> accent RGB (unlit base)
ACCENTS = {
    "Engine":  (0.10, 0.55, 0.95),
    "Cargo":   (0.20, 0.75, 0.35),
    "Weapon":  (0.90, 0.25, 0.15),
    "Sensor":  (0.95, 0.60, 0.10),
    "Reactor": (0.75, 0.15, 0.65),
    "MiningLaser": (0.90, 0.25, 0.15),
    "Drill":   (0.55, 0.55, 0.20),
    "Carcass": (0.30, 0.31, 0.34),   # hull neutral grey
    "Hull":    (0.30, 0.31, 0.34),
    "Nose":    (0.30, 0.31, 0.34),
    "Spine":   (0.30, 0.31, 0.34),
}


def load_obj(path):
    verts, faces = [], []
    with open(path) as f:
        for line in f:
            p = line.split()
            if not p: continue
            if p[0] == 'v':
                verts.append((float(p[1]), float(p[2]), float(p[3])))
            elif p[0] == 'f':
                idx = [int(t.split('/')[0]) - 1 for t in p[1:]]
                for k in range(1, len(idx) - 1):
                    faces.append((idx[0], idx[k], idx[k + 1]))
    return np.array(verts, float), np.array(faces, int)


def render(parts, outname, W=1200, H=820, sz="small"):
    # mount offsets (mirror assemble_ship), relative to size class
    scls = {"small": (250, 340, 95, 55),
            "medium": (380, 720, 190, 130),
            "corvette": (460, 880, 220, 150),
            "large": (520, 980, 250, 175)}
    lx, ly, lz, locz = scls.get(sz, scls["small"])
    k = 1.0 if sz == "small" else {"medium":1.65,"corvette":2.0,"large":2.3}[sz]
    MOUNT = {
        "Engine":  (0, -ly*0.52, locz - 10*k),
        "Cargo":   (0, -ly*0.05, locz - 20*k),
        "Weapon":  (0,  ly*0.34, locz - 25*k),
        "Sensor":  (0,  ly*0.18, locz + 70*k),
        "Reactor": (0, -ly*0.35, locz + 40*k),
        "MiningLaser": (0, ly*0.3, locz - 10*k),
        "Drill":   (0, -ly*0.3, locz - 15*k),
        "Carcass": (0,0,0), "Hull":(0,0,0), "Nose":(0,0,0), "Spine":(0,0,0),
    }
    allV = []
    allF = []          # (F,3) global vertex indices
    partcolor = []     # per-face color (F,3)
    base = 0
    for mtype, path in parts:
        p = os.path.join(OBJDIR, path)
        if not os.path.exists(p):
            print("  skip (no obj):", path); continue
        V, F = load_obj(p)
        # apply mount offset so module sits at its hardpoint position
        V = V + np.array(MOUNT.get(mtype, (0,0,0)))
        allV.append(V)
        allF.append(F + base)
        col = ACCENTS.get(mtype, (0.5, 0.5, 0.5))
        partcolor.append(np.tile(np.array(col), (len(F), 1)))
        base += len(V)
    if not allV:
        print("no geometry"); return
    V = np.vstack(allV)
    allF = np.vstack(allF)
    CO = np.vstack(partcolor)

    center = V.mean(0)
    radius = max((V.max(0) - V.min(0)).max() * 1.4, 60)
    az, el = math.radians(50), math.radians(26)
    cam = np.array([center[0] + radius*math.cos(el)*math.sin(az),
                    center[1] + radius*math.cos(el)*math.cos(az),
                    center[2] + radius*math.sin(el)])
    fwd = center - cam; fwd /= np.linalg.norm(fwd)
    right = np.cross(fwd, np.array([0, 0, 1])); right /= np.linalg.norm(right)
    up = np.cross(right, fwd)
    Rm = np.stack([right, up, -fwd])
    Vc = (V - cam) @ Rm.T
    fov = 45; focal = (H / 2) / math.tan(math.radians(fov / 2))
    zc = -Vc[:, 2]; zc = np.where(zc < 0.1, 0.1, zc)
    xs = Vc[:, 0]*focal/zc + W/2
    ys = -Vc[:, 1]*focal/zc + H/2
    verts2d = np.stack([xs, ys], 1)

    L = np.array([0.4, -0.6, 0.8]); L /= np.linalg.norm(L)
    amb, dif = 0.45, 0.6

    fb = np.ones((H, W, 3), float) * 0.03
    zb = np.full((H, W), 1e9)

    for fi in range(len(allF)):
        i0, i1, i2 = allF[fi]
        p0, p1, p2 = verts2d[i0], verts2d[i1], verts2d[i2]
        e1 = p1 - p0; e2 = p2 - p0
        area = e1[0]*e2[1] - e1[1]*e2[0]
        if area >= 0: continue   # backface
        w0, w1, w2 = V[i0], V[i1], V[i2]
        n = np.cross(w1 - w0, w2 - w0); nl = np.linalg.norm(n)
        if nl < 1e-9: continue
        n = n / nl
        if np.dot(n, (cam - w0)) < 0: n = -n
        lam = amb + dif * max(np.dot(n, L), 0)
        col = CO[fi]
        x0 = max(0, int(min(p0[0], p1[0], p2[0]))); x1 = min(W-1, int(max(p0[0], p1[0], p2[0])))
        y0 = max(0, int(min(p0[1], p1[1], p2[1]))); y1 = min(H-1, int(max(p0[1], p1[1], p2[1])))
        for yy in range(y0, y1 + 1):
            for xx in range(x0, x1 + 1):
                a = ((p1[1]-p2[1])*(xx-p2[0]) + (p2[0]-p1[0])*(yy-p2[1])) / area
                b = ((p2[1]-p0[1])*(xx-p2[0]) + (p0[0]-p2[0])*(yy-p2[1])) / area
                c = 1 - a - b
                if a < 0 or b < 0 or c < 0: continue
                zview = a*(-Vc[i0, 2]) + b*(-Vc[i1, 2]) + c*(-Vc[i2, 2])
                if zview > zb[yy, xx]: continue
                zb[yy, xx] = zview
                fb[yy, xx] = np.clip(col * lam, 0, 1)
    img = Image.fromarray((fb * 255).astype(np.uint8))
    out = os.path.join(OUTDIR, f"{outname}_codes_preview.png")
    img.save(out)
    print("rendered", out, "tris", len(allF))


if __name__ == "__main__":
    outname = sys.argv[1] if len(sys.argv) > 1 else "SM_Ship_Test_PV"
    sz = sys.argv[2] if len(sys.argv) > 2 else "small"
    # build the part list from OBJ dir
    parts = []
    # order: carcass-ish first, then modules (each distinct color)
    for mtype in ["Carcass", "Engine", "Cargo", "Weapon", "Sensor", "Reactor",
                  "MiningLaser", "Drill"]:
        p = f"{outname}_{mtype}.obj"
        if os.path.exists(os.path.join(OBJDIR, p)):
            parts.append((mtype, p))
    # if no carcass named, add the assembled OBJ as grey hull base (modules
    # mounted on top will overwrite it via depth)
    if not any(m == "Carcass" for m, _ in parts):
        ap = f"{outname}_Assembled.obj"
        if os.path.exists(os.path.join(OBJDIR, ap)):
            parts.insert(0, ("Carcass", ap))
    # if still no hull, add raw hull/nose/spine if present
    if not parts:
        for mtype in ["Hull", "Nose", "Spine"]:
            p = f"{outname}_{mtype}.obj"
            if os.path.exists(os.path.join(OBJDIR, p)):
                parts.append((mtype, p))
    print("parts:", [m for m, _ in parts])
    render(parts, outname, sz=sz)