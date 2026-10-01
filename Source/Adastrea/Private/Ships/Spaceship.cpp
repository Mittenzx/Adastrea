#include "Ships/Spaceship.h"
#include "Stations/DockingDebug.h"
#include "Ships/SpaceshipInterior.h"
#include "Ships/SpaceshipDataAsset.h"
#include "Ships/DockingSettingsDataAsset.h"
#include "Ships/SpaceshipControlsComponent.h"
#include "Player/AdastreaPlayerController.h"
#include "AdastreaHUD.h"
#include "Blueprint/UserWidget.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/Material.h"
#include "AdastreaLog.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Stations/SpaceStationModule.h"
#include "Stations/DockingBayModule.h"
#include "Blueprint/UserWidget.h"
#include "UI/AdastreaHUDWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Trading/CargoComponent.h"
#include "Trading/PlayerTraderComponent.h"
#include "Ships/ShipUpgradeComponent.h"
#include "Stations/SpaceStation.h"
#include "Drones/DroneBayComponent.h"
#include "Mining/Asteroid.h"
#include "Audio/ShipEngineAudioComponent.h"
#include "Combat/ShipHealthComponent.h"
#include "Combat/ShipWeaponComponent.h"
#include "Audio/AudioEventLibrary.h"
#include "TimerManager.h"
#include "Ships/ExteriorDressingComponent.h"
#include "Particles/ParticleSystemComponent.h"

namespace ShipEventAudio
{
    /** Turn rate (deg/s) that counts as a rotation burst. */
    constexpr float TurnThresholdDegPerSec = 20.0f;
    /** Above this the "turn" was a teleport (docking, load, respawn), not a manoeuvre. */
    constexpr float TeleportTurnDegPerSec = 720.0f;
    /** Strafe input magnitude that counts as a strafe burst. */
    constexpr float StrafeThreshold = 0.5f;
    /** A new burst needs this long without manoeuvring first. */
    constexpr float MinQuietSeconds = 0.2f;
    /** Rate limits: light ships puff often, heavy ships groan rarely. */
    constexpr float PuffMinInterval = 0.35f;
    constexpr float GroanMinInterval = 2.5f;
    /** Airlock hiss follows the clamps after this delay. */
    constexpr float AirlockHissDelay = 1.2f;
    constexpr float CollisionBumpMinInterval = 0.6f;
}

// Debug flag for docking system - can be disabled for shipping builds
#ifndef DOCKING_DEBUG_ENABLED
    #define DOCKING_DEBUG_ENABLED 1
#endif

ASpaceship::ASpaceship()
{
    // Enable tick for X4-style flight physics
    PrimaryActorTick.bCanEverTick = true;
    InteriorInstance = nullptr;
    SavedExternalPawn = nullptr;
    ShipDataAsset = nullptr;

    // Initialize default movement properties
    DefaultMaxSpeed = 3000.0f;
    DefaultAcceleration = 1000.0f;
    DefaultDeceleration = 1000.0f;
    DefaultTurningBoost = 8.0f;
    TurnRate = 45.0f;

    // Initialize ship status
    CurrentHullIntegrity = 1000.0f;
    MaxHullIntegrity = 1000.0f;
    // Initialize X4-style flight control parameters
    bFlightAssistEnabled = true;              // Flight assist on by default
    RotationDampingFactor = 0.85f;            // Smooth rotation with high damping
    AutoLevelStrength = 0.0f;                 // off - auto-level fights simple mouse-look
    FlightAssistResponsiveness = 2.0f;        // Responsive but not twitchy
    ThrottlePercentage = 0.0f;                // Start at zero throttle
    ThrottleStep = 10.0f;                     // 10% increments
    ThrottleAdjustmentCooldown = 0.1f;        // 10 adjustments per second max
    bBoostActive = false;
    BoostMultiplier = 2.0f;                   // Double speed when boosting
    bTravelModeActive = false;
    TravelModeMultiplier = 5.0f;              // 5x speed in travel mode
    StrafeIndependence = 0.8f;                // High strafe independence
    MouseFlightSensitivity = 1.0f;            // 1:1 mouse sensitivity
        bUseMousePositionFlight = false;          // Simple mouse-delta look (X4 position flight deferred)
        MouseDeadzoneRadius = 50.0f;              // 50 pixel deadzone from center
        MouseMaxRadius = 400.0f;                  // 400 pixels for 100% rotation speed

    // Initialize free look camera parameters
    bFreeLookActive = false;
    FreeLookSensitivity = 1.5f;               // Slightly higher sensitivity for free look
    CameraDistance = 800.0f;                  // Default camera distance
    CameraLagSpeed = 10.0f;                   // Smooth position following
    CameraRotationLagSpeed = 10.0f;           // Smooth rotation following
    DoubleClickThreshold = 0.3f;              // 300ms for double-click detection

    // Initialize physics state
    CurrentVelocity = FVector::ZeroVector;
    TargetVelocity = FVector::ZeroVector;
    RotationVelocity = FRotator::ZeroRotator;
    ForwardInput = 0.0f;
    RightInput = 0.0f;
    UpInput = 0.0f;
    YawInput = 0.0f;
    PitchInput = 0.0f;
    RollInput = 0.0f;
    FreeLookRotation = FRotator::ZeroRotator;
    LastFreeLookClickTime = 0.0f;
    LastThrottleAdjustmentTime = 0.0f;

    // Initialize docking system
    NearbyStation = nullptr;
    CurrentDockingPoint = nullptr;
    bIsDocked = false;
    bIsDocking = false;
    DockingRange = 8000.0f;
    DockingPromptWidget = nullptr;
    DockingPromptWidgetClass = nullptr;
    TradingInterfaceClass = nullptr;
    TradingWidget = nullptr;

    // Create root scene component for proper pivot point
        // This ensures the ship rotates around its center, not the world origin
        ShipRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ShipRoot"));
        RootComponent = ShipRoot;

        // Create the visual ship mesh component attached to the root. The mesh asset
        // (assembled ship silhouette) is assigned per-ship in the Blueprint, e.g.
        // SM_Ship_Fighter_01_Assembled for the fighter.
        ShipMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
        ShipMeshComponent->SetupAttachment(ShipRoot);
        // The generated hull meshes (Tools/generate_adastrea_assets.py) are modelled nose-along
        // Blender +Y, which imports as UE -Y. Yaw 90 turns the nose onto the actor's +X forward.
        ShipMeshComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
        ShipMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); // flying is kinematic

        // Create and configure the floating pawn movement component
    MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
    MovementComponent->MaxSpeed = DefaultMaxSpeed;
    MovementComponent->Acceleration = DefaultAcceleration;
    MovementComponent->Deceleration = DefaultDeceleration;
    MovementComponent->TurningBoost = DefaultTurningBoost;

    // Create and configure the particle component
    ParticleComponent = CreateDefaultSubobject<USpaceshipParticleComponent>(TEXT("ParticleComponent"));

    // Create and configure camera components
    CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
    CameraSpringArm->SetupAttachment(ShipRoot);
    CameraSpringArm->TargetArmLength = CameraDistance;
    CameraSpringArm->bUsePawnControlRotation = false; // We'll control this manually for free look
    CameraSpringArm->bEnableCameraLag = true;
    CameraSpringArm->CameraLagSpeed = CameraLagSpeed;
    CameraSpringArm->bEnableCameraRotationLag = true; // Enable smooth rotation following
    CameraSpringArm->CameraRotationLagSpeed = CameraRotationLagSpeed;
    CameraSpringArm->bDoCollisionTest = false; // Disable collision in space

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(CameraSpringArm, USpringArmComponent::SocketName);

    // Create trading components (cargo hold + player trader) so every ship can trade
    CargoComponent = CreateDefaultSubobject<UCargoComponent>(TEXT("CargoComponent"));
    PlayerTraderComponent = CreateDefaultSubobject<UPlayerTraderComponent>(TEXT("PlayerTraderComponent"));
    UpgradeComponent = CreateDefaultSubobject<UShipUpgradeComponent>(TEXT("UpgradeComponent"));

    // Drone bay hatch under the hull (moved to the keel at BeginPlay).
    // Stays inert unless enabled - see BeginPlay (ships with a MiningRating).
    DroneBay = CreateDefaultSubobject<UDroneBayComponent>(TEXT("DroneBay"));
    DroneBay->SetupAttachment(ShipRoot);
    DroneBay->SetRelativeLocation(FVector(0.0f, 0.0f, -150.0f));

    HealthComponent = CreateDefaultSubobject<UShipHealthComponent>(TEXT("HealthComponent"));
    WeaponComponent = CreateDefaultSubobject<UShipWeaponComponent>(TEXT("WeaponComponent"));

    // Engine voice. Attached to the hull mesh; at BeginPlay it moves to the mesh bounds'
    // centre so AI engine range is measured from the hull surface.
    EngineAudio = CreateDefaultSubobject<UShipEngineAudioComponent>(TEXT("EngineAudio"));
    EngineAudio->SetupAttachment(ShipMeshComponent);
}

void ASpaceship::BeginPlay()
{
    Super::BeginPlay();

    // Nav lights, strobes, beacons and a sensor fit traced onto the hull, unless the
    // Blueprint already places its own dressing component.
    if (UExteriorDressingComponent::IsAutoDressingEnabled() && !FindComponentByClass<UExteriorDressingComponent>())
    {
        UExteriorDressingComponent* Dressing = NewObject<UExteriorDressingComponent>(this, TEXT("ExteriorDressing"));
        Dressing->Preset = EExteriorDressingPreset::Ship;
        Dressing->SetupAttachment(GetRootComponent());
        Dressing->RegisterComponent();
    }

    UE_LOG(LogAdastreaShips, Warning, TEXT("*** ASpaceship::BeginPlay on %s ***"), *GetName());

    // Assign the imported hull material to the ship mesh so the ship renders
    // textured instead of un-textured. The assets agent imported M_*_Hull
    // materials (/Game/Materials) but the BP meshes' overrideMaterials are empty
    // (a BP-component-graph edit that resisted MCP/-ExecutePythonScript). We map
    // the hull per ship class here in code, which is robust and main-side.
    ApplyShipHullMaterial();
    AttachShipWindows();
    FitCameraToHull();

    // Initialize hull integrity from data asset if available
    if (ShipDataAsset)
    {
        MaxHullIntegrity = ShipDataAsset->HullStrength;
        CurrentHullIntegrity = MaxHullIntegrity; // Start at full health
        ApplyDataAssetMobility();

        // Ships rated for mining carry mining drones; cutting power scales with the rating,
        // and a bigger drone complement puts more drones on the rock.
        if (DroneBay && ShipDataAsset->MiningRating > 0)
        {
            DroneBay->bMiningEnabled = true;
            DroneBay->MiningPower = FMath::Max(DroneBay->MiningPower, ShipDataAsset->MiningRating * 0.25f);
            if (ShipDataAsset->DroneCapacity > 0)
            {
                DroneBay->DroneCount = FMath::Clamp(ShipDataAsset->DroneCapacity / 2, 2, 6);
            }
        }
    }

    // Upgrades scale the stats as they stand now (data asset + Blueprint defaults).
    if (UpgradeComponent)
    {
        if (ShipDataAsset)
        {
            UpgradeComponent->ShipTypeID = ShipDataAsset->GetFName();
            if (ShipDataAsset->WeaponSlots > 0)
            {
                UpgradeComponent->CategorySlots.Add(EShipUpgradeCategory::Weapons, FMath::Clamp(ShipDataAsset->WeaponSlots, 1, 6));
            }
        }
        UpgradeComponent->OnUpgradesChanged.AddUniqueDynamic(this, &ASpaceship::RecalculateUpgradedStats);
    }
    RebaseUpgradeStats();

    // Spawn the interior actor if needed
        if (!InteriorInstance.IsValid())
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            ASpaceshipInterior* SpawnedInterior = GetWorld()->SpawnActor<ASpaceshipInterior>(ASpaceshipInterior::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
            if (SpawnedInterior)
            {
                SpawnedInterior->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
                SpawnedInterior->SetActorHiddenInGame(true); // Hide until entered
                // Show the real interior geometry and size the walkable volume to it.
                UStaticMesh* ShellMesh = InteriorShellMesh.IsNull() ? nullptr : InteriorShellMesh.LoadSynchronous();
                SpawnedInterior->ConfigureInterior(ShellMesh, InteriorFamily, false);
                InteriorInstance = SpawnedInterior;
            }
        }
}

#if WITH_EDITOR
void ASpaceship::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // Synchronize camera spring arm settings when properties change in editor
    if (PropertyChangedEvent.Property && CameraSpringArm)
    {
        FName PropertyName = PropertyChangedEvent.Property->GetFName();

        if (PropertyName == GET_MEMBER_NAME_CHECKED(ASpaceship, CameraDistance))
        {
            CameraSpringArm->TargetArmLength = CameraDistance;
        }
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASpaceship, CameraLagSpeed))
        {
            CameraSpringArm->CameraLagSpeed = CameraLagSpeed;
        }
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASpaceship, CameraRotationLagSpeed))
        {
            CameraSpringArm->CameraRotationLagSpeed = CameraRotationLagSpeed;
        }
    }
}
#endif

void ASpaceship::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    UE_LOG(LogAdastreaShips, Warning, TEXT("*** SHIP POSSESSED: %s by controller %s ***"),
        *GetName(),
        NewController ? *NewController->GetName() : TEXT("nullptr"));
}

void ASpaceship::UnPossessed()
{
    FireStopped();

    // Take our priority-10 mapping context off the player before letting go. Every
    // ship adds its own, all mapping the same keys at the same priority; one left
    // behind by a swapped-out (even destroyed) ship can win WASD, mouse and R/F, and
    // the ship now being flown gets no input at all.
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (RuntimeInputMappingContext)
            {
                Subsystem->RemoveMappingContext(RuntimeInputMappingContext);
                UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Removed runtime input mapping context from %s"), *GetName());
            }
        }
    }
    Super::UnPossessed();
}

void ASpaceship::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // A wreck drifts whether or not anyone is aboard, and nothing flies it.
    if (bWrecked)
    {
        TickWreck(DeltaTime);
        return;
    }

        // Only apply flight physics if ship is possessed by a controller
        // This avoids unnecessary CPU usage on unpossessed NPC ships
        if (!GetController())
        {
            return;
        }

    TickSwapShowcase(DeltaTime);

        // Update X4-style mouse position flight controls every frame (when enabled)
        // This allows continuous rotation based on cursor position, not mouse movement
    if (bUseMousePositionFlight && bFlightAssistEnabled)
    {
        UpdateMousePositionFlight(DeltaTime);
    }

    // Apply X4-style flight physics when flight assist is enabled
    if (bFlightAssistEnabled)
    {
        ApplyFlightAssist(DeltaTime);
    }

    // Update velocity based on throttle setting (only when flight assist is on)
    if (bFlightAssistEnabled)
    {
        UpdateThrottleVelocity(DeltaTime);
    }

    // Apply auto-leveling when enabled and no rotation input
    // Note: Currently we don't have explicit roll input, so we check for no yaw input
    // as a proxy for when the player is not actively rotating
    if (bFlightAssistEnabled && FMath::IsNearlyZero(YawInput, 0.01f))
    {
        ApplyAutoLeveling(DeltaTime);
    }

    UpdateThrusterAudio(DeltaTime);
}

void ASpaceship::UpdateThrusterAudio(float DeltaTime)
{
    using namespace ShipEventAudio;

    // Only the player's own ship, and only while it is actually flying.
    if (DeltaTime <= 0.0f || bIsDocked || bIsDocking || IsHidden() || !UAudioEventLibrary::IsLocalPlayerActor(this))
    {
        bThrusterAudioHasRotation = false;
        bThrusterAudioWasManeuvering = false;
        return;
    }

    // Turn rate from the actual rotation change, whichever input path caused it.
    const FQuat Rotation = GetActorQuat();
    float TurnDegPerSec = 0.0f;
    if (bThrusterAudioHasRotation)
    {
        TurnDegPerSec = FMath::RadiansToDegrees(ThrusterAudioLastRotation.AngularDistance(Rotation)) / DeltaTime;
        if (TurnDegPerSec > TeleportTurnDegPerSec)
        {
            TurnDegPerSec = 0.0f;
        }
    }
    ThrusterAudioLastRotation = Rotation;
    bThrusterAudioHasRotation = true;

    // Strafe/vertical: Move() stamps the time while the keys are held (RightInput/UpInput go
    // stale on release). Allow a couple of frames of slack for input/tick ordering.
    const double Now = GetWorld()->GetTimeSeconds();
    const bool bStrafing = ThrusterAudioLastStrafeTime >= 0.0
        && Now - ThrusterAudioLastStrafeTime <= FMath::Max(0.1, 2.0 * DeltaTime);

    const bool bManeuvering = TurnDegPerSec > TurnThresholdDegPerSec || bStrafing;
    if (bManeuvering && !bThrusterAudioWasManeuvering && ThrusterAudioQuietTime >= MinQuietSeconds)
    {
        if (UAudioEventLibrary::GetShipSizeFactor(ShipDataAsset) >= UAudioEventLibrary::HeavyShipSizeFactor)
        {
            UAudioEventLibrary::PlayEvent2D(this, TEXT("Thruster.HeavyGroan"), GroanMinInterval);
        }
        else
        {
            UAudioEventLibrary::PlayEvent2D(this, TEXT("Thruster.Puff"), PuffMinInterval);
        }
    }
    ThrusterAudioQuietTime = bManeuvering ? 0.0f : ThrusterAudioQuietTime + DeltaTime;
    bThrusterAudioWasManeuvering = bManeuvering;
}

void ASpaceship::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved,
    FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
    Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

    if (!bIsDocked && UAudioEventLibrary::IsLocalPlayerActor(this))
    {
        UAudioEventLibrary::PlayEventAtLocation(this, TEXT("Flight.CollisionBump"), HitLocation,
            ShipEventAudio::CollisionBumpMinInterval);
    }
}

void ASpaceship::EnterInterior(APlayerController* PlayerController)
{
    if (InteriorInstance.IsValid() && PlayerController)
    {
        // Hide spaceship exterior, show interior
        InteriorInstance->SetActorHiddenInGame(false);

        // Teleport player to interior start location
        FVector InteriorEntry = InteriorInstance->GetEntryLocation();
        FRotator EntryRotation = InteriorInstance->GetEntryRotation();
        PlayerController->GetPawn()->SetActorLocation(InteriorEntry, false, nullptr, ETeleportType::TeleportPhysics);
        PlayerController->GetPawn()->SetActorRotation(EntryRotation);

        // Optionally, switch possession to a walkable character
        // (Not shown: implement a character class for walking inside)
    }
}

void ASpaceship::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship::SetupPlayerInputComponent called on %s"), *GetName());

    // Ensure the ship has its own actions + mapping context (WASD/look/throttle/dock)
    // so controls work regardless of which GameMode is active.
    EnsureOwnInputActionsAndContext();

    // Setup Enhanced Input bindings for ASpaceship's input (throttle, free look, etc.)
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Check if ASpaceship has its own configured input actions
        bool bHasOwnActions = (MoveAction != nullptr || LookAction != nullptr);

        if (bHasOwnActions)
        {
            UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Using ASpaceship's own input actions (MoveAction=%s, LookAction=%s)"),
                MoveAction ? TEXT("Valid") : TEXT("NULL"),
                LookAction ? TEXT("Valid") : TEXT("NULL"));

            // Use ASpaceship's sophisticated input handling
            if (MoveAction)
            {
                EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASpaceship::Move);
                UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Bound MoveAction"));
            }

            if (LookAction)
            {
                EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASpaceship::Look);
                UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Bound LookAction"));
            }
        }
        else
        {
            UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: No configured input actions, delegating to SpaceshipControlsComponent"));

            // Initialize SpaceshipControlsComponent if present (handles basic movement/look/fire)
            USpaceshipControlsComponent* ControlsComponent = FindComponentByClass<USpaceshipControlsComponent>();
            if (ControlsComponent)
            {
                UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Found SpaceshipControlsComponent, initializing input bindings"));
                ControlsComponent->InitializeInputBindings(PlayerInputComponent);
            }
            else
            {
                UE_LOG(LogAdastreaInput, Warning, TEXT("ASpaceship: No input actions configured and SpaceshipControlsComponent NOT FOUND on %s"), *GetName());
            }
        }

        // Always bind free look, throttle, and dock actions (these are ASpaceship-specific)
        if (FreeLookAction)
        {
            // Bind free look action with Started, Triggered, and Completed events
            EnhancedInputComponent->BindAction(FreeLookAction, ETriggerEvent::Started, this, &ASpaceship::FreeLookStarted);
            EnhancedInputComponent->BindAction(FreeLookAction, ETriggerEvent::Triggered, this, &ASpaceship::FreeLookCamera);
            EnhancedInputComponent->BindAction(FreeLookAction, ETriggerEvent::Completed, this, &ASpaceship::FreeLookCompleted);
            UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Bound FreeLookAction"));
        }

        if (ThrottleUpAction)
        {
            EnhancedInputComponent->BindAction(ThrottleUpAction, ETriggerEvent::Triggered, this, &ASpaceship::ThrottleUp);
            UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Bound ThrottleUpAction"));
        }

        if (ThrottleDownAction)
        {
            EnhancedInputComponent->BindAction(ThrottleDownAction, ETriggerEvent::Triggered, this, &ASpaceship::ThrottleDown);
            UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Bound ThrottleDownAction"));
        }

        if (DockAction)
        {
            // Started, not Triggered: Triggered fires every frame the key is held, which
            // re-requested docking (and logged/alerted "No station in range") ~60x a second.
            EnhancedInputComponent->BindAction(DockAction, ETriggerEvent::Started, this, &ASpaceship::RequestDocking);
            UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Bound DockAction to RequestDocking"));
        }

        if (DroneAction)
        {
            EnhancedInputComponent->BindAction(DroneAction, ETriggerEvent::Started, this, &ASpaceship::ToggleDrones);
        }
        if (LockAsteroidAction)
        {
            EnhancedInputComponent->BindAction(LockAsteroidAction, ETriggerEvent::Started, this, &ASpaceship::LockAsteroid);
        }
        if (FireAction)
        {
            EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ASpaceship::FireStarted);
            EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ASpaceship::FireStopped);
            EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Canceled, this, &ASpaceship::FireStopped);
        }
    }
}

void ASpaceship::LaunchDrones()
{
    if (!DroneBay || !DroneBay->bMiningEnabled || bIsDocked || bIsDocking)
    {
        return;
    }
    // An asteroid picked with the target cycle keys is the job, unless the bay already has one.
    if (!DroneBay->GetTarget())
    {
        if (const AAdastreaPlayerController* PC = Cast<AAdastreaPlayerController>(GetController()))
        {
            if (AAsteroid* Picked = Cast<AAsteroid>(PC->GetLockedTarget()))
            {
                DroneBay->SetTarget(Picked);
            }
        }
    }
    DroneBay->LaunchDrones();
}

void ASpaceship::RecallDrones()
{
    if (DroneBay)
    {
        DroneBay->RecallDrones();
    }
}

void ASpaceship::ToggleDrones()
{
    if (DroneBay && DroneBay->IsDeployed())
    {
        RecallDrones();
    }
    else
    {
        LaunchDrones();
    }
}

void ASpaceship::LockAsteroid()
{
    if (DroneBay && DroneBay->bMiningEnabled)
    {
        const bool bOk = DroneBay->LockNearestAhead();
        UE_LOG(LogAdastreaShips, Log, TEXT("LockAsteroid (%s): %s"), *GetName(),
            bOk ? *FString::Printf(TEXT("locked %s"), *GetNameSafe(DroneBay->GetTarget())) : TEXT("nothing in the aim cone"));
    }
    else
    {
        UE_LOG(LogAdastreaShips, Log, TEXT("LockAsteroid (%s): this ship has no mining drones"), *GetName());
    }
}

namespace ShipWreck
{
    /** Tumble rate range (deg/s per axis). */
    constexpr float MaxSpin = 9.0f;
    /** Fraction of drift speed lost per second, so wrecks slow and stay near the fight. */
    constexpr float DriftDamping = 0.25f;
    const TCHAR* OverlayMaterial = TEXT("/Game/Materials/M_Wreck_Overlay.M_Wreck_Overlay");
}

void ASpaceship::SetWrecked(bool bWreck)
{
    if (bWrecked == bWreck)
    {
        return;
    }
    bWrecked = bWreck;

    if (bWreck)
    {
        WreckVelocity = GetVelocity();
        WreckSpin = FRotator(FMath::FRandRange(-1.0f, 1.0f), FMath::FRandRange(-1.0f, 1.0f), FMath::FRandRange(-1.0f, 1.0f)) * ShipWreck::MaxSpin;
        SetThrottle(0.0f);
        bBoostActive = false;
        FireStopped();
        if (DroneBay)
        {
            DroneBay->RecallDrones();
        }
        if (MovementComponent)
        {
            MovementComponent->StopMovementImmediately();
            MovementComponent->Deactivate();
        }
        // Keep the chase camera level while the hull tumbles.
        if (CameraSpringArm)
        {
            CameraSpringArm->SetUsingAbsoluteRotation(true);
        }
    }
    else
    {
        WreckVelocity = FVector::ZeroVector;
        WreckSpin = FRotator::ZeroRotator;
        // Throttle input still moves the lever on a wreck; come back to life at a standstill.
        SetThrottle(0.0f);
        if (MovementComponent)
        {
            MovementComponent->Activate();
        }
        if (CameraSpringArm)
        {
            CameraSpringArm->SetUsingAbsoluteRotation(false);
            CameraSpringArm->SetRelativeRotation(FRotator::ZeroRotator);
        }
    }

    // Dark hull, no window glow, no nav lights, no engine plumes.
    UMaterialInterface* Overlay = bWreck ? LoadObject<UMaterialInterface>(nullptr, ShipWreck::OverlayMaterial) : nullptr;
    TArray<UStaticMeshComponent*> Meshes;
    GetComponents<UStaticMeshComponent>(Meshes);
    for (UStaticMeshComponent* Mesh : Meshes)
    {
        if (Mesh == ShipMeshComponent || Mesh->GetAttachParent() == ShipMeshComponent)
        {
            Mesh->SetOverlayMaterial(Overlay);
        }
    }
    if (WindowMeshComponent)
    {
        WindowMeshComponent->SetVisibility(!bWreck);
    }
    if (UExteriorDressingComponent* Dressing = FindComponentByClass<UExteriorDressingComponent>())
    {
        Dressing->SetLightsOn(!bWreck);
    }
    TArray<UFXSystemComponent*> Effects;
    GetComponents<UFXSystemComponent>(Effects);
    for (UFXSystemComponent* Fx : Effects)
    {
        if (bWreck)
        {
            Fx->Deactivate();
        }
    }

    if (bWreck)
    {
        if (AAdastreaPlayerController* PC = Cast<AAdastreaPlayerController>(GetController()))
        {
            PC->ShowHUDMessage(TEXT("SHIP DISABLED - engines, weapons and power offline"), 6.0f, true);
        }
    }
    UE_LOG(LogAdastreaShips, Log, TEXT("%s %s"), *GetName(), bWreck ? TEXT("is a wreck") : TEXT("repaired"));
}

void ASpaceship::TickWreck(float DeltaTime)
{
    WreckVelocity *= FMath::Max(0.0f, 1.0f - ShipWreck::DriftDamping * DeltaTime);
    AddActorWorldOffset(WreckVelocity * DeltaTime);
    AddActorLocalRotation(WreckSpin * DeltaTime);
}

void ASpaceship::FireStarted()
{
    if (!WeaponComponent || bIsDocked || bIsDocking)
    {
        return;
    }
    // LMB also picks targets (Tab targeting mode) and clicks the map and menus.
    if (const AAdastreaPlayerController* PC = Cast<AAdastreaPlayerController>(GetController()))
    {
        if (PC->IsTargetingModeActive())
        {
            return;
        }
        if (const AAdastreaHUD* HUD = Cast<AAdastreaHUD>(PC->GetHUD()))
        {
            if (HUD->IsOtherScreenOpen() || HUD->bShowPauseMenu)
            {
                return;
            }
        }
    }
    WeaponComponent->SetTriggerHeld(true);
}

void ASpaceship::FireStopped()
{
    if (WeaponComponent)
    {
        WeaponComponent->SetTriggerHeld(false);
    }
}

void ASpaceship::EnsureOwnInputActionsAndContext()
{
    // Create a self-contained input mapping context so the ship controls work
    // even when the active GameMode doesn't add a content-side IMC (e.g. the
    // playable AdastreaGameMode, which only TestGameMode currently adds).
    if (!RuntimeInputMappingContext)
    {
        RuntimeInputMappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_SpaceshipRuntime"));

        // Movement: W/S = up/down (vertical), A/D = strafe (left/right).
                // Move() routes MoveAction.Get<FVector>().X -> MoveUp (vertical) and
                // .Y -> MoveRight (strafe). Route each key to the right component:
                //   W = +vertical  (default X+=1)            -> .X=+1
                //   S = -vertical  (X+1, negated)            -> .X=-1
                //   D = +strafe    (swizzle XY so X->Y)      -> .Y=+1
                //   A = -strafe    (swizzle XY + negate)     -> .Y=-1
                if (!MoveAction)
                {
                    MoveAction = NewObject<UInputAction>(this, TEXT("IA_Move_Runtime"));
                    MoveAction->ValueType = EInputActionValueType::Axis3D;
                }
                RuntimeInputMappingContext->MapKey(MoveAction, EKeys::W);

                {
                    FEnhancedActionKeyMapping& SMapping = RuntimeInputMappingContext->MapKey(MoveAction, EKeys::S);
                    UInputModifierNegate* SNegate = NewObject<UInputModifierNegate>(RuntimeInputMappingContext);
                    SMapping.Modifiers.Add(SNegate);
                }
                {
                    FEnhancedActionKeyMapping& DMapping = RuntimeInputMappingContext->MapKey(MoveAction, EKeys::D);
                    UInputModifierSwizzleAxis* DSwizzle = NewObject<UInputModifierSwizzleAxis>(RuntimeInputMappingContext);
                    DSwizzle->Order = EInputAxisSwizzle::YXZ;
                    DMapping.Modifiers.Add(DSwizzle);
                }
                {
                    FEnhancedActionKeyMapping& AMapping = RuntimeInputMappingContext->MapKey(MoveAction, EKeys::A);
                    UInputModifierSwizzleAxis* ASwizzle = NewObject<UInputModifierSwizzleAxis>(RuntimeInputMappingContext);
                    ASwizzle->Order = EInputAxisSwizzle::YXZ;
                    AMapping.Modifiers.Add(ASwizzle);
                    UInputModifierNegate* ANegate = NewObject<UInputModifierNegate>(RuntimeInputMappingContext);
                    AMapping.Modifiers.Add(ANegate);
                }

        // Look: mouse XY
        if (!LookAction)
        {
            LookAction = NewObject<UInputAction>(this, TEXT("IA_Look_Runtime"));
            LookAction->ValueType = EInputActionValueType::Axis2D;
        }
        RuntimeInputMappingContext->MapKey(LookAction, EKeys::Mouse2D);

        // Throttle: R = up, F = down (no mouse wheel needed)
        if (!ThrottleUpAction)
        {
            ThrottleUpAction = NewObject<UInputAction>(this, TEXT("IA_ThrottleUp_Runtime"));
            ThrottleUpAction->ValueType = EInputActionValueType::Boolean;
        }
        RuntimeInputMappingContext->MapKey(ThrottleUpAction, EKeys::R);

        if (!ThrottleDownAction)
        {
            ThrottleDownAction = NewObject<UInputAction>(this, TEXT("IA_ThrottleDown_Runtime"));
            ThrottleDownAction->ValueType = EInputActionValueType::Boolean;
        }
        RuntimeInputMappingContext->MapKey(ThrottleDownAction, EKeys::F);

        // Dock: E key (docking/trading prompt). NOTE: F is used for throttle-down,
                // so dock uses E to avoid conflicting.
                if (!DockAction)
                {
                    DockAction = NewObject<UInputAction>(this, TEXT("IA_Dock_Runtime"));
                    DockAction->ValueType = EInputActionValueType::Boolean;
                }
                RuntimeInputMappingContext->MapKey(DockAction, EKeys::E);

                // Drones: T locks the asteroid ahead, L launches / recalls the mining drones.
                if (!DroneAction)
                {
                    DroneAction = NewObject<UInputAction>(this, TEXT("IA_Drones_Runtime"));
                    DroneAction->ValueType = EInputActionValueType::Boolean;
                }
                RuntimeInputMappingContext->MapKey(DroneAction, EKeys::L);
                if (!LockAsteroidAction)
                {
                    LockAsteroidAction = NewObject<UInputAction>(this, TEXT("IA_LockAsteroid_Runtime"));
                    LockAsteroidAction->ValueType = EInputActionValueType::Boolean;
                }
                RuntimeInputMappingContext->MapKey(LockAsteroidAction, EKeys::T);

                // Guns: hold LMB. (In Tab targeting mode LMB picks targets instead; see FireStarted.)
                if (!FireAction)
                {
                    FireAction = NewObject<UInputAction>(this, TEXT("IA_Fire_Runtime"));
                    FireAction->ValueType = EInputActionValueType::Boolean;
                }
                RuntimeInputMappingContext->MapKey(FireAction, EKeys::LeftMouseButton);
    }

    // Add the mapping context to the local player's input subsystem
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(RuntimeInputMappingContext, 10);
            UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Added runtime input mapping context"));
        }
    }
}

void ASpaceship::Move(const FInputActionValue& Value)
{
    // Get the 3D vector input (WASD + QE for vertical)
    const FVector MovementVector = Value.Get<FVector>();
    UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship::Move - MovementVector: X=%.2f Y=%.2f Z=%.2f"),
        MovementVector.X, MovementVector.Y, MovementVector.Z);

    // Control scheme (MVP):
    //   W/S  -> vertical (up/down)  — X axis
    //   A/D  -> strafe (left/right) — Y axis
    //   Forward thrust is driven by the throttle (R/F) via UpdateThrottleVelocity.
    //   X4-style: WASD gives direct vertical + strafe velocity, throttle = cruise.
    MoveUp(MovementVector.X);
    MoveRight(MovementVector.Y);

    // Thruster audio: Triggered re-fires every frame while held, so this stays fresh.
    if (FVector2D(MovementVector.X, MovementVector.Y).Size() > ShipEventAudio::StrafeThreshold && GetWorld())
    {
        ThrusterAudioLastStrafeTime = GetWorld()->GetTimeSeconds();
    }
}

FVector2D ASpaceship::NormalizeLookInputByAspectRatio(const FVector2D& LookInput, APlayerController* PC)
{
    if (!PC)
    {
        return LookInput;
    }

    int32 ViewportSizeX, ViewportSizeY;
    PC->GetViewportSize(ViewportSizeX, ViewportSizeY);

    if (ViewportSizeX <= 0 || ViewportSizeY <= 0)
    {
        return LookInput;
    }

    // Calculate aspect ratio (width/height)
    float AspectRatio = static_cast<float>(ViewportSizeX) / static_cast<float>(ViewportSizeY);

    // Normalize X (horizontal) input by aspect ratio
    // Wider screens produce larger horizontal deltas; dividing by aspect ratio
    // compensates for this, making horizontal/vertical sensitivity feel balanced
    // Y input remains unchanged as vertical deltas are consistent across aspect ratios
    FVector2D NormalizedInput = LookInput;
    NormalizedInput.X /= AspectRatio;

    UE_LOG(LogAdastreaInput, Verbose, TEXT("NormalizeLookInput - RAW: X=%.2f Y=%.2f -> NORMALIZED: X=%.2f Y=%.2f (AspectRatio=%.2f)"),
        LookInput.X, LookInput.Y, NormalizedInput.X, NormalizedInput.Y, AspectRatio);

    return NormalizedInput;
}

void ASpaceship::Look(const FInputActionValue& Value)
{
    // Skip normal look behavior when free look is active
    if (bFreeLookActive)
    {
        return;
    }

    // Pause mouse-look when the player controller is in targeting mode (bLockMouseLook).
    if (APlayerController* LookPC = Cast<APlayerController>(GetController()))
    {
        if (AAdastreaPlayerController* AdPC = Cast<AAdastreaPlayerController>(LookPC))
        {
            if (AdPC->bLockMouseLook)
            {
                return;
            }
        }
    }

    // Get the 2D vector input (mouse X/Y)
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    // Normalize input relative to viewport aspect ratio to compensate for
    // larger horizontal deltas on wide screens, ensuring consistent feel
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        LookAxisVector = NormalizeLookInputByAspectRatio(LookAxisVector, PC);
    }

    // Yaw (mouse X)
    Turn(LookAxisVector.X);

    // Pitch (mouse Y)
    LookUp(LookAxisVector.Y);
}

// Legacy input functions - called by Enhanced Input Move/Look functions
void ASpaceship::MoveForward(float Value)
{
    // Store input for smooth interpolation
    ForwardInput = Value;

    // X4-style: With flight assist, WASD provides direct velocity control
    // Without flight assist, it applies Newtonian acceleration to velocity
    if (bFlightAssistEnabled)
    {
        // Flight assist mode: direct velocity control
        AddMovementInput(GetActorForwardVector(), Value);
    }
    else
    {
        // No flight assist: apply Newtonian acceleration
        // Increment velocity in forward direction based on input
        if (GetWorld() && MovementComponent)
        {
            FVector AccelerationVector = GetActorForwardVector() * Value * DefaultAcceleration * GetWorld()->GetDeltaSeconds();
            MovementComponent->Velocity += AccelerationVector;
        }
    }

    // Update particle throttle based on forward movement
    if (ParticleComponent)
    {
        ParticleComponent->UpdateThrottle(FMath::Abs(Value));

        // Activate RCS thrusters based on direction
        if (Value > 0.0f)
        {
            ParticleComponent->ActivateRCSThruster(ERCSThrusterAxis::Backward, Value);
        }
        else if (Value < 0.0f)
        {
            ParticleComponent->ActivateRCSThruster(ERCSThrusterAxis::Forward, FMath::Abs(Value));
        }
    }
}

void ASpaceship::MoveRight(float Value)
{
    // Store input for smooth interpolation
    RightInput = Value;

    // X4-style: strafe is independent from forward motion
    // Apply strafe independence factor for realistic feel
    float StrafeValue = Value * StrafeIndependence;
    AddMovementInput(GetActorRightVector(), StrafeValue);

    // Activate RCS thrusters for strafing
    if (ParticleComponent)
    {
        if (Value > 0.0f)
        {
            ParticleComponent->ActivateRCSThruster(ERCSThrusterAxis::Left, Value);
        }
        else if (Value < 0.0f)
        {
            ParticleComponent->ActivateRCSThruster(ERCSThrusterAxis::Right, FMath::Abs(Value));
        }
    }
}

void ASpaceship::MoveUp(float Value)
{
    // Store input for smooth interpolation
    UpInput = Value;

    // X4-style: vertical strafe is independent from forward motion
    float StrafeValue = Value * StrafeIndependence;
    AddMovementInput(GetActorUpVector(), StrafeValue);

    // Activate RCS thrusters for vertical movement
    if (ParticleComponent)
    {
        if (Value > 0.0f)
        {
            ParticleComponent->ActivateRCSThruster(ERCSThrusterAxis::Down, Value);
        }
        else if (Value < 0.0f)
        {
            ParticleComponent->ActivateRCSThruster(ERCSThrusterAxis::Up, FMath::Abs(Value));
        }
    }
}

void ASpaceship::Turn(float Value)
{
    // Store input for smooth rotation
    YawInput = Value;

    if (GetWorld())
    {
        const float DeltaSeconds = GetWorld()->GetDeltaSeconds();

        if (bFlightAssistEnabled)
        {
            // Get ship-specific rotation rate multiplier from data asset
            float ShipRotationMultiplier = 1.0f;
            if (ShipDataAsset)
            {
                ShipRotationMultiplier = ShipDataAsset->RotationRateMultiplier;
            }

            // Mouse position flight is now handled in UpdateMousePositionFlight() called from Tick()
            // This function only handles mouse delta mode (when bUseMousePositionFlight is false)
            if (!bUseMousePositionFlight)
            {
                // Original X4-style smooth rotation with damping (mouse delta mode)
                                // Apply mouse flight sensitivity and ship rotation multiplier
                                float RotationRate = Value * TurnRate * ShipRotationMultiplier * MouseFlightSensitivity;

                                UE_LOG(LogAdastreaInput, Verbose, TEXT("ASpaceship::Turn - YawInput=%.2f, RotationRate=%.2f"),
                                    Value, RotationRate);

                                // Simple mouse-look: apply rotation directly (no interpolation lag).
                                // AddActorWorldRotation takes degrees/sec * deltaSeconds.
                                AddActorWorldRotation(FRotator(0.0f, RotationRate * DeltaSeconds, 0.0f));
                            }
                            // else: Mouse position mode - rotation handled in UpdateMousePositionFlight()
        }
        else
        {
            // Without flight assist: direct rotation
            float ShipRotationMultiplier = 1.0f;
            if (ShipDataAsset)
            {
                ShipRotationMultiplier = ShipDataAsset->RotationRateMultiplier;
            }

            FRotator DeltaRotation = FRotator(0.0f, Value * TurnRate * ShipRotationMultiplier * DeltaSeconds, 0.0f);
            AddActorWorldRotation(DeltaRotation);
        }
    }
}

void ASpaceship::LookUp(float Value)
{
    // Store input for smooth rotation
    PitchInput = Value;

    if (GetWorld())
    {
        const float DeltaSeconds = GetWorld()->GetDeltaSeconds();

        if (bFlightAssistEnabled)
        {
            // Get ship-specific rotation rate multiplier from data asset
            float ShipRotationMultiplier = 1.0f;
            if (ShipDataAsset)
            {
                ShipRotationMultiplier = ShipDataAsset->RotationRateMultiplier;
            }

            // Mouse position flight is now handled in UpdateMousePositionFlight() called from Tick()
            // This function only handles mouse delta mode (when bUseMousePositionFlight is false)
            if (!bUseMousePositionFlight)
            {
                // Original X4-style smooth rotation with damping (mouse delta mode)
                                float RotationRate = Value * TurnRate * ShipRotationMultiplier * MouseFlightSensitivity;

                                UE_LOG(LogAdastreaInput, Verbose, TEXT("ASpaceship::LookUp - PitchInput=%.2f, RotationRate=%.2f"),
                                    Value, RotationRate);

                                // Simple mouse-look: apply rotation directly (no interpolation lag).
                                AddActorWorldRotation(FRotator(RotationRate * DeltaSeconds, 0.0f, 0.0f));
                            }
                            // else: Mouse position mode - rotation handled in UpdateMousePositionFlight()
        }
        else
        {
            // Without flight assist: direct rotation
            float ShipRotationMultiplier = 1.0f;
            if (ShipDataAsset)
            {
                ShipRotationMultiplier = ShipDataAsset->RotationRateMultiplier;
            }

            FRotator DeltaRotation = FRotator(Value * TurnRate * ShipRotationMultiplier * DeltaSeconds, 0.0f, 0.0f);
            AddActorWorldRotation(DeltaRotation);
        }
    }
}

void ASpaceship::Roll(float Value)
{
    // Store input for smooth rotation
    RollInput = Value;

    if (GetWorld())
    {
        const float DeltaSeconds = GetWorld()->GetDeltaSeconds();

        // Get ship-specific rotation rate multiplier from data asset
        float ShipRotationMultiplier = 1.0f;
        if (ShipDataAsset)
        {
            ShipRotationMultiplier = ShipDataAsset->RotationRateMultiplier;
        }

        if (bFlightAssistEnabled)
        {
            // X4-style: smooth rotation with damping
            float RotationRate = Value * TurnRate * ShipRotationMultiplier * MouseFlightSensitivity;

            // Interpolate rotation velocity for smooth feel
            RotationVelocity.Roll = FMath::FInterpTo(RotationVelocity.Roll, RotationRate, DeltaSeconds, FlightAssistResponsiveness);

            // Apply roll rotation to actor in local space to avoid gimbal lock
            FRotator DeltaRotation = FRotator(0.0f, 0.0f, RotationVelocity.Roll * DeltaSeconds);
            AddActorLocalRotation(DeltaRotation);
        }
        else
        {
            // Without flight assist: direct rotation
            FRotator DeltaRotation = FRotator(0.0f, 0.0f, Value * TurnRate * ShipRotationMultiplier * DeltaSeconds);
            AddActorLocalRotation(DeltaRotation);
        }
    }
}

void ASpaceship::BeginControl(APlayerController* PC, APawn* ExternalPawn)
{
    if (!PC || !ExternalPawn)
    {
        return;
    }

    // Check if already controlling - prevent overwriting saved pawn
    if (SavedExternalPawn != nullptr)
    {
        return;
    }

    // Store the walking pawn so we can re-possess later
    SavedExternalPawn = ExternalPawn;

    // Disable walking pawn input and hide it
    ExternalPawn->DisableInput(PC);
    ExternalPawn->SetActorHiddenInGame(true);
    ExternalPawn->SetActorEnableCollision(false);

    // Possess the ship
    PC->Possess(this);

    // Ensure movement component is active
    if (MovementComponent)
    {
        MovementComponent->Activate(true);
    }
}

void ASpaceship::EndControl(APlayerController* PC)
{
    if (!PC || !SavedExternalPawn)
    {
        return;
    }

    // Possess back the walking pawn
    PC->Possess(SavedExternalPawn);

    // Restore walking pawn
    SavedExternalPawn->SetActorHiddenInGame(false);
    SavedExternalPawn->SetActorEnableCollision(true);
    SavedExternalPawn->EnableInput(PC);

    // Optionally teleport player to an exit point
    if (InteriorInstance.IsValid())
    {
        FVector ExitLocation = InteriorInstance->GetExitLocation();
        SavedExternalPawn->SetActorLocation(ExitLocation, false, nullptr, ETeleportType::TeleportPhysics);
    }

    SavedExternalPawn = nullptr;
}

void ASpaceship::ShowHUDAlert(const FText& Message, float Duration, bool bIsWarning)
{
    // Get the player controller
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        return;
    }

    // Try to find the HUD widget
    UAdastreaHUDWidget* HUDWidget = nullptr;

    // First check if we have a cached reference to the HUD widget
    // (This would need to be set up when the HUD is created)

    // For now, we'll search for it in the viewport
    TArray<UUserWidget*> FoundWidgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UAdastreaHUDWidget::StaticClass(), false);

    if (FoundWidgets.Num() > 0)
    {
        HUDWidget = Cast<UAdastreaHUDWidget>(FoundWidgets[0]);
    }

    if (HUDWidget)
    {
        // Call the HUD widget's ShowAlert function
        HUDWidget->ShowAlert(Message, Duration, bIsWarning);
    }
    else
    {
        // Fallback: log to screen if HUD widget not found
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, Duration, bIsWarning ? FColor::Red : FColor::Yellow, Message.ToString());
        }

        // Also log to output log
        UE_LOG(LogAdastrea, Log, TEXT("HUD Alert: %s"), *Message.ToString());
    }
}

FText ASpaceship::GetShipName() const
{
    // If we have a data asset, use its name
    if (ShipDataAsset)
    {
        return ShipDataAsset->ShipName;
    }

    // Otherwise, use the actor's label or name
    FString ActorName = GetName();

    return FText::FromString(ActorName);
}

FText ASpaceship::GetShipClass() const
{
    // If we have a data asset, use its class
    if (ShipDataAsset)
    {
        return ShipDataAsset->ShipClass;
    }

    // Default fallback
    return FText::FromString("Starship");
}

void ASpaceship::AttachShipWindows()
{
    if (!ShipMeshComponent || !ShipMeshComponent->GetStaticMesh() || WindowMeshComponent)
    {
        return;
    }

    // SM_Ship_Battleship_01_Assembled[_UniqueUV] -> SM_Ship_Battleship_01_Windows
    FString Base = ShipMeshComponent->GetStaticMesh()->GetName();
    const int32 Cut = Base.Find(TEXT("_Assembled"));
    if (Cut == INDEX_NONE)
    {
        return;
    }
    Base.LeftInline(Cut);
    const FString WinName = Base + TEXT("_Windows");
    const FString WinPath = FString::Printf(TEXT("/AdastreaShips/Meshes/Ships/%s.%s"), *WinName, *WinName);
    UStaticMesh* WinMesh = LoadObject<UStaticMesh>(nullptr, *WinPath, nullptr, LOAD_NoWarn | LOAD_Quiet);
    if (!WinMesh)
    {
        return;
    }

    WindowMeshComponent = NewObject<UStaticMeshComponent>(this, TEXT("ShipWindows"));
    WindowMeshComponent->SetStaticMesh(WinMesh);
    WindowMeshComponent->SetupAttachment(ShipMeshComponent);
    WindowMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WindowMeshComponent->SetCastShadow(false);
    WindowMeshComponent->SetCanEverAffectNavigation(false);
    WindowMeshComponent->RegisterComponent();
    AddInstanceComponent(WindowMeshComponent);
    UE_LOG(LogAdastreaShips, Log, TEXT("AttachShipWindows: %s gets %s"), *GetName(), *WinName);
}

namespace ChaseFraming
{
    /** How much of the screen's half-height the hull's silhouette (seen from behind) fills. */
    constexpr float Fill = 0.45f;
    /** How far the camera sits above the ship's plane, looking down at it (deg). */
    constexpr float Elevation = 12.0f;
    /** Where the hull centre lands below screen centre, as a fraction of the half-height
     * (0.4 puts it about 70% of the way down: bottom middle). */
    constexpr float ScreenDrop = 0.4f;
}

void ASpaceship::FitCameraToHull()
{
    if (!CameraSpringArm || !Camera)
    {
        return;
    }

    // Same rule for every ship: pivot on the hull's real centre (hull pivots are not
    // centred, and BP_CommandXL / BP_Super add their hull in the Blueprint), back off
    // in proportion to its size, and aim so the hull sits in the bottom middle.
    const FBox Hull = GetHullLocalBounds();
    if (!Hull.IsValid)
    {
        return;
    }
    const FVector Ext = Hull.GetExtent();

    FTransform Frame = ShipRoot->GetComponentTransform();
    Frame.SetScale3D(FVector::OneVector);
    CameraSpringArm->SetWorldLocation(Frame.TransformPosition(Hull.GetCenter()));

    // UE holds the horizontal FOV, so the vertical half-angle follows from the aspect.
    const float TanHalfV = FMath::Tan(FMath::DegreesToRadians(Camera->FieldOfView * 0.5f))
        / FMath::Max(Camera->AspectRatio, 1.0f);
    const float ElevRad = FMath::DegreesToRadians(ChaseFraming::Elevation);

    // Size by what the camera sees from behind (beam, and height plus the deck tilted
    // into view), measured at the stern, so long and wide hulls fill the same share.
    const float Silhouette = FMath::Max3<float>(Ext.Y, Ext.Z + Ext.X * FMath::Sin(ElevRad), 100.0f);
    const float Dist = Ext.X + Silhouette / (ChaseFraming::Fill * TanHalfV);
    CameraSpringArm->TargetArmLength = Dist * FMath::Cos(ElevRad);
    CameraSpringArm->SocketOffset = FVector(0.0f, 0.0f, Dist * FMath::Sin(ElevRad));

    const float DropDeg = FMath::RadiansToDegrees(FMath::Atan(ChaseFraming::ScreenDrop * TanHalfV));
    Camera->SetRelativeRotation(FRotator(DropDeg - ChaseFraming::Elevation, 0.0f, 0.0f));

    UE_LOG(LogAdastreaShips, Log, TEXT("FitCameraToHull: %s extent %s centre %s -> arm %.0f"),
        *GetName(), *Ext.ToString(), *Hull.GetCenter().ToString(), CameraSpringArm->TargetArmLength);
}

FBox ASpaceship::GetHullLocalBounds() const
{
    FTransform Frame = ShipRoot->GetComponentTransform();
    Frame.SetScale3D(FVector::OneVector);

    FBox Box(ForceInit);
    TArray<UStaticMeshComponent*> Meshes;
    GetComponents<UStaticMeshComponent>(Meshes);
    for (const UStaticMeshComponent* Mesh : Meshes)
    {
        if (Mesh && Mesh->GetStaticMesh() && Mesh->IsVisible() && !Mesh->bHiddenInGame)
        {
            Box += Mesh->CalcBounds(Mesh->GetComponentTransform().GetRelativeTransform(Frame)).GetBox();
        }
    }
    return Box;
}

float ASpaceship::GetHullHalfLength() const
{
    const FBox Box = GetHullLocalBounds();
    return Box.IsValid ? Box.GetExtent().GetMax() : 0.0f;
}

namespace SwapShowcase
{
    constexpr float Duration = 4.0f;
    /** How far the arm pulls out at the middle of the orbit, as a multiple of its length. */
    constexpr float PullOut = 0.8f;
    /** Camera drop below the ship's plane at the middle of the orbit (deg). */
    constexpr float Pitch = 18.0f;
}

void ASpaceship::PlaySwapShowcase()
{
    if (!CameraSpringArm)
    {
        return;
    }
    SwapShowcaseTime = 0.0f;
    SwapShowcaseArmLength = CameraSpringArm->TargetArmLength;
    // The orbit is faster than the rotation lag; it would trail the whole way round.
    CameraSpringArm->bEnableCameraRotationLag = false;
}

void ASpaceship::TickSwapShowcase(float DeltaTime)
{
    if (SwapShowcaseTime < 0.0f || !CameraSpringArm)
    {
        return;
    }

    SwapShowcaseTime += DeltaTime;
    // Free look takes the camera; or the orbit is done. Either way, hand it back.
    if (bFreeLookActive || SwapShowcaseTime >= SwapShowcase::Duration)
    {
        SwapShowcaseTime = -1.0f;
        CameraSpringArm->TargetArmLength = SwapShowcaseArmLength;
        CameraSpringArm->bEnableCameraRotationLag = true;
        if (!bFreeLookActive)
        {
            CameraSpringArm->SetRelativeRotation(FRotator::ZeroRotator);
        }
        return;
    }

    // One eased turn, starting and ending behind the ship; pulled out and dipped mid-way.
    const float Alpha = SwapShowcaseTime / SwapShowcase::Duration;
    const float Turn = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2.0f);
    const float Swell = FMath::Sin(Alpha * PI);
    CameraSpringArm->TargetArmLength = SwapShowcaseArmLength * (1.0f + SwapShowcase::PullOut * Swell);
    CameraSpringArm->SetRelativeRotation(FRotator(-SwapShowcase::Pitch * Swell, Turn * 360.0f, 0.0f));
}

void ASpaceship::ApplyShipHullMaterial()
{
    if (!ShipMeshComponent || !ShipMeshComponent->GetStaticMesh())
    {
        return;
    }

    if (HullMaterialOverride.IsNull() && ShipMeshComponent->GetStaticMesh()->GetName().Contains(TEXT("UniqueUV")))
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ApplyShipHullMaterial: %s uses unique-UV hull %s but has no HullMaterialOverride; the tiled class material will not line up."),
            *GetName(), *ShipMeshComponent->GetStaticMesh()->GetName());
    }

    UMaterialInterface* Mat = ResolveHullMaterial(GetName(), HullMaterialOverride);
    if (!Mat)
    {
        return;
    }
    ShipMeshComponent->SetMaterial(0, Mat);
    UE_LOG(LogAdastreaShips, Log, TEXT("ApplyShipHullMaterial: %s hull set to %s"),
        *GetName(), *Mat->GetName());
}

UMaterialInterface* ASpaceship::ResolveHullMaterial(const FString& ShipName, const TSoftObjectPtr<UMaterialInterface>& Override)
{
    // A unique-UV baked hull carries its own material; the tiled class material
    // below would be mapped across the wrong UVs.
    if (!Override.IsNull())
    {
        if (UMaterialInterface* OverrideMat = Override.LoadSynchronous())
        {
            return OverrideMat;
        }
        UE_LOG(LogAdastreaShips, Warning, TEXT("ResolveHullMaterial: HullMaterialOverride %s on %s failed to load; using class material."),
            *Override.ToString(), *ShipName);
    }

    // Map the ship's class to its imported hull material (assets agent authored
    // these at /Game/Materials). Default to a generic hull if unknown.
    FString HullMat = TEXT("/Game/Materials/M_Fighter_Hull");
    const FString& ActorName = ShipName;
    if (ActorName.Contains(TEXT("Freighter")))
    {
        HullMat = TEXT("/Game/Materials/M_Freighter_Hull");
    }
    else if (ActorName.Contains(TEXT("Corvette")))
    {
        HullMat = TEXT("/Game/Materials/M_Corvette_Hull");
    }
    else if (ActorName.Contains(TEXT("Gunship")))
    {
        HullMat = TEXT("/Game/Materials/M_Gunship_Hull");
    }
    else if (ActorName.Contains(TEXT("Cruiser")))
    {
        HullMat = TEXT("/Game/Materials/M_Cruiser_Hull");
    }
    else if (ActorName.Contains(TEXT("Destroyer")))
    {
        // Was TEXT("Destroyer") || TEXT("Miner") -> M_Miner_Hull, i.e. the
        // destroyer wore the mining ship's material (and, separately, its
        // MESH too -- see BP_Ship_Destroyer's ShipMeshComponent, fixed
        // 2026-09-15 to point at its own already-generated SM_Ship_Destroyer_
        // 01_Assembled instead). Split into its own dedicated hull now that
        // T_Destroyer_* textures exist (Tools/build_fleet_hull_textures.py).
        HullMat = TEXT("/Game/Materials/M_Destroyer_Hull");
    }
    else if (ActorName.Contains(TEXT("Miner")))
    {
        HullMat = TEXT("/Game/Materials/M_Miner_Hull");
    }
    else if (ActorName.Contains(TEXT("Battleship")))
    {
        // Previously unhandled -> fell through to the M_Fighter_Hull default,
        // wrong for a capital ship. T_Battleship_* textures added alongside
        // Destroyer's (Tools/build_fleet_hull_textures.py).
        HullMat = TEXT("/Game/Materials/M_Battleship_Hull");
    }
    else if (ActorName.Contains(TEXT("Fighter")))
    {
        HullMat = TEXT("/Game/Materials/M_Fighter_Hull");
    }
    else if (ActorName.Contains(TEXT("Carrier")))
    {
        HullMat = TEXT("/Game/Materials/M_Carrier_Hull");
    }
    else if (ActorName.Contains(TEXT("Command")))
    {
        HullMat = TEXT("/Game/Materials/M_Command_Hull");
    }
    else if (ActorName.Contains(TEXT("Frigate")))
    {
        HullMat = TEXT("/Game/Materials/M_Frigate_Hull");
    }
    else if (ActorName.Contains(TEXT("Luxury")))
    {
        HullMat = TEXT("/Game/Materials/M_Luxury_Hull");
    }
    else if (ActorName.Contains(TEXT("Mining")))
    {
        HullMat = TEXT("/Game/Materials/M_Mining_Hull");
    }
    else if (ActorName.Contains(TEXT("Patrol")))
    {
        HullMat = TEXT("/Game/Materials/M_Patrol_Hull");
    }
    else if (ActorName.Contains(TEXT("Science")))
    {
        HullMat = TEXT("/Game/Materials/M_Science_Hull");
    }
    else if (ActorName.Contains(TEXT("Trading")))
    {
        HullMat = TEXT("/Game/Materials/M_Trading_Hull");
    }
    else if (ActorName.Contains(TEXT("Behemoth")))
    {
        HullMat = TEXT("/Game/Materials/M_Transport_Behemoth_Hull");
    }
    else if (ActorName.Contains(TEXT("Genesis")))
    {
        HullMat = TEXT("/Game/Materials/M_Transport_Genesis_Hull");
    }
    else if (ActorName.Contains(TEXT("Utility")))
    {
        HullMat = TEXT("/Game/Materials/M_Utility_Hull");
    }

    UMaterialInterface* Mat = LoadObject<UMaterialInterface>(nullptr, *HullMat);
    if (!Mat)
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ResolveHullMaterial: material %s not found."), *HullMat);
    }
    return Mat;
}

float ASpaceship::GetCurrentHullIntegrity() const
{
    return CurrentHullIntegrity;
}

float ASpaceship::GetMaxHullIntegrity() const
{
    return MaxHullIntegrity;
}

float ASpaceship::GetCurrentSpeed() const
{
    // Use the movement component's velocity when present, else the tracked velocity.
    if (MovementComponent)
    {
        return MovementComponent->Velocity.Size();
    }
    return CurrentVelocity.Size();
}

bool ASpaceship::CanLeaveCockpit(FString& OutReason) const
{
    // Cannot leave while actively docking/undocking.
    if (bIsDocking)
    {
        OutReason = TEXT("Cannot leave cockpit while docking.");
        return false;
    }

    // Cannot leave while the ship is moving.
    if (!IsStationary())
    {
        const int32 Speed = FMath::RoundToInt(GetCurrentSpeed());
        OutReason = FString::Printf(TEXT("Cannot leave cockpit while moving (%d u/s). Stop the ship first."), Speed);
        return false;
    }

    // Cannot leave while in combat.
    if (bIsInCombat)
    {
        OutReason = TEXT("Cannot leave cockpit while in combat.");
        return false;
    }

    // Cannot leave while on autopilot.
    if (bIsOnAutopilot)
    {
        OutReason = TEXT("Cannot leave cockpit while autopilot is engaged.");
        return false;
    }

    OutReason = TEXT("");
    return true;
}

// ==========================================
// X4-STYLE FLIGHT CONTROL IMPLEMENTATION
// ==========================================

void ASpaceship::ToggleFlightAssist()
{
    bFlightAssistEnabled = !bFlightAssistEnabled;

    // When disabling flight assist, preserve current velocity for inertia
    if (!bFlightAssistEnabled && MovementComponent)
    {
        CurrentVelocity = MovementComponent->Velocity;
    }
}

bool ASpaceship::CanAdjustThrottle()
{
    // Rate limit throttle adjustments to prevent excessively fast changes when button is held
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    float CurrentTime = World->GetTimeSeconds();
    if (CurrentTime - LastThrottleAdjustmentTime < ThrottleAdjustmentCooldown)
    {
        return false; // Too soon, skip this adjustment
    }

    LastThrottleAdjustmentTime = CurrentTime;
    return true;
}

void ASpaceship::ThrottleUp()
{
    if (!CanAdjustThrottle())
    {
        return;
    }

    ThrottlePercentage = FMath::Clamp(ThrottlePercentage + ThrottleStep, 0.0f, 100.0f);
}

void ASpaceship::ThrottleDown()
{
    if (!CanAdjustThrottle())
    {
        return;
    }

    ThrottlePercentage = FMath::Clamp(ThrottlePercentage - ThrottleStep, 0.0f, 100.0f);
}

void ASpaceship::SetThrottle(float Percentage)
{
    ThrottlePercentage = FMath::Clamp(Percentage, 0.0f, 100.0f);
}

void ASpaceship::ActivateBoost()
{
    // Cannot boost in travel mode
    if (!bTravelModeActive)
    {
        bBoostActive = true;

        // Update movement component max speed
        if (MovementComponent)
        {
            MovementComponent->MaxSpeed = DefaultMaxSpeed * BoostMultiplier;
        }
    }
}

void ASpaceship::DeactivateBoost()
{
    bBoostActive = false;

    // Restore normal max speed
    if (MovementComponent)
    {
        MovementComponent->MaxSpeed = bTravelModeActive ? (DefaultMaxSpeed * TravelModeMultiplier) : DefaultMaxSpeed;
    }
}

void ASpaceship::ToggleTravelMode()
{
    bTravelModeActive = !bTravelModeActive;

    // Deactivate boost when entering travel mode
    if (bTravelModeActive && bBoostActive)
    {
        DeactivateBoost();
    }

    // Update movement component max speed
    if (MovementComponent)
    {
        if (bTravelModeActive)
        {
            MovementComponent->MaxSpeed = DefaultMaxSpeed * TravelModeMultiplier;
            // Set throttle to 100% for travel mode
            ThrottlePercentage = 100.0f;
        }
        else
        {
            MovementComponent->MaxSpeed = DefaultMaxSpeed;
        }
    }
}

void ASpaceship::ApplyDataAssetMobility()
{
    if (!ShipDataAsset)
    {
        return;
    }

    if (ShipDataAsset->MaxSpeed > 0.0f)
    {
        DefaultMaxSpeed = ShipDataAsset->MaxSpeed * DataAssetSpeedScale;
    }
    if (ShipDataAsset->Acceleration > 0.0f)
    {
        // Braking matches thrust, so a heavy hull doesn't stop on a dime.
        DefaultAcceleration = ShipDataAsset->Acceleration * DataAssetAccelerationScale;
        DefaultDeceleration = DefaultAcceleration;
    }

    if (MovementComponent)
    {
        MovementComponent->MaxSpeed = GetEffectiveMaxSpeed();
        MovementComponent->Acceleration = DefaultAcceleration;
        MovementComponent->Deceleration = DefaultDeceleration;
    }

    UE_LOG(LogAdastreaShips, Log, TEXT("ApplyDataAssetMobility %s (%s): speed %.0f accel %.0f"),
        *GetName(), *ShipDataAsset->GetName(), DefaultMaxSpeed, DefaultAcceleration);
}

float ASpaceship::GetEffectiveMaxSpeed() const
{
    float EffectiveSpeed = DefaultMaxSpeed;

    if (bBoostActive)
    {
        EffectiveSpeed *= BoostMultiplier;
    }
    else if (bTravelModeActive)
    {
        EffectiveSpeed *= TravelModeMultiplier;
    }

    return EffectiveSpeed;
}

namespace
{
    /** The ship's current (pre-upgrade at BeginPlay) value of an upgradeable stat. */
    float ReadShipStat(const ASpaceship& Ship, FName StatName)
    {
        if (StatName == ShipUpgradeStats::MaxSpeed)        { return Ship.DefaultMaxSpeed; }
        if (StatName == ShipUpgradeStats::Acceleration)    { return Ship.DefaultAcceleration; }
        if (StatName == ShipUpgradeStats::BoostMultiplier) { return Ship.BoostMultiplier; }
        if (StatName == ShipUpgradeStats::HullStrength)    { return Ship.MaxHullIntegrity; }
        if (StatName == ShipUpgradeStats::ShieldStrength)  { return Ship.ShipDataAsset ? Ship.ShipDataAsset->ShieldStrength : 0.0f; }
        if (StatName == ShipUpgradeStats::CargoCapacity)   { return Ship.CargoComponent ? Ship.CargoComponent->CargoCapacity : 0.0f; }
        if (StatName == ShipUpgradeStats::WeaponDamage)    { return 1.0f; }
        if (StatName == ShipUpgradeStats::MiningPower)     { return Ship.DroneBay ? Ship.DroneBay->MiningPower : 0.0f; }
        return 0.0f;
    }

    const FName UpgradeableStats[] = {
        ShipUpgradeStats::MaxSpeed, ShipUpgradeStats::Acceleration, ShipUpgradeStats::BoostMultiplier,
        ShipUpgradeStats::HullStrength, ShipUpgradeStats::ShieldStrength, ShipUpgradeStats::CargoCapacity,
        ShipUpgradeStats::WeaponDamage, ShipUpgradeStats::MiningPower,
    };
}

void ASpaceship::RebaseUpgradeStats()
{
    UpgradeBaseStats.Reset();
    for (const FName& Stat : UpgradeableStats)
    {
        UpgradeBaseStats.Add(Stat, ReadShipStat(*this, Stat));
    }
    RecalculateUpgradedStats();
}

float ASpaceship::GetBaseStat(FName StatName) const
{
    const float* Base = UpgradeBaseStats.Find(StatName);
    return Base ? *Base : ReadShipStat(*this, StatName);
}

float ASpaceship::GetUpgradedStat(FName StatName) const
{
    const float Base = GetBaseStat(StatName);
    return UpgradeComponent ? UpgradeComponent->GetStatModifier(StatName, Base) : Base;
}

float ASpaceship::GetMaxShieldStrength() const
{
    return FMath::Max(0.0f, GetUpgradedStat(ShipUpgradeStats::ShieldStrength));
}

float ASpaceship::GetWeaponDamageMultiplier() const
{
    return FMath::Max(0.0f, GetUpgradedStat(ShipUpgradeStats::WeaponDamage));
}

void ASpaceship::RecalculateUpgradedStats()
{
    if (UpgradeBaseStats.Num() == 0)
    {
        // Before BeginPlay: RebaseUpgradeStats() applies everything once the base is known.
        return;
    }

    DefaultMaxSpeed = FMath::Max(0.0f, GetUpgradedStat(ShipUpgradeStats::MaxSpeed));
    DefaultAcceleration = FMath::Max(0.0f, GetUpgradedStat(ShipUpgradeStats::Acceleration));
    BoostMultiplier = FMath::Max(1.0f, GetUpgradedStat(ShipUpgradeStats::BoostMultiplier));
    if (MovementComponent)
    {
        MovementComponent->Acceleration = DefaultAcceleration;
        MovementComponent->MaxSpeed = GetEffectiveMaxSpeed();
    }

    const float OldMaxHull = MaxHullIntegrity;
    MaxHullIntegrity = FMath::Max(1.0f, GetUpgradedStat(ShipUpgradeStats::HullStrength));
    CurrentHullIntegrity = OldMaxHull > 0.0f
        ? FMath::Clamp(CurrentHullIntegrity * (MaxHullIntegrity / OldMaxHull), 0.0f, MaxHullIntegrity)
        : MaxHullIntegrity;

    if (CargoComponent)
    {
        CargoComponent->CargoCapacity = FMath::Max(0.0f, GetUpgradedStat(ShipUpgradeStats::CargoCapacity));
    }
    if (DroneBay)
    {
        DroneBay->MiningPower = FMath::Max(0.0f, GetUpgradedStat(ShipUpgradeStats::MiningPower));
    }

    UE_LOG(LogAdastreaShips, Log, TEXT("RecalculateUpgradedStats %s: speed %.0f accel %.0f boost x%.2f hull %.0f shield %.0f cargo %.0f weapons x%.2f mining %.1f"),
        *GetName(), DefaultMaxSpeed, DefaultAcceleration, BoostMultiplier, MaxHullIntegrity, GetMaxShieldStrength(),
        CargoComponent ? CargoComponent->CargoCapacity : 0.0f, GetWeaponDamageMultiplier(), DroneBay ? DroneBay->MiningPower : 0.0f);
}

bool ASpaceship::CanRemoveUpgrade(const UShipUpgradeDataAsset* Upgrade, FText& OutReason) const
{
    if (!Upgrade || !UpgradeComponent || !CargoComponent)
    {
        return true;
    }

    // A smaller hold must still fit what's in it.
    const float Used = CargoComponent->CargoCapacity - CargoComponent->GetAvailableCargoSpace();
    const float NewCapacity = UpgradeComponent->PreviewStat(ShipUpgradeStats::CargoCapacity,
        GetBaseStat(ShipUpgradeStats::CargoCapacity), nullptr, Upgrade->UpgradeID);
    if (Used > NewCapacity + KINDA_SMALL_NUMBER)
    {
        OutReason = FText::Format(FText::FromString(TEXT("Hold carries {0} units; without it only {1} fit")),
            FText::AsNumber(FMath::RoundToInt(Used)), FText::AsNumber(FMath::FloorToInt(NewCapacity)));
        return false;
    }
    return true;
}

ASpaceStation* ASpaceship::GetDockedStation() const
{
    if (!bIsDocked || !NearbyStation)
    {
        return nullptr;
    }
    if (ASpaceStation* Station = Cast<ASpaceStation>(NearbyStation->GetAttachParentActor()))
    {
        return Station;
    }
    return Cast<ASpaceStation>(NearbyStation->GetOwner());
}

void ASpaceship::ApplyFlightAssist(float DeltaTime)
{
    /**
     * X4-style Flight Assist Implementation:
     *
     * When flight assist is enabled:
     * - Ship maintains orientation when no rotation input is given
     * - Rotation is smoothly damped for natural feel
     * - Velocity is maintained when no movement input is given
     * - Ship responds to throttle setting rather than direct acceleration
     *
     * This creates the "fly-by-wire" feel of X4: Foundations where the ship
     * computer assists the pilot in maintaining stable flight.
     */

    if (!MovementComponent)
    {
        return;
    }

    // Apply rotation damping when no input
    if (FMath::IsNearlyZero(YawInput, 0.01f))
    {
        RotationVelocity.Yaw = FMath::FInterpTo(RotationVelocity.Yaw, 0.0f, DeltaTime, FlightAssistResponsiveness * RotationDampingFactor);
    }

    if (FMath::IsNearlyZero(PitchInput, 0.01f))
    {
        RotationVelocity.Pitch = FMath::FInterpTo(RotationVelocity.Pitch, 0.0f, DeltaTime, FlightAssistResponsiveness * RotationDampingFactor);
    }

    if (FMath::IsNearlyZero(RollInput, 0.01f))
    {
        RotationVelocity.Roll = FMath::FInterpTo(RotationVelocity.Roll, 0.0f, DeltaTime, FlightAssistResponsiveness * RotationDampingFactor);
    }

    // Preserve velocity when no movement input (inertia in space)
    if (FMath::IsNearlyZero(ForwardInput, 0.01f) &&
        FMath::IsNearlyZero(RightInput, 0.01f) &&
        FMath::IsNearlyZero(UpInput, 0.01f))
    {
        // In X4, with flight assist, the ship maintains its velocity
        // This is different from atmosphere flight where you'd slow down
        // Preserve the current velocity to maintain inertia
        if (!CurrentVelocity.IsNearlyZero())
        {
            MovementComponent->Velocity = CurrentVelocity;
        }
    }
    else
    {
        // Update CurrentVelocity when there is input
        CurrentVelocity = MovementComponent->Velocity;
    }
}

void ASpaceship::ApplyAutoLeveling(float DeltaTime)
{
    /**
     * X4-style Auto-Leveling:
     *
     * When no rotation input is given, the ship automatically levels its roll
     * to the ecliptic plane (assuming Z-up in Unreal). This makes it easier
     * to maintain orientation during exploration and combat.
     *
     * Note: Currently triggered by no yaw input as we don't have explicit roll input.
     * The strength can be tuned via AutoLevelStrength (0 = off, 1 = instant).
     */

    if (AutoLevelStrength <= 0.0f)
    {
        return;
    }

    // Get current rotation
    FRotator CurrentRotation = GetActorRotation();

    // Calculate target rotation with roll = 0 (level to ecliptic)
    FRotator TargetRotation = CurrentRotation;
    TargetRotation.Roll = 0.0f;

    // Smoothly interpolate to level position
    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, AutoLevelStrength * 2.0f);
    SetActorRotation(NewRotation);
}

void ASpaceship::UpdateThrottleVelocity(float DeltaTime)
{
    /**
     * X4-style Throttle System:
     *
     * The throttle (0-100%) controls the target velocity, not acceleration directly.
     * The ship will automatically accelerate or decelerate to match the throttle setting.
     *
     * This creates intuitive speed control where:
     * - Throttle 0% = ship comes to a stop
     * - Throttle 50% = ship maintains half speed
     * - Throttle 100% = ship maintains max speed
     *
     * The flight computer handles the acceleration curve automatically.
     */

    if (!MovementComponent)
    {
        return;
    }

    // Convert throttle (0-100) into a forward movement input magnitude (0..1).
    // UFloatingPawnMovement only produces motion through AddMovementInput, so we
    // drive it that way rather than writing Velocity directly (which the movement
    // component overrides each tick). This makes throttle actually move the ship.
    float ThrottleMagnitude = FMath::Clamp(ThrottlePercentage / 100.0f, 0.0f, 1.0f);

    // When throttle is above 0 and we aren't being given explicit forward input,
    // add forward movement input so the ship cruises at the set throttle.
    if (ThrottleMagnitude > 0.0f && FMath::IsNearlyZero(ForwardInput, 0.01f))
    {
        AddMovementInput(GetActorForwardVector(), ThrottleMagnitude);
    }

    // Interpolate a tracked velocity toward the throttle target for any systems
    // (particles, HUD) that read it, and clamp to max speed.
    float EffectiveMaxSpeed = GetEffectiveMaxSpeed();
    float TargetSpeed = ThrottleMagnitude * EffectiveMaxSpeed;
    FVector ForwardVector = GetActorForwardVector();
    TargetVelocity = ForwardVector * TargetSpeed;

    if (bFlightAssistEnabled)
    {
        float CurrentForwardSpeed = FVector::DotProduct(MovementComponent->Velocity, ForwardVector);
        float InterpSpeed = (CurrentForwardSpeed < TargetSpeed) ?
            (DefaultAcceleration / FMath::Max(EffectiveMaxSpeed, 1.0f)) * FlightAssistResponsiveness :
            (DefaultDeceleration / FMath::Max(EffectiveMaxSpeed, 1.0f)) * FlightAssistResponsiveness;
        FVector BlendedVelocity = FMath::VInterpTo(MovementComponent->Velocity, TargetVelocity, DeltaTime, InterpSpeed);
        MovementComponent->Velocity = BlendedVelocity;

        // Keep the "preserve inertia" snapshot in sync with this throttle-driven speed.
        // Without this, ApplyFlightAssist (which runs before this function each tick and
        // re-imposes CurrentVelocity whenever WASD is idle) re-asserts the OLD velocity
        // every frame, undoing this interpolation before it can compound — so throttling
        // down to 0% never actually slowed the ship; it just cruised forever at whatever
        // speed it last had when WASD was touched.
        CurrentVelocity = BlendedVelocity;
    }
}

void ASpaceship::UpdateMousePositionFlight(float DeltaTime)
{
    /**
     * X4-style Mouse Position Flight Implementation:
     *
     * This method is called every frame when mouse position flight is enabled.
     * Unlike mouse delta controls, this continuously checks the mouse cursor position
     * and rotates the ship based on where the cursor is on screen, not how the mouse moves.
     *
     * Key Behavior:
     * - If mouse is right of center → Ship continuously yaws right
     * - If mouse is left of center → Ship continuously yaws left
     * - If mouse is above center → Ship continuously pitches up
     * - If mouse is below center → Ship continuously pitches down
     * - Distance from center controls rotation speed (0% at deadzone, 100% at max radius)
     * - Mouse can remain stationary and ship will continue rotating
     *
     * This creates the intuitive feel of X4: Foundations where you "point" with the cursor.
     */

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        return;
    }

    // Get ship-specific rotation rate multiplier from data asset
    float ShipRotationMultiplier = 1.0f;
    if (ShipDataAsset)
    {
        ShipRotationMultiplier = ShipDataAsset->RotationRateMultiplier;
    }

    // Get mouse position and viewport size
    int32 ViewportSizeX, ViewportSizeY;
    float MouseX, MouseY;

    PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
    PC->GetMousePosition(MouseX, MouseY);

    // Calculate center of screen
    float CenterX = ViewportSizeX * 0.5f;
    float CenterY = ViewportSizeY * 0.5f;

    // Calculate distance from center
    float DeltaX = MouseX - CenterX;
    float DeltaY = MouseY - CenterY;
    float DistanceFromCenter = FMath::Sqrt(DeltaX * DeltaX + DeltaY * DeltaY);

    // Check if within deadzone (no rotation)
    if (DistanceFromCenter < MouseDeadzoneRadius)
    {
        // Within deadzone, smoothly stop rotation
        RotationVelocity.Yaw = FMath::FInterpTo(RotationVelocity.Yaw, 0.0f, DeltaTime, FlightAssistResponsiveness);
        RotationVelocity.Pitch = FMath::FInterpTo(RotationVelocity.Pitch, 0.0f, DeltaTime, FlightAssistResponsiveness);

        // No rotational input intent while inside deadzone
        YawInput = 0.0f;
        PitchInput = 0.0f;
        return;
    }

    // Calculate rotation speed based on distance from center (beyond deadzone)
    float EffectiveDistance = DistanceFromCenter - MouseDeadzoneRadius;
    float MaxEffectiveDistance = MouseMaxRadius - MouseDeadzoneRadius;

    // Protect against invalid configuration
    if (MaxEffectiveDistance <= KINDA_SMALL_NUMBER)
    {
        UE_LOG(LogAdastreaInput, Warning,
            TEXT("ASpaceship::UpdateMousePositionFlight - Invalid mouse radius configuration: MouseMaxRadius (%.2f) must be greater than MouseDeadzoneRadius (%.2f)."),
            MouseMaxRadius, MouseDeadzoneRadius);
        MaxEffectiveDistance = KINDA_SMALL_NUMBER;
    }

    float DistanceRatio = FMath::Clamp(EffectiveDistance / MaxEffectiveDistance, 0.0f, 1.0f);

    // Calculate rotation rates for yaw and pitch
    // Direction is normalized (-1 to 1) and multiplied by distance ratio for speed
    float DirectionX = DeltaX / FMath::Max(DistanceFromCenter, 0.1f);
    float DirectionY = -DeltaY / FMath::Max(DistanceFromCenter, 0.1f); // Inverted for natural pitch

    float YawRotationRate = DirectionX * DistanceRatio * TurnRate * ShipRotationMultiplier * MouseFlightSensitivity;
    float PitchRotationRate = DirectionY * DistanceRatio * TurnRate * ShipRotationMultiplier * MouseFlightSensitivity;

    UE_LOG(LogAdastreaInput, Verbose,
        TEXT("ASpaceship::UpdateMousePositionFlight - MousePos=(%.0f,%.0f), Center=(%.0f,%.0f), Distance=%.0f, DistanceRatio=%.2f, YawRate=%.2f, PitchRate=%.2f"),
        MouseX, MouseY, CenterX, CenterY, DistanceFromCenter, DistanceRatio, YawRotationRate, PitchRotationRate);

    // Interpolate rotation velocity for smooth feel
    RotationVelocity.Yaw = FMath::FInterpTo(RotationVelocity.Yaw, YawRotationRate, DeltaTime, FlightAssistResponsiveness);
    RotationVelocity.Pitch = FMath::FInterpTo(RotationVelocity.Pitch, PitchRotationRate, DeltaTime, FlightAssistResponsiveness);

    // Signal active rotation intent to prevent auto-leveling and damping interference
    YawInput = (FMath::Abs(YawRotationRate) > 0.01f) ? 1.0f : 0.0f;
    PitchInput = (FMath::Abs(PitchRotationRate) > 0.01f) ? 1.0f : 0.0f;

    // Apply rotation to ship
    FRotator DeltaRotation = FRotator(RotationVelocity.Pitch * DeltaTime, RotationVelocity.Yaw * DeltaTime, 0.0f);
    AddActorWorldRotation(DeltaRotation);
}

void ASpaceship::FreeLookStarted()
{
    // Check for double-click to reset camera
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    float TimeSinceLastClick = CurrentTime - LastFreeLookClickTime;

    // If this is a double-click (within threshold), reset camera and exit free look
    // Check if we're within the double-click window, regardless of free look state
    if (TimeSinceLastClick > 0.0f && TimeSinceLastClick <= DoubleClickThreshold)
    {
        UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Free look double-click detected - resetting camera"));

        // Immediately reset camera to ship forward
        if (CameraSpringArm)
        {
            CameraSpringArm->SetRelativeRotation(FRotator::ZeroRotator);
        }

        // Exit free look mode if active, or prevent activation if not yet active
        bFreeLookActive = false;
        FreeLookRotation = FRotator::ZeroRotator;
        LastFreeLookClickTime = 0.0f; // Reset to prevent triple-click issues
        return;
    }

    // Store the click time for double-click detection
    LastFreeLookClickTime = CurrentTime;

    // Normal free look activation
    bFreeLookActive = true;

    // Reset free look rotation for new activation
    FreeLookRotation = FRotator::ZeroRotator;

    UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Free look started"));
}

void ASpaceship::FreeLookCompleted()
{
    bFreeLookActive = false;

    // Reset camera to follow ship rotation
    if (CameraSpringArm)
    {
        // Smoothly return camera to ship's forward direction
        CameraSpringArm->SetRelativeRotation(FRotator::ZeroRotator);
    }

    UE_LOG(LogAdastreaInput, Log, TEXT("ASpaceship: Free look completed"));
}

void ASpaceship::FreeLookCamera(const FInputActionValue& Value)
{
    if (!bFreeLookActive || !CameraSpringArm)
    {
        return;
    }

    // Get the 2D vector input (mouse X/Y)
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    // Normalize input relative to viewport aspect ratio to compensate for
    // larger horizontal deltas on wide screens, ensuring consistent feel
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        LookAxisVector = NormalizeLookInputByAspectRatio(LookAxisVector, PC);
    }

    if (GetWorld())
    {
        const float DeltaSeconds = GetWorld()->GetDeltaSeconds();

        // Apply free look sensitivity to mouse input
        float YawDelta = LookAxisVector.X * FreeLookSensitivity * TurnRate * DeltaSeconds;
        float PitchDelta = LookAxisVector.Y * FreeLookSensitivity * TurnRate * DeltaSeconds;

        // Accumulate free look rotation
        FreeLookRotation.Yaw += YawDelta;
        FreeLookRotation.Pitch += PitchDelta;

        // Clamp pitch to prevent camera flipping
        FreeLookRotation.Pitch = FMath::Clamp(FreeLookRotation.Pitch, -89.0f, 89.0f);

        // Apply free look rotation relative to ship's current rotation
        FRotator NewCameraRotation = GetActorRotation() + FreeLookRotation;
        CameraSpringArm->SetWorldRotation(NewCameraRotation);
    }
}

// ==========================================
// DOCKING SYSTEM IMPLEMENTATION
// ==========================================

void ASpaceship::SetNearbyStation(ASpaceStationModule* Station)
{
    NearbyStation = Station;

#if DOCKING_DEBUG_ENABLED
    // Debug print
    if (GEngine)
    {
        if (Station)
        {
            AdastreaDockingDebug::Print(5.0f, FColor::Green, FString::Printf(TEXT("[DOCKING] Ship entered docking range of station: %s"), *Station->GetName()));
        }
        else
        {
            AdastreaDockingDebug::Print(5.0f, FColor::Yellow, TEXT("[DOCKING] Ship left docking range"));
        }
    }
#endif
}

bool ASpaceship::CanRequestDocking(float& OutDistance, FString& OutStationName) const
{
    OutDistance = 0.0f;
    OutStationName.Reset();

    const ADockingBayModule* DockingBay = Cast<ADockingBayModule>(NearbyStation);
    if (!DockingBay || bIsDocked || bIsDocking)
    {
        return false;
    }

    OutStationName = DockingBay->GetName();
    OutDistance = FVector::Dist(GetActorLocation(), DockingBay->GetActorLocation());

    if (!DockingBay->HasAvailableDocking())
    {
        return false;
    }

    const USceneComponent* DockingPoint = DockingBay->GetAvailableDockingPoint();
    if (!DockingPoint)
    {
        return false;
    }

    OutDistance = FVector::Dist(GetActorLocation(), DockingPoint->GetComponentLocation());
    return OutDistance <= GetEffectiveDockingRange();
}

void ASpaceship::ShowDockingPrompt(bool bShow)
{
    if (bShow)
    {
        // Get effective widget class (from settings or fallback)
        TSubclassOf<UUserWidget> EffectiveWidgetClass = GetEffectiveDockingPromptWidgetClass();

        // Create widget if it doesn't exist
        if (!DockingPromptWidget && EffectiveWidgetClass)
        {
            APlayerController* PC = Cast<APlayerController>(GetController());
            if (PC)
            {
                DockingPromptWidget = CreateWidget<UUserWidget>(PC, EffectiveWidgetClass);
                if (DockingPromptWidget)
                {
                    DockingPromptWidget->AddToViewport();

#if DOCKING_DEBUG_ENABLED
                    // Debug print
                    if (GEngine)
                    {
                        AdastreaDockingDebug::Print(5.0f, FColor::Green, TEXT("[DOCKING] WBP_DockingPrompt created and added to viewport"));
                    }
#endif
                }
                else
                {
#if DOCKING_DEBUG_ENABLED
                    // Debug print - widget creation failed
                    if (GEngine)
                    {
                        AdastreaDockingDebug::Print(5.0f, FColor::Red, TEXT("[DOCKING] ERROR: Failed to create WBP_DockingPrompt widget"));
                    }
#endif
                }
            }
            else
            {
#if DOCKING_DEBUG_ENABLED
                // Debug print - no player controller
                if (GEngine)
                {
                    AdastreaDockingDebug::Print(5.0f, FColor::Red, TEXT("[DOCKING] ERROR: No player controller found for widget creation"));
                }
#endif
            }
        }
        else if (!EffectiveWidgetClass)
        {
            UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::ShowDockingPrompt - No DockingPromptWidgetClass set (neither in DockingSettings nor direct property) on '%s'. Docking prompt UI will not be shown."), *GetName());

#if DOCKING_DEBUG_ENABLED
            // Debug print - widget class not set
            if (GEngine)
            {
                AdastreaDockingDebug::Print(5.0f, FColor::Red, TEXT("[DOCKING] ERROR: DockingPromptWidgetClass not set in Blueprint or Settings"));
            }
#endif
        }

        // Show existing widget
        if (DockingPromptWidget)
        {
            DockingPromptWidget->SetVisibility(ESlateVisibility::Visible);

#if DOCKING_DEBUG_ENABLED
            // Debug print
            if (GEngine)
            {
                AdastreaDockingDebug::Print(3.0f, FColor::Cyan, TEXT("[DOCKING] Docking prompt now visible"));
            }
#endif
        }
    }
    else
    {
        // Hide widget
        if (DockingPromptWidget)
        {
            DockingPromptWidget->SetVisibility(ESlateVisibility::Collapsed);

#if DOCKING_DEBUG_ENABLED
            // Debug print
            if (GEngine)
            {
                AdastreaDockingDebug::Print(3.0f, FColor::Yellow, TEXT("[DOCKING] Docking prompt hidden"));
            }
#endif
        }
    }
}

void ASpaceship::RequestDocking()
{
    if (bWrecked)
    {
        return;
    }
    #if DOCKING_DEBUG_ENABLED

    // Debug print - function entry
    if (GEngine)
    {
        AdastreaDockingDebug::Print(5.0f, FColor::Cyan, TEXT("[DOCKING] RequestDocking() called - Player pressed docking key"));
    }

    #endif

    // Validate nearby station exists
    if (!NearbyStation)
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::RequestDocking - No station in range"));

        #if DOCKING_DEBUG_ENABLED


        // Debug print
        if (GEngine)
        {
            AdastreaDockingDebug::Print(5.0f, FColor::Red, TEXT("[DOCKING] ERROR: No station in range"));
        }


        #endif

        // Show user feedback via HUD message for "No station in range" error
        ShowHUDAlert(FText::FromString("No station in range"), 3.0f, true);
        return;
    }

    #if DOCKING_DEBUG_ENABLED


    // Debug print - station found
    if (GEngine)
    {
        AdastreaDockingDebug::Print(5.0f, FColor::Green, FString::Printf(TEXT("[DOCKING] Station in range: %s"), *NearbyStation->GetName()));
    }


    #endif

    // If already docked, undock instead
    if (bIsDocked)
    {
        #if DOCKING_DEBUG_ENABLED

        // Debug print
        if (GEngine)
        {
            AdastreaDockingDebug::Print(5.0f, FColor::Yellow, TEXT("[DOCKING] Already docked - calling Undock() instead"));
        }

        #endif

        Undock();
        return;
    }

    // Prevent rapid input during docking sequence
    if (bIsDocking)
    {
        #if DOCKING_DEBUG_ENABLED

        // Debug print
        if (GEngine)
        {
            AdastreaDockingDebug::Print(3.0f, FColor::Yellow, TEXT("[DOCKING] Already docking - ignoring input"));
        }

        #endif

        return;
    }

    // Cast to docking bay module to check availability
    ADockingBayModule* DockingBay = Cast<ADockingBayModule>(NearbyStation);
    if (!DockingBay)
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::RequestDocking - Station is not a docking module"));

        #if DOCKING_DEBUG_ENABLED


        // Debug print
        if (GEngine)
        {
            AdastreaDockingDebug::Print(5.0f, FColor::Red, TEXT("[DOCKING] ERROR: Station is not a docking module"));
        }


        #endif

        // Show user feedback via HUD message
        ShowHUDAlert(FText::FromString("Station is not a docking module"), 3.0f, true);
        return;
    }

    #if DOCKING_DEBUG_ENABLED


    // Debug print - docking module found
    if (GEngine)
    {
        AdastreaDockingDebug::Print(5.0f, FColor::Green, TEXT("[DOCKING] Station is a valid docking module"));
    }


    #endif

    // Check ship-size compatibility (X4-style dock sizing - a small dock like
    // DockingPortModule restricts AllowedShipSizeCategories to small ships;
    // DockingBayModule leaves it unrestricted). ShipDataAsset missing is treated
    // as compatible - fail-open, matching how the rest of this project's
    // validation degrades gracefully with no catalog/data asset assigned.
    if (ShipDataAsset && NearbyStation && !NearbyStation->IsShipSizeCompatible(ShipDataAsset->GetSizeCategory()))
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::RequestDocking - Ship size '%s' not compatible with this dock"),
            *ShipDataAsset->GetSizeCategory());

        #if DOCKING_DEBUG_ENABLED
        if (GEngine)
        {
            AdastreaDockingDebug::Print(5.0f, FColor::Red, FString::Printf(TEXT("[DOCKING] ERROR: %s is too large for this dock"), *ShipDataAsset->GetSizeCategory()));
        }
        #endif

        ShowHUDAlert(FText::FromString("Ship too large for this dock"), 3.0f, true);
        return;
    }

    // Check if docking is available
    if (!DockingBay->HasAvailableDocking())
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::RequestDocking - No docking slots available"));

        #if DOCKING_DEBUG_ENABLED


        // Debug print
        if (GEngine)
        {
            AdastreaDockingDebug::Print(5.0f, FColor::Red, FString::Printf(TEXT("[DOCKING] ERROR: No docking slots available (%d/%d occupied)"),
                    DockingBay->MaxDockedShips - DockingBay->GetAvailableDockingSpots(),
                    DockingBay->MaxDockedShips));
        }


        #endif

        // Show user feedback via HUD message
        ShowHUDAlert(FText::FromString("No docking slots available"), 3.0f, true);
        return;
    }

    #if DOCKING_DEBUG_ENABLED


    // Debug print - slots available
    if (GEngine)
    {
        AdastreaDockingDebug::Print(5.0f, FColor::Green, FString::Printf(TEXT("[DOCKING] Docking slots available: %d/%d free"),
                DockingBay->GetAvailableDockingSpots(),
                DockingBay->MaxDockedShips));
    }


    #endif

    // Get available docking point
    USceneComponent* DockingPoint = DockingBay->GetAvailableDockingPoint();
    if (!DockingPoint)
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::RequestDocking - Failed to get docking point"));

        #if DOCKING_DEBUG_ENABLED


        // Debug print
        if (GEngine)
        {
            AdastreaDockingDebug::Print(5.0f, FColor::Red, TEXT("[DOCKING] ERROR: Failed to get docking point (null pointer)"));
        }


        #endif

        // Show user feedback via HUD message
        ShowHUDAlert(FText::FromString("Failed to get docking point"), 3.0f, true);
        return;
    }

    #if DOCKING_DEBUG_ENABLED


    // Debug print - docking point found
    if (GEngine)
    {
        FVector PointLocation = DockingPoint->GetComponentLocation();
        AdastreaDockingDebug::Print(5.0f, FColor::Green, FString::Printf(TEXT("[DOCKING] Docking point found at location: X=%.0f Y=%.0f Z=%.0f"),
                PointLocation.X, PointLocation.Y, PointLocation.Z));
    }


    #endif

    // Check if ship is within docking range
    float DistanceToDockingPoint = FVector::Dist(GetActorLocation(), DockingPoint->GetComponentLocation());
    float EffectiveRange = GetEffectiveDockingRange();

    if (DistanceToDockingPoint > EffectiveRange)
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::RequestDocking - Too far from docking point (%.0f > %.0f)"), DistanceToDockingPoint, EffectiveRange);

        #if DOCKING_DEBUG_ENABLED


        // Debug print
        if (GEngine)
        {
            AdastreaDockingDebug::Print(5.0f, FColor::Red, FString::Printf(TEXT("[DOCKING] ERROR: Too far from docking point (%.0f units > %.0f max)"),
                    DistanceToDockingPoint, EffectiveRange));
        }


        #endif

        // Show user feedback via HUD message
        ShowHUDAlert(FText::FromString("Too far from docking point"), 3.0f, true);
        return;
    }

    #if DOCKING_DEBUG_ENABLED


    // Debug print - distance check passed
    if (GEngine)
    {
        AdastreaDockingDebug::Print(5.0f, FColor::Green, FString::Printf(TEXT("[DOCKING] Distance check passed: %.0f units (within %.0f max)"),
                DistanceToDockingPoint, EffectiveRange));
    }


    #endif

    // Store docking point and begin docking sequence
    CurrentDockingPoint = DockingPoint;
    bIsDocking = true;

    #if DOCKING_DEBUG_ENABLED


    // Debug print - starting docking
    if (GEngine)
    {
        AdastreaDockingDebug::Print(5.0f, FColor::Cyan, TEXT("[DOCKING] All checks passed - starting docking sequence"));
    }


    #endif

    // Navigate to docking point (instant in simplified version)
    NavigateToDockingPoint(CurrentDockingPoint);
}

void ASpaceship::NavigateToDockingPoint(USceneComponent* DockingPoint)
{
    #if DOCKING_DEBUG_ENABLED

    // Debug print - function entry
    if (GEngine)
    {
        AdastreaDockingDebug::Print(5.0f, FColor::Cyan, TEXT("[DOCKING] NavigateToDockingPoint() called - Moving ship to docking point"));
    }

    #endif

    // Validate docking point
    if (!DockingPoint)
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::NavigateToDockingPoint - Invalid docking point"));

        #if DOCKING_DEBUG_ENABLED


        // Debug print
        if (GEngine)
        {
            AdastreaDockingDebug::Print(5.0f, FColor::Red, TEXT("[DOCKING] ERROR: Invalid docking point (null pointer)"));
        }


        #endif

        bIsDocking = false;
        return;
    }

    // Simple MVP approach: instantly teleport to docking point
    // Get target transform from docking point
    FVector TargetLocation = DockingPoint->GetComponentLocation();
    FRotator TargetRotation = DockingPoint->GetComponentRotation();

    #if DOCKING_DEBUG_ENABLED


    // Debug print - target position
    if (GEngine)
    {
        AdastreaDockingDebug::Print(5.0f, FColor::Cyan, FString::Printf(TEXT("[DOCKING] Target docking position: X=%.0f Y=%.0f Z=%.0f"),
                TargetLocation.X, TargetLocation.Y, TargetLocation.Z));
    }


    #endif

    // Instantly move ship to docking point
    SetActorLocationAndRotation(TargetLocation, TargetRotation);

    #if DOCKING_DEBUG_ENABLED


    // Debug print - ship moved
    if (GEngine)
    {
        AdastreaDockingDebug::Print(5.0f, FColor::Green, TEXT("[DOCKING] Ship teleported to docking point successfully"));
    }


    #endif

    // Immediately complete docking
    CompleteDocking();

    UE_LOG(LogAdastreaShips, Log, TEXT("ASpaceship::NavigateToDockingPoint - Instantly docked at point"));
}

void ASpaceship::CompleteDocking()
{
    #if DOCKING_DEBUG_ENABLED

    // Debug print - function entry
    if (GEngine)
    {
        AdastreaDockingDebug::Print(5.0f, FColor::Cyan, TEXT("[DOCKING] CompleteDocking() called - Finalizing docking process"));
    }

    #endif

    // Update docking state
    bIsDocked = true;
    bIsDocking = false;

    // Clamps, then the airlock pressurising a moment later (player ship only).
    // (Not scheduled when the clamp was muted, e.g. while a save re-docks the ship.)
    if (UAudioEventLibrary::IsLocalPlayerActor(this)
        && UAudioEventLibrary::PlayEvent2D(this, TEXT("Dock.ClampEngage"), 1.0f))
    {
        GetWorldTimerManager().SetTimer(AirlockHissTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
        {
            if (bIsDocked)
            {
                UAudioEventLibrary::PlayEvent2D(this, TEXT("Dock.AirlockHiss"), 1.0f);
            }
        }), ShipEventAudio::AirlockHissDelay, false);
    }

    #if DOCKING_DEBUG_ENABLED


    // Debug print - state updated
    if (GEngine)
    {
        AdastreaDockingDebug::Print(5.0f, FColor::Green, TEXT("[DOCKING] Ship state updated: bIsDocked=true, bIsDocking=false"));
    }


    #endif

    // Notify station that ship has docked
    if (NearbyStation)
    {
        ADockingBayModule* DockingBay = Cast<ADockingBayModule>(NearbyStation);
        if (DockingBay)
        {
            DockingBay->DockShip();

            #if DOCKING_DEBUG_ENABLED


            // Debug print - station notified
            if (GEngine)
            {
                AdastreaDockingDebug::Print(5.0f, FColor::Green, TEXT("[DOCKING] Station notified of successful docking"));
            }


            #endif
        }
    }

    // Get player controller
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        #if DOCKING_DEBUG_ENABLED

        // Debug print - no player controller
        if (GEngine)
        {
            AdastreaDockingDebug::Print(5.0f, FColor::Red, TEXT("[DOCKING] ERROR: No player controller found - cannot disable input or create UI"));
        }

        #endif
        return;
    }

    #if DOCKING_DEBUG_ENABLED


    // Debug print - player controller found
    if (GEngine)
    {
        AdastreaDockingDebug::Print(5.0f, FColor::Green, TEXT("[DOCKING] Player controller found"));
    }


    #endif

    // Disable input
    DisableInput(PC);

    #if DOCKING_DEBUG_ENABLED


    // Debug print - input disabled
    if (GEngine)
    {
        AdastreaDockingDebug::Print(5.0f, FColor::Green, TEXT("[DOCKING] Ship input disabled"));
    }


    #endif

    // Hide ship
    SetActorHiddenInGame(true);

    #if DOCKING_DEBUG_ENABLED


    // Debug print - ship hidden
    if (GEngine)
    {
        AdastreaDockingDebug::Print(5.0f, FColor::Green, TEXT("[DOCKING] Ship mesh hidden (player is 'inside' station)"));
    }


    #endif

    // Get effective trading interface class (from settings or fallback)
        TSubclassOf<UUserWidget> EffectiveTradingClass = GetEffectiveTradingInterfaceClass();

        // NEW: Show the canvas-drawn trading screen (reliable in PIE) via the AHUD.
        if (PC)
        {
            if (AAdastreaHUD* GameHUD = Cast<AAdastreaHUD>(PC->GetHUD()))
            {
                GameHUD->ShowStationMenu();
                UE_LOG(LogAdastreaShips, Log, TEXT("ASpaceship::CompleteDocking - Opened station services menu"));
            }
        }

        // Create and show trading widget
    // The legacy UMG trading widget is superseded by the HUD canvas station menu/trade
    // screens; spawning it here would draw over the menu.
    constexpr bool bSpawnLegacyTradingWidget = false;
    if (bSpawnLegacyTradingWidget && EffectiveTradingClass)
    {
        #if DOCKING_DEBUG_ENABLED

        // Debug print - creating widget
        if (GEngine)
        {
            AdastreaDockingDebug::Print(5.0f, FColor::Cyan, TEXT("[DOCKING] Creating trading UI widget..."));
        }

        #endif

        TradingWidget = CreateWidget<UUserWidget>(PC, EffectiveTradingClass);
        if (TradingWidget)
        {
            TradingWidget->AddToViewport();

            #if DOCKING_DEBUG_ENABLED


            // Debug print - widget created successfully
            if (GEngine)
            {
                AdastreaDockingDebug::Print(5.0f, FColor::Green, TEXT("[DOCKING] Trading UI widget created and added to viewport"));
            }


            #endif
        }
        else
        {
            #if DOCKING_DEBUG_ENABLED

            // Debug print - widget creation failed
            if (GEngine)
            {
                AdastreaDockingDebug::Print(5.0f, FColor::Red, TEXT("[DOCKING] ERROR: Failed to create trading UI widget"));
            }

            #endif
        }
    }
    else
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::CompleteDocking - No TradingInterfaceClass set (neither in DockingSettings nor direct property) on '%s'. Trading UI will not be created."), *GetName());

        #if DOCKING_DEBUG_ENABLED


        // Debug print - widget class not set
        if (GEngine)
        {
            AdastreaDockingDebug::Print(5.0f, FColor::Red, TEXT("[DOCKING] ERROR: TradingInterfaceClass not set in Blueprint or Settings"));
        }


        #endif
    }

    // Set input mode to UI only
        PC->bShowMouseCursor = true;
        FInputModeGameAndUI InputMode;
        InputMode.SetHideCursorDuringCapture(false);
        if (TradingWidget)
        {
            InputMode.SetWidgetToFocus(TradingWidget->TakeWidget());
        }
        PC->SetInputMode(InputMode);

    #if DOCKING_DEBUG_ENABLED


    // Debug print - input mode changed
    if (GEngine)
    {
        AdastreaDockingDebug::Print(5.0f, FColor::Green, TEXT("[DOCKING] Input mode set to UI only, mouse cursor shown"));
    }


    #endif

    #if DOCKING_DEBUG_ENABLED


    // Debug print - complete success
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Green,
            TEXT("==================================================="));
        AdastreaDockingDebug::Print(8.0f, FColor::Green, TEXT("[DOCKING] DOCKING COMPLETE - Trading UI should be visible"));
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Green,
            TEXT("==================================================="));
    }


    #endif

    UE_LOG(LogAdastreaShips, Log, TEXT("ASpaceship::CompleteDocking - Docking complete for '%s'"), *GetName());
}

void ASpaceship::Undock()
{
    #if DOCKING_DEBUG_ENABLED

    // Debug print - function entry
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
            TEXT("[UNDOCKING] Undock() called - Beginning undock sequence"));
    }

    #endif

    // Check if actually docked
    if (!bIsDocked)
    {
        UE_LOG(LogAdastreaShips, Warning, TEXT("ASpaceship::Undock - Not currently docked"));

        #if DOCKING_DEBUG_ENABLED


        // Debug print
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
                TEXT("[UNDOCKING] WARNING: Not currently docked - aborting undock"));
        }


        #endif

        return;
    }

    // Notify station that ship is undocking
    if (NearbyStation)
    {
        ADockingBayModule* DockingBay = Cast<ADockingBayModule>(NearbyStation);
        if (DockingBay)
        {
            DockingBay->UndockShip();

            #if DOCKING_DEBUG_ENABLED


            // Debug print - station notified
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
                    FString::Printf(TEXT("[UNDOCKING] Station notified: %s"), *NearbyStation->GetName()));
            }


            #endif
        }
    }

    // Update state
    bIsDocked = false;

    GetWorldTimerManager().ClearTimer(AirlockHissTimerHandle);
    if (UAudioEventLibrary::IsLocalPlayerActor(this))
    {
        UAudioEventLibrary::PlayEvent2D(this, TEXT("Dock.Release"), 1.0f);
    }

    #if DOCKING_DEBUG_ENABLED


    // Debug print - state updated
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
            TEXT("[UNDOCKING] Ship state updated: bIsDocked=false"));
    }


    #endif

    // Remove trading widget
        if (TradingWidget)
        {
            TradingWidget->RemoveFromParent();
            TradingWidget = nullptr;

            #if DOCKING_DEBUG_ENABLED


            // Debug print - widget removed
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
                    TEXT("[UNDOCKING] Trading UI widget removed from viewport"));
            }


            #endif
        }

        // Hide the canvas trading screen (AHUD) on undock.
        if (APlayerController* UndockPC = Cast<APlayerController>(GetController()))
        {
            if (AAdastreaHUD* GameHUD = Cast<AAdastreaHUD>(UndockPC->GetHUD()))
            {
                GameHUD->HideTradeScreen();
            }
            UndockPC->SetInputMode(FInputModeGameOnly());
            UndockPC->bShowMouseCursor = false;
        }

        // Get player controller
        APlayerController* PC = Cast<APlayerController>(GetController());
        if (!PC)
        {
            #if DOCKING_DEBUG_ENABLED

            // Debug print - no player controller
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
                TEXT("[UNDOCKING] ERROR: No player controller found"));
        }

        #endif

        return;
    }

    // Enable input
    EnableInput(PC);

    #if DOCKING_DEBUG_ENABLED


    // Debug print - input enabled
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
            TEXT("[UNDOCKING] Ship input re-enabled"));
    }


    #endif

    // Show ship
    SetActorHiddenInGame(false);

    #if DOCKING_DEBUG_ENABLED


    // Debug print - ship visible
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
            TEXT("[UNDOCKING] Ship mesh shown"));
    }


    #endif

    // Set input mode to game only
    PC->bShowMouseCursor = false;
    FInputModeGameOnly InputMode;
    PC->SetInputMode(InputMode);

    #if DOCKING_DEBUG_ENABLED


    // Debug print - input mode changed
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
            TEXT("[UNDOCKING] Input mode set to game only, mouse cursor hidden"));
    }


    #endif

    // Apply forward impulse to move away from station
    FVector ForwardVector = GetActorForwardVector();
    if (MovementComponent)
    {
        // Add velocity in forward direction for smooth movement away
        MovementComponent->Velocity += ForwardVector * 500.0f;

        #if DOCKING_DEBUG_ENABLED


        // Debug print - impulse applied
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
                TEXT("[UNDOCKING] Forward impulse applied (500 units)"));
        }


        #endif
    }

    #if DOCKING_DEBUG_ENABLED


    // Debug print - complete success
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Green,
            TEXT("==================================================="));
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Green,
            TEXT("[UNDOCKING] UNDOCKING COMPLETE - Player has control"));
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Green,
            TEXT("==================================================="));
    }


    #endif

    UE_LOG(LogAdastreaShips, Log, TEXT("ASpaceship::Undock - Undocked successfully from '%s'"), NearbyStation ? *NearbyStation->GetName() : TEXT("Unknown Station"));
}

// ===== DOCKING CONFIGURATION HELPERS =====

float ASpaceship::GetEffectiveDockingRange() const
{
    // Use DockingSettings if set and valid, otherwise fall back to DockingRange property
    if (DockingSettings)
    {
        const float SettingsRange = DockingSettings->DockingRange;
        if (SettingsRange > 0.0f)
        {
            return SettingsRange;
        }
    }
    return DockingRange;
}

TSubclassOf<UUserWidget> ASpaceship::GetEffectiveDockingPromptWidgetClass() const
{
    // Use DockingSettings if set, otherwise fall back to DockingPromptWidgetClass property
    if (DockingSettings && DockingSettings->DockingPromptWidgetClass)
    {
        return DockingSettings->DockingPromptWidgetClass;
    }
    return DockingPromptWidgetClass;
}

TSubclassOf<UUserWidget> ASpaceship::GetEffectiveTradingInterfaceClass() const
{
    // Use DockingSettings if set, otherwise fall back to TradingInterfaceClass property
    if (DockingSettings && DockingSettings->TradingInterfaceClass)
    {
        return DockingSettings->TradingInterfaceClass;
    }
    return TradingInterfaceClass;
}

void ASpaceship::SetRuntimeInputEnabled(bool bEnabled)
{
	// Without this, RuntimeInputMappingContext (added once in
	// EnsureOwnInputActionsAndContext, priority 10) stays on the subsystem for the
	// rest of the game even after the player leaves the cockpit to walk the
	// interior, competing with the avatar's own mapping context for the same
	// WASD keys.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (RuntimeInputMappingContext)
			{
				if (bEnabled)
				{
					Subsystem->AddMappingContext(RuntimeInputMappingContext, 10);
				}
				else
				{
					Subsystem->RemoveMappingContext(RuntimeInputMappingContext);
				}
			}
		}
	}
	UE_LOG(LogAdastrea, Log, TEXT("ASpaceship::SetRuntimeInputEnabled(%d) on %s"), bEnabled, *GetName());
}
