# Sector Generator Quick Start

Get up and running with the Sector Generator in 5 minutes.

## 1. Create a Config Data Asset (2 minutes)

1. Content Browser → Right-click → **Blueprint Class**
2. Search for `SectorGeneratorConfig`
3. Name it: `DA_SectorConfig_Test`
4. Open the asset

## 2. Configure Basic Settings (2 minutes)

### Required Settings
```
Config Name: "Test Sector"
Naming Theme: Greek
Random Seed: 0 (random)
Density Multiplier: 1.0
Min Object Spacing: 50000.0
```

### Add One Object Definition

Click **+ Add** under Object Definitions:

```
Object Type: Asteroid
Actor Class: BP_Asteroid_Single (or your asteroid BP)
Min Count: 5
Max Count: 10
Distribution Pattern: Random
Scale Variation: 2.0
Random Rotation: true
```

## 3. Place Generator in Level (1 minute)

1. Open your sector map level
2. Find the `ASpaceSectorMap` actor (your sector boundaries)
3. Place Actors → Search `SectorGenerator`
4. Drag into viewport
5. Select it in outliner

## 4. Configure Generator (30 seconds)

In Details panel:
```
Target Sector: [Select your ASpaceSectorMap actor]
Generator Config: DA_SectorConfig_Test
Show Debug Visuals: ✓ (optional)
```

## 5. Generate! (30 seconds)

In Details panel:
- Click **Generate Sector** button
- Watch objects spawn
- Review results

## Quick Commands

| Action | Button |
|--------|--------|
| Generate content | **Generate Sector** |
| Remove all | **Clear Sector** |
| Start over | **Regenerate Sector** |

## Quick Troubleshooting

**Nothing spawns?**
- Check Actor Class is set and valid
- Check Target Sector is assigned
- Look at Output Log for errors

**Objects overlap?**
- Increase Min Object Spacing
- Reduce object counts
- Increase Max Spawn Attempts

**Wrong sector name?**
- Set Custom Sector Name if you want specific name
- Or change Naming Theme for auto-generation

## Next Steps

- Add more object types (stations, ships)
- Experiment with distribution patterns
- Assign faction ownership
- Create custom configs for different sector types
- See `SectorGeneratorGuide.md` for full documentation

## Example Templates

Use these pre-made templates:
- `Assets/SectorGeneratorTemplates/MiningSection.yaml`
- `Assets/SectorGeneratorTemplates/FrontierOutpost.yaml`
- `Assets/SectorGeneratorTemplates/MilitaryBorder.yaml`
- `Assets/SectorGeneratorTemplates/TradeHub.yaml`

Copy values from YAML into your Data Asset.

---

**That's it!** You now have a procedurally generated sector. Experiment and have fun!
