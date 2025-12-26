// Copyright Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AutomatedTestLibrary.h"
#include "TestRunnerActor.generated.h"

/**
 * Actor that can run automated tests in the game world
 * Useful for testing systems during gameplay or in editor
 */
UCLASS()
class ADASTREA_API ATestRunnerActor : public AActor
{
    GENERATED_BODY()

public:
    ATestRunnerActor();

    virtual void BeginPlay() override;

    /**
     * Run all tests and log results
     */
    UFUNCTION(BlueprintCallable, Category = "Testing")
    void RunAllTests();

    /**
     * Run tests for a specific category
     */
    UFUNCTION(BlueprintCallable, Category = "Testing")
    void RunTestCategory(ETestCategory TestCategory);

    /**
     * Run a single test by name
     */
    UFUNCTION(BlueprintCallable, Category = "Testing")
    void RunSingleTest(const FString& TestName);

    /**
     * Get the last test results
     */
    UFUNCTION(BlueprintPure, Category = "Testing")
    const TArray<FTestResult>& GetLastTestResults() const { return LastTestResults; }

    /**
     * Get summary of last test run
     */
    UFUNCTION(BlueprintPure, Category = "Testing")
    FString GetLastTestSummary() const;

private:
    /** Last test results */
    UPROPERTY()
    TArray<FTestResult> LastTestResults;

    /** Log test results */
    void LogTestResults(const TArray<FTestResult>& Results);
};