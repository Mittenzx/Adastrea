// Copyright Mittenzx. All Rights Reserved.

#include "Performance/PerformanceProfiler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformMemory.h"
#include "TimerManager.h"
#include "EngineUtils.h"

UPerformanceProfiler::UPerformanceProfiler()
	: bProfilingEnabled(true)
	, UpdateFrequency(1.0f)
	, bLogPerformanceWarnings(true)
	, WarningFPSThreshold(30.0f)
{
}

void UPerformanceProfiler::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (bProfilingEnabled)
	{
		// Start periodic metric updates
		GetWorld()->GetTimerManager().SetTimer(MetricsUpdateTimer, this, 
			&UPerformanceProfiler::UpdateMetrics, UpdateFrequency, true);
	}
}

void UPerformanceProfiler::Deinitialize()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(MetricsUpdateTimer);
	}

	Super::Deinitialize();
}

FPerformanceMetrics UPerformanceProfiler::GetCurrentMetrics() const
{
	return CurrentMetrics;
}

void UPerformanceProfiler::SetProfilingEnabled(bool bEnabled)
{
	bProfilingEnabled = bEnabled;

	if (bEnabled && !MetricsUpdateTimer.IsValid())
	{
		GetWorld()->GetTimerManager().SetTimer(MetricsUpdateTimer, this,
			&UPerformanceProfiler::UpdateMetrics, UpdateFrequency, true);
	}
	else if (!bEnabled && MetricsUpdateTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(MetricsUpdateTimer);
	}
}

float UPerformanceProfiler::GetAverageFPS(float TimeWindow) const
{
	if (FPSHistory.Num() == 0)
	{
		return 0.0f;
	}

	// Calculate how many samples to average based on time window
	int32 SamplesToAverage = FMath::Min(
		static_cast<int32>(TimeWindow / UpdateFrequency),
		FPSHistory.Num()
	);

	float Sum = 0.0f;
	for (int32 i = FPSHistory.Num() - SamplesToAverage; i < FPSHistory.Num(); ++i)
	{
		Sum += FPSHistory[i];
	}

	return Sum / static_cast<float>(SamplesToAverage);
}

bool UPerformanceProfiler::IsPerformancePoor() const
{
	return CurrentMetrics.FPS < WarningFPSThreshold;
}

void UPerformanceProfiler::BeginProfileScope(const FString& ScopeName)
{
	if (!bProfilingEnabled)
	{
		return;
	}

	ScopeStartTimes.Add(ScopeName, FPlatformTime::Seconds());
}

void UPerformanceProfiler::EndProfileScope(const FString& ScopeName)
{
	if (!bProfilingEnabled)
	{
		return;
	}

	double* StartTime = ScopeStartTimes.Find(ScopeName);
	if (StartTime)
	{
		double EndTime = FPlatformTime::Seconds();
		float ElapsedMS = static_cast<float>((EndTime - *StartTime) * 1000.0);
		ScopeTimings.Add(ScopeName, ElapsedMS);
		ScopeStartTimes.Remove(ScopeName);
	}
}

float UPerformanceProfiler::GetScopeTime(const FString& ScopeName) const
{
	const float* Time = ScopeTimings.Find(ScopeName);
	return Time ? *Time : -1.0f;
}

void UPerformanceProfiler::UpdateMetrics()
{
	if (!bProfilingEnabled)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Update FPS
	float DeltaTime = World->GetDeltaSeconds();
	CurrentMetrics.FPS = (DeltaTime > 0.0f) ? (1.0f / DeltaTime) : 0.0f;
	CurrentMetrics.FrameTimeMS = DeltaTime * 1000.0f;

	// Add to history
	FPSHistory.Add(CurrentMetrics.FPS);
	if (FPSHistory.Num() > MaxHistorySize)
	{
		FPSHistory.RemoveAt(0);
	}

	// Update memory usage
	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	CurrentMetrics.MemoryUsedMB = static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f);

	// Count visible actors (approximate)
	CurrentMetrics.VisibleActors = 0;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor && !Actor->IsHidden())
		{
			CurrentMetrics.VisibleActors++;
		}
	}

	// NOTE: Only the following metrics are currently functional:
	//   - FPS
	//   - FrameTimeMS
	//   - MemoryUsedMB
	//   - VisibleActors
	// The following metrics are placeholders and will always be zero in this implementation:
	//   - GameThreadTimeMS
	//   - RenderThreadTimeMS
	//   - GPUTimeMS
	//   - DrawCalls
	// These require engine stats APIs (FEngineLoop stats or RHI queries) which may not be available in all builds.

	// Check for performance warnings
	if (bLogPerformanceWarnings)
	{
		CheckPerformanceWarnings();
	}
}

void UPerformanceProfiler::CheckPerformanceWarnings()
{
	if (IsPerformancePoor())
	{
		UE_LOG(LogTemp, Warning, TEXT("Performance Warning: FPS dropped to %.1f (threshold: %.1f)"),
			CurrentMetrics.FPS, WarningFPSThreshold);
	}

	// Memory warning (if using more than 2GB)
	if (CurrentMetrics.MemoryUsedMB > 2048.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Performance Warning: High memory usage: %.1f MB"),
			CurrentMetrics.MemoryUsedMB);
	}
}
