# Ship UV / Texture Composition — Reference Analysis & Fix Plan

**Goal:** why our ships' exterior textures look asymmetric/inconsistent, and how
composed sci-fi (X4-style) ships avoid it — with a concrete fix.

## 1. Diagnosis (measured, not guessed)

Our pipeline unwraps with `bpy.ops.uv.smart_project(scale_to_bounds=True)` **per mesh
object**. Every part measured `uv_span = [1.0, 1.0]` → each part fills the full 0–1
tile regardless of its real size. Consequences:

- **Texel density is not normalized.** A 340cm hull and a 40cm engine both map the
  whole 2048px texture to full 0–1 → the engine shows the panel detail ~8× larger
  per cm than the hull. Texture reads at wildly different scales across the ship.
- **Each ~37 smart-projected islands per part gets an arbitrary scale + orientation.**
  The same texture's grooves/rivets/windows point every which way → "noisy, no
  horizontal/vertical consistency," looks asymmetric even on symmetric planes.
- **Mirrored parts diverge.** Left & right engine nacelles smart-project
  independently → the panel/seam detail does not align across the X=0 mirror, so
  the ship reads non-symmetric even though the *geometry* is.

## 2. How composed X4-style ships avoid this (researched)

Findings from pro hard-surface ship art sources (PulseGeek UV guide, BeyondExtent
trimsheets, r/gamedev world-aligned workflow, Egosoft ship-moduring docs):

1. **Consistent texel density is the #1 rule.** Normalize UV scale so N px →
   N cm everywhere (they normalize to a target e.g. 2048px/1m, then intentionally
   scale hero faces up, hidden faces down). "Keeping it consistent prevents
   mismatched sharpness between parts." → we don't do this at all (everything = 0–1).

2. **World-aligned / projection-based materials** (r/gamedev): texture is applied as a
   **world/object-aligned box projection** with a fixed scale-per-meter, so left/right
   and top/bottom are *identical by construction* — no per-island randomness. Perfect
   symmetry, tiny effort, ideal for tileable hull/panel textures.

3. **Trim sheets + shared UV** (BeyondExtent): one tiling material; UV islands may
   extend past 0–1 and overlap because the texture tiles in U/V — consistent across a
   whole fleet, not per-ship. Great for keeping detail density uniform.

4. **Careful orientation / straighten edges** (PulseGeek): align UV edges to texture
   axes "so stripes or trims run true;" mirror only broad shapes, un-mirror focal
   details. → our smart-project orientation is random.

5. **X4 specifics (Ego docs):** parts are split per-texture-set (each material section
   = one texture set), with **uv1** primary + **uv3** overlay channels, and material
   *placement* (skins/dirt) driven by vertex-color masks rather than texture.
   Per-part textures are authored so edge trims align, not random-projected.

**In short:** the fix is to unwrap shared/symmetric geometry with a **normalized,
oriented UV** (world-aligned box projection or a shared trim UV), not per-object
random smart-project.

## 3. Concrete fix plan (ranked)

**A. Normalize texel density (biggest win, cheapest).**
Replace per-object `smart_project` with an **object-axis-aligned box projection at a
fixed scale** (e.g. 1 texture tile per ~2m), so every part uses the same px/cm.
→ instantly fixes the inconsistent scale; mirrored parts align because left/right
get the same projection scale.

**B. Make mirrored parts share UV.**
Either (a) build the mirror as a real mirrored-UV (UV.x = 1−UV.x on the flip) so the
same texture reads symmetric, or (b) object-projection with the same scale so they
naturally align. (a) is more robust for exact symmetry.

**C. Orient islands to axes.**
After projection, straighten UV islands so panel/rivet texture runs horizontally/
vertically consistently instead of random.

**D. (Optional) trim-sheet path** — a single tiling trim texture for hull panels,
shared across all ships, at normalized density (BeyondExtent style). Most consistent,
slightly more setup.

**Recommended start:** A + B (normalized object-aligned projection + mirrored-UV
symmetry). Low risk, no engine change, directly fixes the visible asymmetry and
inconsistency. C is a nice polish on top.

## 4. What I'll do next if you approve
Implement A+B in `gen_texture_set`/`smart_uv` path: a `smart_proj` replacement that
does object-axis box projection at fixed px/cm + mirror-aware UV. Regenerate, convert,
QA, and studio-render to confirm the detail reads symmetric + consistent.