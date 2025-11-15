# Audio System Guide

**Version**: 1.0.0  
**Last Updated**: November 15, 2025  
**System Status**: Alpha - Foundation Complete

---

## Table of Contents

- [Overview](#overview)
- [System Components](#system-components)
- [Quick Start](#quick-start)
- [Sound Effects](#sound-effects)
- [Music System](#music-system)
- [Blueprint Integration](#blueprint-integration)
- [Best Practices](#best-practices)
- [Examples](#examples)

---

## Overview

The Adastrea Audio System provides a comprehensive framework for managing sound effects and dynamic music in the game. It features:

- **Data-Driven Design**: All audio configured through Data Assets
- **Priority Management**: Automatic handling of audio resource limits
- **Spatial Audio**: Full 3D sound support with attenuation
- **Dynamic Music**: Context-aware music with smooth crossfading
- **Blueprint-First**: Complete Blueprint exposure for designers

### Key Features

1. **Sound Effect Management**
   - 8 audio categories (Combat, Engine, UI, Ambient, Voice, Environmental, Mechanical, Alert)
   - Pitch variation and randomization
   - Priority-based playback
   - 3D spatial audio with attenuation

2. **Music System**
   - Mood-based music selection (8 moods)
   - Intensity levels (Low, Medium, High)
   - Smooth crossfading between tracks
   - Layered music support
   - Dynamic mixing based on gameplay

---

## System Components

### Data Assets

#### SoundEffectDataAsset
Defines individual sound effects with properties:
- **Category**: Combat, Engine, UI, Ambient, etc.
- **Volume**: Base volume multiplier (0.0 to 1.0)
- **Pitch**: Base pitch and variation
- **Spatial Settings**: 3D audio configuration
- **Priority**: Resource allocation priority

#### MusicTrackDataAsset
Defines music tracks with dynamic properties:
- **Mood**: Peaceful, Tense, Combat, Exploration, etc.
- **Intensity**: Low, Medium, High
- **Fade Settings**: Fade in/out times
- **Layering**: Support for layered tracks

### Components

#### AdastreaAudioComponent
Actor component for playing sound effects:
- **Master Volume**: Global volume control
- **Max Simultaneous Sounds**: Resource limit
- **Audio Load Tracking**: Current resource usage

#### MusicManagerSubsystem
Game instance subsystem for music management:
- **Music Library**: Available tracks
- **Context Management**: Current mood and intensity
- **Crossfading**: Smooth track transitions

---

## Quick Start

### 1. Create a Sound Effect Data Asset

1. In Content Browser, right-click → **Miscellaneous** → **Data Asset**
2. Select `SoundEffectDataAsset` as parent class
3. Name it `DA_SFX_LaserFire`
4. Configure properties:
   ```
   Display Name: "Laser Fire"
   Category: Combat
   Sound: [Your USoundWave asset]
   Volume: 0.8
   Pitch: 1.0
   Pitch Variation: 0.1
   Is 3D Sound: true
   Max Distance: 10000
   ```

### 2. Add Audio Component to Actor

In your spaceship Blueprint:
```
1. Add Component → AdastreaAudioComponent
2. Set Master Volume to 1.0
3. Set Max Simultaneous Sounds to 8
```

### 3. Play Sound in Blueprint

```blueprint
Event: OnWeaponFired
├─ Get Component → AdastreaAudioComponent
└─ PlaySoundEffect
   ├─ Sound Effect: DA_SFX_LaserFire
   └─ Location Override: [Weapon Location]
```

---

## Sound Effects

### Categories

**Combat**
- Weapon fire, explosions, impacts
- High priority for important feedback

**Engine**
- Thruster sounds, engine loops
- Medium priority, often looping

**UI**
- Button clicks, menu sounds
- Highest priority, always 2D

**Ambient**
- Background atmosphere, space ambience
- Low priority, looping

**Voice**
- Character dialogue, radio chatter
- High priority, exclusive playback

**Environmental**
- Debris, atmosphere effects
- Medium priority

**Mechanical**
- Doors, machinery, docking
- Medium priority

**Alert**
- Warnings, alarms, critical notifications
- Highest priority

### Priority System

The audio component automatically manages resources:
```cpp
// High priority sounds play even under load
Priority: 1.0 (100% chance to play)

// Medium priority reduces under load
Priority: 0.5 (50% chance when at 70%+ load)

// Low priority drops first
Priority: 0.2 (20% chance under heavy load)
```

### Spatial Audio

Configure 3D sound properties:
```
Is 3D Sound: true
Max Distance: 10000 units (hearing range)
Attenuation Distance: 1000 units (falloff start)
```

For UI sounds:
```
Is 3D Sound: false (always 2D, full volume)
```

---

## Music System

### Mood Types

1. **Peaceful** - Exploration in safe space
2. **Tense** - Approaching danger, suspense
3. **Combat** - Active combat engagement
4. **Exploration** - Discovery, wonder
5. **Dramatic** - Story moments, cutscenes
6. **Victory** - Combat success, achievement
7. **Defeat** - Loss, failure state
8. **Ambient** - Menu, background

### Intensity Levels

- **Low**: Calm, minimal action
- **Medium**: Moderate engagement
- **High**: Intense action, combat

### Setting Up Music

#### 1. Create Music Track Data Asset

```
Display Name: "Combat Theme - High Intensity"
Music Track: [Your music asset]
Mood: Combat
Intensity: High
Volume: 0.7
Fade In Time: 2.0 seconds
Fade Out Time: 2.0 seconds
Allow Crossfade: true
Priority: 1.0
```

#### 2. Add to Music Library

In your Game Instance Blueprint:
```blueprint
Event: Init
├─ Get Subsystem → MusicManagerSubsystem
└─ Add Track to Library
   └─ Track: DA_Music_CombatHigh
```

#### 3. Change Music Context

```blueprint
Event: OnCombatStarted
├─ Get Subsystem → MusicManagerSubsystem
└─ Set Music Context
   ├─ New Mood: Combat
   ├─ New Intensity: High
   └─ Force Immediate: false (crossfade)
```

### Layered Music

Create base track + layers for dynamic mixing:

**Base Track**: `DA_Music_ExplorationBase`
```
Mood: Exploration
Intensity: Low
Is Layer: false
```

**Percussion Layer**: `DA_Music_ExplorationPercussion`
```
Mood: Exploration
Intensity: Medium
Is Layer: true
Base Track: DA_Music_ExplorationBase
Layer Volume: 0.6
```

System automatically mixes layers when intensity changes.

---

## Blueprint Integration

### Common Functions

#### AdastreaAudioComponent

```cpp
// Play a sound effect
bool PlaySoundEffect(SoundEffectDataAsset, LocationOverride)

// Stop all sounds
void StopAllSounds()

// Stop specific category
void StopSoundsByCategory(Category)

// Check audio load
float GetCurrentAudioLoad() // Returns 0.0 to 1.0

// Check if category is playing
bool IsCategoryPlaying(Category)
```

#### MusicManagerSubsystem

```cpp
// Set gameplay context
void SetMusicContext(Mood, Intensity, bForceImmediate)

// Play specific track
void PlayTrack(MusicTrackDataAsset, bFadeIn)

// Stop music
void StopMusic(bFadeOut)

// Pause/Resume
void PauseMusic()
void ResumeMusic()

// Volume control
void SetMasterVolume(Volume)

// Query state
bool IsMusicPlaying()
```

### Events

Extend these in Blueprint for custom behavior:
```cpp
// No events exposed yet - subsystems handle internally
// Future: OnMusicChanged, OnAudioLoadHigh events
```

---

## Best Practices

### Sound Effects

1. **Use Categories Appropriately**
   - Group similar sounds for easy management
   - Combat sounds should be Combat category
   - UI sounds always UI category

2. **Set Priorities**
   - Critical feedback: Priority 1.0
   - Important sounds: Priority 0.7
   - Ambient: Priority 0.3

3. **Pitch Variation**
   - Add 0.05-0.1 variation for variety
   - Prevents repetitive sounds
   - Don't use for voice/music

4. **3D Audio**
   - Enable for world sounds
   - Disable for UI and HUD
   - Set appropriate distances

5. **Resource Management**
   - Limit simultaneous sounds (8-16)
   - High priority for player actions
   - Low priority for distant effects

### Music

1. **Context Switching**
   - Use crossfades for smooth transitions
   - Match intensity to gameplay pace
   - Immediate changes for dramatic moments

2. **Track Organization**
   - Name clearly: `DA_Music_[Mood]_[Intensity]`
   - Set appropriate fade times
   - Use layers for dynamic variety

3. **Volume Balance**
   - Base volume: 0.6-0.7
   - Layers: 0.4-0.6
   - Test with sound effects playing

4. **Library Management**
   - Add all tracks at game start
   - Organize by mood and intensity
   - Set priorities for selection

---

## Examples

### Example 1: Weapon Sound Effect

**Create Data Asset**: `DA_SFX_PlasmaCannonFire`
```
Display Name: "Plasma Cannon Fire"
Category: Combat
Sound: S_PlasmaCannonFire
Volume: 0.9
Pitch: 1.0
Pitch Variation: 0.05
Priority: 0.8
Is 3D Sound: true
Max Distance: 15000
Attenuation Distance: 2000
```

**Use in Weapon Blueprint**:
```blueprint
Event: Fire Weapon
├─ Get Component: AdastreaAudioComponent
├─ PlaySoundEffect
│  ├─ DA_SFX_PlasmaCannonFire
│  └─ Location: Weapon Muzzle Location
└─ Branch: Sound Played Successfully
   └─ [Spawn VFX]
```

### Example 2: Dynamic Combat Music

**Create Track**: `DA_Music_CombatLow`
```
Mood: Combat
Intensity: Low
Fade In: 3.0
Fade Out: 3.0
Priority: 0.8
```

**Create Track**: `DA_Music_CombatHigh`
```
Mood: Combat
Intensity: High
Fade In: 1.0
Fade Out: 2.0
Priority: 0.9
```

**Use in Game State**:
```blueprint
Event: Combat Started
├─ Get Subsystem: MusicManagerSubsystem
└─ Set Music Context
   ├─ Mood: Combat
   ├─ Intensity: Low
   └─ Force Immediate: false

Event: Combat Intensified (Enemy Count > 5)
├─ Get Subsystem: MusicManagerSubsystem
└─ Set Music Context
   ├─ Mood: Combat
   ├─ Intensity: High
   └─ Force Immediate: false

Event: Combat Ended
├─ Get Subsystem: MusicManagerSubsystem
└─ Set Music Context
   ├─ Mood: Peaceful
   ├─ Intensity: Low
   └─ Force Immediate: false
```

### Example 3: Ambient Engine Loop

**Create Data Asset**: `DA_SFX_EngineIdle`
```
Display Name: "Engine Idle Loop"
Category: Engine
Sound: S_EngineIdle_Loop
Volume: 0.6
Pitch: 1.0
Is Looping: true
Priority: 0.5
Is 3D Sound: true
Max Distance: 8000
```

**Use in Ship Blueprint**:
```blueprint
Event: BeginPlay
├─ Get Component: AdastreaAudioComponent
└─ PlaySoundEffect: DA_SFX_EngineIdle

Event: EndPlay
├─ Get Component: AdastreaAudioComponent
└─ StopSoundsByCategory: Engine
```

---

## Performance Considerations

1. **Limit Active Sounds**
   - Set MaxSimultaneousSounds based on platform
   - PC: 16-32
   - Console: 8-16

2. **Audio Load Management**
   - Monitor GetCurrentAudioLoad()
   - Reduce distant sounds when near limit
   - Priority system handles this automatically

3. **Music Memory**
   - Stream long tracks (avoid loading all in memory)
   - Use shorter loops when possible
   - Crossfade component automatically cleans up

4. **3D Calculations**
   - Audio component ticks at 0.5 second intervals
   - Distance checks are efficient
   - Occlusion not implemented (future feature)

---

## Troubleshooting

### Sounds Not Playing

1. Check Audio Component exists on actor
2. Verify MasterVolume > 0
3. Check sound asset is assigned in Data Asset
4. Verify priority is appropriate for current load
5. Check MaxSimultaneousSounds limit not reached

### Music Not Changing

1. Verify tracks are in Music Library
2. Check mood/intensity match available tracks
3. Ensure MasterMusicVolume > 0
4. Verify track priority allows selection
5. Check for matching mood/intensity combinations

### Audio Load Too High

1. Reduce MaxSimultaneousSounds
2. Increase UpdateFrequency in LOD component
3. Lower priority of ambient sounds
4. Stop distant sounds manually
5. Use StopSoundsByCategory for cleanup

---

## Future Enhancements

Planned features for future versions:

1. **Audio Occlusion**
   - Walls and obstacles muffle sound
   - Distance-based filtering

2. **Audio Events**
   - OnMusicChanged event
   - OnAudioLoadCritical event
   - OnSoundEffectPlayed event

3. **Reverb Zones**
   - Environmental audio effects
   - Room acoustics simulation

4. **Voice System**
   - Radio distortion effects
   - Character-specific voice processing
   - Subtitle integration

5. **Audio Mixing Groups**
   - Master/SFX/Music/Voice channels
   - Per-group volume controls
   - Ducking system (lower music during dialogue)

---

## See Also

- [TradingSystemGuide.md](TradingSystemGuide.md) - For market audio cues
- [CombatSystemGuide.md](CombatSystemGuide.md) - For weapon sound integration
- [TutorialSystemGuide.md](TutorialSystemGuide.md) - For tutorial voiceovers
- [PerformanceOptimizationGuide.md](PerformanceOptimizationGuide.md) - For audio performance

---

**Note**: This is an alpha system. Please report issues and provide feedback for improvements.
