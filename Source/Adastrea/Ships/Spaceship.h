#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Ships/SpaceshipInterior.h"
#include "Spaceship.generated.h"

UCLASS()
class ADASTREA_API ASpaceship : public APawn
{
    GENERATED_BODY()

public:
    ASpaceship();

    // Reference to the walkable interior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
    ASpaceshipInterior* InteriorInstance;

    // Call to enter the interior
    UFUNCTION(BlueprintCallable, Category="Spaceship")
    void EnterInterior(class APlayerController* PlayerController);

protected:
    virtual void BeginPlay() override;
};