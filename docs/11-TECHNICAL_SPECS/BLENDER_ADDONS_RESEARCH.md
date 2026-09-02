# Blender Add-ons & Tools — Curated for Adastrea (hard-surface space ships)

> Research compiled 2026-09-02. Sources: Superhive/Blender Market, Gumroad, BlenderKit,
> Blender Extensions, StraySpark, CG Cookie, KitBash3D, CGChannel. Prices are approximate
> and change — always confirm on the store page. Blender 5.2 is our version.

## The add-on marketplaces (where to shop)
| Marketplace | What it is | Notes |
|---|---|---|
| **Superhive (formerly Blender Market)** | The #1 paid Blender add-on store | Home of Hard Ops/Boxcutter, KIT OPS, Meshmachine. Huge sci-fi asset / add-on library |
| **Gumroad** | Indie creators, early/cheaper builds | Many add-on devs sell here first (Random Flow, Layer Painter, etc.) |
| **ArtStation Marketplace** | Pro assets, some Bleblender tools, decal packs | Good for kitbash/decal content |
| **Blender Extensions Platform** | The official central store (in-Blender, 4.2+) | Free + paid; where the free "enable built-in" add-ons and community add-ons live. Also has many free alternatives |
| **BlenderKit** | Community asset/add-on library | Thousands of free textures + some add-ons; procedural sci-fi materials |

---

## Tier 1 — Hard-surface modeling core (the big 3, highly recommended)

### 1. Hard Ops + Boxcutter (bundle, ~US$38)
The de-facto hard-surface suite for Blender. Boxcutter = fast interactive boolean cutting in the
viewport; Hard Ops = the boolean/modifier/bevel/cleanup manager around it. Non-destructive, undoable,
purpose-built for vehicles, weapons, props, sci-fi.
**Why for us:** our ships are pure hard-surface. Boxcutter + Hard Ops is the fastest way to carve
real hull geometry (the thing that's been hard in procedural-only). **Best buy.**
- Store: Superhive / Gumroad (masterXeon1001) / ArtStation.

### 2. DECALmachine (~US$25)
Projects **mesh-based decals** (panel gaps, bolts, vents, grates, stickers) with normal+AO baked in,
flat to the surface, no geometry and no UV unwrapping needed. AAA surface detail density fast.
- **Why for us:** exactly the "detail / rivets / panel lines" we've been fighting to add procedurally.
  Stamps believable greeble detail onto hulls in seconds. Pairs with KIT OPS.

### 3. MESHmachine (~US$35)
Advanced mesh cleanup, mirror, bevel, and "float/chamfer" tools; makes clean shading + hard-surface
transitions look professional. **Why:** when clean shading on assembled modular ships starts to matter.

**Bundle tip:** Hard Ops + Boxcutter + DECALmachine (+ MESHmachine) = the classic "portfolio-ready
hard-surface" stack (StraySpark calls exactly this combo the route to professional hard-surface).

---

## Tier 2 — Detail / Kitbash / Greeble (surface busy-work, our biggest timesaver)

### 4. KIT OPS (free / PRO)
Kitbashing system of pre-made **INSERTs** (ports, vents, mechanical bits) that snap onto your mesh as
booleans or standalone parts — non-destructive. Free version places/cuts basic inserts; PRO unlocks
the INSERT factory, favourites, PNG-to-decal, bulk toolkit.
- **Why for us:** instant greeble/kitbash hardpoints — the X4 modular look without hand-modeling every
  plate. Free tier is worth installing today.
- Free packs available (e.g. Andrew Averkin 500-insert kitbash).

### 5. Random Flow (BlenderGuppy, ~US$11)
Random **greeble + paneling generator** — feed it a surface, get tech-y panel breakups, raised plates,
greeble fields. Reddit: "way faster than hard ops/boxcutter/kitbashing for adding nice greebles."
- **Why for us:** best pure "make a blank hull look busy/scifi" tool; complements our procedural texture
  approach. Cheapest high-value pick.

### 6. Cablerator (part of MACHIN3/paid)
Adds believable cable/conduit/pipe runs. **Why:** exposed pipe runs = the Alien interior look + hull detail.

---

## Tier 3 — Material / Texturing / UV
### 7. Layer Painter (~US$30)
Photoshop-style PBR texturing in Blender: layers, masks, folders, blend modes. Stacks procedural +
painted layers. **Why:** our texture approach is code-driven, but Layer Painter lets you hand-grade the
signature real / worn / Alien look.

### 8. UV Packmaster 3 (~US$25) + BAM Tool (free) + Texel Density
UV-packing/textel-density tools. **Why:** we hit texel-density/consistency issues; these normalize UV
density cleanly.

### 9. Mask Tools
Procedural masks to mix PBR sets. Useful for the skin-mask / paint-weathered look.

---

## Tier 4 — Rendering / Lighting / Assets
### 10. Physical Starlight & Atmosphere (~US$25)
Starfield/planet/exposure atmosphere. **Why:** our ships render against dark void; this sells in-scene
presentations.

### 11. BlenderKit procedural sci-fi materials + KitBash3D packs
Thousands of free materials (incl. procedural sci-fi greeble), plus 3d kitbash packs. **Why:** instant
texture/material start points + hard-surface kitbash assets.

---

## Built-in / Free (enable in Preferences → Add-ons)
- **Bool Tool** — free boolean helper.
- **AutoMirror** — symmetry modeling (mirrors our procedural X-mirror pass).
- **Loop Tools** — topology utilities (bridges, relax).
- **Node Wrangler** — shader node power-use (free, core).
- **BlenderKit** (free tier) — material/asset library.

---

## Recommended purchase order for our project
1. **KIT OPS (free)** — zero cost, instant greeble/hardpoints today.
2. **Random Flow (~$11)** — cheapest big win for hull detail density.
3. **Hard Ops + Boxcutter bundle (~$38)** — the core carving workflow.
4. **DECALmachine (~$25)** — rivets/panels/decals without modeling.
5. Then MESHmachine / Cablerator / Layer Painter as polish phases.

## My honest recommendation
For the **procedural generator** we run in headless Blender, the biggest payoff is **Random Flow + KIT OPS**
(hand-authored greeble detail is exactly what procedural primitives struggle to match). For **hand-built**
ships/cockpits (the direction the user chose), **Hard Ops/Boxcutter + DECALmachine** are the professional
tools. Note: most of these are **GUI/interactive tools** — they help when authoring interactively in the
Blender editor, not in our headless `-b --python` pipeline (a real limitation to flag honestly).