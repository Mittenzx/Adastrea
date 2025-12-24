// Copyright (c) 2025 Mittenzx. All rights reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Data Asset Validation Helper
 * 
 * Purpose: Provide validation utilities for Data Assets to catch configuration errors
 * at edit-time rather than runtime.
 * 
 * Addresses:
 * - Anti-Pattern #2: Over-Engineering (Data Assets with 50+ properties, no validation)
 * - Technical Issue #4: Over-Reliance on Data Assets (no validation until runtime)
 * 
 * Usage in Data Asset classes:
 * 
 * #if WITH_EDITOR
 * virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
 * {
 *     Super::PostEditChangeProperty(PropertyChangedEvent);
 *     
 *     // Validate specific property
 *     if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UMyDataAsset, MyProperty))
 *     {
 *         FDataAssetValidation::ClampValue(MyProperty, 0.0f, 100.0f, TEXT("MyProperty"));
 *     }
 *     
 *     // Or validate all properties
 *     ValidateAllProperties();
 * }
 * 
 * void ValidateAllProperties()
 * {
 *     FDataAssetValidation::ClampValue(Health, 1.0f, 1000.0f, TEXT("Health"));
 *     FDataAssetValidation::ClampValue(Speed, 0.0f, 10000.0f, TEXT("Speed"));
 *     FDataAssetValidation::ValidateNotEmpty(DisplayName, TEXT("DisplayName"));
 * }
 * #endif
 * 
 * Benefits:
 * - Catches configuration errors in editor
 * - Provides clear error messages
 * - Reduces runtime bugs from invalid data
 * - Improves designer experience
 */
namespace FDataAssetValidation
{
	/**
	 * Clamp a numeric value to specified range and log if clamped
	 * 
	 * @param Value - Reference to value to clamp
	 * @param Min - Minimum allowed value
	 * @param Max - Maximum allowed value
	 * @param PropertyName - Name for logging purposes
	 * @return true if value was clamped, false if already in range
	 */
	template<typename T>
	static bool ClampValue(T& Value, T Min, T Max, const TCHAR* PropertyName)
	{
		if (Value < Min)
		{
			UE_LOG(LogTemp, Warning, TEXT("Data Asset Validation: %s was %.2f (below minimum %.2f), clamped to %.2f"), 
				PropertyName, (float)Value, (float)Min, (float)Min);
			Value = Min;
			return true;
		}
		else if (Value > Max)
		{
			UE_LOG(LogTemp, Warning, TEXT("Data Asset Validation: %s was %.2f (above maximum %.2f), clamped to %.2f"), 
				PropertyName, (float)Value, (float)Max, (float)Max);
			Value = Max;
			return true;
		}
		return false;
	}

	/**
	 * Validate that FText is not empty
	 * 
	 * @param Text - FText to validate
	 * @param PropertyName - Name for logging purposes
	 * @return true if valid (not empty), false if empty
	 */
	static bool ValidateNotEmpty(const FText& Text, const TCHAR* PropertyName)
	{
		if (Text.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("Data Asset Validation: %s is empty! Please provide a value."), PropertyName);
			return false;
		}
		return true;
	}

	/**
	 * Validate that FString is not empty
	 * 
	 * @param String - FString to validate
	 * @param PropertyName - Name for logging purposes
	 * @return true if valid (not empty), false if empty
	 */
	static bool ValidateNotEmpty(const FString& String, const TCHAR* PropertyName)
	{
		if (String.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("Data Asset Validation: %s is empty! Please provide a value."), PropertyName);
			return false;
		}
		return true;
	}

	/**
	 * Validate that array is not empty
	 * 
	 * @param Array - TArray to validate
	 * @param PropertyName - Name for logging purposes
	 * @return true if valid (has elements), false if empty
	 */
	template<typename T>
	static bool ValidateNotEmpty(const TArray<T>& Array, const TCHAR* PropertyName)
	{
		if (Array.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Data Asset Validation: %s array is empty! Consider adding elements."), PropertyName);
			return false;
		}
		return true;
	}

	/**
	 * Validate that pointer is not null
	 * 
	 * @param Pointer - Pointer to validate
	 * @param PropertyName - Name for logging purposes
	 * @return true if valid (not null), false if null
	 */
	template<typename T>
	static bool ValidateNotNull(const T* Pointer, const TCHAR* PropertyName)
	{
		if (Pointer == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Data Asset Validation: %s is null! Please assign a reference."), PropertyName);
			return false;
		}
		return true;
	}

	/**
	 * Validate that UPROPERTY object reference is valid
	 * 
	 * @param Object - UObject reference to validate
	 * @param PropertyName - Name for logging purposes
	 * @return true if valid (not null), false if null
	 */
	template<typename T>
	static bool ValidateObject(T* Object, const TCHAR* PropertyName)
	{
		if (!IsValid(Object))
		{
			UE_LOG(LogTemp, Error, TEXT("Data Asset Validation: %s is not valid! Please assign a valid reference."), PropertyName);
			return false;
		}
		return true;
	}

	/**
	 * Validate that a value is positive (> 0)
	 * 
	 * @param Value - Value to check
	 * @param PropertyName - Name for logging purposes
	 * @return true if positive, false otherwise
	 */
	template<typename T>
	static bool ValidatePositive(T Value, const TCHAR* PropertyName)
	{
		if (Value <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Data Asset Validation: %s is %.2f (not positive)! Consider using a positive value."), 
				PropertyName, (float)Value);
			return false;
		}
		return true;
	}

	/**
	 * Validate relationship between two values (e.g., Min < Max)
	 * 
	 * @param ValueA - First value
	 * @param ValueB - Second value
	 * @param PropertyNameA - Name of first property
	 * @param PropertyNameB - Name of second property
	 * @return true if ValueA < ValueB, false otherwise
	 */
	template<typename T>
	static bool ValidateLessThan(T ValueA, T ValueB, const TCHAR* PropertyNameA, const TCHAR* PropertyNameB)
	{
		if (ValueA >= ValueB)
		{
			UE_LOG(LogTemp, Error, TEXT("Data Asset Validation: %s (%.2f) should be less than %s (%.2f)!"), 
				PropertyNameA, (float)ValueA, PropertyNameB, (float)ValueB);
			return false;
		}
		return true;
	}

	/**
	 * Log all validation errors for a Data Asset
	 * Call this in PostEditChangeProperty to validate entire asset
	 * 
	 * @param AssetName - Name of the Data Asset being validated
	 * @param ErrorCount - Number of validation errors found
	 */
	static void LogValidationSummary(const TCHAR* AssetName, int32 ErrorCount)
	{
		if (ErrorCount > 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Data Asset Validation: %s has %d validation error(s)! Check the log for details."), 
				AssetName, ErrorCount);
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("Data Asset Validation: %s passed all validation checks."), AssetName);
		}
	}
}

/**
 * Example Usage in a Data Asset:
 * 
 * UCLASS()
 * class UMyDataAsset : public UDataAsset
 * {
 *     GENERATED_BODY()
 * 
 * public:
 *     UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
 *     float Health;
 * 
 *     UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
 *     float MaxHealth;
 * 
 *     UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Info")
 *     FText DisplayName;
 * 
 * #if WITH_EDITOR
 *     virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
 *     {
 *         Super::PostEditChangeProperty(PropertyChangedEvent);
 *         ValidateAllProperties();
 *     }
 * 
 * private:
 *     void ValidateAllProperties()
 *     {
 *         int32 ErrorCount = 0;
 *         
 *         // Clamp values to valid ranges
 *         if (FDataAssetValidation::ClampValue(Health, 0.0f, 10000.0f, TEXT("Health"))) ErrorCount++;
 *         if (FDataAssetValidation::ClampValue(MaxHealth, 1.0f, 10000.0f, TEXT("MaxHealth"))) ErrorCount++;
 *         
 *         // Validate relationships
 *         if (!FDataAssetValidation::ValidateLessThan(Health, MaxHealth, TEXT("Health"), TEXT("MaxHealth"))) ErrorCount++;
 *         
 *         // Validate required fields
 *         if (!FDataAssetValidation::ValidateNotEmpty(DisplayName, TEXT("DisplayName"))) ErrorCount++;
 *         
 *         FDataAssetValidation::LogValidationSummary(TEXT("MyDataAsset"), ErrorCount);
 *     }
 * #endif
 * };
 */
