#include "Drones/DroneBayComponent.h"
#include "Drones/ShipDrone.h"
#include "Drones/CargoDrone.h"
#include "Universe/OrganisationSubsystem.h"
#include "Universe/PirateSubsystem.h"
#include "Mining/Asteroid.h"
#include "Mining/AsteroidDataAsset.h"
#include "Interfaces/ITargetable.h"
#include "Ships/Spaceship.h"
#include "Trading/CargoComponent.h"
#include "Trading/TradeItemDataAsset.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "AdastreaLog.h"
#include "Audio/AudioEventLibrary.h"

namespace DroneBayAudio
{
	constexpr float HoldFullMinInterval = 5.0f;
}

UDroneBayComponent::UDroneBayComponent()
	: bMiningEnabled(false)
	, DroneCount(3)
	, Range(30000.0f)
	, LockRange(80000.0f)
	, MiningPower(1.0f)
	, HopperCapacity(10.0f)
	, DroneSpeed(6000.0f)
	, DroneAcceleration(12000.0f)
	, LaunchInterval(0.6f)
	, MaxAimAngle(35.0f)
	, DroneClass(AShipDrone::StaticClass())
	, bAutoPlaceHatch(true)
	, SessionOreMined(0)
	, Status(EDroneBayStatus::Disabled)
	, bDeployed(false)
	, HatchClearance(1200.0f)
	, LaunchCooldown(0.0f)
	, LastMinedAmount(0)
	, LastMinedTime(-1.0)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	CargoDroneLoad = 25.0f;
	CargoDroneClass = ACargoDrone::StaticClass();
}

void UDroneBayComponent::BeginPlay()
{
	Super::BeginPlay();

	const AActor* Owner = GetOwner();
	if (bAutoPlaceHatch && Owner && GetAttachParent() == Owner->GetRootComponent())
	{
		// Ship meshes differ wildly in size and scale; put the hatch just inside the keel.
		// Visible hull meshes only: editor proxies (camera frustum etc.) would skew the box.
		FBox Local(ForceInit);
		TArray<UStaticMeshComponent*> Meshes;
		Owner->GetComponents<UStaticMeshComponent>(Meshes);
		for (const UStaticMeshComponent* Mesh : Meshes)
		{
			if (Mesh && Mesh->GetStaticMesh() && Mesh->IsVisible() && !Mesh->bHiddenInGame && !Mesh->IsEditorOnly())
			{
				Local += Mesh->CalcBounds(Mesh->GetComponentTransform().GetRelativeTransform(Owner->GetActorTransform())).GetBox();
			}
		}
		if (Local.IsValid)
		{
			const FVector Size = Local.GetSize();
			const FVector Centre = Local.GetCenter();
			SetRelativeLocation(FVector(Centre.X, Centre.Y, Local.Min.Z + Size.Z * 0.1f));
			HatchClearance = FMath::Max(800.0f, Size.Z * 0.35f);
		}
	}
	UE_LOG(LogAdastrea, Log, TEXT("DroneBay on %s: mining=%d drones=%d range=%.0f power=%.2f hatch=%s clearance=%.0f"),
		Owner ? *Owner->GetName() : TEXT("?"), bMiningEnabled, DroneCount, Range, MiningPower,
		*GetRelativeLocation().ToCompactString(), HatchClearance);
}

void UDroneBayComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (AShipDrone* Drone : Drones)
	{
		if (IsValid(Drone))
		{
			Drone->Destroy();
		}
	}
	Drones.Reset();
	for (ACargoDrone* Drone : CargoDrones)
	{
		if (IsValid(Drone))
		{
			Drone->Destroy();
		}
	}
	CargoDrones.Reset();
	Super::EndPlay(EndPlayReason);
}

UCargoComponent* UDroneBayComponent::GetCargo() const
{
	return GetOwner() ? GetOwner()->FindComponentByClass<UCargoComponent>() : nullptr;
}

FVector UDroneBayComponent::GetAimForward() const
{
	return GetOwner() ? GetOwner()->GetActorForwardVector() : GetForwardVector();
}

FVector UDroneBayComponent::GetHatchApproach() const
{
	// Straight "down" out of the hatch, clear of the hull.
	const FVector Down = GetOwner() ? -GetOwner()->GetActorUpVector() : -GetUpVector();
	return GetHatchLocation() + Down * HatchClearance;
}

FVector UDroneBayComponent::GetShipVelocity() const
{
	return GetOwner() ? GetOwner()->GetVelocity() : FVector::ZeroVector;
}

AActor* UDroneBayComponent::GetTarget() const
{
	return Target.Get();
}

bool UDroneBayComponent::SetTarget(AActor* NewTarget)
{
	if (!NewTarget || !NewTarget->IsA<AAsteroid>() || !NewTarget->Implements<UTargetable>() || !ITargetable::Execute_CanBeTargeted(NewTarget))
	{
		return false;
	}
	if (Target.Get() != NewTarget)
	{
		UE_LOG(LogAdastrea, Log, TEXT("DroneBay: target locked -> %s"), *NewTarget->GetName());
	}
	Target = NewTarget;
	return true;
}

void UDroneBayComponent::ClearTarget()
{
	Target = nullptr;
}

bool UDroneBayComponent::LockBestTarget(FVector Origin, FVector AimDirection)
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

bool UDroneBayComponent::LockNearestAhead()
{
	const FVector Origin = GetOwner() ? GetOwner()->GetActorLocation() : GetComponentLocation();
	return LockBestTarget(Origin, GetAimForward());
}

void UDroneBayComponent::LaunchDrones()
{
	if (!bMiningEnabled)
	{
		return;
	}
	if (!Target.IsValid())
	{
		LockNearestAhead();
	}
	if (!bDeployed)
	{
		bDeployed = true;
		LaunchCooldown = 0.0f;
		UE_LOG(LogAdastrea, Log, TEXT("DroneBay: launch ordered (target=%s, %d drones)"),
			Target.IsValid() ? *Target->GetName() : TEXT("none"), DroneCount);
	}
}

void UDroneBayComponent::RecallDrones()
{
	if (bDeployed)
	{
		UE_LOG(LogAdastrea, Log, TEXT("DroneBay: recall ordered (%d out, session ore %d)"), GetDronesOut(), SessionOreMined);
	}
	bDeployed = false;
}

int32 UDroneBayComponent::GetDronesOut() const
{
	int32 Count = 0;
	for (const AShipDrone* Drone : Drones)
	{
		Count += IsValid(Drone) ? 1 : 0;
	}
	return Count;
}

int32 UDroneBayComponent::GetDronesCutting() const
{
	int32 Count = 0;
	for (const AShipDrone* Drone : Drones)
	{
		Count += (IsValid(Drone) && Drone->GetDroneState() == EShipDroneState::Cutting) ? 1 : 0;
	}
	return Count;
}

float UDroneBayComponent::GetOreInTransit() const
{
	float Total = 0.0f;
	for (const AShipDrone* Drone : Drones)
	{
		Total += IsValid(Drone) ? Drone->GetOreAboard() : 0.0f;
	}
	for (const TPair<TObjectPtr<UTradeItemDataAsset>, float>& Entry : BayOre)
	{
		Total += Entry.Value;
	}
	return Total;
}

float UDroneBayComponent::GetTargetSurfaceDistance() const
{
	if (const AAsteroid* Rock = Cast<AAsteroid>(Target.Get()))
	{
		return FMath::Max(FVector::Dist(GetHatchLocation(), Rock->GetActorLocation()) - Rock->GetRadius(), 0.0f);
	}
	return -1.0f;
}

bool UDroneBayComponent::IsTargetInRange() const
{
	const float Dist = GetTargetSurfaceDistance();
	return Dist >= 0.0f && Dist <= Range;
}

UTradeItemDataAsset* UDroneBayComponent::GetTargetOre() const
{
	const AAsteroid* Rock = Cast<AAsteroid>(Target.Get());
	const UAsteroidDataAsset* Type = Rock ? Rock->GetAsteroidType() : nullptr;
	return Type ? Type->OreItem.Get() : nullptr;
}

float UDroneBayComponent::GetDroneCutRate() const
{
	// Mirrors AAsteroid::ExtractOre.
	const AAsteroid* Rock = Cast<AAsteroid>(Target.Get());
	const UAsteroidDataAsset* Type = Rock ? Rock->GetAsteroidType() : nullptr;
	return Type ? Type->OreYieldPerSecond * MiningPower / FMath::Max(Type->Hardness, 0.1f) : 0.0f;
}

bool UDroneBayComponent::IsHoldFull() const
{
	const UCargoComponent* Cargo = GetCargo();
	if (!Cargo)
	{
		return true;
	}
	if (UTradeItemDataAsset* Ore = GetTargetOre())
	{
		// Count what the drones are already bringing home, so they stop going out once it will fill the hold.
		return !Cargo->HasSpaceFor(Ore, 1 + FMath::FloorToInt(GetOreInTransit()));
	}
	return Cargo->GetAvailableCargoSpace() < 1.0f;
}

float UDroneBayComponent::GetSecondsSinceLastMined() const
{
	const UWorld* World = GetWorld();
	return (World && LastMinedTime >= 0.0) ? static_cast<float>(World->GetTimeSeconds() - LastMinedTime) : 1.0e6f;
}

FText UDroneBayComponent::StatusToText(EDroneBayStatus InStatus)
{
	switch (InStatus)
	{
	case EDroneBayStatus::Disabled:   return FText::FromString(TEXT("NO MINING DRONES"));
	case EDroneBayStatus::Stowed:     return FText::FromString(TEXT("DRONES STOWED"));
	case EDroneBayStatus::NoTarget:   return FText::FromString(TEXT("NO TARGET"));
	case EDroneBayStatus::OutOfRange: return FText::FromString(TEXT("OUT OF RANGE"));
	case EDroneBayStatus::HoldFull:   return FText::FromString(TEXT("HOLD FULL"));
	case EDroneBayStatus::Mining:     return FText::FromString(TEXT("MINING"));
	case EDroneBayStatus::Recalling:  return FText::FromString(TEXT("RECALLING"));
	case EDroneBayStatus::Transferring: return FText::FromString(TEXT("TRANSFERRING CARGO"));
	}
	return FText::GetEmpty();
}

FText UDroneBayComponent::GetStatusText() const
{
	const AAsteroid* Rock = Status == EDroneBayStatus::Mining ? Cast<AAsteroid>(Target.Get()) : nullptr;
	const UAsteroidDataAsset* Type = Rock ? Rock->GetAsteroidType() : nullptr;
	switch (Type ? Type->GetResourceKind() : EResourceKind::Rock)
	{
	case EResourceKind::Gas:     return FText::FromString(TEXT("SKIMMING GAS"));
	case EResourceKind::Salvage: return FText::FromString(TEXT("SALVAGING"));
	default:                     return StatusToText(Status);
	}
}

void UDroneBayComponent::SetStatus(EDroneBayStatus NewStatus)
{
	if (Status != NewStatus)
	{
		UE_LOG(LogAdastrea, Log, TEXT("DroneBay status: %s -> %s"),
			*StatusToText(Status).ToString(), *StatusToText(NewStatus).ToString());
		Status = NewStatus;
	}
}

AAsteroid* UDroneBayComponent::GetWorkTarget() const
{
	return (bMiningEnabled && bDeployed && Status == EDroneBayStatus::Mining) ? Cast<AAsteroid>(Target.Get()) : nullptr;
}

void UDroneBayComponent::UnloadOre(UTradeItemDataAsset* Ore, float Amount)
{
	if (Ore && Amount > 0.0f)
	{
		BayOre.FindOrAdd(Ore) += Amount;
		FlushBayOre();
	}
}

void UDroneBayComponent::FlushBayOre()
{
	UCargoComponent* Cargo = GetCargo();
	if (!Cargo)
	{
		return;
	}
	for (auto It = BayOre.CreateIterator(); It; ++It)
	{
		UTradeItemDataAsset* Ore = It.Key();
		if (!Ore)
		{
			It.RemoveCurrent();
			continue;
		}
		int32 Whole = FMath::FloorToInt(It.Value());
		while (Whole > 0 && !Cargo->HasSpaceFor(Ore, Whole))
		{
			--Whole;
		}
		if (Whole > 0 && Cargo->AddCargo(Ore, Whole))
		{
			It.Value() -= Whole;
			SessionOreMined += Whole;
			LastMinedOre = Ore;
			LastMinedAmount = Whole;
			LastMinedTime = GetWorld()->GetTimeSeconds();
			OnOreMined.Broadcast(Ore, Whole);
			UE_LOG(LogAdastrea, Log, TEXT("DroneBay: +%d %s unloaded (hold %d, session %d)"),
				Whole, *Ore->ItemName.ToString(), Cargo->GetItemQuantity(Ore), SessionOreMined);
		}
	}
}

void UDroneBayComponent::LaunchOneDrone()
{
	UWorld* World = GetWorld();
	if (!World || !DroneClass)
	{
		return;
	}
	FActorSpawnParameters Params;
	Params.Owner = GetOwner();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FRotator Facing = GetOwner() ? GetOwner()->GetActorRotation() : GetComponentRotation();
	if (AShipDrone* Drone = World->SpawnActor<AShipDrone>(DroneClass, GetHatchLocation(), Facing, Params))
	{
		Drone->InitDrone(this);
		Drones.Add(Drone);
	}
}

void UDroneBayComponent::NotifyDroneStowed(AShipDrone* Drone)
{
	Drones.Remove(Drone);
}

void UDroneBayComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Drones.RemoveAll([](const AShipDrone* Drone) { return !IsValid(Drone); });
	CargoDrones.RemoveAll([](const ACargoDrone* Drone) { return !IsValid(Drone); });
	FlushBayOre();

	if (!bMiningEnabled)
	{
		bDeployed = false;
		SetStatus(EDroneBayStatus::Disabled);
		return;
	}

	// Docking: everyone is already aboard (drones can't follow the ship into the bay).
	if (const ASpaceship* Ship = Cast<ASpaceship>(GetOwner()); Ship && (Ship->IsDocked() || Ship->IsDocking()))
	{
		RecallDrones();
		for (AShipDrone* Drone : TArray<TObjectPtr<AShipDrone>>(Drones))
		{
			if (IsValid(Drone))
			{
				Drone->ForceStow();
			}
		}
		StopCargoTransfer();
		for (ACargoDrone* Drone : TArray<TObjectPtr<ACargoDrone>>(CargoDrones))
		{
			if (IsValid(Drone))
			{
				Drone->ForceStow();
			}
		}
	}

	// Cargo drones out: the bay is busy moving cargo, not mining.
	TickCargoTransfer(DeltaTime);
	if (IsTransferring())
	{
		SetStatus(EDroneBayStatus::Transferring);
		return;
	}

	// A spent rock ends the job: the drones come home.
	AAsteroid* Rock = Cast<AAsteroid>(Target.Get());
	if (!Rock || !ITargetable::Execute_CanBeTargeted(Rock))
	{
		Target = nullptr;
		Rock = nullptr;
		if (bDeployed)
		{
			UE_LOG(LogAdastrea, Log, TEXT("DroneBay: no rock to work (spent or lost), drones returning"));
			RecallDrones();
		}
	}

	if (bDeployed && Rock && IsHoldFull())
	{
		// Nothing more will fit: bring everyone home.
		if (UAudioEventLibrary::IsLocalPlayerActor(GetOwner()))
		{
			UAudioEventLibrary::PlayEvent2D(this, TEXT("Mining.CargoFull"), DroneBayAudio::HoldFullMinInterval);
		}
		RecallDrones();
	}

	if (!bDeployed)
	{
		if (GetDronesOut() > 0)
		{
			SetStatus(EDroneBayStatus::Recalling);
		}
		else if (!Rock)
		{
			SetStatus(EDroneBayStatus::NoTarget);
		}
		else
		{
			SetStatus(IsHoldFull() ? EDroneBayStatus::HoldFull : EDroneBayStatus::Stowed);
		}
		return;
	}
	if (!IsTargetInRange())
	{
		SetStatus(EDroneBayStatus::OutOfRange);
		return;
	}

	SetStatus(EDroneBayStatus::Mining);
	LaunchCooldown -= DeltaTime;
	if (Drones.Num() < DroneCount && LaunchCooldown <= 0.0f)
	{
		LaunchOneDrone();
		LaunchCooldown = LaunchInterval;
	}
}

// ---------------------------------------------------------------------------
// Cargo transfer
// ---------------------------------------------------------------------------

namespace
{
	UCargoComponent* HoldOf(const AActor* Actor)
	{
		return Actor ? Actor->FindComponentByClass<UCargoComponent>() : nullptr;
	}
}

FName UDroneBayComponent::GetStolenTagFor(const ASpaceship* Other) const
{
	const UOrganisationSubsystem* Orgs = UOrganisationSubsystem::Get(this);
	if (!Orgs || !Other)
	{
		return NAME_None;
	}
	const FName Owner = Orgs->GetOwnerIdOf(Other);
	if (Owner.IsNone() || Owner == Orgs->GetOwnerIdOf(GetOwner()))
	{
		return NAME_None;
	}
	// Pirates have no claim anyone would honour: their wrecks are salvage.
	const UPirateSubsystem* Pirates = UPirateSubsystem::Get(this);
	return Pirates && Pirates->IsPirate(Owner) ? NAME_None : Owner;
}

bool UDroneBayComponent::CanTransferWith(const ASpaceship* Other, bool bTake, FText* OutReason) const
{
	auto Fail = [OutReason](const FString& Why)
	{
		if (OutReason)
		{
			*OutReason = FText::FromString(Why);
		}
		return false;
	};
	const ASpaceship* Self = Cast<ASpaceship>(GetOwner());
	const UCargoComponent* Mine = HoldOf(Self);
	const UCargoComponent* Theirs = HoldOf(Other);
	if (!bMiningEnabled || DroneCount <= 0)
	{
		return Fail(TEXT("This ship has no drones"));
	}
	if (!Self || !Other || Other == Self || !Mine || !Theirs)
	{
		return Fail(TEXT("No cargo hold to work with"));
	}
	if (Self->IsDocked() || Self->IsDocking() || Other->IsDocked())
	{
		return Fail(TEXT("Can't send drones while docked"));
	}
	if (FVector::Dist(GetHatchLocation(), Other->GetActorLocation()) > Range)
	{
		return Fail(FString::Printf(TEXT("Too far for the drones (%.0f m max)"), Range / 100.0f));
	}
	const UOrganisationSubsystem* Orgs = UOrganisationSubsystem::Get(this);
	const bool bSameOwner = Orgs && Orgs->GetOwnerIdOf(Other) == Orgs->GetOwnerIdOf(Self);
	if (bTake && !Other->IsWrecked() && !bSameOwner)
	{
		return Fail(TEXT("Drones only take cargo from wrecks and your own ships"));
	}
	if (!bTake && !bSameOwner)
	{
		return Fail(TEXT("Drones only hand cargo to your own ships"));
	}
	if ((bTake ? Theirs : Mine)->GetTotalUnits() <= 0)
	{
		return Fail(bTake ? TEXT("Its hold is empty") : TEXT("Your hold is empty"));
	}
	return true;
}

bool UDroneBayComponent::StartCargoTransfer(ASpaceship* Other, bool bTake, FName OnlyItem)
{
	FText Reason;
	if (!CanTransferWith(Other, bTake, &Reason))
	{
		UE_LOG(LogAdastrea, Log, TEXT("DroneBay: cargo transfer with %s refused: %s"), *GetNameSafe(Other), *Reason.ToString());
		return false;
	}
	RecallDrones(); // the mining drones come home; the cargo drones take over
	TransferTarget = Other;
	bTransferActive = true;
	bTransferTake = bTake;
	TransferItem = OnlyItem;
	TransferStolenFrom = bTake ? GetStolenTagFor(Other) : NAME_None;
	TransferredUnits = 0;
	CargoLaunchCooldown = 0.0f;
	UE_LOG(LogAdastrea, Log, TEXT("DroneBay: cargo transfer %s %s%s"), bTake ? TEXT("from") : TEXT("to"), *Other->GetName(),
		TransferStolenFrom.IsNone() ? TEXT("") : *FString::Printf(TEXT(" (goods stolen from %s)"), *TransferStolenFrom.ToString()));
	return true;
}

void UDroneBayComponent::StopCargoTransfer()
{
	if (bTransferActive)
	{
		UE_LOG(LogAdastrea, Log, TEXT("DroneBay: cargo transfer with %s stopped, %d units moved so far"), *GetNameSafe(TransferTarget.Get()), TransferredUnits);
	}
	bTransferActive = false;
}

bool UDroneBayComponent::IsTransferring() const
{
	return bTransferActive || !CargoDrones.IsEmpty();
}

bool UDroneBayComponent::IsTransferTarget(const ASpaceship* Ship) const
{
	return bTransferActive && Ship && TransferTarget.Get() == Ship;
}

ASpaceship* UDroneBayComponent::GetTransferTarget() const
{
	return TransferTarget.Get();
}

float UDroneBayComponent::GetInboundVolume() const
{
	// Taking, loaded drones are on their way home; giving, on their way out. Either way the
	// receiving hold must keep room for them.
	float Volume = 0.0f;
	for (const ACargoDrone* Drone : CargoDrones)
	{
		Volume += IsValid(Drone) ? Drone->GetLoadVolume() : 0.0f;
	}
	return Volume;
}

bool UDroneBayComponent::HasTransferWorkLeft() const
{
	const UCargoComponent* Mine = HoldOf(GetOwner());
	const UCargoComponent* Theirs = HoldOf(TransferTarget.Get());
	if (!Mine || !Theirs)
	{
		return false;
	}
	const UCargoComponent* From = bTransferTake ? Theirs : Mine;
	const UCargoComponent* To = bTransferTake ? Mine : Theirs;
	bool bAnything = false;
	float Smallest = TNumericLimits<float>::Max();
	for (const FCargoEntry& Entry : From->CargoInventory)
	{
		if (Entry.Item && Entry.Quantity > 0 && (TransferItem.IsNone() || UTradeItemDataAsset::ItemIdsMatch(Entry.Item->ItemID, TransferItem)))
		{
			bAnything = true;
			Smallest = FMath::Min(Smallest, Entry.Item->VolumePerUnit);
		}
	}
	return bAnything && To->GetAvailableCargoSpace() - GetInboundVolume() >= Smallest;
}

void UDroneBayComponent::FillCargoDrone(ACargoDrone* Drone, UCargoComponent* From, UCargoComponent* To)
{
	if (!Drone || !From || !To)
	{
		return;
	}
	// Room left in the receiving hold, less what other drones are already bringing it.
	const float Room = FMath::Min(CargoDroneLoad, To->GetAvailableCargoSpace() - GetInboundVolume());
	if (Room <= KINDA_SMALL_NUMBER)
	{
		return;
	}
	if (TransferItem.IsNone())
	{
		Drone->Load = From->TakeLoad(Room);
	}
	else
	{
		// Only the chosen goods: set the rest aside, take, and put them back.
		TArray<FCargoEntry> Kept;
		for (int32 i = From->CargoInventory.Num() - 1; i >= 0; --i)
		{
			const FCargoEntry& Entry = From->CargoInventory[i];
			if (!Entry.Item || !UTradeItemDataAsset::ItemIdsMatch(Entry.Item->ItemID, TransferItem))
			{
				Kept.Insert(Entry, 0);
				From->CargoInventory.RemoveAt(i);
			}
		}
		Drone->Load = From->TakeLoad(Room);
		From->CargoInventory.Append(Kept);
	}
	if (!TransferStolenFrom.IsNone())
	{
		for (FCargoEntry& Entry : Drone->Load)
		{
			if (!Entry.IsStolen())
			{
				Entry.StolenFrom = TransferStolenFrom;
			}
		}
	}
}

int32 UDroneBayComponent::EmptyCargoDrone(ACargoDrone* Drone, UCargoComponent* Into)
{
	int32 Moved = 0;
	if (!Drone || !Into)
	{
		return Moved;
	}
	for (int32 i = 0; i < Drone->Load.Num(); ++i)
	{
		FCargoEntry& Entry = Drone->Load[i];
		int32 Units = Entry.Quantity;
		while (Units > 0 && !Into->HasSpaceFor(Entry.Item, Units))
		{
			--Units;
		}
		if (Units > 0 && Into->AddEntry(FCargoEntry(Entry.Item, Units, Entry.StolenFrom)))
		{
			Entry.Quantity -= Units;
			Moved += Units;
		}
		if (Entry.Quantity <= 0)
		{
			Drone->Load.RemoveAt(i--);
		}
	}
	return Moved;
}

void UDroneBayComponent::NotifyCargoDroneAtTarget(ACargoDrone* Drone, ASpaceship* Other)
{
	UCargoComponent* Theirs = HoldOf(Other);
	if (bTransferTake)
	{
		FillCargoDrone(Drone, Theirs, HoldOf(GetOwner()));
	}
	else
	{
		TransferredUnits += EmptyCargoDrone(Drone, Theirs);
	}
}

void UDroneBayComponent::NotifyCargoDroneHome(ACargoDrone* Drone)
{
	// Taking, this is the load; giving, it's whatever didn't fit over there.
	const int32 Moved = EmptyCargoDrone(Drone, HoldOf(GetOwner()));
	if (bTransferTake)
	{
		TransferredUnits += Moved;
	}
	if (Drone && !Drone->Load.IsEmpty())
	{
		UE_LOG(LogAdastrea, Warning, TEXT("DroneBay: no room aboard for a cargo drone's load; %d stacks lost"), Drone->Load.Num());
		Drone->Load.Reset();
	}
}

bool UDroneBayComponent::ReloadCargoDrone(ACargoDrone* Drone)
{
	if (!bTransferActive || !HasTransferWorkLeft())
	{
		return false;
	}
	if (!bTransferTake)
	{
		FillCargoDrone(Drone, HoldOf(GetOwner()), HoldOf(TransferTarget.Get()));
		return Drone && !Drone->Load.IsEmpty();
	}
	return true;
}

void UDroneBayComponent::NotifyCargoDroneStowed(ACargoDrone* Drone)
{
	CargoDrones.Remove(Drone);
}

void UDroneBayComponent::LaunchCargoDrone()
{
	UWorld* World = GetWorld();
	ASpaceship* Other = TransferTarget.Get();
	if (!World || !CargoDroneClass || !Other)
	{
		return;
	}
	FActorSpawnParameters Params;
	Params.Owner = GetOwner();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FRotator Facing = GetOwner() ? GetOwner()->GetActorRotation() : GetComponentRotation();
	ACargoDrone* Drone = World->SpawnActor<ACargoDrone>(CargoDroneClass, GetHatchLocation(), Facing, Params);
	if (!Drone)
	{
		return;
	}
	if (!bTransferTake)
	{
		FillCargoDrone(Drone, HoldOf(GetOwner()), HoldOf(Other));
		if (Drone->Load.IsEmpty())
		{
			Drone->Destroy();
			return;
		}
	}
	Drone->InitDrone(this, Other, CargoDrones.Num());
	CargoDrones.Add(Drone);
}

void UDroneBayComponent::TickCargoTransfer(float DeltaTime)
{
	if (bTransferActive)
	{
		FText Reason;
		ASpaceship* Other = TransferTarget.Get();
		if (!Other || !CanTransferWith(Other, bTransferTake, &Reason))
		{
			// Done (hold emptied), or the other ship left, docked or drifted out of range.
			UE_LOG(LogAdastrea, Log, TEXT("DroneBay: cargo transfer ends: %s"), Other ? *Reason.ToString() : TEXT("other ship gone"));
			StopCargoTransfer();
		}
		else if (!HasTransferWorkLeft())
		{
			// Nothing more to send; the drones out finish their trips first.
			if (CargoDrones.IsEmpty())
			{
				StopCargoTransfer();
			}
		}
		else
		{
			CargoLaunchCooldown -= DeltaTime;
			if (CargoDrones.Num() < DroneCount && CargoLaunchCooldown <= 0.0f)
			{
				LaunchCargoDrone();
				CargoLaunchCooldown = LaunchInterval;
			}
		}
	}
	if (!bTransferActive && TransferTarget.IsValid() && CargoDrones.IsEmpty())
	{
		// The job is over and the last drone is home.
		UE_LOG(LogAdastrea, Log, TEXT("DroneBay: cargo transfer complete, %d units moved"), TransferredUnits);
		TransferTarget = nullptr;
		OnCargoTransferFinished.Broadcast(TransferredUnits);
	}
}
