// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/DockingPortModule.h"

// Debug flag for docking system - can be disabled for shipping builds
#ifndef DOCKING_DEBUG_ENABLED
    #define DOCKING_DEBUG_ENABLED 1
#endif

ADockingPortModule::ADockingPortModule()
{
    ModuleType = TEXT("Docking Port");
    ModulePower = 10.0f;
    ModuleGroup = EStationModuleGroup::Docking;
}

void ADockingPortModule::BeginPlay()
{
    Super::BeginPlay();
    
    // Automatically populate DockingPoints array from components tagged with "DockingPoint"
    // This solves the Unreal Engine limitation where TArray<USceneComponent*> with EditAnywhere
    // only allows creating new components, not selecting existing ones in Class Defaults
    PopulateDockingPointsFromTags();
}

void ADockingPortModule::PopulateDockingPointsFromTags()
{
    // Clear existing array (in case manually added in old workflow)
    DockingPoints.Empty();
    
    // Get all components with the "DockingPoint" tag
    TArray<UActorComponent*> TaggedComponents;
    GetComponentsByTag(USceneComponent::StaticClass(), FName("DockingPoint"), TaggedComponents);
    
#if DOCKING_DEBUG_ENABLED
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
            FString::Printf(TEXT("[DOCKING PORT] Found %d components tagged as 'DockingPoint'"), TaggedComponents.Num()));
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
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
                    FString::Printf(TEXT("[DOCKING PORT] Added docking point: %s at location %s"), 
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
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,
                FString::Printf(TEXT("[DOCKING PORT] WARNING: No docking points found! Tag Scene Components with 'DockingPoint'")));
        }
#endif
        
        UE_LOG(LogTemp, Warning, TEXT("DockingPortModule '%s': No docking points found. Tag Scene Components with 'DockingPoint' to enable docking."), *GetName());
    }
    
    // Warn if fewer docking points than capacity
    if (DockingPoints.Num() < MaxDockedShips)
    {
#if DOCKING_DEBUG_ENABLED
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow,
                FString::Printf(TEXT("[DOCKING PORT] WARNING: Only %d docking points for capacity of %d ships"), 
                    DockingPoints.Num(), MaxDockedShips));
        }
#endif
        
        UE_LOG(LogTemp, Warning, TEXT("DockingPortModule '%s': Only %d docking points defined for MaxDockedShips=%d"), 
            *GetName(), DockingPoints.Num(), MaxDockedShips);
    }
}

USceneComponent* ADockingPortModule::GetAvailableDockingPoint() const
{
#if DOCKING_DEBUG_ENABLED
    // Debug print - function entry
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
            FString::Printf(TEXT("[DOCKING] GetAvailableDockingPoint() called on %s"), *GetName()));
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
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                FString::Printf(TEXT("[DOCKING] ERROR: No docking capacity (%d/%d occupied)"), 
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
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                TEXT("[DOCKING] ERROR: No docking points defined in module"));
        }
#endif
        
        return nullptr;
    }

    // Select the next available docking point based on how many ships are currently docked.
    // This assumes docking points are filled in order and that HasAvailableDocking()
    // already enforces that CurrentDockedShips is within a valid range.
    const int32 NextDockingIndex = FMath::Clamp(CurrentDockedShips, 0, DockingPoints.Num() - 1);
    
#if DOCKING_DEBUG_ENABLED
    // Debug print - point found
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("[DOCKING] Docking point found: Index %d of %d points"), 
                NextDockingIndex, DockingPoints.Num()));
    }
#endif
    
    return DockingPoints[NextDockingIndex];
}

bool ADockingPortModule::DockShip()
{
#if DOCKING_DEBUG_ENABLED
    // Debug print - function entry
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
            TEXT("[DOCKING] DockShip() called on station module"));
    }
#endif
    
    if (!HasAvailableDocking())
    {
#if DOCKING_DEBUG_ENABLED
        // Debug print - no capacity
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                FString::Printf(TEXT("[DOCKING] ERROR: Cannot dock - all slots occupied (%d/%d)"), 
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
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("[DOCKING] Ship docked successfully - occupied slots: %d/%d"), 
                CurrentDockedShips, MaxDockedShips));
    }
#endif
    
    return true;
}

bool ADockingPortModule::UndockShip()
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
