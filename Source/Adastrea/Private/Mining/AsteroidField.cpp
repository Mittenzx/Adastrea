#include "Mining/AsteroidField.h"
#include "Mining/Asteroid.h"
#include "Mining/AsteroidDataAsset.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Camera/PlayerCameraManager.h"
#include "Materials/MaterialInterface.h"
#include "AdastreaLog.h"

AAsteroidField::AAsteroidField()
	: AsteroidCount(120)
	, FieldRadius(60000.0f)
	, VerticalFraction(0.35f)
	, MinSeparation(500.0f)
	, GlobalScale(1.0f)
	, Seed(1337)
	, PromoteRadius(30000.0f)
	, RespawnSeconds(120.0f)
	, UpdateTimer(0.0f)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.4f;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("FieldRoot"));
	RootComponent = Root;
}

void AAsteroidField::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Regenerate();
}

void AAsteroidField::BeginPlay()
{
	Super::BeginPlay();
	if (Rocks.Num() == 0)
	{
		Regenerate();
	}
	UE_LOG(LogAdastrea, Log, TEXT("AsteroidField %s: %d rocks, promote radius %.0f cm"), *GetName(), Rocks.Num(), PromoteRadius);
}

void AAsteroidField::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearField();
	Super::EndPlay(EndPlayReason);
}

void AAsteroidField::ClearField()
{
	for (FFieldRock& Rock : Rocks)
	{
		if (AAsteroid* A = Rock.Actor.Get())
		{
			A->OnDepleted.RemoveAll(this);
			A->Destroy();
		}
	}
	Rocks.Reset();
	for (UHierarchicalInstancedStaticMeshComponent* Comp : InstanceComps)
	{
		if (Comp)
		{
			Comp->DestroyComponent();
		}
	}
	InstanceComps.Reset();
}

UHierarchicalInstancedStaticMeshComponent* AAsteroidField::FindOrCreateInstanceComp(UStaticMesh* Mesh, UAsteroidDataAsset* Type)
{
	UMaterialInterface* Mat = Type ? Type->Material.LoadSynchronous() : nullptr;
	for (UHierarchicalInstancedStaticMeshComponent* Comp : InstanceComps)
	{
		if (Comp && Comp->GetStaticMesh() == Mesh && Comp->GetMaterial(0) == (Mat ? Mat : Mesh->GetMaterial(0)))
		{
			return Comp;
		}
	}
	UHierarchicalInstancedStaticMeshComponent* Comp = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, NAME_None, RF_Transient);
	Comp->SetStaticMesh(Mesh);
	if (Mat)
	{
		Comp->SetMaterial(0, Mat);
	}
	Comp->SetMobility(EComponentMobility::Movable);
	Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Comp->SetupAttachment(RootComponent);
	Comp->RegisterComponent();
	InstanceComps.Add(Comp);
	return Comp;
}

void AAsteroidField::Regenerate()
{
	ClearField();

	TArray<UAsteroidDataAsset*> Types;
	float TotalWeight = 0.0f;
	for (UAsteroidDataAsset* T : AsteroidTypes)
	{
		if (T && T->Rarity > 0.0f)
		{
			Types.Add(T);
			TotalWeight += T->Rarity;
		}
	}
	if (Types.Num() == 0)
	{
		return;
	}

	UStaticMesh* Fallback = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	FRandomStream Rand(Seed);
	const FTransform ActorXf = GetActorTransform();
	TArray<TPair<FVector, float>> Placed; // world centre, radius

	for (int32 i = 0; i < AsteroidCount; ++i)
	{
		// Pick a type by weight.
		float Pick = Rand.FRand() * TotalWeight;
		UAsteroidDataAsset* Type = Types.Last();
		for (UAsteroidDataAsset* T : Types)
		{
			Pick -= T->Rarity;
			if (Pick <= 0.0f) { Type = T; break; }
		}

		UStaticMesh* Mesh = nullptr;
		if (Type->Meshes.Num() > 0)
		{
			Mesh = Type->Meshes[Rand.RandRange(0, Type->Meshes.Num() - 1)].LoadSynchronous();
		}
		if (!Mesh)
		{
			Mesh = Fallback;
		}
		if (!Mesh)
		{
			continue;
		}

		// Small rocks are more common than big ones.
		const float Rel = FMath::Lerp(Type->MinScale, Type->MaxScale, FMath::Pow(Rand.FRand(), 1.6f));
		const float Scale = Rel * GlobalScale;
		const float Radius = Mesh->GetBounds().SphereRadius * Scale;

		FVector Pos = FVector::ZeroVector;
		bool bFound = false;
		for (int32 Try = 0; Try < 30 && !bFound; ++Try)
		{
			const FVector Local(Rand.FRandRange(-1.f, 1.f), Rand.FRandRange(-1.f, 1.f), Rand.FRandRange(-1.f, 1.f));
			if (Local.SizeSquared() > 1.0f) { continue; }
			Pos = ActorXf.TransformPosition(Local * FVector(FieldRadius, FieldRadius, FieldRadius * VerticalFraction));
			bFound = true;
			for (const TPair<FVector, float>& P : Placed)
			{
				if (FVector::Dist(P.Key, Pos) < P.Value + Radius + MinSeparation)
				{
					bFound = false;
					break;
				}
			}
		}
		if (!bFound)
		{
			continue;
		}
		Placed.Add(TPair<FVector, float>(Pos, Radius));

		FFieldRock Rock;
		Rock.Type = Type;
		Rock.Mesh = Mesh;
		Rock.Scale = Scale;
		Rock.Ore = Type->GetTotalOreForScale(Rel);
		Rock.Home = FTransform(FRotator(Rand.FRandRange(0, 360), Rand.FRandRange(0, 360), Rand.FRandRange(0, 360)), Pos, FVector(Scale));
		Rock.Instances = FindOrCreateInstanceComp(Mesh, Type);
		Rock.InstanceIndex = Rock.Instances->AddInstance(Rock.Home, /*bWorldSpace=*/true);
		Rocks.Add(Rock);
	}
}

int32 AAsteroidField::GetPromotedCount() const
{
	int32 N = 0;
	for (const FFieldRock& Rock : Rocks)
	{
		N += Rock.Actor.IsValid() ? 1 : 0;
	}
	return N;
}

void AAsteroidField::SetInstanceVisible(FFieldRock& Rock, bool bVisible, float ScaleFraction)
{
	if (!Rock.Instances || Rock.InstanceIndex == INDEX_NONE)
	{
		return;
	}
	FTransform Xf = Rock.Home;
	Xf.SetScale3D(bVisible ? Rock.Home.GetScale3D() * ScaleFraction : FVector(0.0001f));
	Rock.Instances->UpdateInstanceTransform(Rock.InstanceIndex, Xf, /*bWorldSpace=*/true, /*bMarkRenderStateDirty=*/true, /*bTeleport=*/true);
}

void AAsteroidField::Promote(int32 Index)
{
	FFieldRock& Rock = Rocks[Index];
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AAsteroid* A = GetWorld()->SpawnActor<AAsteroid>(AAsteroid::StaticClass(), Rock.Home.GetLocation(), Rock.Home.Rotator(), Params);
	if (!A)
	{
		return;
	}
	// Relative size (for ore total) is Scale / GlobalScale.
	A->InitAsteroid(Rock.Type, Rock.Mesh, Rock.Scale, Rock.Scale / FMath::Max(GlobalScale, 0.01f), Rock.Ore);
	A->FieldIndex = Index;
	A->OnDepleted.AddDynamic(this, &AAsteroidField::HandleAsteroidDepleted);
	Rock.Actor = A;
	SetInstanceVisible(Rock, false);
}

void AAsteroidField::Demote(int32 Index)
{
	FFieldRock& Rock = Rocks[Index];
	if (AAsteroid* A = Rock.Actor.Get())
	{
		Rock.Ore = A->GetRemainingOre();
		const float Shown = A->GetActorScale3D().X / FMath::Max(Rock.Scale, 0.0001f);
		A->OnDepleted.RemoveAll(this);
		A->Destroy();
		Rock.Actor = nullptr;
		SetInstanceVisible(Rock, true, Shown);
	}
}

void AAsteroidField::HandleAsteroidDepleted(AAsteroid* Asteroid)
{
	if (!Asteroid || !Rocks.IsValidIndex(Asteroid->FieldIndex))
	{
		return;
	}
	FFieldRock& Rock = Rocks[Asteroid->FieldIndex];
	Rock.Ore = 0.0f;
	Rock.Actor = nullptr;
	Rock.RespawnAt = RespawnSeconds > 0.0f ? GetWorld()->GetTimeSeconds() + RespawnSeconds : TNumericLimits<double>::Max();
}

void AAsteroidField::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return;
	}
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}
	const FVector Viewer = PC->GetPawn() ? PC->GetPawn()->GetActorLocation() : PC->PlayerCameraManager->GetCameraLocation();
	const double Now = World->GetTimeSeconds();
	const float DemoteRadius = PromoteRadius * 1.3f;

	for (int32 i = 0; i < Rocks.Num(); ++i)
	{
		FFieldRock& Rock = Rocks[i];

		if (Rock.RespawnAt > 0.0)
		{
			if (Now >= Rock.RespawnAt)
			{
				Rock.RespawnAt = 0.0;
				Rock.Ore = Rock.Type->GetTotalOreForScale(Rock.Scale / FMath::Max(GlobalScale, 0.01f));
				SetInstanceVisible(Rock, true);
			}
			continue;
		}

		const float Dist = FVector::Dist(Viewer, Rock.Home.GetLocation());
		if (!Rock.Actor.IsValid() && Dist < PromoteRadius)
		{
			Promote(i);
		}
		else if (Rock.Actor.IsValid() && Dist > DemoteRadius)
		{
			Demote(i);
		}
	}
}
