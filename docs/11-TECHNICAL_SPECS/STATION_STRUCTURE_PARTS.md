# Station Structure & Support Parts — Beyond "Modules"

**Status:** Design theory | **Last Updated:** 2026-09-06 | **Owner:** crafting/design

Today the game calls everything an `ASpaceStationModule` with a `ModuleType` and an
`EStationModuleGroup`. But a corridor, a turret hardpoint, and a crew mess hall are
**not the same kind of thing** as a Marketplace or a Fabricator. This doc theorises
the layer **under / beside** functional modules: structural, connective, defensive
and personnel parts — and argues they deserve their own taxonomy, not just more
entries in the `*Module` list.

---

## 1. The core argument: "module" vs "part"

A **functional module** (out of scope of this doc but for contrast):
- does one economic/industrial job
- has a recipe in `CraftingTree.json` (Tier-6 craftable)
- has `ModuleType` + `ModuleGroup` + `ModulePower` + an `ASpaceStationModule` subclass
- contributes to a station **aggregate** (power, storage, market, turret DPS)

A **structure / support part**:
- is largely **passive or environmental** — it shapes the station (connects it,
  houses people, mounts defences) more than it "does a job"
- still has a **footprint, a mass, a cost, and connection faces** (the builder model)
- is built from the same crafting economy but sells/serves differently (crew,
  defence, transit, habitability)
- may need **no runtime logic** (a corridor just exists) or a **tiny** one (a turret
  mounts a weapon; a mess hall feeds crew)

The distinction matters for design: it lets parts be cheap/greebles, mass-produced,
modularly recombinable, and procedurally placed — while modules stay individually
crafted, expensive, and meaningful.

---

## 2. A taxonomy of non-module parts (theory)

I propose **five** structural families. Each maps onto existing `EStationModuleGroup`
values so it slots into the current system without new parallel enums.

### Family A — Connectors & Transit (group `Connection`)
Parts that **link** the station into a contiguous, walkable, passable structure.
| Part | Concept | Runtime |
|------|---------|---------|
| **Corridor** (exists: `CorridorModule`) | plain connecting tube | passive |
| **Hub / Spoke Core** | multi-faced junction where 3–6 corridors meet | passive |
| **Rotating Ring / Elevator Core** | vertical transit between floors/spoke levels | passive (visual) |
| **Gantry / Catwalk** | open-air passage along a module's exterior | passive |
| **Docking Spine / Boom** | long arm that reaches out to dock ships | passive |

Design notes:
- These are the builder's **glue** — the cheapest, lightest parts, `Connection` group.
- Should be **mass-produced cheaply** (a corridor is 1×N, not a bespoke module).
- Drive no aggregates; they exist so the cluster stays **connected** (builder rule 4.1).

### Family B — Defence & Armour (group `Defence`)
Parts that **protect** the station. Two sub-roles:
| Part | Concept | Aggregate |
|------|---------|-----------|
| **Turret Hardpoint** (exists: `TurretModule`) | a mounting socket for a weapon | `GetTotalTurretDps` |
| **Point-Defence Array** | flak/pd coverage against missiles/drones | turret DPS (anti-proj) |
| **Shield Projector / Bubble** | area shield over a segment (exists: `ShieldGeneratorModule`) | `GetTotalShieldStrength` |
| **Armour Plating / Whipple Layer** | ablative skin, cheap, absorbs kinetic | `integrity` / damage soak |
| **Mine/Interdiction Launcher** | field-lays mines / jams jump-in zones | defence rating |
| **Sensor Mast / Tartar** | early-warning + targeting | signature / detection |

Design notes:
- Defence parts aggregate into DPS/shield/integrity — the existing `SpaceStation.h`
  getters already read `GetTotalTurretDps` / `GetTotalShieldStrength`.
- Some are **mounts** (turret hardpoint holds a crafted weapon) — a natural place to
  slot weapon-*parts* later (see weapons line in the crafting tree).

### Family C — Habitation & Personnel (group `Habitation`)
Parts that **house and sustain crew** (the human layer).
| Part | Concept | Aggregate |
|------|---------|-----------|
| **Crew Quarters** (exists: `HabitationModule`) | berths | `GetTotalCrewCapacity` |
| **Barracks / Bunkroom** (exists: `BarracksModule`) | dense NPC housing | `GetTotalCrewCapacity` |
| **Mess Hall / Galley** | feeds crew; uses Food rations | `GetTotalResidents` / welfare |
| **Medical Bay / Infirmary** | heals crew; uses MedicalSupplies | welfare / survivability |
| **Recreation & Lounge** | morale / public space | `Public` overlap |
| **Offices / Administration** | station management staff | personnel |
| **Hydroponic Garden** (exists in tree: `OrganicFarming`) | food self-sufficiency | food produce |

Design notes:
- These drive **`GetTotalCrewCapacity` / `GetTotalResidents`** — and my builder's
  `crew_budget()` already reads berths (Habitation/Barracks) vs crew required.
- A station that wants autonomous ops needs the **habitation + food** balance.

### Family D — Life Support & Utility (group `Other` / `Public`)
Passive engineering that keeps the station alive and civilised.
| Part | Concept | Aggregate |
|------|---------|-----------|
| **Life-Support Unit** | air/heat/water recycling | livability |
| **Thermal Radiator Banks** | dump waste heat | heat / power efficiency |
| **Comms Relay / Antenna Farm** | long-range comms | network |
| **Waste & Reclamation** | recycle organics/water | self-sufficiency |
| **Water/Gas Storage Tanks** | buffer for life support | storage (Gas/Liquid) |

### Family E — Resident / Public Greeble (group `Public`)
Small, low-cost **placeables** that sell the "lived-in" feel — the *non-functional*
public furniture the user hinted at:
| Part | Concept |
|------|---------|
| **Observation Deck / Viewport** | a place to look out |
| **Landscaped Atrium / Arboretum** | green space, hull glow |
| **Kiosks / Vendor Stalls** | merchant flavour |
| **Plaza / Concourse** | meeting space (Public) |

Design note: these are almost purely **flavour + public-group** presence; they push
`Public` group identity and habitability without new aggregates.

---

## 3. Why this matters to the builder

The current builder (`STATION_BUILDER.md` / `generate_station_builder.py`) already
models: **grid footprint, connection faces, power, build cost, crew budget, docking,
research gating, in-place upgrade**. The theory above slots straight in:

1. **New part metadata group** — `StationModuleBuilderData.json` can add a
   `part_family` field (`Connector`/`Defence`/`Habitation`/`Utility`/`Public`) for
   each entry, alongside `group`. The palette then filters by family as well as group.
2. **Cheap mass-production** — connectors and greeble carry a tiny cost + no power,
   so a station "shell" (corridors + hull) is affordable while its functional core
   is the expensive part. The cost/crew `build_cost_summary()`/`crew_budget()` already
   handle per-part numbers.
3. **Defence aggregation** — turret/shield/armour parts feed the existing
   `SpaceStation.h` defence getters, so a defended station shows real numbers.
4. **Cluster / connectivity** — corridors remain the glue that makes `validate_cluster`
   pass (every part must connect to its plot core).

---

## 4. Proposed concrete additions (theory → spec)

If we adopt the layer, the minimal first ship is:

- **`ASpaceStationPart`** — a second root class *beside* `ASpaceStationModule`
  (or a subclass with a `bIsPart` flag) holding: `Footprint`, `MassKg`, `PartFamily`,
  `ConnectionFaces`. No fabrication recipe required for greeble; structural parts can
  share the module recipe pipeline.
- **`EStationPartFamily`** enum: `Connector, Defence, Habitation, Utility, Public`.
- **New parts in `CraftingTree.json`** (a handful per family to start):
  - Connector: `Corridor`, `CorridorJunction`, `ElevatorCore`
  - Defence: `TurretHardpoint`, `PointDefenceArray`, `ArmourPlating`
  - Habitation: `Galley`, `MedicalBay`, `CrewQuarters`, `RecreationLounge`
  - Utility: `LifeSupportUnit`, `ThermalRadiator`, `CommsRelay`
  - Public: `ViewportDeck`, `Arboretum`, `Kiosk`
- **Builder metadata** gains `part_family`; the palette groups parts by family.

---

## 5. Balance & economy theory

- **Cost curve**: connectors/greeble ≈ 1–2× a Corridor's cost; defence ≈ weapon cost;
  habitation ≈ a few food-rations worth. So a station's "skin" is cheap, its
  "organs" (modules) are expensive — which makes the crafting economy interesting:
  you build a shell cheaply, then grow capacity module-by-module.
- **Power**: connectors/utility/greeble ≈ 0 (passive); defence/habitation small
  negatives. This keeps `power balance` (cluster-wide) meaningful without punishing
  bare structure.
- **Crew**: habitation parts provide berths; utility (life support) is *required* by
  crew; greeble raises habitability. My `crew_budget()` extends naturally.

---

## 6. Open questions (for a review pass)
- Should a `Part` be a distinct class (`ASpaceStationPart`) or a flag on
  `ASpaceStationModule`? (Flag is cheaper to adopt; class is cleaner.)
- Do connectors *need* a runtime behaviour beyond existing? (No — they're passive.)
- Should defence parts require the **crafted weapon** they mount (turret hardpoint
  consumes a `TurretWeapon`)? (Yes — nice tie to the crafting tree.)
- Which parts, if any, should be **research-gated** like niche labs? (e.g. a
  point-defence array gated by `BeamWeaponResearch`.)

---

## 7. Related
- Builder + validation: `STATION_BUILDER.md`, `generate_station_builder.py`,
  `StationModuleBuilderData.json`
- Station system: `STATION_SYSTEM.md`, `Source/Adastrea/Public/Stations/*.h`
- Crafting tree (parts/weapons): `CRAFTING_TREE.md`, `Content/Data/CraftingTree.json`
