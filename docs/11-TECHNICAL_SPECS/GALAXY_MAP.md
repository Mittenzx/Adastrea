# Galaxy map: star systems and sectors

The full-screen map (**M** while flying) has three layers:

| Layer | Key | Shows |
|---|---|---|
| Sector | `3` | The loaded level in 3D: stations, ships, you (the original map) |
| System | `4` | The current star system: the star, its sectors, gates between them, and jump-lane exits at the rim |
| Universe | `5` or `U` | Every star system and the jump lanes between them |

You can also click the tabs at the top. On the System and Universe layers: click to select, click again (or press **Enter**) to open the selection, and press **Backspace** to go up a layer. Clicking a rim arrow on the System layer switches to that neighbouring system.

### Sector layer

- **Click an object** (station, ship, jump gate or asteroid) to target it. Hover shows its name, type and range.
- The info panel on the right shows the sector's type, security, faction and system, what's in it, your current target (range, and height above or below you), and the sector's jump gates, nearest first.
- Your marker points along your ship's heading. Objects above or below the grid plane have a faint stem down to it.
- Gates and your target that are outside the view get a marker on the edge of the view with their distance.
- Labels that would overlap are moved or hidden; hover an icon to see its name.
- `[1]` / `[2]` show or hide ships / stations. The grid spacing is shown bottom-left.

### Routes

- **System layer:** selecting a sector draws the gate route from your sector to it in green, with arrows, including the lane exit when the route leaves or enters the system. The panel lists the jumps.
- **Universe layer:** selecting a system draws the jump-lane route from your system to it, and the panel shows the number of lane jumps.
- For scripted checks: `adastrea.Map system <sectorId>` selects a sector, and `adastrea.Map universe <systemId>` selects a system.

## Jump gates (travel)

Every sector's level gets a jump gate for each entry in its `gates` and `laneGates`. The gates are spawned when the level starts (`UJumpGateWorldSubsystem`), so maps need no edits.

- **Using a gate:** fly the ship through the ring to jump. The target sector's level loads, and you come out of the gate that leads back, 150 m in front of it.
- **Placement:** automatic gates sit 2 km from the sector centre, in the direction the destination lies on the maps. The sector centre is the `ASpaceSectorMap` marker if there is one, otherwise the centroid of the stations. To place a gate by hand, put an `AJumpGate` in the level and set its **Target Sector Id**. It replaces the automatic one.
- **What travels with you:** ship class and data asset, credits, cargo, and progression. They're carried in an in-memory `UAdastreaSaveGame` snapshot (`USaveGameSubsystem::CollectGameState` / `ApplyGameState`). Player-built stations stay in the sector they were built in, and are still there when you come back in the same session.
- **Offline gates:** a gate to a sector with no `level` is offline (amber) and doesn't jump.
- **Levels with no ship:** if a level's game mode doesn't give the player a ship within 3 s, the ship you jumped in with is spawned at the gate.
- **HUD:** gates appear as markers with distance, and a prompt shows within 3 km. They can be targeted with Y / `[` `]` / click, and are drawn on the Sector map. The System map shows lane gates as violet lines, and the sector panel shows the gate route from your sector.
- **Dev shortcut:** `adastrea.JumpTo <sectorId>` jumps without flying to the gate.

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
   - `laneGates`: sectors **in another system** that this sector has a jump-lane gate to. The two systems must share a `jumpLinks` entry. Also made two-way.
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

- **Saving across sectors.** A save file records the level it was made in, but loading it from another sector applies it to the current level. Player-built stations in sectors you aren't in are only kept for the session.
- Discovery / fog of war. Every system is visible.
- Cooking: only TestLevel and MiningTest are in `MapsToCook`. Add a sector's level there when it becomes reachable in a packaged build.
