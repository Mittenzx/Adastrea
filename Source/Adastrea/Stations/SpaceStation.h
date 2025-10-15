#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceStationModule.h"
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

    // Add a module to the station
    UFUNCTION(BlueprintCallable, Category="Station")
    void AddModule(ASpaceStationModule* Module);

    // Remove a module from the station
    UFUNCTION(BlueprintCallable, Category="Station")
    bool RemoveModule(ASpaceStationModule* Module);

    // Get all attached modules
    UFUNCTION(BlueprintCallable, Category="Station")
    TArray<ASpaceStationModule*> GetModules() const;

    // Get the number of attached modules
    UFUNCTION(BlueprintCallable, Category="Station")
    int32 GetModuleCount() const;

protected:
    virtual void BeginPlay() override;
};
