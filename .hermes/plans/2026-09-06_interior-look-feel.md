# Adastrea Interior Look & Feel Plan

> **For Hermes:** Use subagent-driven-development to implement this plan task-by-task.

**Goal:** Make the ship interior read as a designed, cohesive, fully-textured room — closing the visual gaps between mesh panels (where you can see out into space), applying the full material kit, and lighting it so surfaces have depth.

**Architecture:** The interior is a set of `SM_Int_*` meshes (shell + companion parts) mounted per family (Fighter/CommandBridge/CrewQuarters/Hab) inside `ASpaceshipInterior`, normalized to a ~650-unit room, with the `M_Int_*` material kit applied by slot. The problems to fix are (1) **gaps** — the shell/compart meshes are open at panel seams so you see space through them, and (2) **missing textures** — the `M_Int_*` materials exist but have no authored `T_Int_*` texture maps, so they render flat.

**Tech Stack:** UE 5.8 (C++ `ASpaceshipInterior`, UStaticMeshComponent, UMaterialInterface), editor-Python one-shot material/texture importers (pattern from `ue_fleet_materials.py` / `ue_fleet_materials` skill), Blender headless texture generation (assets lane), AHUD canvas (screen-space only — not for 3D).

---

## Current context / assumptions

- **What exists (verified):** `ASpaceshipInterior::ConfigureInterior` mounts the shell + companion parts per family and applies `M_Int_*` materials by slot; `RevealInterior()` unhides them; avatar is confined to the room and walks first-person (`SetFirstPersonView`).
- **Gaps problem:** The shell meshes (`SM_Int_*_Shell`, `SM_Int_*_Deck`) are **open** at panel seams and in un-authored open sections — from inside you can see the starfield/ship hull between panels. There may also be **gaps between the shell and companion part meshes** (parts mounted at same scale but not perfectly nested).
- **Missing-texture problem:** `M_Int_*` materials are **flat** — there are **no interior `T_*` textures** (only ship-hull `T_*` were enriched). The interior materials therefore have no paneling/seams/grime texture to read.
- **Owner split:** C++/HUD/lighting/interior wiring = **main** (me). Mesh + texture/material **authoring** = **assets** agent (Blender headless + UE importers). Plan assumes both lanes.
- **Constraints (from prior work):** Lumen is OFF on the iGPU → emissive self-glows but won't cast light on nearby walls; use emissive for panel accents. UE-native material authoring (not Blender-baked emission). Kill editor before C++ builds. UMG doesn't render in this project's PIE → canvas HUD only. Keep `pytest` green.

---

## Proposed approach

1. **Close the gaps (main-side mesh/config wiring + assets mesh fix).** Diagnose whether each gap is (a) an authored open section in the shell mesh (assets fixes by adding backing/infill), (b) a shell/part seam misalignment (main fixes scale/position), or (c) the walk-volume extending past the shell (main clamps). Provide a "see-out detector" (camera + trace) to find gaps systematically.
2. **Give the interior real textures (assets lane + main applies).** Author `T_Int_*` texture maps (D/N/R: panel seams, grime, rivets — the retro Alien-style aesthetic in the notes) and rebuild `M_Int_*` from them (same pattern as the successful `M_*_Hull` enrichment). Add emissive `M_Int_Lights`/`M_Int_Console` glow so panels pop without Lumen.
3. **Light the room (main-side C++).** Add an interior point/ambient light on entry so flat surfaces get depth and the emissive accents read; since there's no planet, keep gravity-0 but add a subtle light source in the room center/ceiling.
4. **Verify each interior family** in PIE — gaps closed, textures visible, lit, no see-through.

---

## Step-by-step plan (bite-sized)

### Task 1: Gap diagnostic harness (main)
**Objective:** Find exactly where you can see out between the mesh panels for the Fighter cockpit and CommandBridge shell.
**Files:**
- Create: `Plugins/AdastreaShips/probe_interior_gaps.py`
- Modify: none (probe only)

**Step 1:** Spawn the interior, enter first-person in PIE, and for a grid of view directions, cast a short ray/trace from the avatar eye; log directions where the trace exits the interior volume (no mesh hit within the room's radius).
**Step 2:** Screenshot the confirmed see-out directions from inside; `vision_analyze` to classify each gap as *authored open section* vs *shell/part seam* vs *camera beyond shell*.
**Expected:** A list of gap directions + classification.

### Task 2: Close authored-voids in the shell meshes (assets)
**Objective:** For shell open sections that are genuine authored voids, add infill/backing so you can't see space.
**Files (assets lane):**
- Modify: Blender ship-interior generator (`generate_adastrea_assets.py` interior functions / `fbx2obj.py`)
- Regenerate + re-import `SM_Int_*_Shell`, `SM_Int_*_Deck`

**Step 1:** In the Blender interior generator, add a closed backing infill behind exterior-facing apertures (or thicken thin shell walls) so the hull reads solid from inside.
**Step 2:** Re-run the mesh import (`ue_import_all_meshes.py`) → commit meshes.

### Task 3: Nested shell/part alignment (main)
**Objective:** Ensure companion parts (Console/Deck/Lights/Stations) sit nested within the shell with no see-through seam.
**Files:**
- Modify: `Source/Adastrea/Private/Ships/SpaceshipInterior.cpp` (`MountInteriorPart`), possibly `SpaceStation`-style placement
- Test: `tests/test_interiors_walk_scales.py` + `tests/test_all_interiors.py`

**Step 1:** Verify part meshes mount at the same origin/scale as the shell (they use `SetRelativeScale3D(Scale3D)` + attach to `SceneRoot` — confirm no offset).
**Step 2:** In PIE, for each family, capture the interior and check part-seams for see-through; adjust part relative offsets if needed.

### Task 4: Author `T_Int_*` interior textures (assets)
**Objective:** Give the interior materials real paneling/grime/seam texture.
**Files (assets lane):**
- Create: Blender texture-gen `T_Int_Shell` etc. (D/N/R maps: panel seams, grime streaks, rivets, hazard stripes)
- Rebuild: `M_Int_Shell`, `M_Int_Deck`, `M_Int_Console`, `M_Int_Lights`, `M_Int_Vents`, `M_Int_Bunks`, ... via the `ue_fleet_materials`-style importer

**Step 1:** Author the texture maps in Blender (darkened shell with paneling + cyan console emissive, per the Alien/retro-sci-fi aesthetic in the notes).
**Step 2:** Rebuild + re-import the `M_Int_*` materials from them (5-channel D/N/R/M/E, mirroring the successful `M_*_Hull` enrichment). Commit meshes/textures/materials.

### Task 5: Light the interior (main)
**Objective:** Add interior lighting so walls/emissive read with depth; stop the "flat void" even if materials are textured.
**Files:**
- Modify: `Source/Adastrea/Private/Ships/SpaceshipInterior.cpp` (+ header), or `AdastreaPlayerController.cpp` on enter
- Test: PIE screenshot + vision; `pytest` green

**Step 1:** On `RevealInterior()`/`EnterShipInterior`, spawn or enable an interior point light (or use a lightweight `UStaticLightComponent`-style emissive plane) at the room centre/ceiling, additive so it doesn't blow out the emissive seams.
**Step 2:** PIE-verify the fighter cockpit is lit and panel seams read.

### Task 6: Full-family verification (main)
**Objective:** Confirm the look/feel across Fighter, CommandBridge, CrewQuarters, Hab.
**Files:**
- Test: PIE screenshot per family + `vision_analyze`; `pytest` (walk-scale + all-interiors suites stay green)

**Step 1:** Enter each interior family, capture, verify: no see-through gaps, material texture visible, lit, avatites-visible detail.
**Step 2:** Any residual gaps/see-through → loop back to Task 2/3.

---

## Files likely to change

- `Source/Adastrea/Private/Ships/SpaceshipInterior.cpp` (+ `.h`) — gap seal, alignment, lighting
- `Source/Adastrea/Private/Player/AdastreaPlayerController.cpp` — interior lighting on enter
- `tests/test_interiors_walk_scales.py`, `tests/test_all_interiors.py` — keep green
- `Plugins/AdastreaShips/Content/Materials/Interiors/M_Int_*` — rebuilt with textures (assets)
- `Plugins/AdastreaShips/Content/Meshes/Interiors/SM_Int_*` — infilled shells (assets)
- Blender generator + UE importers (`generate_adastrea_assets.py`, `ue_fleet_materials.py`-style) (assets)

## Tests / validation

- `python -m pytest` — full suite stays green (currently 127 passed)
- `tests/test_interiors_walk_scales.py`, `tests/test_all_interiors.py` — interior suites
- **PIE per family:** screenshot + `vision_analyze` asserting: no see-through gaps, textured surfaces, lit room, emissive accents visible

## Risks / tradeoffs / open questions

- **Risks:** Gap-classification ambiguity (authored-void vs seam vs camera); texture authoring time; lighting perf on iGPU (already ~3 FPS — an interior light is cheap, but additive glow on every panel may cost).
- **Tradeoff:** Infilling the shell vs matching mesh count in `test_all_interiors.py` — if assets adds new meshes, extend the expected set.
- **Open questions:**
  1. Should the interior be **fully sealed** (never see space) or is a small smoked "viewport" desired for the cockpit (windows are good in a cockpit, just not the current open gaps)? — **recommend: sealed shell walls, keep a small emissive viewport/hatch where intended.**
  2. Who authorizes the `T_Int_*` texture art direction (shared retro-sci-fi palette locked with the `M_*_Hull` look)?
  3. Lighting: one central room light, or per-panel emissive strips (emissive cheaper on iGPU)?
  4. Priority: fix gaps first (funnest aesthetic bug) then textures/lighting for polish?

---

*Plan saved. No execution performed (plan mode).*