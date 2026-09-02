# Sci-Fi Texture & Palette Study

Research compiled 2026-09-02 from public sources: free PBR texture hosts
(3dtextures.me, freepbr.com, aitextured, lotpixel, renderhub, textures.com) and
sci-fi color-palette references (Shutterstock sci-fi palettes, r/worldbuilding,
warhammerguild, concept-art blogs). Two real 3dtextures.me sci-fi panel textures
were downloaded to `ref_textures/` (CC0) and analyzed to derive the lessons below.

> **Purpose:** inform the Adastrea procedural material/texture pipeline — what
> real sci-fi hard-surface albedo/color actually does, so our `gen_texture_set`
> output reads as believably manufactured rather than procedurally flat.

## Two studied textures (real world, CC0)
| Texture | Albedo mean | Colors | Character |
|---|---|---|---|
| `sci_fi_metal_plate_005` | 142.7 | 24K | clean grey metal grid, deep bevels, rivets, slatted vents, no grime |
| `sci_fi_panel_010` | 184.4 | 92K | **lived-in blue** painted panels, emissive slots, subtle grime/wear |

## Texture-composition lessons (what real sci-fi panels DO)

1. **Depth/relief is the #1 realism driver.** Real panels have *significant height
   variation*: raised grid lines, deeply recessed inner panels, raised internal
   slats/structures. Lighting+shadow turns this into visible 3D form. Flat grids
   read as procedural/sticker. → Bake strong height contrast into `_D`/`_N`.
2. **Bevels/chamfers everywhere.** Blunt the corners of every raised element. Sharp
   90° edges read low-poly/artificial; subtle chamfers make light catch the edges
   and scream "manufactured."
3. **Functional detailing sells believability:** rivets/bolts (implying assembly),
   slatted vents (implying cooling/flow), access panels. Purpose-grounded geometry.
4. **Modular repetition + interlocking seams** (staggered/offset rows) reads as mass
   production / cultural artistry — avoid a perfect checkerboard.
5. **Material contrast:** a painted base color with unpainted/metallic recessed
   areas (slots/cut-outs) creates interest and layered materiality — distinguish
   `_M` between painted panel vs bare-metal recess.
6. **Wear/grime tells a story** (panel 010): subtle smoke/streaks in crevices +
   faint vertical runoff + paint wearing thin on edges = "used," not pristine. This
   is what separates believable from clean-test-chart. (Phases 1-4 lay groundwork;
   denser directional grime = closer to these.)

## Color-palette learnings (from the palette references)

- **Common faction schemes in sci-fi (design-language):**
  - *Earth=blue / Mars=red* (BA4 ship-design palette) — faction functional colors.
  - *Black+orange, black+cyan, black+purple, black+green* — the pervasive hard-edge
    "darker = gothic/heroic" schemes (r/worldbuilding, the miniatures painting guide).
  - *Alien/natural = purple+green / teal+pink / orange+cyan* (warhammerguild) —
    deliberately non-natural combos for alien factions.
- **Alien (Nostromo) interior palette (Shutterstock's Alien profile):** clinical
  white medical bay + black/grey corridors + **neon green + teal accents** hinting
  at the alien. → directly matches our `Alien_Wall` (dark gunmetal + cool cyan +
  amber) direction; the teal/green accent is the key Alien cue.
- **Per-faction accent identity is the strongest readability lever** — a ship's
  faction palette (not its silhouette alone) is what players key on. Supports our
  per-class identity work (Argon blue, Teladi green, Split red, Paranid purple).

## Concrete generator takeaways
- Increase `_D`/`_N` **depth contrast** (deeper recesses, stronger bevel gradient).
- Add **interlocking/staggered** panel seams, not a uniform checkerboard.
- Strengthen **functional detail** (rivets + slatted vents + access panels) — we have
  the pieces; up their density/consistency.
- Add **material contrast** in `_M` (painted panel vs bare-metal recess) — partially
  present; push the recess bare-metal harder.
- Reaffirm **directional grime** (Phase-3) as the wear/story layer.
- Faction **accent palettes** are the highest-value identity lever (keep per-class).

## Ref files (local reference, CC0)
- `ref_textures/sci_fi_metal_plate_005.png` — clean grey grid with rivets/vents
- `ref_textures/sci_fi_panel_010.png` — lived-in blue painted w/ emissive + grime
Both CC0 (3dtextures.me); used for study only.