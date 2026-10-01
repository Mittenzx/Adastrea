// Copyright Epic Games, Inc. All Rights Reserved.

#include "Characters/AdastreaNPCController.h"
#include "Characters/AdastreaNPC.h"
#include "Characters/DeckNavGrid.h"
#include "Characters/NPCPopulationSubsystem.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

AAdastreaNPCController::AAdastreaNPCController()
{
	PrimaryActorTick.bCanEverTick = true;
	Random.GenerateNewSeed();
}

void AAdastreaNPCController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	// Stagger the first decision so a freshly spawned crew doesn't all set off at once.
	Activity = ENPCActivity::Idle;
	ActivityTimeLeft = Random.FRandRange(0.5f, 2.5f);
}

const FDeckNavGrid* AAdastreaNPCController::GetGrid() const
{
	const UNPCPopulationSubsystem* Population = GetWorld() ? GetWorld()->GetSubsystem<UNPCPopulationSubsystem>() : nullptr;
	return Population ? Population->GetGrid(HomeInterior.Get()) : nullptr;
}

void AAdastreaNPCController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const AAdastreaNPC* NPC = Cast<AAdastreaNPC>(GetPawn());
	if (!NPC)
	{
		return;
	}
	if (NPC->IsRagdoll())
	{
		Activity = ENPCActivity::Disabled;
		Path.Reset();
		return;
	}
	if (Activity == ENPCActivity::Disabled)
	{
		// Back on their feet: collect themselves, then carry on.
		Activity = ENPCActivity::Idle;
		ActivityTimeLeft = Random.FRandRange(1.0f, 2.0f);
	}

	switch (Activity)
	{
	case ENPCActivity::Walking:
		TickWalking(DeltaSeconds);
		break;

	case ENPCActivity::Talking:
		if (TalkTarget.IsValid())
		{
			FaceToward(TalkTarget->GetActorLocation(), DeltaSeconds);
		}
		ActivityTimeLeft -= DeltaSeconds;
		if (ActivityTimeLeft <= 0.0f)
		{
			TalkTarget = nullptr;
			if (ResumeActivity == ENPCActivity::Walking && Path.IsValidIndex(PathIndex))
			{
				Activity = ENPCActivity::Walking;
				LastProgressLocation = GetPawn()->GetActorLocation();
				StuckTime = 0.0f;
			}
			else
			{
				Activity = ResumeActivity == ENPCActivity::UsingFixture ? ENPCActivity::UsingFixture : ENPCActivity::Idle;
				ActivityTimeLeft = Random.FRandRange(1.0f, 3.0f);
			}
		}
		break;

	case ENPCActivity::UsingFixture:
		if (FixtureTarget.IsValid())
		{
			FaceToward(FixtureTarget->GetActorLocation(), DeltaSeconds);
		}
		[[fallthrough]];
	case ENPCActivity::Idle:
	default:
		ActivityTimeLeft -= DeltaSeconds;
		if (ActivityTimeLeft <= 0.0f)
		{
			ChooseNextActivity();
		}
		break;
	}
}

void AAdastreaNPCController::ChooseNextActivity()
{
	const FDeckNavGrid* Grid = GetGrid();
	APawn* Body = GetPawn();
	if (!Grid || !Grid->IsValid() || !Body)
	{
		Activity = ENPCActivity::Idle;
		ActivityTimeLeft = 2.0f;
		return;
	}
	const float HalfHeight = Body->GetRootComponent()->Bounds.BoxExtent.Z;
	const FVector Feet = Body->GetActorLocation() - FVector(0.0f, 0.0f, HalfHeight);

	// Sometimes go and work at a fixture (stand on the nearest walkable spot to it).
	FixtureTarget = nullptr;
	if (Random.FRand() < FixtureVisitChance)
	{
		const UNPCPopulationSubsystem* Population = GetWorld()->GetSubsystem<UNPCPopulationSubsystem>();
		const TArray<TWeakObjectPtr<AActor>>& Fixtures = Population->GetFixtures(HomeInterior.Get());
		if (Fixtures.Num() > 0)
		{
			AActor* Fixture = Fixtures[Random.RandRange(0, Fixtures.Num() - 1)].Get();
			const int32 Spot = Fixture ? Grid->FindNearestNode(Fixture->GetActorLocation(), 220.0f) : INDEX_NONE;
			if (Spot != INDEX_NONE && StartWalkTo(Grid->GetNodeLocation(Spot)))
			{
				FixtureTarget = Fixture;
				Activity = ENPCActivity::Walking;
				return;
			}
		}
	}

	// Otherwise wander somewhere else on the same walkable floor (somewhere nobody
	// is standing in the way of).
	const int32 From = Grid->FindNearestNode(Feet);
	for (int32 Try = 0; Try < 4; ++Try)
	{
		const int32 To = Grid->GetRandomReachableNode(From, Random, 300.0f);
		if (To != INDEX_NONE && StartWalkTo(Grid->GetNodeLocation(To), /*bAvoidPeople=*/true))
		{
			Activity = ENPCActivity::Walking;
			return;
		}
	}
	Activity = ENPCActivity::Idle;
	ActivityTimeLeft = Random.FRandRange(1.0f, 2.0f);
}

void AAdastreaNPCController::GetPeopleNearby(TArray<FVector>& OutLocations, float Radius) const
{
	const APawn* Body = GetPawn();
	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		if (*It != Body && FVector::Dist(It->GetActorLocation(), Body->GetActorLocation()) < Radius)
		{
			OutLocations.Add(It->GetActorLocation());
		}
	}
}

bool AAdastreaNPCController::StartWalkTo(const FVector& WorldGoal, bool bAvoidPeople)
{
	const FDeckNavGrid* Grid = GetGrid();
	APawn* Body = GetPawn();
	if (!Grid || !Body)
	{
		return false;
	}
	const float HalfHeight = Body->GetRootComponent()->Bounds.BoxExtent.Z;
	const FVector Feet = Body->GetActorLocation() - FVector(0.0f, 0.0f, HalfHeight);
	TArray<FVector> People;
	if (bAvoidPeople)
	{
		GetPeopleNearby(People, 600.0f);
	}
	if (!Grid->FindPath(Feet, WorldGoal, Path, People) || Path.Num() == 0)
	{
		return false;
	}
	PathIndex = 0;
	Destination = WorldGoal;
	LastProgressLocation = Body->GetActorLocation();
	StuckTime = 0.0f;
	return true;
}

void AAdastreaNPCController::TickWalking(float DeltaSeconds)
{
	APawn* Body = GetPawn();
	if (!Path.IsValidIndex(PathIndex))
	{
		Activity = ENPCActivity::Idle;
		ActivityTimeLeft = 1.0f;
		return;
	}

	const float HalfHeight = Body->GetRootComponent()->Bounds.BoxExtent.Z;
	const FVector Location = Body->GetActorLocation();
	const FVector Feet = Location - FVector(0.0f, 0.0f, HalfHeight);
	FVector ToTarget = Path[PathIndex] - Feet;
	ToTarget.Z = 0.0f;
	const float Distance = ToTarget.Size();
	const bool bFinalLeg = PathIndex == Path.Num() - 1;

	if (Distance < (bFinalLeg ? 35.0f : 55.0f))
	{
		if (++PathIndex >= Path.Num())
		{
			// Arrived.
			++ArrivedCount;
			Replans = 0;
			Path.Reset();
			if (FixtureTarget.IsValid())
			{
				Activity = ENPCActivity::UsingFixture;
				ActivityTimeLeft = Random.FRandRange(FixtureTime.X, FixtureTime.Y);
			}
			else
			{
				Activity = ENPCActivity::Idle;
				ActivityTimeLeft = Random.FRandRange(IdleTime.X, IdleTime.Y);
			}
		}
		return;
	}

	// Ease off on the last metre so they stop where they meant to.
	const float Throttle = bFinalLeg ? FMath::Clamp(Distance / 120.0f, 0.35f, 1.0f) : 1.0f;
	Body->AddMovementInput(ToTarget / Distance, Throttle);

	// Not getting anywhere (blocked by someone, or a corner the grid was optimistic
	// about): re-plan from here a couple of times, then pick something else to do.
	if (FVector::Dist2D(Location, LastProgressLocation) > 40.0f)
	{
		LastProgressLocation = Location;
		StuckTime = 0.0f;
	}
	else if ((StuckTime += DeltaSeconds) > 2.0f)
	{
		// Usually someone's standing in the way: plan round them. If they close it off
		// (a narrow corridor), go somewhere still reachable instead of pushing at them.
		const FVector Goal = Destination;
		if (Replans < 2 && StartWalkTo(Goal, /*bAvoidPeople=*/true))
		{
			++Replans;
			return;
		}
		++GaveUpCount;
		Replans = 0;
		FixtureTarget = nullptr;
		const FDeckNavGrid* Grid = GetGrid();
		const int32 From = Grid ? Grid->FindNearestNode(Feet) : INDEX_NONE;
		for (int32 Try = 0; Try < 6 && From != INDEX_NONE; ++Try)
		{
			const int32 To = Grid->GetRandomReachableNode(From, Random, 200.0f);
			if (To != INDEX_NONE && StartWalkTo(Grid->GetNodeLocation(To), /*bAvoidPeople=*/true))
			{
				return; // still Walking, to the new spot
			}
		}
		Path.Reset();
		Activity = ENPCActivity::Idle;
		ActivityTimeLeft = Random.FRandRange(1.0f, 2.0f);
	}
}

void AAdastreaNPCController::FaceToward(const FVector& WorldPoint, float DeltaSeconds)
{
	APawn* Body = GetPawn();
	const FVector To = WorldPoint - Body->GetActorLocation();
	if (To.SizeSquared2D() < 1.0f)
	{
		return;
	}
	const float Current = Body->GetActorRotation().Yaw;
	const float Wanted = To.Rotation().Yaw;
	Body->SetActorRotation(FRotator(0.0f, FMath::FixedTurn(Current, Wanted, 240.0f * DeltaSeconds), 0.0f));
}

void AAdastreaNPCController::StartConversation(AActor* Listener, float Duration)
{
	if (Activity == ENPCActivity::Disabled)
	{
		return;
	}
	if (Activity != ENPCActivity::Talking)
	{
		ResumeActivity = Activity;
	}
	Activity = ENPCActivity::Talking;
	ActivityTimeLeft = Duration;
	TalkTarget = Listener;
}
