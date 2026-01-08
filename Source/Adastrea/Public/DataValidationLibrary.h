// Copyright Mittenzx. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DataValidationLibrary.generated.h"

/**
 * Data validation result structure
 */
USTRUCT(BlueprintType)
struct FDataValidationResult
{
    GENERATED_BODY()

    /** Asset that was validated */
    UPROPERTY(BlueprintReadOnly, Category="Validation Result")
    UDataAsset* Asset;

    /** Asset name */
    UPROPERTY(BlueprintReadOnly, Category="Validation Result")
    FString AssetName;

    /** Asset class */
    UPROPERTY(BlueprintReadOnly, Category="Validation Result")
    FString AssetClass;

    /** True if validation passed */
    UPROPERTY(BlueprintReadOnly, Category="Validation Result")
    bool bPassed;

    /** Validation errors */
    UPROPERTY(BlueprintReadOnly, Category="Validation Result")
    TArray<FText> Errors;

    /** Validation warnings */
    UPROPERTY(BlueprintReadOnly, Category="Validation Result")
    TArray<FText> Warnings;

    /** Validation timestamp */
    UPROPERTY(BlueprintReadOnly, Category="Validation Result")
    FDateTime Timestamp;

    FDataValidationResult()
        : Asset(nullptr)
        , bPassed(false)
        , Timestamp(FDateTime::Now())
    {}

    FDataValidationResult(UDataAsset* InAsset, bool InPassed,
                         const TArray<FText>& InErrors,
                         const TArray<FText>& InWarnings)
        : Asset(InAsset)
        , AssetName(InAsset ? InAsset->GetName() : TEXT("Unknown"))
        , AssetClass(InAsset ? InAsset->GetClass()->GetName() : TEXT("Unknown"))
        , bPassed(InPassed)
        , Errors(InErrors)
        , Warnings(InWarnings)
        , Timestamp(FDateTime::Now())
    {}
};

/**
 * Data Validation Library
 *
 * Comprehensive validation system for Data Assets and game data.
 * Provides runtime validation, editor integration, and detailed error reporting.
 *
 * Features:
 * - Data Asset validation with detailed error messages
 * - Batch validation for multiple assets
 * - Validation result caching
 * - Blueprint integration
 * - Editor warnings and errors
 *
 * Usage:
 * - Call validation functions in editor or at runtime
 * - Use for data integrity checks
 * - Integrate with CI/CD pipelines
 */
UCLASS()
class ADASTREA_API UDataValidationLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    //================================================================================
    // DATA ASSET VALIDATION
    //================================================================================

    /**
     * Validate a single Data Asset
     * @param DataAsset Asset to validate
     * @param OutErrors Array to fill with validation errors
     * @param OutWarnings Array to fill with validation warnings
     * @return True if validation passed (no errors)
     */
    UFUNCTION(BlueprintCallable, Category="Data Validation|Assets",
        meta=(DisplayName="Validate Data Asset"))
    static bool ValidateDataAsset(
        UDataAsset* DataAsset,
        TArray<FText>& OutErrors,
        TArray<FText>& OutWarnings
    );

    /**
     * Validate multiple Data Assets
     * @param DataAssets Array of assets to validate
     * @param OutResults Validation results for each asset
     * @return True if all assets passed validation
     */
    UFUNCTION(BlueprintCallable, Category="Data Validation|Assets",
        meta=(DisplayName="Validate Data Assets (Batch)"))
    static bool ValidateDataAssets(
        const TArray<UDataAsset*>& DataAssets,
        TArray<FDataValidationResult>& OutResults
    );

    /**
     * Validate all Data Assets of a specific class
     * @param AssetClass Class of assets to validate
     * @param OutResults Validation results
     * @return True if all assets passed validation
     */
    UFUNCTION(BlueprintCallable, Category="Data Validation|Assets",
        meta=(DisplayName="Validate All Assets of Class"))
    static bool ValidateAllAssetsOfClass(
        TSubclassOf<UDataAsset> AssetClass,
        TArray<FDataValidationResult>& OutResults
    );

    //================================================================================
    // SPECIFIC ASSET TYPE VALIDATION
    //================================================================================

    /**
     * Validate spaceship data asset
     * @param SpaceshipAsset Spaceship asset to validate
     * @param OutErrors Validation errors
     * @param OutWarnings Validation warnings
     * @return True if valid
     */
    UFUNCTION(BlueprintCallable, Category="Data Validation|Spaceships")
    static bool ValidateSpaceshipData(
        USpaceshipDataAsset* SpaceshipAsset,
        TArray<FText>& OutErrors,
        TArray<FText>& OutWarnings
    );

    // REMOVED: ValidateFactionData() - faction system removed per Trade Simulator MVP

    /**
     * Validate weapon data asset
     * TODO: Combat system archived - will be reimplemented in MVP
     * @param WeaponAsset Weapon asset to validate
     * @param OutErrors Validation errors
     * @param OutWarnings Validation warnings
     * @return True if valid
     */
    /*
    UFUNCTION(BlueprintCallable, Category="Data Validation|Weapons")
    static bool ValidateWeaponData(
        UWeaponDataAsset* WeaponAsset,
        TArray<FText>& OutErrors,
        TArray<FText>& OutWarnings
    );
    */

    //================================================================================
    // VALIDATION UTILITIES
    //================================================================================

    /**
     * Get validation summary for multiple results
     * @param Results Validation results to summarize
     * @return Formatted summary string
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Data Validation|Utilities")
    static FString GetValidationSummary(const TArray<FDataValidationResult>& Results);

    /**
     * Export validation results to log
     * @param Results Results to export
     * @param bIncludeWarnings Include warnings in export
     */
    UFUNCTION(BlueprintCallable, Category="Data Validation|Utilities")
    static void ExportValidationResults(
        const TArray<FDataValidationResult>& Results,
        bool bIncludeWarnings = true
    );

    /**
     * Check if validation result has errors
     * @param Result Validation result to check
     * @return True if result has errors
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Data Validation|Utilities")
    static bool HasValidationErrors(const FDataValidationResult& Result);

    /**
     * Check if validation result has warnings
     * @param Result Validation result to check
     * @return True if result has warnings
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Data Validation|Utilities")
    static bool HasValidationWarnings(const FDataValidationResult& Result);

    //================================================================================
    // VALIDATION RULES
    //================================================================================

    /**
     * Validate numeric range
     * @param Value Value to check
     * @param Min Minimum allowed value
     * @param Max Maximum allowed value
     * @param FieldName Name of field being validated
     * @param OutErrors Errors array to append to
     * @return True if value is in range
     */
    UFUNCTION(BlueprintCallable, Category="Data Validation|Rules")
    static bool ValidateNumericRange(
        float Value,
        float Min,
        float Max,
        const FString& FieldName,
        TArray<FText>& OutErrors
    );

    /**
     * Validate string is not empty
     * @param Value String to check
     * @param FieldName Name of field being validated
     * @param OutErrors Errors array to append to
     * @return True if string is not empty
     */
    UFUNCTION(BlueprintCallable, Category="Data Validation|Rules")
    static bool ValidateNotEmpty(
        const FString& Value,
        const FString& FieldName,
        TArray<FText>& OutErrors
    );

    /**
     * Validate object reference is not null
     * @param Object Object to check
     * @param FieldName Name of field being validated
     * @param OutErrors Errors array to append to
     * @return True if object is valid
     */
    UFUNCTION(BlueprintCallable, Category="Data Validation|Rules")
    static bool ValidateObjectReference(
        UObject* Object,
        const FString& FieldName,
        TArray<FText>& OutErrors
    );

    /**
     * Validate array is not empty
     * @param Array Array to check
     * @param FieldName Name of field being validated
     * @param OutErrors Errors array to append to
     * @return True if array is not empty
     */
    UFUNCTION(BlueprintCallable, Category="Data Validation|Rules")
    static bool ValidateArrayNotEmpty(
        const TArray<UObject*>& Array,
        const FString& FieldName,
        TArray<FText>& OutErrors
    );

private:
    /** Internal validation helper for spaceship data */
    static void ValidateSpaceshipStats(
        USpaceshipDataAsset* SpaceshipAsset,
        TArray<FText>& OutErrors,
        TArray<FText>& OutWarnings
    );

    // REMOVED: ValidateFactionRelationships() - faction system removed per Trade Simulator MVP

    /** Internal validation helper for weapon data */
    /** TODO: Combat system archived - will be reimplemented in MVP */
    /*
    static void ValidateWeaponBalance(
        UWeaponDataAsset* WeaponAsset,
        TArray<FText>& OutErrors,
        TArray<FText>& OutWarnings
    );
    */
};