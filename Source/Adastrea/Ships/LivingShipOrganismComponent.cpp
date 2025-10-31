// Copyright Epic Games, Inc. All Rights Reserved.

#include "Ships/LivingShipOrganismComponent.h"
#include "AdastreaLog.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"

ULivingShipOrganismComponent::ULivingShipOrganismComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
    
    // Initialize default values
    Health = 100.0f;
    Mood = 0.0f;
    MutationLevel = 0.0f;
}

void ULivingShipOrganismComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(ULivingShipOrganismComponent, Health);
    DOREPLIFETIME(ULivingShipOrganismComponent, Mood);
    DOREPLIFETIME(ULivingShipOrganismComponent, MutationLevel);
}

void ULivingShipOrganismComponent::Heal(float Amount)
{
    if (Amount < 0.0f)
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("LivingShipOrganismComponent::Heal - Invalid negative amount: %f"), Amount);
        return;
    }
    
    Health = FMath::Clamp(Health + Amount, 0.0f, 100.0f);
    
    UE_LOG(LogAdastreaShips, Log, TEXT("LivingShipOrganismComponent::Heal - Healed by %f, new health: %f"), Amount, Health);
}

void ULivingShipOrganismComponent::TakeDamage(float Amount)
{
    if (Amount < 0.0f)
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("LivingShipOrganismComponent::TakeDamage - Invalid negative amount: %f"), Amount);
        return;
    }
    
    Health = FMath::Clamp(Health - Amount, 0.0f, 100.0f);
    
    UE_LOG(LogAdastreaShips, Log, TEXT("LivingShipOrganismComponent::TakeDamage - Took %f damage, new health: %f"), Amount, Health);
}

void ULivingShipOrganismComponent::Mutate(float Amount)
{
    MutationLevel = FMath::Clamp(MutationLevel + Amount, 0.0f, 100.0f);
    
    UE_LOG(LogAdastreaShips, Log, TEXT("LivingShipOrganismComponent::Mutate - Mutated by %f, new mutation level: %f"), Amount, MutationLevel);
}

FString ULivingShipOrganismComponent::GetOrganismDialogue() const
{
    // Return dialogue based on organism state
    if (Health < 30.0f)
    {
        return TEXT("*weakly pulsating* I... need... help...");
    }
    else if (Health < 60.0f)
    {
        return TEXT("*struggling* Not feeling well...");
    }
    else if (Mood < -50.0f)
    {
        return TEXT("*distressed resonance* Something is wrong!");
    }
    else if (Mood > 50.0f)
    {
        return TEXT("*content humming* Everything is wonderful!");
    }
    else if (MutationLevel > 70.0f)
    {
        return TEXT("*strange harmonics* I feel... different...");
    }
    else
    {
        return TEXT("*steady pulsing* All systems normal.");
    }
}
