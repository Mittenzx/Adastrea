# Performance Optimization Guide

**Version**: 1.0.0  
**Last Updated**: November 15, 2025  
**System Status**: Alpha - Foundation Complete

---

## Table of Contents

- [Overview](#overview)
- [LOD System](#lod-system)
- [Performance Profiler](#performance-profiler)
- [Optimization Strategies](#optimization-strategies)
- [Blueprint Integration](#blueprint-integration)
- [Best Practices](#best-practices)
- [Examples](#examples)

---

## Overview

The Adastrea Performance Optimization System provides tools for maintaining smooth gameplay across different hardware configurations. It features:

- **LOD Management**: Automatic Level of Detail adjustment
- **Performance Profiling**: Real-time metrics tracking
- **Adaptive Quality**: Dynamic adjustment based on frame rate
- **Blueprint-Friendly**: Easy integration for designers

### Key Features

1. **LOD Manager Component**
   - 4 quality levels (High, Medium, Low, VeryLow)
   - Distance-based LOD selection
   - Performance-based adjustment
   - Automatic culling for distant objects

2. **Performance Profiler Subsystem**
   - Real-time FPS tracking
   - Memory usage monitoring
   - Frame time analysis
   - Custom scope profiling

---

## LOD System

### Overview

The LOD (Level of Detail) system automatically adjusts visual quality based on:
- **Distance from camera**: Distant objects use lower quality
- **Performance metrics**: Quality reduces when FPS drops
- **Manual overrides**: Designer control when needed

### LOD Levels

| Level | Description | Typical Distance | Use Case |
|-------|-------------|------------------|----------|
| **High** | Full quality | 0-5000 units | Near player, important objects |
| **Medium** | Reduced quality | 5000-15000 units | Mid-distance objects |
| **Low** | Minimal quality | 15000-30000 units | Far objects, background |
| **VeryLow** | Simplified | 30000+ units | Very distant, almost culled |

### Adding LOD to Actors

#### 1. Add Component

In your actor Blueprint:
```
1. Add Component → LODManagerComponent
2. Configure thresholds:
   - High to Medium Distance: 5000
   - Medium to Low Distance: 15000
   - Low to VeryLow Distance: 30000
3. Set Update Frequency: 0.5 seconds
```

#### 2. Implement LOD Response

In Blueprint Event Graph:
```blueprint
Event: OnLODChanged (from LODManagerComponent)
├─ Switch on LODLevel
│  ├─ High
│  │  └─ Set Static Mesh: HighDetailMesh
│  │  └─ Set Material: HighQualityMaterial
│  ├─ Medium
│  │  └─ Set Static Mesh: MediumDetailMesh
│  │  └─ Set Material: MediumQualityMaterial
│  ├─ Low
│  │  └─ Set Static Mesh: LowDetailMesh
│  │  └─ Disable: Particle Systems
│  └─ VeryLow
│     └─ Set Static Mesh: VeryLowDetailMesh
│     └─ Disable: All Effects
```

### Distance Thresholds

**Space Stations**:
```
High to Medium: 10000 units
Medium to Low: 30000 units
Low to VeryLow: 60000 units
Update Frequency: 1.0 second
```

**Spaceships**:
```
High to Medium: 5000 units
Medium to Low: 15000 units
Low to VeryLow: 30000 units
Update Frequency: 0.5 seconds
```

**Small Objects** (Debris, Asteroids):
```
High to Medium: 2000 units
Medium to Low: 8000 units
Low to VeryLow: 15000 units
Update Frequency: 1.0 second
```

### Performance-Based LOD

Enable automatic quality reduction:
```
Use Performance LOD: true
Target Frame Rate: 60 FPS
```

How it works:
- Below 54 FPS (90%): Reduce quality one level
- Below 48 FPS (80%): Reduce quality two levels
- Above 72 FPS (120%): Increase quality if distance allows

### Manual LOD Control

Override automatic LOD when needed:
```blueprint
Event: Special Cutscene
├─ LODManagerComponent → SetLODLevel
│  └─ LOD Level: High (force high quality)
```

Return to automatic:
```blueprint
Event: Cutscene End
└─ LODManagerComponent → ForceUpdateLOD
```

---

## Performance Profiler

### Overview

The Performance Profiler tracks real-time metrics for optimization:
- **FPS**: Frames per second
- **Frame Time**: Milliseconds per frame
- **Memory**: RAM usage
- **Actor Count**: Visible actors in scene

### Enabling Profiler

In Game Instance Blueprint:
```blueprint
Event: Init
├─ Get Subsystem → PerformanceProfiler
└─ Set Profiling Enabled: true
```

### Reading Metrics

```blueprint
Function: GetPerformanceData
├─ Get Subsystem → PerformanceProfiler
├─ GetCurrentMetrics
└─ Return: Performance Metrics Struct
   ├─ FPS
   ├─ Frame Time MS
   ├─ Memory Used MB
   └─ Visible Actors
```

### Displaying on HUD

```blueprint
Event: Update HUD
├─ Get Subsystem → PerformanceProfiler
├─ GetCurrentMetrics
└─ Set Text
   └─ Format: "FPS: {FPS} | Memory: {Memory}MB"
```

### Performance Warnings

Enable automatic warnings:
```
Log Performance Warnings: true
Warning FPS Threshold: 30 FPS
```

Warnings log when:
- FPS drops below threshold
- Memory usage exceeds 2GB
- Can be extended for custom checks

### Custom Profiling Scopes

Profile specific code sections:
```blueprint
Function: ExpensiveOperation
├─ PerformanceProfiler → BeginProfileScope
│  └─ Scope Name: "AI Update Loop"
├─ [Your expensive code here]
└─ PerformanceProfiler → EndProfileScope
   └─ Scope Name: "AI Update Loop"

Later, check time:
├─ PerformanceProfiler → GetScopeTime
│  └─ Scope Name: "AI Update Loop"
└─ Print: "AI Update took {Time}ms"
```

### Metrics History

Get average FPS over time:
```blueprint
├─ PerformanceProfiler → GetAverageFPS
│  └─ Time Window: 5.0 seconds
└─ Returns: Average FPS
```

Check if performance is poor:
```blueprint
├─ PerformanceProfiler → IsPerformancePoor
└─ Returns: bool (true if below threshold)
```

---

## Optimization Strategies

### 1. LOD for Large Scenes

Apply LOD to all major actors:
```
Space Stations: LODManagerComponent
Spaceships: LODManagerComponent
Asteroids: LODManagerComponent
Effects: Distance-based enable/disable
```

### 2. Culling Distant Objects

```blueprint
Event: Tick (every 1 second)
├─ LODManagerComponent → ShouldCullActor
└─ Branch: Should Cull?
   └─ Set Actor Hidden: true
```

### 3. Reduce Update Frequency

For distant objects:
```cpp
// Near player: Tick every frame
LODManagerComponent->UpdateFrequency = 0.1f

// Far from player: Tick less often
LODManagerComponent->UpdateFrequency = 2.0f
```

### 4. Material LOD

Create material instances for each LOD level:
- **High**: Full PBR, normal maps, reflections
- **Medium**: Simplified PBR, reduced texture resolution
- **Low**: Unlit or simple lit
- **VeryLow**: Solid color, no textures

### 5. Particle System LOD

```blueprint
Event: OnLODChanged
├─ Switch on LODLevel
│  ├─ High: Spawn Rate = 100
│  ├─ Medium: Spawn Rate = 50
│  ├─ Low: Spawn Rate = 20
│  └─ VeryLow: Deactivate All Particles
```

---

## Blueprint Integration

### LODManagerComponent Functions

```cpp
// Manual LOD control
void SetLODLevel(LODLevel)

// Force immediate update
void ForceUpdateLOD()

// Get recommended LOD for distance
ELODLevel GetRecommendedLODForDistance(Distance)

// Check if should be culled
bool ShouldCullActor()
```

### PerformanceProfiler Functions

```cpp
// Get current metrics
FPerformanceMetrics GetCurrentMetrics()

// Enable/disable profiling
void SetProfilingEnabled(bool)

// Get average FPS
float GetAverageFPS(TimeWindow)

// Check performance
bool IsPerformancePoor()

// Scope profiling
void BeginProfileScope(ScopeName)
void EndProfileScope(ScopeName)
float GetScopeTime(ScopeName)
```

### Events

```cpp
// LODManagerComponent
Event OnLODChanged(NewLODLevel, OldLODLevel)
// Fired when LOD changes - implement visual updates here
```

---

## Best Practices

### LOD System

1. **Set Appropriate Thresholds**
   - Large objects: Larger distances
   - Small objects: Smaller distances
   - Important objects: Keep high quality longer

2. **Update Frequency**
   - Static objects: 1-2 seconds
   - Moving objects: 0.5-1 second
   - Critical objects: 0.1-0.5 seconds

3. **Performance LOD**
   - Enable on complex actors
   - Set target FPS realistically
   - Test on target hardware

4. **Visual Transitions**
   - Make LOD changes subtle
   - Use crossfade when possible
   - Test visibility of transitions

### Performance Profiling

1. **Production Builds**
   - Disable detailed profiling
   - Keep basic FPS monitoring
   - Log warnings only

2. **Development**
   - Enable full profiling
   - Display metrics on HUD
   - Profile custom scopes

3. **Scope Naming**
   - Use descriptive names
   - Group related scopes
   - Clean up finished scopes

4. **Metrics Interpretation**
   - FPS < 30: Critical issues
   - Frame Time > 33ms: Target is 30 FPS
   - Memory > 2GB: Investigate leaks

---

## Examples

### Example 1: Space Station LOD

**Add LOD Component**:
```
Component: LODManagerComponent
High to Medium Distance: 10000
Medium to Low Distance: 30000
Low to VeryLow Distance: 60000
Use Performance LOD: true
Target Frame Rate: 60
Update Frequency: 1.0
```

**Implement LOD Response**:
```blueprint
Event: OnLODChanged
├─ Print: "LOD Changed to {NewLODLevel}"
├─ Switch on New LOD Level
│  ├─ High
│  │  ├─ Set Module Detail: High
│  │  ├─ Enable: All Particle Systems
│  │  └─ Enable: Dynamic Lights
│  ├─ Medium
│  │  ├─ Set Module Detail: Medium
│  │  ├─ Disable: Non-Essential Particles
│  │  └─ Reduce: Light Count by 50%
│  ├─ Low
│  │  ├─ Set Module Detail: Low
│  │  ├─ Disable: All Particles
│  │  └─ Disable: Dynamic Lights
│  └─ VeryLow
│     ├─ Set Module Detail: Very Low
│     ├─ Use: Solid Color Materials
│     └─ Disable: All Effects
```

### Example 2: Performance HUD

**Create HUD Widget**:
```blueprint
Event: Update HUD (Tick every 0.5 seconds)
├─ Get Subsystem: PerformanceProfiler
├─ GetCurrentMetrics
├─ Set Text: FPS_Text
│  └─ Format: "FPS: {FPS:0.0}"
├─ Set Text: FrameTime_Text
│  └─ Format: "Frame: {FrameTimeMS:0.2}ms"
├─ Set Text: Memory_Text
│  └─ Format: "Memory: {MemoryUsedMB:0.0}MB"
└─ Set Color: FPS_Text
   └─ Branch: FPS < 30?
      ├─ True: Red
      └─ False: Green
```

### Example 3: Adaptive Quality

**Game State Blueprint**:
```blueprint
Event: Tick (every 1 second)
├─ Get Subsystem: PerformanceProfiler
├─ GetAverageFPS (5 second window)
└─ Branch: Average FPS < 45?
   ├─ True
   │  ├─ Set Global LOD Bias: +1 level
   │  ├─ Reduce Particle Density: 50%
   │  └─ Print: "Reducing quality due to low FPS"
   └─ False
      └─ Branch: Average FPS > 55?
         └─ True
            ├─ Set Global LOD Bias: -1 level
            ├─ Increase Particle Density: 100%
            └─ Print: "Increasing quality due to good FPS"
```

### Example 4: Custom Profiling

**Profile AI System**:
```blueprint
Function: Update AI (runs every tick)
├─ PerformanceProfiler → BeginProfileScope
│  └─ "AI System Update"
├─ For Each: AI Controller
│  ├─ Update Decision Tree
│  ├─ Update Pathfinding
│  └─ Update Animations
└─ PerformanceProfiler → EndProfileScope
   └─ "AI System Update"

Function: Display AI Performance (in debug HUD)
├─ PerformanceProfiler → GetScopeTime
│  └─ "AI System Update"
├─ Print: "AI Update: {Time}ms"
└─ Branch: Time > 10ms?
   └─ True: Print Warning "AI system taking too long!"
```

---

## Performance Targets

### Frame Rate Goals

- **Desktop**: 60 FPS minimum
- **Laptop**: 45 FPS minimum
- **Low-End**: 30 FPS minimum

### Memory Budgets

- **Desktop**: < 4GB
- **Laptop**: < 3GB
- **Low-End**: < 2GB

### LOD Distance Guidelines

| Object Size | High-Med | Med-Low | Low-VeryLow |
|-------------|----------|---------|-------------|
| Huge (Stations) | 10km | 30km | 60km |
| Large (Capital Ships) | 5km | 15km | 30km |
| Medium (Fighters) | 3km | 10km | 20km |
| Small (Debris) | 1km | 5km | 10km |

---

## Troubleshooting

### Low Frame Rate

1. Check PerformanceProfiler metrics
2. Identify bottleneck (CPU vs GPU)
3. Enable Performance LOD on all actors
4. Reduce particle counts
5. Optimize material complexity

### LOD Not Working

1. Verify LODManagerComponent exists
2. Check distance thresholds are set
3. Ensure OnLODChanged is implemented
4. Call ForceUpdateLOD manually
5. Check camera is being found

### Memory Leaks

1. Monitor memory over time
2. Look for increasing trend
3. Check actor destruction
4. Verify particles are cleaned up
5. Use Unreal's memory profiler

---

## Future Enhancements

1. **Occlusion Culling**
   - Hide objects behind others
   - Spatial partitioning optimization

2. **Async Loading**
   - Stream in distant objects
   - Background asset loading

3. **Network Optimization**
   - Replication priority
   - Interest management

4. **GPU Profiling**
   - Detailed GPU metrics
   - Shader complexity analysis

5. **Automatic LOD Generation**
   - Generate LOD meshes automatically
   - Mesh simplification tools

---

## See Also

- [AudioSystemGuide.md](AudioSystemGuide.md) - For audio performance
- [CombatSystemGuide.md](CombatSystemGuide.md) - For combat optimization
- [NavigationSystemGuide.md](NavigationSystemGuide.md) - For pathfinding optimization

---

**Note**: This is an alpha system. Performance targets may change based on testing and feedback.
