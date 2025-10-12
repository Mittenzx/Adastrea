#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceshipWeaponModule.generated.h"

UCLASS()
class ADASTREA_API ASpaceshipWeaponModule : public AActor
{
    GENERATED_BODY()

public:
    ASpaceshipWeaponModule();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
    float Damage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
    float Cooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
    int32 Ammo;
};