#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceshipInterior.generated.h"

class UBoxComponent;
class AAdastreaPlayerController;

/**
 * Which interior kit family a ship's interior belongs to: drives which companion
 * parts (console/deck/bunks/...) get mounted alongside the shell mesh and which
 * kit material set applies. Set explicitly on the ship (preferred) rather than
 * inferred by string-matching the shell mesh's asset name.
 */
UENUM(BlueprintType)
enum class EShipInteriorFamily : uint8
{
    None,
    Fighter,
    CommandBridge,
    CrewQuarters,
    GenerationHab,
    /** Bespoke corvette-hull bridge (SM_Int_Corvette_Bridge_*), replacing the
     * CommandBridge stopgap that Corvette/Cruiser/Destroyer used to share
     * identically. Cruiser/Destroyer stay on CommandBridge until they get
     * their own bespoke pass. */
    CorvetteBridge,
};

/**
 * Represents the walkable interior space of a spaceship
 *
 * This actor defines the physical interior that players can explore when boarding
 * a ship. It manages entry points and interior layout.
 *
 * Usage:
 * - Place this actor in your level or spawn it at runtime
 * - Configure entry location and rotation
 * - Reference from parent ASpaceship
 * - Use Blueprint to add interior geometry and functionality
 */
UCLASS()
class ADASTREA_API ASpaceshipInterior : public AActor
{
    GENERATED_BODY()

public:
    ASpaceshipInterior();

    /**
     * Get the entry point location for player spawn
     * @return World location where player should spawn when entering
     */
    UFUNCTION(BlueprintCallable, Category="Interior")
    FVector GetEntryLocation() const;

    /**
     * Get the entry point rotation for player spawn
     * @return Rotation the player should face when entering
     */
    UFUNCTION(BlueprintCallable, Category="Interior")
    FRotator GetEntryRotation() const;

    /**
     * Get the exit point location for player when leaving ship control
     * @return World location where player should be placed when exiting control
     */
    UFUNCTION(BlueprintCallable, Category="Interior")
    FVector GetExitLocation() const;

    /** Get the collision box that defines the walkable interior volume. */
    UFUNCTION(BlueprintPure, Category="Interior")
    UBoxComponent* GetInteriorVolume() const { return InteriorVolume; }

    /** Set the rect size of the walkable floor (X = forward depth, Y = width). */
        UFUNCTION(BlueprintCallable, Category="Interior")
        void SetFloorDimensions(float ForwardDepth, float Width);

        /**
         * Configure the interior to show (and walk within) a given static-mesh interior.
         * Assigns the mesh, hides it until entered, and sizes the walkable volume + exit
         * trigger to the mesh's real bounds so the avatar walks inside the correct footprint.
         * @param ShellMesh The interior static mesh (e.g. the ship's cockpit/hold shell).
         * @param Family Which companion-part kit + material set to mount alongside the
         *        shell. Pass None to fall back to inferring the family from the shell
         *        mesh's asset name (legacy behaviour, kept for content not yet migrated).
         * @param bShowNow If true, unhide immediately (else hidden until EnterInterior).
         */
        UFUNCTION(BlueprintCallable, Category="Interior")
        void ConfigureInterior(class UStaticMesh* ShellMesh, EShipInteriorFamily Family = EShipInteriorFamily::None, bool bShowNow = false);

        /** Resize the walk volume + seat trigger to the currently-assigned mesh bounds. */
                UFUNCTION(BlueprintCallable, Category="Interior")
                void FitVolumeToMesh();

                /** Un-hide the interior shell so the avatar can see the room around it.
                                         * SetActorHiddenInGame(false) on the actor doesn't always override a
                                         * component explicitly hidden by ConfigureInterior, so force it here. */
                                                void RevealInterior();

                                                /** Spawn (once) an interior ambient light so the room reads lit even at
                                                 * far world coordinates that have no scene lights. */
                                                void SetupInteriorLighting();

                protected:
                        /** Re-point the exported mesh's world-grid material slots to the authored
                         * M_Int_* kit materials so the interior reads as a designed room. */
                                void ApplyInteriorMaterials();

                public:
                        /** Return the interior's floor half-extents (local X/Y walk limits) and a
                         * standing altitude (local Z) the avatar should be held at, plus the
                         * room's true local centre (X/Y) — the mesh's bounding-box centre, which
                         * is very often NOT the actor's pivot. Returns false if no mesh is
                         * configured yet. */
                                bool GetLocalHalfExtents(const float InAltitude, FVector& OutHalfExtents, FVector2D& OutLocalCentreXY) const;

                protected:
                        /** Mount a companion part mesh (Console/Deck/Lights/...) co-located with the
                         * shell, scaled identically, hidden until reveal. Attaches to SceneRoot. */
                        void MountInteriorPart(const FString& PartPath, const FVector& Scale3D);

                        /** Mount all sibling part meshes for the given shell family + prefix. */
                        void MountInteriorParts(FString Prefix, FString Family, const FVector& Scale3D);

protected:
    virtual void OnConstruction(const FTransform& Transform) override;

    /** Called when the avatar overlaps the cockpit/seat trigger -> return to ship. */
        UFUNCTION()
        void OnExitTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                  bool bFromSweep, const FHitResult& SweepResult);

        /** Called when the avatar uses E on the cockpit seat -> return to ship. */
        UFUNCTION()
        void OnSeatInteract(AAdastreaPlayerController* PC);

    // World location where players spawn when entering the interior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
    FVector EntryLocation;

    // Rotation players face when entering the interior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
    FRotator EntryRotation;

    // World location where players spawn when exiting ship control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
    FVector ExitLocation;

    /** Root scene component so the volume can be positioned relative to the ship. */
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interior")
        TObjectPtr<USceneComponent> SceneRoot;

        /** Visible interior geometry (shell/parts) the avatar walks inside. */
                UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interior")
                TObjectPtr<class UStaticMeshComponent> InteriorMesh;

                /** Additional mounted companion part meshes (Console/Deck/Lights/etc.). */
                UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interior")
                TArray<TObjectPtr<class UStaticMeshComponent>> InteriorParts;

    /** Box volume the player can walk within (floor plane). Query-only (not solid) —
     * purely a footprint marker, see FloorCollision for the walkable surface itself. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interior")
    TObjectPtr<UBoxComponent> InteriorVolume;

    /** Thin, solid floor the avatar actually stands and walks on (blocks Pawn only).
     * The interior shell mesh ignores Pawn collision entirely (it's a reused/oversized
     * exterior hull, not clean walkable geometry), so without this the avatar has
     * nothing to stand on and CharacterMovement's normal Walking mode can't work. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interior")
    TObjectPtr<UBoxComponent> FloorCollision;

    /** Trigger volume at the cockpit/seat. Avatar walking into it returns to the ship. */
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interior")
        TObjectPtr<UBoxComponent> ExitTrigger;

        /** Worldwide interactable on the cockpit seat: E also returns to the ship. */
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interior")
        TObjectPtr<class UPlayerInteractableComponent> SeatInteractable;

public:
    // Walkable floor dimensions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
    float FloorForwardDepth = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
    float FloorWidth = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
        float CeilingHeight = 350.0f;

        /** Local offset of the cockpit/seat exit trigger from the interior origin. */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
        FVector ExitTriggerOffset = FVector(500.0f, 0.0f, 175.0f);

        /** Size of the cockpit/seat exit trigger box. */
                        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
                        FVector ExitTriggerSize = FVector(100.0f, 150.0f, 200.0f);

                        /** Grace period (s) after entry during which the exit trigger is
                         * ignored, so the avatar doesn't instantly re-trip the cockpit seat
                         * it just spawned on top of. */
                        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
                        float ExitTriggerGracePeriod = 2.0f;

                        /** World time of the most recent interior entry (used for the grace check). */
                                                float EntranceWorldTime = -1.0f;

                                                /** World location the avatar was placed at on the most recent entry. The
                                                 * exit trigger ignores overlaps until the avatar has actually walked away
                                                 * from this point — otherwise, on rooms where the seat trigger sits only
                                                 * a step or two from the spawn point, a couple of seconds of held
                                                 * movement walks straight into it and instantly boots the player back to
                                                 * the cockpit before they've gone anywhere. */
                                                UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
                                                FVector LastEntryWorldLocation = FVector::ZeroVector;

                                                /** Minimum distance (world units) the avatar must move away from
                                                 * LastEntryWorldLocation before the exit trigger will act on an overlap. */
                                                UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
                                                float MinDistanceFromEntryToExit = 200.0f;

                                                /** Interior light actor spawned on reveal so the room reads lit even at
                                                 * far world coords with no scene lights. Destroyed on hide. */
                                                TObjectPtr<class AActor> InteriorLight;

                /** Default interior shell mesh path when none is configured (fallback). */
                UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interior")
                TSoftObjectPtr<UStaticMesh> DefaultInteriorMesh;
    };