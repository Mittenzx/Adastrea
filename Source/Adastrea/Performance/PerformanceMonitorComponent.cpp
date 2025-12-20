// Copyright Mittenzx. All Rights Reserved.

#include "Performance/PerformanceMonitorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "HAL/PlatformMemory.h"

UPerformanceMonitorComponent::UPerformanceMonitorComponent()
    : bEnabled(true)
    , UpdateFrequency(1.0f)
    , bDetailedLogging(false)
    , FrameTimeWarningThreshold(33.0f) // ~30 FPS
    , bEnableAlerts(true)
    , CurrentFrameTime(0.0f)
    , AverageFrameTime(0.0f)
    , PeakFrameTime(0.0f)
    , FPS(0.0f)
    , MemoryUsage(0)
    , ComponentCount(0)
    , TickableComponentCount(0)
    , TimeSinceLastUpdate(0.0f)
    , bWasPerformanceGood(true)
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UPerformanceMonitorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize metrics
    ResetMetrics();
    CalculateComponentStats();

    if (bDetailedLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Performance Monitor started for %s"), *GetOwner()->GetName());
    }
}

void UPerformanceMonitorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (bDetailedLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Performance Monitor ended for %s"), *GetOwner()->GetName());
    }

    Super::EndPlay(EndPlayReason);
}

void UPerformanceMonitorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bEnabled)
    {
        return;
    }

    // Update frame time history
    CurrentFrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    FrameTimeHistory.Add(CurrentFrameTime);

    // Maintain history buffer (last 60 frames ~1 second at 60fps)
    if (FrameTimeHistory.Num() > 60)
    {
        FrameTimeHistory.RemoveAt(0);
    }

    // Update metrics at specified frequency
    TimeSinceLastUpdate += DeltaTime;
    if (TimeSinceLastUpdate >= UpdateFrequency)
    {
        UpdateMetrics(DeltaTime);
        TimeSinceLastUpdate = 0.0f;
    }

    // Check performance thresholds
    if (bEnableAlerts)
    {
        CheckPerformanceThresholds();
    }
}

void UPerformanceMonitorComponent::UpdateMetrics(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Calculate FPS
    FPS = 1.0f / DeltaTime;

    // Calculate average frame time
    if (FrameTimeHistory.Num() > 0)
    {
        float Sum = 0.0f;
        PeakFrameTime = 0.0f;

        for (float Time : FrameTimeHistory)
        {
            Sum += Time;
            PeakFrameTime = FMath::Max(PeakFrameTime, Time);
        }

        AverageFrameTime = Sum / FrameTimeHistory.Num();
    }

    // Update average frame times history
    AverageFrameTimes.Add(AverageFrameTime);
    if (AverageFrameTimes.Num() > 10) // Keep last 10 seconds
    {
        AverageFrameTimes.RemoveAt(0);
    }

    // Calculate component stats
    CalculateComponentStats();

    // Get memory usage
    MemoryUsage = GetActorMemoryUsage();

    // Log if detailed logging enabled
    if (bDetailedLogging)
    {
        LogPerformanceData();
    }
}

void UPerformanceMonitorComponent::CalculateComponentStats()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    ComponentCount = Owner->GetComponents().Num();
    TickableComponentCount = 0;

    // Count tickable components
    for (UActorComponent* Component : Owner->GetComponents())
    {
        if (Component && Component->PrimaryComponentTick.bCanEverTick)
        {
            TickableComponentCount++;
        }
    }
}

void UPerformanceMonitorComponent::CheckPerformanceThresholds()
{
    bool bIsPerformanceGood = IsPerformanceGood();

    // Check for performance degradation
    if (bWasPerformanceGood && !bIsPerformanceGood)
    {
        // Performance dropped
        OnPerformanceWarning(CurrentFrameTime);

        if (bDetailedLogging)
        {
            UE_LOG(LogTemp, Warning, TEXT("Performance warning for %s: %.1f ms"),
                *GetOwner()->GetName(), CurrentFrameTime);
        }
    }
    else if (!bWasPerformanceGood && bIsPerformanceGood)
    {
        // Performance recovered
        OnPerformanceRecovered();

        if (bDetailedLogging)
        {
            UE_LOG(LogTemp, Log, TEXT("Performance recovered for %s"),
                *GetOwner()->GetName());
        }
    }

    bWasPerformanceGood = bIsPerformanceGood;
}

FString UPerformanceMonitorComponent::GetPerformanceSummary() const
{
    FString Summary = FString::Printf(TEXT("=== Performance Summary: %s ===\n"),
        *GetOwner()->GetName());

    Summary += FString::Printf(TEXT("Frame Time: %.1f ms (Avg: %.1f ms, Peak: %.1f ms)\n"),
        CurrentFrameTime, AverageFrameTime, PeakFrameTime);
    Summary += FString::Printf(TEXT("FPS: %.1f\n"), FPS);
    Summary += FString::Printf(TEXT("Memory: %.1f MB\n"), MemoryUsage / (1024.0f * 1024.0f));
    Summary += FString::Printf(TEXT("Components: %d (%d tickable)\n"),
        ComponentCount, TickableComponentCount);

    Summary += TEXT("Status: ");
    Summary += IsPerformanceGood() ? TEXT("GOOD ✅") : TEXT("WARNING ⚠️");
    Summary += TEXT("\n");

    return Summary;
}

bool UPerformanceMonitorComponent::IsPerformanceGood() const
{
    return CurrentFrameTime < FrameTimeWarningThreshold;
}

FString UPerformanceMonitorComponent::GetPerformanceRecommendations() const
{
    FString Recommendations;

    if (CurrentFrameTime > FrameTimeWarningThreshold)
    {
        Recommendations += TEXT("• Frame time is high. Consider:\n");
        Recommendations += TEXT("  - Reducing component count\n");
        Recommendations += TEXT("  - Optimizing TickComponent implementations\n");
        Recommendations += TEXT("  - Using LOD for distant objects\n");
        Recommendations += TEXT("  - Implementing object pooling\n");
    }

    if (ComponentCount > 20)
    {
        Recommendations += TEXT("• High component count. Consider:\n");
        Recommendations += TEXT("  - Merging related functionality\n");
        Recommendations += TEXT("  - Using component aggregation\n");
        Recommendations += TEXT("  - Lazy component initialization\n");
    }

    if (TickableComponentCount > 10)
    {
        Recommendations += TEXT("• Many tickable components. Consider:\n");
        Recommendations += TEXT("  - Using timers instead of Tick\n");
        Recommendations += TEXT("  - Staggering updates across frames\n");
        Recommendations += TEXT("  - Event-driven updates\n");
    }

    if (MemoryUsage > 50 * 1024 * 1024) // 50MB
    {
        Recommendations += TEXT("• High memory usage. Consider:\n");
        Recommendations += TEXT("  - Object pooling for frequently spawned objects\n");
        Recommendations += TEXT("  - Reducing texture resolutions\n");
        Recommendations += TEXT("  - Implementing LOD for assets\n");
    }

    if (Recommendations.IsEmpty())
    {
        Recommendations = TEXT("Performance looks good! No recommendations at this time.");
    }

    return Recommendations;
}

void UPerformanceMonitorComponent::ResetMetrics()
{
    CurrentFrameTime = 0.0f;
    AverageFrameTime = 0.0f;
    PeakFrameTime = 0.0f;
    FPS = 0.0f;
    MemoryUsage = 0;
    ComponentCount = 0;
    TickableComponentCount = 0;

    FrameTimeHistory.Empty();
    AverageFrameTimes.Empty();

    TimeSinceLastUpdate = 0.0f;
    bWasPerformanceGood = true;
}

void UPerformanceMonitorComponent::ExportPerformanceData()
{
    FString Data = GetPerformanceSummary();
    Data += TEXT("\n=== Detailed Metrics ===\n");
    Data += FString::Printf(TEXT("Frame Time History (%d samples):\n"), FrameTimeHistory.Num());

    for (int32 i = 0; i < FrameTimeHistory.Num(); ++i)
    {
        Data += FString::Printf(TEXT("%.1f"), FrameTimeHistory[i]);
        if (i < FrameTimeHistory.Num() - 1)
        {
            Data += TEXT(", ");
        }
        if ((i + 1) % 10 == 0)
        {
            Data += TEXT("\n");
        }
    }

    Data += TEXT("\n\nAverage Frame Times:\n");
    for (int32 i = 0; i < AverageFrameTimes.Num(); ++i)
    {
        Data += FString::Printf(TEXT("%.1f"), AverageFrameTimes[i]);
        if (i < AverageFrameTimes.Num() - 1)
        {
            Data += TEXT(", ");
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Performance Data Export:\n%s"), *Data);
}

void UPerformanceMonitorComponent::LogPerformanceData() const
{
    UE_LOG(LogTemp, Log, TEXT("Perf[%s]: %.1fms (%.1ffps) | Mem: %.1fMB | Comp: %d/%d"),
        *GetOwner()->GetName(),
        CurrentFrameTime,
        FPS,
        MemoryUsage / (1024.0f * 1024.0f),
        TickableComponentCount,
        ComponentCount);
}

int64 UPerformanceMonitorComponent::GetActorMemoryUsage() const
{
    // This is a simplified estimation
    // In a real implementation, you'd track actual memory allocations
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return 0;
    }

    // Estimate based on component count and basic actor overhead
    int64 EstimatedMemory = 1024; // Base actor overhead

    // Add component overhead (rough estimate)
    EstimatedMemory += Owner->GetComponents().Num() * 512;

    // Add any additional tracking here...

    return EstimatedMemory;
}