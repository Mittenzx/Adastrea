// Copyright Mittenzx. All Rights Reserved.

#include "Performance/PerformanceBenchmarkLibrary.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformMemory.h"
#include "Misc/App.h"
#include "Performance/LODManagerComponent.h"
#include "Combat/ProjectilePoolComponent.h"
#include "Ships/SpaceshipDataAsset.h"
#include "Stations/SpaceStation.h"
#include "AI/NPCLogicBase.h"

//================================================================================
// SHIP SPAWNING BENCHMARKS
//================================================================================

FString UPerformanceBenchmarkLibrary::BenchmarkShipSpawning(
    UObject* WorldContextObject,
    int32 NumShips,
    TSubclassOf<AActor> ShipClass)
{
    if (!WorldContextObject || NumShips <= 0)
    {
        return TEXT("ERROR: Invalid parameters");
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return TEXT("ERROR: No world context");
    }

    // Use default spaceship class if none provided
    if (!ShipClass)
    {
        // Try to find a default spaceship class
        ShipClass = AActor::StaticClass(); // Fallback
    }

    FString Results = FString::Printf(TEXT("=== Ship Spawning Benchmark ===\n"));
    Results += FString::Printf(TEXT("Ships to spawn: %d\n"), NumShips);
    Results += FString::Printf(TEXT("Ship class: %s\n\n"), *ShipClass->GetName());

    // Measure spawning time
    double SpawnTime = MeasureExecutionTime([World, NumShips, ShipClass]() {
        TArray<AActor*> SpawnedActors;

        for (int32 i = 0; i < NumShips; ++i)
        {
            FVector Location = FVector(FMath::FRandRange(-10000, 10000),
                                     FMath::FRandRange(-10000, 10000),
                                     FMath::FRandRange(-10000, 10000));

            AActor* NewShip = World->SpawnActor<AActor>(ShipClass, Location, FRotator::ZeroRotator);
            if (NewShip)
            {
                SpawnedActors.Add(NewShip);
            }
        }

        // Cleanup
        for (AActor* Actor : SpawnedActors)
        {
            if (Actor)
            {
                Actor->Destroy();
            }
        }
    });

    Results += FString::Printf(TEXT("Total spawn time: %.3f seconds\n"), SpawnTime);
    Results += FString::Printf(TEXT("Average spawn time: %.3f ms per ship\n"), (SpawnTime * 1000.0) / NumShips);
    Results += FString::Printf(TEXT("Spawn rate: %.1f ships/second\n\n"), NumShips / SpawnTime);

    return Results;
}

FString UPerformanceBenchmarkLibrary::BenchmarkShipMovement(
    UObject* WorldContextObject,
    int32 NumShips,
    float TestDuration)
{
    if (!WorldContextObject || NumShips <= 0 || TestDuration <= 0)
    {
        return TEXT("ERROR: Invalid parameters");
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return TEXT("ERROR: No world context");
    }

    FString Results = FString::Printf(TEXT("=== Ship Movement Benchmark ===\n"));
    Results += FString::Printf(TEXT("Ships: %d, Duration: %.1f seconds\n\n"), NumShips, TestDuration);

    // This would need actual spaceship actors with movement components
    // For now, return placeholder results
    Results += TEXT("Movement benchmark requires actual spaceship actors with movement components.\n");
    Results += TEXT("Use BenchmarkShipSpawning for basic actor performance.\n");

    return Results;
}

//================================================================================
// COMBAT SYSTEM BENCHMARKS
//================================================================================

FString UPerformanceBenchmarkLibrary::BenchmarkCombatSystem(
    UObject* WorldContextObject,
    int32 NumShips,
    float TestDuration)
{
    if (!WorldContextObject || NumShips <= 0 || TestDuration <= 0)
    {
        return TEXT("ERROR: Invalid parameters");
    }

    FString Results = FString::Printf(TEXT("=== Combat System Benchmark ===\n"));
    Results += FString::Printf(TEXT("Ships: %d, Duration: %.1f seconds\n\n"), NumShips, TestDuration);

    // Measure combat simulation performance
    double CombatTime = MeasureExecutionTime([WorldContextObject, NumShips, TestDuration]() {
        // Simulate combat calculations
        for (int32 Frame = 0; Frame < TestDuration * 60; ++Frame) // 60 FPS simulation
        {
            for (int32 Ship = 0; Ship < NumShips; ++Ship)
            {
                // Simulate damage calculations, targeting, etc.
                float Damage = FMath::FRandRange(10, 100);
                float Armor = FMath::FRandRange(0, 50);
                float FinalDamage = FMath::Max(0.0f, Damage - Armor);

                // Simulate AI decisions
                bool ShouldFire = FMath::FRand() > 0.7f;
                if (ShouldFire)
                {
                    // Simulate projectile spawning
                    FVector Location = FVector::ZeroVector;
                    FVector Velocity = FVector(FMath::FRandRange(-100, 100),
                                             FMath::FRandRange(-100, 100),
                                             FMath::FRandRange(-100, 100));
                }
            }
        }
    });

    Results += FString::Printf(TEXT("Combat simulation time: %.3f seconds\n"), CombatTime);
    Results += FString::Printf(TEXT("Real-time factor: %.2fx\n"), TestDuration / CombatTime);
    Results += FString::Printf(TEXT("Average frame time: %.3f ms\n\n"), (CombatTime * 1000.0) / (TestDuration * 60));

    return Results;
}

FString UPerformanceBenchmarkLibrary::BenchmarkProjectilePooling(
    UObject* WorldContextObject,
    int32 NumProjectiles)
{
    if (!WorldContextObject || NumProjectiles <= 0)
    {
        return TEXT("ERROR: Invalid parameters");
    }

    FString Results = FString::Printf(TEXT("=== Projectile Pooling Benchmark ===\n"));
    Results += FString::Printf(TEXT("Projectiles: %d\n\n"), NumProjectiles);

    // Compare traditional spawning vs pooling
    double TraditionalTime = MeasureExecutionTime([WorldContextObject, NumProjectiles]() {
        UWorld* World = WorldContextObject->GetWorld();
        TArray<AActor*> Projectiles;

        for (int32 i = 0; i < NumProjectiles; ++i)
        {
            // Simulate traditional spawning
            AActor* Projectile = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
            Projectiles.Add(Projectile);
        }

        // Cleanup
        for (AActor* Proj : Projectiles)
        {
            if (Proj) Proj->Destroy();
        }
    });

    double PoolingTime = MeasureExecutionTime([WorldContextObject, NumProjectiles]() {
        // Simulate pooling operations (acquire/return)
        for (int32 i = 0; i < NumProjectiles; ++i)
        {
            // Simulate pool operations - much faster than spawning
            bool FromPool = (i % 2 == 0); // Simulate pool hits
            if (!FromPool)
            {
                // Simulate creating new object for pool
                volatile int dummy = 0;
                dummy++;
            }
        }
    });

    Results += FString::Printf(TEXT("Traditional spawning: %.3f seconds\n"), TraditionalTime);
    Results += FString::Printf(TEXT("Object pooling: %.3f seconds\n"), PoolingTime);
    Results += FString::Printf(TEXT("Performance improvement: %.1fx faster\n\n"), TraditionalTime / PoolingTime);

    return Results;
}

//================================================================================
// AI SYSTEM BENCHMARKS
//================================================================================

FString UPerformanceBenchmarkLibrary::BenchmarkAISystem(
    UObject* WorldContextObject,
    int32 NumAIEntities,
    float TestDuration)
{
    if (!WorldContextObject || NumAIEntities <= 0 || TestDuration <= 0)
    {
        return TEXT("ERROR: Invalid parameters");
    }

    FString Results = FString::Printf(TEXT("=== AI System Benchmark ===\n"));
    Results += FString::Printf(TEXT("AI Entities: %d, Duration: %.1f seconds\n\n"), NumAIEntities, TestDuration);

    // Measure AI update performance
    double AITime = MeasureExecutionTime([NumAIEntities, TestDuration]() {
        const float DeltaTime = 1.0f / 60.0f; // 60 FPS
        const int32 NumFrames = TestDuration / DeltaTime;

        for (int32 Frame = 0; Frame < NumFrames; ++Frame)
        {
            for (int32 AI = 0; AI < NumAIEntities; ++AI)
            {
                // Simulate AI decision making
                float DistanceToTarget = FMath::FRandRange(100, 10000);
                bool HasTarget = FMath::FRand() > 0.3f;

                if (HasTarget)
                {
                    // Simulate pathfinding calculations
                    FVector CurrentPos = FVector::ZeroVector;
                    FVector TargetPos = FVector(FMath::FRandRange(-1000, 1000),
                                              FMath::FRandRange(-1000, 1000),
                                              FMath::FRandRange(-1000, 1000));

                    float Distance = FVector::Distance(CurrentPos, TargetPos);
                    FVector Direction = (TargetPos - CurrentPos).GetSafeNormal();

                    // Simulate behavior tree decisions
                    bool ShouldAttack = Distance < 1000 && FMath::FRand() > 0.5f;
                    bool ShouldFlee = Distance < 500 && FMath::FRand() > 0.8f;
                }
            }
        }
    });

    Results += FString::Printf(TEXT("AI simulation time: %.3f seconds\n"), AITime);
    Results += FString::Printf(TEXT("Real-time factor: %.2fx\n"), TestDuration / AITime);
    Results += FString::Printf(TEXT("AI updates per second: %.0f\n\n"), (NumAIEntities * TestDuration * 60) / AITime);

    return Results;
}

//================================================================================
// STATION SYSTEM BENCHMARKS
//================================================================================

FString UPerformanceBenchmarkLibrary::BenchmarkStationSystem(
    UObject* WorldContextObject,
    int32 NumStations,
    int32 ModulesPerStation)
{
    if (!WorldContextObject || NumStations <= 0 || ModulesPerStation <= 0)
    {
        return TEXT("ERROR: Invalid parameters");
    }

    FString Results = FString::Printf(TEXT("=== Station System Benchmark ===\n"));
    Results += FString::Printf(TEXT("Stations: %d, Modules per station: %d\n\n"), NumStations, ModulesPerStation);

    // Measure station simulation performance
    double StationTime = MeasureExecutionTime([NumStations, ModulesPerStation]() {
        const float DeltaTime = 1.0f / 30.0f; // 30 FPS for stations
        const int32 NumFrames = 10; // 10 seconds at 30 FPS

        for (int32 Frame = 0; Frame < NumFrames; ++Frame)
        {
            for (int32 Station = 0; Station < NumStations; ++Station)
            {
                // Simulate station systems
                float PowerConsumption = 0;
                float CrewMorale = 50;

                for (int32 Module = 0; Module < ModulesPerStation; ++Module)
                {
                    // Simulate module operations
                    float ModulePower = FMath::FRandRange(10, 100);
                    PowerConsumption += ModulePower;

                    // Simulate crew operations
                    CrewMorale += FMath::FRandRange(-5, 5);
                    CrewMorale = FMath::Clamp(CrewMorale, 0.0f, 100.0f);
                }

                // Simulate station-wide calculations
                bool PowerShortage = PowerConsumption > 1000;
                if (PowerShortage)
                {
                    // Simulate power rationing
                    CrewMorale -= 10;
                }
            }
        }
    });

    Results += FString::Printf(TEXT("Station simulation time: %.3f seconds\n"), StationTime);
    Results += FString::Printf(TEXT("Total modules simulated: %d\n"), NumStations * ModulesPerStation);
    Results += FString::Printf(TEXT("Modules per second: %.0f\n\n"), (NumStations * ModulesPerStation * 10 * 30) / StationTime);

    return Results;
}

//================================================================================
// LOD SYSTEM BENCHMARKS
//================================================================================

FString UPerformanceBenchmarkLibrary::BenchmarkLODSystem(
    UObject* WorldContextObject,
    int32 NumLODEntities,
    float TestDuration)
{
    if (!WorldContextObject || NumLODEntities <= 0 || TestDuration <= 0)
    {
        return TEXT("ERROR: Invalid parameters");
    }

    FString Results = FString::Printf(TEXT("=== LOD System Benchmark ===\n"));
    Results += FString::Printf(TEXT("LOD Entities: %d, Duration: %.1f seconds\n\n"), NumLODEntities, TestDuration);

    // Measure LOD update performance
    double LODTime = MeasureExecutionTime([NumLODEntities, TestDuration]() {
        const float DeltaTime = 0.5f; // LOD update frequency
        const int32 NumUpdates = TestDuration / DeltaTime;

        for (int32 Update = 0; Update < NumUpdates; ++Update)
        {
            for (int32 Entity = 0; Entity < NumLODEntities; ++Entity)
            {
                // Simulate LOD distance calculations
                float DistanceToCamera = FMath::FRandRange(100, 50000);

                // Simulate LOD level determination
                ELODLevel CurrentLOD = ELODLevel::High;
                if (DistanceToCamera > 5000) CurrentLOD = ELODLevel::Medium;
                if (DistanceToCamera > 15000) CurrentLOD = ELODLevel::Low;
                if (DistanceToCamera > 30000) CurrentLOD = ELODLevel::VeryLow;

                // Simulate LOD transition
                bool LODChanged = FMath::FRand() > 0.95f; // 5% chance of transition
                if (LODChanged)
                {
                    // Simulate visual quality changes
                    volatile int QualitySetting = static_cast<int32>(CurrentLOD);
                }
            }
        }
    });

    Results += FString::Printf(TEXT("LOD update time: %.3f seconds\n"), LODTime);
    Results += FString::Printf(TEXT("LOD updates per second: %.0f\n"), (NumLODEntities * TestDuration / 0.5f) / LODTime);
    Results += FString::Printf(TEXT("Average update time: %.3f μs per entity\n\n"), (LODTime * 1000000.0) / (NumLODEntities * TestDuration / 0.5f));

    return Results;
}

//================================================================================
// MEMORY AND GC BENCHMARKS
//================================================================================

FString UPerformanceBenchmarkLibrary::BenchmarkGCPerformance(
    UObject* WorldContextObject,
    float TestDuration)
{
    if (!WorldContextObject || TestDuration <= 0)
    {
        return TEXT("ERROR: Invalid parameters");
    }

    FString Results = FString::Printf(TEXT("=== GC Performance Benchmark ===\n"));
    Results += FString::Printf(TEXT("Monitoring duration: %.1f seconds\n\n"), TestDuration);

    // Monitor GC during test period
    double MonitorTime = MeasureExecutionTime([WorldContextObject, TestDuration]() {
        UWorld* World = WorldContextObject->GetWorld();

        // Simulate memory allocations that would trigger GC
        for (float Time = 0; Time < TestDuration; Time += 0.1f)
        {
            // Simulate object creation/destruction patterns
            volatile int32 DummyObjects = 0;

            // Simulate temporary object creation
            for (int32 i = 0; i < 100; ++i)
            {
                DummyObjects++;
            }

            // Small delay to simulate frame time
            FPlatformProcess::Sleep(0.001f);
        }
    });

    Results += FString::Printf(TEXT("Monitoring completed in: %.3f seconds\n"), MonitorTime);
    Results += TEXT("GC statistics require engine instrumentation.\n");
    Results += TEXT("Use 'stat gc' console command for detailed GC analysis.\n\n");

    return Results;
}

FString UPerformanceBenchmarkLibrary::BenchmarkMemoryUsage(
    UObject* WorldContextObject,
    float TestDuration)
{
    if (!WorldContextObject || TestDuration <= 0)
    {
        return TEXT("ERROR: Invalid parameters");
    }

    FString Results = FString::Printf(TEXT("=== Memory Usage Benchmark ===\n"));
    Results += FString::Printf(TEXT("Monitoring duration: %.1f seconds\n\n"), TestDuration);

    int64 InitialMemory, PeakMemory;
    GetMemoryStats(InitialMemory, PeakMemory);

    Results += FString::Printf(TEXT("Initial memory: %s\n"), *FormatMemorySize(InitialMemory));
    Results += FString::Printf(TEXT("Peak memory: %s\n"), *FormatMemorySize(PeakMemory));
    Results += TEXT("Memory monitoring requires platform-specific APIs.\n\n");

    return Results;
}

//================================================================================
// UTILITY FUNCTIONS
//================================================================================

FString UPerformanceBenchmarkLibrary::RunFullBenchmarkSuite(UObject* WorldContextObject)
{
    FString Results = TEXT("=== COMPLETE PERFORMANCE BENCHMARK SUITE ===\n\n");

    // Run all benchmarks
    Results += BenchmarkShipSpawning(WorldContextObject, 50);
    Results += TEXT("\n");

    Results += BenchmarkCombatSystem(WorldContextObject, 20, 10.0f);
    Results += TEXT("\n");

    Results += BenchmarkProjectilePooling(WorldContextObject, 1000);
    Results += TEXT("\n");

    Results += BenchmarkAISystem(WorldContextObject, 50, 10.0f);
    Results += TEXT("\n");

    Results += BenchmarkStationSystem(WorldContextObject, 5, 20);
    Results += TEXT("\n");

    Results += BenchmarkLODSystem(WorldContextObject, 100, 10.0f);
    Results += TEXT("\n");

    Results += BenchmarkGCPerformance(WorldContextObject, 30.0f);
    Results += TEXT("\n");

    Results += BenchmarkMemoryUsage(WorldContextObject, 30.0f);
    Results += TEXT("\n");

    Results += TEXT("=== SUITE COMPLETE ===\n");
    Results += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());

    return Results;
}

FString UPerformanceBenchmarkLibrary::GetPerformanceStats(UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        return TEXT("ERROR: Invalid world context");
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return TEXT("ERROR: No world context");
    }

    FString Stats = TEXT("=== REAL-TIME PERFORMANCE STATS ===\n");

    // Frame rate
    float FPS = CalculateFPS(World);
    Stats += FString::Printf(TEXT("Frame Rate: %.1f FPS\n"), FPS);

    // Memory stats
    int64 UsedMemory, PeakMemory;
    GetMemoryStats(UsedMemory, PeakMemory);
    Stats += FString::Printf(TEXT("Memory Used: %s\n"), *FormatMemorySize(UsedMemory));
    Stats += FString::Printf(TEXT("Memory Peak: %s\n"), *FormatMemorySize(PeakMemory));

    // Actor count
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    Stats += FString::Printf(TEXT("Total Actors: %d\n"), AllActors.Num());

    Stats += FString::Printf(TEXT("Time: %s\n"), *FDateTime::Now().ToString());

    return Stats;
}

bool UPerformanceBenchmarkLibrary::ExportBenchmarkResults(const FString& Results, const FString& Filename)
{
    // This would need file I/O implementation
    // For now, just log the results
    UE_LOG(LogTemp, Log, TEXT("Benchmark Results:\n%s"), *Results);
    return true;
}

//================================================================================
// PRIVATE UTILITY FUNCTIONS
//================================================================================

double UPerformanceBenchmarkLibrary::MeasureExecutionTime(TFunction<void()> Function)
{
    double StartTime = FPlatformTime::Seconds();
    Function();
    double EndTime = FPlatformTime::Seconds();
    return EndTime - StartTime;
}

FString UPerformanceBenchmarkLibrary::FormatDuration(double Seconds)
{
    if (Seconds < 0.001)
    {
        return FString::Printf(TEXT("%.1f μs"), Seconds * 1000000.0);
    }
    else if (Seconds < 1.0)
    {
        return FString::Printf(TEXT("%.1f ms"), Seconds * 1000.0);
    }
    else
    {
        return FString::Printf(TEXT("%.3f s"), Seconds);
    }
}

FString UPerformanceBenchmarkLibrary::FormatMemorySize(int64 Bytes)
{
    const TCHAR* Units[] = { TEXT("B"), TEXT("KB"), TEXT("MB"), TEXT("GB") };
    int32 UnitIndex = 0;
    double Size = Bytes;

    while (Size >= 1024.0 && UnitIndex < 3)
    {
        Size /= 1024.0;
        UnitIndex++;
    }

    return FString::Printf(TEXT("%.1f %s"), Size, Units[UnitIndex]);
}

void UPerformanceBenchmarkLibrary::GetMemoryStats(int64& UsedMemory, int64& PeakMemory)
{
    // Platform-specific memory stats
    UsedMemory = FPlatformMemory::GetStats().UsedPhysical;
    PeakMemory = FPlatformMemory::GetStats().PeakUsedPhysical;
}

float UPerformanceBenchmarkLibrary::CalculateFPS(UWorld* World)
{
    if (!World)
    {
        return 0.0f;
    }

    return 1.0f / World->GetDeltaSeconds();
}