// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/DockingBayModule.h"

// Debug flag for docking system - can be disabled for shipping builds
#ifndef DOCKING_DEBUG_ENABLED
    #define DOCKING_DEBUG_ENABLED 1
#endif

ADockingBayModule::ADockingBayModule()
{
    ModuleType = TEXT("Docking Bay");
    ModulePower = 50.0f;
    ModuleGroup = EStationModuleGroup::Docking;
}

USceneComponent* ADockingBayModule::GetAvailableDockingPoint() const
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

bool ADockingBayModule::DockShip()
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
