# Station System Technical Specification

**Trade Simulator MVP | Modular Station Architecture | Last Updated: August 9, 2026**

---

## 🎯 Overview

Stations are **modular economic hubs** — the primary locations for trading. Built from `SpaceStationModule` actors attached to a root `ASpaceStation`.

**Files**: `Source/Adastrea/Private/Stations/`
**Blueprints**: `Content/Blueprints/Stations/`, `Content/Blueprints/Stations/Modules/`

---

## 🏗️ Core Classes

### ASpaceStation (Root Actor)
```cpp
UCLASS(Blueprintable, BlueprintType)
class ASpaceStation : public AActor
{
    GENERATED_BODY()
public:
    // Modules to spawn on BeginPlay
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station|Modules")
    TArray<TSubclassOf<ASpaceStationModule>> DefaultModuleClasses;
    
    // Spawned module instances
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Station|Modules")
    TArray<ASpaceStationModule*> ActiveModules;
    
    // Power grid
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station|Power")
    float BasePowerGeneration = 0.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Station|Power")
    float TotalPowerGenerated = 0.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Station|Power")
    float TotalPowerRequired = 0.0f;
    
    // Lifecycle
    virtual void BeginPlay() override;
    
    // Module management
    UFUNCTION(BlueprintCallable, Category="Station|Modules")
    void SpawnDefaultModules();
    
    UFUNCTION(BlueprintCallable, Category="Station|Modules")
    ASpaceStationModule* SpawnModule(TSubclassOf<ASpaceStationModule> ModuleClass, FTransform Transform);
    
    UFUNCTION(BlueprintCallable, Category="Station|Modules")
    TArray<ASpaceStationModule*> GetModulesOfType(TSubclassOf<ASpaceStationModule> ModuleClass) const;
    
    // Trading shortcuts
    UFUNCTION(BlueprintCallable, Category="Station|Trading")
    bool HasMarketplace() const;
    
    UFUNCTION(BlueprintCallable, Category="Station|Trading")
    AMarketplaceModule* GetMarketplaceModule() const;
    
    UFUNCTION(BlueprintCallable, Category="Station|Trading")
    ADockingBayModule* GetDockingBayModule() const;
    
    // Power calculations
    UFUNCTION(BlueprintCallable, Category="Station|Power")
    float GetPowerBalance() const { return TotalPowerGenerated - TotalPowerRequired; }
    
    UFUNCTION(BlueprintCallable, Category="Station|Power")
    float GetPowerEfficiency() const;
    
    // Integrity (station health)
    UFUNCTION(BlueprintCallable, Category="Station|Integrity")
    float GetOverallIntegrity() const;
    
    // Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FModuleSpawnedDelegate, ASpaceStationModule*, Module);
    UPROPERTY(BlueprintAssignable, Category="Station|Modules")
    FModuleSpawnedDelegate OnModuleSpawned;
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FModuleDestroyedDelegate, ASpaceStationModule*, Module);
    UPROPERTY(BlueprintAssignable, Category="Station|Modules")
    FModuleDestroyedDelegate OnModuleDestroyed;
};
```

### ASpaceStationModule (Base Class)
```cpp
UCLASS(Blueprintable, Abstract, BlueprintType)
class ASpaceStationModule : public AActor
{
    GENERATED_BODY()
public:
    // Module identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module")
    FText ModuleName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module")
    FName ModuleType;               // "Marketplace", "DockingBay", "CargoBay", etc.
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module")
    EStationModuleGroup ModuleGroup; // Public, Industrial, Military, Utility
    
    // Power
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Power")
    float ModulePower = 0.0f;       // Negative = consumes, Positive = generates
    
    // Integrity (health)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Integrity")
    float BaseIntegrity = 100.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Integrity")
    float CurrentIntegrity = 100.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Integrity")
    EModuleIntegrityStatus IntegrityStatus = EModuleIntegrityStatus::Operational;
    
    // Parent station
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Station")
    ASpaceStation* ParentStation;
    
    // Lifecycle
    virtual void BeginPlay() override;
    virtual void OnRegisteredToStation(ASpaceStation* Station);
    virtual void OnUnregisteredFromStation();
    
    // Integrity management
    UFUNCTION(BlueprintCallable, Category="Integrity")
    void TakeDamage(float Damage);
    
    UFUNCTION(BlueprintCallable, Category="Integrity")
    void Repair(float Amount);
    
    UFUNCTION(BlueprintCallable, Category="Integrity")
    void UpdateIntegrityStatus();
    
    // Power
    UFUNCTION(BlueprintPure, Category="Power")
    float GetPowerConsumption() const { return FMath::Abs(FMath::Min(ModulePower, 0.0f)); }
    
    UFUNCTION(BlueprintPure, Category="Power")
    float GetPowerGeneration() const { return FMath::Max(ModulePower, 0.0f); }
    
    // Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIntegrityChangedDelegate, float, NewIntegrity);
    UPROPERTY(BlueprintAssignable, Category="Integrity")
    FIntegrityChangedDelegate OnIntegrityChanged;
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStatusChangedDelegate, EModuleIntegrityStatus, NewStatus);
    UPROPERTY(BlueprintAssignable, Category="Integrity")
    FStatusChangedDelegate OnStatusChanged;
};
```

### AMarketplaceModule (Trading Facility)
```cpp
UCLASS(Blueprintable, BlueprintType)
class AMarketplaceModule : public ASpaceStationModule
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Marketplace")
    UMarketDataAsset* MarketDataAsset;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Marketplace")
    bool bIsOpen = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Marketplace")
    FText MarketplaceName = FText::FromString(TEXT("Marketplace"));
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Marketplace")
    FText MarketDescription;
    
    // Trading availability
    UFUNCTION(BlueprintPure, Category="Marketplace")
    UMarketDataAsset* GetMarketData() const { return MarketDataAsset; }
    
    UFUNCTION(BlueprintPure, Category="Marketplace")
    bool IsAvailableForTrading() const { return bIsOpen && MarketDataAsset != nullptr; }
    
    UFUNCTION(BlueprintCallable, Category="Marketplace")
    void SetMarketData(UMarketDataAsset* NewMarketData) { MarketDataAsset = NewMarketData; }
    
    UFUNCTION(BlueprintCallable, Category="Marketplace")
    void SetOpen(bool bOpen) { bIsOpen = bOpen; }
    
    // Power: Marketplace consumes 40 units
    AMarketplaceModule() { ModulePower = -40.0f; ModuleType = TEXT("Marketplace"); ModuleGroup = EStationModuleGroup::Public; }
};
```

### ADockingBayModule (Ship Docking)
```cpp
UCLASS(Blueprintable, BlueprintType)
class ADockingBayModule : public ASpaceStationModule
{
    GENERATED_BODY()
public:
    // Docking ports
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Docking")
    TArray<FDockingPort> DockingPorts;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Docking")
    int32 MaxSimultaneousDocks = 4;
    
    // Docking logic
    UFUNCTION(BlueprintCallable, Category="Docking")
    bool RequestDock(ASpaceship* Ship, int32& OutPortIndex);
    
    UFUNCTION(BlueprintCallable, Category="Docking")
    void UndockShip(ASpaceship* Ship);
    
    UFUNCTION(BlueprintPure, Category="Docking")
    bool HasAvailablePort() const;
    
    UFUNCTION(BlueprintPure, Category="Docking")
    int32 GetOccupiedPorts() const;
    
    // Power: Docking bay consumes 30 units
    ADockingBayModule() { ModulePower = -30.0f; ModuleType = TEXT("DockingBay"); ModuleGroup = EStationModuleGroup::Public; }
};
```

---

## ⚡ Power System

```cpp
// Station calculates power on module changes
void ASpaceStation::RecalculatePower()
{
    TotalPowerGenerated = BasePowerGeneration;
    TotalPowerRequired = 0.0f;
    
    for (ASpaceStationModule* Module : ActiveModules)
    {
        if (Module->ModulePower > 0)
            TotalPowerGenerated += Module->ModulePower;
        else
            TotalPowerRequired += FMath::Abs(Module->ModulePower);
    }
    
    // Distribute power to modules proportionally
    float Efficiency = GetPowerEfficiency(); // 0.0 - 1.0
    for (ASpaceStationModule* Module : ActiveModules)
    {
        Module->SetPowerLevel(Efficiency); // Module handles its own power state
    }
}

float ASpaceStation::GetPowerEfficiency() const
{
    if (TotalPowerRequired <= 0) return 1.0f;
    return FMath::Clamp(TotalPowerGenerated / TotalPowerRequired, 0.0f, 1.0f);
}
```

**Module Power Levels**:
- `EModulePowerLevel::FullPower` (1.0) — Normal operation
- `EModulePowerLevel::MediumPower` (0.5) — Reduced function
- `EModulePowerLevel::LowPower` (0.25) — Minimal function
- `EModulePowerLevel::CriticalPower` (0.0) — Offline

---

## 🛡️ Integrity System

```cpp
// Module integrity states
UENUM(BlueprintType)
enum class EModuleIntegrityStatus : uint8
{
    Operational,    // 100% - 75%
    Degraded,       // 74% - 50%
    Damaged,        // 49% - 25%
    Critical,       // 24% - 1%
    Destroyed       // 0%
};

// Status update logic
void ASpaceStationModule::UpdateIntegrityStatus()
{
    float Ratio = CurrentIntegrity / BaseIntegrity;
    
    if (Ratio >= 0.75f) IntegrityStatus = EModuleIntegrityStatus::Operational;
    else if (Ratio >= 0.50f) IntegrityStatus = EModuleIntegrityStatus::Degraded;
    else if (Ratio >= 0.25f) IntegrityStatus = EModuleIntegrityStatus::Damaged;
    else if (Ratio > 0.0f) IntegrityStatus = EModuleIntegrityStatus::Critical;
    else IntegrityStatus = EModuleIntegrityStatus::Destroyed;
    
    OnStatusChanged.Broadcast(IntegrityStatus);
    
    // Disable functionality at Critical/Destroyed
    if (IntegrityStatus >= EModuleIntegrityStatus::Critical)
    {
        SetModuleEnabled(false);
    }
}
```

---

## 📦 Module Types (MVP Relevant)

| Module | Class | Power | Purpose | MVP Required |
|--------|-------|-------|---------|--------------|
| Marketplace | `AMarketplaceModule` | -40 | Trading facility | ✅ YES |
| Docking Bay | `ADockingBayModule` | -30 | Ship docking | ✅ YES |
| Cargo Bay | `ACargoBayModule` | -20 | Station storage | ✅ Recommended |
| Reactor | `AReactorModule` | +200 | Power generation | ⚠️ If needed |
| Solar Array | `ASolarArrayModule` | +50 | Power generation | ⚠️ If needed |
| Shield Gen | `AShieldGeneratorModule` | -60 | Defense | ❌ Deferred |
| Turret | `ATurretModule` | -40 | Defense | ❌ Deferred |
| Habitation | `AHabitationModule` | -25 | Crew quarters | ❌ Deferred |
| Barracks | `ABarracksModule` | -30 | NPC housing | ❌ Deferred |
| Fabrication | `AFabricationModule` | -50 | Crafting | ❌ Deferred |
| Science Lab | `AScienceLabModule` | -40 | Research | ❌ Deferred |
| Fuel Depot | `AFuelDepotModule` | -30 | Refueling | ❌ Deferred |
| Processing | `AProcessingModule` | -40 | Refining | ❌ Deferred |

---

## 🎮 Blueprint Setup (MVP)

### BP_TradeStation (Derived from ASpaceStation)
```ini
; Class Defaults
DefaultModuleClasses[0] = BP_SpaceStationModule_DockingBay
DefaultModuleClasses[1] = BP_SpaceStationModule_Marketplace
DefaultModuleClasses[2] = BP_CargoBayModule
; Optional: BP_ReactorModule, BP_SolarArrayModule

BasePowerGeneration = 0  ; Power comes from Reactor/Solar modules
```

### BP_SpaceStationModule_Marketplace (Derived from AMarketplaceModule)
```ini
; Class Defaults
MarketDataAsset = DA_Market_Agricultural  ; Per-station override
bIsOpen = true
MarketplaceName = "Agricultural Exchange"
ModulePower = -40
```

---

## 🧪 Testing

```bash
# Validates integrity, power, trading efficiency calculations
python test_station_systems.py

# Output includes:
# - Module integrity status transitions
# - Power distribution scenarios (adequate/deficit/critical)
# - Trading efficiency by station type
# - Error case handling
```

---

## 📋 MVP Checklist

- [x] ASpaceStation C++ complete
- [x] ASpaceStationModule C++ complete
- [x] AMarketplaceModule C++ complete
- [x] ADockingBayModule C++ complete
- [x] ACargoBayModule C++ complete
- [x] Power system C++ complete
- [x] Integrity system C++ complete
- [x] BP_SpaceStationModule_DockingBay Blueprint exists
- [x] BP_SpaceStationModule_Marketplace Blueprint exists
- [x] BP_CargoBayModule Blueprint exists
- [ ] BP_TradeStation Blueprint created & wired
- [ ] MarketDataAssets assigned to marketplace modules
- [ ] Test level with 3+ stations placed
- [ ] PIE test: dock → trade → undock

---

*Back to [System Reference](../06-SYSTEM_REFERENCE.md) | [INDEX.md](../INDEX.md)*