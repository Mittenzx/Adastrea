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
BASICS = {
    "Basic_MetalGrey":   dict(base=(0.55, 0.56, 0.58), rough=0.42, metal=1.0, ao=0.85, em=(0,0,0)),
    "Basic_MetalDark":   dict(base=(0.24, 0.25, 0.27), rough=0.50, metal=1.0, ao=0.80, em=(0,0,0)),
    "Basic_HullMatte":   dict(base=(0.42, 0.44, 0.46), rough=0.80, metal=0.0, ao=0.70, em=(0,0,0)),
    "Basic_CabinWhite":  dict(base=(0.82, 0.83, 0.85), rough=0.72, metal=0.0, ao=0.60, em=(0,0,0)),
    "Basic_CabinMint":   dict(base=(0.72, 0.80, 0.78), rough=0.70, metal=0.0, ao=0.62, em=(0,0,0)),
    "Basic_ConsoleBlack":dict(base=(0.12, 0.13, 0.15), rough=0.55, metal=0.1, ao=0.65, em=(0,0,0)),
    "Basic_Glass":       dict(base=(0.30, 0.36, 0.42), rough=0.10, metal=0.9, ao=0.9, em=(0.04,0.08,0.12)),
    "Basic_HazardY":     dict(base=(0.75, 0.58, 0.10), rough=0.60, metal=0.1, ao=0.70, em=(0.02,0.01,0)),
    "Basic_FloorGrey":   dict(base=(0.30, 0.31, 0.33), rough=0.85, metal=0.0, ao=0.55, em=(0,0,0)),
    "Basic_AccentTeal":  dict(base=(0.12, 0.40, 0.42), rough=0.55, metal=0.3, ao=0.70, em=(0,0.03,0.04)),
}


def write(name, maps):
    for suf, arr in maps.items():
        img = Image.fromarray((np.clip(arr, 0, 1) * 255).astype(np.uint8))
        img.save(os.path.join(BASE, f"T_{name}_{suf}.png"))


def gen(name, cfg, seed=0):
    rng = np.random.default_rng(seed)
    base = np.array(cfg["base"], dtype=np.float32)
    # BaseColor: flat color + very subtle large-scale variation (light/patch)
    patch = rng.random((SIZE, SIZE)).astype(np.float32) * 0.02 - 0.01
    D = np.zeros((SIZE, SIZE, 4), dtype=np.float32)
    for ch in range(3):
        D[..., ch] = np.clip(base[ch] + patch, 0, 1)
    D[..., 3] = 1.0
    # Normal: near-flat (only faint micro-noise for N map so it isn't pure flat)
    nscale = 0.4
    Nz = np.clip(0.5 + (rng.random((SIZE, SIZE)).astype(np.float32) - 0.5) * nscale, 0, 1)
    # build a proper normal map: ~ (0.5,0.5,1.0) neutral with tiny variation
    ndx = np.full((SIZE, SIZE), 0.5, np.float32)
    ndy = np.full((SIZE, SIZE), 0.5, np.float32)
    N = np.zeros((SIZE, SIZE, 4), dtype=np.float32)
    N[..., 0] = ndx
    N[..., 1] = ndy
    N[..., 2] = Nz          # near 1.0 = flat
    N[..., 3] = 1.0
    # Roughness / Metallic / AO: near-constant
    R = np.full((SIZE, SIZE), cfg["rough"], np.float32)
    M = np.full((SIZE, SIZE), cfg["metal"], np.float32)
    AO = np.full((SIZE, SIZE), cfg["ao"], np.float32)
    # slight roughness noise for realism
    R = np.clip(R + (rng.random((SIZE, SIZE)).astype(np.float32) - 0.5) * 0.06, 0, 1)
    # Emissive: near-zero flat
    E = np.zeros((SIZE, SIZE, 4), dtype=np.float32)
    em = np.array(cfg["em"], dtype=np.float32)
    E[..., 0] = em[0]; E[..., 1] = em[1]; E[..., 2] = em[2]; E[..., 3] = 0.0
    # write color maps as RGBA, scalar maps as L
    write(name, {"D": D, "N": N, "E": E})
    for suf, arr in (("R", R), ("M", M), ("AO", AO)):
        img = Image.fromarray((arr * 255).astype(np.uint8), mode="L")
        img.save(os.path.join(BASE, f"T_{name}_{suf}.png"))


def main():
    for i, (name, cfg) in enumerate(BASICS.items()):
        gen(name, cfg, seed=i * 7 + 1)
        print("ok", name)
    print(f"done — {len(BASICS)} basic texture sets -> {BASE}")


if __name__ == "__main__":
    main()