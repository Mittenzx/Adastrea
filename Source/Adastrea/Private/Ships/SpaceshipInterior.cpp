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
#include "Engine/PointLight.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Materials/Material.h"
#include "AdastreaLog.h"

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
            UE_LOG(LogAdastrea, Log,
                TEXT("InteriorExitTrigger: ignoring spawn overlap (%.1fs < grace %.1fs)"),
                GetWorld()->GetTimeSeconds() - EntranceWorldTime, ExitTriggerGracePeriod);
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
            UE_LOG(LogAdastrea, Log,
                TEXT("InteriorExitTrigger: ignoring overlap, avatar hasn't left the entry area yet (%.0f < %.0f)"),
                FVector::Dist(Avatar->GetActorLocation(), LastEntryWorldLocation), MinDistanceFromEntryToExit);
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
    const float TargetRadius = 650.0f;
        const FBoxSphereBounds RawBounds = Mesh->GetBounds();
        float Scale = 1.0f;
        if (RawBounds.SphereRadius > 1.0f)
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
    SetupInteriorLighting();
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

void ASpaceshipInterior::MountInteriorPart(const FString& PartPath, const FVector& Scale3D)
{
    UStaticMesh* PartMesh = LoadObject<UStaticMesh>(nullptr, *PartPath);
    if (!PartMesh)
    {
        UE_LOG(LogAdastrea, Log, TEXT("Interior: companion part missing (%s)."), *PartPath);
        return;
    }
    UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(this);
    Comp->SetupAttachment(SceneRoot);
    Comp->SetStaticMesh(PartMesh);
    Comp->SetRelativeScale3D(Scale3D);
    Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Comp->SetHiddenInGame(true); // hidden until reveal
    Comp->RegisterComponent();
    InteriorParts.Add(Comp);
    UE_LOG(LogAdastrea, Log, TEXT("Interior: mounted companion part %s"), *PartMesh->GetName());
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
            MountInteriorPart(ObjPath, Scale3D);
        };

    // Mount every known kit part for this family, skipping any that don't exist.
    if (Family == TEXT("CommandBridge") || Family == TEXT("CorvetteBridge"))
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

void ASpaceshipInterior::ApplyInteriorMaterials()
{
    if (!InteriorMesh)
    {
        return;
    }
    const UStaticMesh* Mesh = InteriorMesh->GetStaticMesh();
    if (!Mesh)
    {
        return;
    }

    // Slot name -> kit material path. The exported meshes ship with WorldGridMaterial
    // on every slot, so the slot NAME (M_Int_Shell, M_Int_Deck, ...) is the reliable
    // key, not the current material's type. Only slots present on THIS mesh get
    // loaded, so a simple shell (e.g. the fighter empty room, 1-2 slots) doesn't pay
    // for loading all twelve kit materials.
    static const TPair<FName, const TCHAR*> SlotMaterials[] = {
        { TEXT("M_Int_Shell"),    TEXT("/AdastreaShips/Materials/Interiors/M_Int_Shell") },
        { TEXT("M_Int_Deck"),     TEXT("/AdastreaShips/Materials/Interiors/M_Int_Deck") },
        { TEXT("M_Int_Console"),  TEXT("/AdastreaShips/Materials/Interiors/M_Int_Console") },
        { TEXT("M_Int_Lights"),   TEXT("/AdastreaShips/Materials/Interiors/M_Int_Lights") },
        { TEXT("M_Int_Vents"),    TEXT("/AdastreaShips/Materials/Interiors/M_Int_Vents") },
        { TEXT("M_Int_Stations"), TEXT("/AdastreaShips/Materials/Interiors/M_Int_Stations") },
        { TEXT("M_Int_Viewport"), TEXT("/AdastreaShips/Materials/Interiors/M_Int_Viewport") },
        { TEXT("M_Int_Bunks"),    TEXT("/AdastreaShips/Materials/Interiors/M_Int_Bunks") },
        { TEXT("M_Int_Desks"),    TEXT("/AdastreaShips/Materials/Interiors/M_Int_Desks") },
        { TEXT("M_Int_Galley"),   TEXT("/AdastreaShips/Materials/Interiors/M_Int_Galley") },
        { TEXT("M_Int_Mess"),     TEXT("/AdastreaShips/Materials/Interiors/M_Int_Mess") },
        { TEXT("M_Int_Hatch"),    TEXT("/AdastreaShips/Materials/Interiors/M_Int_Hatch") },
        // Fighter cockpit uses an M_Interior_* slot name; map its shell slot too.
        { TEXT("M_Interior_Cockpit"), TEXT("/AdastreaShips/Materials/Interiors/M_Int_Shell") },
        // SM_Int_Freighter_EngineRoom's only slot is M_Interior_Eng, which has no
        // bespoke material yet (known issue: reads flat grey without this). Reuse
        // the shell material as a stand-in until a real engineering-bay material
        // is authored, rather than leave it on the exported placeholder grid material.
        { TEXT("M_Interior_Eng"), TEXT("/AdastreaShips/Materials/Interiors/M_Int_Shell") },
        // SM_Int_Generationship_Hab's slot is M_Interior_Hab; same gap, same stand-in.
        { TEXT("M_Interior_Hab"), TEXT("/AdastreaShips/Materials/Interiors/M_Int_Shell") },
    };

    for (const TPair<FName, const TCHAR*>& Entry : SlotMaterials)
    {
        if (InteriorMesh->GetMaterialByName(Entry.Key) == nullptr)
        {
            continue; // this slot doesn't exist on this mesh
        }
        if (UMaterialInterface* Mat = LoadObject<UMaterialInterface>(nullptr, Entry.Value))
        {
            InteriorMesh->SetMaterialByName(Entry.Key, Mat);
        }
    }

    UE_LOG(LogAdastrea, Log, TEXT("Interior %s materials applied by slot name."), *GetName());
}

bool ASpaceshipInterior::GetLocalHalfExtents(const float InAltitude, FVector& OutHalfExtents, FVector2D& OutLocalCentreXY) const
{
    if (!InteriorMesh || !InteriorMesh->GetStaticMesh())
    {
        return false;
    }
    // Use the STATIC MESH ASSET's own bounds (its local, unrotated frame) scaled by the
    // component's scale — NOT InteriorMesh->Bounds, which is a WORLD-space axis-aligned
    // box. The interior is rigidly attached to the ship, so its world rotation is
    // whatever heading the ship is parked at; a rotated room's world AABB is larger than
    // its true local extents, so clamping the avatar's LOCAL position (see caller) against
    // that inflated world-space size let it walk straight through the visible walls
    // (hull collision is disabled while inside) before this clamp ever engaged.
    const FBoxSphereBounds LocalBounds = InteriorMesh->GetStaticMesh()->GetBounds();
    const FVector Scale = InteriorMesh->GetComponentScale();
    const FVector Extent = LocalBounds.BoxExtent * Scale;
    const FVector Origin = LocalBounds.Origin * Scale;
    OutHalfExtents = FVector(FMath::Max(Extent.X, 100.0f),
                             FMath::Max(Extent.Y, 100.0f),
                             InAltitude);
    // The mesh's bounding-box centre, NOT the actor's local (0,0) — see FitVolumeToMesh,
    // which places the floor/trigger/spawn around this same centre.
    OutLocalCentreXY = FVector2D(Origin.X, Origin.Y);
    return true;
}

void ASpaceshipInterior::FitVolumeToMesh()
{
    if (!InteriorMesh || !InteriorMesh->GetStaticMesh())
    {
        return;
    }

    // Use the STATIC MESH ASSET's own bounds (local, unrotated frame) scaled by the
    // component's scale, not InteriorMesh->Bounds (world-space) — see GetLocalHalfExtents
    // for why: the interior's world rotation tracks the ship's current heading, so the
    // world-space AABB is inflated whenever the ship isn't level, oversizing the walkable
    // volume and mis-placing the exit trigger relative to the visible mesh.
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

    // Seat/exit trigger near the front of the interior.
    if (ExitTrigger)
    {
        ExitTrigger->SetRelativeLocation(FVector(Origin.X + HalfDepth * 0.6f, Origin.Y, HalfHeight * 0.6f));
    }

    // Default entry point: the BACK of the interior (opposite the seat/exit trigger,
    // which sits at +0.6*HalfDepth), standing ON the floor (FloorZ + roughly a capsule
    // half-height) rather than at a fixed height that could float above or sink below
    // the real floor once the avatar uses real gravity/Walking mode.
    EntryLocation = FVector(Origin.X - HalfDepth * 0.6f, Origin.Y, FloorZ + 100.0f);
    UE_LOG(LogAdastrea, Log, TEXT("Interior %s volume fitted to mesh bounds (d=%.0f w=%.0f h=%.0f)"),
        *GetName(), HalfDepth * 2, HalfWidth * 2, HalfHeight * 2);
}
