#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceStationModule.generated.h"

// Forward declaration
class UFactionDataAsset;

UENUM(BlueprintType)
enum class EStationModuleGroup : uint8
{
    Docking,
    Power,
    Storage,
    Processing,
    Defence,
    Habitation,
    Public,
    Connection,
    Other
};

UCLASS()
class ADASTREA_API ASpaceStationModule : public AActor
{
    GENERATED_BODY()

public:
    ASpaceStationModule();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module")
    FString ModuleType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module")
    float ModulePower;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module")
    EStationModuleGroup ModuleGroup;

    // Faction this module belongs to (can differ from station faction)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module")
    UFactionDataAsset* ModuleFaction;

    /**
     * Get the faction assigned to this module
     * @return The faction data asset, or nullptr if none assigned
     */
    UFUNCTION(BlueprintCallable, Category="Module")
    UFactionDataAsset* GetModuleFaction() const;

    /**
     * Set the faction for this module
     * @param NewFaction The faction to assign to this module
     */
    UFUNCTION(BlueprintCallable, Category="Module")
    void SetModuleFaction(UFactionDataAsset* NewFaction);
};