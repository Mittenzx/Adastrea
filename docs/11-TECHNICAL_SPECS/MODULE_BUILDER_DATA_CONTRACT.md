# Module-Builder Data Contract (for main)

**Status:** Ready for implementation | **Written:** 2026-08-30 | **Audience:** main/gameplay

This is the exact contract for the **in-game ship module-builder** UI. It documents
the two JSON files the asset side produces, how the game should read them, and how
to mount/place modules. Point your builder at these files and this spec and you are
most of the way there.

---

## 1. Where the data lives

All under `Assets/FBX/generated/`:

- **Per-ship hardpoint manifests**: `<ShipName>_hardpoints.json`
  (10 ships/stations have one today)
- **Global module catalog**: `SM_Modules_catalog.json`
- **Mesh FBX**: `<ShipName>_<Part>.fbx` (e.g. `SM_Ship_Fighter_01_Engine.fbx`,
  `..._Weapon.fbx`, `..._Sensor.fbx`, `..._Reactor.fbx`, `..._Cargo.fbx`)

> Not part of this contract, but available: `<ShipName>_Assembled.fbx` (full ship),
> `<ShipName>_Carcass.fbx` (base shell), and `previews/module_picker.png` (a
> color-coded sheet of every module variant — usable as the builder's icon
> thumbnails).

---

## 2. Module catalog schema — `SM_Modules_catalog.json`

```json
{
  "modules": {
    "engine": {
      "variant_name": "engine",
      "options": [
        {"id": "standard",  "description": "Twin nacelles + bells (balanced)"},
        {"id": "ion",       "description": "Long ion-drive pylons (fast, sleek)"},
        {"id": "compact",   "description": "Squat single pod (budget)"}
      ],
      "fits_hardpoints": ["HP_Engine"]
    },
    "cargo":    { "options": [{"id":"containers"},{"id":"bulk_tank"},{"id":"flat_rack"}], "fits_hardpoints": ["HP_Cargo"] },
    "weapon":   { "options": [{"id":"cannon"},{"id":"tri_laser"},{"id":"missile"}],        "fits_hardpoints": ["HP_Nose"] },
    "sensor":   { "options": [{"id":"mast"},{"id":"dome"},{"id":"cross"}],                 "fits_hardpoints": ["HP_Sensor"] },
    "reactor":  { "options": [{"id":"core"},{"id":"fusion_ring"},{"id":"spike"}],          "fits_hardpoints": ["HP_Core"] }
  }
}
```

### Reading it
- **`modules.<type>.options[]`** = the module *variants* the player can choose for
  that type. `id` maps to the mesh suffix AND to the builder variant key.
- **`modules.<type>.fits_hardpoints[]`** = which hardpoint sockets accept this
  module type. Use to drive "can this module go here?" checks in the UI.

---

## 3. Hardpoint manifest schema — `<ShipName>_hardpoints.json`

```json
{
  "ship":         "SM_Ship_Fighter_01",
  "size_class":   "small",
  "scale":        1.0,
  "carcass_dims": [250, 340, 95],          // x, y, z in unreal-cm
  "hardpoints": {
    "HP_Engine": {
      "position":          [0.0, -176.8, 49.3],  // x,y,z in unreal-cm (world/ship-local)
      "mount_orientation": "AXIS_FORWARD",        // how the module's forward axis aligns
      "allowed_types":     ["engine"],
      "occupied":          true
    },
    "HP_Cargo":  { "position": [0.0, -17.0, 37.9],  "allowed_types": ["cargo"],  "occupied": true },
    "HP_Nose":   { "position": [0.0, 115.6, 34.1],  "allowed_types": ["weapon"], "occupied": true },
    "HP_Sensor": { "position": [0.0, 61.2, 123.4],  "allowed_types": ["sensor"], "occupied": true },
    "HP_Core":   { "position": [0.0, -119.0, 93.0], "allowed_types": ["reactor"],"occupied": false },
    "HP_Underslung": { "position": [0.0, -102.0, 39.8], "allowed_types": ["drill"], "occupied": false },
    "HP_Forward":    { "position": [0.0, 102.0, 45.5],  "allowed_types": ["mining_laser"], "occupied": false }
  }
}
```

### Fields
- **`position`** — the socket's location in Unreal cm, ship-local space (the ship's
  origin). Attach the module mesh here, then apply `mount_orientation`.
- **`mount_orientation`** — convention for which axis points forward. Today all are
  `AXIS_FORWARD`. For a part whose native forward is +Y (Blender), align its +Y to
  the ship's forward. If you support more orientations later, the value documents it.
- **`allowed_types[]`** — the module *types* (from the catalog) this socket accepts.
- **`occupied`** — whether a module is currently mounted there (the ship was built
  with it). Use to show "installed / empty" in the UI and to load the initial build.

### The hardpoint set (union across ships)
`HP_Engine`, `HP_Cargo`, `HP_Nose`, `HP_Sensor`, `HP_Core`, `HP_Underslung`, `HP_Forward`
— not every ship has every one, so always read from the manifest, don't hardcode.

---

## 4. Mapping a chosen module → mesh + placement

To place a module `type` with variant `id` on ship `X`:

1. **Mesh to spawn**: The module's FBX is **variant-specific geometry**. A variant is
   produced by building the ship with a particular config, e.g.
   `assemble_ship('small', name, {'engine':True, 'engine_variant':'ion'})` yields a
   *differently-shaped* `Engine.fbx` (compare `SM_Ship_Fighter_01_Engine.fbx` =
   standard vs `SM_Ship_Fighter_02_Arcangel_Engine.fbx` = ion). So to give the
   player the `ion` engine, you need an Arcangel-style `Engine` mesh, not the
   standard one.
   - **Practical path for the builder**: the asset side already ships variant ships
     as full part sets — `SM_Ship_Fighter_02_Arcangel_*` (ion engine, tri-laser,
     fusion reactor, dome sensor, flat-rack cargo) and
     `SM_Ship_Freighter_02_Bulkhauler_*` (compact engine, bulk-tank, spike reactor,
     cross sensor). Use those FBX as the variant meshes, OR ask the asset side to
     export a variant-set for a given build.
   - The catalog's `options[id]` is the variant key; the asset side can build any
     combination on request.
2. **Socket**: look up the matching hardpoint in the ship manifest by `allowed_types`.
3. **Attach** the mesh at `hardpoint.position` (ship-local), rotate so its forward
   matches `mount_orientation`.

### Current built-in defaults (what each ship ships with)
- **Fighter** `small`: engine, cargo, weapon(twin), sensor — HP_Core/Underslung/Forward empty.
- **Freighter** `medium`: engine, cargo, sensor, reactor.
- **Gunship** `small`: engine, weapon(twin), sensor(asym).
- **Corvette** `corvette`: engine, cargo, weapon(twin), sensor, reactor.
- **Miner** `corvette`: engine, cargo, drill, mining_laser, sensor.
- **Mining Station** `large`: drill, mining_laser, sensor, reactor, cargo.
- **Generation Ship** `medium`: engine, cargo, sensor, reactor, habitat_ring.
- **Asteroid Hab** `large`: habitat_ring, asteroid_shell, sensor.

---

## 5. Suggested builder UI flow

1. **Player docks at a station** (matches the existing dock→trade loop) → open builder.
2. List the ship's **hardpoints** from its manifest. Show each socket's current
   module (from `occupied`/catalog) or "empty".
3. For a selected socket, offer the **options** from the catalog whose type is in
   `allowed_types`. Render each with its `SM_Combat`/module_picker thumbnail.
4. **On confirm**: swap the mesh at that socket (spawn new part FBX, despawn old),
   update the manifest `occupied` + chosen variant, persist to the save.
5. **Validate**: a module type only mounts where `allowed_types` includes it; miss a
   `fits_hardpoints` entry and the option won't show.

---

## 6. Design fixture (off the shelf)

- **Colors in the picker**: engine=blue, cargo=green, weapon=red, sensor=amber,
  reactor=magenta, hull=grey (matches `previews/module_picker.png`).
- Keep the builder **station-only** for v1 (simpler, matches the dock→trade loop);
  add in-flight editing later.