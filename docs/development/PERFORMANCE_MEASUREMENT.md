# Performance Measurement Infrastructure

**Purpose**: Add performance tracking to address Anti-Pattern #11 (No Profiling/Metrics)  
**Status**: Implementation Guide  
**Last Updated**: 2025-12-24

---

## 🎯 Problem Statement

**Critical Review Finding**: "Zero Performance Metrics" - No FPS targets, memory budgets, or profiling

**Impact**: Building blind, will discover performance issues after 3 months of work

**Solution**: Add measurement infrastructure NOW, before content creation

---

## 📊 Required Metrics (MVP)

### 1. Frame Rate (FPS)
**Target**: 60 FPS on mid-range PC  
**Minimum**: 30 FPS acceptable for prototype  
**Measure**: Every build, every playtest

### 2. Memory Usage
**Target**: <2 GB RAM usage  
**Maximum**: <4 GB RAM  
**Measure**: At startup, during gameplay, peak usage

### 3. Load Times
**Target**: <30 seconds startup  
**Maximum**: <60 seconds  
**Measure**: From launch to playable

### 4. Frame Time Budget
**Target**: <16.6ms per frame (60 FPS)  
**Breakdown**:
- Game logic: <5ms
- Rendering: <10ms
- Other: <1.6ms

---

## 🛠️ Implementation Plan

### Phase 1: Add Basic FPS Counter (Week 1)

**Add to PlayerController or HUD**:

```cpp
// AdastreaPlayerController.h
UCLASS()
class AAdastreaPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;

protected:
    // Performance tracking
    UPROPERTY(BlueprintReadOnly, Category="Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category="Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category="Performance")
    float MemoryUsageMB;

private:
    void UpdatePerformanceMetrics(float DeltaTime);
    
    // Rolling average for smoother display
    TArray<float> FrameTimeSamples;
    int32 MaxSamples = 60;
};
```

**Implementation**:

```cpp
// AdastreaPlayerController.cpp
void AAdastreaPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdatePerformanceMetrics(DeltaTime);
}

void AAdastreaPlayerController::UpdatePerformanceMetrics(float DeltaTime)
{
    // Update FPS
    if (DeltaTime > 0.0f)
    {
        CurrentFPS = 1.0f / DeltaTime;
    }

    // Calculate average frame time
    FrameTimeSamples.Add(DeltaTime * 1000.0f); // Convert to ms
    if (FrameTimeSamples.Num() > MaxSamples)
    {
        FrameTimeSamples.RemoveAt(0);
    }

    AverageFrameTime = 0.0f;
    for (float Sample : FrameTimeSamples)
    {
        AverageFrameTime += Sample;
    }
    AverageFrameTime /= FrameTimeSamples.Num();

    // Get memory usage (approximate)
    // Note: Include "HAL/PlatformMemory.h" to use FPlatformMemory
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
}
```

**Display in HUD Widget**:

```cpp
// In WBP_HUD Blueprint
// Add Text Block bound to PlayerController->CurrentFPS
// Format: "FPS: {CurrentFPS}"
```

---

### Phase 2: Add Console Commands (Week 1)

**Add debugging commands**:

```cpp
// AdastreaPlayerController.h
UFUNCTION(Exec, Category="Performance")
void ShowPerformanceStats();

UFUNCTION(Exec, Category="Performance")
void HidePerformanceStats();

UFUNCTION(Exec, Category="Performance")
void DumpPerformanceReport();
```

**Implementation**:

```cpp
void AAdastreaPlayerController::ShowPerformanceStats()
{
    // Enable on-screen stats
    ConsoleCommand("stat fps");
    ConsoleCommand("stat unit");
}

void AAdastreaPlayerController::HidePerformanceStats()
{
    ConsoleCommand("stat fps 0");
    ConsoleCommand("stat unit 0");
}

void AAdastreaPlayerController::DumpPerformanceReport()
{
    UE_LOG(LogTemp, Display, TEXT("=== Performance Report ==="));
    UE_LOG(LogTemp, Display, TEXT("FPS: %.2f"), CurrentFPS);
    UE_LOG(LogTemp, Display, TEXT("Frame Time: %.2f ms"), AverageFrameTime);
    UE_LOG(LogTemp, Display, TEXT("Memory: %.2f MB"), MemoryUsageMB);
    
    // Additional profiling
    ConsoleCommand("memreport");
}
```

---

### Phase 3: Add Automated Metrics Collection (Week 2)

**Create Performance Tracking Subsystem**:

```cpp
// PerformanceTrackingSubsystem.h
UCLASS()
class UPerformanceTrackingSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category="Performance")
    void StartTracking();

    UFUNCTION(BlueprintCallable, Category="Performance")
    void StopTracking();

    UFUNCTION(BlueprintCallable, Category="Performance")
    void SaveReport(const FString& Filename);

protected:
    struct FPerformanceSample
    {
        float Timestamp;
        float FPS;
        float FrameTimeMS;
        float MemoryMB;
        FString SceneName;
    };

    TArray<FPerformanceSample> Samples;
    bool bIsTracking;
    float TrackingStartTime;

    void RecordSample();
};
```

---

### Phase 4: Set Performance Budgets (Week 2)

**Create Configuration File**:

`Config/DefaultPerformance.ini`:
```ini
[/Script/Adastrea.PerformanceTrackingSubsystem]
; Target performance budgets
TargetFPS=60
MinimumFPS=30
MaxMemoryMB=2048
MaxLoadTimeSeconds=30

; Warning thresholds
WarnBelowFPS=45
WarnAboveMemoryMB=3072
WarnAboveFrameTimeMS=20

; Profiling settings
EnableAutoProfiler=True
SampleRateSeconds=1.0
SaveReportsOnExit=True
```

**Load in code**:

```cpp
void UPerformanceTrackingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Load performance budgets from config
    GConfig->GetFloat(TEXT("/Script/Adastrea.PerformanceTrackingSubsystem"),
                      TEXT("TargetFPS"), TargetFPS, GGameIni);
    
    UE_LOG(LogTemp, Display, TEXT("Performance Tracking: Target FPS = %.0f"), TargetFPS);
}
```

---

## 📈 Measurement Workflow

### Every Build
1. Launch editor
2. Run `ShowPerformanceStats` command
3. Play for 5 minutes
4. Note average FPS, memory usage
5. Compare to previous build

### Every Playtest
1. Start performance tracking
2. Playtesters play normally
3. Stop tracking on exit
4. Review performance report
5. Identify bottlenecks

### Every Week
1. Collect all performance data
2. Chart FPS over time
3. Chart memory over time
4. Identify trends
5. Address regressions

---

## 🎯 Performance Targets by Phase

### Phase 1: Hardcoded Prototype (Week 1-4)
**Targets**:
- FPS: >30 (acceptable)
- Memory: <1 GB
- Load: <15 seconds

**Focus**: Get it working, performance is secondary

### Phase 2: Structured Version (Week 5-8)
**Targets**:
- FPS: >45 (improving)
- Memory: <2 GB
- Load: <30 seconds

**Focus**: Add content without major performance loss

### Phase 3: Polish & Demo (Week 9-12)
**Targets**:
- FPS: >60 (polished)
- Memory: <2 GB
- Load: <30 seconds

**Focus**: Optimize for smooth experience

---

## 🚨 Performance Red Flags

**Stop and Optimize If**:
- FPS drops below 20 consistently
- Memory usage >4 GB
- Load times >2 minutes
- Frequent stuttering/hitches
- Frame time spikes >100ms

**Common Causes**:
1. Too many Tick functions (use timers)
2. Complex materials (simplify)
3. High poly meshes (add LODs)
4. Unoptimized Blueprints (convert to C++)
5. Memory leaks (check garbage collection)

---

## 📊 Profiling Tools

### Built-in Unreal Tools

**Console Commands**:
```
stat fps          - Show FPS counter
stat unit         - Show frame time breakdown
stat game         - Show game thread time
stat memory       - Show memory stats
memreport         - Detailed memory report
dumpticks         - Show all ticking actors
```

**Profiler**:
```
Session Frontend → Profiler
- Record gameplay session
- Analyze frame time
- Identify bottlenecks
```

### External Tools

**For Advanced Profiling** (Week 8+):
- [Intel VTune](https://www.intel.com/content/www/us/en/developer/tools/oneapi/vtune-profiler.html) - CPU profiling
- [Nsight Graphics](https://developer.nvidia.com/nsight-graphics) - GPU profiling
- [Visual Studio Profiler](https://learn.microsoft.com/en-us/visualstudio/profiling/) - General profiling

---

## ✅ Implementation Checklist

### Week 1
- [ ] Add FPS counter to HUD
- [ ] Add memory tracking
- [ ] Add console commands
- [ ] Test in gameplay

### Week 2
- [ ] Create performance subsystem
- [ ] Add automated tracking
- [ ] Set performance budgets
- [ ] Create config file

### Week 4
- [ ] Collect baseline metrics
- [ ] Document performance profile
- [ ] Identify bottlenecks
- [ ] Create optimization plan

### Week 8
- [ ] Re-measure after content added
- [ ] Compare to baselines
- [ ] Optimize if needed
- [ ] Update targets if necessary

### Week 12
- [ ] Final performance pass
- [ ] Achieve all targets
- [ ] Document final performance
- [ ] Include in demo materials

---

## 📝 Anti-Pattern Notes

This addresses:
- **Anti-Pattern #11**: Missing Performance Considerations
- **Anti-Pattern #15**: No Profiling or Metrics

**Key Improvements**:
- Measure from Week 1 (not "later")
- Set budgets upfront
- Track trends over time
- Catch regressions early

---

## 🎓 Learning Resources

**Unreal Engine Performance**:
- [Performance Guidelines](https://dev.epicgames.com/documentation/en-us/unreal-engine/performance-guidelines-for-artists-and-designers-in-unreal-engine)
- [Profiling Documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/cpu-profiling-in-unreal-engine)

**General Game Performance**:
- Keep frame time consistent (more important than high FPS)
- Optimize for minimum FPS, not average
- Test on target hardware (mid-range PC)

---

**Next Steps**: Implement basic FPS counter in Week 1

---

**Last Updated**: 2025-12-24  
**Maintained By**: Adastrea MVP Team
