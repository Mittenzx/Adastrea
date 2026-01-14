# Checklist of Errors (Sorted by Type & Location)

## Type: Operator & Assignment Errors

- [ ] **C2679**: No operator '=' found for assigning `TArray<FPlayerFactionReputation, FDefaultAllocator>`
  - **File**: Source\Adastrea\Player\SaveGameSubsystem.cpp
  - **Line**: 455

---

## Type: Local Function Definition Errors

All `C2601` errors indicate local function definitions inside another function, which is illegal in C++.

- [ ] **C2601**: 'ASpaceship::ToggleFlightAssist'
  - **File**: Source\Adastrea\Ships\Spaceship.cpp
  - **Line**: 398
- [ ] **C2601**: 'ASpaceship::ThrottleUp'
  - **File**: Source\Adastrea\Ships\Spaceship.cpp
  - **Line**: 409
- [ ] **C2601**: 'ASpaceship::ThrottleDown'
  - **File**: Source\Adastrea\Ships\Spaceship.cpp
  - **Line**: 414
- [ ] **C2601**: 'ASpaceship::SetThrottle'
  - **File**: Source\Adastrea\Ships\Spaceship.cpp
  - **Line**: 419
- [ ] **C2601**: 'ASpaceship::ActivateBoost'
  - **File**: Source\Adastrea\Ships\Spaceship.cpp
  - **Line**: 424
- [ ] **C2601**: 'ASpaceship::DeactivateBoost'
  - **File**: Source\Adastrea\Ships\Spaceship.cpp
  - **Line**: 439
- [ ] **C2601**: 'ASpaceship::ToggleTravelMode'
  - **File**: Source\Adastrea\Ships\Spaceship.cpp
  - **Line**: 450
- [ ] **C2601**: 'ASpaceship::GetEffectiveMaxSpeed'
  - **File**: Source\Adastrea\Ships\Spaceship.cpp
  - **Line**: 476
- [ ] **C2601**: 'ASpaceship::ApplyFlightAssist'
  - **File**: Source\Adastrea\Ships\Spaceship.cpp
  - **Line**: 492
- [ ] **C2601**: 'ASpaceship::ApplyAutoLeveling'
  - **File**: Source\Adastrea\Ships\Spaceship.cpp
  - **Line**: 543
- [ ] **C2601**: 'ASpaceship::UpdateThrottleVelocity'
  - **File**: Source\Adastrea\Ships\Spaceship.cpp
  - **Line**: 573

---

## Type: Syntax Error

- [ ] **C2059**: Syntax error: ';'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 7

---

## Type: Undeclared Identifiers

All `C2065` errors indicate a variable/identifier not declared in scope.

- [ ] **C2065**: 'ShipName'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 7
- [ ] **C2065**: 'Description'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 8
- [ ] **C2065**: 'ShipClass'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 9
- [ ] **C2065**: 'ShipID'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 10
- [ ] **C2065**: 'HullStrength'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 13
- [ ] **C2065**: 'CargoCapacity'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 14
- [ ] **C2065**: 'CrewRequired'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 15
- [ ] **C2065**: 'MaxCrew'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 16
- [ ] **C2065**: 'ModularPoints'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 17
- [ ] **C2065**: 'ArmorRating'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 20
- [ ] **C2065**: 'ShieldStrength'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 21
- [ ] **C2065**: 'ShieldRechargeRate'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 22
- [ ] **C2065**: 'WeaponSlots'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 23
- [ ] **C2065**: 'WeaponPowerCapacity'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 24
- [ ] **C2065**: 'PointDefenseRating'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 25
- [ ] **C2065**: 'MaxSpeed'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 28
- [ ] **C2065**: 'Acceleration'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 29
- [ ] **C2065**: 'Maneuverability'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 30
- [ ] **C2065**: 'JumpRange'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 31
- [ ] **C2065**: 'FuelCapacity'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 32
- [ ] **C2065**: 'FuelConsumptionRate'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 33
- [ ] **C2065**: 'SensorRange'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 36
- [ ] **C2065**: 'SensorResolution'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 37
- [ ] **C2065**: 'StealthRating'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 38
- [ ] **C2065**: 'RepairSystemRating'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 39
- [ ] **C2065**: 'ScienceRating'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 40
- [ ] **C2065**: 'MedicalRating'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 41
- [ ] **C2065**: 'PowerCapacity'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 44
- [ ] **C2065**: 'LifeSupportRating'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 45
- [ ] **C2065**: 'MaintenanceLevel'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 46
- [ ] **C2065**: 'HangarCapacity'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 47
- [ ] **C2065**: 'DroneCapacity'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 48
- [ ] **C2065**: 'AISystemRating'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 49
- [ ] **C2065**: 'DiplomacyRating'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 52
- [ ] **C2065**: 'BoardingCapability'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 53
- [ ] **C2065**: 'CustomizableSlots'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 54
- [ ] **C2065**: 'EWarRating'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 55
- [ ] **C2065**: 'MiningRating'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 56
- [ ] **C2065**: 'Manufacturer'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 59
- [ ] **C2065**: 'YearBuilt'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 60
- [ ] **C2065**: 'RarityTier'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 61
- [ ] **C2065**: 'LoreNotes'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 62
- [ ] **C2065**: 'PrimaryColor'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 63
- [ ] **C2065**: 'SecondaryColor'
  - **File**: Source\Adastrea\Ships\SpaceshipDataAsset.cpp
  - **Line**: 64

---

## Suppression State
All errors currently **not suppressed**.
