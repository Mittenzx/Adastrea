// Copyright Mittenzx. All Rights Reserved.

#include "AutomatedTestBlueprintLibrary.h"

bool UAutomatedTestBlueprintLibrary::RunAllTests(UObject* WorldContextObject, TArray<FTestResult>& OutResults)
{
    return UAutomatedTestLibrary::RunAllTests(WorldContextObject, OutResults);
}

bool UAutomatedTestBlueprintLibrary::RunTestCategory(UObject* WorldContextObject, ETestCategory TestCategory, TArray<FTestResult>& OutResults)
{
    return UAutomatedTestLibrary::RunTestCategory(WorldContextObject, TestCategory, OutResults);
}

bool UAutomatedTestBlueprintLibrary::RunSingleTest(UObject* WorldContextObject, const FString& TestName, FTestResult& OutResult)
{
    return UAutomatedTestLibrary::RunSingleTest(WorldContextObject, TestName, OutResult);
}

FString UAutomatedTestBlueprintLibrary::GetTestSummary(const TArray<FTestResult>& Results)
{
    return UAutomatedTestLibrary::GetTestSummary(Results);
}

void UAutomatedTestBlueprintLibrary::ExportTestResults(const TArray<FTestResult>& Results)
{
    UAutomatedTestLibrary::ExportTestResults(Results);
}

bool UAutomatedTestBlueprintLibrary::TestPassed(const FTestResult& Result)
{
    return UAutomatedTestLibrary::TestPassed(Result);
}

float UAutomatedTestBlueprintLibrary::GetTestExecutionTime(const FTestResult& Result)
{
    return UAutomatedTestLibrary::GetTestExecutionTime(Result);
}