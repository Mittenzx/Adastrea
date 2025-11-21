# Exploration System Guide

Complete guide for implementing exploration, scanning, and discovery in Adastrea.

**Version**: 1.0.0  
**Last Updated**: November 20, 2025  
**Status**: Phase 1 Complete (Scanner & Sensors)

---

## Table of Contents

1. [Overview](#overview)
2. [System Architecture](#system-architecture)
3. [Scanner Data Assets](#scanner-data-assets)
4. [Scanner Component](#scanner-component)
5. [Scannable Object Component](#scannable-object-component)
6. [Passive Scanning](#passive-scanning)
7. [Active Scanning](#active-scanning)
8. [Signal Identification](#signal-identification)
9. [Blueprint Integration](#blueprint-integration)
10. [YAML Templates](#yaml-templates)
11. [Best Practices](#best-practices)
12. [Troubleshooting](#troubleshooting)

---

## Overview

The Exploration System provides comprehensive scanning and discovery mechanics for space exploration including:

- **Passive Scanning**: Continuous detection of nearby objects
- **Active Scanning**: Targeted analysis revealing detailed information
- **Signal Identification**: Identify object types and characteristics
- **Discovery Tracking**: Track what has been scanned and discovered
- **Sensor Resolution**: Range and accuracy mechanics

### Key Features

✅ **Data-Driven Design**: All scanner configs via Data Assets  
✅ **Component-Based**: Modular scanner/scannable architecture  
✅ **Blueprint-First**: Full Blueprint exposure  
✅ **Performance-Conscious**: LOD-based scanning, efficient updates  
✅ **Flexible Detection**: Stealth, masking, signature strength  

### Core Gameplay Pillars

1. **Discovery**: Find new objects and locations
2. **Analysis**: Learn about objects through scanning
3. **Information Management**: Track and organize discoveries
4. **Tactical Advantage**: Better scanners = better intel

---

## System Architecture

### Class Hierarchy

```
UPrimaryDataAsset
└── ScannerDataAsset           (Scanner configuration)

UActorComponent
├── ScannerComponent            (Ship-mounted scanner)
└── ScannableObjectComponent    (Detectable object)
```

### Data Flow

```
Scanner Component (Passive Mode)
    ↓
Finds Scannable Objects in Range
    ↓
Checks Detection Requirements
    ↓
Adds to Detected Objects List
    ↓
Fires OnObjectDetected Event
    ↓
UI Updates with New Contact

Scanner Component (Active Mode)
    ↓
Target Selected for Scan
    ↓
Scan Progress Updates
    ↓
Scan Completes
    ↓
Marks Object as Scanned
    ↓
Fires OnScanComplete Event with Data
    ↓
UI Shows Detailed Information
```

### Integration Points

**With Ship Systems:**
- Mounted on spaceship actors
- Uses ship energy for active scans
- Upgrade path for better scanners

**With Combat:**
- Detect hostile ships
- Identify threat levels
- Target selection assistance

**With Exploration:**
- Discover anomalies
- Find resource locations
- Reveal hidden objects

**With Quest System:**
- Scan objectives
- Discovery triggers
- Information gathering missions

---

## Scanner Data Assets

### Creating a Scanner Data Asset

1. Right-click in Content Browser
2. Miscellaneous → Data Asset
3. Select `ScannerDataAsset` as parent class
4. Name: `DA_Scanner_[Type]` (e.g., `DA_Scanner_Advanced`)

### Scanner Types

#### Basic Scanner
```yaml
Scanner Name: "Basic Scanner"
Scanner Type: Basic
Passive Range: 1000000  # 10 km
Active Range: 5000000   # 50 km
Scan Duration: 5.0
Signal Identification Accuracy: 60
Can Deep Scan: false
```

**Use Case**: Starting ships, civilian vessels

#### Advanced Scanner
```yaml
Scanner Name: "Advanced Scanner"
Scanner Type: Advanced
Passive Range: 2000000  # 20 km
Active Range: 10000000  # 100 km
Scan Duration: 3.0
Signal Identification Accuracy: 80
Can Deep Scan: true
Deep Scan Duration Multiplier: 2.0
```

**Use Case**: Mid-tier exploration ships

#### Military Scanner
```yaml
Scanner Name: "Military Scanner"
Scanner Type: Military
Passive Range: 3000000  # 30 km
Active Range: 15000000  # 150 km
Scan Duration: 2.0
Signal Identification Accuracy: 90
Can Detect Stealth: true
Supported Signal Types: [Ship, Station, Debris]
```

**Use Case**: Combat ships, security vessels

#### Scientific Scanner
```yaml
Scanner Name: "Scientific Scanner"
Scanner Type: Scientific
Passive Range: 1500000  # 15 km
Active Range: 20000000  # 200 km
Scan Duration: 8.0
Signal Identification Accuracy: 95
Can Deep Scan: true
Deep Scan Duration Multiplier: 3.0
Detail Reveal Distance: 1000000  # 10 km
```

**Use Case**: Research vessels, anomaly hunters

#### Exploration Scanner
```yaml
Scanner Name: "Exploration Scanner"
Scanner Type: Exploration
Passive Range: 5000000  # 50 km
Active Range: 25000000  # 250 km
Scan Duration: 4.0
Signal Identification Accuracy: 75
Can Deep Scan: true
Minimum Object Size: 50  # Can detect smaller objects
Max Simultaneous Targets: 200
```

**Use Case**: Long-range explorers, survey ships

### Configuration Properties

#### Passive Scanning
- **Passive Range**: Max detection distance in cm
- **Passive Scan Interval**: Update frequency in seconds
- **Passive Detection Threshold**: Minimum signature strength (0-100)
- **Can Detect Stealth**: See through stealth technology

#### Active Scanning
- **Active Range**: Max active scan distance in cm
- **Scan Duration**: Time to complete scan in seconds
- **Scan Cooldown**: Time between scans in seconds
- **Energy Cost**: Energy required per scan
- **Can Deep Scan**: Capable of deep analysis
- **Deep Scan Duration Multiplier**: How much longer deep scans take

#### Signal Identification
- **Signal Identification Accuracy**: Accuracy % (0-100)
- **Detail Reveal Distance**: Distance for full detail in cm
- **Supported Signal Types**: Array of detectable types (empty = all)

#### Sensor Resolution
- **Minimum Object Size**: Smallest detectable size in cm
- **Max Simultaneous Targets**: Target tracking limit
- **Angular Resolution**: Precision in degrees

---

## Scanner Component

### Adding to Spaceship

1. Open spaceship Blueprint
2. Add Component → Scanner Component
3. Assign Scanner Data Asset
4. Enable Passive Scanning (checkbox)

### Configuration

```cpp
// In Blueprint Construction Script or BeginPlay
Scanner Component
├── Scanner Data: DA_Scanner_Advanced
├── Passive Scanning Enabled: true
└── Debug Show Detections: false (enable for testing)
```

### Passive Scanning

Runs automatically in background:

```cpp
// Automatically updates every PassiveScanInterval
// Detects objects within PassiveRange
// Fires OnObjectDetected for new contacts
// Fires OnObjectLost when objects leave range
```

**Blueprint Event: OnObjectDetected**
```cpp
Event OnObjectDetected(Detected Object)
├── Get Scan Data For Object
├── Add to UI Contact List
├── Play Detection Sound
└── Update Minimap
```

**Blueprint Event: OnObjectLost**
```cpp
Event OnObjectLost(Lost Object)
├── Remove from UI Contact List
└── Update Minimap
```

### Active Scanning

Targeted scan of specific object:

```cpp
// Start active scan
Start Active Scan
├── Target: [Selected Actor]
├── Scan Mode: Active (or DeepScan)
└── Returns: true if scan started

// Check if can scan
Can Start Active Scan
├── Target: [Actor]
├── Out Reason: [Text variable]
└── Returns: true if possible
```

**Blueprint Event: OnScanComplete**
```cpp
Event OnScanComplete(Scanned Object, Scan Data)
├── Display Scan Results UI
├── Update Database Entry
├── Grant Quest Progress (if applicable)
└── Play Success Sound
```

**Blueprint Event: OnScanFailed**
```cpp
Event OnScanFailed(Reason)
├── Display Error Message
└── Play Error Sound
```

### Querying Detections

```cpp
// Get all detected objects of type
Get Detected Objects By Type
├── Signal Type: Ship
└── Returns: Array of Detected Objects

// Get nearest object
Get Nearest Detected Object
└── Returns: Closest Detected Object

// Check if object is detected
Is Object Detected
├── Actor: [Target Actor]
└── Returns: true if in detection list

// Get scan progress
Get Scan Progress
└── Returns: 0-1 (0% to 100%)
```

---

## Scannable Object Component

### Adding to Objects

Any actor that should be detectable:

1. Add Component → Scannable Object Component
2. Configure basic properties
3. Set scan data for each detail level

### Basic Configuration

```cpp
Scannable Object Component
├── Display Name: "Cargo Freighter"
├── Signal Type: Ship
├── Signature Strength: 70
├── Object Size: 5000 (50 meters)
├── Passively Detectable: true
├── Stealth Enabled: false
└── Masked Signature: false
```

### Scan Data Levels

#### Basic Scan Data
Information revealed at long range or quick scan:

```cpp
Basic Scan Data
├── Object Name: "Unknown Ship"
├── Signal Type: Ship
├── Detail Level: Basic
├── Mass: 50000 kg
├── Composition: "Metallic"
└── Is Hostile: false
```

#### Detailed Scan Data
Information revealed at medium range or active scan:

```cpp
Detailed Scan Data
├── Object Name: "Cargo Freighter"
├── Signal Type: Ship
├── Detail Level: Detailed
├── Mass: 50000 kg
├── Composition: "Titanium alloy hull"
├── Faction ID: "merchants_guild"
├── Additional Details: "Carrying trade goods"
└── Is Hostile: false
```

#### Complete Scan Data
Information revealed by deep scan or close range:

```cpp
Complete Scan Data
├── Object Name: "Cargo Freighter 'Star Merchant'"
├── Signal Type: Ship
├── Detail Level: Complete
├── Mass: 52340 kg
├── Composition: "Titanium alloy hull, cargo: electronics"
├── Faction ID: "merchants_guild"
├── Additional Details: "Captain: John Smith, Destination: Station Alpha, Cargo Value: 50,000 credits"
└── Is Hostile: false
```

### Stealth & Masking

#### Stealth Configuration
```cpp
Stealth Enabled: true
Signature Strength: 20  # Reduced signature
Passively Detectable: false  # Only active scans can detect
```

**Detection**: Only scanners with `bCanDetectStealth = true` can detect

#### Signal Masking
```cpp
Masked Signature: true
Masked Signal Type: Debris  # Appears as debris
Signal Identification Accuracy: 75+  # Needed to see through
```

**Detection**: Low-accuracy scanners see masked type, high-accuracy see real type

### Blueprint Events

```cpp
Event OnFirstDetected()
├── Play Alert Sound
├── Log Discovery
└── Award Quest Progress

Event OnScanned(Detail Level)
├── Update Database
├── Grant Information Rewards
└── Trigger Related Events
```

---

## Passive Scanning

### How It Works

1. **Update Interval**: Scanner checks every `PassiveScanInterval` seconds
2. **Range Check**: Finds all scannable objects within `PassiveRange`
3. **Detection Check**: Validates each object against detection requirements
4. **Update List**: Adds new detections, updates existing, removes lost contacts
5. **Fire Events**: Triggers `OnObjectDetected` for new contacts

### Detection Requirements

For passive detection to succeed:

```cpp
✓ Object within Passive Range
✓ Object is Passively Detectable (flag)
✓ Signature Strength >= Passive Detection Threshold
✓ NOT stealthy (unless scanner has bCanDetectStealth)
✓ Scanner supports Signal Type (or supports all)
✓ Object Size >= Minimum Object Size
✓ Within Max Simultaneous Targets limit
```

### Performance Optimization

```cpp
// Passive scanning is efficient:
- Updates at configurable interval (default 2 seconds)
- Uses spatial queries (not full actor search)
- Respects max simultaneous targets limit
- No rendering or complex calculations
```

### Example: Contact List UI

```cpp
Event OnObjectDetected(Detected Object)
├── Create Contact Widget
├── Set Name: Get Scan Data → Object Name
├── Set Distance: Detected Object.Distance
├── Set Type Icon: Based on Signal Type
└── Add to Contact List

Event Tick
├── For Each Detected Object
│   ├── Update Distance Display
│   ├── Update Direction Indicator
│   └── Update Detail Level Icon
└── Sort by Distance
```

---

## Active Scanning

### Starting a Scan

```cpp
// Player presses scan button
On Scan Input
├── Get Targeted Actor
├── Can Start Active Scan?
│   ├── Yes: Start Active Scan
│   │   ├── Target: Targeted Actor
│   │   ├── Scan Mode: Active
│   │   └── Show Scan Progress UI
│   └── No: Show Error Message
└── Play UI Feedback
```

### Scan Progress

```cpp
Event Tick
├── Is Scan In Progress?
│   ├── Get Scan Progress (0-1)
│   ├── Update Progress Bar
│   ├── Show Scanning Animation
│   └── Display Time Remaining
└── If Progress = 1.0: Hide UI
```

### Deep Scanning

```cpp
// For detailed analysis (planets, anomalies)
On Deep Scan Input
├── Can Start Active Scan?
│   ├── Scanner Has bCanDeepScan?
│   │   ├── Yes: Start Active Scan
│   │   │   ├── Target: Target
│   │   │   ├── Scan Mode: DeepScan
│   │   │   └── Duration = Base * Multiplier
│   │   └── No: Show "Upgrade Required"
│   └── Show Scan UI
```

### Scan Results

```cpp
Event OnScanComplete(Scanned Object, Scan Data)
├── Display Results Panel
│   ├── Object Name
│   ├── Signal Type
│   ├── Mass
│   ├── Composition
│   ├── Faction (if any)
│   └── Additional Details
├── Update Discovery Database
├── Grant Experience/Rewards
└── Check Quest Objectives
```

### Interruption Handling

```cpp
// Scan can be interrupted by:
- Ship taking damage
- Target destroyed or out of range
- Player cancels scan
- Ship enters combat

Event OnScanFailed(Reason)
├── Hide Scan UI
├── Display Failure Message
└── Allow new scan (cooldown applies)
```

---

## Signal Identification

### Signal Types

```cpp
enum ESignalType
{
    Unknown,        // Unidentified
    Ship,           // Spacecraft
    Station,        // Space station
    Planet,         // Planetary body
    Asteroid,       // Asteroid or rock
    Anomaly,        // Spatial anomaly
    Debris,         // Wreckage or junk
    Beacon          // Active beacon
}
```

### Identification Process

1. **Detection**: Object appears on sensors
2. **Classification**: Signal type identified based on scanner accuracy
3. **Verification**: Active scan confirms or reveals true type
4. **Database Entry**: Logged in player's discovery database

### Accuracy Mechanics

```cpp
Signal Identification Accuracy (0-100%)
├── 0-40%: Often misidentifies signals
├── 41-60%: Basic identification, some errors
├── 61-80%: Reliable identification
├── 81-95%: Sees through masking sometimes
└── 96-100%: Always accurate, sees through masking
```

### Example: Target Classification UI

```cpp
Get Visible Signal Type(Detected Object)
├── Scanner Accuracy < 75 && Object Masked
│   └── Return: Masked Signal Type (deception)
├── Scanner Accuracy >= 75 && Object Masked
│   └── Return: True Signal Type (sees through)
└── Return: True Signal Type

Display in UI:
├── Unknown: Grey question mark icon
├── Ship: Ship icon (color by faction)
├── Station: Station icon
├── Planet: Planet icon
├── Asteroid: Rock icon
├── Anomaly: Special icon (glowing)
├── Debris: Debris icon
└── Beacon: Beacon icon (pulsing)
```

---

## Blueprint Integration

### Complete Scanner Setup Example

```cpp
// In Spaceship Blueprint

Components:
└── Scanner Component (Scanner)

Construction Script:
├── Scanner → Set Scanner Data: DA_Scanner_Advanced
└── Scanner → Set Passive Scanning Enabled: true

Event BeginPlay:
└── Scanner → OnObjectDetected → Add Dynamic
    └── Bind to: HandleObjectDetected

Event HandleObjectDetected(Detected Object):
├── Get Scan Data For Object(Detected Object)
├── Print String: "Detected: {Object Name}"
├── Add to Contact List Widget
└── Play Detection Sound

Event OnScanButtonPressed:
├── Get Targeted Actor
├── Scanner → Can Start Active Scan?
│   ├── True: Scanner → Start Active Scan
│   │   ├── Target: Targeted Actor
│   │   └── Scan Mode: Active
│   └── False: Show Error UI
└── Update UI

Event OnScanComplete(Scanned Object, Scan Data):
├── Display Scan Results Panel
│   ├── Title: Scan Data.Object Name
│   ├── Type: Scan Data.Signal Type
│   ├── Details: Scan Data.Additional Details
│   └── Faction: Scan Data.Faction ID
└── Grant Quest Progress (if applicable)
```

### HUD Integration

```cpp
// Scanner HUD Widget

Variables:
├── Scanner Component Reference
└── Detected Objects Array

Event Construct:
└── Get Owner → Get Component → Scanner Component

Event Tick:
├── Scanner → Get Detected Objects By Type(Ship)
├── Update Contact List
│   ├── For Each Object
│   │   ├── Get Distance
│   │   ├── Get Direction
│   │   ├── Get Signal Type
│   │   └── Update Contact Widget
│   └── Sort by Distance
└── Scanner → Get Scan Progress
    └── Update Scan Progress Bar
```

---

## YAML Templates

### Basic Scanner Template

```yaml
scanner_name: "Basic Scanner"
description: "Standard ship scanner with basic detection capabilities"
scanner_type: Basic

passive_scanning:
  range: 1000000  # 10 km
  scan_interval: 2.0
  detection_threshold: 50
  can_detect_stealth: false

active_scanning:
  range: 5000000  # 50 km
  scan_duration: 5.0
  scan_cooldown: 10.0
  energy_cost: 50
  can_deep_scan: false

signal_identification:
  accuracy: 60
  detail_reveal_distance: 500000  # 5 km

sensor_resolution:
  minimum_object_size: 100  # 1 meter
  max_simultaneous_targets: 50
  angular_resolution: 5.0
```

### Scannable Ship Template

```yaml
display_name: "Cargo Freighter"
signal_type: Ship
signature_strength: 70
object_size: 5000  # 50 meters
passively_detectable: true
stealth_enabled: false

basic_scan_data:
  object_name: "Unknown Ship"
  signal_type: Ship
  mass: 50000
  composition: "Metallic"
  is_hostile: false

detailed_scan_data:
  object_name: "Cargo Freighter"
  signal_type: Ship
  mass: 50000
  composition: "Titanium alloy hull"
  faction_id: "merchants_guild"
  additional_details: "Carrying trade goods"
  is_hostile: false

complete_scan_data:
  object_name: "Cargo Freighter 'Star Merchant'"
  signal_type: Ship
  mass: 52340
  composition: "Titanium alloy hull, cargo: electronics"
  faction_id: "merchants_guild"
  additional_details: "Captain: John Smith, Destination: Station Alpha, Cargo Value: 50,000 credits"
  is_hostile: false
```

### Anomaly Template

```yaml
display_name: "Spatial Anomaly"
signal_type: Anomaly
signature_strength: 90
object_size: 10000  # 100 meters
passively_detectable: true

basic_scan_data:
  object_name: "Unknown Energy Signature"
  signal_type: Anomaly
  detail_level: Basic

detailed_scan_data:
  object_name: "Quantum Fluctuation"
  signal_type: Anomaly
  composition: "Exotic energy readings"
  additional_details: "Appears to be a temporal distortion"
  detail_level: Detailed

complete_scan_data:
  object_name: "Stable Wormhole"
  signal_type: Anomaly
  composition: "Exotic matter stabilizing space-time tear"
  additional_details: "Destination: Unknown sector. Navigable. High radiation levels."
  detail_level: Complete
```

---

## Best Practices

### Scanner Design

1. **Progression System**: Start with Basic, upgrade to Advanced/Specialized
2. **Trade-offs**: Military = combat focus, Scientific = analysis focus, Exploration = range
3. **Energy Management**: Balance scan frequency with ship power
4. **Cooldowns**: Prevent spam scanning, add tactical decision-making

### Scannable Objects

1. **Progressive Revelation**: Basic → Detailed → Complete information
2. **Meaningful Detail Levels**: Each level should add useful information
3. **Context-Appropriate**: Military scanners reveal tactical data, scientific reveal composition
4. **Quest Integration**: Use scan objectives for information gathering missions

### Performance

1. **Update Frequency**: Balance between responsiveness and CPU usage
2. **Range Limits**: Don't scan entire map, use reasonable ranges
3. **Target Limits**: Cap max simultaneous targets to prevent slowdown
4. **LOD Awareness**: Disable scanning when far from action

### UI/UX

1. **Clear Feedback**: Show scan progress, detection ranges, cooldowns
2. **Visual Hierarchy**: Important contacts stand out
3. **Information Density**: Don't overwhelm player with too much data
4. **Audio Cues**: Distinctive sounds for detection, scan complete, failures

---

## Troubleshooting

### Objects Not Detected

**Problem**: Scannable objects not appearing in passive scan

**Solutions**:
- ✓ Check PassiveScanningEnabled is true
- ✓ Verify object is within PassiveRange
- ✓ Check SignatureStrength >= PassiveDetectionThreshold
- ✓ Ensure bPassivelyDetectable is true on scannable component
- ✓ Verify scanner supports signal type (or empty array = all)
- ✓ Check ObjectSize >= MinimumObjectSize

### Active Scan Won't Start

**Problem**: Cannot initiate active scan

**Solutions**:
- ✓ Check ScanCooldownRemaining = 0
- ✓ Verify no scan currently in progress
- ✓ Ensure target is within ActiveRange
- ✓ Confirm target has ScannableObjectComponent
- ✓ Check target is valid (not destroyed)

### Stealth Not Working

**Problem**: Stealth ships still being detected

**Solutions**:
- ✓ Verify bStealthEnabled is true on scannable component
- ✓ Check scanner doesn't have bCanDetectStealth
- ✓ Reduce SignatureStrength to very low value
- ✓ Set bPassivelyDetectable to false

### Performance Issues

**Problem**: Game slowing down during scanning

**Solutions**:
- ✓ Increase PassiveScanInterval (reduce update frequency)
- ✓ Reduce PassiveRange and ActiveRange
- ✓ Lower MaxSimultaneousTargets
- ✓ Disable bDebugShowDetections in production

### Signal Masking Not Working

**Problem**: Masked signals showing true type

**Solutions**:
- ✓ Check bMaskedSignature is true
- ✓ Verify MaskedSignalType is set
- ✓ Lower scanner SignalIdentificationAccuracy
- ✓ Ensure scanner accuracy < 75 for masking to work

---

## Next Steps

### Phase 2: Discovery System (Q1 2026)

Planned features:
- Anomaly system with rewards
- Discovery database and tracking
- Rarity tiers and special discoveries
- Archaeological sites
- Derelict ships and stations

### Phase 3: Resource Gathering (Q1-Q2 2026)

Planned features:
- Mining mechanics integration
- Asteroid mining with scanners
- Gas giant harvesting
- Salvaging derelicts
- Resource quality scanning

### Phase 4: Cartography (Q2 2026)

Planned features:
- Star map integration
- Discovery vs undiscovered regions
- Navigation markers from scans
- Jump route mapping
- Player-created maps

---

## Related Documentation

- [ROADMAP.md](../ROADMAP.md) - Project roadmap and status
- [NavigationSystemGuide.md](NavigationSystemGuide.md) - Navigation integration
- [QuestSystemGuide.md](QuestSystemGuide.md) - Quest objectives with scanning
- [CombatSystemGuide.md](CombatSystemGuide.md) - Target selection with scanners

---

**Built with ❤️ for the Adastrea community**

*"Explore the unknown, discover the stars."*
