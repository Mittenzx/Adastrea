// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Characters/AdastreaCharacter.h"
#include "Player/WorldInteractable.h"
#include "AdastreaNPC.generated.h"

/** What an NPC does aboard — drives their lines now, schedules and gear later. */
UENUM(BlueprintType)
enum class ENPCRole : uint8
{
	Crew,
	Engineer,
	Pilot,
	Medic,
	Cook,
	Security,
};

/**
 * A person walking a ship deck or station: the shared Manny body
 * (AAdastreaCharacter) on ordinary CharacterMovement, driven by an
 * AAdastreaNPCController. The player can talk to them (E).
 *
 * Who they are (name, role) is plain data so the population code can make
 * a crew from a ship's numbers; appearance variety comes later.
 */
UCLASS()
class ADASTREA_API AAdastreaNPC : public AAdastreaCharacter, public IWorldInteractable
{
	GENERATED_BODY()

public:
	AAdastreaNPC();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	ENPCRole CrewRole = ENPCRole::Crew;

	/** Walking pace range (cm/s); each NPC picks one so a crew doesn't march in step. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	FVector2D WalkSpeedRange = FVector2D(120.0f, 165.0f);

	/** Something to say for the role (random line). */
	UFUNCTION(BlueprintPure, Category="NPC")
	FText GetGreeting() const;

	UFUNCTION(BlueprintPure, Category="NPC")
	static FText GetRoleName(ENPCRole InRole);

	// IWorldInteractable
	virtual FText GetInteractPrompt_Implementation() const override;
	virtual void Interact_Implementation(AAdastreaPlayerController* PC) override;
};
