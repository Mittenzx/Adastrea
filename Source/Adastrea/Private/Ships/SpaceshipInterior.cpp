#include "Ships/SpaceshipInterior.h"
#include "Ships/Spaceship.h"
#include "Ships/SpaceshipAvatar.h"
#include "Player/AdastreaPlayerController.h"
#include "Player/PlayerInteractableComponent.h"
#include "Components/BoxComponent.h"
#include "Components/LightComponent.h"
#include "Components/LocalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Scene.h" // ELightUnits (full enum def, LightComponent.h only forward-declares it)
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "Engine/CollisionProfile.h"
#include "Engine/PointLight.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Materials/Material.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Audio/AudioEventLibrary.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "AdastreaLog.h"

namespace
{
    // Defined next to ASpaceshipInterior::ApplyInteriorMaterials().
    void ApplyKitMaterialsBySlot(UStaticMeshComponent* Comp);

    /** Full-deck shells from Tools/build_ship_decks.py list their sibling parts as
     * P_<Part> sockets (Tools/import_ship_decks.py adds them), so any ship's deck
     * mounts without a per-ship family or a hard-coded part list. */
    bool HasDeckPartSockets(const UStaticMesh* Mesh)
    {
        if (!Mesh)
        {
            return false;
        }
        for (const UStaticMeshSocket* Socket : Mesh->Sockets)
        {
            if (Socket && Socket->SocketName.ToString().StartsWith(TEXT("P_")))
            {
                return true;
            }
        }
        return false;
    }
}

ASpaceshipInterior::ASpaceshipInterior()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    // Visible interior geometry (shell/parts) the avatar walks inside.
    InteriorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InteriorMesh"));
    InteriorMesh->SetupAttachment(SceneRoot);
        InteriorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // solid to world/objects
            InteriorMesh->SetCollisionObjectType(ECC_WorldStatic);
            InteriorMesh->SetCollisionResponseToAllChannels(ECR_Block);
            InteriorMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore); // avatar not blocked by shell; clamp confines it
        InteriorMesh->SetHiddenInGame(true); // hidden until the player enters

    // Box volume defines the walkable interior region (floor plane).
    InteriorVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteriorVolume"));
    InteriorVolume->SetupAttachment(SceneRoot);
    InteriorVolume->SetBoxExtent(FVector(500.0f, 300.0f, 175.0f)); // default 1000x600x350
    InteriorVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // overlay only, not solid
        InteriorVolume->SetCollisionObjectType(ECC_WorldStatic);
        InteriorVolume->SetCollisionResponseToAllChannels(ECR_Ignore);

    // Thin solid floor slab the avatar actually walks on (real ground for
    // CharacterMovement's normal Walking mode — see header comment).
    FloorCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("FloorCollision"));
    FloorCollision->SetupAttachment(SceneRoot);
    FloorCollision->SetBoxExtent(FVector(500.0f, 300.0f, 10.0f));
    FloorCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        FloorCollision->SetCollisionObjectType(ECC_WorldStatic);
        FloorCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
        FloorCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

    // Four thin walls enclosing the walkable footprint (real collision, same reasoning
    // as FloorCollision — see header comment).
    auto MakeWall = [this](const TCHAR* Name) -> UBoxComponent*
    {
        UBoxComponent* Wall = CreateDefaultSubobject<UBoxComponent>(Name);
        Wall->SetupAttachment(SceneRoot);
        Wall->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Wall->SetCollisionObjectType(ECC_WorldStatic);
        Wall->SetCollisionResponseToAllChannels(ECR_Ignore);
        Wall->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
        return Wall;
    };
    WallNorth = MakeWall(TEXT("WallNorth"));
    WallSouth = MakeWall(TEXT("WallSouth"));
    WallEast = MakeWall(TEXT("WallEast"));
    WallWest = MakeWall(TEXT("WallWest"));

    // Trigger volume at the cockpit/seat: walking the avatar into it re-possesses the ship.
    ExitTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("ExitTrigger"));
    ExitTrigger->SetupAttachment(SceneRoot);
    ExitTrigger->SetBoxExtent(FVector(100.0f, 150.0f, 200.0f));
    ExitTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ExitTrigger->SetCollisionObjectType(ECC_WorldDynamic);
    ExitTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    ExitTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    ExitTrigger->SetGenerateOverlapEvents(true);
        ExitTrigger->OnComponentBeginOverlap.AddDynamic(this, &ASpaceshipInterior::OnExitTriggerOverlap);

        // Worldwide E-interactable at the cockpit seat: pressing E also returns to the ship.
        SeatInteractable = CreateDefaultSubobject<UPlayerInteractableComponent>(TEXT("SeatInteractable"));
        SeatInteractable->InteractPrompt = NSLOCTEXT("Adastrea", "BoardCockpit", "Board Cockpit");
        SeatInteractable->InteractionRadius = 250.0f;
        SeatInteractable->OnInteracted.AddDynamic(this, &ASpaceshipInterior::OnSeatInteract);

        EntryLocation = FVector(0, 0, 200); // Example entry point
        EntryRotation = FRotator(0, 0, 0);
        ExitLocation = FVector(0, 0, 100); // Default exit point (same as entry)
    }

void ASpaceshipInterior::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // Apply the walkable floor dimensions.
    if (InteriorVolume)
    {
        InteriorVolume->SetBoxExtent(FVector(FloorForwardDepth * 0.5f, FloorWidth * 0.5f, CeilingHeight * 0.5f));
        // Lift the volume so its bottom is at the floor (avatar stands on it).
        InteriorVolume->SetRelativeLocation(FVector(0.0f, 0.0f, CeilingHeight * 0.5f));
    }

    // Match the solid floor slab to the same footprint, sitting right at floor level.
    if (FloorCollision)
    {
        FloorCollision->SetBoxExtent(FVector(FloorForwardDepth * 0.5f, FloorWidth * 0.5f, 10.0f));
        FloorCollision->SetRelativeLocation(FVector(0.0f, 0.0f, -10.0f));
    }

    // Place the cockpit/seat exit trigger at the configured local offset.
    if (ExitTrigger)
    {
        ExitTrigger->SetBoxExtent(ExitTriggerSize);
        ExitTrigger->SetRelativeLocation(ExitTriggerOffset);
    }
}

FVector ASpaceshipInterior::GetEntryLocation() const
{
    return EntryLocation;
}

FRotator ASpaceshipInterior::GetEntryRotation() const
{
    return EntryRotation;
}

FVector ASpaceshipInterior::GetExitLocation() const
{
    return ExitLocation;
}

void ASpaceshipInterior::SetFloorDimensions(float ForwardDepth, float Width)
{
    FloorForwardDepth = ForwardDepth;
    FloorWidth = Width;
    if (InteriorVolume)
    {
        InteriorVolume->SetBoxExtent(FVector(ForwardDepth * 0.5f, Width * 0.5f, CeilingHeight * 0.5f));
    }
}

void ASpaceshipInterior::OnExitTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                              bool bFromSweep, const FHitResult& SweepResult)
{
    // Only react to the walking avatar entering the cockpit/seat trigger.
        ASpaceshipAvatar* Avatar = Cast<ASpaceshipAvatar>(OtherActor);
        if (!Avatar)
        {
            return;
        }

        // Ignore the overlap that immediately follows entry — the avatar spawns on the
        // cockpit seat, so without this grace it bounces straight back to the ship.
        if (EntranceWorldTime > 0.0f && GetWorld()->GetTimeSeconds() - EntranceWorldTime < ExitTriggerGracePeriod)
        {
            const float Remaining = ExitTriggerGracePeriod - (GetWorld()->GetTimeSeconds() - EntranceWorldTime);
            UE_LOG(LogAdastrea, Log,
                TEXT("InteriorExitTrigger: ignoring spawn overlap (%.1fs < grace %.1fs), rechecking when it ends"),
                GetWorld()->GetTimeSeconds() - EntranceWorldTime, ExitTriggerGracePeriod);
            // An avatar that reaches the seat inside the grace window and stays there
            // gets no second BeginOverlap, so look again once the grace is over (the
            // walked-away-from-entry check below still applies).
            TWeakObjectPtr<ASpaceshipAvatar> WeakAvatar(Avatar);
            FTimerHandle Recheck;
            GetWorldTimerManager().SetTimer(Recheck, FTimerDelegate::CreateWeakLambda(this, [this, WeakAvatar]()
            {
                ASpaceshipAvatar* A = WeakAvatar.Get();
                if (A && ExitTrigger && ExitTrigger->IsOverlappingActor(A))
                {
                    OnExitTriggerOverlap(ExitTrigger, A, nullptr, 0, false, FHitResult());
                }
            }), Remaining + 0.05f, false);
            return;
        }

        // Also require the avatar to have actually walked away from the entry point at
        // least once. The grace period above only covers the first couple of seconds;
        // on smaller rooms the seat trigger can still be well within a few seconds' walk
        // of the spawn point, so a player who spawns, pauses, then holds a movement key
        // would otherwise get bounced straight back to the cockpit before going anywhere.
        if (FVector::DistSquared(Avatar->GetActorLocation(), LastEntryWorldLocation) <
            FMath::Square(MinDistanceFromEntryToExit))
        {
            UE_LOG(LogAdastrea, Verbose,
                TEXT("InteriorExitTrigger: ignoring overlap, avatar hasn't left the entry area yet (%.0f < %.0f)"),
                FVector::Dist(Avatar->GetActorLocation(), LastEntryWorldLocation), MinDistanceFromEntryToExit);
            // An avatar that stepped into the trigger this close to its spawn and then
            // walks on to the seat stays inside it and gets no second BeginOverlap, so
            // keep looking while it is still overlapping (each recheck re-runs this test).
            TWeakObjectPtr<ASpaceshipAvatar> WeakAvatar(Avatar);
            FTimerHandle Recheck;
            GetWorldTimerManager().SetTimer(Recheck, FTimerDelegate::CreateWeakLambda(this, [this, WeakAvatar]()
            {
                ASpaceshipAvatar* A = WeakAvatar.Get();
                if (A && ExitTrigger && ExitTrigger->IsOverlappingActor(A))
                {
                    OnExitTriggerOverlap(ExitTrigger, A, nullptr, 0, false, FHitResult());
                }
            }), 0.25f, false);
            return;
        }

        if (AAdastreaPlayerController* PC = Cast<AAdastreaPlayerController>(Avatar->GetController()))
    {
        if (ASpaceship* SourceShip = Avatar->SourceShip)
        {
            UE_LOG(LogAdastrea, Log, TEXT("InteriorExitTrigger: avatar reached the cockpit seat -> returning to ship."));
            PC->ExitShipInterior(SourceShip);
        }
    }
}
void ASpaceshipInterior::OnSeatInteract(AAdastreaPlayerController* PC)
{
    if (!PC)
    {
        return;
    }

    // The avatar interacting with the seat returns to the ship its SourceShip points at.
    // Guard: only act when the player is the on-foot avatar standing in this interior.
    ASpaceshipAvatar* Avatar = Cast<ASpaceshipAvatar>(PC->GetPawn());
    if (!Avatar || !Avatar->CurrentInterior || Avatar->CurrentInterior != this)
    {
        return;
    }

    if (ASpaceship* SourceShip = Avatar->SourceShip)
    {
        UE_LOG(LogAdastrea, Log, TEXT("InteriorSeat: E interact -> returning to %s cockpit."), *SourceShip->GetName());
        PC->ExitShipInterior(SourceShip);
    }
}

void ASpaceshipInterior::ConfigureInterior(UStaticMesh* ShellMesh, EShipInteriorFamily Family, bool bShowNow)
{
    if (!InteriorMesh)
    {
        return;
    }

    // Tear down companion parts from any previous configuration -- otherwise a
    // re-configure (e.g. Fighter -> CommandXLBridge) leaves the old family's
    // parts mounted alongside the new kit.
    for (TObjectPtr<UStaticMeshComponent> Part : InteriorParts)
    {
        if (Part)
        {
            Part->DestroyComponent();
        }
    }
    InteriorParts.Empty();

    // Resolve the mesh: explicit arg, else the configured default, else a fallback path.
    UStaticMesh* Mesh = ShellMesh;
    if (!Mesh && !DefaultInteriorMesh.IsNull())
    {
        Mesh = DefaultInteriorMesh.LoadSynchronous();
    }

    if (!Mesh)
    {
        // Fallback: try the fighter's cabin interior by path so the walk is
        // never empty.
        static const TCHAR* Fallback = TEXT("/AdastreaShips/Meshes/Interiors/SM_Int_Fighter_Cabin.SM_Int_Fighter_Cabin");
        Mesh = LoadObject<UStaticMesh>(nullptr, Fallback);
    }

    InteriorMesh->SetStaticMesh(Mesh);

    // Assign the authored M_Int_* kit materials over the world-grid defaults so the
    // interior reads as a designed room (the exported meshes ship with a placeholder
    // grid material on every slot). Map each material slot to its matching kit asset.
    ApplyInteriorMaterials();

    // Normalize the interior to a human-walkable size. The authored interior
    // shells are large (radius 8k-60k, ship-shell scale), but the avatar is
    // human-scale (~192 units tall). If left at raw scale, the walk volume
    // becomes a gigantic void. Scale the mesh so its shell radius maps to a
    // comfortable room (~650 units ~ a few metres across for a 1.9m avatar).
    // Full-deck kits are NOT normalized: they are authored at real size (100x in the
    // FBX like every SM_Int_* kit), so a fixed 0.01 gives 1 design cm = 1 uu.
    // Socket-driven decks (every ship but the Battleship) are recognised by their
    // P_ part sockets, whatever family the ship is set to.
    const bool bSocketDeck = HasDeckPartSockets(Mesh);
    const bool bFullDeck = (Family == EShipInteriorFamily::BattleshipDecks) || bSocketDeck;
    ConfiguredFamily = Family;
    WalkStepHeight = bFullDeck ? 45.0f : 0.0f;
    WalkCollision = nullptr;
    for (TObjectPtr<UPointLightComponent> L : SocketLights)
    {
        if (L)
        {
            L->DestroyComponent();
        }
    }
    SocketLights.Empty();
    SocketLightColours.Empty();
    SocketLightIntensities.Empty();
    for (TObjectPtr<AInteriorFixture> F : Fixtures)
    {
        if (F)
        {
            F->Destroy();
        }
    }
    Fixtures.Empty();

    const float TargetRadius = 650.0f;
        const FBoxSphereBounds RawBounds = Mesh->GetBounds();
        float Scale = 1.0f;
        if (bFullDeck)
        {
            Scale = 0.01f;
            InteriorMesh->SetRelativeScale3D(FVector(Scale));
            // The shell's auto-generated hull would be a ~120 m convex blob around
            // the ship while it flies; the deck walks on its own Collision part.
            InteriorMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
            InteriorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
        else if (RawBounds.SphereRadius > 1.0f)
        {
            Scale = TargetRadius / RawBounds.SphereRadius;
            InteriorMesh->SetRelativeScale3D(FVector(Scale, Scale, Scale));
        }
        else
        {
            InteriorMesh->SetRelativeScale3D(FVector::OneVector);
        }

    // Fit the walkable volume + seat trigger to the (now normalized) mesh bounds
        // so the avatar walks inside a correctly-sized interior footprint.
        FitVolumeToMesh();

        // Mount companion part meshes (Console/Deck/Lights/Stations/etc.) for the
        // interior family, at the same scale as the shell, so the room isn't an empty
        // shell.
        if (Scale > 0.0f)
        {
            const FVector Scale3D(Scale, Scale, Scale);
            FString Prefix;
            FString FamilyString;
            switch (Family)
            {
            case EShipInteriorFamily::CommandBridge:
                Prefix = TEXT("/AdastreaShips/Meshes/Interiors/SM_Int_CommandBridge");
                FamilyString = TEXT("CommandBridge");
                break;
            case EShipInteriorFamily::CorvetteBridge:
                Prefix = TEXT("/AdastreaShips/Meshes/Interiors/SM_Int_Corvette_Bridge");
                FamilyString = TEXT("CorvetteBridge");
                break;
            case EShipInteriorFamily::BattleshipBridge:
                Prefix = TEXT("/AdastreaShips/Meshes/Interiors/SM_Int_Battleship_Bridge");
                FamilyString = TEXT("BattleshipBridge");
                break;
            case EShipInteriorFamily::CommandXLBridge:
                Prefix = TEXT("/AdastreaShips/Meshes/Interiors/SM_Int_CommandXL_Bridge");
                FamilyString = TEXT("CommandXLBridge");
                break;
            case EShipInteriorFamily::BattleshipDecks:
                Prefix = TEXT("/AdastreaShips/Meshes/Interiors/SM_Int_Battleship_Decks");
                FamilyString = TEXT("BattleshipDecks");
                break;
            case EShipInteriorFamily::CrewQuarters:
                Prefix = TEXT("/AdastreaShips/Meshes/Interiors/SM_Int_Freighter_CrewQuarters");
                FamilyString = TEXT("CrewQuarters");
                break;
            case EShipInteriorFamily::GenerationHab:
                Prefix = TEXT("/AdastreaShips/Meshes/Interiors/SM_Int_Generationship_Hab");
                FamilyString = TEXT("Hab");
                break;
            case EShipInteriorFamily::Fighter:
                // v3 fighter cabin (2026-09-14) split into zones like the
                // bridge families -- v2 was a single joined mesh because this
                // case never routed through MountInteriorParts, so it had no
                // way to mount a separate sourced Console piece.
                Prefix = TEXT("/AdastreaShips/Meshes/Interiors/SM_Int_Fighter_Cabin");
                FamilyString = TEXT("Fighter");
                break;
            case EShipInteriorFamily::None:
            default:
                break;
            }

            if (bSocketDeck)
            {
                // Siblings share the shell's package path minus "_Shell".
                Prefix = Mesh->GetOutermost()->GetName();
                Prefix.RemoveFromEnd(TEXT("_Shell"));
                FamilyString = TEXT("SocketDeck");
            }

            // Legacy fallback: content that hasn't been migrated to an explicit
            // EShipInteriorFamily yet is inferred from the shell mesh's own name.
            if (Prefix.IsEmpty())
            {
                const FString ShellName = Mesh->GetName();
                if (ShellName.Contains(TEXT("CommandBridge")))
                {
                    Prefix = TEXT("/AdastreaShips/Meshes/Interiors/SM_Int_CommandBridge");
                    FamilyString = TEXT("CommandBridge");
                }
                else if (ShellName.Contains(TEXT("CrewQuarters")))
                {
                    Prefix = TEXT("/AdastreaShips/Meshes/Interiors/SM_Int_Freighter_CrewQuarters");
                    FamilyString = TEXT("CrewQuarters");
                }
                else if (ShellName.Contains(TEXT("Hab")))
                {
                    Prefix = TEXT("/AdastreaShips/Meshes/Interiors/SM_Int_Generationship_Hab");
                    FamilyString = TEXT("Hab");
                }
            }

            if (!Prefix.IsEmpty())
            {
                MountInteriorParts(Prefix, FamilyString, Scale3D);
            }
        }
        KitScale = Scale;
        if (bFullDeck)
        {
            ApplyDeckSockets(Scale);
            SpawnFixtures(Scale);
        }

        // Hidden until the player enters unless asked to show now.
        InteriorMesh->SetHiddenInGame(!bShowNow);
    UE_LOG(LogAdastrea, Log, TEXT("Interior %s configured with mesh %s (visible=%d)"),
        *GetName(), Mesh ? *Mesh->GetName() : TEXT("NULL"), bShowNow ? 1 : 0);
}

void ASpaceshipInterior::RevealInterior()
{
    if (InteriorMesh)
    {
        InteriorMesh->SetHiddenInGame(false);
    }
    for (TObjectPtr<UStaticMeshComponent> Part : InteriorParts)
    {
        if (Part)
        {
            Part->SetHiddenInGame(false);
        }
    }
    SetActorHiddenInGame(false);
    SetWalkCollisionEnabled(true);
    SetupInteriorLighting();
    SetFixturesActive(true);
}

void ASpaceshipInterior::SetWalkCollisionEnabled(bool bEnabled)
{
    if (WalkCollision)
    {
        WalkCollision->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
    }
}

void ASpaceshipInterior::ApplyDeckSockets(float Scale)
{
    UStaticMesh* Mesh = InteriorMesh ? InteriorMesh->GetStaticMesh() : nullptr;
    if (!Mesh)
    {
        return;
    }
    // Socket locations are in mesh space (100x); the component only carries scale.
    if (const UStaticMeshSocket* Entry = Mesh->FindSocket(TEXT("Entry")))
    {
        EntryLocation = Entry->RelativeLocation * Scale;
        EntryRotation = FRotator(0.0f, Entry->RelativeRotation.Yaw, 0.0f);
    }
    if (const UStaticMeshSocket* Seat = Mesh->FindSocket(TEXT("Seat")))
    {
        const FVector SeatLocal = Seat->RelativeLocation * Scale;
        if (ExitTrigger)
        {
            ExitTrigger->SetRelativeLocation(SeatLocal);
        }
        if (SeatInteractable)
        {
            SeatInteractable->InteractionPointOffset = SeatLocal;
        }
    }
    else
    {
        UE_LOG(LogAdastrea, Warning, TEXT("Interior %s: full-deck shell %s has no Seat socket (run Tools/import_battleship_decks.py)."),
            *GetName(), *Mesh->GetName());
    }
    UE_LOG(LogAdastrea, Log, TEXT("Interior %s deck sockets: entry=(%s) yaw=%.0f"),
        *GetName(), *EntryLocation.ToString(), EntryRotation.Yaw);
}

void ASpaceshipInterior::SpawnSocketLights()
{
    UStaticMesh* Mesh = InteriorMesh ? InteriorMesh->GetStaticMesh() : nullptr;
    if (!Mesh || SocketLights.Num() > 0)
    {
        return;
    }
    // L_<Colour>_<RadiusM>_<N>; LS_ casts shadows. Intensity grows with the
    // radius squared so a 16 m hangar flood reads as bright as a 7 m corridor tube.
    for (const UStaticMeshSocket* Socket : Mesh->Sockets)
    {
        if (!Socket)
        {
            continue;
        }
        TArray<FString> Bits;
        Socket->SocketName.ToString().ParseIntoArray(Bits, TEXT("_"));
        if (Bits.Num() < 3 || (Bits[0] != TEXT("L") && Bits[0] != TEXT("LS")))
        {
            continue;
        }
        FLinearColor Colour(0.82f, 0.9f, 1.0f);
        float Base = 5000.0f;
        switch (Bits[1].IsEmpty() ? TEXT('W') : Bits[1][0])
        {
        case TEXT('A'): Colour = FLinearColor(1.0f, 0.62f, 0.28f); Base = 5000.0f; break;
        case TEXT('R'): Colour = FLinearColor(1.0f, 0.1f, 0.05f);  Base = 1800.0f; break;
        case TEXT('G'): Colour = FLinearColor(0.35f, 1.0f, 0.45f); Base = 1500.0f; break;
        case TEXT('B'): Colour = FLinearColor(0.4f, 0.6f, 1.0f);   Base = 3000.0f; break;
        case TEXT('C'): Colour = FLinearColor(0.3f, 0.9f, 1.0f);   Base = 2500.0f; break;  // clinical cyan
        case TEXT('P'): Colour = FLinearColor(1.0f, 0.78f, 0.55f); Base = 4500.0f; break;  // warm (luxury)
        default: break;
        }
        const float RadiusM = FMath::Max(1.0f, FCString::Atof(*Bits[2]));
        UPointLightComponent* L = NewObject<UPointLightComponent>(this);
        L->SetupAttachment(SceneRoot);
        L->SetRelativeLocation(Socket->RelativeLocation * KitScale);
        L->SetIntensityUnits(ELightUnits::Candelas);
        L->SetIntensity(Base * FMath::Square(RadiusM / 8.0f));
        L->SetAttenuationRadius(RadiusM * 100.0f);
        L->SetLightColor(Colour);
        L->SetSourceRadius(15.0f);
        L->SetCastShadows(Bits[0] == TEXT("LS"));
        L->RegisterComponent();
        SocketLights.Add(L);
        SocketLightColours.Add(Colour);
        SocketLightIntensities.Add(L->Intensity);
    }
    UE_LOG(LogAdastrea, Log, TEXT("Interior %s: %d socket lights spawned."), *GetName(), SocketLights.Num());
}

void ASpaceshipInterior::SetupInteriorLighting()
{
    // The interior often sits at far world coordinates (away from the level's
    // scene lights), so as-authored it reads as a dark flat void. Interior
    // lighting plan (no-Lumen, iGPU):
    //   1. A low ambient Directional Light (Cast Shadows = OFF) so the room is
    //      "dim but readable" rather than pitch black (the world/ambient meta-light
    //      trick).
    //   2. A couple of Point/Rect fixture lights with tight attenuation where a
    //      real lamp/screen is, Cast Shadows = ON on these for depth.
    // All attach to the interior so they move/scale with the ship.
    if (ConfiguredFamily == EShipInteriorFamily::BattleshipDecks || WalkCollision)
    {
        // Full decks (the only kits with a walk-collision part) are lit from their
        // own fixtures; the room-sized
        // fill below would be one hot spot in the middle of a 20-120 m deck.
        SpawnSocketLights();
        return;
    }
    if (InteriorLight)
    {
        return; // already spawned
    }
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    const FVector Centre = GetActorLocation();
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // 1) Ambient base fill — a broad, low-intensity, NO-shadow point light centred in
        // the room. (Not a directional: a spawned directional competes with the level's
        // main DirectionalLight for forward-shading and logs a "competing directional
        // lights" warning.) A wide fill point keeps everything dim-but-readable, shadows off.
        InteriorLight = World->SpawnActor<APointLight>(
            APointLight::StaticClass(),
            Centre + FVector(0.0f, 0.0f, 180.0f), FRotator::ZeroRotator, Params);
        if (APointLight* Fill = Cast<APointLight>(InteriorLight))
        {
            if (UPointLightComponent* LC = Cast<UPointLightComponent>(Fill->GetLightComponent()))
            {
                LC->SetCastShadows(false);
                LC->SetAttenuationRadius(4000.0f); // wide room fill
                // Explicit units + a real value: UE5's default PointLightComponent
                // uses physically-based Candela, where 30 is close to imperceptible
                // over a 4000-unit falloff radius -- rooms were reading as pitch
                // black, not just dim as the comment intended. 5000 cd matches
                // roughly what a default UE5 Point Light actor ships with.
                LC->SetIntensityUnits(ELightUnits::Candelas);
                LC->SetIntensity(5000.0f);
                LC->SetLightColor(FLinearColor(0.75f, 0.82f, 0.9f)); // cool neutral
            }
            if (USceneComponent* LightRoot = Fill->GetRootComponent())
            {
                LightRoot->SetAbsolute(false, false, false);
            }
            Fill->AttachToActor(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
            UE_LOG(LogAdastrea, Log, TEXT("Interior %s ambient fill spawned."), *GetName());
        }

        // 2) Fixture points — warm pools at the room's near-floor, cast shadows for depth.
        const int32 FixtureCount = 2;
        for (int32 i = 0; i < FixtureCount; ++i)
        {
            const float Angle = 2.0f * PI * i / FixtureCount;
            const FVector Offset(300.0f * FMath::Cos(Angle), 300.0f * FMath::Sin(Angle), 80.0f);
            APointLight* Fixture = World->SpawnActor<APointLight>(
                APointLight::StaticClass(), Centre + Offset, FRotator::ZeroRotator, Params);
            if (Fixture)
            {
                if (UPointLightComponent* LC = Cast<UPointLightComponent>(Fixture->GetLightComponent()))
                {
                    // Same units fix as the ambient fill above -- 3000 candela
                    // over a 500-unit radius was still too dim to read as a
                    // "warm pool," never mind fill the whole room.
                    LC->SetIntensityUnits(ELightUnits::Candelas);
                    LC->SetIntensity(12000.0f);
                    LC->SetAttenuationRadius(500.0f); // tight pool, ~ room scale
                    LC->SetCastShadows(true);
                    LC->SetLightColor(FLinearColor(1.0f, 0.85f, 0.65f)); // warm pool
                }
                if (USceneComponent* LightRoot = Fixture->GetRootComponent())
                {
                    LightRoot->SetAbsolute(false, false, false);
                }
                Fixture->AttachToActor(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
                UE_LOG(LogAdastrea, Log, TEXT("Interior %s fixture light %d spawned."), *GetName(), i);
            }
        }
    }

UStaticMeshComponent* ASpaceshipInterior::MountInteriorPart(const FString& PartPath, const FVector& Scale3D)
{
    UStaticMesh* PartMesh = LoadObject<UStaticMesh>(nullptr, *PartPath);
    if (!PartMesh)
    {
        UE_LOG(LogAdastrea, Log, TEXT("Interior: companion part missing (%s)."), *PartPath);
        return nullptr;
    }
    UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(this);
    Comp->SetupAttachment(SceneRoot);
    Comp->SetStaticMesh(PartMesh);
    ApplyKitMaterialsBySlot(Comp); // parts ship with WorldGridMaterial slots too
    Comp->SetRelativeScale3D(Scale3D);
    Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Comp->SetHiddenInGame(true); // hidden until reveal
    Comp->RegisterComponent();
    InteriorParts.Add(Comp);
    UE_LOG(LogAdastrea, Log, TEXT("Interior: mounted companion part %s"), *PartMesh->GetName());
    return Comp;
}

void ASpaceshipInterior::MountInteriorParts(FString Prefix, FString Family, const FVector& Scale3D)
{
    auto TryPart = [&](const TCHAR* Suffix)
        {
            // Object path = /AdastreaShips/Meshes/Interiors/<prefix>_<suffix>.<basename>_<suffix>
            // The '.' suffix is the SHORT object name (last path segment), not the full path.
            const int32 LastSlash = Prefix.Find(TEXT("/"), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
            const FString BaseName = LastSlash >= 0 ? Prefix.Right(Prefix.Len() - LastSlash - 1) : Prefix;
            const FString ObjPath = Prefix + TEXT("_") + Suffix + TEXT(".") + BaseName + TEXT("_") + Suffix;
            return MountInteriorPart(ObjPath, Scale3D);
        };

    if (Family == TEXT("BattleshipDecks") || Family == TEXT("SocketDeck"))
    {
        if (Family == TEXT("BattleshipDecks"))
        {
            // Zone parts (Tools/build_battleship_decks.py PARTS, minus Shell).
            static const TCHAR* DeckParts[] = {
                TEXT("Spine"), TEXT("CIC"), TEXT("Quarters"), TEXT("Mess"), TEXT("Medbay"),
                TEXT("Briefing"), TEXT("Armory"), TEXT("Hangar"), TEXT("Dropship"),
                TEXT("Engineering"), TEXT("Lights"),
            };
            for (const TCHAR* Suffix : DeckParts)
            {
                TryPart(Suffix);
            }
        }
        else if (const UStaticMesh* Shell = InteriorMesh ? InteriorMesh->GetStaticMesh() : nullptr)
        {
            // P_<Part> sockets name every sibling (Tools/build_ship_decks.py).
            for (const UStaticMeshSocket* Socket : Shell->Sockets)
            {
                const FString Name = Socket ? Socket->SocketName.ToString() : FString();
                if (Name.StartsWith(TEXT("P_")) && Name != TEXT("P_Collision"))
                {
                    TryPart(*Name.Mid(2));
                }
            }
        }
        // Walk collision: never drawn, blocks only the avatar, and only while the
        // player is aboard (SetWalkCollisionEnabled). The mesh is complex-as-simple,
        // so the avatar sweeps against the authored floors, ramps and railings.
        if (UStaticMeshComponent* Col = TryPart(TEXT("Collision")))
        {
            Col->SetVisibility(false);
            Col->SetCollisionObjectType(ECC_WorldStatic);
            Col->SetCollisionResponseToAllChannels(ECR_Ignore);
            Col->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
            Col->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            WalkCollision = Col;
        }
        else
        {
            UE_LOG(LogAdastrea, Warning, TEXT("Interior %s: %s has no Collision part - the avatar will fall to the base slab."),
                *GetName(), *Prefix);
        }
        return;
    }

    // Mount every known kit part for this family, skipping any that don't exist.
    if (Family == TEXT("CommandBridge") || Family == TEXT("CorvetteBridge")
        || Family == TEXT("BattleshipBridge") || Family == TEXT("CommandXLBridge"))
    {
        TryPart(TEXT("Console"));
        TryPart(TEXT("Deck"));
        TryPart(TEXT("Lights"));
        TryPart(TEXT("Stations"));
        TryPart(TEXT("Viewport"));
        TryPart(TEXT("Hatch"));
    }
    else if (Family == TEXT("CrewQuarters"))
    {
        TryPart(TEXT("Bunks"));
        TryPart(TEXT("Desks"));
        TryPart(TEXT("Galley"));
        TryPart(TEXT("Lights"));
        TryPart(TEXT("Mess"));
        TryPart(TEXT("Vents"));
        TryPart(TEXT("Hatch"));
    }
    else if (Family == TEXT("Hab"))
    {
        TryPart(TEXT("Bunks"));
        TryPart(TEXT("Desks"));
        TryPart(TEXT("Galley"));
        TryPart(TEXT("Lights"));
        TryPart(TEXT("Mess"));
        TryPart(TEXT("Vents"));
        TryPart(TEXT("Hatch"));
    }
    else if (Family == TEXT("Fighter"))
    {
        // v3 cabin zones (see generate_adastrea_assets.py build_fighter_cabin_
        // interior) -- no Deck/Stations/Hatch/Bunks-style parts, a one-seat
        // cabin doesn't need them (hatch geometry is folded into Shell).
        TryPart(TEXT("Console"));
        TryPart(TEXT("Lights"));
        TryPart(TEXT("Viewport"));
    }
}

namespace
{
    // Slot name -> kit material path, applied to the interior shell AND to every
    // mounted companion part (Viewport/Console/Deck/...). Companion parts used to
    // skip this entirely and rendered on the exported WorldGridMaterial.
    // Optional second path = fallback when the first isn't authored/imported yet.
    struct FInteriorSlotMaterial
    {
        const TCHAR* Slot;
        const TCHAR* Material;
        const TCHAR* Fallback;
    };

    void ApplyKitMaterialsBySlot(UStaticMeshComponent* Comp)
    {
        if (!Comp || !Comp->GetStaticMesh())
        {
            return;
        }

        // First path: textured instance of M_IntSurface_Oriented built by
        // Tools/import_art_gap_assets.py --interiors per Assets/Textures/generated/
        // interiors/MATERIAL_MAPPING.md. Fallback: the legacy flat-colour kit material.
        static const FInteriorSlotMaterial SlotMaterials[] = {
            { TEXT("M_Int_Shell"),    TEXT("/AdastreaShips/Materials/Interiors/MI_Int_Shell"),    TEXT("/AdastreaShips/Materials/Interiors/M_Int_Shell") },
            { TEXT("M_Int_Deck"),     TEXT("/AdastreaShips/Materials/Interiors/MI_Int_Deck"),     TEXT("/AdastreaShips/Materials/Interiors/M_Int_Deck") },
            { TEXT("M_Int_Console"),  TEXT("/AdastreaShips/Materials/Interiors/MI_Int_Console"),  TEXT("/AdastreaShips/Materials/Interiors/M_Int_Console") },
            { TEXT("M_Int_Lights"),   TEXT("/AdastreaShips/Materials/Interiors/M_Int_Lights"),    nullptr },
            { TEXT("M_Int_Vents"),    TEXT("/AdastreaShips/Materials/Interiors/MI_Int_Vents"),    TEXT("/AdastreaShips/Materials/Interiors/M_Int_Vents") },
            { TEXT("M_Int_Stations"), TEXT("/AdastreaShips/Materials/Interiors/MI_Int_Stations"), TEXT("/AdastreaShips/Materials/Interiors/M_Int_Stations") },
            // Translucent glass (T_Int_Glass set, rebuilt in place).
            { TEXT("M_Int_Viewport"), TEXT("/AdastreaShips/Materials/Interiors/M_Int_Viewport"),  nullptr },
            { TEXT("M_Int_Bunks"),    TEXT("/AdastreaShips/Materials/Interiors/MI_Int_Bunks"),    TEXT("/AdastreaShips/Materials/Interiors/M_Int_Bunks") },
            { TEXT("M_Int_Desks"),    TEXT("/AdastreaShips/Materials/Interiors/MI_Int_Desks"),    TEXT("/AdastreaShips/Materials/Interiors/M_Int_Desks") },
            { TEXT("M_Int_Galley"),   TEXT("/AdastreaShips/Materials/Interiors/MI_Int_Galley"),   TEXT("/AdastreaShips/Materials/Interiors/M_Int_Galley") },
            { TEXT("M_Int_Mess"),     TEXT("/AdastreaShips/Materials/Interiors/MI_Int_Mess"),     TEXT("/AdastreaShips/Materials/Interiors/M_Int_Mess") },
            { TEXT("M_Int_Hatch"),    TEXT("/AdastreaShips/Materials/Interiors/MI_Int_Hatch"),    TEXT("/AdastreaShips/Materials/Interiors/M_Int_Hatch") },
            // Fighter cockpit uses an M_Interior_* slot name.
            { TEXT("M_Interior_Cockpit"), TEXT("/AdastreaShips/Materials/Interiors/MI_Interior_Cockpit"), TEXT("/AdastreaShips/Materials/Interiors/M_Int_Shell") },
            // SM_Int_Freighter_EngineRoom's only slot: textured engineering bay
            // (EngGrate floor / EngWall walls / ShipCeiling), was flat grey.
            { TEXT("M_Interior_Eng"), TEXT("/AdastreaShips/Materials/Interiors/MI_Interior_Eng"), TEXT("/AdastreaShips/Materials/Interiors/M_Int_Shell") },
            // Generationship Hab; Corridor/Airlock share this slot name and are
            // redirected to MI_Interior_Corridor below.
            { TEXT("M_Interior_Hab"), TEXT("/AdastreaShips/Materials/Interiors/MI_Interior_Hab"), TEXT("/AdastreaShips/Materials/Interiors/M_Int_Shell") },
        };

        for (const FInteriorSlotMaterial& Entry : SlotMaterials)
        {
            const FName SlotName(Entry.Slot);
            if (Comp->GetMaterialIndex(SlotName) == INDEX_NONE)
            {
                continue; // this slot doesn't exist on this mesh
            }
            const TCHAR* MaterialPath = Entry.Material;
            if (SlotName == FName(TEXT("M_Interior_Hab")))
            {
                const FString MeshName = Comp->GetStaticMesh()->GetName();
                if (MeshName.Contains(TEXT("Corridor")) || MeshName.Contains(TEXT("Airlock")))
                {
                    MaterialPath = TEXT("/AdastreaShips/Materials/Interiors/MI_Interior_Corridor");
                }
            }
            UMaterialInterface* Mat = LoadObject<UMaterialInterface>(nullptr, MaterialPath, nullptr, LOAD_NoWarn | LOAD_Quiet);
            if (!Mat && Entry.Fallback)
            {
                Mat = LoadObject<UMaterialInterface>(nullptr, Entry.Fallback, nullptr, LOAD_NoWarn | LOAD_Quiet);
            }
            if (Mat)
            {
                Comp->SetMaterialByName(SlotName, Mat);
            }
        }
    }
}

void ASpaceshipInterior::ApplyInteriorMaterials()
{
    if (!InteriorMesh || !InteriorMesh->GetStaticMesh())
    {
        return;
    }
    ApplyKitMaterialsBySlot(InteriorMesh);
    UE_LOG(LogAdastrea, Log, TEXT("Interior %s materials applied by slot name."), *GetName());
}


void ASpaceshipInterior::FitVolumeToMesh()
{
    if (!InteriorMesh || !InteriorMesh->GetStaticMesh())
    {
        return;
    }

    // Use the STATIC MESH ASSET's own bounds (local, unrotated frame) scaled by the
    // component's scale, not InteriorMesh->Bounds (world-space): the interior is rigidly
    // attached to the ship, so its world rotation tracks whatever heading the ship is
    // parked at, and a rotated room's world AABB is larger than its true local extents —
    // sizing/placing the floor/walls/trigger/spawn off that inflated world-space box
    // would put them somewhere other than where the visible mesh actually is.
    const FBoxSphereBounds RawBounds = InteriorMesh->GetStaticMesh()->GetBounds();
    const FVector Scale = InteriorMesh->GetComponentScale();
    const FVector Origin = RawBounds.Origin * Scale;
    const FVector Extent = RawBounds.BoxExtent * Scale;
    const float HalfDepth = FMath::Max(Extent.X, 50.0f);
    const float HalfWidth = FMath::Max(Extent.Y, 50.0f);
    const float HalfHeight = FMath::Max(Extent.Z, 100.0f);

    // Walkable floor: cover the interior's footprint (X/Y), modest height, sits on the floor.
    // Every placement below is centred on Origin.X/Y, NOT the interior actor's local (0,0) —
    // the mesh's own bounding-box centre is very often NOT at its pivot (an asymmetric
    // cockpit/cabin layout, an off-centre canopy, etc.), so treating (0,0) as "the middle
    // of the room" silently misplaces the floor/trigger/spawn relative to the actual
    // visible geometry. (Z already accounted for Origin.Z; X/Y did not, which is the
    // same class of bug.)
    const float FloorZ = Origin.Z - Extent.Z;
    if (InteriorVolume)
    {
        InteriorVolume->SetBoxExtent(FVector(HalfDepth, HalfWidth, 100.0f));
        // Rise so its bottom is at the interior floor (mesh bounds min Z, local).
        InteriorVolume->SetRelativeLocation(FVector(Origin.X, Origin.Y, FloorZ + 100.0f));
    }

    // Solid floor slab the avatar actually stands on: top surface flush with FloorZ.
    if (FloorCollision)
    {
        FloorCollision->SetBoxExtent(FVector(HalfDepth, HalfWidth, 10.0f));
        FloorCollision->SetRelativeLocation(FVector(Origin.X, Origin.Y, FloorZ - 10.0f));
    }

    // Four walls enclosing the footprint, centred mid-height, real Pawn-blocking collision
    // so CharacterMovement stops the avatar naturally instead of a manual position clamp.
    constexpr float WallHalfThickness = 10.0f;
    const float RoomMidZ = FloorZ + HalfHeight;
    if (WallNorth)
    {
        WallNorth->SetBoxExtent(FVector(WallHalfThickness, HalfWidth, HalfHeight));
        WallNorth->SetRelativeLocation(FVector(Origin.X + HalfDepth + WallHalfThickness, Origin.Y, RoomMidZ));
    }
    if (WallSouth)
    {
        WallSouth->SetBoxExtent(FVector(WallHalfThickness, HalfWidth, HalfHeight));
        WallSouth->SetRelativeLocation(FVector(Origin.X - HalfDepth - WallHalfThickness, Origin.Y, RoomMidZ));
    }
    if (WallEast)
    {
        // Extended past the depth by a wall-thickness on each end so it seals the corners
        // against WallNorth/WallSouth rather than leaving a gap an avatar could slip through.
        WallEast->SetBoxExtent(FVector(HalfDepth + WallHalfThickness, WallHalfThickness, HalfHeight));
        WallEast->SetRelativeLocation(FVector(Origin.X, Origin.Y + HalfWidth + WallHalfThickness, RoomMidZ));
    }
    if (WallWest)
    {
        WallWest->SetBoxExtent(FVector(HalfDepth + WallHalfThickness, WallHalfThickness, HalfHeight));
        WallWest->SetRelativeLocation(FVector(Origin.X, Origin.Y - HalfWidth - WallHalfThickness, RoomMidZ));
    }

    // Seat/exit trigger near the front of the interior.
    if (ExitTrigger)
    {
        ExitTrigger->SetRelativeLocation(FVector(Origin.X + HalfDepth * 0.6f, Origin.Y, HalfHeight * 0.6f));
    }

    // The "Board Cockpit" E-prompt belongs on the same seat. Left at its default zero
    // offset it sat at the interior actor's origin (the middle of the room, over the
    // capital bridges' holo-table/captain's chair), so it appeared ~3.5 m from the helm
    // and already within scan range of the entry spot.
    if (SeatInteractable)
    {
        SeatInteractable->InteractionPointOffset = FVector(Origin.X + HalfDepth * 0.6f, Origin.Y, FloorZ + 100.0f);
    }

    // Default entry point: the BACK of the interior (opposite the seat/exit trigger,
    // which sits at +0.6*HalfDepth), standing ON the floor (FloorZ + roughly a capsule
    // half-height) rather than at a fixed height that could float above or sink below
    // the real floor once the avatar uses real gravity/Walking mode.
    EntryLocation = FVector(Origin.X - HalfDepth * 0.6f, Origin.Y, FloorZ + 100.0f);
    UE_LOG(LogAdastrea, Log, TEXT("Interior %s volume fitted to mesh bounds (d=%.0f w=%.0f h=%.0f)"),
        *GetName(), HalfDepth * 2, HalfWidth * 2, HalfHeight * 2);
}

// ----------------------------------------------------------------------------
// Fixtures: exterior monitors, consoles, alert button, lights, intercom, ...
// ----------------------------------------------------------------------------

ASpaceship* ASpaceshipInterior::GetOwningShip() const
{
    return Cast<ASpaceship>(GetOwner());
}

void ASpaceshipInterior::SpawnFixtures(float Scale)
{
    UStaticMesh* Mesh = InteriorMesh ? InteriorMesh->GetStaticMesh() : nullptr;
    UWorld* World = GetWorld();
    if (!Mesh || !World)
    {
        return;
    }
    // X_<Mesh>_<Arg>_<N>: Mesh picks SM_Prop_Fx_<Mesh> and the fixture kind, Arg is the
    // feed (monitors) or readout (consoles), "-" when unused. Yaw faces the fixture.
    for (const UStaticMeshSocket* Socket : Mesh->Sockets)
    {
        if (!Socket)
        {
            continue;
        }
        TArray<FString> Bits;
        Socket->SocketName.ToString().ParseIntoArray(Bits, TEXT("_"));
        if (Bits.Num() < 3 || Bits[0] != TEXT("X"))
        {
            continue;
        }
        EInteriorFixtureKind Kind;
        if (!AInteriorFixture::ParseKind(Bits[1], Kind))
        {
            UE_LOG(LogAdastrea, Warning, TEXT("Interior %s: unknown fixture socket %s"), *GetName(), *Socket->SocketName.ToString());
            continue;
        }
        FActorSpawnParameters Params;
        Params.Owner = this;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AInteriorFixture* F = World->SpawnActor<AInteriorFixture>(AInteriorFixture::StaticClass(), GetActorTransform(), Params);
        if (!F)
        {
            continue;
        }
        F->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
        F->SetActorRelativeLocation(Socket->RelativeLocation * Scale);
        F->SetActorRelativeRotation(FRotator(0.0f, Socket->RelativeRotation.Yaw, 0.0f));
        F->Setup(this, Kind, FName(*Bits[2]), FName(*Bits[1]));
        Fixtures.Add(F);
    }
    if (Fixtures.Num() > 0)
    {
        UE_LOG(LogAdastrea, Log, TEXT("Interior %s: %d fixtures spawned."), *GetName(), Fixtures.Num());
    }
}

void ASpaceshipInterior::SetFixturesActive(bool bActive)
{
    bFixturesActive = bActive;
    for (TObjectPtr<AInteriorFixture> F : Fixtures)
    {
        if (F)
        {
            F->SetFixtureActive(bActive);
        }
    }
    FTimerManager& TM = GetWorldTimerManager();
    TM.ClearTimer(FeedTimer);
    if (bActive && FeedCaptures.Num() > 0)
    {
        TM.SetTimer(FeedTimer, this, &ASpaceshipInterior::CaptureFeeds, 1.0f / FMath::Max(1.0f, FeedCapturesPerSecond), true);
    }
    TM.ClearTimer(AlertTimer);
    if (bActive && bRedAlert)
    {
        TM.SetTimer(AlertTimer, this, &ASpaceshipInterior::TickAlert, 0.1f, true);
    }
    if (bActive)
    {
        ApplyLighting(0.0f);
    }
}

UTextureRenderTarget2D* ASpaceshipInterior::AcquireFeed(EExteriorFeed Feed)
{
    const int32 Index = static_cast<int32>(Feed);
    if (FeedTargets.IsValidIndex(Index) && FeedTargets[Index])
    {
        return FeedTargets[Index];
    }
    ASpaceship* Ship = GetOwningShip();
    if (!Ship || !Ship->GetRootComponent())
    {
        return nullptr;
    }
    FeedTargets.SetNum(FMath::Max(FeedTargets.Num(), Index + 1));
    FeedCaptures.SetNum(FMath::Max(FeedCaptures.Num(), Index + 1));

    UTextureRenderTarget2D* RT = NewObject<UTextureRenderTarget2D>(this);
    RT->RenderTargetFormat = RTF_RGBA8_SRGB;
    RT->ClearColor = FLinearColor::Black;
    RT->InitAutoFormat(FeedResolution.X, FeedResolution.Y);
    RT->UpdateResourceImmediate(true);

    // Camera placement around the hull: the visible hull meshes only, in the ship's
    // local space. (All components would include big triggers/effects and put the
    // cameras kilometres away.)
    FBox B(ForceInit);
    TArray<UStaticMeshComponent*> HullMeshes;
    Ship->GetComponents<UStaticMeshComponent>(HullMeshes);
    const FTransform ShipXf = Ship->GetActorTransform();
    for (const UStaticMeshComponent* SMC : HullMeshes)
    {
        if (!SMC || !SMC->GetStaticMesh() || !SMC->IsVisible() || SMC->GetOwner() != Ship)
        {
            continue;
        }
        const FBox Local = SMC->CalcBounds(SMC->GetComponentTransform().GetRelativeTransform(ShipXf)).GetBox();
        if (Local.GetExtent().GetMax() < 50000.0f)
        {
            B += Local;
        }
    }
    const FVector C = B.IsValid ? B.GetCenter() : FVector::ZeroVector;
    const FVector E = B.IsValid ? B.GetExtent() : FVector(1000.0f);
    const float Len = FMath::Max(E.X, 300.0f);
    FVector Loc = C;
    FRotator Rot = FRotator::ZeroRotator;
    float Fov = 80.0f;
    switch (Feed)
    {
    case EExteriorFeed::Bow:       Loc = FVector(C.X + E.X + 40.0f, C.Y, C.Z); break;
    case EExteriorFeed::Stern:     Loc = FVector(C.X - E.X - 40.0f, C.Y, C.Z); Rot = FRotator(0.0f, 180.0f, 0.0f); break;
    case EExteriorFeed::Chase:     Loc = FVector(C.X - Len * 3.2f, C.Y, C.Z + E.Z + Len * 0.9f); Rot = FRotator(-14.0f, 0.0f, 0.0f); Fov = 60.0f; break;
    case EExteriorFeed::Dorsal:    Loc = FVector(C.X + E.X * 0.4f, C.Y, C.Z + E.Z + Len * 0.25f); Rot = FRotator(-18.0f, 180.0f, 0.0f); Fov = 90.0f; break;
    case EExteriorFeed::Ventral:   Loc = FVector(C.X, C.Y, C.Z - E.Z - 40.0f); Rot = FRotator(-90.0f, 0.0f, 0.0f); Fov = 100.0f; break;
    case EExteriorFeed::Port:      Loc = FVector(C.X, C.Y - E.Y - 40.0f, C.Z); Rot = FRotator(0.0f, -90.0f, 0.0f); break;
    case EExteriorFeed::Starboard: Loc = FVector(C.X, C.Y + E.Y + 40.0f, C.Z); Rot = FRotator(0.0f, 90.0f, 0.0f); break;
    }

    USceneCaptureComponent2D* Cap = NewObject<USceneCaptureComponent2D>(Ship);
    Cap->SetupAttachment(Ship->GetRootComponent());
    Cap->SetAbsolute(false, false, true);
    Cap->SetRelativeLocationAndRotation(Loc, Rot);
    Cap->FOVAngle = Fov;
    Cap->TextureTarget = RT;
    Cap->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
    Cap->bCaptureEveryFrame = false;
    Cap->bCaptureOnMovement = false;
    Cap->bAlwaysPersistRenderingState = false;
    Cap->LODDistanceFactor = 2.0f;
    // A monitor feed needs no GI or reflections; keep captures cheap.
    Cap->PostProcessSettings.bOverride_DynamicGlobalIlluminationMethod = true;
    Cap->PostProcessSettings.DynamicGlobalIlluminationMethod = EDynamicGlobalIlluminationMethod::None;
    Cap->PostProcessSettings.bOverride_ReflectionMethod = true;
    Cap->PostProcessSettings.ReflectionMethod = EReflectionMethod::None;
    Cap->ShowFlags.SetMotionBlur(false);
    Cap->HiddenActors.Add(this);            // the interior pocket sits under the ship
    Cap->RegisterComponent();

    FeedTargets[Index] = RT;
    FeedCaptures[Index] = Cap;
    Cap->CaptureScene();
    if (bFixturesActive && !GetWorldTimerManager().IsTimerActive(FeedTimer))
    {
        GetWorldTimerManager().SetTimer(FeedTimer, this, &ASpaceshipInterior::CaptureFeeds, 1.0f / FMath::Max(1.0f, FeedCapturesPerSecond), true);
    }
    UE_LOG(LogAdastrea, Log, TEXT("Interior %s: exterior feed %s created on %s."), *GetName(),
        *AInteriorFixture::FeedName(Feed), *Ship->GetName());
    return RT;
}

void ASpaceshipInterior::CaptureFeeds()
{
    // Only feeds shown on a monitor near the avatar; up to two captures per tick,
    // round-robin, so several feeds share the budget.
    const APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    const APawn* Pawn = PC ? PC->GetPawn() : nullptr;
    if (!Pawn || !FeedCaptures.Num())
    {
        return;
    }
    TArray<bool> Wanted;
    Wanted.Init(false, FeedCaptures.Num());
    for (TObjectPtr<AInteriorFixture> F : Fixtures)
    {
        if (F && F->GetKind() == EInteriorFixtureKind::Monitor &&
            FVector::DistSquared(F->GetActorLocation(), Pawn->GetActorLocation()) < FMath::Square(2500.0f))
        {
            const int32 I = static_cast<int32>(F->GetFeed());
            if (Wanted.IsValidIndex(I))
            {
                Wanted[I] = true;
            }
        }
    }
    int32 Budget = 2;
    for (int32 k = 0; k < FeedCaptures.Num() && Budget > 0; ++k)
    {
        const int32 I = (NextFeedToCapture + k) % FeedCaptures.Num();
        if (Wanted[I] && FeedCaptures[I])
        {
            FeedCaptures[I]->CaptureScene();
            --Budget;
            NextFeedToCapture = I + 1;
        }
    }
}

void ASpaceshipInterior::SetRedAlert(bool bOn)
{
    bRedAlert = bOn;
    AlertClock = 0.0f;
    KlaxonClock = 0.0f;
    GetWorldTimerManager().ClearTimer(AlertTimer);
    if (bOn && bFixturesActive)
    {
        GetWorldTimerManager().SetTimer(AlertTimer, this, &ASpaceshipInterior::TickAlert, 0.1f, true);
    }
    ApplyLighting(0.0f);
}

void ASpaceshipInterior::TickAlert()
{
    AlertClock += 0.1f;
    KlaxonClock -= 0.1f;
    if (KlaxonClock <= 0.0f)
    {
        UAudioEventLibrary::PlayEvent2D(this, TEXT("Flight.SpeedWarning"));
        KlaxonClock = 1.6f;
    }
    ApplyLighting(0.5f + 0.5f * FMath::Sin(AlertClock * 4.0f));
}

int32 ASpaceshipInterior::CycleLighting()
{
    LightingMode = (LightingMode + 1) % 3;
    ApplyLighting(0.0f);
    return LightingMode;
}

void ASpaceshipInterior::ApplyLighting(float AlertPulse)
{
    static const float ModeScale[] = { 1.0f, 0.35f, 0.12f };
    const FLinearColor AlertRed(1.0f, 0.08f, 0.04f);
    const FLinearColor Emergency(1.0f, 0.55f, 0.3f);
    for (int32 i = 0; i < SocketLights.Num(); ++i)
    {
        UPointLightComponent* L = SocketLights[i];
        if (!L || !SocketLightColours.IsValidIndex(i))
        {
            continue;
        }
        FLinearColor Col = SocketLightColours[i];
        float Scale = ModeScale[FMath::Clamp(LightingMode, 0, 2)];
        if (LightingMode == 2)
        {
            Col = FLinearColor::LerpUsingHSV(Col, Emergency, 0.6f);
        }
        if (bRedAlert)
        {
            Col = AlertRed;
            Scale = FMath::Max(Scale, 0.2f) * (0.25f + 0.75f * AlertPulse);
        }
        L->SetLightColor(Col);
        L->SetIntensity(SocketLightIntensities[i] * Scale);
    }
}

void ASpaceshipInterior::Destroyed()
{
    for (TObjectPtr<AInteriorFixture> F : Fixtures)
    {
        if (F)
        {
            F->Destroy();
        }
    }
    Fixtures.Empty();
    for (TObjectPtr<USceneCaptureComponent2D> Cap : FeedCaptures)
    {
        if (Cap)
        {
            Cap->DestroyComponent();
        }
    }
    FeedCaptures.Empty();
    Super::Destroyed();
}
