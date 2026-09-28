"""Generate BASIC flat textures — simple tileable PBR sets (D/N/R/M/AO/E) for
plain materials that shouldn't look busy (matte metal, plain hull, cabin panels,
glass, console, hazard). Light, clean, deterministic.

Writes T_<name>_<map>.png into Assets/FBX/generated/Textures/ alongside the
detailed sci-fi sets. Usage: python Tools/gen_basic_textures.py
"""
import os
import numpy as np
from PIL import Image

BASE = r"C:\Users\akuma\Adastrea\Assets\FBX\generated\Textures"
os.makedirs(BASE, exist_ok=True)
SIZE = 1024

# name -> dict(base_color(rgb,0-1), rough, metal, ao, emissive(rgb), matte:bool)
# PBR pass 2026-09-28 (scored by Tools/texture_benchmark.py): metallic is 0 or 1;
# bare metals have >= 180 sRGB albedo (a "dark metal" is a dark anodised coating,
# i.e. a dielectric); AO is 1.0 because a flat tile has no cavities (a constant AO
# only dims the material); `bump` is a faint micro-surface (paint orange-peel /
# brushing) so normals aren't dead flat.
BASICS = {
    "Basic_MetalGrey":   dict(base=(0.78, 0.79, 0.81), rough=0.42, metal=1.0, em=(0,0,0), bump=0.6, brushed=True),
    "Basic_MetalDark":   dict(base=(0.24, 0.25, 0.27), rough=0.45, metal=0.0, em=(0,0,0), bump=0.6, brushed=True),
    "Basic_HullMatte":   dict(base=(0.42, 0.44, 0.46), rough=0.80, metal=0.0, em=(0,0,0), bump=1.0),
    "Basic_CabinWhite":  dict(base=(0.82, 0.83, 0.85), rough=0.72, metal=0.0, em=(0,0,0), bump=0.8),
    "Basic_CabinMint":   dict(base=(0.72, 0.80, 0.78), rough=0.70, metal=0.0, em=(0,0,0), bump=0.8),
    "Basic_ConsoleBlack":dict(base=(0.16, 0.17, 0.19), rough=0.55, metal=0.0, em=(0,0,0), bump=0.5),
    "Basic_Glass":       dict(base=(0.30, 0.36, 0.42), rough=0.08, metal=0.0, em=(0.04,0.08,0.12), bump=0.25),
    "Basic_HazardY":     dict(base=(0.75, 0.58, 0.10), rough=0.60, metal=0.0, em=(0.02,0.01,0), bump=1.0),
    "Basic_FloorGrey":   dict(base=(0.30, 0.31, 0.33), rough=0.85, metal=0.0, em=(0,0,0), bump=1.2),
    "Basic_AccentTeal":  dict(base=(0.12, 0.40, 0.42), rough=0.55, metal=0.0, em=(0,0.03,0.04), bump=0.8),
}


def fbm(rng, wavelengths, aniso=1.0):
    """Tileable fBm (FFT Gaussian bands), unit std. aniso > 1 stretches along U."""
    fy = np.fft.fftfreq(SIZE)[:, None]
    fx = np.fft.fftfreq(SIZE)[None, :] * aniso
    fr = np.sqrt(fx * fx + fy * fy)
    out = np.zeros((SIZE, SIZE), np.float32)
    for wl in wavelengths:
        band = np.real(np.fft.ifft2(np.fft.fft2(rng.standard_normal((SIZE, SIZE))) * np.exp(-2.0 * (fr * wl) ** 2)))
        out += (band / max(band.std(), 1e-6) * wl / wavelengths[0]).astype(np.float32)
    return out / max(out.std(), 1e-6)


def write(name, maps):
    for suf, arr in maps.items():
        img = Image.fromarray((np.clip(arr, 0, 1) * 255).astype(np.uint8))
        img.save(os.path.join(BASE, f"T_{name}_{suf}.png"))


def gen(name, cfg, seed=0):
    rng = np.random.default_rng(seed)
    base = np.array(cfg["base"], dtype=np.float32)
    aniso = 8.0 if cfg.get("brushed") else 1.0
    # BaseColor: flat colour + faint large-scale fading (+-3 %) and fine grain
    tone = 0.03 * fbm(rng, (256, 64, 16)) + 0.035 * fbm(rng, (4, 2), aniso)
    D = np.zeros((SIZE, SIZE, 4), dtype=np.float32)
    for ch in range(3):
        D[..., ch] = np.clip(base[ch] * (1.0 + tone), 0, 1)
    D[..., 3] = 1.0
    # Normal: unit-length tangent normals from a faint micro-height field
    h = fbm(rng, (6, 3, 1.5), aniso) * cfg.get("bump", 1.0)
    dy, dx = np.gradient(h)
    k = 0.12
    n = np.stack([-dx * k, dy * k, np.ones_like(h)], -1)   # DirectX (green down)
    n /= np.linalg.norm(n, axis=-1, keepdims=True)
    N = np.ones((SIZE, SIZE, 4), dtype=np.float32)
    N[..., :3] = n * 0.5 + 0.5
    # Roughness: base + smooth variation (smudges) + fine grain
    R = np.clip(cfg["rough"] + 0.05 * fbm(rng, (128, 32)) + 0.025 * fbm(rng, (3, 1.5), aniso), 0.02, 1)
    M = np.full((SIZE, SIZE), cfg["metal"], np.float32)
    AO = np.ones((SIZE, SIZE), np.float32)
    E = np.zeros((SIZE, SIZE, 4), dtype=np.float32)
    em = np.array(cfg["em"], dtype=np.float32)
    E[..., 0] = em[0]; E[..., 1] = em[1]; E[..., 2] = em[2]; E[..., 3] = 1.0
    write(name, {"D": D, "N": N, "E": E})
    for suf, arr in (("R", R), ("M", M), ("AO", AO)):
        img = Image.fromarray((np.clip(arr, 0, 1) * 255).astype(np.uint8), mode="L")
        img.save(os.path.join(BASE, f"T_{name}_{suf}.png"))


def main():
    for i, (name, cfg) in enumerate(BASICS.items()):
        gen(name, cfg, seed=i * 7 + 1)
        print("ok", name)
    print(f"done — {len(BASICS)} basic texture sets -> {BASE}")


if __name__ == "__main__":
    main()