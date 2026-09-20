#include "Mining/Asteroid.h"
#include "Mining/AsteroidDataAsset.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Trading/TradeItemDataAsset.h"
#include "AdastreaLog.h"

AAsteroid::AAsteroid()
	: FieldIndex(-1)
	, TotalOre(0.0f)
	, RemainingOre(0.0f)
	, FullScale(1.0f)
	, MinShrinkScale(0.35f)
	, MeshBaseRadius(100.0f)
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AsteroidMesh"));
	RootComponent = Mesh;
	Mesh->SetMobility(EComponentMobility::Movable);
	// The ship flies kinematically, but line traces (targeting) must still hit rocks.
	Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AAsteroid::InitAsteroid(UAsteroidDataAsset* InType, UStaticMesh* InMesh, float InScale, float InRelScale, float OreOverride)
{
	AsteroidType = InType;
	FullScale = FMath::Max(InScale, 0.01f);
	if (InMesh)
	{
		Mesh->SetStaticMesh(InMesh);
		MeshBaseRadius = InMesh->GetBounds().SphereRadius;
	}
	if (AsteroidType)
	{
		if (UMaterialInterface* Mat = AsteroidType->Material.LoadSynchronous())
		{
			MID = UMaterialInstanceDynamic::Create(Mat, this);
			Mesh->SetMaterial(0, MID);
		}
		TotalOre = AsteroidType->GetTotalOreForScale(InRelScale);
	}
	else
	{
		TotalOre = 100.0f;
	}
	RemainingOre = OreOverride >= 0.0f ? FMath::Min(OreOverride, TotalOre) : TotalOre;
	UpdateVisuals();
}

float AAsteroid::GetRadius() const
{
	return MeshBaseRadius * GetActorScale3D().GetMax();
}

float AAsteroid::ExtractOre(float MiningPower, float DeltaTime)
{
	if (RemainingOre <= 0.0f || !AsteroidType)
	{
		return 0.0f;
	}
	const float Rate = AsteroidType->OreYieldPerSecond * MiningPower / FMath::Max(AsteroidType->Hardness, 0.1f);
	const float Removed = FMath::Min(Rate * DeltaTime, RemainingOre);
	RemainingOre -= Removed;
	UpdateVisuals();

	if (RemainingOre <= KINDA_SMALL_NUMBER)
	{
		RemainingOre = 0.0f;
		UE_LOG(LogAdastrea, Log, TEXT("Asteroid %s depleted"), *GetName());
		OnDepleted.Broadcast(this);
		Destroy();
	}
	return Removed;
}

void AAsteroid::UpdateVisuals()
{
	// Linear size follows the cube root of the remaining volume, floored at MinShrinkScale.
	const float Fraction = GetOreFraction();
	const float S = FullScale * FMath::Lerp(MinShrinkScale, 1.0f, FMath::Pow(Fraction, 1.0f / 3.0f));
	SetActorScale3D(FVector(S));
	if (MID)
	{
		MID->SetScalarParameterValue(TEXT("OreFraction"), Fraction);
	}
}

bool AAsteroid::CanBeTargeted_Implementation() const
{
	return RemainingOre > 0.0f;
}

FText AAsteroid::GetTargetDisplayName_Implementation() const
{
	return AsteroidType ? AsteroidType->DisplayName : FText::FromString(TEXT("Asteroid"));
}

float AAsteroid::GetDistanceFromLocation_Implementation(FVector FromLocation) const
{
	return FVector::Dist(FromLocation, GetActorLocation());
}
