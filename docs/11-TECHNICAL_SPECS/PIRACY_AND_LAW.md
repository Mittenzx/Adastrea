# Piracy and Law

Design for what combat is *for*: who attacks whom, why, and what happens afterwards.
Builds on the combat rebuild (guns, shields, wrecks, station turrets, player rescue) and on
`UOrganisationSubsystem` (Authorities, independent orgs, owned ship records).

Status: design agreed 2026-10-02. Built so far: steps 1 and 2 of the build order (marked **Built**).

## Setting: people are the scarcest resource

See [`../SETTING.md`](../SETTING.md). The game takes place in remote galaxies far from the old
empires' wars, which left few people alive. **People are worth more than ships or cargo**, so
**nobody kills unless they have to**, not even pirates. Everything below follows from that:

- **Pirates want people as well as cargo.** Captured crew are put to work, ransomed back to their
  organisation or Authority, or recruited. A gang's strength is how many people it has, not just
  its ships.
- **Escape pods matter.** Picking up pods is a job: returning them to their organisation or
  Authority earns a reward, and pirates collect them as captives. Leaving pods to drift is wasteful
  rather than cruel.
- **Killing is the worst crime there is,** for lawful people and pirates alike. A killer loses
  standing everywhere, including with pirate gangs, who won't deal with someone who wastes people.
  Destroying an escape pod counts as killing.
- **Death is rare and costly in the story.** The kill switch's destroy setting is the pirates'
  last resort, used only when an escape would expose them.

## Ground rules (already decided)

- Ships are never blown up. At zero hull they become disabled, drifting wrecks.
- Stations are invulnerable to combat for now.
- Drones do utility work (cargo transfer, towing). No tractor or repair beams.

## 1. Security comes from who governs the space

- Each sector has a **security level**, set by the Authority that governs it: high near the
  Authority's seat, lower toward the edges, none in ungoverned sectors.
- High security means station turrets plus **Authority patrols** (`EShipRole::Patrol`, which has
  no AI yet) that answer distress beacons quickly. Low security means a slow answer; none means
  no answer.
- Being safe near certain stations follows from this, and the map can show it.
- **Built (step 1):** a sector's level comes from the `security` field in `Galaxy.json` (already
  shown on the galaxy map). `UDistressSubsystem` raises a call when a hostile shoots a Civil ship
  and sends the nearest free `APatrolController`s: High up to 3 at once, Medium 2 after 15 s,
  Low 1 after 40 s, None nobody. Patrols also attack hostiles within 250 m without a call. In
  this first version security is set per sector; it doesn't yet fall off with distance from the
  seat station.

## 2. Pirates are organisations with a goal

- Pirates are independent organisations (no `ParentId`) with a home base, a wallet and a set of
  **needs**. They raid to meet those needs, not just to earn.
- Encounter: hail and demand → if refused, **disable** (never destroy) → drones strip what they
  came for → retreat when their own shields fail.
- A gang that keeps losing ships, crew or supplies can't replace them and fades out.

### 2a. What pirates need

Cash is weak for pirates. They can't dock at lawful stations, so credits only buy things at fences
and guild hubs, where pirates pay a premium. Things they can use directly, or can't get any other
way, are worth more. Roughly most valuable first:

| Need | Why it matters | Where they get it |
|---|---|---|
| **People** | The scarcest thing there is. A gang can't grow without crew. | Captured crews, collected escape pods, recruits |
| **Access** | They can't dock at lawful stations | A captured player with clean standing, sent to buy for them |
| **Ships and parts** | Can't buy hulls legally; keeps the fleet flying | Boarded ships, salvage from wrecks, drones |
| **Supplies** | Fuel, food, ammunition and repair materials keep the base running | Tankers and freighters, fences |
| **Information** | Routes, manifests, patrol timings, where the hubs are | Prisoners, spies, scouting jobs |
| **Cash and ransom** | Useful, but turned into goods at poor rates | Ransoming captives, selling at fences |

- Each gang tracks a **level for each need** (an inventory alongside its wallet). Levels fall over
  time with upkeep and losses, and rise from raids and trades.
- **What they're short of decides their targets.** A raider scores targets by how well they fill
  its gang's biggest shortage, weighed against risk (security level, the target's escort, how far
  from home):
  - short of fuel → tankers;
  - short of crew → crewed ships and drifting pods;
  - short of parts → wrecks and lightly armed ships;
  - short of information → scouts and couriers.
  Cargo value still counts, through what the gang can trade it for.
- **The demand matches the need.** A hail asks for what they lack ("fuel, and we let you go"), so
  the player can see why they're being stopped, and can pay in kind instead of fighting.
- **Desperation.** The longer a shortage lasts, the more risk a gang accepts: it raids nearer to
  lawful space, takes on better-armed targets, and makes harsher demands. Cutting a gang off from
  its supplies is a way to beat it without fighting it, though it gets dangerous before it fails.
- **A captured player's work follows the gang's needs:** hauling fuel, running purchases at lawful
  stations, scouting routes, towing wrecks. That's why the debt is left for later: what a captive
  owes depends on what they're useful for.
- This uses systems that already exist: the economy and crafting tree supply the goods, drones
  are equipment they want, and organisation wallets already track their finances.

### 2b. Built (step 2)

- **Gangs.** An organisation with a `pirate` block in `Organisations.json` is a gang
  (`UPirateSubsystem`): the Cinder Wolves (home: the Relay, Low security) and the Ashen Reach
  (Varos Gauntlet, None). The block gives starting levels (0-100) for the six needs and,
  optionally, how much each falls per economy tick. Levels are saved with the organisations.
  Pirates never buy ships at lawful yards.
- **Levels.** They fall every economy tick (upkeep) and when a raider is disabled (parts -12,
  people -4). Raids raise them: cargo maps to a need by item category and name (fuel, food,
  water, medicine: supplies; components, alloys, electronics: parts; data: information;
  anything else is fenced for cash at a poor rate), and stripping a wreck gives parts.
  People and access are tracked but nothing fills them until boarding and capture.
- **Strength.** Parts decide how many raiders a gang can send (1 to 3; none below 8, so a gang
  that keeps losing ships fades out).
- **Desperation.** While the biggest raidable shortage stays below 30 it builds from 0 to 1
  over 10 minutes (and eases twice as fast once the shortage is filled). It divides the cost of
  risk by up to 3, raises the share demanded from half the cargo to all of it, and widens
  auto-raids' reach from 2 to 4 gate hops from home.
- **Choosing a target** (`URaidSubsystem`): every ship in the level that isn't a raider or a
  patrol. Value is what its cargo (and a cut of its credits, when short of cash) is worth in
  need points, weighed by shortage and importance; AI traders x1.25; wrecks add salvage.
  Risk: sector security, patrols within 3 km, an armed target (the player), gate hops from home.
- **Encounter.** Raiders lie in wait ahead of a moving target, or of where an AI trader is
  heading (they fly no faster than their prey). They hail over comms from 2.5 km, close in and
  hold 300 m off it. The hail names the need ("We're
  running short of fuel and food. Hand over 20 Helium-3 and we let you go.").
  - The player sees a **comms panel**: the gang's name in its colour, the line, what they want,
    a 20 s countdown, **J comply / K refuse**. Silence, or shooting a raider, is a refusal.
  - AI traders decide in 3 s: more likely to refuse with patrols near or in secure space.
  - **Comply:** the target stops (AI traders hold still), and the raiders' **loot drones**
    (`ALootDrone`) fly over, clamp on, take the demand and fly it back. Moving more than 1 km
    from where you agreed counts as refusing.
  - **Refuse:** the raiders disable the target (never destroy it), then drones strip the wreck's
    whole hold plus salvage.
  - **Retreat:** a raider whose shields fail breaks off and runs; once all have, or the job is
    done, they leave the level (despawn 3 km out). Raiders fight back against whoever shoots
    them. Patrols treat them as hostiles, so distress calls still work.
- **Auto-raids** (not in CombatTest): every minute after the first two, each gang with ships to
  spare and within reach may raid, more readily in lawless space and when desperate.
- **Console:** `adastrea.PirateInfo`, `adastrea.PirateNeed Gang|all Need Level`,
  `adastrea.PirateDesperation Gang|all 0-1`, `adastrea.PirateUpkeep [N]`, `adastrea.PirateUpkeepScale`,
  `adastrea.Raid [Gang] [TargetName] [DistanceM]`, `adastrea.ClearRaids`, `adastrea.AutoRaids 0/1`,
  `adastrea.RaidAIComply -1/0/1`, `adastrea.RaidTrader Item Units [DistanceM]`, `RaidComply`,
  `RaidRefuse`. Test: `Tools/pie_raid_test.py` in CombatTest (scoring, AI comply, AI refuse,
  player comply, player refuse with raiders breaking off). A trader that refuses can still
  shelter under a station: bolts stop at the station's shields.
- **Not yet:** stolen tags on taken cargo (step 3), standing (step 4), boarding and captives
  (step 5), so pods and people aren't targets yet. Raiders spawn at the edge of the level rather
  than flying from a base.

## 3. Stolen cargo

- Cargo taken by force carries a **stolen** tag and its rightful owner organisation.
- Lawful stations scan on docking: confiscate, fine, lower standing with that Authority.
- **Fences** (independent stations and guild hub ships) buy stolen cargo at a discount.

## 4. Boarding and capture

- **Boarding is automatic.** When a ship is disabled and a ship that can board (a boarding
  capability on the ship's data) comes within range, the boarding happens. There's no on-foot
  fight in this version.
- The crew of a boarded ship eject in pods (the escape pod from player rescue).
- Ownership of the boarded ship passes to the boarder's organisation, but it stays **registered**
  to the old owner. Flying it near lawful stations gets you flagged.
- Independent shipyards re-register, repaint and refit stolen ships for a fee.

## 5. When the player is captured

- If the player's ship is disabled and a pirate that can board reaches it before the rescue tow
  drone does, the player is **captured**. In secure space the tow drone usually wins; in lawless
  space the pirates do.
- The pirates **keep the player's ship**. It becomes theirs.
- The player is put to **work for the pirates** (hauling stolen cargo to fences, scouting trade
  lanes, towing wrecks) in a basic ship the pirates provide.
- The basic ship has a **kill switch**: try to leave before you've earned your way out and it
  is set off. What it does depends on **how much the escape threatens the pirates**:
  - **Low risk** (still in their space, nowhere near help): the ship is **disabled**. They
    collect you and add to the debt.
  - **High risk** (heading for Authority space or a patrol, or you know where their base is): the
    ship is **destroyed**. This is the one deliberate exception to "ships are never blown up". It
    is the pirates' doing, not normal combat.
  - The player should be able to read the risk (warnings on the kill-switch ship), so a run is a
    gamble they choose to take rather than a surprise.
- When the debt is worked off, the player is free to go.

## 6. The player as pirate

- Combat sides stop being fixed labels (today the player, traders and turrets are all `Civil`)
  and become **standing with each organisation**, so the player can attack anyone.
- Crimes count only when they're witnessed: a patrol, a turret, or a victim whose pod gets away
  and reports it.
- Low standing with an Authority leads to scans, fines, then being refused docking and hunted in
  its sectors.
- A pirate player boards with a ship that can board, sells at fences, and re-registers prizes at
  independent yards.

## 7. Guild hub ships

- Each neutral guild has **one** large mobile base instead of a home station. Smaller ships dock
  with it to trade, resupply and repair.
- Each hub has **its own rules on board** (for example: no stolen cargo, no weapons fire within
  range, a docking fee, members only, or anything goes). The rules are data on the guild.
- Hubs move around, mostly through low-security space, so finding one is part of the game. Many
  of them act as fences.
- Cargo between the hub and docked ships moves by drones.

## Build order

1. Sector security level and Authority patrols that answer beacons
2. **Built.** Pirate organisation with need levels; raider AI: choose a target by need → demand
   → disable → drone loot → retreat
3. Stolen tag on cargo, lawful station scans, fence markets
4. Standing with each organisation instead of fixed sides; witnesses; the player can turn pirate
5. Automatic boarding, change of ownership, registration, independent refit yards
6. Player capture: kill-switch ship and working off the debt
7. Guild hub ships with on-board rules

## Open questions

- **Kill-switch death.** What happens when the high-risk kill switch destroys the player's ship?
  Proposed, not yet agreed: **succession.** You continue as a new captain in the same world and
  inherit the old captain's estate minus a large cut. Reputation isn't inherited, and a
  criminal's estate is seized in lawful space, so dying never clears a wanted level for free.
  Dying must cost more than working off the debt, or running becomes a free way out.
- **Debt (left for later, by choice).** Many things can affect it, including what the gang needs
  and what the captive is useful for. Can a friend or Authority buy the player out?
- **Buying back.** Can the player later buy or steal back their old ship from the pirates?
- **Hub ships.** Can the hub ships themselves be attacked (they're ships, not stations), or are
  they protected the way stations are?
