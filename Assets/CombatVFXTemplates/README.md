# Combat VFX Templates

YAML template files for creating combat visual effects in Adastrea.

## Available Templates

### Weapon Fire Effects

| Template | Weapon Type | Description |
|----------|-------------|-------------|
| `WeaponVFX_EnergyLaser.yaml` | Energy | Laser beam with glow and heat distortion |
| `WeaponVFX_PlasmaCannon.yaml` | Plasma | Ball lightning with electrical arcing |
| `WeaponVFX_MissileLauncher.yaml` | Missile | Smoke trail with engine glow |

### Impact Effects

| Template | Description |
|----------|-------------|
| `ImpactEffects_Standard.yaml` | Shield/hull impacts and explosions (3 sizes) |

## How to Use Templates

### Step 1: Choose Template
Select the template that matches your weapon type or effect category.

### Step 2: Copy Template
```bash
cp WeaponVFX_EnergyLaser.yaml MyWeapon.yaml
```

### Step 3: Customize Values
Edit the YAML file to match your weapon's characteristics:
- Change colors
- Adjust particle counts
- Set performance limits
- Configure visual properties

### Step 4: Create Data Asset in Unreal
1. Open Unreal Editor
2. Content Browser → Right-click → Blueprint
3. Select `CombatVFXDataAsset` or `ImpactEffectDataAsset`
4. Name following convention: `DA_VFX_[Name]`
5. Configure properties to match your YAML template

### Step 5: Assign Niagara Systems
In the Data Asset properties panel:
- Assign Niagara systems for effects
- Assign materials for visual feedback
- Assign sounds for audio feedback
- Set camera shakes for impact

## Template Structure

### Weapon VFX Template Structure
```yaml
weapon_vfx:
  vfx_name: ""              # Display name
  description: ""           # Brief description
  weapon_type: ""           # Energy/Projectile/Missile/etc
  
  fire_effect:
    # Niagara systems
    # Beam/projectile properties
    # Visual settings
    
  heat_visuals:
    # Heat gradient colors
    # Vent effects
    # Shimmer settings
    
  performance:
    # Particle budget
    # Priority
    # Culling distance
```

### Impact Effects Template Structure
```yaml
impact_effects:
  effect_name: ""
  description: ""
  
  shield_impact:
    # Ripple effect
    # Dissipation particles
    # Colors and timing
    
  hull_impact:
    # Sparks and debris
    # Damage decals
    # Flash effects
    
  explosions:
    small: {}
    medium: {}
    large: {}
    
  performance:
    # Budgets and priorities
```

## Creating Additional Templates

To create templates for other weapon types:

1. Copy an existing similar template
2. Adjust weapon_type
3. Modify colors to match weapon theme
4. Adjust particle counts for weapon scale
5. Set appropriate performance limits
6. Document special notes for the weapon type

### Example: Creating Torpedo Template

```yaml
weapon_vfx:
  vfx_name: "Heavy Torpedo Effects"
  weapon_type: Torpedo
  
  fire_effect:
    beam_type: Projectile
    beam_width: 30.0  # Larger than missiles
    beam_color:
      r: 0.2
      g: 0.5
      b: 1.0  # Blue plasma
    
  # ... rest of configuration
```

## Performance Guidelines

When customizing templates, keep these performance targets:

| Effect Type | Max Particles | Target Time |
|-------------|---------------|-------------|
| Energy Weapon | 500 | <0.8ms |
| Projectile Weapon | 600 | <0.6ms |
| Missile | 600 | <1.0ms |
| Plasma | 800 | <1.0ms |
| Shield Impact | 500 | <1.0ms |
| Hull Impact | 400 | <0.8ms |
| Small Explosion | 1000 | <1.5ms |
| Medium Explosion | 2000 | <2.5ms |
| Large Explosion | 5000 | <5.0ms |

**Total Budget**: <5ms for all active effects

## Color Coding Standards

Follow these color conventions for consistency:

### Weapon Types
- **Energy**: Blue (`0, 0.5, 1`)
- **Projectile**: Yellow-white (`1, 1, 0.8`)
- **Missile**: Orange-red (`1, 0.5, 0.2`)
- **Torpedo**: Blue-white (`0.5, 0.7, 1`)
- **Beam**: Bright cyan (`0, 1, 1`)
- **Plasma**: Green (`0, 1, 0.4`)

### Heat States
- **Cool**: Blue (`0.1, 0.3, 0.5`)
- **Warning**: Orange (`1, 0.5, 0`)
- **Overheat**: Red (`1, 0, 0`)

### Shield States
- **Full**: Green (`0, 1, 0`)
- **Recharging**: Blue (`0, 0.5, 1`)
- **Down**: Red (`1, 0, 0`)

### Damage Indicators
- **Low Damage**: Blue (`0, 0.5, 1`)
- **Medium Damage**: Cyan (`0, 1, 1`)
- **High Damage**: Yellow-white (`1, 1, 0.5`)

## Naming Conventions

### Data Assets
```
DA_VFX_[WeaponType][Variant]
Examples:
  DA_VFX_EnergyLaser
  DA_VFX_PlasmaHeavyCannon
  DA_VFX_MissileLight

DA_Impact_[Variant]
Examples:
  DA_Impact_Standard
  DA_Impact_Heavy
  DA_Impact_Capital
```

### Niagara Systems
```
NS_[Category]_[Effect]
Examples:
  NS_Energy_LaserBeam
  NS_Plasma_BallLightning
  NS_Shield_ImpactRipple
  NS_Explosion_Medium
```

### Materials
```
M_[Type]_[Purpose]
Examples:
  M_Shield_Ripple
  M_Weapon_HeatGradient
  M_Decal_HullDamage
```

### Sounds
```
S_[Category]_[Action]
Examples:
  S_EnergyLaser_Fire
  S_Shield_Impact
  S_Explosion_Medium
```

## Additional Resources

- **Full System Guide**: `../CombatVFXSystemGuide.md`
- **Workflow Guide**: `../CombatVFXWorkflow.md`
- **Combat System**: `../CombatSystemGuide.md`
- **Niagara Docs**: Unreal Engine 5 Niagara documentation

## Contributing

To add new templates:
1. Follow the structure of existing templates
2. Include comprehensive comments
3. Document special features
4. Provide usage notes
5. Test with actual implementation

---

**Last Updated**: November 18, 2025  
**Template Version**: 1.0
