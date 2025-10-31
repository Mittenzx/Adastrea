#pragma once

#include "CoreMinimal.h"
#include "Ships/SpaceshipDataAsset.h"
#include "MyFirstSpaceshipDataAsset.generated.h"

/**
 * Example spaceship data asset with default values used by MyFirstSpaceshipDataAsset.cpp
 */
UCLASS(BlueprintType)
class ADASTREA_API UMyFirstSpaceshipDataAsset : public USpaceshipDataAsset
{
    GENERATED_BODY()

public:
    UMyFirstSpaceshipDataAsset();

    // Basic info
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Basic")
    FText ShipName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Basic")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Basic")
    FText ShipClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Basic")
    FName ShipID;

    // Core stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Core")
    float HullStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Core")
    float CargoCapacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Core")
    int32 CrewRequired;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Core")
    int32 MaxCrew;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Core")
    int32 ModularPoints;

    // Combat stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Combat")
    float ArmorRating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Combat")
    float ShieldStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Combat")
    float ShieldRechargeRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Combat")
    int32 WeaponSlots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Combat")
    float WeaponPowerCapacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Combat")
    float PointDefenseRating;

    // Mobility stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Mobility")
    float MaxSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Mobility")
    float Acceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Mobility")
    int32 Maneuverability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Mobility")
    float JumpRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Mobility")
    float FuelCapacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Mobility")
    float FuelConsumptionRate;

    // Utility stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Utility")
    float SensorRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Utility")
    int32 SensorResolution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Utility")
    int32 StealthRating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Utility")
    int32 RepairSystemRating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Utility")
    int32 ScienceRating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Utility")
    int32 MedicalRating;

    // Operational stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Operational")
    float PowerCapacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Operational")
    int32 LifeSupportRating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Operational")
    int32 MaintenanceLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Operational")
    int32 HangarCapacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Operational")
    int32 DroneCapacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Operational")
    int32 AISystemRating;

    // Advanced stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Advanced")
    int32 DiplomacyRating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Advanced")
    int32 BoardingCapability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Advanced")
    int32 CustomizableSlots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Advanced")
    int32 EWarRating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Advanced")
    int32 MiningRating;

    // Lore
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Lore")
    FText Manufacturer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Lore")
    int32 YearBuilt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Lore")
    FText RarityTier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Lore", meta=(MultiLine=true))
    FText LoreNotes;

    // Colors
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Visual")
    FLinearColor PrimaryColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Visual")
    FLinearColor SecondaryColor;
};
