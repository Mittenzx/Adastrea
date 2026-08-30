# SHIP TEXTURES & SKINS — next level (X4 benchmark / plan)

> Research-backed plan to take Adastrea's ship textures/skins from "flat procedural
> panels" to X4-grade. Benchmarked against how X4: Foundations (Egosoft) actually
> builds its ship materials, paint/skin system, and vertex channels.
> **Status:** Plan / no code changed. **Sources:** Egosoft X-Community Wiki
> (Making custom ships, Paintmods, Modifying paintmods), Steam custom-paintmod guide.

---

## 1. What X4 actually does (the grounded mechanics)

From the Egosoft docs, an X4 ship part is a **mesh + a per-part material** driven by
a `P1_complex_surface.fx` shader with these inputs:

- **Albedo/diffuse** (`_diffuse`), **roughness** (`_roughness`), **metal** (`_metal`),
  **normal** (`_normal`) — a standard PBR set (metal-rough workflow).
- **Shader-driven color/weathering**, not baked: material floats like `diffuseStr`,
  `normalStr`, `environmentStr`, `Smoothness`, `Metallness`, `AnisoX/Y`.

The **paint/skin system is where X4 really separates**: a paintmod is a **pattern
texture + an XML recipe of color/`shader` values** that get **mixed on top of the
base texture at runtime**, not re-baked per skin. The recipe is:
`paintmods.xml` → `<paint ware hue brightness saturation metal smooth dirt extradirt
pattern strength scale sharpness invert red green blue alpha/>`.

Critically, this relies on **vertex-color channels** baked into the mesh:
- **`col`** = base part color in-game (red/blue/grey/black)
- **`paintmodmask`** = *where* a paintmod shows vs. doesn't (#FF0000 apply / #000000 don't)
- **`dirt`** = inverted baked AO→ rust/dirt overlay (newer ships)
- **`uv3`** = separate UV channel for the *dirt/streak overlay* texture
- **`uv1`** = primary texture UV
- **`idcode`** = generated registration number placement

So X4 gets its look from: **(a) per-part PBR material sections, (b) runtime-blended
skins via vertex masks instead of thousands of baked skins, and (c) a dirt/streak
UV3 overlay + AO** — all cheap, all data-driven.

### The "paintmods" loop (what makes it feel good)
1. Ships have **many quality tiers** (basic/advanced/exceptional).
2. Skins are **reputation-gated faction paints** (+27 rep) — a progression/reward.
3. Apply via a **"Redesign" menu** (dock → right-click ship → Redesign → preview free,
   install consumes the paintmod).
4. ~55 paintmods in base game; swap color/hue/brightness/saturation/metal at runtime.

---

## 2. Where Adastrea is today vs. X4

| Aspect | Adastrea now | X4 |
|---|---|---|
| Texture set | Procedural `_D/_N/_R/_M/_E` per material | Same PBR set, but per-part sections |
| Skins | None (one baked texture per ship) | Runtime-blended pattern+color over a mask |
| Weathering | Some grime/hazard | Vertex-AO `dirt` + `uv3` streak overlay channel |
| Part separation | Ships = parts, but each maps one texture | Each part = dedicated material section + vertex channels |
| Detail density | Moderate panel/rivet | Tile+detail_xl LOD budget, high-density |
| Material control | Fixed shader values | Shader floats: aniso, env, smoothness, metallness |

---

## 3. The plan — take ships to the next level (in priority order)

### Phase 1 — Per-part texture sets (biggest visual lift, cheapest)
**Problem:** today *every* ship part (carcass engine cargo) uses the same
`T_Ship_Hull` → flat, no contrast between a hull and a weapon.
**Do:** give **each part type its own material section**. In `gen_texture_set`, add
per-part variants so Carcass, Engine, Weapon, Cargo, Sensor, Reactor each get a
distinct base accent + grime weight. This alone makes ships read as assembled
machines instead of one texture.

### Phase 2 — Runtime skin system (the X4 hallmark) — data/mask, not bake
**Do:** build the Adastrea equivalent of `paintmodmask`:
1. Generate a **vertex-color mask** (or a UV2 mask texture) on each ship part marking
   which regions accept a skin (hull panels) vs. which stay fixed (glow strips,
   hazard, glass → masked out).
2. Define a camera of **SKINS as data** (JSON): `{id, baseHue, baseBright, saturation,
   accentRGB, pattern/strength/scale, dirt, metal, smooth}` — mirroring X4's
   `hue/brightness/saturation/metal` recipe.
3. The UE material tint the base texture by the skin recipe **at runtime**
   (UE: a `Colorize`/`HueShift` on the albedo, gated by the mask).
=> Result: **any skin on any ship for free** (base texture untouched), exactly like X4.

### Phase 3 — AO / dirt / streak overlay (the "realism" jump)
**Do:** bake an AO per part → derive a `dirt` overlay, and add a **streak UV channel
(uv3-equivalent)** carrying grime/streaks/scratches as a second overlay that reads
above the base. This is the single biggest "it looks worn/real" win and X4's trick.

### Phase 4 — Material quality (shader knobs)
**Do:** per-part control of `roughness`, `metallic`, `aniso`, `smoothness`, and a
proper **specular/reflection** on metallic plating — currently only roughness+metal
are used. Add an anisotropy pass on long panels so reflections break realistically.

### Phase 5 — Pain/Skin gameplay loop (if the goal is the X4 *feel*)
**Do:** a small economy loop: ships earn/own skins, skinnable at a dock "Redesign
bay", with a quality tier + reputation gate. (Asset side can ship the first few
skins to prove the system.)

---

## 4. Concrete "next level" build order (what I'd start now)

1. **Per-part texture sections** (Phase 1) — quick, no new systems, big contrast win.
2. **Vertex-color / UV2 skin mask** on the assembled ships (Phase 2a) — establishes
   the extensible skin hook.
3. **First 4-6 skins as data** (Phase 2b) — prove any-skin-on-any-ship.
4. **AO + dirt streak UV3 overlay** (Phase 3) — realism jump.
5. Then knobs (4) and the gameplay loop (5) opportunistically.

Each phase is independently shippable and reverts cleanly. All stay within the
existing asset pipeline (procedural texture gen in `generate_adastrea_assets.py`,
per-part FBX, UE material builder).

---

## 5. Honest limits (what we can't fully match cheaply)
- **True runtime skin blending** needs UE material work (a `SkinColorize` on albedo
  gated by mask + a skin data asset). That's a main/gameplay-side hook; asset side
  delivers the **masks + skin data + textures** so the game can drive it.
- **XTex-grade 4K/2× upscales** of every texture need heavy assets/size — not worth
  it on this iGPU; keep 2048 (ships) / 1024-512 (props, FX) as now.
- **True aniso + complex shader** need UE 5.8 material features; we can bake the look
  into textures (fake anisotropic streak normals) as a budget alternative.

*Plan written 2026-08-30. Companion to `MODULE_BUILDER_DATA_CONTRACT.md` and the
rest of `11-TECHNICAL_SPECS`.*