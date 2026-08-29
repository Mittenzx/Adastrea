#!/usr/bin/env python3
"""Generate a HIGHER-RES (4096x2048) realistic starfield with hundreds of
thousands of TINY 1px stars + subtle nebula. Soft small stars read realistic;
the previous 2048 tex was too coarse so stars magnified into squares.
"""
import random
import numpy as np
from PIL import Image, ImageDraw, ImageFilter

W, H = 4096, 2048
random.seed(42)

# Subtle nebula (few soft blobs)
cloud = Image.new("RGB", (W, H), (0, 0, 0))
dc = ImageDraw.Draw(cloud)
pg = random.Random(7)
neb = [(36, 16, 64), (16, 38, 66), (58, 26, 36), (8, 46, 42), (28, 22, 56)]
for _ in range(26):
    cx, cy = pg.uniform(0, W), pg.uniform(0, H)
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

# Mostly 1px pinpoints, a scattering of 2px, very few 3px
for _ in range(26000):
    sx, sy = rg.uniform(0, W), rg.uniform(0, H)
    b = rg.uniform(0.5, 1.0)
    size = 1
    r = rg.random()
    if r < 0.10: size = 2
    elif r < 0.14: size = 3
    c = tuple(int(v*b) for v in rg.choice(cols))
    d.ellipse([sx-size, sy-size, sx+size, sy+size], fill=c)

# A handful of brighter 4px stars with a tiny soft glow
for _ in range(90):
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