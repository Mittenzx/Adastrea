#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerModData.generated.h"

/**
 * Player mod type enumeration
 */
UENUM(BlueprintType)
enum class EModType : uint8
{
    Passive UMETA(DisplayName = "Passive"),  // Always active when installed
    Active  UMETA(DisplayName = "Active")    // Requires activation and has cooldown
};

/**
 * Player mod definition structure
 * Defines the properties and effects of a player mod
 */
USTRUCT(BlueprintType)
struct FPlayerModDefinition
{
    GENERATED_BODY()

    /** Unique identifier for this mod */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mod")
    FName ModID;

    /** Display name of the mod */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mod")
    FText ModName;

    /** Description of what the mod does */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mod", meta=(MultiLine=true))
    FText ModDescription;

    /** Type of mod (Passive or Active) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mod")
    EModType Type;

    /** Cooldown duration for active mods (in seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mod", meta=(EditCondition="Type == EModType::Active", ClampMin="0.0"))
    float Cooldown;

    /** Stat modifiers applied by this mod */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mod|Stats")
    TMap<FName, float> StatModifiers;

    FPlayerModDefinition()
        : ModID(NAME_None)
        , ModName(FText::FromString(TEXT("Unknown Mod")))
        , ModDescription(FText::FromString(TEXT("No description available.")))
        , Type(EModType::Passive)
        , Cooldown(0.0f)
    {}
};

/**
 * Player mod data asset
 * Stores the definition of a player mod that can be installed and used
 */
UCLASS(BlueprintType)
class UPlayerModDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    /** The mod definition */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mod")
    FPlayerModDefinition ModDefinition;

    UPlayerModDataAsset()
    {
        // Default initialization
    }
};
