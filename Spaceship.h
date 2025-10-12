#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Spaceship.generated.h"

class ASpaceshipWeaponModule;
class ASpaceshipEngineModule;

UCLASS()
class ADASTREA_API ASpaceship : public APawn
{
    GENERATED_BODY()

public:
    ASpaceship();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modules")
    TArray<ASpaceshipWeaponModule*> WeaponModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modules")
    TArray<ASpaceshipEngineModule*> EngineModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
    float Shield;
};