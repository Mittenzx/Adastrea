// Copyright Mittenzx. All Rights Reserved.

#include "TestRunnerActor.h"
#include "AutomatedTestBlueprintLibrary.h"

ATestRunnerActor::ATestRunnerActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ATestRunnerActor::BeginPlay()
{
    Super::BeginPlay();

    // Optionally run tests on begin play in editor
#if WITH_EDITOR
    if (GetWorld()->IsEditorWorld())
    {
        UE_LOG(LogTemp, Log, TEXT("TestRunnerActor: Running tests on BeginPlay"));
        RunAllTests();
    }
#endif
}

void ATestRunnerActor::RunAllTests()
{
    LastTestResults.Empty();
    bool bAllPassed = UAutomatedTestBlueprintLibrary::RunAllTests(this, LastTestResults);

    LogTestResults(LastTestResults);

    if (bAllPassed)
    {
        UE_LOG(LogTemp, Log, TEXT("✅ All tests passed!"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("❌ Some tests failed!"));
    }
}

void ATestRunnerActor::RunTestCategory(ETestCategory TestCategory)
{
    LastTestResults.Empty();
    bool bAllPassed = UAutomatedTestBlueprintLibrary::RunTestCategory(this, TestCategory, LastTestResults);

    LogTestResults(LastTestResults);

    FString CategoryName;
    switch (TestCategory)
    {
    case ETestCategory::UnitTests:
        CategoryName = TEXT("Unit Tests");
        break;
    case ETestCategory::IntegrationTests:
        CategoryName = TEXT("Integration Tests");
        break;
    case ETestCategory::PerformanceTests:
        CategoryName = TEXT("Performance Tests");
        break;
    case ETestCategory::DataValidationTests:
        CategoryName = TEXT("Data Validation Tests");
        break;
    }

    if (bAllPassed)
    {
        UE_LOG(LogTemp, Log, TEXT("✅ All %s passed!"), *CategoryName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("❌ Some %s failed!"), *CategoryName);
    }
}

void ATestRunnerActor::RunSingleTest(const FString& TestName)
{
    LastTestResults.Empty();
    FTestResult Result;
    bool bPassed = UAutomatedTestBlueprintLibrary::RunSingleTest(this, TestName, Result);

    LastTestResults.Add(Result);

    UE_LOG(LogTemp, Log, TEXT("Test Result: [%s] %s (%.3fs)"),
        bPassed ? TEXT("PASS") : TEXT("FAIL"), *TestName, Result.ExecutionTime);

    if (!Result.Message.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("  Message: %s"), *Result.Message);
    }

    for (const FString& Error : Result.Errors)
    {
        UE_LOG(LogTemp, Error, TEXT("  ❌ %s"), *Error);
    }

    if (Result.Metrics.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("  Metrics:"));
        for (const auto& Metric : Result.Metrics)
        {
            UE_LOG(LogTemp, Log, TEXT("    %s: %.3f"), *Metric.Key, Metric.Value);
        }
    }
}

FString ATestRunnerActor::GetLastTestSummary() const
{
    return UAutomatedTestBlueprintLibrary::GetTestSummary(LastTestResults);
}

void ATestRunnerActor::LogTestResults(const TArray<FTestResult>& Results)
{
    FString Summary = UAutomatedTestBlueprintLibrary::GetTestSummary(Results);
    UE_LOG(LogTemp, Log, TEXT("Test Summary:\n%s"), *Summary);
}