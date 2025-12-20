// Copyright Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AutomatedTestLibrary.generated.h"

/**
 * Test category enumeration
 */
UENUM(BlueprintType)
enum class ETestCategory : uint8
{
    UnitTests UMETA(DisplayName = "Unit Tests"),
    IntegrationTests UMETA(DisplayName = "Integration Tests"),
    PerformanceTests UMETA(DisplayName = "Performance Tests"),
    DataValidationTests UMETA(DisplayName = "Data Validation Tests"),
    All UMETA(DisplayName = "All Tests")
};

/**
 * Test result structure
 */
USTRUCT(BlueprintType)
struct FTestResult
{
    GENERATED_BODY()

    /** Test name */
    UPROPERTY(BlueprintReadOnly, Category="Test Result")
    FString TestName;

    /** Test category */
    UPROPERTY(BlueprintReadOnly, Category="Test Result")
    ETestCategory Category;

    /** True if test passed */
    UPROPERTY(BlueprintReadOnly, Category="Test Result")
    bool bPassed;

    /** Execution time in seconds */
    UPROPERTY(BlueprintReadOnly, Category="Test Result")
    float ExecutionTime;

    /** Test message (success/failure details) */
    UPROPERTY(BlueprintReadOnly, Category="Test Result")
    FString Message;

    /** Detailed error messages */
    UPROPERTY(BlueprintReadOnly, Category="Test Result")
    TArray<FString> Errors;

    /** Performance metrics */
    UPROPERTY(BlueprintReadOnly, Category="Test Result")
    TMap<FString, float> Metrics;

    /** Test timestamp */
    UPROPERTY(BlueprintReadOnly, Category="Test Result")
    FDateTime Timestamp;

    FTestResult()
        : bPassed(false)
        , ExecutionTime(0.0f)
        , Timestamp(FDateTime::Now())
    {}

    FTestResult(const FString& InTestName, ETestCategory InCategory, bool InPassed,
               float InExecutionTime, const FString& InMessage)
        : TestName(InTestName)
        , Category(InCategory)
        , bPassed(InPassed)
        , ExecutionTime(InExecutionTime)
        , Message(InMessage)
        , Timestamp(FDateTime::Now())
    {}
};

/**
 * Automated Testing Library
 *
 * Comprehensive testing framework for Adastrea game systems.
 * Provides automated tests for performance, functionality, and data integrity.
 *
 * Features:
 * - Unit tests for core systems
 * - Integration tests for system interactions
 * - Performance regression tests
 * - Data validation tests
 * - Automated test execution
 *
 * Usage:
 * - Call test functions in editor or runtime
 * - Use for CI/CD integration
 * - Run comprehensive test suites
 */
UCLASS()
class ADASTREA_API UAutomatedTestLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    //================================================================================
    // TEST EXECUTION
    //================================================================================

    /**
     * Run all automated tests
     * @param WorldContextObject World context
     * @param OutResults Test results for all tests
     * @return True if all tests passed
     */
    UFUNCTION(BlueprintCallable, Category="Automated Testing|Execution")
    static bool RunAllTests(UObject* WorldContextObject, TArray<FTestResult>& OutResults);

    /**
     * Run specific test category
     * @param WorldContextObject World context
     * @param TestCategory Category to run
     * @param OutResults Test results
     * @return True if all tests in category passed
     */
    UFUNCTION(BlueprintCallable, Category="Automated Testing|Execution")
    static bool RunTestCategory(
        UObject* WorldContextObject,
        ETestCategory TestCategory,
        TArray<FTestResult>& OutResults
    );

    /**
     * Run single test
     * @param WorldContextObject World context
     * @param TestName Name of test to run
     * @param OutResult Test result
     * @return True if test passed
     */
    UFUNCTION(BlueprintCallable, Category="Automated Testing|Execution")
    static bool RunSingleTest(
        UObject* WorldContextObject,
        const FString& TestName,
        FTestResult& OutResult
    );

    //================================================================================
    // UNIT TESTS
    //================================================================================

    /**
     * Test spaceship data asset calculations
     * @param WorldContextObject World context
     * @param OutResult Test result
     * @return True if test passed
     */
    UFUNCTION(BlueprintCallable, Category="Automated Testing|Unit Tests")
    static bool TestSpaceshipCalculations(UObject* WorldContextObject, FTestResult& OutResult);

    /**
     * Test faction relationship calculations
     * @param WorldContextObject World context
     * @param OutResult Test result
     * @return True if test passed
     */
    UFUNCTION(BlueprintCallable, Category="Automated Testing|Unit Tests")
    static bool TestFactionRelationships(UObject* WorldContextObject, FTestResult& OutResult);

    /**
     * Test weapon damage calculations
     * @param WorldContextObject World context
     * @param OutResult Test result
     * @return True if test passed
     */
    UFUNCTION(BlueprintCallable, Category="Automated Testing|Unit Tests")
    static bool TestWeaponDamage(UObject* WorldContextObject, FTestResult& OutResult);

    /**
     * Test data validation functions
     * @param WorldContextObject World context
     * @param OutResult Test result
     * @return True if test passed
     */
    UFUNCTION(BlueprintCallable, Category="Automated Testing|Unit Tests")
    static bool TestDataValidation(UObject* WorldContextObject, FTestResult& OutResult);

    //================================================================================
    // INTEGRATION TESTS
    //================================================================================

    /**
     * Test spaceship and weapon integration
     * @param WorldContextObject World context
     * @param OutResult Test result
     * @return True if test passed
     */
    UFUNCTION(BlueprintCallable, Category="Automated Testing|Integration")
    static bool TestSpaceshipWeaponIntegration(UObject* WorldContextObject, FTestResult& OutResult);

    /**
     * Test faction and station integration
     * @param WorldContextObject World context
     * @param OutResult Test result
     * @return True if test passed
     */
    UFUNCTION(BlueprintCallable, Category="Automated Testing|Integration")
    static bool TestFactionStationIntegration(UObject* WorldContextObject, FTestResult& OutResult);

    /**
     * Test combat system integration
     * @param WorldContextObject World context
     * @param OutResult Test result
     * @return True if test passed
     */
    UFUNCTION(BlueprintCallable, Category="Automated Testing|Integration")
    static bool TestCombatSystemIntegration(UObject* WorldContextObject, FTestResult& OutResult);

    //================================================================================
    // PERFORMANCE TESTS
    //================================================================================

    /**
     * Test performance regression for spaceship spawning
     * @param WorldContextObject World context
     * @param OutResult Test result
     * @return True if performance is acceptable
     */
    UFUNCTION(BlueprintCallable, Category="Automated Testing|Performance")
    static bool TestSpaceshipSpawnPerformance(UObject* WorldContextObject, FTestResult& OutResult);

    /**
     * Test AI system performance
     * @param WorldContextObject World context
     * @param OutResult Test result
     * @return True if performance is acceptable
     */
    UFUNCTION(BlueprintCallable, Category="Automated Testing|Performance")
    static bool TestAISystemPerformance(UObject* WorldContextObject, FTestResult& OutResult);

    /**
     * Test data asset loading performance
     * @param WorldContextObject World context
     * @param OutResult Test result
     * @return True if performance is acceptable
     */
    UFUNCTION(BlueprintCallable, Category="Automated Testing|Performance")
    static bool TestDataAssetLoadPerformance(UObject* WorldContextObject, FTestResult& OutResult);

    //================================================================================
    // UTILITIES
    //================================================================================

    /**
     * Get test summary
     * @param Results Test results to summarize
     * @return Formatted summary string
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Automated Testing|Utilities")
    static FString GetTestSummary(const TArray<FTestResult>& Results);

    /**
     * Export test results to log
     * @param Results Results to export
     */
    UFUNCTION(BlueprintCallable, Category="Automated Testing|Utilities")
    static void ExportTestResults(const TArray<FTestResult>& Results);

    /**
     * Check if test result passed
     * @param Result Test result to check
     * @return True if test passed
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Automated Testing|Utilities")
    static bool TestPassed(const FTestResult& Result);

    /**
     * Get test execution time
     * @param Result Test result
     * @return Execution time in seconds
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Automated Testing|Utilities")
    static float GetTestExecutionTime(const FTestResult& Result);

private:
    /** Internal test execution helper */
    static FTestResult ExecuteTest(TFunction<bool(FTestResult&)> TestFunction, const FString& TestName);

    /** Performance threshold checking */
    static bool CheckPerformanceThreshold(float ActualTime, float MaxAllowedTime, FString& OutMessage);

    /** Memory usage checking */
    static bool CheckMemoryUsage(int64 ActualUsage, int64 MaxAllowedUsage, FString& OutMessage);
};