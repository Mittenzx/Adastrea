# Adastrea Procedural 3D Asset Kit — Import & Setup

Realistic cyberpunk-sci-fi hard-surface kit, generated **procedurally/code-driven**
in Blender 5.2 headless and exported for Unreal Engine 5.8. All meshes pass
strict numeric QA (tri budget, non-degenerate faces, bounds, UV presence, symmetry).

## What's in the pack

```
Assets/FBX/generated/
├── Ships/
│   ├── SM_Ship_Fighter_01.fbx          # agile interceptor        (1,414 tris)
│   ├── SM_Ship_Freighter_01.fbx        # cargo hauler             (3,284 tris)
│   └── SM_Ship_Gunship_02.fbx          # angular smuggler gunship (2,652 tris)
├── Station/
│   └── SM_Station_Habitation_01.fbx    # vertical mega-hab + dock ring (3,142 tris)
├── Objects/                            # other space objects
│   ├── SM_Obj_Satellite_01.fbx         # comms satellite          (372 tris)
│   ├── SM_Obj_AsteroidDerelict_01.fbx  # wrecked asteroid + debris (160 tris)
│   ├── SM_Obj_FuelCell_01.fbx          # triple fuel tank         (648 tris)
│   └── SM_Obj_CommsTower_01.fbx        # relay tower + dishes     (930 tris)
└── Props/
    ├── SM_Prop_Crate_Container_01.fbx  # shipping crate           (216 tris)
    └── SM_Prop_Container_Hazard_02.fbx # hazardous tank           (952 tris)
```

Plus **54 tileable PBR textures** (`T_*_D/_N/_R/_M/_AO/_E`) in `Textures/`:
- `T_Ship_Hull`, `T_Freighter`, `T_Gunship`   (ships)
- `T_Station_Hab`  (lit window grid + neon cyan trim — the cyberpunk look)
- `T_Satellite`, `T_SolarPanel`, `T_Derelict`, `T_Prop_Crate`, `T_Prop_Tank`

And `previews/` — isometric matplotlib renders for eyeballing each mesh.

## Conventions baked in
| Property | Value |
|----------|-------|
| Scale | 1 Blender cm = 1 Unreal unit (real-world) |
| Orientation | Z-up, forward −Y (UE default) |
| Mesh naming | `SM_[Category]_[Object]_[Variant]` |
| Texture naming | `T_[Object]_[Map]_[Variant]` |
| Clean-up | coincident verts merged, degenerate faces removed at source |

## Importing into UE (repeat once, save the preset)
1. **Content Browser > Import** → select an FBX.
2. FBX import settings: **Import As Static Mesh**, **Scale 1.0** (no auto-scale),
   defaults for axis. **Enable = Mesh only.**
3. Mesh → **Collision → Add Auto Convex** (or box/sphere for props).
4. Create one `M_*` PBR material per set; connect: `_D`→BaseColor, `_N`→Normal,
   `_R`→Roughness, `_M`→Metallic, `_AO`→AO, `_E`→Emissive.
   - Enable Emissive on `_E` where you want neon/windows to glow.
   - **Phase 4 material knobs:** `_R` now carries **anisotropic grain** (directional
     low-freq striations, H/V-grad ~1.3) so long panels break reflections
     realistically — feed `_R` into a Roughness channel and, for best gold, drive a
     masked Anisotropy node from the same `_R` where grain is strong. `_M` carries
     painted-vs-bare-metal variation (raised seams bare-metal, recesses primer).
   - Per-class feel is baked into each texture set's `rough`/`metal` params (e.g.
     Argon polished 0.30/0.97, Miner heavy-duty 0.52/0.88, Teladi matte 0.45/0.92).
5. Add **2–3 LODs** (Mesh LODInfo) — the biggest win on an Intel Arc iGPU.

### Per-part material→texture pairing (Phase 1)
Each ship part now has its **own texture set** so parts read as distinct materials
(a cool carcass, hot engines, dark weapons, amber reactor). Map the FBX material to
its texture set like so:

| FBX material | Texture set | Palette |
|---|---|---|
| `M_Engine_*` / `M_Engine_Ion/Compact` | `T_Engine_*` | warm grey + hot orange glow |
| `M_Cargo_*` / `M_Cargo_Hold/Tank/Rack` | `T_Cargo_*` | olive + green container marker |
| `M_Weapon_*` / `M_Weapon_Block/TriLaser/Missile` | `T_Weapon_*` | dark + menacing red |
| `M_Sensor_*` / `M_Sensor_Block/Dome/Cross` | `T_Sensor_*` | light + cool cyan scan |
| `M_Reactor_*` / `M_Reactor_Block/Fusion/Spike` | `T_Reactor_*` | warm + amber core |
| `M_Drill` (has `M_Station_Hab` currently) | `T_Drill_*` | industrial yellow warning |
| `M_MiningLaser` (uses `M_Weapon_Block`) | `T_MiningLaser_*` | teal beam |
| `M_Canopy` | `T_Canopy_*` | dark glazed glass + lit viewport band (bridges/cockpits) |

> The Carcass/hull keeps `T_Ship_Hull` (or `T_Freighter` / `T_Gunship` for those
> ships); engaged, the FBX `M_*` name is the hook — create the UE material `M_Engine`
> from `T_Engine_*` and assign it to the part's material slot.

### Skin system (Phase 2 — runtime ship skins, X4-style)
Each texture set also ships a **skin mask**: `T_<name>_SKIN.png` (grayscale,
`1.0` = skinnable hull panel, `0.0` = fixed region that must NOT be recolored —
accent stripes / windows / neon / hazard). Skins are defined as **data** in
`Assets/FBX/generated/skins.json` (8 base skins, each a `hue/brightness/saturation`
recipe mirroring X4's paintmod recipe plus `metal`/`smooth`).

**UE material hook (to make skins work in-engine):**
1. In the ship hull material, connect the `T_<name>_SKIN` mask between the base
   albedo and the output: `albedo = lerp(baseAlbedo, skinColor, skinMask)`.
2. `skinColor` comes from the selected skin's recipe — a `LinearColor` parameter the
   game sets from `skins.json` (derive RGB from hue/saturation, or just lerp the
   hull tint toward the skin's accent color).
3. Left/right-click the ship at a dock → cycle the skin (Redesign-bay analog).
   Because the skin COLOR is a runtime parameter, **any skin applies to any ship
   with no per-skin texture** — exactly X4's payoff.

Preview: `python Tools/render_studio.py SM_Ship_Fighter_01 --skin argon_blue`
recolors the skinnable hull while keeping windows/accents fixed (verified).

## Regenerate / tweak
Everything is code-driven — edit `Tools/generate_adastrea_assets.py` (a parametric
builder: box/cyl/cone/sphere/torus/rock primitives + bevel + join + clean) and re-run:
```
"C:\Program Files\Blender Foundation\Blender 5.2\blender.exe" -b --python Tools/generate_adastrea_assets.py
python Tools/fbx2obj.py && python Tools/qa_assets.py   # convert + QA
python Tools/render_objs.py                            # fresh previews
```
Want a new ship? Add a `build_*()` function with primitives, wire it into `main()`, regenerate.