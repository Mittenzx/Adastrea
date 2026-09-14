#!/usr/bin/env python3
"""Generate a HIGHER-RES (4096x2048) realistic starfield with hundreds of
thousands of TINY 1px stars + subtle nebula. Soft small stars read realistic;
the previous 2048 tex was too coarse so stars magnified into squares.

v2 additions (still pure PIL/numpy, no Blender/UE needed):
  1. Milky Way band -- real dark-sky views show a distinct band of much higher
     star (and nebula) density along the galactic plane; the v1 texture was
     fully uniform-random, which reads noticeably more "generated" than real.
     Modeled as a wavy band across the equirect U axis (a tilted great circle
     projects as a sine wave in equirect, not a straight line) with a Gaussian
     falloff in V controlling how much extra star/nebula density concentrates
     near it.
  2. Magnitude-realistic brightness -- real stars follow a steep
     count-vs-brightness curve (far more faint stars than bright ones); v1 drew
     brightness uniformly in [0.5, 1.0], which reads as a flat "wall of
     medium-bright dots" instead of the sparse-bright/dense-faint look of an
     actual sky.
"""
import math
import random
import numpy as np
from PIL import Image, ImageDraw, ImageFilter

W, H = 4096, 2048
random.seed(42)

# Wavy Milky Way band: v(u) = center + amplitude * sin(2*pi*u*freq + phase).
BAND_PHASE = 1.3
BAND_FREQ = 1.0
BAND_AMPLITUDE = 0.16   # how far the band wanders in V (fraction of H)
BAND_WIDTH = 0.055      # gaussian sigma in V for the dense band (fraction of H)


def band_v(u_frac):
    return 0.5 + BAND_AMPLITUDE * math.sin(2.0 * math.pi * u_frac * BAND_FREQ + BAND_PHASE)


# ---- Nebula: subtle soft blobs, concentrated near the Milky Way band (like
# real emission/dust nebulae, which cluster near the galactic plane) rather
# than scattered uniformly across the whole sky. ----
cloud = Image.new("RGB", (W, H), (0, 0, 0))
dc = ImageDraw.Draw(cloud)
pg = random.Random(7)
neb = [(36, 16, 64), (16, 38, 66), (58, 26, 36), (8, 46, 42), (28, 22, 56)]
for _ in range(30):
    cx = pg.uniform(0, W)
    cy_center = band_v(cx / W) * H
    cy = pg.gauss(cy_center, BAND_WIDTH * H * 2.2)  # looser than the star band
    cy = min(max(cy, 0), H - 1)
    r = pg.uniform(240, 900)
    dc.ellipse([cx - r, cy - r, cx + r, cy + r], fill=pg.choice(neb))
cloud = cloud.filter(ImageFilter.GaussianBlur(320))
neb_a = np.asarray(cloud, dtype=np.float32) / 255.0
base = np.zeros((H, W, 3), dtype=np.float32)
base[:] = 0.010
base += neb_a * 0.30

img = Image.fromarray((np.clip(base, 0, 1) * 255).astype(np.uint8)).convert("RGB")
d = ImageDraw.Draw(img)
rg = random.Random(123)
cols = [(255,255,255),(200,215,255),(255,235,205),(210,230,255),(255,250,230)]

# Mostly 1px pinpoints, a scattering of 2px, very few 3px. Density mixes a
# sparse uniform "nearby stars" field with a denser Milky Way band population,
# and brightness is skewed toward faint (real magnitude distributions have far
# more dim stars than bright ones -- uniform brightness reads artificial).
TOTAL_STARS = 32000
BAND_FRACTION = 0.55  # share of stars drawn concentrated in the band vs uniform sky

for _ in range(TOTAL_STARS):
    if rg.random() < BAND_FRACTION:
        sx = rg.uniform(0, W)
        centre_y = band_v(sx / W) * H
        sy = rg.gauss(centre_y, BAND_WIDTH * H)
        sy = min(max(sy, 0), H - 1)
    else:
        sx, sy = rg.uniform(0, W), rg.uniform(0, H)

    # Skewed toward dim: random()**2.5 concentrates near 0, with a floor so
    # stars never fully vanish into the background.
    b = 0.15 + 0.85 * (rg.random() ** 2.5)
    size = 1
    r = rg.random()
    if r < 0.10: size = 2
    elif r < 0.14: size = 3
    c = tuple(int(v*b) for v in rg.choice(cols))
    d.ellipse([sx-size, sy-size, sx+size, sy+size], fill=c)

# A handful of brighter 4px stars with a tiny soft glow -- also weighted
# toward the band, matching how the brightest naked-eye stars still skew
# toward the galactic plane on a real dark-sky map.
for _ in range(90):
    if rg.random() < BAND_FRACTION:
        sx = rg.uniform(0, W)
        sy = min(max(rg.gauss(band_v(sx / W) * H, BAND_WIDTH * H), 0), H - 1)
    else:
        sx, sy = rg.uniform(0, W), rg.uniform(0, H)
    glow = Image.new("RGB", (W, H), (0,0,0))
    dg = ImageDraw.Draw(glow)
    dg.ellipse([sx-5, sy-5, sx+5, sy+5], fill=(255,255,255))
    glow = glow.filter(ImageFilter.GaussianBlur(5))
    ga = np.asarray(glow, dtype=np.float32)/255.0*0.9
    ar = np.asarray(img, dtype=np.float32)
    img = Image.fromarray(np.maximum(ar, ga*255).astype(np.uint8))

out = r"C:\Users\akuma\Adastrea\Assets\FBX\generated\T_Starfield.png"
img.save(out)
print("saved", out, img.size)
