// Copyright Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PerformanceBenchmarkLibrary.generated.h"

/**
 * Performance Benchmarking Library
 *
 * Provides comprehensive performance testing tools for Adastrea systems.
 * Use these functions to measure and validate performance improvements.
 *
 * Usage:
 * - Call benchmark functions in development builds
 * - Results are logged to console and can be captured for analysis
 * - Use in automated testing or manual performance validation
 *
 * Example:
 * @code
 * FString Result = UPerformanceBenchmarkLibrary::BenchmarkShipSpawning(this, 100);
 * UE_LOG(LogTemp, Log, TEXT("Benchmark Result: %s"), *Result);
 * @endcode
 */
UCLASS()
class ADASTREA_API UPerformanceBenchmarkLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    //================================================================================
    // SHIP SPAWNING BENCHMARKS
    //================================================================================

    /**
     * Benchmark ship spawning performance
     * Measures time to spawn, initialize, and destroy ships
     *
     * @param WorldContextObject World context for spawning
     * @param NumShips Number of ships to spawn (recommended: 10-1000)
     * @param ShipClass Class of ship to spawn (nullptr = default fighter)
     * @return Benchmark results as formatted string
     */
    UFUNCTION(BlueprintCallable, Category="Performance|Benchmarks|Ships",
        meta=(WorldContext="WorldContextObject"))
    static FString BenchmarkShipSpawning(
        UObject* WorldContextObject,
        int32 NumShips,
        TSubclassOf<AActor> ShipClass = nullptr
    );

    /**
     * Benchmark ship movement and physics performance
     * Measures performance with ships in motion
     *
     * @param WorldContextObject World context
     * @param NumShips Number of ships to test
     * @param TestDuration Duration of test in seconds
     * @return Benchmark results
     */
    UFUNCTION(BlueprintCallable, Category="Performance|Benchmarks|Ships",
        meta=(WorldContext="WorldContextObject"))
    static FString BenchmarkShipMovement(
        UObject* WorldContextObject,
        int32 NumShips,
        float TestDuration = 10.0f
    );

    //================================================================================
    // COMBAT SYSTEM BENCHMARKS
    //================================================================================

    /**
     * Benchmark combat system performance
     * Measures projectile spawning, collision detection, and damage calculation
     *
     * @param WorldContextObject World context
     * @param NumShips Number of combat ships
     * @param TestDuration Duration of combat test
     * @return Benchmark results
     */
    UFUNCTION(BlueprintCallable, Category="Performance|Benchmarks|Combat",
        meta=(WorldContext="WorldContextObject"))
    static FString BenchmarkCombatSystem(
        UObject* WorldContextObject,
        int32 NumShips,
        float TestDuration = 30.0f
    );

    /**
     * Benchmark projectile pooling vs traditional spawning
     * Compares performance of pooled vs non-pooled projectiles
     *
     * @param WorldContextObject World context
     * @param NumProjectiles Number of projectiles to test
     * @return Comparison results
     */
    UFUNCTION(BlueprintCallable, Category="Performance|Benchmarks|Combat",
        meta=(WorldContext="WorldContextObject"))
    static FString BenchmarkProjectilePooling(
        UObject* WorldContextObject,
        int32 NumProjectiles
    );

    //================================================================================
    // AI SYSTEM BENCHMARKS
    //================================================================================

    /**
     * Benchmark AI system performance
     * Measures AI update frequency and decision-making performance
     *
     * @param WorldContextObject World context
     * @param NumAIEntities Number of AI entities to test
     * @param TestDuration Duration of test
     * @return Benchmark results
     */
    UFUNCTION(BlueprintCallable, Category="Performance|Benchmarks|AI",
        meta=(WorldContext="WorldContextObject"))
    static FString BenchmarkAISystem(
        UObject* WorldContextObject,
        int32 NumAIEntities,
        float TestDuration = 30.0f
    );

    //================================================================================
    // STATION SYSTEM BENCHMARKS
    //================================================================================

    /**
     * Benchmark space station performance
     * Measures station module updates and interaction performance
     *
     * @param WorldContextObject World context
     * @param NumStations Number of stations to test
     * @param ModulesPerStation Average modules per station
     * @return Benchmark results
     */
    UFUNCTION(BlueprintCallable, Category="Performance|Benchmarks|Stations",
        meta=(WorldContext="WorldContextObject"))
    static FString BenchmarkStationSystem(
        UObject* WorldContextObject,
        int32 NumStations,
        int32 ModulesPerStation = 10
    );

    //================================================================================
    // LOD SYSTEM BENCHMARKS
    //================================================================================

    /**
     * Benchmark LOD system performance
     * Measures LOD update frequency and visual quality transitions
     *
     * @param WorldContextObject World context
     * @param NumLODEntities Number of entities with LOD components
     * @param TestDuration Duration of test
     * @return Benchmark results
     */
    UFUNCTION(BlueprintCallable, Category="Performance|Benchmarks|LOD",
        meta=(WorldContext="WorldContextObject"))
    static FString BenchmarkLODSystem(
        UObject* WorldContextObject,
        int32 NumLODEntities,
        float TestDuration = 30.0f
    );

    //================================================================================
    // MEMORY AND GC BENCHMARKS
    //================================================================================

    /**
     * Benchmark garbage collection performance
     * Measures GC frequency and duration under load
     *
     * @param WorldContextObject World context
     * @param TestDuration Duration to monitor GC
     * @return GC performance analysis
     */
    UFUNCTION(BlueprintCallable, Category="Performance|Benchmarks|Memory",
        meta=(WorldContext="WorldContextObject"))
    static FString BenchmarkGCPerformance(
        UObject* WorldContextObject,
        float TestDuration = 60.0f
    );

    /**
     * Benchmark memory allocation patterns
     * Measures memory usage growth over time
     *
     * @param WorldContextObject World context
     * @param TestDuration Duration to monitor memory
     * @return Memory usage analysis
     */
    UFUNCTION(BlueprintCallable, Category="Performance|Benchmarks|Memory",
        meta=(WorldContext="WorldContextObject"))
    static FString BenchmarkMemoryUsage(
        UObject* WorldContextObject,
        float TestDuration = 60.0f
    );

    //================================================================================
    // UTILITY FUNCTIONS
    //================================================================================

    /**
     * Run all performance benchmarks
     * Comprehensive test suite for all systems
     *
     * @param WorldContextObject World context
     * @return Complete benchmark report
     */
    UFUNCTION(BlueprintCallable, Category="Performance|Benchmarks",
        meta=(WorldContext="WorldContextObject"))
    static FString RunFullBenchmarkSuite(UObject* WorldContextObject);

    /**
     * Get current performance statistics
     * Real-time performance metrics
     *
     * @param WorldContextObject World context
     * @return Current performance stats
     */
    UFUNCTION(BlueprintCallable, Category="Performance|Stats",
        meta=(WorldContext="WorldContextObject"))
    static FString GetPerformanceStats(UObject* WorldContextObject);

    /**
     * Export benchmark results to file
     * Saves results for external analysis
     *
     * @param Results Benchmark results string
     * @param Filename Output filename (without path)
     * @return Success status
     */
    UFUNCTION(BlueprintCallable, Category="Performance|Utilities")
    static bool ExportBenchmarkResults(const FString& Results, const FString& Filename);

    /**
     * Measure execution time of a function
     * @param Function Function to measure
     * @return Execution time in seconds
     */
    static double MeasureExecutionTime(TFunction<void()> Function);

private:

    /** Format time duration for display */
    static FString FormatDuration(double Seconds);

    /** Format memory size for display */
    static FString FormatMemorySize(int64 Bytes);

    /** Get current memory statistics */
    static void GetMemoryStats(int64& UsedMemory, int64& PeakMemory);

    /** Calculate frames per second */
    static float CalculateFPS(UWorld* World);
};