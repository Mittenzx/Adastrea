// LivingShipOrganismComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LivingShipOrganismComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ADASTREA_API ULivingShipOrganismComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    ULivingShipOrganismComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category="Organism")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category="Organism")
    float Mood; // -100 (distressed) ... 0 (neutral) ... +100 (happy)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category="Organism")
    float MutationLevel;

    UFUNCTION(BlueprintCallable, Category="Organism")
    void Heal(float Amount);

    UFUNCTION(BlueprintCallable, Category="Organism")
    void TakeDamage(float Amount);

    UFUNCTION(BlueprintCallable, Category="Organism")
    void Mutate(float Amount);

    UFUNCTION(BlueprintCallable, Category="Organism")
    FString GetOrganismDialogue() const;

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};