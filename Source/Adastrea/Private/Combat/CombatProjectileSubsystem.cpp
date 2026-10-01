#include "Combat/CombatProjectileSubsystem.h"
#include "Combat/ShipHealthComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "AdastreaLog.h"

namespace CombatFX
{
	/** Bolt thickness (cm). The engine cylinder is 100 across and 100 tall. */
	constexpr float BoltDiameter = 45.0f;
	constexpr float FlashLife = 0.18f;
	constexpr float ShipFlashSize = 260.0f;
	constexpr float WorldFlashSize = 160.0f;
}

UCombatProjectileSubsystem::UCombatProjectileSubsystem()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> LaserMat(TEXT("/Game/Materials/M_Combat_Laser.M_Combat_Laser"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> HitMat(TEXT("/Game/Materials/M_Combat_Hit.M_Combat_Hit"));
	BoltMesh = Cylinder.Object;
	FlashMesh = Sphere.Object;
	BoltMaterial = LaserMat.Object;
	FlashMaterial = HitMat.Object;
}

UCombatProjectileSubsystem* UCombatProjectileSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	return World ? World->GetSubsystem<UCombatProjectileSubsystem>() : nullptr;
}

bool UCombatProjectileSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId UCombatProjectileSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UCombatProjectileSubsystem, STATGROUP_Tickables);
}

void UCombatProjectileSubsystem::Deinitialize()
{
	if (IsValid(VisualActor))
	{
		VisualActor->Destroy();
	}
	VisualActor = nullptr;
	Bolts.Reset();
	Flashes.Reset();
	Targets.Reset();
	Super::Deinitialize();
}

void UCombatProjectileSubsystem::FireBolt(const FVector& Start, const FVector& Velocity, float Damage, float Life, float Length, AActor* Instigator, float Age)
{
	FCombatBolt& Bolt = Bolts.AddDefaulted_GetRef();
	Bolt.Position = Start;
	Bolt.Velocity = Velocity;
	Bolt.Life = Life;
	Bolt.Damage = Damage;
	Bolt.Length = Length;
	Bolt.PendingTime = FMath::Max(Age, 0.0f);
	Bolt.Instigator = Instigator;
}

void UCombatProjectileSubsystem::RegisterTarget(UShipHealthComponent* Target)
{
	if (Target)
	{
		Targets.AddUnique(Target);
	}
}

void UCombatProjectileSubsystem::UnregisterTarget(UShipHealthComponent* Target)
{
	Targets.Remove(Target);
}

int32 UCombatProjectileSubsystem::GetHitCount(const AActor* Instigator) const
{
	const int32* Count = HitCounts.Find(Instigator);
	return Count ? *Count : 0;
}

void UCombatProjectileSubsystem::Tick(float DeltaTime)
{
	if (Bolts.Num() > 0)
	{
		StepBolts(DeltaTime);
	}
	if (Bolts.Num() > 0 || Flashes.Num() > 0 || (BoltMeshes && BoltMeshes->GetInstanceCount() > 0)
		|| (FlashMeshes && FlashMeshes->GetInstanceCount() > 0))
	{
		UpdateVisuals(DeltaTime);
	}
}

void UCombatProjectileSubsystem::StepBolts(float DeltaTime)
{
	UWorld* World = GetWorld();
	Targets.RemoveAll([](const TWeakObjectPtr<UShipHealthComponent>& T) { return !T.IsValid(); });

	for (int32 i = Bolts.Num() - 1; i >= 0; --i)
	{
		FCombatBolt& Bolt = Bolts[i];
		const float StepTime = DeltaTime + Bolt.PendingTime;
		Bolt.PendingTime = 0.0f;
		const FVector From = Bolt.Position;
		const FVector To = From + Bolt.Velocity * StepTime;
		AActor* Instigator = Bolt.Instigator.Get();
		const UShipHealthComponent* InstigatorHealth = Instigator ? Instigator->FindComponentByClass<UShipHealthComponent>() : nullptr;
		const int32 InstigatorTeam = InstigatorHealth ? InstigatorHealth->Team : 0;

		// Nearest ship hull along the path.
		float BestTime = 2.0f;
		FVector BestPoint = FVector::ZeroVector;
		UShipHealthComponent* BestShip = nullptr;
		for (const TWeakObjectPtr<UShipHealthComponent>& WeakTarget : Targets)
		{
			UShipHealthComponent* Target = WeakTarget.Get();
			const AActor* Owner = Target->GetOwner();
			if (Target->IsDestroyed() || Owner == Instigator || Owner->IsHidden()
				|| (InstigatorTeam != 0 && Target->Team == InstigatorTeam))
			{
				continue;
			}
			FVector Hit;
			float Time;
			if (Target->SegmentHit(From, To, Hit, Time) && Time < BestTime)
			{
				BestTime = Time;
				BestPoint = Hit;
				BestShip = Target;
			}
		}

		// Stations, asteroids and other solid geometry stop bolts too (ship hulls have
		// no collision, so this never finds a ship).
		FHitResult WorldHit;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(CombatBolt), false, Instigator);
		const bool bWorldHit = World->LineTraceSingleByChannel(WorldHit, From, To, ECC_Visibility, Params)
			&& WorldHit.Time < BestTime;

		if (bWorldHit)
		{
			Flashes.Add({ WorldHit.ImpactPoint, 0.0f, CombatFX::WorldFlashSize });
			Bolts.RemoveAtSwap(i);
			continue;
		}
		if (BestShip)
		{
			BestShip->ApplyDamage(Bolt.Damage, Instigator);
			if (Instigator)
			{
				++HitCounts.FindOrAdd(Instigator);
			}
			Flashes.Add({ BestPoint, 0.0f, CombatFX::ShipFlashSize });
			Bolts.RemoveAtSwap(i);
			continue;
		}

		Bolt.Position = To;
		Bolt.Life -= StepTime;
		if (Bolt.Life <= 0.0f)
		{
			Bolts.RemoveAtSwap(i);
		}
	}
}

void UCombatProjectileSubsystem::EnsureVisualActor()
{
	if (IsValid(VisualActor))
	{
		return;
	}
	UWorld* World = GetWorld();
	FActorSpawnParameters Spawn;
	Spawn.Name = MakeUniqueObjectName(World->PersistentLevel, AActor::StaticClass(), TEXT("CombatFX"));
	Spawn.ObjectFlags |= RF_Transient;
	Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	VisualActor = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, Spawn);

	USceneComponent* Root = NewObject<USceneComponent>(VisualActor, TEXT("Root"));
	VisualActor->SetRootComponent(Root);
	Root->RegisterComponent();

	auto MakeLayer = [this, Root](const TCHAR* Name, UStaticMesh* Mesh, UMaterialInterface* Material)
	{
		UInstancedStaticMeshComponent* Layer = NewObject<UInstancedStaticMeshComponent>(VisualActor, Name);
		Layer->SetupAttachment(Root);
		Layer->SetMobility(EComponentMobility::Movable);
		Layer->SetStaticMesh(Mesh);
		if (Material)
		{
			Layer->SetMaterial(0, Material);
		}
		Layer->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Layer->SetCastShadow(false);
		Layer->SetCanEverAffectNavigation(false);
		Layer->RegisterComponent();
		return Layer;
	};
	BoltMeshes = MakeLayer(TEXT("Bolts"), BoltMesh, BoltMaterial);
	FlashMeshes = MakeLayer(TEXT("Flashes"), FlashMesh, FlashMaterial);
}

static void SyncInstances(UInstancedStaticMeshComponent* Layer, const TArray<FTransform>& Transforms)
{
	if (Transforms.Num() == 0)
	{
		Layer->ClearInstances();
		return;
	}
	while (Layer->GetInstanceCount() < Transforms.Num())
	{
		Layer->AddInstance(FTransform::Identity, true);
	}
	while (Layer->GetInstanceCount() > Transforms.Num())
	{
		Layer->RemoveInstance(Layer->GetInstanceCount() - 1);
	}
	Layer->BatchUpdateInstancesTransforms(0, Transforms, true, true, true);
}

void UCombatProjectileSubsystem::UpdateVisuals(float DeltaTime)
{
	EnsureVisualActor();
	if (!BoltMeshes || !FlashMeshes)
	{
		return;
	}

	TArray<FTransform> Transforms;
	Transforms.Reserve(Bolts.Num());
	for (const FCombatBolt& Bolt : Bolts)
	{
		// The cylinder stands along +Z around its centre: lay it along the flight path,
		// trailing behind the bolt's head.
		const FVector Dir = Bolt.Velocity.GetSafeNormal();
		const FQuat Rot = FRotationMatrix::MakeFromZ(Dir).ToQuat();
		const FVector Scale(CombatFX::BoltDiameter / 100.0f, CombatFX::BoltDiameter / 100.0f, Bolt.Length / 100.0f);
		Transforms.Emplace(Rot, Bolt.Position - Dir * (Bolt.Length * 0.5f), Scale);
	}
	SyncInstances(BoltMeshes, Transforms);

	Transforms.Reset();
	for (int32 i = Flashes.Num() - 1; i >= 0; --i)
	{
		FCombatFlash& Flash = Flashes[i];
		Flash.Age += DeltaTime;
		if (Flash.Age >= CombatFX::FlashLife)
		{
			Flashes.RemoveAtSwap(i);
		}
	}
	for (const FCombatFlash& Flash : Flashes)
	{
		// Pops to full size, then shrinks away.
		const float Alpha = Flash.Age / CombatFX::FlashLife;
		const float Size = Flash.Size * FMath::Sin(PI * FMath::Clamp(0.25f + 0.75f * Alpha, 0.0f, 1.0f));
		Transforms.Emplace(FQuat::Identity, Flash.Position, FVector(FMath::Max(Size, 1.0f) / 100.0f));
	}
	SyncInstances(FlashMeshes, Transforms);
}
