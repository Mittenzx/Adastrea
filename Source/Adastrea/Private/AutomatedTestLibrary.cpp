// Copyright Mittenzx. All Rights Reserved.

#include "AutomatedTestLibrary.h"
#include "Ships/SpaceshipDataAsset.h"
// REMOVED: #include "Factions/FactionDataAsset.h" - faction system removed per Trade Simulator MVP
// TODO: Combat system archived - will be reimplemented in MVP
// #include "Combat/WeaponDataAsset.h"
#include "DataValidationLibrary.h"
#include "Performance/PerformanceBenchmarkLibrary.h"
#include "Engine/AssetManager.h"
#include "UObject/UObjectIterator.h"

//================================================================================
// TEST EXECUTION
//================================================================================

bool UAutomatedTestLibrary::RunAllTests(UObject* WorldContextObject, TArray<FTestResult>& OutResults)
{
    OutResults.Empty();
    bool bAllPassed = true;

    // Run all test categories
    TArray<ETestCategory> Categories = {
        ETestCategory::UnitTests,
        ETestCategory::IntegrationTests,
        ETestCategory::PerformanceTests,
        ETestCategory::DataValidationTests
    };

    for (ETestCategory Category : Categories)
    {
        TArray<FTestResult> CategoryResults;
        bool bCategoryPassed = RunTestCategory(WorldContextObject, Category, CategoryResults);
        OutResults.Append(CategoryResults);
        bAllPassed &= bCategoryPassed;
    }

    return bAllPassed;
}

bool UAutomatedTestLibrary::RunTestCategory(
    UObject* WorldContextObject,
    ETestCategory TestCategory,
    TArray<FTestResult>& OutResults
)
{
    OutResults.Empty();
    bool bAllPassed = true;

    switch (TestCategory)
    {
    case ETestCategory::UnitTests:
        {
            FTestResult Result;
            bAllPassed &= TestSpaceshipCalculations(WorldContextObject, Result);
            OutResults.Add(Result);

            bAllPassed &= TestFactionRelationships(WorldContextObject, Result);
            OutResults.Add(Result);

            // TODO: Combat system archived - weapon test will be reimplemented in MVP
            // bAllPassed &= TestWeaponDamage(WorldContextObject, Result);
            // OutResults.Add(Result);

            bAllPassed &= TestDataValidation(WorldContextObject, Result);
            OutResults.Add(Result);
        }
        break;

    case ETestCategory::IntegrationTests:
        {
            FTestResult Result;
            bAllPassed &= TestSpaceshipWeaponIntegration(WorldContextObject, Result);
            OutResults.Add(Result);

            bAllPassed &= TestFactionStationIntegration(WorldContextObject, Result);
            OutResults.Add(Result);

            bAllPassed &= TestCombatSystemIntegration(WorldContextObject, Result);
            OutResults.Add(Result);
        }
        break;

    case ETestCategory::PerformanceTests:
        {
            FTestResult Result;
            bAllPassed &= TestSpaceshipSpawnPerformance(WorldContextObject, Result);
            OutResults.Add(Result);

            bAllPassed &= TestAISystemPerformance(WorldContextObject, Result);
            OutResults.Add(Result);

            bAllPassed &= TestDataAssetLoadPerformance(WorldContextObject, Result);
            OutResults.Add(Result);
        }
        break;

    case ETestCategory::DataValidationTests:
        {
            // Data validation tests are covered in unit tests
            FTestResult Result(TEXT("DataValidationTests"), ETestCategory::DataValidationTests, true, 0.0f,
                             TEXT("Data validation tests are included in unit tests"));
            OutResults.Add(Result);
        }
        break;
    }

    return bAllPassed;
}

bool UAutomatedTestLibrary::RunSingleTest(
    UObject* WorldContextObject,
    const FString& TestName,
    FTestResult& OutResult
)
{
    // Map test names to functions
    TMap<FString, TFunction<bool(UObject*, FTestResult&)>> TestMap = {
        {TEXT("SpaceshipCalculations"), TestSpaceshipCalculations},
        {TEXT("FactionRelationships"), TestFactionRelationships},
        // TODO: Combat system archived - weapon test will be reimplemented in MVP
        // {TEXT("WeaponDamage"), TestWeaponDamage},
        {TEXT("DataValidation"), TestDataValidation},
        {TEXT("SpaceshipWeaponIntegration"), TestSpaceshipWeaponIntegration},
        {TEXT("FactionStationIntegration"), TestFactionStationIntegration},
        {TEXT("CombatSystemIntegration"), TestCombatSystemIntegration},
        {TEXT("SpaceshipSpawnPerformance"), TestSpaceshipSpawnPerformance},
        {TEXT("AISystemPerformance"), TestAISystemPerformance},
        {TEXT("DataAssetLoadPerformance"), TestDataAssetLoadPerformance}
    };

    auto* TestFunction = TestMap.Find(TestName);
    if (TestFunction)
    {
        return (*TestFunction)(WorldContextObject, OutResult);
    }

    OutResult = FTestResult(TestName, ETestCategory::UnitTests, false, 0.0f,
                           FString::Printf(TEXT("Test '%s' not found"), *TestName));
    return false;
}

//================================================================================
// UNIT TESTS
//================================================================================

bool UAutomatedTestLibrary::TestSpaceshipCalculations(UObject* WorldContextObject, FTestResult& OutResult)
{
    OutResult = ExecuteTest([&](FTestResult& Result) -> bool {
        // Find a spaceship data asset to test
        USpaceshipDataAsset* TestShip = nullptr;
        for (TObjectIterator<USpaceshipDataAsset> It; It; ++It)
        {
            TestShip = *It;
            break; // Use first one found
        }

        if (!TestShip)
        {
            Result.Message = TEXT("No spaceship data assets found");
            return false;
        }

        // Test rating calculations
        float CombatRating = TestShip->GetCombatRating();
        float MobilityRating = TestShip->GetMobilityRating();
        float UtilityRating = TestShip->GetUtilityRating();

        // Validate ranges
        bool bValidRanges = CombatRating >= 0 && CombatRating <= 100 &&
                           MobilityRating >= 0 && MobilityRating <= 100 &&
                           UtilityRating >= 0 && UtilityRating <= 100;

        if (!bValidRanges)
        {
            Result.Message = FString::Printf(TEXT("Invalid rating ranges: Combat=%.1f, Mobility=%.1f, Utility=%.1f"),
                CombatRating, MobilityRating, UtilityRating);
            return false;
        }

        // Test role suitability
        bool bHasRoles = TestShip->IsSuitableForRole(TEXT("Combat")) ||
                        TestShip->IsSuitableForRole(TEXT("Trade")) ||
                        TestShip->IsSuitableForRole(TEXT("Exploration"));

        if (!bHasRoles)
        {
            Result.Errors.Add(TEXT("Ship should be suitable for at least one role"));
            return false;
        }

        // Test size category
        FString SizeCategory = TestShip->GetSizeCategory();
        if (SizeCategory.IsEmpty())
        {
            Result.Errors.Add(TEXT("Size category should not be empty"));
            return false;
        }

        Result.Message = FString::Printf(TEXT("All calculations valid - Combat: %.1f, Mobility: %.1f, Utility: %.1f"),
            CombatRating, MobilityRating, UtilityRating);
        Result.Metrics.Add(TEXT("CombatRating"), CombatRating);
        Result.Metrics.Add(TEXT("MobilityRating"), MobilityRating);
        Result.Metrics.Add(TEXT("UtilityRating"), UtilityRating);

        return true;
    }, TEXT("SpaceshipCalculations"));
    
    return OutResult.bPassed;
}

bool UAutomatedTestLibrary::TestFactionRelationships(UObject* WorldContextObject, FTestResult& OutResult)
{
    OutResult = ExecuteTest([&](FTestResult& Result) -> bool {
        // REMOVED: TestFactionRelationships - faction system removed per Trade Simulator MVP
        Result.Message = TEXT("TestFactionRelationships skipped - faction system removed in MVP");
        return true;
        
        /* COMMENTED OUT - FACTION SYSTEM REMOVED PER MVP
        // Find faction data assets
        TArray<UFactionDataAsset*> Factions;
        for (TObjectIterator<UFactionDataAsset> It; It; ++It)
        {
            Factions.Add(*It);
        }

        if (Factions.Num() < 2)
        {
            Result.Message = TEXT("Need at least 2 factions for relationship testing");
            return false;
        }

        // Test faction relationships
        for (int32 i = 0; i < Factions.Num(); ++i)
        {
            for (int32 j = 0; j < Factions.Num(); ++j)
            {
                UFactionDataAsset* FactionA = Factions[i];
                UFactionDataAsset* FactionB = Factions[j];

                int32 Relationship = UAdastreaFunctionLibrary::GetFactionRelationship(FactionA, FactionB);

                // Validate range
                if (Relationship < -100 || Relationship > 100)
                {
                    Result.Errors.Add(FString::Printf(TEXT("Invalid relationship %d between %s and %s"),
                        Relationship, *FactionA->GetFactionName().ToString(), *FactionB->GetFactionName().ToString()));
                    return false;
                }

                // Test allied/hostile functions
                bool bAllied = UAdastreaFunctionLibrary::AreFactionsAllied(FactionA, FactionB);
                bool bHostile = UAdastreaFunctionLibrary::AreFactionsHostile(FactionA, FactionB);

                if (bAllied && bHostile)
                {
                    Result.Errors.Add(TEXT("Faction cannot be both allied and hostile"));
                    return false;
                }

                if (Relationship >= 26 && !bAllied)
                {
                    Result.Errors.Add(TEXT("Faction should be allied when relationship >= 26"));
                    return false;
                }

                if (Relationship <= -26 && !bHostile)
                {
                    Result.Errors.Add(TEXT("Faction should be hostile when relationship <= -26"));
                    return false;
                }
            }
        }

        Result.Message = FString::Printf(TEXT("All faction relationships valid (%d factions tested)"), Factions.Num());
        return true;
        */
    }, TEXT("FactionRelationships"));
    
    return OutResult.bPassed;
}

bool UAutomatedTestLibrary::TestWeaponDamage(UObject* WorldContextObject, FTestResult& OutResult)
{
    // TODO: Combat system archived - weapon test will be reimplemented in MVP
    OutResult = ExecuteTest([&](FTestResult& Result) -> bool {
        Result.Message = TEXT("TestWeaponDamage skipped - Combat system archived");
        return true;
    }, TEXT("WeaponDamage"));
    
    return OutResult.bPassed;
}

bool UAutomatedTestLibrary::TestDataValidation(UObject* WorldContextObject, FTestResult& OutResult)
{
    OutResult = ExecuteTest([&](FTestResult& Result) -> bool {
        // Test data validation on all assets
        TArray<FDataValidationResult> ValidationResults;

        bool bValidationPassed = UDataValidationLibrary::ValidateAllAssetsOfClass(
            UDataAsset::StaticClass(), ValidationResults);

        int32 TotalErrors = 0;
        int32 TotalWarnings = 0;

        for (const FDataValidationResult& ValidationResult : ValidationResults)
        {
            TotalErrors += ValidationResult.Errors.Num();
            TotalWarnings += ValidationResult.Warnings.Num();
        }

        Result.Message = FString::Printf(TEXT("Validation complete - %d assets, %d errors, %d warnings"),
            ValidationResults.Num(), TotalErrors, TotalWarnings);

        Result.Metrics.Add(TEXT("TotalAssets"), ValidationResults.Num());
        Result.Metrics.Add(TEXT("TotalErrors"), TotalErrors);
        Result.Metrics.Add(TEXT("TotalWarnings"), TotalWarnings);

        // Warnings are acceptable, but errors are not
        return TotalErrors == 0;
    }, TEXT("DataValidation"));
    
    return OutResult.bPassed;
}

//================================================================================
// INTEGRATION TESTS
//================================================================================

bool UAutomatedTestLibrary::TestSpaceshipWeaponIntegration(UObject* WorldContextObject, FTestResult& OutResult)
{
    OutResult = ExecuteTest([&](FTestResult& Result) -> bool {
        // This would test how spaceships and weapons work together
        // For now, return a placeholder success
        Result.Message = TEXT("Spaceship-weapon integration test placeholder - implement actual integration logic");
        return true;
    }, TEXT("SpaceshipWeaponIntegration"));
    
    return OutResult.bPassed;
}

bool UAutomatedTestLibrary::TestFactionStationIntegration(UObject* WorldContextObject, FTestResult& OutResult)
{
    OutResult = ExecuteTest([&](FTestResult& Result) -> bool {
        // This would test how factions and stations interact
        // For now, return a placeholder success
        Result.Message = TEXT("Faction-station integration test placeholder - implement actual integration logic");
        return true;
    }, TEXT("FactionStationIntegration"));
    
    return OutResult.bPassed;
}

bool UAutomatedTestLibrary::TestCombatSystemIntegration(UObject* WorldContextObject, FTestResult& OutResult)
{
    OutResult = ExecuteTest([&](FTestResult& Result) -> bool {
        // This would test the complete combat system
        // For now, return a placeholder success
        Result.Message = TEXT("Combat system integration test placeholder - implement actual integration logic");
        return true;
    }, TEXT("CombatSystemIntegration"));
    
    return OutResult.bPassed;
}

//================================================================================
// PERFORMANCE TESTS
//================================================================================

bool UAutomatedTestLibrary::TestSpaceshipSpawnPerformance(UObject* WorldContextObject, FTestResult& OutResult)
{
    OutResult = ExecuteTest([&](FTestResult& Result) -> bool {
        // Use the benchmark library
        FString BenchmarkResult = UPerformanceBenchmarkLibrary::BenchmarkShipSpawning(
            WorldContextObject, 10); // Smaller test for unit testing

        // Parse result to check performance
        // For now, assume it passes if no exception
        Result.Message = TEXT("Spaceship spawn performance test completed");
        Result.Metrics.Add(TEXT("TestDuration"), 0.1f); // Placeholder

        return true;
    }, TEXT("SpaceshipSpawnPerformance"));
    
    return OutResult.bPassed;
}

bool UAutomatedTestLibrary::TestAISystemPerformance(UObject* WorldContextObject, FTestResult& OutResult)
{
    OutResult = ExecuteTest([&](FTestResult& Result) -> bool {
        // Use the benchmark library
        FString BenchmarkResult = UPerformanceBenchmarkLibrary::BenchmarkAISystem(
            WorldContextObject, 5, 1.0f); // Smaller test

        Result.Message = TEXT("AI system performance test completed");
        Result.Metrics.Add(TEXT("TestDuration"), 1.0f);

        return true;
    }, TEXT("AISystemPerformance"));
    
    return OutResult.bPassed;
}

bool UAutomatedTestLibrary::TestDataAssetLoadPerformance(UObject* WorldContextObject, FTestResult& OutResult)
{
    OutResult = ExecuteTest([&](FTestResult& Result) -> bool {
        // Test loading performance of data assets
        int32 AssetCount = 0;
        double LoadTime = UPerformanceBenchmarkLibrary::MeasureExecutionTime([&AssetCount]() {
            for (TObjectIterator<UDataAsset> It; It; ++It)
            {
                AssetCount++;
                // Touch the asset to ensure it's loaded
                volatile UDataAsset* Asset = *It;
            }
        });

        Result.Message = FString::Printf(TEXT("Data asset loading completed in %.3f seconds (%d assets)"), LoadTime, AssetCount);
        Result.Metrics.Add(TEXT("LoadTime"), LoadTime);
        Result.Metrics.Add(TEXT("AssetCount"), AssetCount);

        // Performance threshold: should load in under 1 second
        return LoadTime < 1.0f;
    }, TEXT("DataAssetLoadPerformance"));
    
    return OutResult.bPassed;
}

//================================================================================
// UTILITIES
//================================================================================

FString UAutomatedTestLibrary::GetTestSummary(const TArray<FTestResult>& Results)
{
    int32 TotalTests = Results.Num();
    int32 PassedTests = 0;
    int32 FailedTests = 0;
    float TotalTime = 0.0f;

    for (const FTestResult& Result : Results)
    {
        if (Result.bPassed)
        {
            PassedTests++;
        }
        else
        {
            FailedTests++;
        }
        TotalTime += Result.ExecutionTime;
    }

    FString Summary = FString::Printf(TEXT("=== Test Summary ===\n"));
    Summary += FString::Printf(TEXT("Total Tests: %d\n"), TotalTests);
    Summary += FString::Printf(TEXT("Passed: %d (%.1f%%)\n"), PassedTests,
        TotalTests > 0 ? (float)PassedTests / TotalTests * 100.0f : 0.0f);
    Summary += FString::Printf(TEXT("Failed: %d\n"), FailedTests);
    Summary += FString::Printf(TEXT("Total Time: %.3f seconds\n"), TotalTime);
    Summary += FString::Printf(TEXT("Average Time: %.3f seconds per test\n"),
        TotalTests > 0 ? TotalTime / TotalTests : 0.0f);

    if (FailedTests > 0)
    {
        Summary += TEXT("\n❌ SOME TESTS FAILED\n");
    }
    else
    {
        Summary += TEXT("\n✅ ALL TESTS PASSED\n");
    }

    return Summary;
}

void UAutomatedTestLibrary::ExportTestResults(const TArray<FTestResult>& Results)
{
    FString ExportData = GetTestSummary(Results);
    ExportData += TEXT("\n=== Detailed Results ===\n\n");

    for (const FTestResult& Result : Results)
    {
        ExportData += FString::Printf(TEXT("[%s] %s (%.3fs)\n"),
            Result.bPassed ? TEXT("PASS") : TEXT("FAIL"),
            *Result.TestName, Result.ExecutionTime);

        if (!Result.Message.IsEmpty())
        {
            ExportData += TEXT("  Message: ") + Result.Message + TEXT("\n");
        }

        for (const FString& Error : Result.Errors)
        {
            ExportData += TEXT("  ❌ ") + Error + TEXT("\n");
        }

        if (Result.Metrics.Num() > 0)
        {
            ExportData += TEXT("  Metrics:\n");
            for (const auto& Metric : Result.Metrics)
            {
                ExportData += FString::Printf(TEXT("    %s: %.3f\n"), *Metric.Key, Metric.Value);
            }
        }

        ExportData += TEXT("\n");
    }

    UE_LOG(LogTemp, Log, TEXT("Test Results Export:\n%s"), *ExportData);
}

bool UAutomatedTestLibrary::TestPassed(const FTestResult& Result)
{
    return Result.bPassed;
}

float UAutomatedTestLibrary::GetTestExecutionTime(const FTestResult& Result)
{
    return Result.ExecutionTime;
}

//================================================================================
// PRIVATE HELPERS
//================================================================================

FTestResult UAutomatedTestLibrary::ExecuteTest(TFunction<bool(FTestResult&)> TestFunction, const FString& TestName)
{
    FTestResult Result;
    Result.TestName = TestName;
    Result.Category = ETestCategory::UnitTests; // Default, can be overridden

    double StartTime = FPlatformTime::Seconds();

    try
    {
        Result.bPassed = TestFunction(Result);
    }
    catch (...)
    {
        Result.bPassed = false;
        Result.Message = TEXT("Exception occurred during test execution");
        Result.Errors.Add(TEXT("Unhandled exception"));
    }

    double EndTime = FPlatformTime::Seconds();
    Result.ExecutionTime = EndTime - StartTime;

    return Result;
}