#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceStationModule.generated.h"

UENUM(BlueprintType)
enum class EStationModuleGroup : uint8
{
    Docking,
    Power,
    Storage,
    Processing,
    Defense,
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
};