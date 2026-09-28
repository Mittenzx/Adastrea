"""Make existing baked texture sets PBR-plausible in place (no re-bake, no mesh/UV change).

For baked atlases whose bake cache is gone (unique ship hulls, capital-bridge
atlases). Applies the same rules as generate_adastrea_assets.PBR_CONFORM and
generate_interior_texture_library.pbr_conform, scored by Tools/texture_benchmark.py:

  metallic   snapped to 0/1 (smoothstep around --metal-cut, anti-aliased)
  albedo     metal lifted to >= 182 sRGB, dielectric soft-kneed into 35-240 sRGB,
             both by remaps that keep relative variation
  --coat-dark  metal darker than 150 sRGB and unsaturated becomes a dark coating
             (dielectric) instead of being lifted - keeps dark interiors dark
  --unshade K  divide out baked contact shading D *= (1-K) + K*AO  (use the K the
             composer multiplied in; the unique-hull composer used 0.35)
  AO         floored at 0.2

Run: python Tools/pbr_conform_textures.py [--metal-cut 0.6] [--coat-dark] [--unshade 0.35]
                                          [--backup DIR] <texdir> Set1 Set2 ...
"""
import argparse
import os
import shutil

import numpy as np
from PIL import Image


def srgb_lum(rgb):
    lin = np.where(rgb <= 0.04045, rgb / 12.92, ((rgb + 0.055) / 1.055) ** 2.4)
    y = lin[..., 0] * 0.2126 + lin[..., 1] * 0.7152 + lin[..., 2] * 0.0722
    return np.where(y <= 0.0031308, y * 12.92, 1.055 * np.power(np.maximum(y, 0), 1 / 2.4) - 0.055)


def load(p, gray=False):
    return np.asarray(Image.open(p).convert("L" if gray else "RGB")).astype(np.float32) / 255.0


def save(p, a):
    a8 = np.round(np.clip(a, 0, 1) * 255).astype(np.uint8)
    Image.fromarray(a8, "RGB" if a8.ndim == 3 else "L").save(p)


def conform_arrays(D, M, AO, E, metal_cut=0.6, coat_dark=False, unshade=0.0):
    """Array version (float 0-1, D/E HxWx3 sRGB, M/AO HxW). Returns D, M, AO.
    Also called by the bake composers in build_unique_hull_textures.py and
    bake_interior_unique.py so rebakes come out conformed."""
    lit = E.max(axis=-1) > 0.15

    if unshade > 0:
        D = D / ((1 - unshade) + unshade * np.clip(AO, 0.05, 1))[..., None]
        D = np.clip(D, 0, 1)
    t = np.clip((M - (metal_cut - 0.1)) / 0.2, 0, 1)
    M = t * t * (3 - 2 * t)
    lum = srgb_lum(D)
    if coat_dark:
        mx, mn = D.max(axis=-1), D.min(axis=-1)
        sat = (mx - mn) / np.maximum(mx, 1e-3)
        coated = np.clip((150 / 255 - lum) / (20 / 255), 0, 1) * np.clip((0.35 - sat) / 0.1, 0, 1)
        M = M * (1 - coated)
    met_t = np.where(lum < 235 / 255, 182 / 255 + lum * (53 / 235), lum)
    die_t = np.where(lum < 70 / 255, 35 / 255 + lum * (35 / 70), lum)
    die_t = np.where(lum > 215 / 255, 215 / 255 + (lum - 215 / 255) * (25 / 40), die_t)
    wm = np.clip((M - 0.3) / 0.15, 0, 1)   # albedo follows the metal mask tightly
    target = wm * met_t + (1 - wm) * die_t
    scale = np.where(lit, 1.0, target / np.maximum(lum, 1e-3))
    D = np.clip(D * scale[..., None], 0, 1)
    AO = np.clip(AO, 0.2, 1)
    return D.astype(np.float32), M.astype(np.float32), AO.astype(np.float32)


def conform(texdir, name, metal_cut, coat_dark, unshade, backup):
    f = {s: os.path.join(texdir, f"T_{name}_{s}.png") for s in ("D", "M", "AO", "E")}
    if backup:
        os.makedirs(backup, exist_ok=True)
        for p in f.values():
            if os.path.exists(p):
                shutil.copy2(p, backup)
    D = load(f["D"])
    M = load(f["M"], True)
    AO = load(f["AO"], True) if os.path.exists(f["AO"]) else np.ones(M.shape, np.float32)
    E = load(f["E"]) if os.path.exists(f["E"]) else np.zeros_like(D)
    D, M, AO = conform_arrays(D, M, AO, E, metal_cut, coat_dark, unshade)
    save(f["D"], D)
    save(f["M"], M)
    if os.path.exists(f["AO"]):
        save(f["AO"], AO)
    print("conformed", name)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("texdir")
    ap.add_argument("sets", nargs="+")
    ap.add_argument("--metal-cut", type=float, default=0.6)
    ap.add_argument("--coat-dark", action="store_true")
    ap.add_argument("--unshade", type=float, default=0.0)
    ap.add_argument("--backup", default="")
    a = ap.parse_args()
    for s in a.sets:
        conform(a.texdir, s, a.metal_cut, a.coat_dark, a.unshade, a.backup)


if __name__ == "__main__":
    main()
