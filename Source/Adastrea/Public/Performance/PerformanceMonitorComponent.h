// Copyright Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PerformanceMonitorComponent.generated.h"

/**
 * Performance Monitor Component
 *
 * Tracks real-time performance metrics for individual actors.
 * Useful for identifying performance bottlenecks and optimizing specific systems.
 *
 * Features:
 * - Frame time tracking
 * - Memory usage monitoring
 * - Component update frequency analysis
 * - Performance warnings and alerts
 *
 * Usage:
 * 1. Add to any actor that needs performance monitoring
 * 2. Configure monitoring parameters
 * 3. Access metrics via Blueprint or C++
 * 4. Use for debugging and optimization
 */
UCLASS(ClassGroup = (Performance), meta = (BlueprintSpawnableComponent))
class ADASTREA_API UPerformanceMonitorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerformanceMonitorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    //================================================================================
    // CONFIGURATION
    //================================================================================

    /** Enable performance monitoring */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor")
    bool bEnabled;

    /** How often to update metrics (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor",
        meta = (ClampMin = 0.1, ClampMax = 10.0))
    float UpdateFrequency;

    /** Enable detailed logging */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor")
    bool bDetailedLogging;

    /** Warning threshold for frame time (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor",
        meta = (ClampMin = 1.0, ClampMax = 100.0))
    float FrameTimeWarningThreshold;

    /** Enable performance alerts */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor")
    bool bEnableAlerts;

    //================================================================================
    // REAL-TIME METRICS
    //================================================================================

    /** Current frame time (ms) */
    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float CurrentFrameTime;

    /** Average frame time over last second (ms) */
    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float AverageFrameTime;

    /** Peak frame time in last second (ms) */
    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float PeakFrameTime;

    /** Frames per second */
    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float FPS;

    /** Memory usage (bytes) */
    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    int64 MemoryUsage;

    /** Number of components on this actor */
    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    int32 ComponentCount;

    /** Number of tickable components */
    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    int32 TickableComponentCount;

    //================================================================================
    // HISTORICAL DATA
    //================================================================================

    /** Frame time history (last 60 frames) */
    UPROPERTY(BlueprintReadOnly, Category = "Performance History")
    TArray<float> FrameTimeHistory;

    /** Average frame times over time */
    UPROPERTY(BlueprintReadOnly, Category = "Performance History")
    TArray<float> AverageFrameTimes;

    //================================================================================
    // BLUEPRINT FUNCTIONS
    //================================================================================

    /**
     * Get current performance summary
     * @return Formatted performance report
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance Monitor")
    FString GetPerformanceSummary() const;

    /**
     * Check if performance is within acceptable limits
     * @return True if performance is good
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance Monitor")
    bool IsPerformanceGood() const;

    /**
     * Get performance recommendations
     * @return Suggestions for optimization
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance Monitor")
    FString GetPerformanceRecommendations() const;

    /**
     * Reset performance metrics
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void ResetMetrics();

    /**
     * Export performance data to log
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void ExportPerformanceData();

    //================================================================================
    // EVENTS
    //================================================================================

    /**
     * Event fired when performance drops below threshold
     * @param FrameTime Current frame time in ms
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Performance Events")
    void OnPerformanceWarning(float FrameTime);

    /**
     * Event fired when performance returns to normal
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Performance Events")
    void OnPerformanceRecovered();

private:
    /** Time since last update */
    float TimeSinceLastUpdate;

    /** Frame time buffer for averaging */
    TArray<float> FrameTimeBuffer;

    /** Previous performance state */
    bool bWasPerformanceGood;

    /** Update performance metrics */
    void UpdateMetrics(float DeltaTime);

    /** Calculate component statistics */
    void CalculateComponentStats();

    /** Check for performance issues */
    void CheckPerformanceThresholds();

    /** Log performance data */
    void LogPerformanceData() const;

    /** Get memory usage for this actor */
    int64 GetActorMemoryUsage() const;
};