"""Module-picker preview sheet — render each module variant as a standalone
color-coded thumbnail and tile into a grid sheet. Uses the deterministic
software rasterizer (color-fill per module type). The in-game builder can show
these to the player as "what each module looks like".

Usage: python Tools/module_picker.py
Output: Assets/FBX/generated/previews/module_picker.png

The variant part OBJs come from the preview_build variant ships:
  SM_Ship_Fighter_02_Arcangel_*        (ion / tri_laser / fusion / dome)
  SM_Ship_Freighter_02_Bulkhauler_*    (compact / bulk_tank / spike / cross)
"""
import os, math, sys
import numpy as np
from PIL import Image

OBJDIR = r"C:\Users\akuma\Adastrea\Assets\FBX\generated\obj"
OUTDIR = r"C:\Users\akuma\Adastrea\Assets\FBX\generated\previews"

ACCENTS = {
    "Engine":  (0.10, 0.55, 0.95),   # blue
    "Cargo":   (0.20, 0.75, 0.35),   # green
    "Weapon":  (0.90, 0.25, 0.15),   # red
    "Sensor":  (0.95, 0.60, 0.10),   # amber
    "Reactor": (0.75, 0.15, 0.65),   # magenta
}

# Which OBJs to show per module type (variant ships carry the distinct variants)
PICKER = [
    # (label, module_type, obj_path)
    ("Engine - Ion",      "Engine",  "SM_Ship_Fighter_02_Arcangel_Engine.obj"),
    ("Engine - Compact",  "Engine",  "SM_Ship_Freighter_02_Bulkhauler_Engine.obj"),
    ("Weapon - Tri-Laser","Weapon",  "SM_Ship_Fighter_02_Arcangel_Weapon.obj"),
    ("Reactor - Fusion",  "Reactor", "SM_Ship_Fighter_02_Arcangel_Reactor.obj"),
    ("Sensor - Dome",     "Sensor",  "SM_Ship_Fighter_02_Arcangel_Sensor.obj"),
    ("Sensor - Cross",    "Sensor",  "SM_Ship_Freighter_02_Bulkhauler_Sensor.obj"),
    ("Reactor - Spike",   "Reactor", "SM_Ship_Freighter_02_Bulkhauler_Reactor.obj"),
    ("Cargo - Bulk Tank", "Cargo",   "SM_Ship_Freighter_02_Bulkhauler_Cargo.obj"),
]


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


def render_one(obj_path, accent, W=300, H=300):
    V, F = load_obj(os.path.join(OBJDIR, obj_path))
    if len(F) == 0:
        return None
    # fit to a unit box for a clean centered thumbnail
    center = (V.max(0) + V.min(0)) / 2
    s = (V.max(0) - V.min(0)).max()
    Vn = (V - center) / max(s, 1e-6) * 1.6   # normalize to ~ -0.8..0.8

    # camera
    az, el = math.radians(40), math.radians(24)
    radius = 2.6
    cam = np.array([radius*math.cos(el)*math.sin(az),
                    radius*math.cos(el)*math.cos(az),
                    radius*math.sin(el)])
    fwd = -cam; fwd /= np.linalg.norm(fwd)
    right = np.cross(fwd, np.array([0,0,1])); right /= np.linalg.norm(right)
    up = np.cross(right, fwd)
    Rm = np.stack([right, up, -fwd])
    Vc = (Vn - cam) @ Rm.T
    focal = (H/2) / math.tan(math.radians(23))
    zc = -Vc[:,2]; zc = np.where(zc<0.1, 0.1, zc)
    xs = Vc[:,0]*focal/zc + W/2
    ys = -Vc[:,1]*focal/zc + H/2
    v2d = np.stack([xs, ys], 1)

    L = np.array([0.4, -0.6, 0.8]); L /= np.linalg.norm(L)
    amb, dif = 0.45, 0.6
    fb = np.ones((H, W, 3), float) * 0.04
    zb = np.full((H, W), 1e9)

    for fi in range(len(F)):
        i0, i1, i2 = F[fi]
        p0, p1, p2 = v2d[i0], v2d[i1], v2d[i2]
        e1 = p1-p0; e2 = p2-p0
        area = e1[0]*e2[1]-e1[1]*e2[0]
        if area >= 0: continue
        w0, w1, w2 = Vn[i0], Vn[i1], Vn[i2]
        n = np.cross(w1-w0, w2-w0); nl = np.linalg.norm(n)
        if nl < 1e-9: continue
        n = n/nl
        if np.dot(n, (cam-w0)) < 0: n = -n
        lam = amb + dif*max(np.dot(n, L), 0)
        x0 = max(0, int(min(p0[0],p1[0],p2[0]))); x1 = min(W-1, int(max(p0[0],p1[0],p2[0])))
        y0 = max(0, int(min(p0[1],p1[1],p2[1]))); y1 = min(H-1, int(max(p0[1],p1[1],p2[1])))
        for yy in range(y0, y1+1):
            for xx in range(x0, x1+1):
                a = ((p1[1]-p2[1])*(xx-p2[0]) + (p2[0]-p1[0])*(yy-p2[1])) / area
                b = ((p2[1]-p0[1])*(xx-p2[0]) + (p0[0]-p2[0])*(yy-p2[1])) / area
                c = 1-a-b
                if a<0 or b<0 or c<0: continue
                zview = a*(-Vc[i0,2])+b*(-Vc[i1,2])+c*(-Vc[i2,2])
                if zview > zb[yy,xx]: continue
                zb[yy,xx] = zview
                fb[yy,xx] = np.clip(np.array(accent)*lam, 0, 1)
    return Image.fromarray((fb*255).astype(np.uint8))


def build_sheet():
    cols = 4
    thumbs = []
    labels = []
    for label, mtype, obj in PICKER:
        accent = ACCENTS.get(mtype, (0.5,0.5,0.5))
        im = render_one(obj, accent, 280, 280)
        if im is None:
            print("  skip", label); continue
        thumbs.append(im)
        labels.append(label)
    if not thumbs:
        print("no thumbnails"); return
    rows = int(np.ceil(len(thumbs)/cols))
    tw, th = 280, 280
    pad = 12
    cellw, cellh = tw + pad*2, th + 40 + pad
    sheet = Image.new("RGB", (cols*cellw, rows*cellh), (22, 24, 28))
    from PIL import ImageDraw
    dr = ImageDraw.Draw(sheet)
    for i, (im, label) in enumerate(zip(thumbs, labels)):
        r, c = divmod(i, cols)
        x = c*cellw + pad
        y = r*cellh + pad
        sheet.paste(im, (x, y))
        dr.text((x, y+th+8), label, fill=(230,230,230))
    out = os.path.join(OUTDIR, "module_picker.png")
    sheet.save(out)
    print("saved", out, sheet.size, len(thumbs), "modules")


if __name__ == "__main__":
    build_sheet()