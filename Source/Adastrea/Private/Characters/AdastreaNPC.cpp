// Copyright Epic Games, Inc. All Rights Reserved.

#include "Characters/AdastreaNPC.h"
#include "Characters/AdastreaNPCController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/AdastreaPlayerController.h"

#define LOCTEXT_NAMESPACE "AdastreaNPC"

AAdastreaNPC::AAdastreaNPC()
{
	AIControllerClass = AAdastreaNPCController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Face where they walk, turning at a person's pace (not snapping to a controller).
	bUseControllerRotationYaw = false;
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate = FRotator(0.0f, 300.0f, 0.0f);
		MoveComp->MaxWalkSpeed = 150.0f;
		// Ease in and out of a walk rather than starting and stopping dead.
		MoveComp->MaxAcceleration = 500.0f;
		MoveComp->BrakingDecelerationWalking = 500.0f;
		// Step around each other (and the player) in corridors.
		MoveComp->bUseRVOAvoidance = true;
		MoveComp->AvoidanceConsiderationRadius = 200.0f;
		MoveComp->AvoidanceWeight = 0.5f;
	}
}

void AAdastreaNPC::BeginPlay()
{
	Super::BeginPlay();
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = FMath::FRandRange(WalkSpeedRange.X, WalkSpeedRange.Y);
	}
}

FText AAdastreaNPC::GetRoleName(ENPCRole InRole)
{
	switch (InRole)
	{
	case ENPCRole::Engineer: return LOCTEXT("RoleEngineer", "Engineer");
	case ENPCRole::Pilot:    return LOCTEXT("RolePilot", "Pilot");
	case ENPCRole::Medic:    return LOCTEXT("RoleMedic", "Medic");
	case ENPCRole::Cook:     return LOCTEXT("RoleCook", "Cook");
	case ENPCRole::Security: return LOCTEXT("RoleSecurity", "Security");
	default:                 return LOCTEXT("RoleCrew", "Crew");
	}
}

FText AAdastreaNPC::GetGreeting() const
{
	// A few lines per role; picked fresh each time.
	static const TMap<ENPCRole, TArray<FText>> Lines = {
		{ ENPCRole::Crew,     { LOCTEXT("Crew1", "Another shift, another sector."), LOCTEXT("Crew2", "Need something, Captain?"), LOCTEXT("Crew3", "All quiet down here.") } },
		{ ENPCRole::Engineer, { LOCTEXT("Eng1", "Reactor's purring. Mostly."), LOCTEXT("Eng2", "Don't touch the blue conduits."), LOCTEXT("Eng3", "I'll have the drones recalibrated by end of shift.") } },
		{ ENPCRole::Pilot,    { LOCTEXT("Pilot1", "She handles better than the specs say."), LOCTEXT("Pilot2", "Plot me something interesting, Captain."), LOCTEXT("Pilot3", "Nav's green across the board.") } },
		{ ENPCRole::Medic,    { LOCTEXT("Med1", "Drink some water. That's an order."), LOCTEXT("Med2", "Sickbay's stocked if anyone needs it."), LOCTEXT("Med3", "No new injuries today. Let's keep it that way.") } },
		{ ENPCRole::Cook,     { LOCTEXT("Cook1", "Coffee's fresh. Probably."), LOCTEXT("Cook2", "Protein loaf again, sorry."), LOCTEXT("Cook3", "Galley's open if you're hungry.") } },
		{ ENPCRole::Security, { LOCTEXT("Sec1", "Corridors are clear, Captain."), LOCTEXT("Sec2", "Keeping an eye on the airlock."), LOCTEXT("Sec3", "Quiet shift. I like quiet.") } },
	};
	const TArray<FText>* Options = Lines.Find(CrewRole);
	return Options && Options->Num() > 0 ? (*Options)[FMath::RandRange(0, Options->Num() - 1)] : FText::GetEmpty();
}

FText AAdastreaNPC::GetInteractPrompt_Implementation() const
{
	return FText::Format(LOCTEXT("TalkPrompt", "Talk to {0} ({1})"), DisplayName, GetRoleName(CrewRole));
}

void AAdastreaNPC::Interact_Implementation(AAdastreaPlayerController* PC)
{
	if (!PC || IsRagdoll())
	{
		return;
	}
	if (AAdastreaNPCController* Brain = Cast<AAdastreaNPCController>(GetController()))
	{
		Brain->StartConversation(PC->GetPawn());
	}
	PC->ShowHUDMessage(FString::Printf(TEXT("%s (%s): \"%s\""),
		*DisplayName.ToString(), *GetRoleName(CrewRole).ToString(), *GetGreeting().ToString()), 5.0f, false);
}

#undef LOCTEXT_NAMESPACE
