# HUD Quick Reference

Quick reference guide for the Adastrea HUD system.

## Player Info Display (C++)

```cpp
// Individual updates
HUDWidget->UpdatePlayerName(FText::FromString("Captain"));
HUDWidget->UpdatePlayerLevel(15);
HUDWidget->UpdatePlayerCredits(50000);

// Batch update
HUDWidget->UpdatePlayerInfo(
    FText::FromString("Captain"), 15, 50000
);

// Auto-refresh from Game Instance
HUDWidget->RefreshPlayerInfo();
```

## Ship Info Display (C++)

```cpp
// Individual updates
HUDWidget->UpdateShipName(FText::FromString("USS Enterprise"));
HUDWidget->UpdateShipClass(FText::FromString("Cruiser"));
HUDWidget->UpdateShipIntegrity(750.0f, 1000.0f);

// Batch update
HUDWidget->UpdateShipInfo(
    FText::FromString("USS Enterprise"),
    FText::FromString("Cruiser"),
    750.0f, 1000.0f
);

// Auto-refresh from Spaceship
HUDWidget->RefreshShipInfo();
```

## Ship Status Display (C++)

```cpp
HUDWidget->UpdateHealth(800.0f, 1000.0f);
HUDWidget->UpdateShield(450.0f, 500.0f);
HUDWidget->UpdateSpeed(2500.0f, 3000.0f);
HUDWidget->UpdateWeaponStatus(0, 85.0f, 100.0f);
HUDWidget->ShowAlert(FText::FromString("Warning!"), 3.0f, true);
```

## Setting Up a Spaceship with Data Asset

1. Create `USpaceshipDataAsset` in editor
2. Fill in `ShipName`, `ShipClass`, `HullStrength`, etc.
3. Assign to spaceship's `Ship Data Asset` property
4. Ship automatically initializes on BeginPlay

## Blueprint Nodes

**Player Info:**
- `Update Player Name`
- `Update Player Level`
- `Update Player Credits`
- `Update Player Info` (batch)
- `Refresh Player Info` (auto)

**Ship Info:**
- `Update Ship Name`
- `Update Ship Class`
- `Update Ship Integrity`
- `Update Ship Info` (batch)
- `Refresh Ship Info` (auto)

**Ship Status:**
- `Update Health`
- `Update Shield`
- `Update Speed`
- `Update Weapon Status`
- `Update Target Info`
- `Clear Target Info`
- `Show Alert`

## State Variables (Blueprint ReadOnly)

```
CurrentPlayerName (Text)
CurrentPlayerLevel (Integer)
CurrentPlayerCredits (Integer)
CurrentShipName (Text)
CurrentShipClass (Text)
ShipIntegrityPercent (Float 0-1)
HealthPercent (Float 0-1)
ShieldPercent (Float 0-1)
CurrentSpeedValue (Float)
bHasTarget (Boolean)
ControlledSpaceship (Spaceship Reference)
```

## Auto-Update Pattern (Blueprint)

Override `UpdateHUDFromGameState`:
```
Event UpdateHUDFromGameState
  ├─ Get Controlled Spaceship
  ├─ Is Valid?
  │   ├─ Refresh Ship Info
  │   ├─ Get Speed
  │   └─ Update Speed Display
  └─ Parent: UpdateHUDFromGameState
```

## Common Patterns

### On Ship Possession
```cpp
void OnPossessShip(ASpaceship* Ship)
{
    if (HUDWidget)
    {
        HUDWidget->RefreshShipInfo();
    }
}
```

### On Credits Change
```cpp
void OnCreditsChanged(int32 NewAmount)
{
    if (HUDWidget)
    {
        HUDWidget->UpdatePlayerCredits(NewAmount);
    }
}
```

### On Ship Damage
```cpp
void OnShipDamaged()
{
    if (HUDWidget)
    {
        HUDWidget->RefreshShipInfo();
        // Or manually:
        // HUDWidget->UpdateShipIntegrity(CurrentHull, MaxHull);
    }
}
```

## Troubleshooting Quick Fixes

**HUD not showing ship name?**
→ Assign `SpaceshipDataAsset` to ship

**Player info shows defaults?**
→ Call `RefreshPlayerInfo()` or manually `UpdatePlayerInfo()`

**HUD not updating?**
→ Ensure widget is added to viewport and visible

**Performance issues?**
→ Don't update every frame, use timers for infrequent updates

---

For complete documentation, see `HUD_USAGE_GUIDE.md`
