"""Objective quality benchmark for the generated PBR texture sets.

Scores every T_<Set>_{D,N,R,M,AO,E} group against published PBR rules plus two
image-statistics checks, so texture iterations can be compared by number instead
of by eye. Pure numpy/PIL, no Blender or Unreal needed.

Benchmarks and where the thresholds come from
---------------------------------------------
albedo      Substance 3D Designer "PBR BaseColor/Metallic Validate": dielectric
            base colour luminance 30-240 sRGB (strict dark limit 50), metal
            reflectance 70-100 % = 180-255 sRGB.
metal       Unreal "Physically Based Materials": metallic is effectively binary
            (0 or 1); in-between only for mixed/corroded surfaces.
rough       Real surfaces sit mostly in 0.2-0.8 and are never perfectly uniform:
            a flat roughness map is the usual "CG plastic" giveaway. Scores the
            in-range share and the local variation.
normal      Tangent-space validity: unit length, z > 0, no mean tilt, not flat.
lighting    Albedo must not carry baked shading: correlation of albedo luminance
            with AO, plus the share of crushed-black AO.
tiling      Seam continuity for tiling sets (edge-pair difference vs. the
            average neighbouring-column difference). Skipped for unique UVs.
detail      Anti-gaming guard for the albedo rule: clamping a whole hull to one
            legal value passes "albedo" but reads as flat plastic. Scores the
            local albedo variation (std of sRGB luminance in 16 px blocks).
spectrum    Natural-image statistics: the radially averaged power spectrum of
            real surfaces falls off as 1/f^beta with beta about 2
            (Field 1987; van der Schaaf & van Hateren 1996).

Run:  python Tools/texture_benchmark.py [--dirs d1,d2] [--only A,B]
                                        [--json out.json] [--md out.md]
"""
import argparse
import json
import os
import re
import sys

import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
DEFAULT_DIRS = [os.path.join(ROOT, "Assets", "FBX", "generated", "Textures"),
                os.path.join(ROOT, "Assets", "Textures", "generated", "interiors")]
SUFFIXES = ("D", "N", "R", "M", "AO", "E")
WEIGHTS = {"albedo": 20, "detail": 10, "metal": 15, "rough": 15, "normal": 15,
           "lighting": 10, "tiling": 10, "spectrum": 5}
SAMPLE = 512  # analysis resolution for the per-pixel checks

# Sets whose UVs are unique (baked atlases), so seams are expected.
UNIQUE_RE = re.compile(r"(_Unique$|Bridge_|_Kit$|^Int_Cockpit$)")
# Sets that are transmissive or emissive by design: skip the dark-albedo rule.
# Wall trim sheets: tile along U only (V spans floor to ceiling).
WALL_RE = re.compile(r"Wall")
GLASS_RE = re.compile(r"(Glass|Viewport|Glow|Combat_)")


def load(path, gray=False):
    im = Image.open(path)
    im = im.convert("L" if gray else "RGB")
    return np.asarray(im).astype(np.float32) / 255.0


def sample(a, n=SAMPLE):
    """Nearest-neighbour stride sample: keeps per-pixel values honest (no filtering)."""
    sy = max(1, a.shape[0] // n)
    sx = max(1, a.shape[1] // n)
    return a[::sy, ::sx]


def srgb_to_lin(c):
    return np.where(c <= 0.04045, c / 12.92, ((c + 0.055) / 1.055) ** 2.4)


def lin_to_srgb(c):
    return np.where(c <= 0.0031308, c * 12.92, 1.055 * np.power(np.maximum(c, 0), 1 / 2.4) - 0.055)


def luminance_srgb(rgb):
    lin = srgb_to_lin(rgb)
    y = lin[..., 0] * 0.2126 + lin[..., 1] * 0.7152 + lin[..., 2] * 0.0722
    return lin_to_srgb(y) * 255.0


def ramp(x, good, bad):
    """1 at `good`, 0 at `bad`, linear between (works for either direction)."""
    t = (x - good) / (bad - good)
    return float(np.clip(1.0 - t, 0.0, 1.0))


def discover(dirs):
    sets = {}
    pat = re.compile(r"^T_(.+)_(" + "|".join(SUFFIXES) + r")\.png$")
    for d in dirs:
        if not os.path.isdir(d):
            continue
        for f in os.listdir(d):
            m = pat.match(f)
            if m:
                sets.setdefault(m.group(1), {})[m.group(2)] = os.path.join(d, f)
    return {k: v for k, v in sets.items() if "D" in v}


def spectral_beta(lum):
    g = lum - lum.mean()
    n = min(g.shape)
    g = g[:n, :n] * np.outer(np.hanning(n), np.hanning(n))
    p = np.abs(np.fft.fftshift(np.fft.fft2(g))) ** 2
    yy, xx = np.indices(p.shape)
    r = np.hypot(yy - n / 2, xx - n / 2).astype(int)
    radial = np.bincount(r.ravel(), p.ravel()) / np.maximum(np.bincount(r.ravel()), 1)
    f = np.arange(len(radial))
    band = (f >= 4) & (f <= n // 4) & (radial > 0)
    if band.sum() < 5:
        return 0.0
    slope = np.polyfit(np.log(f[band]), np.log(radial[band]), 1)[0]
    return float(-slope)


def seam_ratio(a, u_only=False):
    """Edge-pair difference over mean adjacent difference, worst axis.

    Wall sets are floor-to-ceiling trim sheets that only repeat along U, so
    only the left/right edge pair is tested for them.
    """
    out = []
    axes = (a,) if u_only else (a, a.transpose(1, 0, *range(2, a.ndim)))
    for arr in axes:
        inner = np.abs(np.diff(arr, axis=1)).mean()
        edge = np.abs(arr[:, 0] - arr[:, -1]).mean()
        out.append(edge / max(inner, 1e-4))
    return float(max(out))


def score_set(name, files):
    notes = []
    D = sample(load(files["D"]))
    shape = D.shape[:2]

    def opt(key, default, gray=True):
        if key in files:
            a = sample(load(files[key], gray=gray))
            if a.shape[:2] != shape:
                a = np.asarray(Image.fromarray((a * 255).astype(np.uint8)).resize(shape[::-1], Image.NEAREST)) / 255.0
            return a
        return np.full(shape, default, np.float32)

    M = opt("M", 0.0)
    R = opt("R", 0.5)
    AO = opt("AO", 1.0)
    E = opt("E", 0.0, gray=True)
    emissive = E > 0.15  # real light sources, not a faint tint glow
    lum = luminance_srgb(D)
    glass = bool(GLASS_RE.search(name))
    unique = bool(UNIQUE_RE.search(name))
    m = {}

    # --- albedo (Substance validator ranges) ---
    valid = ~emissive
    if "MASK" in files and "Grate" in name:
        # opacity-masked grating: the holes are never rendered
        valid &= opt("MASK", 1.0) >= 0.5
    metal = (M >= 0.5) & valid
    diel = (M < 0.5) & valid
    n_all = max(valid.sum(), 1)
    lo = 0 if glass else 30
    diel_ok = ((lum >= lo) & (lum <= 240))[diel].sum()
    diel_strict = ((lum >= (0 if glass else 50)) & (lum <= 240))[diel].sum()
    metal_ok = ((lum >= 180) & (lum <= 255))[metal].sum()
    frac = (diel_ok + metal_ok) / n_all
    strict = (diel_strict + metal_ok) / n_all
    m["albedo"] = 100 * (0.8 * frac + 0.2 * strict)
    m["_diel_out"] = float(1 - diel_ok / max(diel.sum(), 1)) if diel.any() else 0.0
    m["_metal_dark"] = float(1 - metal_ok / max(metal.sum(), 1)) if metal.any() else 0.0
    if m["_metal_dark"] > 0.25:
        notes.append("metal pixels too dark (%.0f%% < 180 sRGB)" % (100 * m["_metal_dark"]))
    if m["_diel_out"] > 0.10:
        notes.append("dielectric albedo out of 30-240 (%.0f%%)" % (100 * m["_diel_out"]))

    # --- albedo detail (guards against clamp-to-legal flattening) ---
    hh, ww = (lum.shape[0] // 16) * 16, (lum.shape[1] // 16) * 16
    lb = lum[:hh, :ww].reshape(hh // 16, 16, ww // 16, 16)
    lvar = float(np.median(lb.std(axis=(1, 3))))
    m["_albedo_local_std"] = lvar
    m["detail"] = 100 * ramp(lvar, 3.0, 0.3)
    if lvar < 1.0:
        notes.append("albedo flat (local std %.1f sRGB)" % lvar)

    # --- metallic binarity (UE doc) ---
    binary = ((M <= 0.1) | (M >= 0.9)).mean()
    m["metal"] = 100 * binary
    if binary < 0.8:
        notes.append("metallic not binary (%.0f%% in 0.1-0.9)" % (100 * (1 - binary)))

    # --- roughness ---
    in_rng = ((R >= 0.05) & (R <= 0.98)).mean()
    # local variation: std inside 16px blocks, averaged (flat-per-material maps score low)
    h, w = (R.shape[0] // 16) * 16, (R.shape[1] // 16) * 16
    blocks = R[:h, :w].reshape(h // 16, 16, w // 16, 16)
    local_std = float(blocks.std(axis=(1, 3)).mean())
    m["_rough_local_std"] = local_std
    m["rough"] = 100 * (0.4 * in_rng + 0.6 * min(1.0, local_std / 0.035))
    if local_std < 0.015:
        notes.append("roughness nearly flat (local std %.3f)" % local_std)

    # --- normal map ---
    if "N" in files:
        Nm = sample(load(files["N"])) * 2.0 - 1.0
        ln = np.linalg.norm(Nm, axis=-1)
        len_ok = (np.abs(ln - 1.0) < 0.1).mean()
        z_ok = (Nm[..., 2] > 0).mean()
        bias = float(np.hypot(Nm[..., 0].mean(), Nm[..., 1].mean()))
        flat = float(Nm[..., :2].std())
        m["_n_bias"], m["_n_std"] = bias, flat
        flat_ok = 1.0 if glass else ramp(flat, 0.04, 0.005)   # glass is flat by nature
        m["normal"] = 100 * (0.35 * len_ok + 0.25 * z_ok + 0.2 * ramp(bias, 0.02, 0.1)
                             + 0.2 * flat_ok)
        if bias > 0.05:
            notes.append("normal map tilted (mean xy %.3f)" % bias)
        if flat < 0.01 and not glass:
            notes.append("normal map almost flat")
    else:
        m["normal"] = 0.0
        notes.append("no normal map")

    # --- lighting baked into albedo ---
    ao_valid = valid & (AO < 0.999)
    if ao_valid.sum() > 100 and AO[ao_valid].std() > 1e-3 and lum[ao_valid].std() > 1e-3:
        corr = float(np.corrcoef(lum[ao_valid], AO[ao_valid])[0, 1])
    else:
        corr = 0.0
    crushed = float((AO < 0.15).mean())
    m["_ao_corr"], m["_ao_crushed"] = corr, crushed
    m["lighting"] = 100 * (0.6 * ramp(corr, 0.45, 0.9) + 0.4 * ramp(crushed, 0.01, 0.10))
    if corr > 0.7:
        notes.append("AO baked into albedo (corr %.2f)" % corr)
    if crushed > 0.05:
        notes.append("AO crushed to black (%.0f%% < 0.15)" % (100 * crushed))

    # --- tiling ---
    if unique:
        m["tiling"] = None
    else:
        full = load(files["D"])
        sr = seam_ratio(full, u_only=bool(WALL_RE.search(name)))
        m["_seam_ratio"] = sr
        m["tiling"] = 100 * ramp(sr, 1.5, 4.0)
        if sr > 2.5:
            notes.append("visible tiling seam (ratio %.1f)" % sr)

    # --- spectrum ---
    beta = spectral_beta(lum / 255.0)
    m["_beta"] = beta
    m["spectrum"] = 100 * (1.0 if 1.6 <= beta <= 2.8 else ramp(abs(beta - 2.2), 0.6, 1.6))

    tot, wsum = 0.0, 0.0
    for k, wgt in WEIGHTS.items():
        if m.get(k) is not None:
            tot += wgt * m[k]
            wsum += wgt
    m["score"] = tot / wsum
    return {"name": name, "unique": unique, "metrics": m, "notes": notes}


def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument("--dirs", default=",".join(DEFAULT_DIRS))
    ap.add_argument("--only", default="")
    ap.add_argument("--json", default="")
    ap.add_argument("--md", default="")
    a = ap.parse_args(argv)
    sets = discover([d for d in a.dirs.split(",") if d])
    if a.only:
        keep = set(a.only.split(","))
        sets = {k: v for k, v in sets.items() if k in keep}
    results = [score_set(k, sets[k]) for k in sorted(sets)]
    results.sort(key=lambda r: r["metrics"]["score"])
    cols = ["score"] + list(WEIGHTS)
    lines = ["| set | " + " | ".join(cols) + " | notes |",
             "|---|" + "---|" * len(cols) + "---|"]
    for r in results:
        mm = r["metrics"]
        cells = ["-" if mm.get(c) is None else "%.0f" % mm[c] for c in cols]
        lines.append("| %s | %s | %s |" % (r["name"], " | ".join(cells), "; ".join(r["notes"])))
    mean = float(np.mean([r["metrics"]["score"] for r in results])) if results else 0.0
    lines.append("")
    lines.append("**Mean score: %.1f over %d sets**" % (mean, len(results)))
    text = "\n".join(lines)
    print(text)
    if a.md:
        open(a.md, "w", encoding="utf-8").write(text + "\n")
    if a.json:
        json.dump({"mean": mean, "sets": results}, open(a.json, "w"), indent=1)
    return 0


if __name__ == "__main__":
    sys.exit(main())
