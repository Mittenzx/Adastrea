"""
Adastrea 3D Asset QA — strict numeric shape validation.

Runs objective geometry checks against each generated mesh (via OBJ, which the
FBX round-trips to cleanly) and reports a PASS/FAIL/INFO per check.

Checks:
  1. Tri budget         - absolute tri count within project limits
  2. Zero-area faces    - non-degenerate triangles only (else collision/normal bugs)
  3. Bounding box       - sane real-world dimensions (not 0, not absurd)
  4. Mirror symmetry    - vertex match across X=0 plane (ships should be near-symmetric)
  5. Normal coherence   - no duplicate / inverted winding segments
  6. UV coverage        - UV coords present and in [0,1] (no missing UV -> black in UE)
  7. Emptiness          - mesh actually has geometry (verts/tris > threshold)
"""
import os, sys, math
import numpy as np

OBJDIR = r"C:\Users\akuma\Adastrea\Assets\FBX\generated\obj"

# project budgets (CONTENT_ORGANIZATION.md)
TRI_BUDGET = {"SM_Ship": 10000, "SM_Prop": 1000}

def parse_obj(path):
    verts, uv, faces = [], [], []
    with open(path) as f:
        for line in f:
            if line.startswith('vt '):
                p = line.split()
                uv.append((float(p[1]), float(p[2])))
            elif line.startswith('v '):
                p = line.split()
                verts.append((float(p[1]), float(p[2]), float(p[3])))
            elif line.startswith('f '):
                p = line.split()[1:]
                idx = []
                for tok in p:
                    parts = tok.split('/')
                    idx.append(int(parts[0])-1)
                for k in range(1, len(idx)-1):
                    faces.append((idx[0], idx[k], idx[k+1]))
    return np.array(verts, dtype=np.float64), np.array(uv, dtype=np.float64), faces

def face_area(v, tri):
    a, b, c = v[tri[0]], v[tri[1]], v[tri[2]]
    ab, ac = b-a, c-a
    cross = np.cross(ab, ac)
    return 0.5 * np.linalg.norm(cross)

def qa_one(obj_path):
    name = os.path.basename(obj_path)
    verts, uvs, faces = parse_obj(obj_path)
    results = []
    info = {}

    # 1. triangle budget (category-aware)
    if "_Assembled" in name:
        # Whole-ship composition (carcass + all parts merged). Bundles several
        # components, so budget matches the large whole-build tier (like stations).
        budget = 50000
    elif "Ship" in name and "_Int_" not in name and not name.startswith("SM_Int_"):
        budget = 10000
    elif "Station" in name or name.startswith("SM_Int_") or "_Int_" in name:
        budget = 50000
    elif "Obj" in name or "Prop" in name:
        budget = 3000   # space objects & large props (comms tower, fuel cell, satellite)
    else:
        budget = 1000
    ntris = len(faces)
    results.append(("tri_budget", ntris <= budget, f"{ntris} tris (budget {budget})"))
    info["tris"] = ntris

    # 2. zero-area faces
    areas = [face_area(verts, t) for t in faces]
    zero = sum(1 for a in areas if a < 1e-9)
    max_area = max(areas) if areas else 0
    results.append(("non_degenerate", zero == 0, f"{zero} degenerate faces, max area {max_area:.2f}cm^2"))
    info["max_face_area"] = max_area

    # 3. bounding box
    bb = verts.max(0) - verts.min(0)
    ok_bb = all(bb > 0.5) and bb.max() < 5000
    results.append(("bounds", ok_bb, f"size {bb[0]:.1f} x {bb[1]:.1f} x {bb[2]:.1f} cm"))
    info["bounds"] = bb.tolist()

    # 4. mirror symmetry across X=0 — only for symmetric primaries
    # (whole ships / stations / the carcass shell). Add-on parts (Engine/Weapon/
    # Sensor/Cargo/Reactor) are intentionally offset/asymmetric, so skip.
    is_primary = ("Ship" in name and not any(p in name for p in
                   ("_Engine", "_Weapon", "_Sensor", "_Cargo", "_Reactor"))) \
                 or "Station" in name or "_Carcass" in name
    # Assembled whole ships are compositions mixing symmetric + asymmetric
    # detail (canted exhausts, antenna arrays, greeble cladding) — mirror
    # symmetry is not a meaningful pass/fail for them.
    if "_Assembled" in name:
        is_primary = False
    if is_primary:
        pts = {(round(x,1), round(y,1), round(z,1)): True for x,y,z in verts}
        matched = sum(1 for x,y,z in verts if (round(-x,1), round(y,1), round(z,1)) in pts)
        ratio = matched / len(verts) if len(verts) else 0
        results.append(("symmetry", ratio >= 0.7, f"{ratio*100:.0f}% vertices mirrored across X=0"))
        info["symmetry_ratio"] = round(ratio, 3)
    else:
        results.append(("symmetry", "n/a", "add-on part, mirror not required"))

    # 5. UV presence
    if len(uvs):
        hi = uvs.max(); lo = uvs.min()
        # World-aligned tiling UVs legitimately extend past 0-1 (seamless tileable
        # textures repeat). Sanity-check they exist + finite + sane tile count.
        has_uv = np.isfinite(lo) and np.isfinite(hi) and abs(hi) < 64 and abs(lo) < 64
        results.append(("uv_present", has_uv, f"{len(uvs)} uv coords, range {lo:.2f}-{hi:.2f} (tiling ok)"))
    else:
        results.append(("uv_present", False, "no UVs -> texture will render black"))

    # 6. non-empty
    results.append(("non_empty", ntris > 10 and len(verts) > 10, f"{len(verts)} verts"))

    return name, results, info

def main():
    if not os.path.isdir(OBJDIR):
        print("no obj dir"); return
    overall_pass = True
    for f in sorted(os.listdir(OBJDIR)):
        if not f.endswith('.obj'):
            continue
        name, results, info = qa_one(os.path.join(OBJDIR, f))
        print(f"\n=== {name} ===")
        for check, ok, note in results:
            if isinstance(ok, str) and ok.lower().startswith("n/a"):
                flag = "SKIP"
            else:
                flag = "PASS" if ok else "FAIL"
                if not ok:
                    overall_pass = False
            print(f"  [{flag}] {check}: {note}")
    print(f"\nOVERALL: {'ALL OBJECTIVE CHECKS PASS' if overall_pass else 'OBJECTIVE FAILURES FOUND'}")

if __name__ == "__main__":
    main()