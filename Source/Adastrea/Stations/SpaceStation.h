#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Stations/SpaceStationModule.h"
#include "SpaceStation.generated.h"

UCLASS()
class ADASTREA_API ASpaceStation : public AActor
{
    GENERATED_BODY()

public:
    ASpaceStation();

    // Array of attached modules
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station")
    TArray<ASpaceStationModule*> Modules;

    // Add a module to the station at a specific location
    UFUNCTION(BlueprintCallable, Category="Station")
    bool AddModule(ASpaceStationModule* Module, FVector RelativeLocation);

    // Remove a module from the station
    UFUNCTION(BlueprintCallable, Category="Station")
    bool RemoveModule(ASpaceStationModule* Module);

    // Move a module to a new location within the station
    UFUNCTION(BlueprintCallable, Category="Station")
    bool MoveModule(ASpaceStationModule* Module, FVector NewRelativeLocation);

    // Get all modules of a specific type
    UFUNCTION(BlueprintCallable, Category="Station")
    TArray<ASpaceStationModule*> GetModulesByType(const FString& ModuleType);

protected:
    virtual void BeginPlay() override;
};
