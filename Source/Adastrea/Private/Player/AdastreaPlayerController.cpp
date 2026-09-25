// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/AdastreaPlayerController.h"
#include "Trading/CargoComponent.h"
#include "Ships/Spaceship.h"
#include "Ships/SpaceshipAvatar.h"
#include "Ships/SpaceshipInterior.h"
#include "Ships/SpaceshipDataAsset.h"
#include "Stations/SpaceStation.h"
#include "Mining/Asteroid.h"
#include "EngineUtils.h"
#include "AdastreaHUD.h"
#include "Stations/SpaceStationModule.h"
#include "Stations/DockingBayModule.h"
#include "Stations/MarketplaceModule.h"
#include "Trading/MarketDataAsset.h"
#include "Trading/PlayerTraderComponent.h"
#include "Trading/TradeItemDataAsset.h"
#include "AdastreaLog.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "UI/AdastreaHUDWidget.h"
#include "UI/ShipStatusWidget.h"
#include "UI/InventoryWidget.h"
#include "UI/InventoryComponent.h"
#include "UI/TradingInterfaceWidget.h"
#include "UI/StationManagementWidget.h"
// REMOVED: #include "Factions/FactionDataAsset.h" - faction system removed per Trade Simulator MVP
// REMOVED: #include "Interfaces/IFactionMember.h" - faction system removed per Trade Simulator MVP
#include "Stations/StationInterior.h"
#include "Player/WorldInteractable.h"
#include "TimerManager.h"

AAdastreaPlayerController::AAdastreaPlayerController()
{
	// Set default values
	// StationEditorWidgetClass is resolved lazily in CreateStationEditorWidget()
	// instead of here - a ConstructorHelpers::FClassFinder load in the constructor
	// runs during CDO construction, very early in engine startup, before there's
	// any guarantee the StationEditor module (which WBP_StationEditor's parent
	// class UStationEditorWidgetCpp lives in) has been loaded. Adastrea doesn't
	// module-depend on StationEditor by design (see the circular-dependency fix
	// doc), so that's a real race, not a hypothetical one: it crashed the editor
	// on startup (EXCEPTION_ACCESS_VIOLATION chasing a null class) the one time
	// it lost. Left null here; resolved on first actual use instead.
	StationEditorWidgetClass = nullptr;
	ModuleCatalog = nullptr;
	StationSearchRadius = 5000.0f;
	DefaultStationClass = ASpaceStation::StaticClass();
	NewStationSpawnDistance = 3000.0f;
	TradingInteractionRadius = 8000.0f;
	StationCheckInterval = 0.5f;
	StationEditorWidget = nullptr;
	bIsStationEditorOpen = false;
	HUDWidgetClass = nullptr;
	HUDWidget = nullptr;
	ShipStatusWidgetClass = nullptr;
	ShipStatusWidget = nullptr;
	bIsShipStatusOpen = false;
	MainMenuWidgetClass = nullptr;
	MainMenuWidget = nullptr;
	bIsMainMenuOpen = false;
	InventoryWidgetClass = nullptr;
	InventoryWidget = nullptr;
	bIsInventoryOpen = false;
	TradingWidgetClass = nullptr;
	TradingWidget = nullptr;
	bIsTradingOpen = false;
	StationManagementWidgetClass = nullptr;
	StationManagementWidget = nullptr;
	bIsStationManagementOpen = false;
	NearbyTradableStation = nullptr;
	bWasNearTradableStation = false;
	bTargetingModeActive = false;
	HoveredTargetActor = nullptr;
	LockedTargetActor = nullptr;
	bLockMouseLook = false;
}

void AAdastreaPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Input mapping contexts are now configured by the GameMode through DA_InputConfig
	// This ensures centralized input configuration and prevents conflicts

	// Create HUD widget. Prefer the configured Blueprint class (designer HUD) if
	// set; otherwise instantiate the base UAdastreaHUDWidget, which self-builds a
	// runtime text panel (coords/speed/throttle/credits/cargo) so we always have
	// a single, visible HUD.
	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UAdastreaHUDWidget>(this, HUDWidgetClass);
	}
	else
	{
		HUDWidget = CreateWidget<UAdastreaHUDWidget>(this, UAdastreaHUDWidget::StaticClass());
	}
	if (HUDWidget)
	{
		HUDWidget->AddToViewport(0);
		HUDWidget->InitializeHUD();
		UE_LOG(LogAdastrea, Log, TEXT("AdastreaPlayerController: Created and initialized HUD widget"));
	}
	else
	{
		UE_LOG(LogAdastrea, Warning, TEXT("AdastreaPlayerController: Failed to create HUD widget"));
	}

	// Start timer to check for nearby tradable stations
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(
			StationCheckTimerHandle,
			this,
			&AAdastreaPlayerController::CheckForNearbyTradableStations,
			StationCheckInterval,
			true  // Loop
		);
		UE_LOG(LogAdastrea, Log, TEXT("AdastreaPlayerController: Started nearby station check timer"));
	}
}

void AAdastreaPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateSpaceBackdrop();
}

void AAdastreaPlayerController::UpdateSpaceBackdrop()
{
	// The star-dome backdrop is a huge but fixed-in-world-space mesh; if the
	// camera travels any appreciable fraction of its radius (routine between
	// sectors), nearer parts of the star pattern visibly shift against farther
	// ones -- background stars should read as infinitely far away, with zero
	// positional parallax, regardless of how far the ship flies. Recentering it
	// on the camera every frame (position only, no rotation) gives that for
	// free without needing an enormous dome or a real skybox/cubemap.
	if (!bSpaceBackdropSearchDone)
	{
		bSpaceBackdropSearchDone = true;
		TArray<AActor*> Found;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("SpaceBackdrop"), Found);
		if (Found.Num() > 0)
		{
			SpaceBackdropActor = Found[0];
		}
		else
		{
			UE_LOG(LogAdastrea, Warning, TEXT("UpdateSpaceBackdrop: no actor tagged 'SpaceBackdrop' found; star dome will not follow the camera."));
		}
	}

	if (!SpaceBackdropActor)
	{
		return;
	}

	FVector CameraLocation = FVector::ZeroVector;
	if (PlayerCameraManager)
	{
		CameraLocation = PlayerCameraManager->GetCameraLocation();
	}
	else if (APawn* ControlledPawn = GetPawn())
	{
		CameraLocation = ControlledPawn->GetActorLocation();
	}
	SpaceBackdropActor->SetActorLocation(CameraLocation);
}

void AAdastreaPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Input mapping contexts and bindings are configured by the GameMode through DA_InputConfig
	// This ensures centralized input configuration and prevents conflicts between systems
	//
	// Targeting toggle (Tab) + click-to-target are bound directly for reliability.
	if (InputComponent)
	{
		// Tab: toggle targeting mode (cursor shown, click selects/locks a station)
		InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &AAdastreaPlayerController::HandleTargetingToggle);
		// LMB: in targeting mode, click a station/ship/asteroid to lock it as the target / map click
		InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AAdastreaPlayerController::HandleTargetClick);
		// ] / [: cycle next/previous target (nearest-first), Y: lock nearest, Z: clear lock
		InputComponent->BindKey(EKeys::RightBracket, IE_Pressed, this, &AAdastreaPlayerController::HandleNextTarget);
		InputComponent->BindKey(EKeys::LeftBracket, IE_Pressed, this, &AAdastreaPlayerController::HandlePreviousTarget);
		InputComponent->BindKey(EKeys::Y, IE_Pressed, this, &AAdastreaPlayerController::HandleNearestTarget);
		InputComponent->BindKey(EKeys::Z, IE_Pressed, this, &AAdastreaPlayerController::HandleClearTarget);
		// M: toggle the full-screen sector map
		InputComponent->BindKey(EKeys::M, IE_Pressed, this, &AAdastreaPlayerController::HandleMapToggle);
		// Map navigation (only acted on when the map is open)
		InputComponent->BindKey(EKeys::Up,    IE_Repeat, this, &AAdastreaPlayerController::HandleMapOrbitUp);
		InputComponent->BindKey(EKeys::Down,  IE_Repeat, this, &AAdastreaPlayerController::HandleMapOrbitDown);
		InputComponent->BindKey(EKeys::Left,  IE_Repeat, this, &AAdastreaPlayerController::HandleMapOrbitLeft);
		InputComponent->BindKey(EKeys::Right, IE_Repeat, this, &AAdastreaPlayerController::HandleMapOrbitRight);
		InputComponent->BindKey(EKeys::Equals, IE_Pressed, this, &AAdastreaPlayerController::HandleMapZoomIn);
		InputComponent->BindKey(EKeys::Hyphen, IE_Pressed, this, &AAdastreaPlayerController::HandleMapZoomOut);
		InputComponent->BindKey(EKeys::C, IE_Pressed, this, &AAdastreaPlayerController::HandleMapCenter);
		InputComponent->BindKey(EKeys::One, IE_Pressed, this, &AAdastreaPlayerController::HandleMapToggleShips);
		InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AAdastreaPlayerController::HandleMapToggleStations);
		// Trade screen input (only acted on when the trade screen is open)
		InputComponent->BindKey(EKeys::Up, IE_Pressed, this, &AAdastreaPlayerController::HandleTradeSelectUp);
		InputComponent->BindKey(EKeys::Down, IE_Pressed, this, &AAdastreaPlayerController::HandleTradeSelectDown);
		InputComponent->BindKey(EKeys::B, IE_Pressed, this, &AAdastreaPlayerController::HandleTradeToggleMode);
		InputComponent->BindKey(EKeys::S, IE_Pressed, this, &AAdastreaPlayerController::HandleTradeToggleMode);
		InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &AAdastreaPlayerController::HandleTradeExecute1);
		InputComponent->BindKey(EKeys::Q, IE_Pressed, this, &AAdastreaPlayerController::HandleTradeExecute5);
		InputComponent->BindKey(EKeys::X, IE_Pressed, this, &AAdastreaPlayerController::HandleTradeSellAll);
		InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AAdastreaPlayerController::HandleTradeClose);
		InputComponent->BindKey(EKeys::BackSpace, IE_Pressed, this, &AAdastreaPlayerController::HandleTradeClose);
		InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &AAdastreaPlayerController::HandleStationMenuConfirm);
		// V: toggle between flying the ship (cockpit) and walking the interior (avatar)
		InputComponent->BindKey(EKeys::V, IE_Pressed, this, &AAdastreaPlayerController::HandleToggleInterior);
		// Ship-select screen input (only acted on when the ship-select screen is open)
		InputComponent->BindKey(EKeys::Left, IE_Pressed, this, &AAdastreaPlayerController::HandleShipSelectRotateL);
		InputComponent->BindKey(EKeys::Right, IE_Pressed, this, &AAdastreaPlayerController::HandleShipSelectRotateR);
		InputComponent->BindKey(EKeys::Up, IE_Pressed, this, &AAdastreaPlayerController::HandleShipSelectPrev);
		InputComponent->BindKey(EKeys::Down, IE_Pressed, this, &AAdastreaPlayerController::HandleShipSelectNext);
		InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &AAdastreaPlayerController::HandleShipSelectFly);
		InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AAdastreaPlayerController::HandleShipSelectClose);
		// P: open the ship-select screen
		InputComponent->BindKey(EKeys::P, IE_Pressed, this, &AAdastreaPlayerController::HandleShipSelectOpen);
		InputComponent->BindKey(EKeys::N, IE_Pressed, this, &AAdastreaPlayerController::HandleStationInfoToggle);
		// G: open/close the full Station Editor (module placement, undo/redo, construction
		// queue). Was previously unreachable - IA_OpenStationEditor/StationEditorAction were
		// declared but never bound to ToggleStationEditor(). G is unused by any other system.
		InputComponent->BindKey(EKeys::G, IE_Pressed, this, &AAdastreaPlayerController::ToggleStationEditor);
	}
}

void AAdastreaPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Explicitly clear the station check timer to avoid dangling callbacks and keep timer usage maintainable
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StationCheckTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void AAdastreaPlayerController::OnPossessSpaceship_Implementation(ASpaceship* NewSpaceship)
{
	// Default implementation
	// Can be overridden in Blueprints
	if (NewSpaceship)
	{
		UE_LOG(LogAdastrea, Log, TEXT("Player controller possessing spaceship: %s"), *NewSpaceship->GetName());
	}
}

void AAdastreaPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Capture the mouse so Enhanced Input mouse-look (Mouse2D) works.
	// Without GameOnly input mode the cursor stays visible and mouse movement
	// rotates nothing.
	if (InPawn)
	{
		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
		bEnableClickEvents = false;
		bEnableMouseOverEvents = false;
		UE_LOG(LogAdastrea, Log, TEXT("AdastreaPlayerController: Captured mouse for pawn %s"), *InPawn->GetName());
	}
}

void AAdastreaPlayerController::OnUnPossess()
{
	// Release mouse capture when leaving the pawn
	Super::OnUnPossess();
	SetInputMode(FInputModeGameAndUI());
	bShowMouseCursor = true;
}

ASpaceship* AAdastreaPlayerController::GetControlledSpaceship() const
{
	return Cast<ASpaceship>(GetPawn());
}

bool AAdastreaPlayerController::IsControllingSpaceship() const
{
	return GetControlledSpaceship() != nullptr;
}

void AAdastreaPlayerController::ToggleTargetingMode()
{
	bTargetingModeActive = !bTargetingModeActive;

	if (bTargetingModeActive)
	{
		// Show the cursor so the player can select/lock a station. Ship mouse-look
		// is paused while the cursor is shown (GameAndUI mode keeps keyboard flight).
		SetInputMode(FInputModeGameAndUI());
		bShowMouseCursor = true;
		bEnableClickEvents = true;
		bEnableMouseOverEvents = true;
		bLockMouseLook = true; // pause ship mouse-look (checked by ship's Tick)
		UE_LOG(LogAdastrea, Log, TEXT("Targeting mode ON - click a station, ship or asteroid to lock it (Tab to exit)"));
	}
	else
	{
		// Return to ship-control mouse-look.
		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
		bEnableClickEvents = false;
		bEnableMouseOverEvents = false;
		bLockMouseLook = false;
		UE_LOG(LogAdastrea, Log, TEXT("Targeting mode OFF - mouse controls ship look again"));
	}
}

void AAdastreaPlayerController::ClearTarget()
{
	LockedTargetActor = nullptr;
}

void AAdastreaPlayerController::HandleTargetingToggle()
{
	if (!IsControllingSpaceship())
	{
		return;
	}
	ToggleTargetingMode();
}

namespace AdastreaTargeting
{
	/** Stations and ships further than this can't be targeted. */
	constexpr float MaxTargetRange = 200000.0f;
	/** Asteroids are only cycled through within this range, so a dense field doesn't bury ships/stations. */
	constexpr float MaxAsteroidCycleRange = 30000.0f;

	struct FTargetCandidate
	{
		AActor* Actor;
		FVector Center;       // where the reticle/pick sphere is centred
		float SelectRadius;   // clickable sphere radius
		float Distance;       // from the player's ship
	};
}

/**
 * Every targetable actor in range: stations, other ships, and asteroids that still
 * hold ore. Sorted nearest-first.
 */
static TArray<AdastreaTargeting::FTargetCandidate> GatherAdastreaTargetCandidates(UWorld* World, const APawn* OwnPawn, const FVector& FromLoc)
{
	using namespace AdastreaTargeting;
	TArray<FTargetCandidate> Out;
	if (!World)
	{
		return Out;
	}

	auto Add = [&](AActor* Actor, const FVector& Center, float SelectRadius)
	{
		const float D = FVector::Dist(FromLoc, Center);
		if (D <= MaxTargetRange)
		{
			Out.Add({ Actor, Center, SelectRadius, D });
		}
	};

	// Stations: generous fixed clickable sphere.
	for (TActorIterator<ASpaceStation> It(World); It; ++It)
	{
		Add(*It, It->GetActorLocation(), 15000.0f);
	}

	// Ships (other than our own): sphere from the actor bounds, with a floor so
	// small fighters are still clickable at range.
	for (TActorIterator<ASpaceship> It(World); It; ++It)
	{
		ASpaceship* Ship = *It;
		if (Ship == OwnPawn)
		{
			continue;
		}
		FVector Origin, Extent;
		Ship->GetActorBounds(true, Origin, Extent);
		Add(Ship, Origin, FMath::Max(Extent.Size(), 2000.0f));
	}

	// Asteroids that still hold ore.
	for (TActorIterator<AAsteroid> It(World); It; ++It)
	{
		AAsteroid* Rock = *It;
		if (!ITargetable::Execute_CanBeTargeted(Rock))
		{
			continue;
		}
		Add(Rock, Rock->GetActorLocation(), FMath::Max(Rock->GetRadius() * 1.2f, 1000.0f));
	}

	Out.Sort([](const FTargetCandidate& A, const FTargetCandidate& B) { return A.Distance < B.Distance; });
	return Out;
}

/**
 * Screen-space ray cast at the cursor to find a targetable actor under it
 * (station, ship or asteroid). Each candidate is treated as a clickable sphere;
 * if several are under the cursor, the one nearest the player's ship wins.
 */
AActor* AAdastreaPlayerController::GetTargetUnderCursor()
{
	// Project the cursor screen position to a world ray.
	FVector WorldOrigin;
	FVector WorldDirection;
	if (!DeprojectMousePositionToWorld(WorldOrigin, WorldDirection))
	{
		return nullptr;
	}

	APawn* OwnPawn = GetPawn();
	const FVector ShipLoc = OwnPawn ? OwnPawn->GetActorLocation() : WorldOrigin;

	// Candidates are nearest-first, so the first one the ray passes through wins.
	for (const AdastreaTargeting::FTargetCandidate& C : GatherAdastreaTargetCandidates(GetWorld(), OwnPawn, ShipLoc))
	{
		// Closest point on the cursor ray to the candidate's centre.
		const float Proj = FVector::DotProduct(C.Center - WorldOrigin, WorldDirection);
		if (Proj <= 0.0f)
		{
			continue; // behind the camera
		}
		const FVector Closest = WorldOrigin + WorldDirection * Proj;
		if (FVector::DistSquared(Closest, C.Center) < FMath::Square(C.SelectRadius))
		{
			return C.Actor;
		}
	}
	return nullptr;
}

/** Targets reachable by the cycle keys, nearest-first (asteroids limited to mining range). */
TArray<AActor*> AAdastreaPlayerController::GetCycleTargets() const
{
	TArray<AActor*> Out;
	APawn* OwnPawn = GetPawn();
	if (!OwnPawn)
	{
		return Out;
	}
	for (const AdastreaTargeting::FTargetCandidate& C : GatherAdastreaTargetCandidates(GetWorld(), OwnPawn, OwnPawn->GetActorLocation()))
	{
		if (C.Actor->IsA<AAsteroid>() && C.Distance > AdastreaTargeting::MaxAsteroidCycleRange)
		{
			continue;
		}
		Out.Add(C.Actor);
	}
	return Out;
}

void AAdastreaPlayerController::CycleTarget(int32 Direction)
{
	if (!IsControllingSpaceship())
	{
		return;
	}

	const TArray<AActor*> Targets = GetCycleTargets();
	if (Targets.Num() == 0)
	{
		ShowHUDMessage(TEXT("No targets in range"), 2.0f, true);
		return;
	}

	// Step from the current target through the nearest-first list, wrapping.
	// With nothing locked (or the lock out of range), Next starts at the nearest
	// and Previous at the farthest.
	const int32 Current = Targets.IndexOfByKey(GetLockedTarget());
	int32 NewIndex;
	if (Current == INDEX_NONE)
	{
		NewIndex = Direction >= 0 ? 0 : Targets.Num() - 1;
	}
	else
	{
		NewIndex = (Current + Direction + Targets.Num()) % Targets.Num();
	}

	LockedTargetActor = Targets[NewIndex];
	UE_LOG(LogAdastrea, Log, TEXT("TARGET CYCLED (%d/%d): %s"), NewIndex + 1, Targets.Num(), *LockedTargetActor->GetName());
}

void AAdastreaPlayerController::HandleNextTarget()
{
	CycleTarget(+1);
}

void AAdastreaPlayerController::HandlePreviousTarget()
{
	CycleTarget(-1);
}

void AAdastreaPlayerController::HandleNearestTarget()
{
	if (!IsControllingSpaceship())
	{
		return;
	}
	const TArray<AActor*> Targets = GetCycleTargets();
	if (Targets.Num() == 0)
	{
		ShowHUDMessage(TEXT("No targets in range"), 2.0f, true);
		return;
	}
	LockedTargetActor = Targets[0];
	UE_LOG(LogAdastrea, Log, TEXT("NEAREST TARGET: %s"), *LockedTargetActor->GetName());
}

void AAdastreaPlayerController::HandleClearTarget()
{
	if (IsControllingSpaceship())
	{
		ClearTarget();
	}
}

void AAdastreaPlayerController::HandleTargetClick()
{
	// If the map is open, interpret the click on the map (convert pixel -> world point,
	// pick nearest station to that point). Otherwise use normal targeting cursor picking.
	if (AAdastreaHUD* GameHUD = Cast<AAdastreaHUD>(GetHUD()))
	{
		if (GameHUD->bShowMap)
		{
			HandleMapClick();
			return;
		}
	}

	if (!bTargetingModeActive)
	{
		return;
	}
	if (AActor* Target = GetTargetUnderCursor())
	{
		LockedTargetActor = Target;
		UE_LOG(LogAdastrea, Log, TEXT("TARGET LOCKED: %s"), *Target->GetName());
	}
}

void AAdastreaPlayerController::HandleMapClick()
{
	// Convert the cursor screen position into a world (X,Y) point using the same
	// map projection as DrawSectorMap, then target the nearest station to it.
	if (!GetWorld())
	{
		return;
	}
	FVector2D MousePos;
	if (!GetMousePosition(MousePos.X, MousePos.Y))
	{
		return;
	}
	int32 VX = 0, VY = 0;
	GetViewportSize(VX, VY);
	float VW = (float)VX, VH = (float)VY;

	// Recompute map bounds (match DrawSectorMap).
	APawn* ControlledPawn = GetPawn();
	FVector ShipPos = ControlledPawn ? ControlledPawn->GetActorLocation() : FVector::ZeroVector;
	float MinX = ShipPos.X, MaxX = ShipPos.X, MinY = ShipPos.Y, MaxY = ShipPos.Y;
	TArray<AActor*> Stations;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpaceStation::StaticClass(), Stations);
	for (AActor* S : Stations)
	{
		if (!S) continue;
		FVector L = S->GetActorLocation();
		MinX = FMath::Min(MinX, L.X); MaxX = FMath::Max(MaxX, L.X);
		MinY = FMath::Min(MinY, L.Y); MaxY = FMath::Max(MaxY, L.Y);
	}
	float ExtentX = FMath::Max(MaxX - MinX, 20000.0f);
	float ExtentY = FMath::Max(MaxY - MinY, 20000.0f);
	float Cx = (MinX + MaxX) * 0.5f, Cy = (MinY + MaxY) * 0.5f;
	MinX = Cx - ExtentX * 0.5f; MaxX = Cx + ExtentX * 0.5f;
	MinY = Cy - ExtentY * 0.5f; MaxY = Cy + ExtentY * 0.5f;

	const float Margin = 20.0f;
	float MapW = VW - Margin * 2.0f;
	float MapH = VH - Margin * 2.0f - 40.0f;
	float WA = ExtentX / FMath::Max(ExtentY, 1.0f);
	float MA = MapW / FMath::Max(MapH, 1.0f);
	float Scale;
	if (WA > MA) { Scale = MapW / FMath::Max(ExtentX, 1.0f); MapH = ExtentY * Scale; }
	else         { Scale = MapH / FMath::Max(ExtentY, 1.0f); MapW = ExtentX * Scale; }
	float PX = Margin + (VW - Margin * 2.0f - MapW) * 0.5f;
	float PY = Margin + (VH - Margin * 2.0f - MapH) * 0.5f;

	// Inverse map: screen -> world.
	// sx = PX + (W.X - MinX)*Scale  => W.X = MinX + (sx - PX)/Scale
	// sy = PY + (MaxY - W.Y)*Scale  => W.Y = MaxY - (sy - PY)/Scale
	FVector ClickWorld((float)(MinX + (MousePos.X - PX) / Scale), (float)(MaxY - (MousePos.Y - PY) / Scale), 0.0f);

	// Pick nearest station to the clicked world point.
	ASpaceStation* Best = nullptr;
	float BestDist = TNumericLimits<float>::Max();
	for (AActor* S : Stations)
	{
		if (!S) continue;
		FVector L = S->GetActorLocation();
		float D = FVector::Dist(ClickWorld, L);
		if (D < BestDist) { BestDist = D; Best = Cast<ASpaceStation>(S); }
	}
	if (Best)
	{
		LockedTargetActor = Best;
		UE_LOG(LogAdastrea, Log, TEXT("MAP TARGET LOCKED: %s"), *Best->GetName());
	}
}

void AAdastreaPlayerController::HandleTradeSelectUp()
{
	if (AAdastreaHUD* H = Cast<AAdastreaHUD>(GetHUD()))
	{
		if (H->bShowStationMenu) { H->MoveStationMenuSelection(-1); }
		else if (H->bShowTradeScreen) { H->MoveTradeSelection(-1); }
	}
}

void AAdastreaPlayerController::HandleTradeSelectDown()
{
	if (AAdastreaHUD* H = Cast<AAdastreaHUD>(GetHUD()))
	{
		if (H->bShowStationMenu) { H->MoveStationMenuSelection(1); }
		else if (H->bShowTradeScreen) { H->MoveTradeSelection(1); }
	}
}

void AAdastreaPlayerController::HandleTradeToggleMode()
{
	if (AAdastreaHUD* H = Cast<AAdastreaHUD>(GetHUD()))
	{
		if (H->bShowTradeScreen) { H->ToggleBuySellMode(); }
	}
}

void AAdastreaPlayerController::HandleTradeExecute1()   { ExecuteTrade(1); }
void AAdastreaPlayerController::HandleTradeExecute5()   { ExecuteTrade(5); }

void AAdastreaPlayerController::HandleTradeSellAll()
{
	AAdastreaHUD* H = Cast<AAdastreaHUD>(GetHUD());
	ASpaceship* Ship = GetControlledSpaceship();
	ASpaceStation* Station = GetNearestTradableStation();
	if (!H || !H->bShowTradeScreen || H->bBuyMode || !Ship || !Ship->CargoComponent || !Station || !Station->GetMarketplaceModule())
	{
		return;
	}
	UMarketDataAsset* Market = Station->GetMarketplaceModule()->GetMarketData();
	if (!Market || !Market->Inventory.IsValidIndex(H->SelectedTradeIndex))
	{
		return;
	}
	const int32 Held = Ship->CargoComponent->GetItemQuantity(Market->Inventory[H->SelectedTradeIndex].TradeItem);
	if (Held > 0)
	{
		ExecuteTrade(Held);
	}
}

void AAdastreaPlayerController::ExecuteTrade(int32 Quantity)
{
	AAdastreaHUD* H = Cast<AAdastreaHUD>(GetHUD());
	if (!H || !H->bShowTradeScreen)
	{
		return;
	}
	ASpaceship* Ship = GetControlledSpaceship();
	if (!Ship || !Ship->PlayerTraderComponent || !Ship->CargoComponent)
	{
		return;
	}
	// Get the selected market item.
	ASpaceStation* Station = GetNearestTradableStation();
	if (!Station || !Station->GetMarketplaceModule())
	{
		return;
	}
	UMarketDataAsset* Market = Station->GetMarketplaceModule()->GetMarketData();
	if (!Market || !Market->Inventory.IsValidIndex(H->SelectedTradeIndex))
	{
		return;
	}
	const FMarketInventoryEntry& Entry = Market->Inventory[H->SelectedTradeIndex];
	UTradeItemDataAsset* Item = Entry.TradeItem;
	if (!Item)
	{
		return;
	}
	const bool bOK = H->bBuyMode
		? Ship->PlayerTraderComponent->BuyItem(Market, Item, Quantity, Ship->CargoComponent)
		: Ship->PlayerTraderComponent->SellItem(Market, Item, Quantity, Ship->CargoComponent);
	UE_LOG(LogAdastrea, Log, TEXT("Trade %s x%d %s (ok=%d)"),
		H->bBuyMode ? TEXT("BUY") : TEXT("SELL"), Quantity, *Item->GetName(), bOK);
}

void AAdastreaPlayerController::HandleStationMenuConfirm()
{
	if (AAdastreaHUD* H = Cast<AAdastreaHUD>(GetHUD()))
	{
		if (H->bShowStationMenu) { H->ConfirmStationMenuSelection(this); }
	}
}

void AAdastreaPlayerController::HandleTradeClose()
{
	if (AAdastreaHUD* H = Cast<AAdastreaHUD>(GetHUD()))
	{
		if (H->bShowStationMenu)
		{
			// Escape from the top-level station menu undocks.
			H->UndockFromStationMenu(this);
		}
		else if (H->bShowTradeScreen)
		{
			// Leaving the trading department returns to the station menu.
			H->HideTradeScreen();
			CloseTrading();
			H->ShowStationMenu();
		}
	}
}

void AAdastreaPlayerController::HandleShipSelectPrev()
{
	if (AAdastreaHUD* H = Cast<AAdastreaHUD>(GetHUD()))
	{
		if (H->bShowShipSelect) { H->CycleShipSelect(-1); }
	}
}

void AAdastreaPlayerController::HandleShipSelectNext()
{
	if (AAdastreaHUD* H = Cast<AAdastreaHUD>(GetHUD()))
	{
		if (H->bShowShipSelect) { H->CycleShipSelect(1); }
	}
}

void AAdastreaPlayerController::HandleShipSelectRotateL()
{
	if (AAdastreaHUD* H = Cast<AAdastreaHUD>(GetHUD()))
	{
		if (H->bShowShipSelect) { H->OrbitShipPreview(-8.0f, 0.0f); }
	}
}

void AAdastreaPlayerController::HandleShipSelectRotateR()
{
	if (AAdastreaHUD* H = Cast<AAdastreaHUD>(GetHUD()))
	{
		if (H->bShowShipSelect) { H->OrbitShipPreview(8.0f, 0.0f); }
	}
}

void AAdastreaPlayerController::HandleShipSelectFly()
{
	if (AAdastreaHUD* H = Cast<AAdastreaHUD>(GetHUD()))
	{
		if (H->bShowShipSelect) { H->SpawnSelectedShip(this); }
	}
}

void AAdastreaPlayerController::HandleShipSelectClose()
{
	if (AAdastreaHUD* H = Cast<AAdastreaHUD>(GetHUD()))
	{
		if (H->bShowShipSelect) { H->HideShipSelect(); }
	}
}

void AAdastreaPlayerController::HandleShipSelectOpen()
{
	if (AAdastreaHUD* H = Cast<AAdastreaHUD>(GetHUD()))
	{
		H->ShowShipSelect();
		// pause ship mouse-look so keys/cursor don't fight
		bLockMouseLook = true;
	}
}

void AAdastreaPlayerController::HandleMapOrbitUp()
{
	if (AAdastreaHUD* G = GetMapHUD()) { if (G->bShowMap) G->MapOrbit(0, -4.0f); }
}

void AAdastreaPlayerController::HandleMapOrbitDown()
{
	if (AAdastreaHUD* G = GetMapHUD()) { if (G->bShowMap) G->MapOrbit(0, 4.0f); }
}

void AAdastreaPlayerController::HandleMapOrbitLeft()
{
	if (AAdastreaHUD* G = GetMapHUD()) { if (G->bShowMap) G->MapOrbit(-4.0f, 0); }
}

void AAdastreaPlayerController::HandleMapOrbitRight()
{
	if (AAdastreaHUD* G = GetMapHUD()) { if (G->bShowMap) G->MapOrbit(4.0f, 0); }
}

void AAdastreaPlayerController::HandleMapZoomIn()
{
	if (AAdastreaHUD* GameHUD = GetMapHUD())
	{
		if (GameHUD->bShowMap) { GameHUD->MapZoomBy(-30000.0f); }
	}
}

void AAdastreaPlayerController::HandleMapZoomOut()
{
	if (AAdastreaHUD* GameHUD = GetMapHUD())
	{
		if (GameHUD->bShowMap) { GameHUD->MapZoomBy(30000.0f); }
	}
}

void AAdastreaPlayerController::HandleMapCenter()
{
	if (AAdastreaHUD* GameHUD = GetMapHUD())
	{
		if (!GameHUD->bShowMap) { return; }
		if (APawn* ControlledPawn = GetPawn())
		{
			GameHUD->MapRecenter(ControlledPawn->GetActorLocation());
		}
	}
}

void AAdastreaPlayerController::HandleMapToggleShips()
{
	if (AAdastreaHUD* GameHUD = GetMapHUD())
	{
		if (GameHUD->bShowMap) { GameHUD->bShowShips = !GameHUD->bShowShips; }
	}
}

void AAdastreaPlayerController::HandleMapToggleStations()
{
	if (AAdastreaHUD* GameHUD = GetMapHUD())
	{
		if (GameHUD->bShowMap) { GameHUD->bShowStations = !GameHUD->bShowStations; }
	}
}

AAdastreaHUD* AAdastreaPlayerController::GetMapHUD()
{
	return Cast<AAdastreaHUD>(GetHUD());
}

void AAdastreaPlayerController::HandleMapToggle()
{
	if (!IsControllingSpaceship())
	{
		return;
	}
	AAdastreaHUD* GameHUD = Cast<AAdastreaHUD>(GetHUD());
	if (!GameHUD)
	{
		return;
	}

	// Toggle the map.
	GameHUD->ToggleMap();
	const bool bMapOpen = GameHUD->bShowMap;

	// When the map is open, switch to mouse mode (cursor shown, ship mouse-look
	// paused) so the player can freely move the mouse over the map. On close,
	// return to ship mouse-look.
	if (bMapOpen)
	{
		SetInputMode(FInputModeGameAndUI());
		bShowMouseCursor = true;
		bEnableClickEvents = true;
		bEnableMouseOverEvents = true;
		bLockMouseLook = true; // pause ship mouse-look while map is open
		bTargetingModeActive = false; // keep separate from map (no hover reticle on map)
		UE_LOG(LogAdastrea, Log, TEXT("Map opened - mouse mode (ship look paused)"));
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
		bEnableClickEvents = false;
		bEnableMouseOverEvents = false;
		bLockMouseLook = false;
		UE_LOG(LogAdastrea, Log, TEXT("Map closed - ship look restored"));
	}
}

void AAdastreaPlayerController::HandleStationInfoToggle()
{
	AAdastreaHUD* GameHUD = Cast<AAdastreaHUD>(GetHUD());
	if (!GameHUD)
	{
		return;
	}

	// Toggle the station info screen.
	GameHUD->bShowStationInfo = !GameHUD->bShowStationInfo;
	const bool bOpen = GameHUD->bShowStationInfo;

	// Same input-mode dance as the map: pause ship mouse-look while the info
	// screen is open so keys/cursor don't fight flight controls.
	if (bOpen)
	{
		SetInputMode(FInputModeGameAndUI());
		bShowMouseCursor = true;
		bLockMouseLook = true;
		UE_LOG(LogAdastrea, Log, TEXT("Station info opened - ship look paused"));
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
		bLockMouseLook = false;
		UE_LOG(LogAdastrea, Log, TEXT("Station info closed - ship look restored"));
	}
}

void AAdastreaPlayerController::ToggleStationEditor()
{
	// Only allow station editor when controlling a spaceship
	if (!IsControllingSpaceship())
	{
		UE_LOG(LogAdastrea, Warning, TEXT("ToggleStationEditor: Not controlling a spaceship - station editor not available"));
		return;
	}

	// Toggle editor state
	if (bIsStationEditorOpen)
	{
		// Close the editor
		HideStationEditor();
		UE_LOG(LogAdastrea, Log, TEXT("ToggleStationEditor: Closed station editor"));

		// Broadcast the event for backward compatibility with existing Blueprints
		OnStationEditorToggle.Broadcast();
	}
	else
	{
		// Open the editor - edit the nearest existing station if there is one,
		// otherwise spawn a fresh empty station so the player can build from
		// scratch without needing to be near anything first.
		ASpaceStation* TargetStation = FindNearestStation();
		const bool bIsNewStation = (TargetStation == nullptr);
		if (bIsNewStation)
		{
			TargetStation = SpawnStationForBuilder();
		}

		if (!TargetStation)
		{
			UE_LOG(LogAdastrea, Warning, TEXT("ToggleStationEditor: No station nearby and failed to spawn a new one"));
			// Don't broadcast event on failure - no state change occurred
			return;
		}

		ShowStationEditor(TargetStation);
		UE_LOG(LogAdastrea, Log, TEXT("ToggleStationEditor: Opened station editor for %s station: %s"),
			bIsNewStation ? TEXT("new") : TEXT("existing"), *TargetStation->GetName());

		// Broadcast the event for backward compatibility with existing Blueprints
		OnStationEditorToggle.Broadcast();
	}
}

bool AAdastreaPlayerController::IsStationEditorOpen() const
{
	return bIsStationEditorOpen && StationEditorWidget && StationEditorWidget->IsInViewport();
}

UUserWidget* AAdastreaPlayerController::GetStationEditorWidget() const
{
	return StationEditorWidget;
}

ASpaceStation* AAdastreaPlayerController::FindNearestStation()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return nullptr;
	}

	FVector PlayerLocation = ControlledPawn->GetActorLocation();

	// Find all space stations in the world
	TArray<AActor*> FoundStations;
	UGameplayStatics::GetAllActorsOfClass(World, ASpaceStation::StaticClass(), FoundStations);

	ASpaceStation* NearestStation = nullptr;
	float NearestDistance = StationSearchRadius;

	// GetAllActorsOfClass returns actors of the specified class, so no Cast needed
	for (AActor* Actor : FoundStations)
	{
		ASpaceStation* Station = static_cast<ASpaceStation*>(Actor);
		float Distance = FVector::Dist(PlayerLocation, Station->GetActorLocation());
		if (Distance < NearestDistance)
		{
			NearestDistance = Distance;
			NearestStation = Station;
		}
	}

	return NearestStation;
}

ASpaceStation* AAdastreaPlayerController::SpawnStationForBuilder()
{
	UWorld* World = GetWorld();
	APawn* ControlledPawn = GetPawn();
	if (!World || !ControlledPawn)
	{
		return nullptr;
	}

	UClass* ClassToSpawn = DefaultStationClass ? *DefaultStationClass : ASpaceStation::StaticClass();

	// Drop it out in front of the player, axis-aligned (not player-facing) so
	// it matches the editor's world-axis-aligned build grid from the start.
	// Named StationSpawnLocation (not SpawnLocation) - APlayerController already
	// declares a SpawnLocation member and /we4458 makes shadowing it a hard error.
	const FVector StationSpawnLocation = ControlledPawn->GetActorLocation()
		+ ControlledPawn->GetActorForwardVector() * NewStationSpawnDistance;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ASpaceStation* NewStation = World->SpawnActor<ASpaceStation>(ClassToSpawn, StationSpawnLocation, FRotator::ZeroRotator, SpawnParams);

	if (NewStation)
	{
		UE_LOG(LogAdastrea, Log, TEXT("SpawnStationForBuilder: Spawned new empty station %s at (%.0f, %.0f, %.0f) for from-scratch building"),
			*NewStation->GetName(), StationSpawnLocation.X, StationSpawnLocation.Y, StationSpawnLocation.Z);
	}
	else
	{
		UE_LOG(LogAdastrea, Error, TEXT("SpawnStationForBuilder: Failed to spawn station of class %s"), *ClassToSpawn->GetName());
	}

	return NewStation;
}

ASpaceStation* AAdastreaPlayerController::GetNearestStation()
{
	return FindNearestStation();
}

UUserWidget* AAdastreaPlayerController::CreateStationEditorWidget()
{
	// Return existing widget if already created
	if (StationEditorWidget)
	{
		return StationEditorWidget;
	}

	// Lazily resolve the default WBP_StationEditor if nothing was assigned in
	// Blueprint. Deliberately NOT done in the constructor (CDO construction runs
	// too early in engine startup for a cross-module Blueprint load to be safe -
	// see the comment on the StationEditorWidgetClass default above). By the time
	// a player actually opens the editor at runtime, every module is loaded.
	if (!StationEditorWidgetClass)
	{
		StationEditorWidgetClass = StaticLoadClass(UUserWidget::StaticClass(), nullptr,
			TEXT("/Game/UI/Stations/WBP_StationEditor.WBP_StationEditor_C"));
	}

	// Check if widget class is assigned
	if (!StationEditorWidgetClass)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateStationEditorWidget: StationEditorWidgetClass is not set! Assign it in Blueprint."));
		return nullptr;
	}

	// Create the widget as a generic UUserWidget to avoid StationEditor dependency
	StationEditorWidget = CreateWidget<UUserWidget>(this, StationEditorWidgetClass);

	if (!StationEditorWidget)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateStationEditorWidget: Failed to create widget from class"));
		return nullptr;
	}

	UE_LOG(LogAdastrea, Log, TEXT("CreateStationEditorWidget: Successfully created station editor widget"));

	return StationEditorWidget;
}

void AAdastreaPlayerController::ShowStationEditor(ASpaceStation* Station)
{
	if (!Station)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("ShowStationEditor: Invalid station pointer"));
		return;
	}

	// Create widget if needed (no StationEditor-specific logic)
	if (!CreateStationEditorWidget())
	{
		return;
	}

	// Try to initialize as C++ widget (UStationEditorWidgetCpp) with InitializeEditor function
	UFunction* InitializeEditorFunc = StationEditorWidget->FindFunction(FName("InitializeEditor"));
	if (InitializeEditorFunc)
	{
		// Use ProcessEvent to call InitializeEditor(Station, ModuleCatalog)
		struct FInitializeEditorParams
		{
			ASpaceStation* Station;
			UDataAsset* Catalog;
		};

		FInitializeEditorParams Params;
		Params.Station = Station;
		Params.Catalog = ModuleCatalog;

		StationEditorWidget->ProcessEvent(InitializeEditorFunc, &Params);
		UE_LOG(LogAdastrea, Log, TEXT("ShowStationEditor: Called InitializeEditor on C++ widget"));
	}
	else
	{
		// Fallback to legacy Blueprint widget initialization
		// Initialize the widget using Blueprint-callable functions
		// The widget must implement SetStation() as a Blueprint function
		// This approach avoids circular dependency with StationEditor module

		// Use UFunction to call SetStation through reflection (Blueprint-safe)
		UFunction* SetStationFunc = StationEditorWidget->FindFunction(FName("SetStation"));
		if (SetStationFunc)
		{
			// Runtime validation: Ensure function signature matches struct
			if (SetStationFunc->NumParms != 1 || SetStationFunc->ParmsSize != sizeof(FSetStationParams))
			{
				UE_LOG(LogAdastrea, Error, TEXT("ShowStationEditor: SetStation function signature mismatch (expected 1 param, size %d; got %d params, size %d)"),
					sizeof(FSetStationParams), SetStationFunc->NumParms, SetStationFunc->ParmsSize);
				return;
			}

			FSetStationParams Params;
			Params.Station = Station;

			StationEditorWidget->ProcessEvent(SetStationFunc, &Params);
			UE_LOG(LogAdastrea, Log, TEXT("ShowStationEditor: Called SetStation on widget"));
		}
		else
		{
			UE_LOG(LogAdastrea, Warning, TEXT("ShowStationEditor: Widget does not have SetStation or InitializeEditor function. Configure in Blueprint."));
		}

		// Set ModuleCatalog through Blueprint property if available
		if (ModuleCatalog)
		{
			// Find and set the ModuleCatalog property if it exists
			if (FProperty* CatalogProp = StationEditorWidget->GetClass()->FindPropertyByName(FName("ModuleCatalog")))
			{
				// Verify it's an object property before setting (type safety)
				if (FObjectProperty* ObjProp = CastField<FObjectProperty>(CatalogProp))
				{
					// Use safe SetPropertyValue_InContainer instead of raw memory copy
					void* PropertyAddress = ObjProp->ContainerPtrToValuePtr<void>(StationEditorWidget);
					ObjProp->SetObjectPropertyValue(PropertyAddress, ModuleCatalog);
					UE_LOG(LogAdastrea, Log, TEXT("ShowStationEditor: Set ModuleCatalog on widget"));
				}
				else
				{
					UE_LOG(LogAdastrea, Warning, TEXT("ShowStationEditor: ModuleCatalog property is not an object property"));
				}
			}
		}
	}

	// Add widget to viewport
	if (!StationEditorWidget->IsInViewport())
	{
		StationEditorWidget->AddToViewport();
	}

	// Switch to UI input mode
	SetInputMode(FInputModeGameAndUI());
	bShowMouseCursor = true;

	bIsStationEditorOpen = true;

	UE_LOG(LogAdastrea, Log, TEXT("ShowStationEditor: Station editor now visible"));
}

void AAdastreaPlayerController::HideStationEditor()
{
	if (!StationEditorWidget)
	{
		bIsStationEditorOpen = false;
		return;
	}

	// Remove widget from viewport
	if (StationEditorWidget->IsInViewport())
	{
		StationEditorWidget->RemoveFromParent();
	}

	// Restore game input mode
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;

	bIsStationEditorOpen = false;

	UE_LOG(LogAdastrea, Log, TEXT("HideStationEditor: Station editor hidden"));
}

void AAdastreaPlayerController::ToggleShipStatus()
{
	// Only allow ship status when controlling a spaceship
	if (!IsControllingSpaceship())
	{
		UE_LOG(LogAdastrea, Warning, TEXT("ToggleShipStatus: Not controlling a spaceship - ship status not available"));
		return;
	}

	// Toggle ship status state
	if (bIsShipStatusOpen)
	{
		// Close the ship status screen
		HideShipStatus();
		UE_LOG(LogAdastrea, Log, TEXT("ToggleShipStatus: Closed ship status screen"));
	}
	else
	{
		// Open the ship status screen
		ShowShipStatus();
		UE_LOG(LogAdastrea, Log, TEXT("ToggleShipStatus: Opened ship status screen"));
	}
}

bool AAdastreaPlayerController::IsShipStatusOpen() const
{
	return bIsShipStatusOpen && ShipStatusWidget && ShipStatusWidget->IsInViewport();
}

UShipStatusWidget* AAdastreaPlayerController::GetShipStatusWidget() const
{
	return ShipStatusWidget;
}

UShipStatusWidget* AAdastreaPlayerController::CreateShipStatusWidget()
{
	// Return existing widget if already created
	if (ShipStatusWidget)
	{
		return ShipStatusWidget;
	}

	// Check if widget class is assigned
	if (!ShipStatusWidgetClass)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateShipStatusWidget: ShipStatusWidgetClass is not set! Assign it in Blueprint."));
		return nullptr;
	}

	// Create the widget
	ShipStatusWidget = CreateWidget<UShipStatusWidget>(this, ShipStatusWidgetClass);

	if (!ShipStatusWidget)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateShipStatusWidget: Failed to create widget from class"));
		return nullptr;
	}

	UE_LOG(LogAdastrea, Log, TEXT("CreateShipStatusWidget: Successfully created ship status widget"));

	return ShipStatusWidget;
}

void AAdastreaPlayerController::ShowShipStatus()
{
	// Create widget if needed
	if (!CreateShipStatusWidget())
	{
		return;
	}

	// Get the current spaceship
	ASpaceship* CurrentShip = GetControlledSpaceship();
	if (!CurrentShip)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("ShowShipStatus: No spaceship to display"));
		return;
	}

	// Initialize with current spaceship
	ShipStatusWidget->InitializeWithSpaceship(CurrentShip);

	// Add widget to viewport
	if (!ShipStatusWidget->IsInViewport())
	{
		ShipStatusWidget->AddToViewport();
	}

	// Switch to UI input mode
	SetInputMode(FInputModeGameAndUI());
	bShowMouseCursor = true;

	bIsShipStatusOpen = true;

	UE_LOG(LogAdastrea, Log, TEXT("ShowShipStatus: Ship status screen now visible"));
}

void AAdastreaPlayerController::HideShipStatus()
{
	if (!ShipStatusWidget)
	{
		bIsShipStatusOpen = false;
		return;
	}

	// Remove widget from viewport
	if (ShipStatusWidget->IsInViewport())
	{
		ShipStatusWidget->RemoveFromParent();
	}

	// Restore game input mode
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;

	bIsShipStatusOpen = false;

	UE_LOG(LogAdastrea, Log, TEXT("HideShipStatus: Ship status screen hidden"));
}

void AAdastreaPlayerController::ToggleMainMenu()
{
	// Toggle main menu state
	if (bIsMainMenuOpen)
	{
		// Close the main menu
		HideMainMenu();
		UE_LOG(LogAdastrea, Log, TEXT("ToggleMainMenu: Closed main menu"));
	}
	else
	{
		// Open the main menu
		ShowMainMenu();
		UE_LOG(LogAdastrea, Log, TEXT("ToggleMainMenu: Opened main menu"));
	}
}

bool AAdastreaPlayerController::IsMainMenuOpen() const
{
	return bIsMainMenuOpen && MainMenuWidget && MainMenuWidget->IsInViewport();
}

UUserWidget* AAdastreaPlayerController::GetMainMenuWidget() const
{
	return MainMenuWidget;
}

UUserWidget* AAdastreaPlayerController::CreateMainMenuWidget()
{
	// Return existing widget if already created
	if (MainMenuWidget)
	{
		return MainMenuWidget;
	}

	// Check if widget class is assigned
	if (!MainMenuWidgetClass)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateMainMenuWidget: MainMenuWidgetClass is not set! Assign it in Blueprint."));
		return nullptr;
	}

	// Create the widget
	MainMenuWidget = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);

	if (!MainMenuWidget)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateMainMenuWidget: Failed to create widget from class"));
		return nullptr;
	}

	UE_LOG(LogAdastrea, Log, TEXT("CreateMainMenuWidget: Successfully created main menu widget"));

	return MainMenuWidget;
}

void AAdastreaPlayerController::ShowMainMenu()
{
	// Create widget if needed
	if (!CreateMainMenuWidget())
	{
		return;
	}

	// Add widget to viewport
	if (!MainMenuWidget->IsInViewport())
	{
		MainMenuWidget->AddToViewport();
	}

	// Switch to UI input mode with focus on the menu
	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(MainMenuWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;

	bIsMainMenuOpen = true;

	UE_LOG(LogAdastrea, Log, TEXT("ShowMainMenu: Main menu now visible"));
}

void AAdastreaPlayerController::HideMainMenu()
{
	if (!MainMenuWidget)
	{
		// No widget to hide, but ensure state is consistent
		if (bIsMainMenuOpen)
		{
			UE_LOG(LogAdastrea, Warning, TEXT("HideMainMenu: Menu marked as open but widget is null - resetting state"));
		}
		bIsMainMenuOpen = false;
		return;
	}

	// Remove widget from viewport
	if (MainMenuWidget->IsInViewport())
	{
		MainMenuWidget->RemoveFromParent();
	}

	// Restore game input mode
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;

	bIsMainMenuOpen = false;

	UE_LOG(LogAdastrea, Log, TEXT("HideMainMenu: Main menu hidden"));
}

// ====================
// Inventory Widget Implementation
// ====================

void AAdastreaPlayerController::ToggleInventory()
{
	// Toggle inventory state
	if (bIsInventoryOpen)
	{
		// Close the inventory
		HideInventory();
		UE_LOG(LogAdastrea, Log, TEXT("ToggleInventory: Closed inventory"));
	}
	else
	{
		// Open the inventory
		ShowInventory();
		UE_LOG(LogAdastrea, Log, TEXT("ToggleInventory: Opened inventory"));
	}
}

bool AAdastreaPlayerController::IsInventoryOpen() const
{
	return bIsInventoryOpen && InventoryWidget && InventoryWidget->IsInViewport();
}

UInventoryWidget* AAdastreaPlayerController::GetInventoryWidget() const
{
	return InventoryWidget;
}

UInventoryWidget* AAdastreaPlayerController::CreateInventoryWidget()
{
	// Return existing widget if already created
	if (InventoryWidget)
	{
		return InventoryWidget;
	}

	// Check if widget class is assigned
	if (!InventoryWidgetClass)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateInventoryWidget: InventoryWidgetClass is not set! Assign it in Blueprint."));
		return nullptr;
	}

	// Create the widget
	InventoryWidget = CreateWidget<UInventoryWidget>(this, InventoryWidgetClass);

	if (!InventoryWidget)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateInventoryWidget: Failed to create widget from class"));
		return nullptr;
	}

	UE_LOG(LogAdastrea, Log, TEXT("CreateInventoryWidget: Successfully created inventory widget"));

	return InventoryWidget;
}

void AAdastreaPlayerController::ShowInventory()
{
	// Create widget if needed
	if (!CreateInventoryWidget())
	{
		return;
	}

	// Get the player's inventory component
	APawn* ControlledPawn = GetPawn();
	UInventoryComponent* PlayerInventory = nullptr;

	if (ControlledPawn)
	{
		PlayerInventory = ControlledPawn->FindComponentByClass<UInventoryComponent>();
	}

	if (!PlayerInventory)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("ShowInventory: No inventory component found on controlled pawn"));
		// Still show the widget, but it won't have data
	}

	// Initialize with inventory component
	if (PlayerInventory)
	{
		InventoryWidget->InitializeInventory(PlayerInventory);
	}

	// Add widget to viewport
	if (!InventoryWidget->IsInViewport())
	{
		InventoryWidget->AddToViewport();
	}

	// Switch to UI input mode
	SetInputMode(FInputModeGameAndUI());
	bShowMouseCursor = true;

	bIsInventoryOpen = true;

	UE_LOG(LogAdastrea, Log, TEXT("ShowInventory: Inventory now visible"));
}

void AAdastreaPlayerController::HideInventory()
{
	if (!InventoryWidget)
	{
		bIsInventoryOpen = false;
		return;
	}

	// Remove widget from viewport
	if (InventoryWidget->IsInViewport())
	{
		InventoryWidget->RemoveFromParent();
	}

	// Restore game input mode
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;

	bIsInventoryOpen = false;

	UE_LOG(LogAdastrea, Log, TEXT("HideInventory: Inventory hidden"));
}

// ====================
// Trading Widget Implementation
// ====================

// REMOVED: OpenTrading(UFactionDataAsset*) - faction system removed per Trade Simulator MVP
// Use direct market access via trading widget OpenMarket() instead

void AAdastreaPlayerController::CloseTrading()
{
	HideTrading();
	UE_LOG(LogAdastrea, Log, TEXT("CloseTrading: Closed trading interface"));
}

bool AAdastreaPlayerController::IsTradingOpen() const
{
	return bIsTradingOpen && TradingWidget && TradingWidget->IsInViewport();
}

UTradingInterfaceWidget* AAdastreaPlayerController::GetTradingWidget() const
{
	return TradingWidget;
}

UTradingInterfaceWidget* AAdastreaPlayerController::CreateTradingWidget()
{
	// Return existing widget if already created
	if (TradingWidget)
	{
		return TradingWidget;
	}

	// Check if widget class is assigned
	if (!TradingWidgetClass)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateTradingWidget: TradingWidgetClass is not set! Assign it in Blueprint."));
		return nullptr;
	}

	// Create the widget
	TradingWidget = CreateWidget<UTradingInterfaceWidget>(this, TradingWidgetClass);

	if (!TradingWidget)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateTradingWidget: Failed to create widget from class"));
		return nullptr;
	}

	UE_LOG(LogAdastrea, Log, TEXT("CreateTradingWidget: Successfully created trading widget"));

	return TradingWidget;
}

void AAdastreaPlayerController::ShowTrading()
{
	if (!TradingWidget)
	{
		return;
	}

	// Add widget to viewport
	if (!TradingWidget->IsInViewport())
	{
		TradingWidget->AddToViewport();
	}

	// Switch to UI input mode
	SetInputMode(FInputModeGameAndUI());
	bShowMouseCursor = true;

	bIsTradingOpen = true;

	UE_LOG(LogAdastrea, Log, TEXT("ShowTrading: Trading interface now visible"));
}

void AAdastreaPlayerController::HideTrading()
{
	if (!TradingWidget)
	{
		bIsTradingOpen = false;
		return;
	}

	// Remove widget from viewport
	if (TradingWidget->IsInViewport())
	{
		TradingWidget->RemoveFromParent();
	}

	// Restore game input mode
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;

	bIsTradingOpen = false;

	UE_LOG(LogAdastrea, Log, TEXT("HideTrading: Trading interface hidden"));
}

// ====================
// Station Management Widget Implementation
// ====================

void AAdastreaPlayerController::OpenStationManagement(ASpaceStation* Station)
{
	if (!Station)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("OpenStationManagement: Invalid station"));
		return;
	}

	// Create widget if needed
	if (!CreateStationManagementWidget())
	{
		return;
	}

	// Initialize with station
	StationManagementWidget->SetManagedStation(Station);

	// Show the widget
	ShowStationManagement();

	UE_LOG(LogAdastrea, Log, TEXT("OpenStationManagement: Opened management for station: %s"), *Station->GetName());
}

void AAdastreaPlayerController::CloseStationManagement()
{
	HideStationManagement();
	UE_LOG(LogAdastrea, Log, TEXT("CloseStationManagement: Closed station management interface"));
}

bool AAdastreaPlayerController::IsStationManagementOpen() const
{
	return bIsStationManagementOpen && StationManagementWidget && StationManagementWidget->IsInViewport();
}

UStationManagementWidget* AAdastreaPlayerController::GetStationManagementWidget() const
{
	return StationManagementWidget;
}

UStationManagementWidget* AAdastreaPlayerController::CreateStationManagementWidget()
{
	// Return existing widget if already created
	if (StationManagementWidget)
	{
		return StationManagementWidget;
	}

	// Check if widget class is assigned
	if (!StationManagementWidgetClass)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateStationManagementWidget: StationManagementWidgetClass is not set! Assign it in Blueprint."));
		return nullptr;
	}

	// Create the widget
	StationManagementWidget = CreateWidget<UStationManagementWidget>(this, StationManagementWidgetClass);

	if (!StationManagementWidget)
	{
		UE_LOG(LogAdastrea, Error, TEXT("CreateStationManagementWidget: Failed to create widget from class"));
		return nullptr;
	}

	UE_LOG(LogAdastrea, Log, TEXT("CreateStationManagementWidget: Successfully created station management widget"));

	return StationManagementWidget;
}

void AAdastreaPlayerController::ShowStationManagement()
{
	if (!StationManagementWidget)
	{
		return;
	}

	// Add widget to viewport
	if (!StationManagementWidget->IsInViewport())
	{
		StationManagementWidget->AddToViewport();
	}

	// Switch to UI input mode
	SetInputMode(FInputModeGameAndUI());
	bShowMouseCursor = true;

	bIsStationManagementOpen = true;

	UE_LOG(LogAdastrea, Log, TEXT("ShowStationManagement: Station management interface now visible"));
}

void AAdastreaPlayerController::HideStationManagement()
{
	if (!StationManagementWidget)
	{
		bIsStationManagementOpen = false;
		return;
	}

	// Remove widget from viewport
	if (StationManagementWidget->IsInViewport())
	{
		StationManagementWidget->RemoveFromParent();
	}

	// Restore game input mode
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;

	bIsStationManagementOpen = false;

	UE_LOG(LogAdastrea, Log, TEXT("HideStationManagement: Station management interface hidden"));
}

// ====================
// Trading Interaction Implementation
// ====================

void AAdastreaPlayerController::CheckForNearbyTradableStations()
{
	// Only check if controlling a spaceship
	if (!IsControllingSpaceship())
	{
		// Clear nearby station if we're not in a spaceship
		if (NearbyTradableStation != nullptr)
		{
			NearbyTradableStation = nullptr;
			bWasNearTradableStation = false;
			OnNearbyTradableStationChanged(false, nullptr);
		}
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	FVector PlayerLocation = ControlledPawn->GetActorLocation();

	// Find all space stations in the world
	TArray<AActor*> FoundStations;
	UGameplayStatics::GetAllActorsOfClass(World, ASpaceStation::StaticClass(), FoundStations);

	ASpaceStation* ClosestStation = nullptr;
	float ClosestDistance = TradingInteractionRadius;

	// Find the nearest station within trading interaction radius
	for (AActor* Actor : FoundStations)
	{
		ASpaceStation* Station = Cast<ASpaceStation>(Actor);
		if (!Station)
		{
			continue;
		}

		float Distance = FVector::Dist(PlayerLocation, Station->GetActorLocation());

		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestStation = Station;
		}
	}

	// Check if the nearby station state changed
	bool bIsCurrentlyNear = (ClosestStation != nullptr);

	if (bIsCurrentlyNear != bWasNearTradableStation || ClosestStation != NearbyTradableStation)
	{
		// State changed - update and notify
		NearbyTradableStation = ClosestStation;
		bWasNearTradableStation = bIsCurrentlyNear;

		OnNearbyTradableStationChanged(bIsCurrentlyNear, ClosestStation);

		if (bIsCurrentlyNear)
		{
			UE_LOG(LogAdastrea, Log, TEXT("CheckForNearbyTradableStations: Now near station '%s' at distance %.1f"),
				*ClosestStation->GetName(), ClosestDistance);
		}
		else
		{
			UE_LOG(LogAdastrea, Log, TEXT("CheckForNearbyTradableStations: Left station trading range"));
		}
	}

	// Bridge the controller's proximity detection into the ship's docking system:
	// ASpaceship::NearbyStation is what gates RequestDocking ('E to dock'), but
	// nothing was ever setting it, so docking could never trigger. Keep it in
	// sync with the nearest nearby station's docking-bay module (or null when
	// out of range).
	if (ASpaceship* Ship = GetControlledSpaceship())
	{
		// Docking range is independent of the (larger) trading radius: pick the
		// nearest station docking bay within the ship's effective docking range.
		ASpaceStationModule* DockTarget = nullptr;
		float BestDockDist = Ship->GetEffectiveDockingRange();
		for (AActor* Actor : FoundStations)
		{
			const ASpaceStation* Station = Cast<ASpaceStation>(Actor);
			ADockingBayModule* Bay = Station ? Station->GetDockingBayModule() : nullptr;
			if (!Bay)
			{
				continue;
			}
			const float Dist = FVector::Dist(PlayerLocation, Bay->GetActorLocation());
			if (Dist <= BestDockDist)
			{
				BestDockDist = Dist;
				DockTarget = Bay;
			}
		}
		if (DockTarget != LastDockTarget.Get())
		{
			UE_LOG(LogAdastrea, Log, TEXT("Docking range: %s (%.0f cm)"),
				DockTarget ? *DockTarget->GetName() : TEXT("left range"), DockTarget ? BestDockDist : 0.0f);
		}
		LastDockTarget = DockTarget;
		Ship->SetNearbyStation(DockTarget);
	}
}

void AAdastreaPlayerController::AttemptTradeWithNearestStation()
{
	if (!IsControllingSpaceship())
	{
		UE_LOG(LogAdastrea, Warning, TEXT("AttemptTradeWithNearestStation: Not controlling a spaceship"));
		return;
	}

	// Get the nearest tradable station
	ASpaceStation* Station = GetNearestTradableStation();

	if (!Station)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("AttemptTradeWithNearestStation: No station within trading range"));
		return;
	}

	// REMOVED: Faction-based trading logic - faction system removed per Trade Simulator MVP
	// This function is currently non-functional and should not be called
	// Trading must be initiated through direct market access using TradingWidget->OpenMarket(UMarketDataAsset*)

	UE_LOG(LogAdastrea, Error, TEXT("AttemptTradeWithNearestStation: This function is deprecated and non-functional. "
		"Trading system has been refactored. Use direct market access via Blueprint or alternative input binding."));

	// Optionally: Could display a UI message to the player
	// Note: This function is deprecated - market-based trading will be implemented in post-MVP
}

bool AAdastreaPlayerController::IsNearTradableStation() const
{
	return NearbyTradableStation != nullptr;
}

ASpaceStation* AAdastreaPlayerController::GetNearestTradableStation() const
{
	// Return the cached nearby station (updated by timer)
	return NearbyTradableStation;
}

// ====================
// Interior walk (avatar) control
// ====================

void AAdastreaPlayerController::HandleToggleInterior()
{
	if (ASpaceship* Ship = GetControlledSpaceship())
	{
		// Currently flying the ship -> leave the cockpit and walk the interior.
		EnterShipInterior(Ship);
	}
	else if (IsWalkingStation())
	{
		// Walking the station -> back to the docked ship.
		ExitStationInterior(false);
	}
	else if (IsOnFoot())
	{
		// Currently on foot -> sit back down in the source ship's cockpit.
		if (ASpaceship* SourceShip = InteriorSourceShip.Get())
		{
			ExitShipInterior(SourceShip);
		}
	}
}

void AAdastreaPlayerController::ShowHUDMessage(const FString& InMessage, float DurationSecs, bool bIsWarning)
{
	if (AAdastreaHUD* GameHUD = GetMapHUD())
	{
		GameHUD->ShowMessage(InMessage, DurationSecs, bIsWarning);
	}
}

void AAdastreaPlayerController::EnterShipInterior(ASpaceship* Ship)
{
	if (!Ship)
	{
		return;
	}

	// Gate: you can only leave the cockpit when stationary, out of combat,
	// and not on autopilot.
	FString LeaveReason;
	if (!Ship->CanLeaveCockpit(LeaveReason))
	{
		UE_LOG(LogAdastrea, Warning, TEXT("EnterShipInterior: %s"), *LeaveReason);
		ShowHUDMessage(LeaveReason, 3.0f, true);
		return;
	}

	ASpaceshipInterior* Interior = Ship->GetInteriorInstance();
	if (!Interior)
	{
		UE_LOG(LogAdastrea, Warning, TEXT("EnterShipInterior: ship %s has no interior instance; nothing to walk through."), *Ship->GetName());
		return;
	}

	// Save the ship as the source so we can return to its cockpit.
	InteriorSourceShip = Ship;

	// Walk the interior in a pocket well below the ship, the same way station rooms
	// are built: at the ship's own location a docked ship's interior overlaps the
	// station's docking-bay collision and the avatar gets shoved onto the roof.
	// Stays attached (KeepRelative), so ExitShipInterior only has to zero it again.
	Interior->SetActorLocation(Ship->GetActorLocation() + FVector(0.0f, 0.0f, -100000.0f));

	// Resolve a per-ship spawn override from the ship's data asset (durable tuning).
	// Local space of the interior; falls back to the interior actor's defaults.
	FVector SpawnLocal = Interior->GetEntryLocation();
	FRotator SpawnLocalRot = Interior->GetEntryRotation();
	if (Ship->ShipDataAsset && Ship->ShipDataAsset->bUseCustomAvatarSpawn)
	{
		SpawnLocal = Ship->ShipDataAsset->AvatarSpawnOffset;
		SpawnLocalRot = FRotator(0.0f, Ship->ShipDataAsset->AvatarSpawnYaw, 0.0f);
	}
	const FVector SpawnWorld = Interior->GetActorTransform().TransformPosition(SpawnLocal);
	const FRotator SpawnWorldRot = Interior->GetActorRotation() + SpawnLocalRot;

	// DEBUG MARKER: draw a green box outline at the avatar spawn point so you can
	// SEE where the avatar drops when entering this ship, and log the exact numbers
	// to tune AvatarSpawnOffset/AvatarSpawnYaw on the ship's data asset.
	if (UWorld* W = GetWorld())
	{
		DrawDebugBox(W, SpawnWorld, FVector(30.0f, 30.0f, 100.0f),
		             FColor::Green, false, 8.0f, 0, 3.0f);
		UE_LOG(LogAdastrea, Log,
			TEXT("AvatarSpawn[%s] local=(%s) world=(%s) yaw=%.1f (dataAsset=%d)"),
			*Ship->GetName(), *SpawnLocal.ToString(), *SpawnWorld.ToString(),
			SpawnLocalRot.Yaw, Ship->ShipDataAsset && Ship->ShipDataAsset->bUseCustomAvatarSpawn ? 1 : 0);
	}

	// (Re)spawn the avatar if needed.
	if (!AvatarPawn)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AvatarPawn = GetWorld()->SpawnActor<ASpaceshipAvatar>(ASpaceshipAvatar::StaticClass(),
			SpawnWorld, SpawnWorldRot, Params);
	}

	if (!AvatarPawn)
	{
		UE_LOG(LogAdastrea, Error, TEXT("EnterShipInterior: failed to spawn avatar."));
		return;
	}

	// Position the avatar at the interior entry and mark its source ship.
	AvatarPawn->SetActorLocation(SpawnWorld);
	AvatarPawn->SetActorRotation(SpawnWorldRot);
	AvatarPawn->SourceShip = Ship;
	AvatarPawn->CurrentInterior = Interior;

	// Show the interior, switch possession, capture mouse for the avatar camera.
	Interior->SetActorHiddenInGame(false);
	Interior->RevealInterior(); // explicitly unhide the shell mesh too
	// Movement is the avatar's own direct swept translation (CharacterMovementComponent
	// is disabled entirely on this class — see ASpaceshipAvatar's constructor), so there's
	// no movement mode/gravity to configure here.
	// The avatar walks INSIDE the ship's hull — disable the ship's solid collision
	// so CharacterMovement doesn't eject the avatar out of the shell on spawn.
	// GetChildrenComponents(true, ...) walks the WHOLE attachment tree under the
	// ship's root, which also includes the interior actor's own components (it's
	// attached to the ship) — skip those, or this disables the interior's own
	// FloorCollision too and the avatar free-falls straight through the hull.
	if (USceneComponent* ShipRoot = Ship->GetRootComponent())
	{
		TArray<USceneComponent*> ShipChildren;
		ShipRoot->GetChildrenComponents(true, ShipChildren);
		for (USceneComponent* C : ShipChildren)
		{
			if (!C || C->GetOwner() == Interior)
			{
				continue;
			}
			UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(C);
			if (Prim)
			{
				Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				Prim->SetSimulatePhysics(false);
			}
		}
	}
	// Record the entrance time/location so the exit trigger ignores the spawn overlap
	// for a short grace period, and ignores any overlap at all until the avatar has
	// actually walked away from the spawn point (else the avatar instantly bounces
	// back to the cockpit).
	Interior->EntranceWorldTime = Interior->GetWorld()->GetTimeSeconds();
	Interior->LastEntryWorldLocation = SpawnWorld;
	// Disable the ship's flight input mapping context so it can't keep stealing
	// WASD/look from the avatar's own bindings while we walk the interior.
	Ship->SetRuntimeInputEnabled(false);
	UnPossess();
	Possess(AvatarPawn);
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
	// Frame the interior from the avatar's eyes (first-person) so the room walls
	// surround the view instead of a 3rd-person camera clipping through them.
	AvatarPawn->SetFirstPersonView(true);

	UE_LOG(LogAdastrea, Log, TEXT("EnterShipInterior: player walked into %s's interior."), *Ship->GetName());
}

void AAdastreaPlayerController::ExitShipInterior(ASpaceship* Ship)
{
	if (!Ship)
	{
		return;
	}

	// Return to the ship's cockpit (its transform was unchanged while docked).
	if (AvatarPawn)
	{
		UnPossess();
	}
	Possess(Ship);
	// Re-enable the ship's flight input mapping context now that we're back at the
	// helm. Must come after Possess() — SetRuntimeInputEnabled looks up the subsystem
	// via Ship->GetController(), which is null until the ship is possessed.
	Ship->SetRuntimeInputEnabled(true);
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;

	if (AvatarPawn)
	{
		AvatarPawn->SetFirstPersonView(false); // restore 3rd-person ship cam
	}

	ASpaceshipInterior* Interior = Ship->GetInteriorInstance();
	if (Interior)
	{
		// Bring the interior back from its walk pocket (see EnterShipInterior).
		Interior->SetActorRelativeLocation(FVector::ZeroVector);
	}

	// Restore the ship's collision now that we're back at the helm. Skip the
	// interior's own components (attached under the ship's root, so they're in this
	// same descendant walk) — it manages its own collision setup (FloorCollision must
	// stay solid, InteriorVolume/ExitTrigger must stay query-only).
	if (USceneComponent* ShipRoot = Ship->GetRootComponent())
	{
		TArray<USceneComponent*> ShipChildren;
		ShipRoot->GetChildrenComponents(true, ShipChildren);
		for (USceneComponent* C : ShipChildren)
		{
			if (!C || C->GetOwner() == Interior)
			{
				continue;
			}
			UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(C);
			if (Prim)
			{
				Prim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				if (Prim->IsSimulatingPhysics())
				{
					Prim->SetSimulatePhysics(false);
				}
			}
		}
	}

	if (Interior)
	{
		Interior->SetActorHiddenInGame(true);
	}

	InteriorSourceShip = nullptr;
	UE_LOG(LogAdastrea, Log, TEXT("ExitShipInterior: player returned to %s's cockpit."), *Ship->GetName());
}

// ====================
// Station interior walk
// ====================

namespace
{
	/** Spawn a station room at Origin (Room is set before BeginPlay builds it). */
	AStationInterior* SpawnStationRoom(UWorld* World, const FVector& Origin, EStationRoom Room)
	{
		FTransform T(FRotator::ZeroRotator, Origin);
		AStationInterior* Interior = World->SpawnActorDeferred<AStationInterior>(
			AStationInterior::StaticClass(), T, nullptr, nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (Interior)
		{
			Interior->Room = Room;
			Interior->FinishSpawning(T);
		}
		return Interior;
	}

	/** Destroy a station room along with the terminals attached to it. */
	void DestroyStationRoom(AStationInterior* Interior)
	{
		if (!Interior)
		{
			return;
		}
		TArray<AActor*> Attached;
		Interior->GetAttachedActors(Attached);
		for (AActor* A : Attached)
		{
			A->Destroy();
		}
		Interior->Destroy();
	}
}

void AAdastreaPlayerController::EnterStationInterior(ASpaceship* Ship)
{
	EnterStationRoom(Ship, EStationRoom::Concourse);
}

void AAdastreaPlayerController::SwitchStationRoom(EStationRoom Room)
{
	UWorld* World = GetWorld();
	if (!World || !ActiveStationInterior || !AvatarPawn)
	{
		return;
	}
	const FVector Origin = ActiveStationInterior->GetActorLocation();
	DestroyStationRoom(ActiveStationInterior);
	ActiveStationInterior = SpawnStationRoom(World, Origin, Room);
	if (!ActiveStationInterior)
	{
		UE_LOG(LogAdastrea, Error, TEXT("SwitchStationRoom: failed to build room; returning to ship."));
		ActiveStationInterior = nullptr;
		return;
	}
	const FTransform Arrival = ActiveStationInterior->GetArrivalTransform();
	AvatarPawn->SetActorLocationAndRotation(Arrival.GetLocation(), Arrival.Rotator());
	SetControlRotation(Arrival.Rotator());
	if (AAdastreaHUD* H = Cast<AAdastreaHUD>(GetHUD()))
	{
		H->SetCurrentInteractable(nullptr);
	}
}

void AAdastreaPlayerController::EnterStationRoom(ASpaceship* Ship, EStationRoom Room)
{
	UWorld* World = GetWorld();
	if (!Ship || !World || ActiveStationInterior)
	{
		return;
	}

	// Build the station far below the sector so nothing else intrudes on it.
	const FVector InteriorOrigin = Ship->GetActorLocation() + FVector(0.0f, 0.0f, -200000.0f);
	ActiveStationInterior = SpawnStationRoom(World, InteriorOrigin, Room);
	if (!ActiveStationInterior)
	{
		UE_LOG(LogAdastrea, Error, TEXT("EnterStationRoom: failed to spawn station interior."));
		return;
	}

	const FTransform Arrival = ActiveStationInterior->GetArrivalTransform();

	if (!AvatarPawn)
	{
		FActorSpawnParameters AvatarParams;
		AvatarParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AvatarPawn = World->SpawnActor<ASpaceshipAvatar>(ASpaceshipAvatar::StaticClass(),
			Arrival.GetLocation(), Arrival.Rotator(), AvatarParams);
	}
	if (!AvatarPawn)
	{
		UE_LOG(LogAdastrea, Error, TEXT("EnterStationInterior: failed to spawn avatar."));
		ActiveStationInterior->Destroy();
		ActiveStationInterior = nullptr;
		return;
	}

	StationVisitShip = Ship;
	AvatarPawn->SetActorLocationAndRotation(Arrival.GetLocation(), Arrival.Rotator());
	AvatarPawn->SourceShip = Ship;
	AvatarPawn->CurrentInterior = nullptr;
	AvatarPawn->bWalkingStation = true;

	if (AAdastreaHUD* H = Cast<AAdastreaHUD>(GetHUD()))
	{
		H->HideStationMenu();
	}

	// Same hand-off as walking a ship interior: silence flight input, swap possession.
	Ship->SetRuntimeInputEnabled(false);
	UnPossess();
	Possess(AvatarPawn);
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
	AvatarPawn->SetFirstPersonView(true);

	UE_LOG(LogAdastrea, Log, TEXT("EnterStationInterior: player left %s to walk the station."), *Ship->GetName());
}

void AAdastreaPlayerController::ExitStationInterior(bool bOpenTrade)
{
	ASpaceship* Ship = StationVisitShip.Get();
	if (!ActiveStationInterior || !Ship)
	{
		return;
	}

	if (AvatarPawn)
	{
		UnPossess();
		AvatarPawn->bWalkingStation = false;
		AvatarPawn->SetFirstPersonView(false);
	}
	Possess(Ship);
	Ship->SetRuntimeInputEnabled(true);
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;

	// Tear the interior down (terminals are attached to it) and drop the stale prompt.
	DestroyStationRoom(ActiveStationInterior);
	ActiveStationInterior = nullptr;
	StationVisitShip = nullptr;

	if (AAdastreaHUD* H = Cast<AAdastreaHUD>(GetHUD()))
	{
		H->SetCurrentInteractable(nullptr);
		if (bOpenTrade)
		{
			H->ShowTradeScreen();
		}
		else
		{
			H->ShowStationMenu();
		}
	}

	UE_LOG(LogAdastrea, Log, TEXT("ExitStationInterior: player returned to %s."), *Ship->GetName());
}

void AAdastreaPlayerController::DebugStationWalk()
{
	ASpaceship* Ship = GetControlledSpaceship();
	UE_LOG(LogAdastrea, Log, TEXT("STATIONTEST: ship=%s"), Ship ? *Ship->GetName() : TEXT("none"));
	if (!Ship)
	{
		return;
	}
	EnterStationInterior(Ship);
	FTimerHandle H1, H2, H3;
	GetWorldTimerManager().SetTimer(H1, FTimerDelegate::CreateLambda([this]()
	{
		UE_LOG(LogAdastrea, Log, TEXT("STATIONTEST: walking=%d onFoot=%d avatarLoc=%s interior=%s"),
			IsWalkingStation(), IsOnFoot(), AvatarPawn ? *AvatarPawn->GetActorLocation().ToString() : TEXT("-"),
			ActiveStationInterior ? *ActiveStationInterior->GetActorLocation().ToString() : TEXT("-"));
		if (ActiveStationInterior && AvatarPawn)
		{
			const FVector Kiosk = ActiveStationInterior->GetActorTransform().TransformPosition(FVector(200, 540, 100));
			AvatarPawn->SetActorLocation(Kiosk);
		}
	}), 1.5f, false);
	GetWorldTimerManager().SetTimer(H2, FTimerDelegate::CreateLambda([this]()
	{
		AActor* I = AvatarPawn ? AvatarPawn->GetCurrentInteractableActor() : nullptr;
		UE_LOG(LogAdastrea, Log, TEXT("STATIONTEST: avatarLoc=%s interactable=%s"),
			AvatarPawn ? *AvatarPawn->GetActorLocation().ToString() : TEXT("-"), I ? *I->GetName() : TEXT("none"));
		if (I && I->Implements<UWorldInteractable>())
		{
			IWorldInteractable::Execute_Interact(I, this);
		}
	}), 3.0f, false);
	GetWorldTimerManager().SetTimer(H3, FTimerDelegate::CreateLambda([this]()
	{
		AAdastreaHUD* H = Cast<AAdastreaHUD>(GetHUD());
		UE_LOG(LogAdastrea, Log, TEXT("STATIONTEST: after interact walking=%d pawn=%s tradeScreen=%d"),
			IsWalkingStation(), GetPawn() ? *GetPawn()->GetName() : TEXT("none"), H ? H->bShowTradeScreen : -1);
	}), 4.0f, false);
}

void AAdastreaPlayerController::DebugToggleInterior()
{
	HandleToggleInterior();
	UE_LOG(LogAdastrea, Log, TEXT("INTERIORTEST: onFoot=%d pawn=%s"),
		IsOnFoot() ? 1 : 0, GetPawn() ? *GetPawn()->GetName() : TEXT("none"));
}

void AAdastreaPlayerController::DebugStationRooms()
{
	ASpaceship* Ship = GetControlledSpaceship();
	if (!Ship)
	{
		return;
	}
	EnterStationInterior(Ship);
	static const EStationRoom Order[] = { EStationRoom::Maintenance, EStationRoom::Habitation, EStationRoom::Concourse };
	for (int32 i = 0; i < 3; ++i)
	{
		FTimerHandle H;
		GetWorldTimerManager().SetTimer(H, FTimerDelegate::CreateLambda([this, i]()
		{
			SwitchStationRoom(Order[i]);
		}), 2.0f * (i + 1), false);
	}
	for (int32 i = 0; i < 4; ++i)
	{
		FTimerHandle H;
		GetWorldTimerManager().SetTimer(H, FTimerDelegate::CreateLambda([this, i]()
		{
			if (!ActiveStationInterior || !AvatarPawn) { UE_LOG(LogAdastrea, Log, TEXT("STATIONTEST: room %d no interior"), i); return; }
			int32 Terminals = 0;
			TArray<AActor*> Att; ActiveStationInterior->GetAttachedActors(Att);
			for (AActor* A : Att) { if (Cast<AStationTerminal>(A)) { ++Terminals; } }
			UE_LOG(LogAdastrea, Log, TEXT("STATIONTEST: room=%d terminals=%d avatarRelZ=%.1f pawn=%s"),
				(int32)ActiveStationInterior->Room, Terminals,
				AvatarPawn->GetActorLocation().Z - ActiveStationInterior->GetActorLocation().Z,
				GetPawn() ? *GetPawn()->GetName() : TEXT("none"));
		}), 2.0f * i + 1.0f, false);
	}
	FTimerHandle HE;
	GetWorldTimerManager().SetTimer(HE, FTimerDelegate::CreateLambda([this]()
	{
		ExitStationInterior(false);
		UE_LOG(LogAdastrea, Log, TEXT("STATIONTEST: exited walking=%d pawn=%s"), IsWalkingStation(), GetPawn() ? *GetPawn()->GetName() : TEXT("none"));
	}), 9.0f, false);
}

void AAdastreaPlayerController::HandleStationTerminalUsed(EStationTerminalType Type)
{
	switch (Type)
	{
	case EStationTerminalType::Trading:       ExitStationInterior(true); break;
	case EStationTerminalType::Airlock:       ExitStationInterior(false); break;
	case EStationTerminalType::ToConcourse:   SwitchStationRoom(EStationRoom::Concourse); break;
	case EStationTerminalType::ToMaintenance: SwitchStationRoom(EStationRoom::Maintenance); break;
	case EStationTerminalType::ToHabitation:  SwitchStationRoom(EStationRoom::Habitation); break;
	default: break;
	}
}
