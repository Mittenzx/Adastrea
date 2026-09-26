// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/DockingBayModule.h"
#include "Stations/DockingDebug.h"
#include "Stations/SpaceStation.h"

// Debug flag for docking system - can be disabled for shipping builds
#ifndef DOCKING_DEBUG_ENABLED
    #define DOCKING_DEBUG_ENABLED 1
#endif

namespace DockingBayDefaults
{
    // Half-extents of SM_StationModule_DockingBay_01 are 600 x 400 x 200 cm; berths sit
    // this far from the module centre along each axis (hull face + clearance).
    constexpr float BerthOffsetX = 1600.0f;
    constexpr float BerthOffsetY = 1400.0f;
    // A default berth is unusable if another module of the same station comes within
    // this distance of it (covers a corvette's ~7.2 m half-length pointing back at the bay).
    constexpr float BerthClearance = 750.0f;
}

ADockingBayModule::ADockingBayModule()
{
    ModuleType = TEXT("Docking Bay");
    ModulePower = 50.0f;
    ModuleGroup = EStationModuleGroup::Docking;

    // Native default docking points. The Station Editor spawns this native class
    // directly (no hand-authored Blueprint points), so without these a player-built
    // docking bay reported "No docking points found" and could never be docked at.
    // Layout: SM_StationModule_DockingBay_01 is 1200 x 800 x 400 cm (3x2x1 grid cells),
    // centred on the actor origin. Ships teleport onto the point's world transform
    // (ASpaceship::NavigateToDockingPoint), so each point sits clear of the hull's
    // collision and faces away from the module (nose out, so a corvette's ~7.2 m
    // half-length still clears the hull). Points fill in order, so index 0 is the
    // mesh's berthing port, which sits on the -Y face (Blender +Y imports as UE -Y).
    struct FDefaultPoint { FVector Location; FRotator Rotation; };
    static const FDefaultPoint DefaultPoints[NumDefaultDockingPoints] = {
        { FVector(0.0f,    -DockingBayDefaults::BerthOffsetY,  0.0f), FRotator(0.0f,  -90.0f, 0.0f) },
        { FVector(0.0f,     DockingBayDefaults::BerthOffsetY,  0.0f), FRotator(0.0f,   90.0f, 0.0f) },
        { FVector( DockingBayDefaults::BerthOffsetX, 0.0f,     0.0f), FRotator(0.0f,    0.0f, 0.0f) },
        { FVector(-DockingBayDefaults::BerthOffsetX, 0.0f,     0.0f), FRotator(0.0f,  180.0f, 0.0f) },
    };

    for (int32 Index = 0; Index < NumDefaultDockingPoints; ++Index)
    {
        // Unique names so they can't clash with SCS components a Blueprint subclass
        // already calls "DockingPoint*".
        USceneComponent* Point = CreateDefaultSubobject<USceneComponent>(*FString::Printf(TEXT("NativeDockingPoint_%d"), Index));
        Point->SetupAttachment(RootComponent);
        Point->SetRelativeLocationAndRotation(DefaultPoints[Index].Location, DefaultPoints[Index].Rotation);
        Point->ComponentTags.Add(FName("DockingPoint"));
        DefaultDockingPoints.Add(Point);
    }
}

void ADockingBayModule::BeginPlay()
{
    Super::BeginPlay();

    // Automatically populate DockingPoints array from components tagged with "DockingPoint"
    // This solves the Unreal Engine limitation where TArray<USceneComponent*> with EditAnywhere
    // only allows creating new components, not selecting existing ones in Class Defaults
    PopulateDockingPointsFromTags();
}

void ADockingBayModule::PopulateDockingPointsFromTags()
{
    // Clear existing array (in case manually added in old workflow)
    DockingPoints.Empty();

    // Get all components with the "DockingPoint" tag
    TArray<UActorComponent*> TaggedComponents = GetComponentsByTag(USceneComponent::StaticClass(), FName("DockingPoint"));

    // Hand-authored points (Blueprint SCS or per-instance components, e.g.
    // BP_SpaceStationModule_DockingBay) take precedence over the native defaults, so a
    // Blueprint never ends up with its own points plus four extra native ones.
    const bool bHasAuthoredPoints = TaggedComponents.ContainsByPredicate([this](const UActorComponent* Component)
    {
        return !DefaultDockingPoints.Contains(Component);
    });
    if (bHasAuthoredPoints)
    {
        TaggedComponents.RemoveAll([this](const UActorComponent* Component)
        {
            return DefaultDockingPoints.Contains(Component);
        });
    }

#if DOCKING_DEBUG_ENABLED
    if (GEngine)
    {
        AdastreaDockingDebug::Print(5.0f, FColor::Cyan, FString::Printf(TEXT("[DOCKING] Found %d components tagged as 'DockingPoint'"), TaggedComponents.Num()));
    }
#endif

    // Cast and add to DockingPoints array
    for (UActorComponent* Component : TaggedComponents)
    {
        if (USceneComponent* SceneComp = Cast<USceneComponent>(Component))
        {
            DockingPoints.Add(SceneComp);

#if DOCKING_DEBUG_ENABLED
            if (GEngine)
            {
                AdastreaDockingDebug::Print(5.0f, FColor::Green, FString::Printf(TEXT("[DOCKING] Added docking point: %s at location %s"),
                        *SceneComp->GetName(),
                        *SceneComp->GetComponentLocation().ToString()));
            }
#endif
        }
    }

    // Warn if no docking points were found
    if (DockingPoints.Num() == 0)
    {
#if DOCKING_DEBUG_ENABLED
        if (GEngine)
        {
            AdastreaDockingDebug::Print(10.0f, FColor::Red, FString::Printf(TEXT("[DOCKING] WARNING: No docking points found! Tag Scene Components with 'DockingPoint'")));
        }
#endif

        UE_LOG(LogTemp, Warning, TEXT("DockingBayModule '%s': No docking points found. Tag Scene Components with 'DockingPoint' to enable docking."), *GetName());
    }

    // Warn if fewer docking points than capacity
    if (DockingPoints.Num() < MaxDockedShips)
    {
#if DOCKING_DEBUG_ENABLED
        if (GEngine)
        {
            AdastreaDockingDebug::Print(10.0f, FColor::Yellow, FString::Printf(TEXT("[DOCKING] WARNING: Only %d docking points for capacity of %d ships"),
                    DockingPoints.Num(), MaxDockedShips));
        }
#endif

        UE_LOG(LogTemp, Warning, TEXT("DockingBayModule '%s': Only %d docking points defined for MaxDockedShips=%d"),
            *GetName(), DockingPoints.Num(), MaxDockedShips);
    }
}

USceneComponent* ADockingBayModule::GetAvailableDockingPoint() const
{
#if DOCKING_DEBUG_ENABLED
    // Debug print - function entry
    if (GEngine)
    {
        AdastreaDockingDebug::Print(5.0f, FColor::Cyan, FString::Printf(TEXT("[DOCKING] GetAvailableDockingPoint() called on %s"), *GetName()));
    }
#endif

    // NOTE: Validation checks are split into separate conditions (rather than compound condition)
    // to provide more specific error messages for debugging. This makes it easier to identify
    // whether the issue is lack of capacity or missing docking point configuration.

    // Only provide a docking point if we have capacity and at least one point defined
    if (!HasAvailableDocking())
    {
#if DOCKING_DEBUG_ENABLED
        // Debug print - no capacity
        if (GEngine)
        {
            AdastreaDockingDebug::Print(5.0f, FColor::Red, FString::Printf(TEXT("[DOCKING] ERROR: No docking capacity (%d/%d occupied)"),
                    CurrentDockedShips, MaxDockedShips));
        }
#endif

        return nullptr;
    }

    if (DockingPoints.Num() <= 0)
    {
#if DOCKING_DEBUG_ENABLED
        // Debug print - no docking points
        if (GEngine)
        {
            AdastreaDockingDebug::Print(5.0f, FColor::Red, TEXT("[DOCKING] ERROR: No docking points defined in module"));
        }
#endif

        return nullptr;
    }

    // The native default berths sit on all four faces, but on a player-built station
    // some faces are attached to neighbouring modules; skip berths that would park a
    // ship inside one. Hand-authored (Blueprint) points are trusted as-is.
    TArray<USceneComponent*> Candidates;
    const bool bUsingDefaultPoints = DefaultDockingPoints.Num() > 0 && DockingPoints.Contains(DefaultDockingPoints[0]);
    for (USceneComponent* Point : DockingPoints)
    {
        if (Point && (!bUsingDefaultPoints || !IsDockingPointObstructed(Point)))
        {
            Candidates.Add(Point);
        }
    }
    if (Candidates.Num() == 0)
    {
        Candidates = DockingPoints; // every face is built over: better a cramped berth than none
    }

    // Select the next available docking point based on how many ships are currently docked.
    // This assumes docking points are filled in order and that HasAvailableDocking()
    // already enforces that CurrentDockedShips is within a valid range.
    const int32 NextDockingIndex = FMath::Clamp(CurrentDockedShips, 0, Candidates.Num() - 1);

#if DOCKING_DEBUG_ENABLED
    // Debug print - point found
    if (GEngine)
    {
        AdastreaDockingDebug::Print(5.0f, FColor::Green, FString::Printf(TEXT("[DOCKING] Docking point found: Index %d of %d usable points (%d total)"),
                NextDockingIndex, Candidates.Num(), DockingPoints.Num()));
    }
#endif

    return Candidates[NextDockingIndex];
}

bool ADockingBayModule::IsDockingPointObstructed(const USceneComponent* Point) const
{
    const ASpaceStation* Station = Cast<ASpaceStation>(GetAttachParentActor());
    if (!Point || !Station)
    {
        return false;
    }

    const FVector PointLocation = Point->GetComponentLocation();
    const float ClearanceSq = FMath::Square(DockingBayDefaults::BerthClearance);
    for (const ASpaceStationModule* Other : Station->GetModules())
    {
        if (!IsValid(Other) || Other == this)
        {
            continue;
        }
        const FBox Bounds = Other->GetComponentsBoundingBox(true);
        if (Bounds.IsValid && Bounds.ComputeSquaredDistanceToPoint(PointLocation) < ClearanceSq)
        {
            return true;
        }
    }
    return false;
}

bool ADockingBayModule::DockShip()
{
#if DOCKING_DEBUG_ENABLED
    // Debug print - function entry
    if (GEngine)
    {
        AdastreaDockingDebug::Print(5.0f, FColor::Cyan, TEXT("[DOCKING] DockShip() called on station module"));
    }
#endif

    if (!HasAvailableDocking())
    {
#if DOCKING_DEBUG_ENABLED
        // Debug print - no capacity
        if (GEngine)
        {
            AdastreaDockingDebug::Print(5.0f, FColor::Red, FString::Printf(TEXT("[DOCKING] ERROR: Cannot dock - all slots occupied (%d/%d)"),
                    CurrentDockedShips, MaxDockedShips));
        }
#endif

        return false;
    }

    CurrentDockedShips++;

#if DOCKING_DEBUG_ENABLED
    // Debug print - docked successfully
    if (GEngine)
    {
        AdastreaDockingDebug::Print(5.0f, FColor::Green, FString::Printf(TEXT("[DOCKING] Ship docked successfully - occupied slots: %d/%d"),
                CurrentDockedShips, MaxDockedShips));
    }
#endif

    return true;
}

bool ADockingBayModule::UndockShip()
{
#if DOCKING_DEBUG_ENABLED
    // Debug print - function entry
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
            TEXT("[UNDOCKING] UndockShip() called on station module"));
    }
#endif

    if (CurrentDockedShips <= 0)
    {
#if DOCKING_DEBUG_ENABLED
        // Debug print - no ships docked
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
                TEXT("[UNDOCKING] WARNING: No ships currently docked"));
        }
#endif

        return false;
    }

    CurrentDockedShips--;

#if DOCKING_DEBUG_ENABLED
    // Debug print - undocked successfully
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
            FString::Printf(TEXT("[UNDOCKING] Ship undocked successfully - occupied slots: %d/%d"),
                CurrentDockedShips, MaxDockedShips));
    }
#endif

    return true;
}
