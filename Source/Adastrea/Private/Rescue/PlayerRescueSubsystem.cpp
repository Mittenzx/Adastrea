#include "Rescue/PlayerRescueSubsystem.h"
#include "Rescue/EscapePod.h"
#include "Rescue/TowDrone.h"
#include "AI/AIMinerController.h"
#include "AI/AIPilotController.h"
#include "AI/HostileFighterController.h"
#include "Combat/ShipHealthComponent.h"
#include "Player/AdastreaPlayerController.h"
#include "Ships/Spaceship.h"
#include "Stations/DockingBayModule.h"
#include "Stations/SpaceStation.h"
#include "Engine/World.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "EngineUtils.h"
#include "AdastreaLog.h"

namespace PlayerRescue
{
	/** Repair in place after this long when there's no friendly station at all (s). */
	constexpr float FallbackRepairSeconds = 5.0f;
	/** The tow drops the wreck this far out from the docking bay (cm), inside docking range. */
	constexpr float DropDistance = 3000.0f;
	/** The pod is taken aboard a station this close to its bay (cm). */
	constexpr float StationArrival = 600.0f;
	/** Extra margin beyond the pickup ship's hull for the pod to be taken aboard (cm). */
	constexpr float ShipArrivalMargin = 400.0f;
	constexpr float CameraBlendSeconds = 1.5f;
	/** Tow shot: camera this far to the side of the wreck, above it, and back along the path (cm). */
	constexpr float TowShotSide = 3500.0f;
	constexpr float TowShotUp = 1200.0f;
	constexpr float TowShotBack = 1500.0f;
}

UPlayerRescueSubsystem* UPlayerRescueSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	return World ? World->GetSubsystem<UPlayerRescueSubsystem>() : nullptr;
}

bool UPlayerRescueSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId UPlayerRescueSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UPlayerRescueSubsystem, STATGROUP_Tickables);
}

void UPlayerRescueSubsystem::BeginRescue(ASpaceship* InWreck, APlayerController* PC)
{
	if (IsRescueUnderway() || !InWreck || !PC)
	{
		return;
	}
	// Disabling happens mid-damage (inside the projectile step); swap pawns next tick.
	Wreck = InWreck;
	PlayerController = PC;
	bPending = true;
}

void UPlayerRescueSubsystem::Message(const FString& Text, float Seconds, bool bWarning) const
{
	if (AAdastreaPlayerController* PC = Cast<AAdastreaPlayerController>(PlayerController.Get()))
	{
		PC->ShowHUDMessage(Text, Seconds, bWarning);
	}
	UE_LOG(LogAdastreaCombat, Log, TEXT("Rescue: %s"), *Text);
}

ASpaceStation* UPlayerRescueSubsystem::FindNearestFriendlyStation(const FVector& Location) const
{
	ASpaceStation* Best = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	for (TActorIterator<ASpaceStation> It(GetWorld()); It; ++It)
	{
		if (const ADockingBayModule* Bay = It->GetDockingBayModule())
		{
			const float DistSq = FVector::DistSquared(Location, Bay->GetActorLocation());
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				Best = *It;
			}
		}
	}
	return Best;
}

ASpaceship* UPlayerRescueSubsystem::FindPickupShip(const FVector& Location, float MaxDistance) const
{
	ASpaceship* Best = nullptr;
	float BestDistSq = FMath::Square(MaxDistance);
	for (TActorIterator<ASpaceship> It(GetWorld()); It; ++It)
	{
		ASpaceship* Ship = *It;
		const AAIPilotController* Pilot = Cast<AAIPilotController>(Ship->GetController());
		// Traders in flight only: miners are busy with rocks, hostiles aren't friends,
		// and a docked ship can't come and fetch anyone.
		if (!Pilot || Pilot->IsA<AAIMinerController>() || Pilot->IsA<AHostileFighterController>()
			|| Ship->IsWrecked() || Ship->IsDocked() || Ship->IsDocking())
		{
			continue;
		}
		const float DistSq = FVector::DistSquared(Location, Ship->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = Ship;
		}
	}
	return Best;
}

void UPlayerRescueSubsystem::Tick(float DeltaTime)
{
	if (bPending)
	{
		bPending = false;
		StartRescue();
		return;
	}
	if (IsRescueUnderway())
	{
		TickRescue(DeltaTime);
	}
}

void UPlayerRescueSubsystem::StartRescue()
{
	ASpaceship* Ship = Wreck.Get();
	APlayerController* PC = PlayerController.Get();
	if (!Ship || !PC)
	{
		Reset();
		return;
	}

	ASpaceStation* Home = FindNearestFriendlyStation(Ship->GetActorLocation());
	ADockingBayModule* Bay = Home ? Home->GetDockingBayModule() : nullptr;
	if (!Bay)
	{
		FallbackClock = 0.0f;
		Message(FString::Printf(TEXT("SHIP DISABLED - no friendly station in range. Emergency repair in %.0f s"), PlayerRescue::FallbackRepairSeconds), PlayerRescue::FallbackRepairSeconds, true);
		return;
	}
	Station = Home;
	const FString StationName = AAIPilotController::GetStationDisplayName(Home);

	UWorld* World = GetWorld();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Tow drone out of the bay; it leaves the wreck just outside, on the side it comes from.
	const FVector BayLoc = Bay->GetActorLocation();
	const FVector Out = (Ship->GetActorLocation() - BayLoc).GetSafeNormal();
	const FVector DropPoint = BayLoc + Out * PlayerRescue::DropDistance;
	if (ATowDrone* Drone = World->SpawnActor<ATowDrone>(ATowDrone::StaticClass(), BayLoc + Out * 600.0f, Out.Rotation(), Params))
	{
		Drone->StartTow(Ship, DropPoint, BayLoc);
		Tow = Drone;
	}

	// Eject: the pilot takes the pod (and the camera with it).
	const FBox Hull = Ship->GetHullLocalBounds();
	const float HullHeight = Hull.IsValid ? Hull.GetExtent().Z : 200.0f;
	const FVector Up = Ship->GetActorUpVector();
	AEscapePod* EscapePod = World->SpawnActor<AEscapePod>(AEscapePod::StaticClass(),
		Ship->GetActorLocation() + Up * (HullHeight + 250.0f), Ship->GetActorRotation(), Params);
	if (!EscapePod)
	{
		Reset();
		return;
	}
	Pod = EscapePod;
	PC->Possess(EscapePod);
	EscapePod->Launch(Ship->GetVelocity(), Up);

	// Pickup: a trader if one is nearer than the station, else the station itself.
	const float StationDist = FVector::Dist(Ship->GetActorLocation(), BayLoc);
	if (ASpaceship* Trader = FindPickupShip(Ship->GetActorLocation(), StationDist))
	{
		PickupShip = Trader;
		EscapePod->SetDestination(Trader, Trader->GetHullHalfLength() + PlayerRescue::ShipArrivalMargin);
		Message(FString::Printf(TEXT("SHIP DISABLED - ejecting. %s is picking you up; tow drone dispatched from %s"),
			*Trader->GetShipName().ToString(), *StationName), 6.0f, true);
	}
	else
	{
		EscapePod->SetDestination(Bay, PlayerRescue::StationArrival);
		Message(FString::Printf(TEXT("SHIP DISABLED - ejecting to %s; tow drone on its way"), *StationName), 6.0f, true);
	}
}

void UPlayerRescueSubsystem::TickRescue(float DeltaTime)
{
	ASpaceship* Ship = Wreck.Get();
	APlayerController* PC = PlayerController.Get();

	// No station anywhere: patch the ship up where it is.
	if (FallbackClock >= 0.0f)
	{
		FallbackClock += DeltaTime;
		if (FallbackClock >= PlayerRescue::FallbackRepairSeconds)
		{
			if (Ship && Ship->HealthComponent)
			{
				Ship->HealthComponent->Restore();
			}
			Message(TEXT("Emergency repairs complete"), 2.0f);
			Reset();
		}
		return;
	}

	ASpaceStation* Home = Station.Get();
	AEscapePod* EscapePod = Pod.Get();
	ADockingBayModule* Bay = Home ? Home->GetDockingBayModule() : nullptr;
	if (!Ship || !PC || !EscapePod || !Bay)
	{
		UE_LOG(LogAdastreaCombat, Warning, TEXT("Rescue: lost track (%s); giving up"), *GetStatus());
		Reset();
		return;
	}
	const FString StationName = AAIPilotController::GetStationDisplayName(Home);

	// Riding with a trader: once aboard, have it ferry us to the tow's station.
	if (ASpaceship* Trader = PickupShip.Get(); Trader && !bPilotAtStation)
	{
		AAIPilotController* Pilot = Cast<AAIPilotController>(Trader->GetController());
		if (Trader->IsWrecked() || !Pilot)
		{
			// The ride was lost: the pod heads for the station on its own.
			PickupShip = nullptr;
			bFerryOrdered = false;
			EscapePod->SetDestination(Bay, PlayerRescue::StationArrival);
			PC->SetViewTargetWithBlend(EscapePod, PlayerRescue::CameraBlendSeconds);
			Message(TEXT("Your ride was disabled - the pod is heading for the station"), 4.0f, true);
		}
		else if (EscapePod->HasArrived() && !bFerryOrdered)
		{
			bFerryOrdered = Pilot->FerryTo(Home);
			PC->SetViewTargetWithBlend(Trader, PlayerRescue::CameraBlendSeconds);
			Message(FString::Printf(TEXT("Aboard %s - heading to %s"), *Trader->GetShipName().ToString(), *StationName), 5.0f);
		}
		else if (bFerryOrdered && Trader->IsDocked() && Trader->GetDockedStation() == Home)
		{
			bPilotAtStation = true;
		}
	}
	else if (!PickupShip.IsValid() && EscapePod->HasArrived() && EscapePod->GetDestination() == Bay)
	{
		bPilotAtStation = true;
	}

	// At the station: watch the tow bring the ship in.
	if (bPilotAtStation && !bWatchingTow)
	{
		bWatchingTow = true;
		EscapePod->SetDestination(Bay, PlayerRescue::StationArrival);
		FActorSpawnParameters CamParams;
		CamParams.ObjectFlags |= RF_Transient;
		WatchCamera = GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), GetTowShot(Ship, Bay), CamParams);
		if (WatchCamera)
		{
			WatchCamera->GetCameraComponent()->SetConstraintAspectRatio(false);
			PC->SetViewTargetWithBlend(WatchCamera, PlayerRescue::CameraBlendSeconds);
		}
		Message(FString::Printf(TEXT("Safe aboard %s - waiting for the tow"), *StationName), 5.0f);
	}

	// Keep the tow shot framed as the wreck comes in.
	if (WatchCamera)
	{
		const FTransform Shot = GetTowShot(Ship, Bay);
		WatchCamera->SetActorLocationAndRotation(
			FMath::VInterpTo(WatchCamera->GetActorLocation(), Shot.GetLocation(), DeltaTime, 2.0f),
			FMath::RInterpTo(WatchCamera->GetActorRotation(), Shot.Rotator(), DeltaTime, 2.0f));
	}

	ATowDrone* Drone = Tow.Get();
	if (!Drone)
	{
		UE_LOG(LogAdastreaCombat, Warning, TEXT("Rescue: tow drone lost; repairing in place"));
		FallbackClock = 0.0f;
		return;
	}
	if (bPilotAtStation && Drone->HasDelivered())
	{
		Reunite();
	}
}

FTransform UPlayerRescueSubsystem::GetTowShot(const ASpaceship* Ship, const AActor* Bay)
{
	// Off to the side of the tow path and a little above, looking past the wreck at the station.
	const FVector WreckLoc = Ship->GetActorLocation();
	const FVector Path = (Bay->GetActorLocation() - WreckLoc).GetSafeNormal();
	FVector Side = FVector::CrossProduct(Path, FVector::UpVector).GetSafeNormal();
	if (Side.IsNearlyZero())
	{
		Side = FVector::RightVector;
	}
	const FVector Eye = WreckLoc + Side * PlayerRescue::TowShotSide + FVector::UpVector * PlayerRescue::TowShotUp - Path * PlayerRescue::TowShotBack;
	const FVector Focus = FMath::Lerp(WreckLoc, Bay->GetActorLocation(), 0.3f);
	return FTransform((Focus - Eye).Rotation(), Eye);
}

void UPlayerRescueSubsystem::Reunite()
{
	ASpaceship* Ship = Wreck.Get();
	APlayerController* PC = PlayerController.Get();
	ASpaceStation* Home = Station.Get();
	ADockingBayModule* Bay = Home ? Home->GetDockingBayModule() : nullptr;

	if (ATowDrone* Drone = Tow.Get())
	{
		Drone->Release();
	}
	if (Ship->HealthComponent)
	{
		Ship->HealthComponent->Restore();   // free repair; clears the wreck state
	}
	PC->Possess(Ship);
	if (AEscapePod* EscapePod = Pod.Get())
	{
		EscapePod->Destroy();
	}
	if (WatchCamera)
	{
		WatchCamera->Destroy();
		WatchCamera = nullptr;
	}
	if (Bay)
	{
		Ship->SetNearbyStation(Bay);
		Ship->RequestDocking();
	}
	Message(FString::Printf(TEXT("%s repaired your ship free of charge - docking"), *AAIPilotController::GetStationDisplayName(Home)), 5.0f);
	Reset();
}

void UPlayerRescueSubsystem::Reset()
{
	Wreck = nullptr;
	PlayerController = nullptr;
	Station = nullptr;
	Tow = nullptr;
	Pod = nullptr;
	PickupShip = nullptr;
	bPending = false;
	bFerryOrdered = false;
	bPilotAtStation = false;
	bWatchingTow = false;
	FallbackClock = -1.0f;
	if (WatchCamera)
	{
		WatchCamera->Destroy();
	}
	WatchCamera = nullptr;
}

FString UPlayerRescueSubsystem::GetStatus() const
{
	if (!IsRescueUnderway())
	{
		return TEXT("idle");
	}
	if (FallbackClock >= 0.0f)
	{
		return FString::Printf(TEXT("emergency repair %.1fs"), FallbackClock);
	}
	const ATowDrone* Drone = Tow.Get();
	const AEscapePod* EscapePod = Pod.Get();
	return FString::Printf(TEXT("tow=%s pod=%s pickup=%s ferry=%d atStation=%d"),
		Drone ? *UEnum::GetValueAsString(Drone->GetTowState()) : TEXT("none"),
		EscapePod ? *UEnum::GetValueAsString(EscapePod->GetPodState()) : TEXT("none"),
		*GetNameSafe(PickupShip.Get()), bFerryOrdered, bPilotAtStation);
}
