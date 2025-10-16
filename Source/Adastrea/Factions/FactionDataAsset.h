#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FactionDataAsset.generated.h"

/**
 * Data Asset for storing faction information.
 * This allows designers to create faction configurations as Blueprint Data Assets.
 */
UCLASS(BlueprintType)
class ADASTREA_API UFactionDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // Display name of the faction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Info")
    FText FactionName;

    // Brief description of the faction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Info", meta=(MultiLine=true))
    FText Description;

    // Faction emblem/logo color
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Info")
    FLinearColor PrimaryColor;

    // Secondary faction color
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Info")
    FLinearColor SecondaryColor;

    // Faction's reputation with the player (-100 to 100)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Relations", meta=(ClampMin="-100", ClampMax="100"))
    int32 InitialReputation;

    // Whether this faction is hostile by default
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Relations")
    bool bHostileByDefault;

    // Technology level of the faction (1-10)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Attributes", meta=(ClampMin="1", ClampMax="10"))
    int32 TechnologyLevel;

    // Military strength of the faction (1-10)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Attributes", meta=(ClampMin="1", ClampMax="10"))
    int32 MilitaryStrength;

    // Economic power of the faction (1-10)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Attributes", meta=(ClampMin="1", ClampMax="10"))
    int32 EconomicPower;

    // Unique identifier for the faction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Faction Info")
    FName FactionID;

    UFactionDataAsset();
};
