"""Remove windows / seam glow that a unique-hull bake painted into its textures.

Windows are separate geometry now (Tools/build_ship_windows.py), so a baked hull
that also gets a _Windows mesh must not show painted ones too. The unique-hull
composer (build_unique_hull_textures.compose) gave window glass roughness exactly
0.10, which makes a clean mask. Masked texels in D/N/R/M/AO are filled from the
surrounding plating (normalized-convolution blur, grown until the hole is closed).

  --mode strip      windows filled in, emissive cleared (hull gets geometry windows)
  --mode glow-only  keep the baked windows, clear only the emissive seam grid

Run: python Tools/strip_baked_windows.py [--mode strip|glow-only] Battleship_Unique
"""
import argparse
import os

import numpy as np
from PIL import Image

TEX = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                   "Assets", "FBX", "generated", "Textures")


def blur(a, sigma):
    """Periodic Gaussian blur over the last two spatial axes via FFT."""
    h, w = a.shape[:2]
    fy = np.fft.fftfreq(h)[:, None]
    fx = np.fft.fftfreq(w)[None, :]
    g = np.exp(-2 * (np.pi * sigma) ** 2 * (fx ** 2 + fy ** 2))
    if a.ndim == 2:
        return np.real(np.fft.ifft2(np.fft.fft2(a) * g))
    return np.stack([np.real(np.fft.ifft2(np.fft.fft2(a[..., c]) * g)) for c in range(a.shape[2])], -1)


def fill(a, hole):
    """Replace hole texels with the weighted average of nearby valid texels."""
    out = a.astype(np.float64).copy()
    known = (~hole).astype(np.float64)
    todo = hole.copy()
    for sigma in (3, 6, 12, 24, 48):
        wsum = blur(known, sigma)
        vals = blur(out * (known[..., None] if out.ndim == 3 else known), sigma)
        est = vals / np.maximum(wsum[..., None] if out.ndim == 3 else wsum, 1e-6)
        ok = todo & (wsum > 0.05)
        out[ok] = est[ok]
        todo &= ~ok
        if not todo.any():
            break
    return out


def load(name, suf, mode="RGB"):
    return np.asarray(Image.open(os.path.join(TEX, f"T_{name}_{suf}.png")).convert(mode)).astype(np.float64) / 255.0


def save(name, suf, a):
    a8 = np.round(np.clip(a, 0, 1) * 255).astype(np.uint8)
    Image.fromarray(a8, "RGB" if a8.ndim == 3 else "L").save(os.path.join(TEX, f"T_{name}_{suf}.png"))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("sets", nargs="+")
    ap.add_argument("--mode", choices=("strip", "glow-only"), default="strip")
    a = ap.parse_args()
    for name in a.sets:
        R = load(name, "R", "L")
        win = np.abs(R * 255 - 25.5) <= 1.0                  # composer's glass roughness 0.10
        # grow by 2 px so the frame/bleed around the glass goes too
        win = blur(win.astype(np.float64), 1.5) > 0.05
        E = load(name, "E")
        if a.mode == "glow-only":
            save(name, "E", E * win[..., None])
            print(f"{name}: seam glow cleared, {win.mean():.4%} window texels kept")
            continue
        for suf, mode in (("D", "RGB"), ("N", "RGB"), ("R", "L"), ("M", "L"), ("AO", "L")):
            if os.path.exists(os.path.join(TEX, f"T_{name}_{suf}.png")):
                save(name, suf, fill(load(name, suf, mode), win))
        save(name, "E", np.zeros_like(E))
        print(f"{name}: {win.mean():.4%} window texels filled, emissive cleared")


if __name__ == "__main__":
    main()
