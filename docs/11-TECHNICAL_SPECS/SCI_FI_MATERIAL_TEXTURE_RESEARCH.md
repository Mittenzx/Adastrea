# Sci-Fi Materials & Textures — Research File
# Sources: real YouTube tutorials, GitHub repos, and community guides gathered for
# building a sci-fi material/texture pipeline (validated against our Adastrea setup).

> Status: RESEARCH COMPILATION (live-verified sources + links). Everything below is
> actionable for next steps in `generate_adastrea_assets.py` / UE material work.

---

## 1. Best-practice principles (from guides + industry)

1. **Albaluz defines color, don't bake lighting into it.** Albedo should be flat
   material color; all shading comes from Normal/Roughness/lighting. Keep
   *"physically plausible"* — avoid crushed shadows / blown highlights, keep metals
   either metal or not (PBR-metallic: black albedo = metal, value-driven).
   *(Sunstrike, aitextured, Medium GameTextures "Metallic Magic".)*

2. **Roughness does most of the work** for realism ("metallic handles specularity,
   roughness handles most of the look"). Tune roughness **in context** — check in
   multiple lighting setups (day/night), not just a studio light.
   *(Epic UE4 phys-based docs, Sunstrike.)*

3. **Metalness is black/white (0 or 1)** — no in-between unless a breakable edge.
   *(WoLD UE5 material guide.)*

4. **Pack maps / mask for control**: a packed **ORM** (Occlusion-Roughness-Metal)
   or per-channel masks for roughness/metallic/emissive keeps materials reusable.
   *(aitextured sci-fi, Substance→UE5 export gives Color/Normal/ORM/Emissive.)*

5. **Parametrize + use Material Instances** for batch color/paint variants — don't
   rebuild a material per variant. *(UhiyamaLab UE5 material-pins + instances.)*

6. **Emissive is the "pop" lever when GI is off.** For Lumen-disabled (iGPU) or
   unlit scenes, emissive accents (seams, windows, hazard) read as lit even without
   bounce — matches main's feedback that emissive should be used when Lumen is off.

7. **Heightmap-first authoring.** Author a height/pattern map, then derive
   normal/roughness/AO from it — one source of truth, consistent micro-detail.
   *(80.lv Substance sci-fi workflow: "plug a heightmap → normal/roughness/AO.")*

---

## 2. Key techniques (mechanical, copy-adaptable)

- **Directional bevels** — random repeated shapes w/ a bevel filter → looks
  machined/manmade, breaks uniformity. Rotate/scale bevel for variety.
  *(80.lv Matthias Schmidt.)*
- **Symmetry & mirroring** — mirror nodes break repetition + get more mileage from
  trim sheets/atlases (two mirrored sides = one atlas row). Great for our mirrored
  ship kitbash.
- **Smart Materials** (procedural PBR, no UV) — layers stack dirt/edge-wear/emissive
  that *react to geometry* (UV/shape-aware). The "no UV needed" variant is what
  Blender-side smart_uv world-aligned triplanar already approximates.
  *(Polycount Smart Material Painter; Reddit; Blender Artists.)*
- **Normal from luminance/sobel** — Sobel gradient of a height/greyscale source →
  normal (Z=1, strength-scaled). This is EXACTLY our `_N` builder.
  *(ComfyUI_Blender_toolbox SimplePBRGenerator: sobel_x/sobel_y + normal_strength.)*
- **Grayscale→roughness, heuristic metallic** — dark=rough heuristics are a fine
  starting point but real pipelines use authoring/SVBRDF estimation.
  *(ComfyUI_Blender_toolbox, Freepbrtextures channel notes.)*
- **Tileability (seamless)**: edge-blend + circular-charn for true seamless; use a
  high-segment UV sphere for sky domes (not low-poly) + bilinear filter on stars.
  *(ComfyUI toolbox; our space-asset-pipeline skill.)*

---

## 3. Modern AI PBR pipelines (optional, powerful)

- **Ubisoft La Forge CHORD** (`github.com/ubisoft/ubisoft-laforge-chord`,
  arXiv 2509.09952, HF `Ubisoft/ubisoft-laforge-chord`) — a single texture image →
  Base Color / Normal / **Height** / Roughness / Metalness maps via *chain-of-
  rendering-decomposition*; plus a 2x/4x PBR-map upscaler. ComfyUI node
  `ubisoft/ComfyUI-Chord`. License-gated (agree on HF). Great to estimate PBR maps
  from an authored albedo to bootstrap roughness/normal/height.
- **ComfyUI-Blender-Toolbox** (`GeekatplayStudio/ComfyUI_Blender_toolbox`) — PBR
  extraction, seamless tiling, Blender bridge, round-trip mesh/UV→texture→back.
  Includes `nodes/pbr_nodes.py` SimplePBRGenerator (heuristic sobel-normal,
  grayscale-roughness) we can subclass/hard to our generator.

---

## 4. YouTube tutorials (watch for technique)

- **"UE5 | Sci-fi Material in Unreal Engine 5.2 Tutorial"** — download files,
  material setup for sci-fi in UE5. (On-topic, high value.)
- **"Making a Procedural Sci-Fi Material in Blender"** — Blender node approach.
- **"AI-Generated Seamless PBR Textures for Unreal (Free Workflow)"** (Beyond the
  Viewport) — 3-step Unreal→ComfyUI→Unreal texture workflow.
- **"Blender Export FBX with Textures to Unreal 5/Unity"** (Markom3D) — FBX+texture
  round-trip (relevant to our emissive-lost-on-FBX problem).
- **"Make Textures for your Game at LIGHTSPEED"** — Blender+Substance3D+UE.
- Nicer sci-fi tutorials: **"Modelling a Sci-Fi Apartment in Blender+Substance+UE"**
  (Nico Linde), **"Best Method for Sci-Fi Buildings? Modelling vs Procedural"**
  (r/blender) — combine curated hand areas + procedural greebles.

---

## 5. Mapping to Adastrea (opportunities)

**What we already do right:** heightfield→normal/roughness/AO (principle 7) ✓, Sobel
normal ✓, emissive seams for Lumen-off pop ✓, mirrored kitbash ✓, ORM packed-ish
(D/N/R/M/AO/E separate) ✓.

**Ports worth adopting:**
1. **Roughness-tune in context** — add a per-class lighting-check step (already have
   per-class rough knobs; formalize a "check day/night" QA).
2. **Albedo keep flat / value-range check** — add a QA that flags albedo crushed
   blacks / blown whites (best-practice 1).
3. **Material Instances for skins** — we already ship `SKIN` masks + skins.json;
   ensure the UE M_*_Hull uses a parameterized base so skins = instance tints
   (matches principle 5 + our skin system).
4. **CHORD optional pass** — for a "realistic-substance" variant, feed an authored
   albedo through CHORD to derive trueheight/roughness instead of heuristics
   (elevates `_R`/`_M` fidelity). Deferred — needs HF + model download.
5. **Directional bevels node** — adapt the "random repeated bevel" trick to our
   plate/cladding pass for more manmade variety.
6. **Symmetry-slice for trim sheets** — reuse mirrored symmetry to stretch our
   texture sets across bigger atlases.

**Full pipeline recommendation (industry-standard, matches us):**
`Author height/pattern → derive D/N/R/AO/E → ORM-pack → parametrize → Material
Instances for variants → home-make emissive (Lumen-off) → QA in context (day/night)
→ multi-LOD.` Optional AI boost: CHORD to synthesize roughness/height from albedo.

---

## 6. Sources (live)
| Type | Source | Link |
|---|---|---|
| Repo | Ubisoft CHORD | https://github.com/ubisoft/ubisoft-laforge-chord |
| Repo | ComfyUI-Blender-Toolbox | https://github.com/GeekatplayStudio/ComfyUI_Blender_toolbox |
| Guide | Sci-Fi Hard-Surface Substance | https://80.lv/articles/sci-fi-hard-surface-workflow-and-custom-tools-for-substance-designer |
| Guide | PBR maps explained (ORM) | https://aitextured.com/articles/pbr_texture_maps_explained_albedo_normal_roughness_metallic_orm.html |
| Guide | UE5 material crash course | https://www.worldofleveldesign.com/categories/ue5/standard-material-beginners-guide.php |
| Guide | Game-texturing PBR | https://sunstrikestudios.com/en/blog/texturing_3d_models_for_games/ |
| Forum | Blender Artists sci-fi panels | https://blenderartists.org/t/procedural-sci-fi-panels-material-blender-tutorial/1397219 |
| Forum | r/blender smart materials | https://www.reddit.com/r/blender/comments/yaq4yu/scifi_crate_with_smart_material_fully_done_in/ |
| Forum | r/Substance3D sci-fi panels | https://www.reddit.com/r/Substance3D/comments/15xe3zr/scifi_panels_fully_procedural_material/ |
| YT | UE5 sci-fi material tutorial | https://www.youtube.com/results?search_query=sci-fi+material+texture+pipeline+blender+unreal+PBR |