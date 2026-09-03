# Adastrea — UE Material Reference (M_* → T_*)

Authoritative mapping for building **Unreal Engine 5.8 materials** from the Adastrea
procedural kit. Every part's FBX carries a material named `M_<Part>` as the hook;
UE builds one PBR material from the matching `T_<Set>_*` texture files and assigns
it to the part. This table is the single source of truth — it mirrors the
`MAT_TO_TEX` dict in `Tools/generate_adastrea_assets.py`.

## How to build each UE material
For every row below, create UE material `M_<name>` and connect the `T_<Set>` maps:
- `T_<Set>_D.png`  →  **Base Color**
- `T_<Set>_N.png`  →  **Normal**  (set texture to "None"/normal file)
- `T_<Set>_R.png`  →  **Roughness**  (sRGB off)
- `T_<Set>_M.png`  →  **Metallic**  (sRGB off)
- `T_<Set>_AO.png` →  AO (multiply into Base Color)
- `T_<Set>_E.png`  →  **Emissive**  (enable where you want neon/windows to glow)

The `M_*` material name is what the imported FBX parts reference — name the UE asset
exactly `M_<Part>` so the import maps slots automatically.

## Complete mapping

| UE material (`M_*`) | Texture set (`T_*`) | Palette / usage |
|---|---|---|
| **Hulls / carcasses** | | |
| `M_Hull` · `M_Fighter_Hull` | `Ship_Hull` | Argon blue-grey hull |
| `M_Freighter_Hull` | `Freighter` | Teladi green trader |
| `M_Gunship_Hull` | `Gunship` | Split red warship |
| `M_Corvette_Hull` | `Corvette` | Paranid purple |
| `M_Miner_Hull` | `Miner` | industrial rust |
| **Propulsion** | | |
| `M_Engine_Block`/`Ion`/`Compact` · `M_Fighter_Engine` | `Engine` | warm grey + hot orange glow |
| **Cargo** | | |
| `M_Cargo_Hold`/`Rack`/`Tank` | `Cargo` | olive + green container marker |
| **Weapons** | | |
| `M_Weapon_Block`/`TriLaser`/`Missile` | `Weapon` | dark + menacing red |
| **Sensors** | | |
| `M_Sensor_Block`/`Dome`/`Cross` | `Sensor` | light + cool cyan scan |
| **Reactors** | | |
| `M_Reactor_Block`/`Fusion`/`Spike` | `Reactor` | warm + amber core |
| **Mining / industry** | | |
| `M_Drill` | `Drill` | industrial yellow warning |
| `M_MiningLaser` | `MiningLaser` | teal beam |
| `M_Habitat_Ring` | `HabRing` | habitat ring |
| `M_Asteroid_Shell` | `AsteroidShell` | asteroid |
| **Station** | | |
| `M_Station_Hab` | `Station_Hab` | lit window grid + neon cyan trim (cyberpunk) |
| **Props / objects** | | |
| `M_Prop_Crate` | `Prop_Crate` | shipping crate |
| `M_Prop_Tank` | `Prop_Tank` | tank |
| `M_Derelict` | `Derelict` | wrecked/derelict |
| `M_FuelCell` | `FuelCell` | fuel cell |
| `M_Satellite` | `Satellite` | comms satellite |
| `M_CommsTower` | `CommsTower` | relay tower |
| **Combat FX** | | |
| `M_Combat_Plasma` | `Combat_Plasma` | red plasma bolt |
| `M_Combat_Laser` | `Combat_Laser` | cyan laser |
| `M_Combat_Missile` | `Combat_Missile` | torpedo |
| `M_Combat_Hit` | `Combat_Hit` | impact flash |
| **Interiors (first-person walk)** | | |
| `M_Interior_Deck` | `Int_Deck` | floor |
| `M_Interior_Hab` | `Int_Wall` | corridor/crew walls (modern grey) |
| `M_Interior_Cockpit` | `Int_Cockpit` | cockpit console |
| `M_Interior_Alien` | `Alien_Wall` | old-school sci-fi (Alien) hold |
| **Glass** | | |
| `M_Fighter_Glass` | `Basic_Glass` | transparent glazed canopy |

## Per-class material feel (baked into each texture set's rough/metal params)
Argon polished 0.30/0.97 · Paranid sleek 0.32/0.96 · Teladi matte 0.45/0.92 ·
Split worn 0.40/0.94 · Miner heavy-duty 0.52/0.88.

## Notes
- **`_R` carries anisotropic grain** (H/V-grad ~1.3) — for best reflections feed the
  same `_R` into a masked **Anisotropy** node where grain is strong.
- **`_M` varies painted-vs-bare-metal** (raised seams bare-metal, recesses primer).
- **Skin system (Phase 2):** also ship `T_<Set>_SKIN.png` (1.0 = skinnable, 0.0 =
  fixed accent/window/neon). Wire `albedo = lerp(baseAlbedo, skinColor, skinMask)`
  and drive `skinColor` from `Assets/FBX/generated/skins.json`.
- **Hand-authored ships** (`Assets/FBX/hand_author/`, e.g. `SM_Ship_Frigate_Hand`,
  `SM_Ship_Cutlass_Hand`) use `M_Hull` → `T_Ship_Hull` by default; overrides map
  `Frigate`→`Freighter`, `Cutlass`→`Gunship` in `Tools/hand_ships.py`.
- This mapping lives in two places and SHOULD match: this doc + `MAT_TO_TEX` in the
  generator. If you add a texture set, update both.

## Interior furniture materials (solid-color, modular)

The crew-quarters hab and the flagship command bridge interiors are split into
**separate FBX items per furniture zone**, each with a DISTINCT **solid-color**
material (not texture-driven). These are `M_Int_*` materials — build each as a UE
material with a plain solid Base Color (from the FBX import) and assign to the
part's slot. They are independent, so UE can color them separately.

| FBX item (part) | `M_Int_*` material | Color |
|---|---|---|
| Shell | `M_Int_Shell` | grey (0.42,0.44,0.47) |
| Lights | `M_Int_Lights` | cool blue |
| Bunks | `M_Int_Bunks` | tan |
| Mess | `M_Int_Mess` | warm brown |
| Desks | `M_Int_Desks` | teal |
| Galley | `M_Int_Galley` | light steel |
| Vents | `M_Int_Vents` | red accent |
| Hatch | `M_Int_Hatch` | dark |
| (Bridge) Deck | `M_Int_Deck` | warm grey |
| (Bridge) Console | `M_Int_Console` | deep teal |
| (Bridge) Stations | `M_Int_Stations` | teal |
| (Bridge) Viewport | `M_Int_Viewport` | dark glass |
| (Bridge) Lights | `M_Int_Lights` | cool |
| (Bridge) Hatch | `M_Int_Hatch` | dark |

These FBX live in `Assets/FBX/generated/SM_Int_*_<Zone>.fbx` (crew quarters:
`_Shell/_Bunks/_Mess/_Desks/_Galley/_Lights/_Vents/_Hatch`; command bridge:
`SM_Int_CommandBridge_*`). Import each onto the walkable interior; the solid color
is carried in the FBX material.