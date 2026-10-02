# Galaxy Plan

What the playable galaxy contains: how big it is, how it's divided into regions, what each region
is for, where the resources are, what's dangerous, and what you find in a sector.
Builds on `Content/Data/Universe/Galaxy.json` (8 systems, 17 sectors today),
`Organisations.json` (5 Authorities), the crafting tree's 37 raw resources, and
[`PIRACY_AND_LAW.md`](PIRACY_AND_LAW.md).

Status: **agreed 2026-10-02** (see Decisions at the end). Names of new systems are still
placeholders.

## Ground rules this plan keeps

- **People are scarce; nobody kills unless they must** ([`../SETTING.md`](../SETTING.md)). So the
  danger in dangerous space isn't dying. It's **being disabled where nobody will come for you**:
  stranded, captured, or losing your ship and cargo.
- Ships become wrecks, never explode. Stations are invulnerable.
- Drones mine, extract gas, tow and move cargo. No beams.
- Security (who answers a distress call) comes from the governing Authority, as built in step 1 of
  the piracy plan.

## 1. Scale

Five layers, from big to small:

| Layer | What it is | Count (full plan) |
|---|---|---|
| **Galaxy** | The playable galaxy, one of the remote galaxies far from the old empires' wars. Others may be added later | 1 |
| **Region** | A group of systems with one character: who runs it, how safe, what it's for | 6 |
| **System** | A star with its sectors, on the universe map | ~20 |
| **Sector** | One playable space you jump into (a level today) | ~60 |
| **Point of interest (POI)** | A place inside a sector: station, belt, wreck field, gas skim zone | 3-6 per sector |

Why these numbers: about 60 sectors is enough that the map feels large and regions feel different
(X4 has ~100+ with a big team), and few enough that every sector can have something worth going
there for. Core systems have 4-5 sectors, outer ones 2-3: busy where people live, sparse where
they don't.

**Built in waves, not all at once:**

| Wave | Systems | Sectors | Goal |
|---|---|---|---|
| 1 | 8 (today's) + 2 | ~24 | Every region exists with at least one sector of each danger type, so the whole loop can be played |
| 2 | ~14 | ~40 | Regions filled out; trade routes between regions matter |
| 3 | ~20 | ~60 | Hidden lanes, deep Pale and Sable systems, late-game rewards |

**Every sector is hand-built.** Each sector is its own level (`"level"` in `Galaxy.json`), built
by hand by Claude in the editor, no template levels. The `resources`, `hazards` and `pois` data
below is the brief for building each level and what the map, economy and hazard systems read; it
doesn't generate the level. Expect about one sector per session, so the waves set the pace.

## 2. Regions

The galaxy splits into six regions. Each has a character, a security band, a reason to go there
and its own dangers. Directions match the coordinates already on the universe map.

```
                 Graveyard (NW)
                     |
   Sable Dark (W) -- Varos Marches -- Compact Core -- Kestrel -- Pale Expanse (E)
       :  (hidden lanes)                  |
       :........................... Free Belts (SE)
```

| Region | Systems | Security | Governed by | What it's for |
|---|---|---|---|---|
| **Compact Core** | Adastrea, Lumen, Kestrel, *Meridian* | High → Medium | Trade Compact, Lumen Accord, Kestrel Port Authority | Safe start, markets, shipyards, research, food |
| **Varos Marches** | Varos, *Brand*, *Halt* | Low → None | Varos Syndicate | Military border, tolls, salvage; the way to the Graveyard |
| **Free Belts** | Corvid, *Tamsin*, *Orrin* | Medium → Low | Haven Moot (+ guilds) | Mining and gas: the galaxy's raw materials |
| **The Graveyard** | Nyx, *Ashfall*, *Requiem* | None | Nobody | The old empires' battlefield: salvage, derelicts, war machines still running |
| **Pale Expanse** | Thule, *Vigil*, *Hollow* | None | Unclaimed | Exploration: unsurveyed hot stars, rare resources, anomalies |
| **Sable Dark** | Sable, *Murk*, *Lantern* | None | Pirate gangs | Pirate bases, fences, guild hub ships; reached only by hidden lanes |

*Italic* = new system.

### Region types (reusable)

So more regions can be added later, each region has a **type** that sets its defaults:

| Type | Security band | Population | Hazards | Rewards |
|---|---|---|---|---|
| **Heartland** | High-Medium | Many stations, traffic, patrols | Few | Common resources, best markets, safety |
| **March** (border) | Low | Military stations, tolls | Contested lanes, scans | Salvage, smuggling, military work |
| **Frontier** | Medium-Low | Mining outposts, free ports | Environment (tides, ice, storms) | Bulk and mid-tier resources |
| **Ruin** | None | Scavengers only | Old war machines, mines, radiation | Salvage, derelict tech, lore |
| **Wilds** | None | Almost nobody | Environment, no rescue, fuel range | Rare resources, anomalies, discovery |
| **Haven** (pirate) | None | Pirate bases | Pirates, kill-switch ships, no law | Fences, refits, black-market goods |

## 3. Systems and sectors (full plan)

Existing sectors kept as they are; new ones marked *new*. Security in brackets.

**Compact Core**
- **Adastrea** (G2V, home): Prime (High, trade hub), Alpha Reach (High, industrial), Beta Drift
  (Medium, research), Cinder Belt (Medium, mining), Relay (Low, frontier gate).
- **Lumen** (F0V): Academy (High, research), Array (High, energy), *Lumen Shade* (Medium,
  farming station in a planet's shadow).
- **Kestrel** (K5V): Port (High, trade hub), Skim (Medium, gas), Edge (Low, smugglers).
- ***Meridian*** (G8V, new): the Core's **food and habitat** system. A garden world's orbit,
  agriculture stations, and the biggest population. Since people are the scarcest resource, this
  is the most valuable system in the galaxy, and pirates' biggest prize. 3 sectors: *Meridian
  Gardens* (High, farming), *Meridian Hab* (High, habitat/recruiting crew), *Meridian Reach*
  (Medium, ice for water).

**Varos Marches**
- **Varos** (M4V): Gauntlet (None, contested), Hold (Low, military), Scrap (Low, salvage).
- ***Halt*** (new): the **toll gate** between the Core and the Graveyard. Syndicate scans every
  ship, charges passage, confiscates stolen cargo. 2 sectors: *Halt Gate* (Low), *Halt Shoals*
  (None, smugglers' way around the toll through a debris field).
- ***Brand*** (new): Syndicate shipyard and refit yards, the only place outside the Core to buy
  warships. 2 sectors.

**Free Belts**
- **Corvid** (binary): Belt (Low, rare metals, tides), Haven (Medium, free port).
- ***Tamsin*** (new): comet and ice system. Water and volatiles. 3 sectors.
- ***Orrin*** (new): gas giant cluster. Hydrogen, methane, nitrogen in bulk; Helium-3 in the
  storm bands. 3 sectors.

**The Graveyard**
- **Nyx** (pulsar): Derelict (None, anomaly). Add *Nyx Storm* (None, radiation).
- ***Ashfall*** (new): where two empire fleets died. A huge wreck field, salvage everywhere,
  mines still armed. 3 sectors.
- ***Requiem*** (new): a dead empire fortress world, its defence platforms still awake. The
  hardest sector in the galaxy and the best old-war tech. 2 sectors.

**Pale Expanse**
- **Thule** (B8V): Outpost (None). Add *Thule Corona* (None, Helium-3 and heat).
- ***Vigil*** (new): an unexplained beacon nobody built. Exploration and lore. 2 sectors.
- ***Hollow*** (new): a rogue planet or dark mass that bends lanes. Gravity hazards, rarest ores.
  2 sectors. Wave 3.

**Sable Dark**
- **Sable** (M1V): no lane yet. Discovered through a hidden lane from Kestrel Edge or Halt Shoals.
  2 sectors.
- ***Murk*** (new): dark nebula, sensors nearly blind. Pirate ambushes. 2 sectors.
- ***Lantern*** (new): the pirates' "free town": fences, refits, re-registering stolen ships,
  a guild hub ship often parked here. 2 sectors.

## 4. Resources

**Rule: the more dangerous the space, the more valuable what's in it.** Common materials are near
home; the rarest are where nobody will rescue you. Every region is also **short of something**,
so goods must travel, which is what traders and pirates live on.

| Region | Rich in | Short of (imports) |
|---|---|---|
| Compact Core | Iron, copper, nickel, aluminium, silicon, water ice; **food** (all organics, Meridian); energy | Rare metals, Helium-3, salvage |
| Varos Marches | Scrap metal, salvaged components, derelict hull plate; zinc, manganese | Food, water, fuel |
| Free Belts | Titanium, chromium, cobalt, lithium, zinc, manganese, tungsten (Corvid); water ice, volatiles (Tamsin); hydrogen, methane, nitrogen, noble gas (Orrin) | Food, components, people |
| The Graveyard | Salvage of all kinds, derelict tech; uranium (Nyx) | Everything; nobody lives there |
| Pale Expanse | Platinum, palladium, gold, silver, rare earth elements, carbon crystal, precious stones; **Helium-3** (Thule Corona) | Everything |
| Sable Dark | Little of its own; it takes what it needs | People, fuel, parts (see the pirate needs table) |

How a sector lists them (proposed `Galaxy.json` field):

```json
"resources": [
  { "item": "TitaniumOre", "richness": "High" },
  { "item": "CobaltOre",   "richness": "Medium" }
]
```

`richness` (Low / Medium / High) sets how many asteroids or gas pockets a sector spawns and how
fast they refill. Item ids are the crafting tree's (`CraftingTree.json`), so every raw resource has
at least one sector, which can be checked by a script.

## 5. Dangers

Two separate ratings per sector, both shown on the map:

- **Security** (already built): how fast help comes. High / Medium / Low / None.
- **Hazard**: what the place itself does to you. None / Mild / Severe / Extreme.

A sector can be safe and hazardous at once (a patrolled radiation zone), or calm and lawless
(a quiet pirate nebula). Keeping them separate is what makes regions feel different.

### Environmental hazards

Each has a clear effect, a warning, and a way to cope. None kills; all can disable or strand you.

| Hazard | Where | Effect | How you cope |
|---|---|---|---|
| **Radiation storm** | Nyx (pulsar), Thule (blue star) | Comes in waves with a countdown: shields drain, crew and sensors suffer | Rad shielding upgrade; shelter behind asteroids or a station |
| **Nebula (ionised)** | Beta Drift, Murk, Sable | Sensor range cut, no long-range locks, map blank beyond a short range | Better sensors; local knowledge; it's also where you hide |
| **Gravity tides** | Corvid binary, Hollow | Belts and wrecks drift; your ship is pushed off course | Stronger engines; time your mining between tides |
| **Dense debris** | Cinder Belt, Ashfall, Halt Shoals | Hull damage at high speed | Fly slow; shields up |
| **Old minefields** | Ashfall, Requiem, Varos Gauntlet | Hidden mines that **disable** ships | Scanner sweeps; follow cleared routes (sold as information) |
| **Heat** | Thule Corona, Lumen Array | Heat builds near the star; systems shut down at max | Heat sinks; short runs in and out |
| **Gas storms** | Kestrel Skim, Orrin | EMP bursts knock out drones and systems briefly | Shielded drones; watch the storm bands |
| **Distance** | Pale Expanse, Sable Dark | No stations, no fuel, no rescue | Fuel range planning; carry supplies |

### Dangers from others

| Danger | Where | What happens |
|---|---|---|
| **Pirates** | Low and None security, most in Sable Dark and the Marches | Hail, demand, disable, strip, possibly capture (piracy plan) |
| **Old war machines** | The Graveyard | Automated defences from the empires' war. They are machines, not people: they disable anything that comes near and don't take prisoners. Your ship drifts and nobody comes, unless you've arranged help |
| **Syndicate tolls and scans** | Varos Marches, Halt | Pay, be scanned, have stolen cargo taken. Refuse and they disable you |
| **Desperate gangs** | Anywhere near Sable, worse over time | A starving gang raids nearer to lawful space (piracy plan §2a) |

### Danger by region (summary)

| Region | Security | Hazard | The danger, in one line |
|---|---|---|---|
| Compact Core | High-Medium | None-Mild | Small: debris at Cinder, smugglers at the edges |
| Varos Marches | Low-None | Mild-Severe | The Syndicate takes its cut; the Gauntlet is a fight |
| Free Belts | Medium-Low | Mild-Severe | The space itself: tides, ice, storms; pirates prey on miners |
| The Graveyard | None | Severe-Extreme | Old machines disable you and nobody comes |
| Pale Expanse | None | Severe-Extreme | Radiation, heat and distance; get stranded, stay stranded |
| Sable Dark | None | Mild-Severe | Pirates' home: capture is likely |

Proposed `Galaxy.json` fields:

```json
"hazard": "Severe",
"hazards": [ { "type": "RadiationStorm", "severity": "Severe" } ]
```

## 6. Points of interest

Every sector has **one anchor** (the reason it exists) and **2-5 secondary** POIs, plus dynamic
events that come and go.

| POI | Kind | Notes |
|---|---|---|
| Station (trade hub, refinery, shipyard, habitat, farm, research, fuel depot, military, free port) | Anchor | Existing station modules |
| Asteroid field (by ore), ice field | Anchor or secondary | Drone mining |
| Gas skim zone | Anchor or secondary | Drone gas extraction |
| Jump gate / relay | Secondary | Every sector has at least one |
| Wreck field | Secondary | Salvage; some hold escape pods |
| Derelict | Secondary | Old empire ship or station to explore: tech, lore, data on hidden lanes |
| Old war platform / minefield | Hazard POI | Graveyard and Gauntlet |
| Pirate base | Anchor (hidden) | Found by scouting, prisoners, bought information |
| Guild hub ship | Mobile | Moves between low-security sectors (piracy plan §7) |
| Smuggler cache | Secondary (hidden) | Stolen goods stashed off the lanes |
| Anomaly | Anchor | Pulsar, beacon, rogue mass; Vigil and Hollow |
| Survey beacon | Secondary | Reveals a sector's map; Pale Expanse |

Dynamic events (not in the data, raised by game systems): distress calls, drifting escape pods,
pirate raids, radiation storm warnings, a guild hub ship arriving.

Proposed `Galaxy.json` field:

```json
"pois": [
  { "type": "Station",       "id": "corvid_haven_port", "name": "Haven Port" },
  { "type": "AsteroidField", "resources": ["CobaltOre", "TungstenOre"], "richness": "High" },
  { "type": "WreckField",    "size": "Small" }
]
```

## 7. Data changes

All in `Galaxy.json`, read by `UGalaxySubsystem`:

- `regions[]`: `id`, `name`, `type` (Heartland / March / Frontier / Ruin / Wilds / Haven),
  `description`, map tint colour.
- `systems[].region`: which region the system is in.
- `sectors[].hazard` and `sectors[].hazards[]`.
- `sectors[].resources[]`.
- `sectors[].pois[]`.
- `jumpLinks` can be marked `"hidden": true` (Sable lanes) until discovered.
- New Authorities and guilds in `Organisations.json` for new systems (Meridian under the Compact,
  Brand and Halt under the Syndicate, Tamsin and Orrin guilds under the Haven Moot).
- A validation script: every raw resource appears in at least one sector, every sector has a gate,
  every region is reachable, hidden lanes lead somewhere, and every listed POI exists in its
  sector's level.

## 8. Build order

1. Add `regions`, `hazard`, `resources` and `pois` to `Galaxy.json` and `UGalaxySubsystem`; fill
   them in for today's 17 sectors. Show region tint and hazard rating on the galaxy map.
2. Reusable pieces for hand-building: asteroid fields that spawn a given ore list, wreck fields,
   ice fields, gas skim zones (placeable actors, not template levels).
3. Wave 1: hand-build a level for each of today's 17 sectors that doesn't have one, then Meridian,
   Halt and the missing sectors of Nyx and Thule (~24 sectors). Hazard effects for radiation,
   nebula and debris.
4. Old war machines (Graveyard) and minefields.
5. Wave 2: Free Belts and Marches filled out, hidden lanes, Sable reachable (~40 sectors).
   Remaining hazards (tides, heat, gas storms).
6. Wave 3: Pale Expanse and Sable Dark in full, anomalies (~60 sectors).

## Decisions (2026-10-02)

- **Scale:** about 20 systems and 60 sectors, in three waves. Agreed.
- **One galaxy:** this plan is one galaxy among the remote galaxies; others may come later.
- **Old war machines:** the only things that don't take prisoners are machines left over from the
  empires' war. They disable rather than kill. Agreed for now; may be revisited.
- **Sectors are hand-built,** one level each, by Claude. No template levels.
- **Meridian,** the food and habitat system, is the most valuable place in the galaxy, because
  people are.

## Still open

- **Names.** All new system and sector names are placeholders.
