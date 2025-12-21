// Copyright Mittenzx. All Rights Reserved.

#include "DataValidationLibrary.h"
#include "Ships/SpaceshipDataAsset.h"
#include "Factions/FactionDataAsset.h"
#include "Combat/WeaponDataAsset.h"
#include "Engine/AssetManager.h"
#include "UObject/UObjectIterator.h"

//================================================================================
// DATA ASSET VALIDATION
//================================================================================

bool UDataValidationLibrary::ValidateDataAsset(
    UDataAsset* DataAsset,
    TArray<FText>& OutErrors,
    TArray<FText>& OutWarnings
)
{
    OutErrors.Empty();
    OutWarnings.Empty();

    if (!DataAsset)
    {
        OutErrors.Add(FText::FromString("DataAsset is null"));
        return false;
    }

    bool bPassed = true;

    // Try specific validation based on asset type
    if (USpaceshipDataAsset* SpaceshipAsset = Cast<USpaceshipDataAsset>(DataAsset))
    {
        bPassed &= ValidateSpaceshipData(SpaceshipAsset, OutErrors, OutWarnings);
    }
    else if (UFactionDataAsset* FactionAsset = Cast<UFactionDataAsset>(DataAsset))
    {
        bPassed &= ValidateFactionData(FactionAsset, OutErrors, OutWarnings);
    }
    else if (UWeaponDataAsset* WeaponAsset = Cast<UWeaponDataAsset>(DataAsset))
    {
        bPassed &= ValidateWeaponData(WeaponAsset, OutErrors, OutWarnings);
    }
    else
    {
        // Generic validation for unknown asset types
        if (DataAsset->GetName().IsEmpty())
        {
            OutErrors.Add(FText::FromString("Asset name is empty"));
            bPassed = false;
        }

        // TODO: Update to UE5 data validation system
        // The UDataValidation interface has been replaced with FDataValidationContext in UE5
        // This section is commented out until proper UE5 validation is implemented
        /*
        // Check if asset implements IsDataValid
        if (DataAsset->Implements<UDataValidation>())
        {
            TArray<FText> ValidationErrors;
            EDataValidationResult Result = IDataValidation::Execute_IsDataValid(DataAsset, ValidationErrors);

            if (Result == EDataValidationResult::Invalid)
            {
                OutErrors.Append(ValidationErrors);
                bPassed = false;
            }
        }
        */
    }

    return bPassed && OutErrors.Num() == 0;
}

bool UDataValidationLibrary::ValidateDataAssets(
    const TArray<UDataAsset*>& DataAssets,
    TArray<FDataValidationResult>& OutResults
)
{
    OutResults.Empty();
    bool bAllPassed = true;

    for (UDataAsset* Asset : DataAssets)
    {
        TArray<FText> Errors;
        TArray<FText> Warnings;
        bool bPassed = ValidateDataAsset(Asset, Errors, Warnings);

        OutResults.Add(FDataValidationResult(Asset, bPassed, Errors, Warnings));
        bAllPassed &= bPassed;
    }

    return bAllPassed;
}

bool UDataValidationLibrary::ValidateAllAssetsOfClass(
    TSubclassOf<UDataAsset> AssetClass,
    TArray<FDataValidationResult>& OutResults
)
{
    OutResults.Empty();
    bool bAllPassed = true;

    // Find all assets of the specified class
    for (TObjectIterator<UDataAsset> It; It; ++It)
    {
        UDataAsset* Asset = *It;
        if (Asset && Asset->IsA(AssetClass))
        {
            TArray<FText> Errors;
            TArray<FText> Warnings;
            bool bPassed = ValidateDataAsset(Asset, Errors, Warnings);

            OutResults.Add(FDataValidationResult(Asset, bPassed, Errors, Warnings));
            bAllPassed &= bPassed;
        }
    }

    return bAllPassed;
}

//================================================================================
// SPECIFIC ASSET TYPE VALIDATION
//================================================================================

bool UDataValidationLibrary::ValidateSpaceshipData(
    USpaceshipDataAsset* SpaceshipAsset,
    TArray<FText>& OutErrors,
    TArray<FText>& OutWarnings
)
{
    if (!SpaceshipAsset)
    {
        OutErrors.Add(FText::FromString("SpaceshipAsset is null"));
        return false;
    }

    bool bPassed = true;

    // Basic field validation
    if (!ValidateNotEmpty(SpaceshipAsset->ShipName.ToString(), "ShipName", OutErrors))
        bPassed = false;

    if (!ValidateNotEmpty(SpaceshipAsset->ShipClass.ToString(), "ShipClass", OutErrors))
        bPassed = false;

    // Numeric range validation
    if (!ValidateNumericRange(SpaceshipAsset->MaxSpeed, 0.0f, 10000.0f, "MaxSpeed", OutErrors))
        bPassed = false;

    if (!ValidateNumericRange(SpaceshipAsset->HullStrength, 1.0f, 100000.0f, "HullStrength", OutErrors))
        bPassed = false;

    if (!ValidateNumericRange(SpaceshipAsset->ArmorRating, 0.0f, 1000.0f, "ArmorRating", OutErrors))
        bPassed = false;

    if (!ValidateNumericRange(SpaceshipAsset->ShieldStrength, 0.0f, 10000.0f, "ShieldStrength", OutErrors))
        bPassed = false;

    // Crew validation
    if (!ValidateNumericRange(SpaceshipAsset->CrewRequired, 1, 1000, "CrewRequired", OutErrors))
        bPassed = false;

    if (!ValidateNumericRange(SpaceshipAsset->MaxCrew, SpaceshipAsset->CrewRequired, 1000, "MaxCrew", OutErrors))
        bPassed = false;

    // Note: ShipMesh and Hardpoints properties not present in current SpaceshipDataAsset
    // These validations are commented out until the properties are added
    // TODO: Add visual mesh and hardpoint configuration to SpaceshipDataAsset
    /*
    // Object reference validation
    if (!ValidateObjectReference(SpaceshipAsset->ShipMesh, "ShipMesh", OutErrors))
        bPassed = false;

    // Array validation
    if (!ValidateArrayNotEmpty(SpaceshipAsset->Hardpoints, "Hardpoints", OutWarnings))
    {
        // Warning only - ships can have no hardpoints
    }
    */

    // Detailed stat validation
    ValidateSpaceshipStats(SpaceshipAsset, OutErrors, OutWarnings);

    return bPassed;
}

bool UDataValidationLibrary::ValidateFactionData(
    UFactionDataAsset* FactionAsset,
    TArray<FText>& OutErrors,
    TArray<FText>& OutWarnings
)
{
    if (!FactionAsset)
    {
        OutErrors.Add(FText::FromString("FactionAsset is null"));
        return false;
    }

    bool bPassed = true;

    // Basic field validation
    if (!ValidateNotEmpty(FactionAsset->FactionName.ToString(), "FactionName", OutErrors))
        bPassed = false;

    if (!ValidateNotEmpty(FactionAsset->Description.ToString(), "Description", OutErrors))
        bPassed = false;

    if (FactionAsset->FactionID.IsNone())
    {
        OutErrors.Add(FText::FromString("FactionID is not set"));
        bPassed = false;
    }

    // Reputation range validation
    if (!ValidateNumericRange(FactionAsset->InitialReputation, -100.0f, 100.0f, "InitialReputation", OutErrors))
        bPassed = false;

    // Technology level validation
    if (!ValidateNumericRange(FactionAsset->TechnologyLevel, 1, 10, "TechnologyLevel", OutErrors))
        bPassed = false;

    // Relationship validation
    ValidateFactionRelationships(FactionAsset, OutErrors, OutWarnings);

    return bPassed;
}

bool UDataValidationLibrary::ValidateWeaponData(
    UWeaponDataAsset* WeaponAsset,
    TArray<FText>& OutErrors,
    TArray<FText>& OutWarnings
)
{
    if (!WeaponAsset)
    {
        OutErrors.Add(FText::FromString("WeaponAsset is null"));
        return false;
    }

    bool bPassed = true;

    // Basic field validation
    if (!ValidateNotEmpty(WeaponAsset->WeaponName.ToString(), "WeaponName", OutErrors))
        bPassed = false;

    if (!ValidateNotEmpty(WeaponAsset->Description.ToString(), "Description", OutErrors))
        bPassed = false;

    // Damage validation
    if (!ValidateNumericRange(WeaponAsset->BaseDamage, 0.0f, 10000.0f, "BaseDamage", OutErrors))
        bPassed = false;

    // Range validation
    if (!ValidateNumericRange(WeaponAsset->MaxRange, 100.0f, 100000.0f, "MaxRange", OutErrors))
        bPassed = false;

    // Fire rate validation
    if (!ValidateNumericRange(WeaponAsset->RateOfFire, 0.1f, 100.0f, "RateOfFire", OutErrors))
        bPassed = false;

    // Energy validation
    if (!ValidateNumericRange(WeaponAsset->PowerPerShot, 0.0f, 1000.0f, "PowerPerShot", OutErrors))
        bPassed = false;

    // Balance validation
    ValidateWeaponBalance(WeaponAsset, OutErrors, OutWarnings);

    return bPassed;
}

//================================================================================
// VALIDATION UTILITIES
//================================================================================

FString UDataValidationLibrary::GetValidationSummary(const TArray<FDataValidationResult>& Results)
{
    int32 TotalAssets = Results.Num();
    int32 PassedAssets = 0;
    int32 TotalErrors = 0;
    int32 TotalWarnings = 0;

    for (const FDataValidationResult& Result : Results)
    {
        if (Result.bPassed)
        {
            PassedAssets++;
        }
        TotalErrors += Result.Errors.Num();
        TotalWarnings += Result.Warnings.Num();
    }

    FString Summary = FString::Printf(TEXT("=== Data Validation Summary ===\n"));
    Summary += FString::Printf(TEXT("Total Assets: %d\n"), TotalAssets);
    Summary += FString::Printf(TEXT("Passed: %d (%.1f%%)\n"), PassedAssets,
        TotalAssets > 0 ? (float)PassedAssets / TotalAssets * 100.0f : 0.0f);
    Summary += FString::Printf(TEXT("Failed: %d\n"), TotalAssets - PassedAssets);
    Summary += FString::Printf(TEXT("Total Errors: %d\n"), TotalErrors);
    Summary += FString::Printf(TEXT("Total Warnings: %d\n"), TotalWarnings);

    if (TotalErrors > 0)
    {
        Summary += TEXT("\n❌ VALIDATION FAILED - Fix errors before proceeding\n");
    }
    else if (TotalWarnings > 0)
    {
        Summary += TEXT("\n⚠️  VALIDATION PASSED with warnings - Consider addressing warnings\n");
    }
    else
    {
        Summary += TEXT("\n✅ VALIDATION PASSED - All assets are valid\n");
    }

    return Summary;
}

void UDataValidationLibrary::ExportValidationResults(
    const TArray<FDataValidationResult>& Results,
    bool bIncludeWarnings)
{
    FString ExportData = GetValidationSummary(Results);
    ExportData += TEXT("\n=== Detailed Results ===\n\n");

    for (const FDataValidationResult& Result : Results)
    {
        ExportData += FString::Printf(TEXT("[%s] %s\n"),
            Result.bPassed ? TEXT("PASS") : TEXT("FAIL"),
            *Result.AssetName);

        if (!Result.bPassed || (bIncludeWarnings && Result.Warnings.Num() > 0))
        {
            ExportData += TEXT("  Class: ") + Result.AssetClass + TEXT("\n");

            for (const FText& Error : Result.Errors)
            {
                ExportData += TEXT("  ❌ ") + Error.ToString() + TEXT("\n");
            }

            if (bIncludeWarnings)
            {
                for (const FText& Warning : Result.Warnings)
                {
                    ExportData += TEXT("  ⚠️  ") + Warning.ToString() + TEXT("\n");
                }
            }

            ExportData += TEXT("\n");
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Data Validation Export:\n%s"), *ExportData);
}

bool UDataValidationLibrary::HasValidationErrors(const FDataValidationResult& Result)
{
    return Result.Errors.Num() > 0;
}

bool UDataValidationLibrary::HasValidationWarnings(const FDataValidationResult& Result)
{
    return Result.Warnings.Num() > 0;
}

//================================================================================
// VALIDATION RULES
//================================================================================

bool UDataValidationLibrary::ValidateNumericRange(
    float Value,
    float Min,
    float Max,
    const FString& FieldName,
    TArray<FText>& OutErrors
)
{
    if (Value < Min || Value > Max)
    {
        OutErrors.Add(FText::FromString(FString::Printf(TEXT("%s (%.2f) is outside valid range [%.2f, %.2f]"),
            *FieldName, Value, Min, Max)));
        return false;
    }
    return true;
}

bool UDataValidationLibrary::ValidateNotEmpty(
    const FString& Value,
    const FString& FieldName,
    TArray<FText>& OutErrors
)
{
    if (Value.IsEmpty())
    {
        OutErrors.Add(FText::FromString(FString::Printf(TEXT("%s is empty"), *FieldName)));
        return false;
    }
    return true;
}

bool UDataValidationLibrary::ValidateObjectReference(
    UObject* Object,
    const FString& FieldName,
    TArray<FText>& OutErrors
)
{
    if (!Object)
    {
        OutErrors.Add(FText::FromString(FString::Printf(TEXT("%s is not set"), *FieldName)));
        return false;
    }
    return true;
}

bool UDataValidationLibrary::ValidateArrayNotEmpty(
    const TArray<UObject*>& Array,
    const FString& FieldName,
    TArray<FText>& OutErrors
)
{
    if (Array.Num() == 0)
    {
        OutErrors.Add(FText::FromString(FString::Printf(TEXT("%s array is empty"), *FieldName)));
        return false;
    }
    return true;
}

//================================================================================
// PRIVATE VALIDATION HELPERS
//================================================================================

void UDataValidationLibrary::ValidateSpaceshipStats(
    USpaceshipDataAsset* SpaceshipAsset,
    TArray<FText>& OutErrors,
    TArray<FText>& OutWarnings
)
{
    // Check for logical inconsistencies
    if (SpaceshipAsset->MaxSpeed > 0 && SpaceshipAsset->Acceleration <= 0)
    {
        OutWarnings.Add(FText::FromString("Ship has speed but zero acceleration"));
    }

    if (SpaceshipAsset->MaxHealth > 0 && SpaceshipAsset->ArmorRating <= 0 && SpaceshipAsset->ShieldStrength <= 0)
    {
        OutWarnings.Add(FText::FromString("Ship has health but no armor or shields"));
    }

    // Check balance ratios
    float CombatRatio = (SpaceshipAsset->ArmorRating + SpaceshipAsset->ShieldStrength) /
                       FMath::Max(1.0f, SpaceshipAsset->MaxHealth);

    if (CombatRatio < 0.1f)
    {
        OutWarnings.Add(FText::FromString("Ship may be under-armored for its health"));
    }
    else if (CombatRatio > 5.0f)
    {
        OutWarnings.Add(FText::FromString("Ship may be over-armored for its health"));
    }

    // Crew efficiency check
    if (SpaceshipAsset->MaxCrew > 0)
    {
        float CrewEfficiency = SpaceshipAsset->MaxCrew / FMath::Max(1, SpaceshipAsset->MinCrew);
        if (CrewEfficiency < 1.1f)
        {
            OutWarnings.Add(FText::FromString("Crew range is very narrow"));
        }
    }
}

void UDataValidationLibrary::ValidateFactionRelationships(
    UFactionDataAsset* FactionAsset,
    TArray<FText>& OutErrors,
    TArray<FText>& OutWarnings
)
{
    // Check for extreme reputation values
    if (FactionAsset->InitialReputation <= -80)
    {
        OutWarnings.Add(FText::FromString("Faction starts with very low reputation - may be unplayable"));
    }
    else if (FactionAsset->InitialReputation >= 80)
    {
        OutWarnings.Add(FText::FromString("Faction starts with very high reputation - may break game balance"));
    }

    // Technology level warnings
    if (FactionAsset->TechnologyLevel <= 2)
    {
        OutWarnings.Add(FText::FromString("Very low technology level - limited gameplay options"));
    }
    else if (FactionAsset->TechnologyLevel >= 9)
    {
        OutWarnings.Add(FText::FromString("Very high technology level - may overpower other factions"));
    }

    // Hostile faction warnings
    if (FactionAsset->bIsHostileByDefault && !FactionAsset->bIsMajorFaction)
    {
        OutWarnings.Add(FText::FromString("Minor faction marked as hostile by default - consider making it major"));
    }
}

void UDataValidationLibrary::ValidateWeaponBalance(
    UWeaponDataAsset* WeaponAsset,
    TArray<FText>& OutErrors,
    TArray<FText>& OutWarnings
)
{
    // DPS calculation
    float DPS = WeaponAsset->BaseDamage * WeaponAsset->RateOfFire;

    // Efficiency check (damage per energy)
    if (WeaponAsset->PowerPerShot > 0)
    {
        float Efficiency = DPS / WeaponAsset->PowerPerShot;

        if (Efficiency < 1.0f)
        {
            OutWarnings.Add(FText::FromString("Weapon has low damage efficiency - high energy cost for damage"));
        }
        else if (Efficiency > 10.0f)
        {
            OutWarnings.Add(FText::FromString("Weapon has very high efficiency - may be overpowered"));
        }
    }

    // Range vs fire rate balance
    float RangeEfficiency = WeaponAsset->MaxRange / FMath::Max(1.0f, WeaponAsset->RateOfFire * 100);

    if (RangeEfficiency < 10.0f)
    {
        OutWarnings.Add(FText::FromString("Weapon has short range for its fire rate - may be hard to use"));
    }

    // Damage type balance
    if (WeaponAsset->DamageType == EDamageType::Energy && WeaponAsset->PowerPerShot <= 0)
    {
        OutWarnings.Add(FText::FromString("Energy weapon has no energy consumption"));
    }
}