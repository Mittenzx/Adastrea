# Galaxy map: star systems and sectors

The full-screen map (**M** while flying) has three layers:

| Layer | Key | Shows |
|---|---|---|
| Sector | `3` | The loaded level in 3D: stations, ships, you (the original map) |
| System | `4` | The current star system: the star, its sectors, gates between them, and jump-lane exits at the rim |
| Universe | `5` or `U` | Every star system and the jump lanes between them |

You can also click the tabs at the top. On the System and Universe layers: click to select, click again (or press **Enter**) to open the selection, and press **Backspace** to go up a layer. Clicking a rim arrow on the System layer switches to that neighbouring system.

## Where the data lives

`Content/Data/Universe/Galaxy.json`, loaded by `UGalaxySubsystem` (a GameInstance subsystem, `Source/Adastrea/Public/Universe/GalaxySubsystem.h`). If the file is missing or broken, a two-system built-in galaxy is used and the Universe panel shows a warning.

- `adastrea.ReloadGalaxy` re-reads the JSON in PIE, so you can edit and look again without restarting.
- `adastrea.GalaxyInfo` logs every system and sector, and which sector the current level resolves to.
- `tests/test_galaxy_data.py` validates the file: unique IDs, links resolve, gates stay inside their system, and every `level` has a `.umap`.

## Adding a sector

1. Add an entry to a system's `sectors` in `Galaxy.json`:
   ```json
   { "id": "kestrel_port", "name": "Kestrel Port", "type": "Trade Hub", "security": "High",
     "faction": "Adastrea Trade Compact", "description": "...",
     "level": "", "orbitRadius": 0.3, "orbitAngle": 200, "gates": ["kestrel_edge"] }
   ```
   - `id`: unique across the whole galaxy.
   - `level`: leave empty while the sector is only planned. It draws as a grey hexagon.
   - `orbitRadius` (0.1 to 1) and `orbitAngle` (degrees, 0 = right, counter-clockwise) set where it sits on the System layer.
   - `gates`: other sectors **in the same system**. Links are made two-way on load, so list each gate once.
2. When the level exists, set `level` to its package path, e.g. `/Game/Maps/Kestrel_Port`. The sector turns teal ("built").
3. To tie the level to the sector, either:
   - rely on the `level` path (the subsystem matches the loaded map's package name), or
   - place an `ASpaceSectorMap` in the level and set its **Sector Id**. This takes priority, and supports several sector markers in one level (the one containing the player wins).
4. Run `pytest tests/test_galaxy_data.py`.

## Adding a star system

Add an entry to `systems` with `id`, `name`, `starClass`, `starColor` ([r, g, b] from 0 to 1), `position` ([x, y] in light-years on the Universe layer), `faction`, `description`, `jumpLinks` (other system IDs, made two-way on load) and `sectors`. A system with no lanes shows as uncharted.

## Currently mapped levels

| Sector | System | Level |
|---|---|---|
| Adastrea Prime | Adastrea | `/Game/Maps/TestLevel` |
| Alpha Reach | Adastrea | `/Game/Maps/SectorTest_Alpha` |
| Beta Drift | Adastrea | `/Game/Maps/SectorTest_Beta` |
| Cinder Belt | Adastrea | `/Game/Maps/MiningTest` |
| Varos Gauntlet | Varos | `/Game/Maps/CombatArena` |

All other sectors are planned.

## Not done yet

- **Travel.** Nothing moves the player between sectors or systems yet: no gate or jump actors, and no level streaming or loading. The map only shows the layout and the player's sector.
- Discovery / fog of war. Every system is visible.
- Cooking: only TestLevel and MiningTest are in `MapsToCook`. Add a sector's level there when it becomes reachable in a packaged build.
