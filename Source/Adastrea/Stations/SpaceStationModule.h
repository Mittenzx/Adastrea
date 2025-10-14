#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceStationModule.generated.h"

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
};