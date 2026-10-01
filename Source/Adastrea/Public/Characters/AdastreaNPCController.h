// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Controller.h"
#include "AdastreaNPCController.generated.h"

class AAdastreaNPC;
class FDeckNavGrid;

/** What the NPC is doing right now. */
UENUM(BlueprintType)
enum class ENPCActivity : uint8
{
	/** Standing about (or waiting for a walkable floor to exist). */
	Idle,
	/** Walking a path to a destination. */
	Walking,
	/** Standing at a fixture, facing it, as if working it. */
	UsingFixture,
	/** Stopped and turned to face whoever spoke to them. */
	Talking,
	/** Limp (ragdoll) or otherwise unable to act. */
	Disabled,
};

/**
 * Lightweight NPC brain — a plain AController (the project doesn't build
 * AIModule), ticking a small activity loop:
 *   Idle (a few seconds) -> Walk to a random spot or a fixture -> Idle / UseFixture -> ...
 * plus Talking when the player speaks to them and Disabled while ragdolled.
 *
 * Paths come from the interior's FDeckNavGrid (via UNPCPopulationSubsystem);
 * the pawn is steered along them with movement input, so CharacterMovement and
 * its RVO avoidance do the actual walking. A walker that stops making progress
 * re-plans, then gives up on that destination.
 */
UCLASS()
class ADASTREA_API AAdastreaNPCController : public AController
{
	GENERATED_BODY()

public:
	AAdastreaNPCController();

	virtual void Tick(float DeltaSeconds) override;
	virtual void OnPossess(APawn* InPawn) override;

	/** Stop and face Listener for a few seconds (after which the routine resumes). */
	UFUNCTION(BlueprintCallable, Category="NPC")
	void StartConversation(AActor* Listener, float Duration = 5.0f);

	UFUNCTION(BlueprintPure, Category="NPC")
	ENPCActivity GetActivity() const { return Activity; }

	/** Destination of the current walk (world), for debugging/tests. */
	UFUNCTION(BlueprintPure, Category="NPC")
	FVector GetDestination() const { return Destination; }

	/** Completed walks, and walks abandoned because the NPC got stuck (tests read these). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="NPC")
	int32 ArrivedCount = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="NPC")
	int32 GaveUpCount = 0;

	/** The interior this NPC lives in (its walkable grid is looked up from it). */
	UPROPERTY(BlueprintReadWrite, Category="NPC")
	TWeakObjectPtr<AActor> HomeInterior;

protected:
	void ChooseNextActivity();
	/** Plan a walk to WorldGoal; with bAvoidPeople, the path keeps clear of everyone nearby. */
	bool StartWalkTo(const FVector& WorldGoal, bool bAvoidPeople = false);
	/** Where the other people near this NPC are standing (capsule centres). */
	void GetPeopleNearby(TArray<FVector>& OutLocations, float Radius) const;
	void TickWalking(float DeltaSeconds);
	/** Turn the body toward a world point at a person's pace. */
	void FaceToward(const FVector& WorldPoint, float DeltaSeconds);
	const FDeckNavGrid* GetGrid() const;

	ENPCActivity Activity = ENPCActivity::Idle;
	float ActivityTimeLeft = 0.0f;

	TArray<FVector> Path;
	int32 PathIndex = 0;
	FVector Destination = FVector::ZeroVector;
	/** Where to look on arrival (a fixture), if the walk was to one. */
	TWeakObjectPtr<AActor> FixtureTarget;
	TWeakObjectPtr<AActor> TalkTarget;
	ENPCActivity ResumeActivity = ENPCActivity::Idle;

	// Stuck detection.
	FVector LastProgressLocation = FVector::ZeroVector;
	float StuckTime = 0.0f;
	int32 Replans = 0;

	FRandomStream Random;

	/** Chance a walk heads for a fixture rather than a random spot. */
	UPROPERTY(EditAnywhere, Category="NPC")
	float FixtureVisitChance = 0.4f;

	/** Idle and fixture-use durations (s). */
	UPROPERTY(EditAnywhere, Category="NPC")
	FVector2D IdleTime = FVector2D(2.0f, 6.0f);
	UPROPERTY(EditAnywhere, Category="NPC")
	FVector2D FixtureTime = FVector2D(5.0f, 12.0f);
};
