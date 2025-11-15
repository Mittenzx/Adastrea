// Copyright Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PerformanceProfiler.generated.h"

/**
 * Performance metrics structure
 * 
 * Note: Currently functional metrics are FPS, FrameTimeMS, MemoryUsedMB, and VisibleActors.
 * Metrics requiring engine stats APIs (GameThreadTimeMS, RenderThreadTimeMS, GPUTimeMS, DrawCalls)
 * are placeholders and will always be zero in this implementation.
 */
USTRUCT(BlueprintType)
struct FPerformanceMetrics
{
	GENERATED_BODY()

	/** Current frames per second (FUNCTIONAL) */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float FPS;

	/** Frame time in milliseconds (FUNCTIONAL) */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float FrameTimeMS;

	/** Game thread time in milliseconds (PLACEHOLDER - always zero) */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float GameThreadTimeMS;

	/** Render thread time in milliseconds (PLACEHOLDER - always zero) */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float RenderThreadTimeMS;

	/** GPU time in milliseconds (PLACEHOLDER - always zero) */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float GPUTimeMS;

	/** Memory used in megabytes (FUNCTIONAL) */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float MemoryUsedMB;

	/** Number of draw calls (PLACEHOLDER - always zero) */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	int32 DrawCalls;

	/** Number of visible actors (FUNCTIONAL) */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	int32 VisibleActors;

	FPerformanceMetrics()
		: FPS(0.0f)
		, FrameTimeMS(0.0f)
		, GameThreadTimeMS(0.0f)
		, RenderThreadTimeMS(0.0f)
		, GPUTimeMS(0.0f)
		, MemoryUsedMB(0.0f)
		, DrawCalls(0)
		, VisibleActors(0)
	{
	}
};

/**
 * Performance Profiler Subsystem for tracking game performance
 * 
 * This subsystem provides real-time performance monitoring and metrics
 * for optimization purposes.
 * 
 * Usage:
 * 1. Access via UGameInstance::GetSubsystem<UPerformanceProfiler>()
 * 2. Call GetCurrentMetrics to get performance data
 * 3. Use for debugging and optimization
 * 
 * Example:
 * - Enable profiling during development
 * - Display FPS and frame time on HUD
 * - Log performance metrics for analysis
 */
UCLASS()
class ADASTREA_API UPerformanceProfiler : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPerformanceProfiler();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================================================
	// Configuration
	// ========================================================================

	/** Whether profiling is enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiler Settings")
	bool bProfilingEnabled;

	/** How often to update metrics (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiler Settings",
		meta = (ClampMin = 0.1, ClampMax = 5.0))
	float UpdateFrequency;

	/** Whether to log performance warnings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiler Settings")
	bool bLogPerformanceWarnings;

	/** FPS threshold for performance warnings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiler Settings",
		meta = (ClampMin = 15.0, ClampMax = 60.0))
	float WarningFPSThreshold;

	// ========================================================================
	// Blueprint Functions
	// ========================================================================

	/**
	 * Get current performance metrics
	 * @return Current performance data
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance Profiler")
	FPerformanceMetrics GetCurrentMetrics() const;

	/**
	 * Enable or disable profiling
	 * @param bEnabled Whether to enable profiling
	 */
	UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
	void SetProfilingEnabled(bool bEnabled);

	/**
	 * Get average FPS over the last N seconds
	 * @param TimeWindow Time window in seconds
	 * @return Average FPS
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance Profiler")
	float GetAverageFPS(float TimeWindow = 5.0f) const;

	/**
	 * Check if performance is below acceptable threshold
	 * @return True if performance is poor
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance Profiler")
	bool IsPerformancePoor() const;

	/**
	 * Start a named performance scope for detailed profiling
	 * @param ScopeName Name of the scope to profile
	 */
	UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
	void BeginProfileScope(const FString& ScopeName);

	/**
	 * End a named performance scope
	 * @param ScopeName Name of the scope to end
	 */
	UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
	void EndProfileScope(const FString& ScopeName);

	/**
	 * Get the time taken by a named scope (in milliseconds)
	 * @param ScopeName Name of the scope
	 * @return Time in milliseconds, or -1 if scope not found
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance Profiler")
	float GetScopeTime(const FString& ScopeName) const;

private:
	/** Current metrics */
	FPerformanceMetrics CurrentMetrics;

	/** FPS history for averaging */
	TArray<float> FPSHistory;

	/** Max history size */
	static constexpr int32 MaxHistorySize = 300; // 5 seconds at 60fps

	/** Scope timing data */
	TMap<FString, double> ScopeStartTimes;
	TMap<FString, float> ScopeTimings;

	/** Timer handle for metric updates */
	FTimerHandle MetricsUpdateTimer;

	/** Update all performance metrics */
	void UpdateMetrics();

	/** Check for performance issues and log warnings */
	void CheckPerformanceWarnings();
};
