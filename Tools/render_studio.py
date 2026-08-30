"""STUDIO SCENE for rendering ships with lighting — deterministic software
rasterizer (numpy/PIL). The Cycles path keeps failing on the multi-object
assembled FBX (normals/material/framing), so this reuses the proven software
fill that made the interiors + module picker render cleanly.

It builds a proper lit scene: gradient studio backdrop + floor plane + 3-point
Lambert lighting (key/fill/rim), and texture-maps each part with its PBR base +
emissive into the diffuse shading.

Usage: python Tools/render_studio.py <ship_outname> [more...]
  e.g. python Tools/render_studio.py SM_Ship_Fighter_01_Assembled SM_Ship_Corvette_01_Assembled
Output: Assets/FBX/generated/scene_renders/<name>.png
"""
import os, sys, math
import numpy as np
from PIL import Image

OBJDIR = r"C:\Users\akuma\Adastrea\Assets\FBX\generated\obj"
TEXDIR = r"C:\Users\akuma\Adastrea\Assets\FBX\generated\Textures"
OUTDIR = r"C:\Users\akuma\Adastrea\Assets\FBX\generated\scene_renders"
os.makedirs(OUTDIR, exist_ok=True)

# which texture set per part type
PART_TEX = {
    "Carcass": "T_Ship_Hull", "Engine": "T_Ship_Hull", "Cargo": "T_Ship_Hull",
    "Weapon": "T_Ship_Hull", "Sensor": "T_Ship_Hull", "Reactor": "T_Ship_Hull",
    "Assembled": "T_Ship_Hull",
}


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
                    tri.append((int(vi)-1, int(uv)-1 if uv else -1))
                for k in range(1, len(tri)-1):
                    faces.append((tri[0], tri[k], tri[k+1]))
    faces = np.array(faces, dtype=int)
    return (np.array(verts, float), np.array(uvs, float), faces)


def load_tex(name):
    d = os.path.join(TEXDIR, name + "_D.png")
    e = os.path.join(TEXDIR, name + "_E.png")
    D = np.asarray(Image.open(d).convert('RGB'), float) / 255.0
    E = np.asarray(Image.open(e).convert('RGB'), float) / 255.0 if os.path.exists(e) else np.zeros_like(D)
    return D, E


def render_one(objname, W=1400, H=1000):
    # load the assembled OBJ (single mesh = whole ship already joined)
    p = os.path.join(OBJDIR, objname + ".obj")
    if not os.path.exists(p):
        print("NO OBJ", objname); return
    V, UV, F = load_obj(p)
    if len(F) == 0:
        print("NO FACES", objname); return
    tname = "T_Ship_Hull"
    D, E = load_tex(tname)

    # ground floor plane (a large quad below the ship)
    ymin = V[:, 2].min()
    span = (V.max(0) - V.min(0)).max() * 3.5
    fy = ymin - (V[:, 2].max() - V[:, 2].min()) * 0.15
    floor_col = np.array([0.14, 0.15, 0.17])
    # We'll raster the floor separately before the ship.

    center = V.mean(0)
    radius = max((V.max(0) - V.min(0)).max() * 1.2, 60)   # proven framing value

    # camera: 3/4 turntable view
    az, el = math.radians(40), math.radians(20)
    cam = np.array([center[0] + radius*math.cos(el)*math.sin(az),
                    center[1] + radius*math.cos(el)*math.cos(az),
                    center[2] + radius*math.sin(el)])
    fwd = (center - cam); fwd /= np.linalg.norm(fwd)
    right = np.cross(fwd, np.array([0, 0, 1])); right /= np.linalg.norm(right)
    up = np.cross(right, fwd)
    Rm = np.stack([right, up, -fwd])
    Vc = (V - cam) @ Rm.T
    fov = 40; focal = (H/2) / math.tan(math.radians(fov/2))
    zc = -Vc[:, 2]; zc = np.where(zc < 0.1, 0.1, zc)
    xs = Vc[:, 0]*focal/zc + W/2
    ys = -Vc[:, 1]*focal/zc + H/2
    v2d = np.stack([xs, ys], 1)

    # 3-point lighting (key/fill/rim) as Lambert factors
    L_key = np.array([0.5, -0.5, 0.7]); L_key /= np.linalg.norm(L_key)
    L_fill = np.array([-0.6, 0.3, 0.4]); L_fill /= np.linalg.norm(L_fill)
    L_rim = np.array([-0.4, -0.6, -0.5]); L_rim /= np.linalg.norm(L_rim)
    kk, kf, kr = 0.9, 0.35, 0.25
    amb = 0.3

    fb = np.full((H, W, 3), 0.0)
    zb = np.full((H, W), 1e9)

    # raster floor first (a big quad) so the ship sits on it
    # floor corners in world
    fc = [(center[0]-span/2, center[1]-span/2, fy), (center[0]+span/2, center[1]-span/2, fy),
          (center[0]+span/2, center[1]+span/2, fy), (center[0]-span/2, center[1]+span/2, fy)]
    # project floor quad
    proj = []
    for c in fc:
        c = np.array(c)
        vc = (c - cam) @ Rm.T
        zz = -vc[2]; zz = max(zz, 0.1)
        proj.append([vc[0]*focal/zz + W/2, -vc[1]*focal/zz + H/2, -vc[2]])
    # two triangles
    for i0, i1, i2 in [(0, 1, 2), (0, 2, 3)]:
        p0 = np.array(proj[i0][:2]); p1 = np.array(proj[i1][:2]); p2 = np.array(proj[i2][:2])
        e1 = p1-p0; e2 = p2-p0
        area = e1[0]*e2[1]-e1[1]*e2[0]
        if abs(area) < 1e-6: continue
        x0 = max(0, int(min(p0[0], p1[0], p2[0]))); x1 = min(W-1, int(max(p0[0], p1[0], p2[0])))
        y0 = max(0, int(min(p0[1], p1[1], p2[1]))); y1 = min(H-1, int(max(p0[1], p1[1], p2[1])))
        # shade floor with a soft gradient (vignette)
        for yy in range(y0, y1+1):
            for xx in range(x0, x1+1):
                a = ((p1[1]-p2[1])*(xx-p2[0]) + (p2[0]-p1[0])*(yy-p2[1])) / area
                b = ((p2[1]-p0[1])*(xx-p2[0]) + (p0[0]-p2[0])*(yy-p2[1])) / area
                c = 1-a-b
                if a < 0 or b < 0 or c < 0: continue
                zv = a*(-proj[i0][2])+b*(-proj[i1][2])+c*(-proj[i2][2])
                if zv > zb[yy, xx]: continue
                zb[yy, xx] = zv
                grad = 0.7 + 0.3*(0.5-c)  # lighten toward camera
                fb[yy, xx] = floor_col * grad

    # background gradient (dark studio top to lighter horizon)
    for yy in range(H):
        t = yy / H
        fb[yy, :, :] = (0.04 + 0.16*(1-t))  # darker top (#0.04) to lighter bottom
    # (we overwrite with floor below horizon; the ship z-test handles the rest)

    # raster ship with texturing + 3-point lighting
    def tri(face, fuv):
        i0, i1, i2 = face[:, 0]
        p0, p1, p2 = v2d[i0], v2d[i1], v2d[i2]
        e1 = p1-p0; e2 = p2-p0
        area = e1[0]*e2[1]-e1[1]*e2[0]
        if area >= 0: return
        w0, w1, w2 = V[i0], V[i1], V[i2]
        n = np.cross(w1-w0, w2-w0); nl = np.linalg.norm(n)
        if nl < 1e-9: return
        n = n/nl
        if np.dot(n, (cam-w0)) < 0: n = -n
        # 3-point lambert
        lam = amb + kk*max(np.dot(n, L_key), 0) + kf*max(np.dot(n, L_fill), 0) + kr*max(np.dot(n, L_rim), 0)
        lam = min(lam, 1.3)
        # texture sample
        u0 = fuv[0][1]; u1 = fuv[1][1]; u2 = fuv[2][1]
        x0 = max(0, int(min(p0[0], p1[0], p2[0]))); x1 = min(W-1, int(max(p0[0], p1[0], p2[0])))
        y0 = max(0, int(min(p0[1], p1[1], p2[1]))); y1 = min(H-1, int(max(p0[1], p1[1], p2[1])))
        th, tw = D.shape[0], D.shape[1]
        for yy in range(y0, y1+1):
            for xx in range(x0, x1+1):
                a = ((p1[1]-p2[1])*(xx-p2[0]) + (p2[0]-p1[0])*(yy-p2[1])) / area
                b = ((p2[1]-p0[1])*(xx-p2[0]) + (p0[0]-p2[0])*(yy-p2[1])) / area
                c = 1-a-b
                if a < 0 or b < 0 or c < 0: continue
                zview = a*(-Vc[i0,2])+b*(-Vc[i1,2])+c*(-Vc[i2,2])
                if zview > zb[yy, xx]: continue
                zb[yy, xx] = zview
                col = D[th-1, tw-1]  # default
                if u0 >= 0 and u1 >= 0 and u2 >= 0:
                    uu = UV[u0][0]*a + UV[u1][0]*b + UV[u2][0]*c
                    vv = UV[u0][1]*a + UV[u1][1]*b + UV[u2][1]*c
                    tx = int((uu % 1.0)*tw) % tw; ty = int((vv % 1.0)*th) % th
                    col = D[ty, tx]
                    em = E[ty, tx]
                    if em.max() > 0.25:
                        col = col*lam*0.6 + em*np.array([1.6, 1.6, 1.6])
                    else:
                        col = col*lam
                else:
                    col = np.array([0.6, 0.62, 0.66])*lam
                fb[yy, xx] = np.clip(col, 0, 1)

    for fi in range(len(F)):
        tri(F[fi], F[fi])

    img = Image.fromarray((fb*255).astype(np.uint8))
    out = os.path.join(OUTDIR, objname + ".png")
    img.save(out)
    print("OK", objname, len(F), "tris ->", os.path.basename(out))


for name in sys.argv[1:]:
    try:
        render_one(name)
    except Exception as e:
        import traceback; traceback.print_exc()
        print("ERROR", name)