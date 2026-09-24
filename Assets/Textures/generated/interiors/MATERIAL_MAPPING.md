# Interior texture library: material mapping

Produced by `Tools/generate_interior_texture_library.py`. Test renders are made by
`Tools/render_interior_texture_tests.py` (Blender 5.2 headless, Cycles).
The library replaces the hull-recipe interior sets (`T_Int_Wall/Deck/Cockpit/Glow`, `T_Alien_*`),
which read as wallpaper at human scale. Every set here is authored in real millimetres
(seams 4-6 mm, rivets about 10 mm, buttons 40 mm, pipes 56-180 mm).

**Audience:** unreal-integrator (building the materials) and Roster (CommandXL/Battleship
interiors). Please use these sets and don't fork new ones. If you need a variant, ask Interiors.

## 1. Import settings (every set)

| Map | File | UE settings |
|---|---|---|
| BaseColor | `T_Int_<Set>_D.png` | sRGB ON, Default |
| Normal | `T_Int_<Set>_N.png` | **DirectX (green-down)**: TC_Normalmap, sRGB OFF, **Flip Green OFF** (same as the 88188d68 reimport). Flat = (128,128,255). |
| Roughness | `_R.png` | sRGB OFF, Masks (or Grayscale) |
| Metallic | `_M.png` | sRGB OFF |
| AO | `_AO.png` | sRGB OFF |
| Emissive | `_E.png` | sRGB ON (black where unlit) |
| Mask | `_MASK.png` (EngGrate, Glass only) | sRGB OFF |

The channels are separate maps, matching the existing `T_*_D/N/R/M/AO/E` pipeline (no ORM packing).
Suggested UE path: `/Game/Textures/Interiors/` (or `/AdastreaShips/Textures/Interiors/` for the ship kit).
All PNGs are in `Assets/Textures/generated/interiors/`. All tiles are seamless. Wall sets repeat
seamlessly in U, and their V is floor-anchored (see section 3).

## 2. Sets

| Set | Res | Tile (world) | Tiling on smart_uv meshes (1 UV = 200 cm) | Content |
|---|---|---|---|---|
| `ShipDeck` | 2048 | 200 cm | 1.0 | 1 m deck plates: diamond tread, anti-slip grit field with yellow border, flush access hatch, bolt rows |
| `ShipWall` | 2048 | 200 cm (floor-anchored) | 1.0 | Kick plate 0-30 cm, recessed bulkhead panel, teal ID stripe at 104-111 cm, vent/service hatch 150-200 cm, rivet rows |
| `ShipWallUpper` | 2048 | 200 cm | 1.0 | Seamless 1 m riveted panels, for wall area above 2 m and for machinery/furniture boxes |
| `ShipCeiling` | 2048 | 200 cm | 1.0 | Structural ribs every 1 m, perforated acoustic panels, 16 cm LED strip along U (`_E`) |
| `EngWall` | 2048 | 200 cm (floor-anchored) | 1.0 | Hazard band 0-16 cm, conduit bundle, junction box + LED, colour-coded pipes (gas yellow / coolant blue / copper / steel) 125-180 cm, oil streaks |
| `EngWallUpper` | 2048 | 200 cm | 1.0 | Seamless dark stiffened plating + vertical conduit pair |
| `EngGrate` | 2048 | 200 cm | 1.0 | Bar-grating floor (35.7 mm bearing bars, 100 mm cross bars, serrated), 1 m panels with frames; `_MASK` = opacity |
| `Hazard` | 1024 | 100 cm | 2.0 | 45 degree yellow/black trim, chipped and scratched |
| `Console` | 1024 | 100 cm | 2.0 (or 0-1 UV per console face) | Two bezelled screens with UI (`_E`), 8x3 key grid with lit keys, toggles + LEDs, sliders, screws |
| `HabWall` | 2048 | 200 cm (floor-anchored) | 1.0 | Wood-veneer wainscot 0-90 cm, brass rail 90-96 cm, diamond-tufted upholstery 96-200 cm |
| `HabFloor` | 2048 | 200 cm | 1.0 | 50 cm quarter-turn carpet tiles, warm umber |
| `Glass` | 1024 | 200 cm | 1.0 | Viewport glass: tint, smudges, fingerprints, dust, scratches; `_MASK` = dirt amount |
| `StnFloor` | 2048 | 200 cm | 1.0 | Polished terrazzo tiles 100x50 cm running bond, brass inlay line every 2 m |
| `StnWall` | 2048 | 200 cm (floor-anchored) | 1.0 | Warm off-white panels, kick base 0-15 cm, amber wayfinding band 100-106 cm with cove light (`_E`), speaker grille |
| `StnWallUpper` | 2048 | 200 cm | 1.0 | Seamless warm 1 m panels (no rivets) |
| `StnCeiling` | 2048 | 200 cm | 1.0 | 1 m coffers with warm-white 56 cm diffusers (`_E`), brass trim |

Texel density: 2048 px over 2 m = 10.24 px/cm (1K sets: 1024 px over 1 m = 10.24 px/cm; Glass: 5.1 px/cm).
The palette is shared: ship = cool gunmetal + teal ID, station = warm off-white/terrazzo + amber, and hazard yellow is common to both.

**Mirror note:** `smart_uv()` flips U on -X faces, so no set has readable text or handed arrows.
Everything is mirror-safe.

## 3. Master materials to build

1. **`M_IntSurface_Master`** (opaque). Params: the 6 texture slots, `Tiling` (scalar, default 1),
   `EmissiveStrength` (default 0; lit sets use 4-8), `UseWorldAligned` (static switch).
   - Mesh UV path: `TexCoord * Tiling`.
   - World-aligned path (for the station's scaled engine cubes, which have 0-1 UVs per face):
     WorldAlignedTexture with `TextureSize = TILE_cm / Tiling` (200 cm, or 100 cm for Hazard/Console).
     Use the WorldAlignedNormal node for `_N`.
   - AO goes to AmbientOcclusion (don't multiply it into BaseColor; the Blender test renders did that only because Cycles has no AO input).
2. **`M_IntSurface_Oriented`**: for the **single-slot rooms** (floor, walls and ceiling joined into one
   mesh: `SM_Int_Freighter_EngineRoom` = `M_Interior_Eng`, `SM_Int_Standard_Corridor` and
   `SM_Int_Standard_Airlock` = `M_Interior_Hab`, `SM_Int_Xenomorph_AlienHold` = `M_Interior_Alien`).
   Blend three texture sets by `VertexNormalWS.z`: > 0.7 is the floor set, < -0.7 is the ceiling set, otherwise the wall set.
   Blend with a hard lerp (0.05 transition) to avoid smearing. Walls: use `WallLower` below
   `FloorZ + 200 cm` and `WallUpper` above it. FloorZ can be a per-MI scalar or
   `ObjectBounds` min Z (ObjectPositionWS - ObjectBounds.z/2). Floor-anchored V = (WorldZ - FloorZ)/200.
3. **`M_IntGrate_Masked`**: Blend Mode Masked, `_MASK` to Opacity Mask (clip 0.5), Two Sided ON.
   Put a dark sub-floor (ShipDeck at 0.3 tiling, or plain black) about 30 cm below it, or keep it opaque:
   the void pixels are already painted near-black, so the opaque version reads fine at distance.
4. **`M_IntGlass_Translucent`** (see section 5).

## 4. Existing material to new set

### Ship kit: `/AdastreaShips/Materials/Interiors/M_Int_*` (slot names in `SpaceshipInterior.cpp::ApplyInteriorMaterials`)

| Material / slot | New set(s) | Tiling | Notes |
|---|---|---|---|
| `M_Int_Shell` | Oriented: floor `ShipDeck`, walls `ShipWall`/`ShipWallUpper`, ceiling `ShipCeiling` | 1.0 | Shell contains walls + ceiling, so use `M_IntSurface_Oriented`. Ceiling EmissiveStrength 5. |
| `M_Int_Deck` | `ShipDeck` | 1.0 | Bridge deck. |
| `M_Int_Console` | `Console` | 2.0 | EmissiveStrength 3-4. |
| `M_Int_Stations` | `Console` on up/forward faces, `ShipWallUpper` on the rest (oriented, z > 0.3 is Console) | 2.0 / 1.0 | Bridge workstations. |
| `M_Int_Lights` | `ShipCeiling` `_E` only, or a flat emissive (0.88,0.94,1.0) x 8 | 1.0 | The light fixtures are small, so a flat emissive is fine. |
| `M_Int_Vents` | `EngWallUpper` | 1.0 | Keep the old red accent via a 0.8 tint lerp on BaseColor if you want the signal colour. |
| `M_Int_Hatch` | `ShipWallUpper` + `Hazard` border (oriented not needed) | 1.0 / 2.0 | A simple version is `ShipWallUpper` tinted 0.6. |
| `M_Int_Viewport` | **`Glass`** (translucent, section 5) | 1.0 | Fixes "bridge viewport doesn't read as glass". |
| `M_Int_Bunks` | `HabWall` (upholstery rows: use V offset 0.5 so the tufting is on the bunk faces) | 1.0 | For a plainer frame, use `StnWallUpper`. |
| `M_Int_Desks` | `StnWallUpper` (tops), `Console` on any screen faces | 1.0 | |
| `M_Int_Galley` | `StnWallUpper` + `StnFloor` for counter tops | 1.0 | |
| `M_Int_Mess` | `HabWall` (wainscot band) / `HabFloor` | 1.0 | Warm social space. |

### Project content: `/Game/Materials/M_Interior_*` and the FBX slot names

| Material / slot | New set(s) | Tiling | Notes |
|---|---|---|---|
| `M_Interior_Deck` | `ShipDeck` | 1.0 | Replaces `T_Int_Deck`. |
| `M_Interior_Hab` (Corridor, Airlock, Generationship Hab) | Corridor/Airlock: oriented `ShipDeck` / `ShipWall`+`ShipWallUpper` / `ShipCeiling`. Generationship Hab: oriented `HabFloor` / `HabWall`+`StnWallUpper` / `StnCeiling` | 1.0 | Two MIs of `M_IntSurface_Oriented`: `MI_Interior_Corridor` and `MI_Interior_Hab`. The slot name stays `M_Interior_Hab`. Pick the MI per mesh. |
| **`M_Interior_Eng`** (no asset today, currently stand-in `M_Int_Shell`) | Oriented: floor `EngGrate` (opaque variant) or `ShipDeck`, walls `EngWall` + `EngWallUpper`, ceiling `ShipCeiling` | 1.0 | **Fixes the flat-grey engineering bay.** Create `/AdastreaShips/Materials/Interiors/M_Interior_Eng` and point the `M_Interior_Eng` entry in `ApplyInteriorMaterials` at it (Source change: integrator's call). |
| `M_Interior_Cockpit` | `Console` (upward faces) + `ShipWallUpper` | 2.0 / 1.0 | Currently mapped to `M_Int_Shell`. |
| `M_Interior_Accent` | `Hazard` | 2.0 | |
| `M_Interior_Alien` | Keep `T_Alien_*` for now (deliberately "old-school"), or oriented `EngGrate` / `EngWallUpper` tinted (0.55,0.6,0.5) | 1.0 | Low priority. |
| `M_Fighter_Glass` | `Glass` | 1.0 | Same translucent MI. |

### Walkable station interior (`AStationInterior`, commit efc96f1a)

`AStationInterior` currently builds every slab from `/Engine/BasicShapes/Cube` with a tinted
`BasicShapeMaterial` MID (`MakeTinted`, param `Color`). The cube's UVs are 0-1 per face and the cube is
non-uniformly scaled, so **use the world-aligned path** (`UseWorldAligned = true`, TextureSize 200 cm).
Suggested approach: give `AddBox` a material argument (or a name to material table) instead of `BaseMaterial`,
and keep `Color` as an optional tint param on the MI (`BaseColor * lerp(1, Color, TintAmount)`, TintAmount default 0).

| Room / boxes | New set | Notes |
|---|---|---|
| All rooms: `Floor` | Concourse `StnFloor`, Maintenance `ShipDeck`, Habitation `HabFloor` | |
| All rooms: `Ceiling` | Concourse/Hab `StnCeiling` (Emissive 5), Maintenance `ShipCeiling` (Emissive 6) | Rooms already have point lights; the emissive only needs to be read as the source. |
| All rooms: `WallN/S/E/W`, bulkheads, lintels, cabin fronts | Concourse `StnWall` below 200 cm + `StnWallUpper` above (H = 400 cm). Maintenance `EngWall` + `EngWallUpper` (H = 700 cm). Habitation `HabWall` + `StnWallUpper` (H = 350 cm). | A wall box spans the full room height, so the MI needs the lower/upper split by world Z (actor-local Z = 0 is the floor). |
| Concourse `PillarN/S_*` | `StnWallUpper` | |
| Concourse `CenterStrip`, `DoorTrimL/R` | Keep a tinted emissive (these are light/trim accents) | |
| Concourse `AirlockStripe`, Maintenance `CradleHazardL/R`, `FloorLane`, `GantryBeam` | `Hazard` (TextureSize 100 cm) | |
| Concourse `IslandBase` | `StnWallUpper`; `IslandPlanter` stays a tint | |
| Maintenance `CradleBase` | `EngGrate` opaque | |
| Maintenance `ShipHull/Nose/Wings/Engine` | Keep the hull materials (not interior); `ShipGlow` stays emissive | |
| Maintenance `GantryRail*`, `ToolRack_*` | `EngWallUpper` | |
| Maintenance `Crate_*` | existing `T_Prop_Crate` | |
| Habitation wood/fabric furniture | `HabWall` (wainscot band, V offset 0.1) for wood, `HabFloor` tinted for fabric | |
| `AStationTerminal` `Pedestal` / `Screen` | Pedestal `ShipWallUpper` (tint 0.5); Screen `Console` UV-mapped 0-1 or a flat emissive | |

## 5. Glass / viewport material (`M_IntGlass_Translucent`)

- Blend Mode **Translucent**, Lighting Mode **Surface TranslucencyVolume** (or Surface ForwardShading
  for sharp reflections on the bridge), Two Sided OFF, **Screen Space Reflections ON** (Lumen: enable
  "High Quality Translucency Reflections" in the project if affordable).
- BaseColor = `_D` (cool clear tint about 0.80/0.88/0.92).
- Metallic = 0, Specular = 0.5.
- Roughness = `lerp(0.03, 0.45, _MASK)`. `_R` already encodes this, so either works.
- **Opacity = `lerp(0.10, 0.55, _MASK)`**: clean glass stays near-invisible while smudges and dust catch light.
  This is what makes it read as glass rather than a black hole.
- Refraction: IOR 1.52 via Refraction Mode "Index of Refraction", or Pixel Normal Offset with strength 0.02 (subtle).
- Normal = `_N` at 0.3 intensity (flatten the scratches).
- Optional Fresnel: `Opacity += Fresnel(exponent 4) * 0.25` so grazing angles look reflective.
- Tiling 1.0 over the viewport (the texture is 2 m). For small portholes use 2.0.
- Place a faint rim-lit `_MASK`-driven emissive (x 0.02, colour (0.5,0.7,1.0)) if the viewport faces pure black space and
  still vanishes.

## 6. Parameter defaults per MI (EmissiveStrength)

`ShipCeiling` 5-6, `StnCeiling` 4-5, `StnWall` 2-3 (cove slot), `Console` 3-4, `EngWall` 2 (junction LED).
All others: 0.

## 7. Previews

- Per-set contact sheets: `previews/contact_<Set>.png` (lit 2x2 tiled check + every map + 1:1 crop).
- Whole library: `previews/contact_all.png`.
- Blender test renders at eye height: `previews/render_corridor.png`, `render_engbay.png`,
  `render_concourse.png`, `render_habcabin.png`, `render_bridge_viewport.png`.

Regenerate: `python Tools/generate_interior_texture_library.py [SetName ...]`. It is deterministic, takes about 6 min for all sets.
Re-render: `blender -b --python Tools/render_interior_texture_tests.py -- [corridor eng concourse hab bridge]`.
