// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/DockingBayModule.h"

ADockingBayModule::ADockingBayModule()
{
    ModuleType = TEXT("Docking Bay");
    ModulePower = 50.0f;
    ModuleGroup = EStationModuleGroup::Docking;
}

USceneComponent* ADockingBayModule::GetAvailableDockingPoint() const
{
    // Debug print - function entry
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
            FString::Printf(TEXT("[DOCKING] GetAvailableDockingPoint() called on %s"), *GetName()));
    }
    
    // Only provide a docking point if we have capacity and at least one point defined
    if (!HasAvailableDocking())
    {
        // Debug print - no capacity
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                FString::Printf(TEXT("[DOCKING] ERROR: No docking capacity (%d/%d occupied)"), 
                    CurrentDockedShips, MaxDockedShips));
        }
        
        return nullptr;
    }
    
    if (DockingPoints.Num() <= 0)
    {
        // Debug print - no docking points
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                TEXT("[DOCKING] ERROR: No docking points defined in module"));
        }
        
        return nullptr;
    }

    // Select the next available docking point based on how many ships are currently docked.
    // This assumes docking points are filled in order and that HasAvailableDocking()
    // already enforces that CurrentDockedShips is within a valid range.
    const int32 NextDockingIndex = FMath::Clamp(CurrentDockedShips, 0, DockingPoints.Num() - 1);
    
    // Debug print - point found
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("[DOCKING] Docking point found: Index %d of %d points"), 
                NextDockingIndex, DockingPoints.Num()));
    }
    
    return DockingPoints[NextDockingIndex];
}

bool ADockingBayModule::DockShip()
{
    // Debug print - function entry
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
            TEXT("[DOCKING] DockShip() called on station module"));
    }
    
    if (!HasAvailableDocking())
    {
        // Debug print - no capacity
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                FString::Printf(TEXT("[DOCKING] ERROR: Cannot dock - all slots occupied (%d/%d)"), 
                    CurrentDockedShips, MaxDockedShips));
        }
        
        return false;
    }

    CurrentDockedShips++;
    
    // Debug print - docked successfully
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("[DOCKING] Ship docked successfully - occupied slots: %d/%d"), 
                CurrentDockedShips, MaxDockedShips));
    }
    
    return true;
}

bool ADockingBayModule::UndockShip()
{
    // Debug print - function entry
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
            TEXT("[UNDOCKING] UndockShip() called on station module"));
    }
    
    if (CurrentDockedShips <= 0)
    {
        // Debug print - no ships docked
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                TEXT("[UNDOCKING] WARNING: No ships currently docked"));
        }
        
        return false;
    }

    CurrentDockedShips--;
    
    // Debug print - undocked successfully
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("[UNDOCKING] Ship undocked successfully - occupied slots: %d/%d"), 
                CurrentDockedShips, MaxDockedShips));
    }
    
    return true;
}
