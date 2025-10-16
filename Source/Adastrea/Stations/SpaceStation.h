#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceStationModule.h"
#include "SpaceStation.generated.h"

// Forward declaration
class UFactionDataAsset;

UCLASS()
class ADASTREA_API ASpaceStation : public AActor
{
    GENERATED_BODY()

public:
    ASpaceStation();

    // Faction this station belongs to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station")
    UFactionDataAsset* Faction;

    // Array of attached modules
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station")
    TArray<ASpaceStationModule*> Modules;

    // Add a module to the station (simple version without position)
    UFUNCTION(BlueprintCallable, Category="Station")
    void AddModule(ASpaceStationModule* Module);

    // Add a module to the station at a specific location
    UFUNCTION(BlueprintCallable, Category="Station")
    bool AddModuleAtLocation(ASpaceStationModule* Module, FVector RelativeLocation);

    // Remove a module from the station
    UFUNCTION(BlueprintCallable, Category="Station")
    bool RemoveModule(ASpaceStationModule* Module);

    // Move a module to a new location within the station
    UFUNCTION(BlueprintCallable, Category="Station")
    bool MoveModule(ASpaceStationModule* Module, FVector NewRelativeLocation);

    // Get all attached modules
    UFUNCTION(BlueprintCallable, Category="Station")
    TArray<ASpaceStationModule*> GetModules() const;

    // Get all modules of a specific type
    UFUNCTION(BlueprintCallable, Category="Station")
    TArray<ASpaceStationModule*> GetModulesByType(const FString& ModuleType);

    // Get the number of attached modules
    UFUNCTION(BlueprintCallable, Category="Station")
    int32 GetModuleCount() const;

    // Get the faction this station belongs to
    UFUNCTION(BlueprintCallable, Category="Station")
    UFactionDataAsset* GetFaction() const;

    // Set the faction this station belongs to
    UFUNCTION(BlueprintCallable, Category="Station")
    void SetFaction(UFactionDataAsset* NewFaction);

protected:
    virtual void BeginPlay() override;
};
