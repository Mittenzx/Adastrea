# Phase 4 Systems - Quick Reference

**Version**: 1.0.0  
**Last Updated**: November 15, 2025

---

## Audio System

### Play Sound Effect
```cpp
AdastreaAudioComponent->PlaySoundEffect(DA_SFX_LaserFire, WeaponLocation)
```

### Set Music Context
```cpp
MusicManagerSubsystem->SetMusicContext(EMusicMood::Combat, EMusicIntensity::High)
```

### Create Sound Effect Data Asset
```
Category: Combat
Volume: 0.8
Pitch: 1.0, Variation: 0.1
Is 3D Sound: true
Max Distance: 10000
```

---

## Performance System

### Add LOD Component
```cpp
// In Blueprint: Add Component → LODManagerComponent
HighToMediumDistance: 5000
MediumToLowDistance: 15000
UpdateFrequency: 0.5
```

### Implement LOD Response
```cpp
Event OnLODChanged(NewLevel, OldLevel)
Switch on NewLevel:
  High: Full quality meshes/materials
  Medium: Reduced quality
  Low: Minimal quality
  VeryLow: Simplified
```

### Get Performance Metrics
```cpp
PerformanceProfiler->GetCurrentMetrics() // Returns FPerformanceMetrics
PerformanceProfiler->GetAverageFPS(5.0f) // 5 second average
```

---

## Station Management UI

### Display Widget
```cpp
CreateWidget(WBP_StationManagement)
StationManagementWidget->SetManagedStation(Station)
```

### Get Modules
```cpp
StationManagementWidget->GetModulesByGroup(EStationModuleGroup::Docking)
StationManagementWidget->GetTotalModuleCount()
StationManagementWidget->GetOperationalStatusPercent() // 0-100
```

### Events to Implement
```cpp
Event OnStationStatusUpdated() // Update UI
Event OnModuleAdded(Module) // Refresh list
Event OnModuleRemoved(Module) // Refresh list
```

---

## Trading Interface UI

### Display Widget
```cpp
CreateWidget(WBP_TradingInterface)
TradingInterfaceWidget->SetTradePartner(Faction)
```

### Filter and Sort
```cpp
TradingInterfaceWidget->SetCategoryFilter("Weapons")
TradingInterfaceWidget->SetSortMode("Price")
TradingInterfaceWidget->ToggleBuySellView()
```

### Cart Operations
```cpp
TradingInterfaceWidget->AddToCart(Item, Quantity)
TradingInterfaceWidget->GetCartTotal() // Returns credits
TradingInterfaceWidget->ExecuteTrade() // Process transaction
```

---

## Tutorial System

### Start Tutorial
```cpp
TutorialManagerSubsystem->StartTutorial("BasicControls", [Array of Steps])
```

### Create Tutorial Step
```
Step Type: Wait For Action
Title: "Fire Your Weapon"
Description: "Press Left Mouse Button"
Action To Wait For: "FireWeapon"
Time Limit: 30
```

### Complete Step
```cpp
// When action is performed:
TutorialManagerSubsystem->CompleteCurrentStep()
```

### Check Completion
```cpp
TutorialManagerSubsystem->IsTutorialCompleted("BasicControls")
TutorialManagerSubsystem->GetTutorialProgress() // 0-100%
```

---

## Common Patterns

### Data Asset Creation
1. Right-click → Miscellaneous → Data Asset
2. Select parent class (SoundEffectDataAsset, MusicTrackDataAsset, etc.)
3. Configure properties
4. Reference in Blueprints

### Blueprint Events
```cpp
// Bind events in Construct or BeginPlay
Event OnSystemEvent
└─ Update UI / Respond to change
```

### Subsystem Access
```cpp
GetGameInstance()->GetSubsystem<UMusicManagerSubsystem>()
GetGameInstance()->GetSubsystem<UPerformanceProfiler>()
GetGameInstance()->GetSubsystem<UTutorialManagerSubsystem>()
```

### Component Usage
```cpp
// Add in Blueprint: Add Component
AddComponent(AdastreaAudioComponent)
AddComponent(LODManagerComponent)
```

---

## File Locations

### Headers
```
Source/Adastrea/Public/Audio/
Source/Adastrea/Public/Performance/
Source/Adastrea/Public/UI/
Source/Adastrea/Public/Tutorial/
```

### Implementation
```
Source/Adastrea/Audio/
Source/Adastrea/Performance/
Source/Adastrea/UI/
Source/Adastrea/Tutorial/
```

### Documentation
```
Assets/AudioSystemGuide.md
Assets/PerformanceOptimizationGuide.md
Assets/StationManagementGuide.md
Assets/TutorialSystemGuide.md
Assets/Phase4SystemsSummary.md
```

---

## Key Enums

### Audio
```cpp
ESoundEffectCategory: Combat, Engine, UI, Ambient, Voice, Environmental, Mechanical, Alert
EMusicMood: Peaceful, Tense, Combat, Exploration, Dramatic, Victory, Defeat, Ambient
EMusicIntensity: Low, Medium, High
```

### Performance
```cpp
ELODLevel: High, Medium, Low, VeryLow
```

### Station
```cpp
EStationModuleGroup: None, Docking, Power, Storage, Defence, LivingQuarters, 
                     Command, Industrial, Trade, Research, Entertainment
```

### Tutorial
```cpp
ETutorialStepType: Message, HighlightUI, WaitForAction, WaitForCondition, 
                   PlayAnimation, SpawnActor
```

---

## Troubleshooting

### Sounds Not Playing
- Check AdastreaAudioComponent exists
- Verify MasterVolume > 0
- Check MaxSimultaneousSounds limit
- Verify sound asset assigned

### LOD Not Changing
- Check LODManagerComponent added
- Verify distance thresholds set
- Implement OnLODChanged event
- Call ForceUpdateLOD

### Tutorial Not Starting
- Check subsystem initialized
- Verify steps are valid
- Check bTutorialsEnabled = true
- Ensure not already completed

### UI Not Updating
- Implement OnStationStatusUpdated
- Call RefreshStationData
- Check timer is running
- Verify station is valid

---

## Best Practices

1. **Audio**: Use categories, set priorities, add pitch variation
2. **Performance**: Add LOD to all visible actors, monitor metrics
3. **UI**: Event-driven updates, cache widgets, throttle refreshes
4. **Tutorials**: Short steps, clear instructions, allow skipping

---

## See Full Documentation

- [AudioSystemGuide.md](AudioSystemGuide.md) - Complete audio guide
- [PerformanceOptimizationGuide.md](PerformanceOptimizationGuide.md) - LOD and profiling
- [StationManagementGuide.md](StationManagementGuide.md) - Station UI
- [TutorialSystemGuide.md](TutorialSystemGuide.md) - Tutorial framework
- [Phase4SystemsSummary.md](Phase4SystemsSummary.md) - Implementation summary
