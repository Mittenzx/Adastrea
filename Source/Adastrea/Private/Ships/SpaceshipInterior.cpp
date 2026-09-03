#include "Ships/SpaceshipInterior.h"
#include "Ships/Spaceship.h"
#include "Ships/SpaceshipAvatar.h"
#include "Player/AdastreaPlayerController.h"
#include "Player/PlayerInteractableComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "AdastreaLog.h"

ASpaceshipInterior::ASpaceshipInterior()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    // Visible interior geometry (shell/parts) the avatar walks inside.
    InteriorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InteriorMesh"));
    InteriorMesh->SetupAttachment(SceneRoot);
    InteriorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    InteriorMesh->SetCollisionObjectType(ECC_WorldStatic);
    InteriorMesh->SetCollisionResponseToAllChannels(ECR_Block);
    InteriorMesh->SetHiddenInGame(true); // hidden until the player enters

    // Box volume defines the walkable interior region (floor plane).
    InteriorVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteriorVolume"));
    InteriorVolume->SetupAttachment(SceneRoot);
    InteriorVolume->SetBoxExtent(FVector(500.0f, 300.0f, 175.0f)); // default 1000x600x350
    InteriorVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteriorVolume->SetCollisionObjectType(ECC_WorldStatic);
    InteriorVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteriorVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block); // keep the avatar on the floor region

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

void ASpaceshipInterior::ConfigureInterior(UStaticMesh* ShellMesh, bool bShowNow)
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
        // Fallback: try the fighter cockpit interior by path so the walk is never empty.
        static const TCHAR* Fallback = TEXT("/AdastreaShips/Meshes/Interiors/SM_Int_Fighter_Cockpit.SM_Int_Fighter_Cockpit");
        Mesh = LoadObject<UStaticMesh>(nullptr, Fallback);
    }

    InteriorMesh->SetStaticMesh(Mesh);

    // Fit the walkable volume + seat trigger to the mesh bounds so the avatar walks
    // inside the real interior footprint.
    FitVolumeToMesh();

    // Hidden until the player enters unless asked to show now.
    InteriorMesh->SetHiddenInGame(!bShowNow);
    UE_LOG(LogAdastrea, Log, TEXT("Interior %s configured with mesh %s (visible=%d)"),
        *GetName(), Mesh ? *Mesh->GetName() : TEXT("NULL"), bShowNow ? 1 : 0);
}

void ASpaceshipInterior::FitVolumeToMesh()
{
    if (!InteriorMesh || !InteriorMesh->GetStaticMesh())
    {
        return;
    }

    const FBoxSphereBounds Bounds = InteriorMesh->Bounds;
    const FVector Extent = Bounds.BoxExtent;
    const float HalfDepth = FMath::Max(Extent.X, 50.0f);
    const float HalfWidth = FMath::Max(Extent.Y, 50.0f);
    const float HalfHeight = FMath::Max(Extent.Z, 100.0f);

    // Walkable floor: cover the interior's footprint (X/Y), modest height, sits on the floor.
    if (InteriorVolume)
    {
        InteriorVolume->SetBoxExtent(FVector(HalfDepth, HalfWidth, 100.0f));
        // Rise so its bottom is at the interior floor (mesh bounds min Z, local).
        const float FloorZ = Bounds.Origin.Z - Extent.Z;
        InteriorVolume->SetRelativeLocation(FVector(0.0f, 0.0f, FloorZ + 100.0f));
    }

    // Seat/exit trigger near the front of the interior.
    if (ExitTrigger)
    {
        ExitTrigger->SetRelativeLocation(FVector(HalfDepth * 0.6f, 0.0f, HalfHeight * 0.6f));
    }

    // Default entry point: centre of the interior, standing on the floor.
    EntryLocation = FVector(0.0f, 0.0f, 200.0f);
    UE_LOG(LogAdastrea, Log, TEXT("Interior %s volume fitted to mesh bounds (d=%.0f w=%.0f h=%.0f)"),
        *GetName(), HalfDepth * 2, HalfWidth * 2, HalfHeight * 2);
}
