# Adastrea — Research & Breakthrough Tree
*Derived from `Content/Data/CraftingTree.json` — single source of truth.*
| Research Level | Meaning |
|---|---|
| 1 | Base — no research required |
| 2 | Unlocks **Mk2** upgrades |
| 3 | Unlocks **Mk3** upgrades |
## Research Branches
Different **science/research labs** produce **different breakthroughs**. Each branch: build its lab module → research the rl2 breakthrough (unlocks Mk2) → research the rl3 breakthrough (unlocks Mk3).
### Propulsion — `PhysicsLab` (`PhysicsLabModule`)
- **`AdvancedPropulsionResearch`** — *Advanced Propulsion Engines* (Mk3) — unlocks 4 crafts: Fuel Tank Mk3, Ship Engine Mk3, Ship Reactor Mk3, Thruster Mk3
- **`PropulsionResearch`** — *Propulsion Research* (Mk2) — unlocks 4 crafts: Fuel Tank Mk2, Ship Engine Mk2, Ship Reactor Mk2, Thruster Mk2
### Materials — `MaterialsLab` (`MaterialsLabModule`)
- **`MaterialsResearch`** — *Materials Research* (Mk2) — unlocks 5 crafts: Armour Hull Mk2, Cargo Pod Mk2, Hull Plating Mk2, Life Support Pod Mk2, Ship Components Mk2
- **`NanoMaterialsResearch`** — *Nano-Materials* (Mk3) — unlocks 5 crafts: Armour Hull Mk3, Cargo Pod Mk3, Hull Plating Mk3, Life Support Pod Mk3, Ship Components Mk3
### Computing — `ElectronicsLab` (`ElectronicsLabModule`)
- **`ComputingResearch`** — *Computing Research* (Mk2) — unlocks 33 crafts: Advanced Sensors Mk2, Automated Defence Computer Mk2, Basic Computer Mk2, Capacitor Bank Mk2, Cargo Computer Mk2, Circuit Board Mk2, Clock Generator Mk2, Control Unit Mk2, Data Matrix Mk2, Electronics Mk2, Encrypted Circuit Mk2, Engineering Computer Mk2, Fiber Optic Cable Mk2, Logic Chip Mk2, Memory Unit Mk2, Micro Capacitor Mk2, Microchips Mk2, Navigation Computer Mk2, Optical Lens Mk2, Pilot Computer Mk2, Power Distribution Unit Mk2, Quantum Processor Mk2, Research Computer Mk2, Scan Computer Mk2, Science Computer Mk2, Sensor Array Mk2, Servo Actuator Mk2, Ship Computer Mk2, Signal Amplifier Mk2, Super Conducting Wire Mk2, Targeting Computer Mk2, Trading Computer Mk2, Transceiver Unit Mk2
- **`QuantumComputingResearch`** — *Quantum Computing* (Mk3) — unlocks 15 crafts: Advanced Sensors Mk3, Automated Defence Computer Mk3, Basic Computer Mk3, Circuit Board Mk3, Control Unit Mk3, Electronics Mk3, Logic Chip Mk3, Memory Unit Mk3, Microchips Mk3, Power Distribution Unit Mk3, Quantum Processor Mk3, Research Computer Mk3, Science Computer Mk3, Sensor Array Mk3, Ship Computer Mk3
### Defence — `WeaponsLab` (`WeaponsLabModule`)
- **`AdvancedDefenceResearch`** — *Advanced Defence Systems* (Mk3) — unlocks 3 crafts: Energy Cannon Mk3, Ship Shield Generator Mk3, Turret Weapon Mk3
- **`DefenceResearch`** — *Defence Research* (Mk2) — unlocks 5 crafts: Energy Cannon Mk2, Gravitation Generator Mk2, Shield Capacitor Mk2, Ship Shield Generator Mk2, Turret Weapon Mk2
### Bio — `BiologyLab` (`BiologyLabModule`)
- **`AdvancedBioResearch`** — *Advanced Bio-Tech* (Mk3) — unlocks 2 crafts: Medical Supplies Mk3, Refined Medicine Mk3
- **`BioResearch`** — *Biotech Research* (Mk2) — unlocks 3 crafts: Medical Supplies Mk2, Refined Medicine Mk2, Vaccines Mk2
### Projectile Weapons — `ProjectileWeaponsLab` (`ProjectileWeaponsLab`)
- **`KineticWeaponResearch`** — *Kinetic Weapons* (Mk3) — unlocks 6 crafts: Missile Launcher Mk2, Missile Launcher Mk3, Railgun Mk2, Railgun Mk3, Torpedo Launcher Mk2, Torpedo Launcher Mk3
### Beam Weapons — `BeamWeaponsLab` (`BeamWeaponsLab`)
- **`BeamWeaponResearch`** — *Beam Weapons* (Mk3) — unlocks 6 crafts: Plasma Cannon Mk2, Plasma Cannon Mk3, Point Defence Laser Mk2, Point Defence Laser Mk3, Tri Laser Mk2, Tri Laser Mk3
## Cross-Lab Coupling
Reverse-engineering the crafting tree shows the strong couplings: several rl3 breakthroughs and *every* computer-family item require `QuantumProcessor_Mk2` (unlocked by ElectronicsLab **ComputingResearch**). So maxing one lab is impossible — late-game tech spans multiple labs.
## Research Items (all)
| ResearchID | Name | Level | Lab | Unlocks |
|---|---|---|---|---|
| `AdvancedBioResearch` | Advanced Bio-Tech | 3 | BiologyLab | 2 |
| `AdvancedDefenceResearch` | Advanced Defence Systems | 3 | WeaponsLab | 3 |
| `AdvancedPropulsionResearch` | Advanced Propulsion Engines | 3 | PhysicsLab | 4 |
| `BeamWeaponResearch` | Beam Weapons | 3 | BeamWeaponsLab | 6 |
| `BioResearch` | Biotech Research | 2 | BiologyLab | 3 |
| `ComputingResearch` | Computing Research | 2 | ElectronicsLab | 33 |
| `DefenceResearch` | Defence Research | 2 | WeaponsLab | 5 |
| `KineticWeaponResearch` | Kinetic Weapons | 3 | ProjectileWeaponsLab | 6 |
| `MaterialsResearch` | Materials Research | 2 | MaterialsLab | 5 |
| `NanoMaterialsResearch` | Nano-Materials | 3 | MaterialsLab | 5 |
| `PropulsionResearch` | Propulsion Research | 2 | PhysicsLab | 4 |
| `QuantumComputingResearch` | Quantum Computing | 3 | ElectronicsLab | 15 |
| `ResearchData` | Research Data | 2 | ScienceLab | 0 |
