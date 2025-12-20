// Copyright Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AutomatedTestLibrary.h"
#include "AutomatedTestBlueprintLibrary.generated.h"

/**
 * Blueprint-accessible functions for running automated tests
 * Provides easy access to the testing framework from Blueprints
 */
UCLASS()
class ADASTREA_API UAutomatedTestBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Run all automated tests and return results
     * @param WorldContextObject - World context for the test execution
     * @param OutResults - Array to store test results
     * @return True if all tests passed
     */
    UFUNCTION(BlueprintCallable, Category = "Automated Testing")
    static bool RunAllTests(UObject* WorldContextObject, TArray<FTestResult>& OutResults);

    /**
     * Run tests for a specific category
     * @param WorldContextObject - World context for the test execution
     * @param TestCategory - Category of tests to run
     * @param OutResults - Array to store test results
     * @return True if all tests in category passed
     */
    UFUNCTION(BlueprintCallable, Category = "Automated Testing")
    static bool RunTestCategory(UObject* WorldContextObject, ETestCategory TestCategory, TArray<FTestResult>& OutResults);

    /**
     * Run a single test by name
     * @param WorldContextObject - World context for the test execution
     * @param TestName - Name of the test to run
     * @param OutResult - Result of the test execution
     * @return True if test passed
     */
    UFUNCTION(BlueprintCallable, Category = "Automated Testing")
    static bool RunSingleTest(UObject* WorldContextObject, const FString& TestName, FTestResult& OutResult);

    /**
     * Get a summary of test results as a formatted string
     * @param Results - Test results to summarize
     * @return Formatted summary string
     */
    UFUNCTION(BlueprintPure, Category = "Automated Testing")
    static FString GetTestSummary(const TArray<FTestResult>& Results);

    /**
     * Export test results to log
     * @param Results - Test results to export
     */
    UFUNCTION(BlueprintCallable, Category = "Automated Testing")
    static void ExportTestResults(const TArray<FTestResult>& Results);

    /**
     * Check if a test result indicates success
     * @param Result - Test result to check
     * @return True if test passed
     */
    UFUNCTION(BlueprintPure, Category = "Automated Testing")
    static bool TestPassed(const FTestResult& Result);

    /**
     * Get execution time of a test
     * @param Result - Test result to query
     * @return Execution time in seconds
     */
    UFUNCTION(BlueprintPure, Category = "Automated Testing")
    static float GetTestExecutionTime(const FTestResult& Result);
};