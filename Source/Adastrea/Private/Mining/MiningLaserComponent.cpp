#include "Mining/MiningLaserComponent.h"
#include "Mining/Asteroid.h"
#include "Mining/AsteroidDataAsset.h"
#include "Interfaces/ITargetable.h"
#include "Trading/CargoComponent.h"
#include "Trading/TradeItemDataAsset.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "EngineUtils.h"
#include "AdastreaLog.h"
#include "Audio/AudioEventLibrary.h"
#include "Components/AudioComponent.h"

namespace MiningEventAudio
{
	/** Beam-off gap before the loop fades (aim flicker at the cone edge shouldn't retrigger it). */
	constexpr float LoopReleaseDelay = 0.15f;
	constexpr float LoopFadeIn = 0.08f;
	constexpr float LoopFadeOut = 0.25f;
	/** Loop pitch at point-blank and at full range: a longer beam sounds lower. */
	constexpr float PitchNear = 1.1f;
	constexpr float PitchFar = 0.9f;
	constexpr float CargoFullMinInterval = 5.0f;
	constexpr float DepletedMinInterval = 0.5f;
}

UMiningLaserComponent::UMiningLaserComponent()
	: bMiningEnabled(false)
	, Range(30000.0f)
	, LockRange(80000.0f)
	, MiningPower(1.0f)
	, MaxAimAngle(35.0f)
	, BeamWidth(20.0f)
	, BeamMaterial(FSoftObjectPath(TEXT("/Game/Materials/M_MiningLaser.M_MiningLaser")))
	, ImpactDust(FSoftObjectPath(TEXT("/Game/VFX/Environment/NS_Asteroid_Dust.NS_Asteroid_Dust")))
	, SessionOreMined(0)
	, Status(EMiningStatus::Disabled)
	, bTriggerHeld(false)
	, CarriedOre(0.0f)
	, LastMinedAmount(0)
	, LastMinedTime(-1.0)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UMiningLaserComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	BeamMesh = NewObject<UStaticMeshComponent>(Owner, TEXT("MiningBeamMesh"));
	BeamMesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")));
	if (UMaterialInterface* Mat = BeamMaterial.LoadSynchronous())
	{
		BeamMesh->SetMaterial(0, Mat);
	}
	BeamMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeamMesh->SetCastShadow(false);
	BeamMesh->SetMobility(EComponentMobility::Movable);
	BeamMesh->SetVisibility(false);
	BeamMesh->RegisterComponent();
	BeamMesh->SetUsingAbsoluteLocation(true);
	BeamMesh->SetUsingAbsoluteRotation(true);
	BeamMesh->SetUsingAbsoluteScale(true);
	BeamMesh->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);

	if (UNiagaraSystem* Dust = ImpactDust.LoadSynchronous())
	{
		DustFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Dust, GetComponentLocation(),
			FRotator::ZeroRotator, FVector::OneVector, false, false);
	}
	UE_LOG(LogAdastrea, Log, TEXT("MiningLaser on %s: enabled=%d range=%.0f power=%.2f beamMat=%s dust=%s"),
		*Owner->GetName(), bMiningEnabled, Range, MiningPower,
		BeamMesh->GetMaterial(0) ? *BeamMesh->GetMaterial(0)->GetName() : TEXT("none"),
		DustFX ? TEXT("ok") : TEXT("none"));
}

void UMiningLaserComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BeamMesh)
	{
		BeamMesh->DestroyComponent();
	}
	if (DustFX)
	{
		DustFX->DestroyComponent();
	}
	UAudioEventLibrary::FadeOutAndRelease(LaserLoopAudio, 0.0f);
	LaserLoopAudio = nullptr;
	Super::EndPlay(EndPlayReason);
}

UCargoComponent* UMiningLaserComponent::GetCargo() const
{
	return GetOwner() ? GetOwner()->FindComponentByClass<UCargoComponent>() : nullptr;
}

FVector UMiningLaserComponent::GetAimForward() const
{
	return GetOwner() ? GetOwner()->GetActorForwardVector() : GetForwardVector();
}

AActor* UMiningLaserComponent::GetTarget() const
{
	return Target.Get();
}

bool UMiningLaserComponent::SetTarget(AActor* NewTarget)
{
	if (!NewTarget || !NewTarget->Implements<UTargetable>() || !ITargetable::Execute_CanBeTargeted(NewTarget))
	{
		return false;
	}
	if (Target.Get() != NewTarget)
	{
		CarriedOre = 0.0f;
		UE_LOG(LogAdastrea, Log, TEXT("MiningLaser: target locked -> %s"), *NewTarget->GetName());
	}
	Target = NewTarget;
	return true;
}

void UMiningLaserComponent::ClearTarget()
{
	Target = nullptr;
	CarriedOre = 0.0f;
}

bool UMiningLaserComponent::LockBestTarget(FVector Origin, FVector AimDirection)
{
	if (!GetWorld())
	{
		return false;
	}
	const FVector Dir = AimDirection.GetSafeNormal();
	AAsteroid* Best = nullptr;
	float BestScore = -1.0f;
	for (TActorIterator<AAsteroid> It(GetWorld()); It; ++It)
	{
		AAsteroid* Rock = *It;
		if (!Rock || !ITargetable::Execute_CanBeTargeted(Rock))
		{
			continue;
		}
		const FVector ToRock = Rock->GetActorLocation() - Origin;
		const float Dist = ToRock.Size();
		if (Dist - Rock->GetRadius() > LockRange || Dist < KINDA_SMALL_NUMBER)
		{
			continue;
		}
		// Angular size makes big rocks easier to pick than small distant ones.
		const float Cos = FVector::DotProduct(ToRock / Dist, Dir);
		const float AngRadius = FMath::Atan2(Rock->GetRadius(), Dist);
		const float AngOff = FMath::Acos(FMath::Clamp(Cos, -1.0f, 1.0f));
		if (AngOff > FMath::DegreesToRadians(MaxAimAngle) + AngRadius)
		{
			continue;
		}
		const float Score = 1.0f - FMath::Max(AngOff - AngRadius, 0.0f) / FMath::DegreesToRadians(MaxAimAngle + 1.0f)
			- Dist / (LockRange * 4.0f);
		if (!Best || Score > BestScore)
		{
			Best = Rock;
			BestScore = Score;
		}
	}
	return Best && SetTarget(Best);
}

bool UMiningLaserComponent::LockNearestAhead()
{
	return LockBestTarget(GetMuzzleLocation(), GetAimForward());
}

void UMiningLaserComponent::StartMining()
{
	if (!bMiningEnabled)
	{
		return;
	}
	bTriggerHeld = true;
	if (!Target.IsValid())
	{
		LockNearestAhead();
	}
	UE_LOG(LogAdastrea, Log, TEXT("MiningLaser: trigger pressed (target=%s)"),
		Target.IsValid() ? *Target->GetName() : TEXT("none"));
}

void UMiningLaserComponent::StopMining()
{
	if (bTriggerHeld)
	{
		UE_LOG(LogAdastrea, Log, TEXT("MiningLaser: trigger released (session ore %d)"), SessionOreMined);
	}
	bTriggerHeld = false;
}

float UMiningLaserComponent::GetTargetSurfaceDistance() const
{
	if (const AAsteroid* Rock = Cast<AAsteroid>(Target.Get()))
	{
		return FMath::Max(FVector::Dist(GetMuzzleLocation(), Rock->GetActorLocation()) - Rock->GetRadius(), 0.0f);
	}
	return -1.0f;
}

bool UMiningLaserComponent::IsTargetInRange() const
{
	const float Dist = GetTargetSurfaceDistance();
	return Dist >= 0.0f && Dist <= Range;
}

UTradeItemDataAsset* UMiningLaserComponent::GetTargetOre() const
{
	const AAsteroid* Rock = Cast<AAsteroid>(Target.Get());
	const UAsteroidDataAsset* Type = Rock ? Rock->GetAsteroidType() : nullptr;
	return Type ? Type->OreItem.Get() : nullptr;
}

float UMiningLaserComponent::GetExtractionRate() const
{
	// Mirrors AAsteroid::ExtractOre.
	const AAsteroid* Rock = Cast<AAsteroid>(Target.Get());
	const UAsteroidDataAsset* Type = Rock ? Rock->GetAsteroidType() : nullptr;
	return Type ? Type->OreYieldPerSecond * MiningPower / FMath::Max(Type->Hardness, 0.1f) : 0.0f;
}

float UMiningLaserComponent::GetSecondsSinceLastMined() const
{
	const UWorld* World = GetWorld();
	return (World && LastMinedTime >= 0.0) ? static_cast<float>(World->GetTimeSeconds() - LastMinedTime) : 1.0e6f;
}

FText UMiningLaserComponent::StatusToText(EMiningStatus InStatus)
{
	switch (InStatus)
	{
	case EMiningStatus::Disabled:   return FText::FromString(TEXT("NO MINING LASER"));
	case EMiningStatus::Idle:       return FText::FromString(TEXT("READY"));
	case EMiningStatus::NoTarget:   return FText::FromString(TEXT("NO TARGET"));
	case EMiningStatus::OutOfRange: return FText::FromString(TEXT("OUT OF RANGE"));
	case EMiningStatus::OffAim:     return FText::FromString(TEXT("OFF AIM"));
	case EMiningStatus::CargoFull:  return FText::FromString(TEXT("CARGO FULL"));
	case EMiningStatus::Mining:     return FText::FromString(TEXT("MINING"));
	}
	return FText::GetEmpty();
}

void UMiningLaserComponent::SetStatus(EMiningStatus NewStatus)
{
	if (Status != NewStatus)
	{
		UE_LOG(LogAdastrea, Log, TEXT("MiningLaser status: %s -> %s"),
			*StatusToText(Status).ToString(), *StatusToText(NewStatus).ToString());
		Status = NewStatus;

		// The hold filled up under the beam (the trigger is held, so the player is mining).
		if (NewStatus == EMiningStatus::CargoFull && UAudioEventLibrary::IsLocalPlayerActor(GetOwner()))
		{
			UAudioEventLibrary::PlayEvent2D(this, TEXT("Mining.CargoFull"), MiningEventAudio::CargoFullMinInterval);
		}
	}
}

void UMiningLaserComponent::UpdateLaserAudio(bool bBeamActive, float BeamLength)
{
	using namespace MiningEventAudio;

	if (bBeamActive)
	{
		LaserAudioOffTime = 0.0f;
		if (!LaserLoopAudio && UAudioEventLibrary::IsLocalPlayerActor(GetOwner()))
		{
			LaserLoopAudio = UAudioEventLibrary::SpawnEventAttached(this, TEXT("Mining.LaserLoop"), this, false);
			if (LaserLoopAudio)
			{
				LaserLoopAudio->FadeIn(LoopFadeIn, 1.0f);
			}
		}
		if (LaserLoopAudio)
		{
			const float Pitch = FMath::Lerp(PitchNear, PitchFar, FMath::Clamp(BeamLength / FMath::Max(Range, 1.0f), 0.0f, 1.0f));
			if (!FMath::IsNearlyEqual(LaserLoopAudio->PitchMultiplier, Pitch, 0.01f))
			{
				LaserLoopAudio->SetPitchMultiplier(Pitch);
			}
		}
		return;
	}

	if (LaserLoopAudio)
	{
		LaserAudioOffTime += GetWorld() ? GetWorld()->GetDeltaSeconds() : LoopReleaseDelay;
		if (LaserAudioOffTime >= LoopReleaseDelay)
		{
			UAudioEventLibrary::FadeOutAndRelease(LaserLoopAudio, LoopFadeOut);
			LaserLoopAudio = nullptr;
		}
	}
}

void UMiningLaserComponent::SetBeamActive(bool bActive, const FVector& HitPoint)
{
	if (!BeamMesh)
	{
		return;
	}
	UpdateLaserAudio(bActive, bActive ? FVector::Dist(GetMuzzleLocation(), HitPoint) : 0.0f);
	BeamMesh->SetVisibility(bActive);
	if (DustFX)
	{
		if (bActive)
		{
			DustFX->SetWorldLocation(HitPoint);
			if (!DustFX->IsActive())
			{
				DustFX->Activate(true);
			}
		}
		else if (DustFX->IsActive())
		{
			DustFX->Deactivate();
		}
	}
	if (bActive)
	{
		const FVector Start = GetMuzzleLocation();
		const FVector Delta = HitPoint - Start;
		const float Len = Delta.Size();
		if (Len > KINDA_SMALL_NUMBER)
		{
			// Engine cylinder is 100uu tall/wide, axis along Z.
			BeamMesh->SetWorldLocationAndRotation(Start + Delta * 0.5f, FRotationMatrix::MakeFromZ(Delta).Rotator());
			BeamMesh->SetWorldScale3D(FVector(BeamWidth / 100.0f, BeamWidth / 100.0f, Len / 100.0f));
		}
	}
}

void UMiningLaserComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bMiningEnabled)
	{
		SetStatus(EMiningStatus::Disabled);
		SetBeamActive(false);
		return;
	}

	// Depleted / destroyed rock: drop the lock, and re-acquire while the trigger is held.
	AAsteroid* Rock = Cast<AAsteroid>(Target.Get());
	if (!Rock || !ITargetable::Execute_CanBeTargeted(Rock))
	{
		Target = nullptr;
		if (bTriggerHeld && LockNearestAhead())
		{
			Rock = Cast<AAsteroid>(Target.Get());
		}
	}
	if (!Rock)
	{
		SetStatus(EMiningStatus::NoTarget);
		SetBeamActive(false);
		return;
	}
	if (!bTriggerHeld)
	{
		SetStatus(EMiningStatus::Idle);
		SetBeamActive(false);
		return;
	}

	const FVector Muzzle = GetMuzzleLocation();
	const FVector ToRock = Rock->GetActorLocation() - Muzzle;
	const float Dist = ToRock.Size();
	if (Dist - Rock->GetRadius() > Range)
	{
		SetStatus(EMiningStatus::OutOfRange);
		SetBeamActive(false);
		return;
	}
	const float Cos = FVector::DotProduct(ToRock.GetSafeNormal(), GetAimForward());
	const float AngOff = FMath::Acos(FMath::Clamp(Cos, -1.0f, 1.0f));
	const float AngRadius = Dist > KINDA_SMALL_NUMBER ? FMath::Atan2(Rock->GetRadius(), Dist) : 0.0f;
	if (AngOff - AngRadius > FMath::DegreesToRadians(MaxAimAngle))
	{
		SetStatus(EMiningStatus::OffAim);
		SetBeamActive(false);
		return;
	}

	UAsteroidDataAsset* Type = Rock->GetAsteroidType();
	UTradeItemDataAsset* Ore = Type ? Type->OreItem : nullptr;
	UCargoComponent* Cargo = GetCargo();
	if (!Ore || !Cargo)
	{
		SetStatus(EMiningStatus::NoTarget);
		SetBeamActive(false);
		return;
	}
	if (!Cargo->HasSpaceFor(Ore, 1))
	{
		SetStatus(EMiningStatus::CargoFull);
		SetBeamActive(false);
		return;
	}

	// Beam ends on the near surface of the rock (before extraction may destroy it).
	const FVector HitPoint = Rock->GetActorLocation() - ToRock.GetSafeNormal() * Rock->GetRadius() * 0.85f;
	SetStatus(EMiningStatus::Mining);
	SetBeamActive(true, HitPoint);

	const FString RockName = Rock->GetName();
	const float Removed = Rock->ExtractOre(MiningPower, DeltaTime); // may destroy Rock
	CarriedOre += Removed;
	if (!IsValid(Rock) && UAudioEventLibrary::IsLocalPlayerActor(GetOwner()))
	{
		// Our beam took the last of it: the rock cracks apart where the beam hit.
		UAudioEventLibrary::PlayEventAtLocation(this, TEXT("Mining.AsteroidDepleted"), HitPoint,
			MiningEventAudio::DepletedMinInterval);
	}
	int32 Whole = FMath::FloorToInt(CarriedOre);
	while (Whole > 0 && !Cargo->HasSpaceFor(Ore, Whole))
	{
		--Whole;
	}
	if (Whole > 0 && Cargo->AddCargo(Ore, Whole))
	{
		CarriedOre -= Whole;
		SessionOreMined += Whole;
		LastMinedOre = Ore;
		LastMinedAmount = Whole;
		LastMinedTime = GetWorld()->GetTimeSeconds();
		OnOreMined.Broadcast(Ore, Whole);
		UE_LOG(LogAdastrea, Log, TEXT("MiningLaser: +%d %s from %s (hold %d, session %d)"),
			Whole, *Ore->ItemName.ToString(), *RockName, Cargo->GetItemQuantity(Ore), SessionOreMined);
	}
}
