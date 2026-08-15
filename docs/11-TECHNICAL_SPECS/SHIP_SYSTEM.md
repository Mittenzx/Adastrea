# Ship System Technical Specification

**Trade Simulator MVP | Player Ship & Flight | Last Updated: August 9, 2026**

---

## 🎯 Overview

Ships are the **player's avatar** — the vehicle for trading. Modular design supports progression from starter to advanced.

**Files**: `Source/Adastrea/Private/Ships/`
**Blueprints**: `Content/Blueprints/Ships/`
**DataAssets**: `Content/DataAssets/Ships/`

---

## 🚀 Core Classes

### ASpaceship (Base Pawn)
```cpp
UCLASS(Blueprintable, BlueprintType)
class ASpaceship : public APawn
{
    GENERATED_BODY()
public:
    // Mesh
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    UStaticMeshComponent* ShipMesh;
    
    // Movement
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    USpaceshipControlsComponent* ControlsComponent;
    
    // Customization
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    UShipCustomizationComponent* CustomizationComponent;
    
    // Cargo (for trading)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Trading")
    UCargoComponent* CargoComponent;
    
    // Trading logic
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Trading")
    UPlayerTraderComponent* PlayerTraderComponent;
    
    // DataAsset reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship")
    USpaceshipDataAsset* ShipDataAsset;
    
    // Lifecycle
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    
    // DataAsset application
    UFUNCTION(BlueprintCallable, Category="Ship")
    void ApplyShipDataAsset(USpaceshipDataAsset* DataAsset);
    
    // Cargo delegation
    UFUNCTION(BlueprintCallable, Category="Trading")
    UCargoComponent* GetCargoComponent() const { return CargoComponent; }
    
    UFUNCTION(BlueprintCallable, Category="Trading")
    UPlayerTraderComponent* GetPlayerTraderComponent() const { return PlayerTraderComponent; }
    
    // Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FShipDataAppliedDelegate, USpaceshipDataAsset*, DataAsset);
    UPROPERTY(BlueprintAssignable, Category="Ship")
    FShipDataAppliedDelegate OnShipDataApplied;
};
```

### USpaceshipControlsComponent (Flight Model)
```cpp
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class USpaceshipControlsComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    // Flight parameters (from DataAsset)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flight")
    float MaxSpeed = 2000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flight")
    float Acceleration = 500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flight")
    float TurnRate = 90.0f;           // Degrees/sec
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flight")
    float PitchRate = 90.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flight")
    float RollRate = 180.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flight")
    float Damping = 0.98f;            // Velocity retention per frame
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flight")
    float BoostMultiplier = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flight")
    float BoostDuration = 5.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flight")
    float BoostCooldown = 10.0f;
    
    // Input (Enhanced Input)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
    UInputAction* ThrustAction;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
    UInputAction* StrafeAction;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
    UInputAction* TurnAction;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
    UInputAction* BoostAction;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
    UInputAction* BrakeAction;
    
    // Flight state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State")
    FVector CurrentVelocity;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State")
    bool bIsBoosting = false;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State")
    float BoostTimer = 0.0f;
    
    // Movement functions
    UFUNCTION(BlueprintCallable, Category="Flight")
    void Thrust(float Value);
    
    UFUNCTION(BlueprintCallable, Category="Flight")
    void Strafe(float Value);
    
    UFUNCTION(BlueprintCallable, Category="Flight")
    void Turn(float Yaw, float Pitch, float Roll);
    
    UFUNCTION(BlueprintCallable, Category="Flight")
    void ActivateBoost();
    
    UFUNCTION(BlueprintCallable, Category="Flight")
    void Brake();
    
    // Tick
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    
    // Apply DataAsset settings
    UFUNCTION(BlueprintCallable, Category="Flight")
    void ApplyFlightData(const FShipFlightData& FlightData);
};
```

### UShipCustomizationComponent (Progression)
```cpp
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UShipCustomizationComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    // Module slots
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Slots")
    int32 EngineSlots = 1;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Slots")
    int32 ShieldSlots = 1;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Slots")
    int32 WeaponSlots = 0;        // MVP: No weapons
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Slots")
    int32 UtilitySlots = 2;
    
    // Installed modules
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Modules")
    TArray<UShipModuleComponent*> InstalledModules;
    
    // Visual
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visual")
    TArray<FShipVisualVariant> VisualVariants;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Visual")
    int32 CurrentVisualVariant = 0;
    
    // Module management
    UFUNCTION(BlueprintCallable, Category="Customization")
    bool InstallModule(UShipModuleComponent* Module);
    
    UFUNCTION(BlueprintCallable, Category="Customization")
    bool UninstallModule(UShipModuleComponent* Module);
    
    UFUNCTION(BlueprintCallable, Category="Customization")
    TArray<UShipModuleComponent*> GetModulesByType(EShipModuleType Type) const;
    
    // Visual variants
    UFUNCTION(BlueprintCallable, Category="Visual")
    void SetVisualVariant(int32 VariantIndex);
    
    // Apply DataAsset
    UFUNCTION(BlueprintCallable, Category="Customization")
    void ApplyShipData(USpaceshipDataAsset* DataAsset);
};
```

### UShipModuleComponent (Equipment)
```cpp
UCLASS(Blueprintable, Abstract, BlueprintType)
class UShipModuleComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module")
    FText ModuleName;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module")
    EShipModuleType ModuleType;     // Engine, Shield, Weapon, Utility
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module")
    int32 SlotSize = 1;             // Some modules take multiple slots
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module")
    float PowerDraw = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module")
    int32 CreditCost = 1000;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Module")
    int32 TechLevel = 1;            // 1-5, limits where available
    
    // Stats by module type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Engine")
    float SpeedBonus = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Engine")
    float AccelerationBonus = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Shield")
    float ShieldCapacity = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Shield")
    float ShieldRegenRate = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Cargo")
    int32 CargoCapacityBonus = 0;
    
    // Installation
    UFUNCTION(BlueprintNativeEvent, Category="Module")
    void OnInstalled(ASpaceship* Ship);
    
    UFUNCTION(BlueprintNativeEvent, Category="Module")
    void OnUninstalled(ASpaceship* Ship);
};
```

---

## 📦 DataAsset: USpaceshipDataAsset

```cpp
UCLASS(Blueprintable, BlueprintType)
class USpaceshipDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    FText ShipName;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Identity")
    FName ShipID;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Class")
    EShipClass ShipClass;           // Starter, Freighter, Corvette, Cruiser, etc.
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Class")
    int32 TechLevel = 1;
    
    // Base stats
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
    float BaseMaxSpeed = 2000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
    float BaseAcceleration = 500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
    float BaseTurnRate = 90.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
    int32 BaseCargoCapacity = 50;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
    int32 BaseShieldCapacity = 100;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
    int32 BaseHullIntegrity = 1000;
    
    // Module slots
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slots")
    int32 EngineSlots = 1;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slots")
    int32 ShieldSlots = 1;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slots")
    int32 WeaponSlots = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slots")
    int32 UtilitySlots = 2;
    
    // Default modules
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Defaults")
    TArray<TSoftObjectPtr<UShipModuleComponent>> DefaultModules;
    
    // Visual
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visual")
    TSoftObjectPtr<UStaticMesh> ShipMesh;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visual")
    TArray<FShipVisualVariant> VisualVariants;
    
    // Economy
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Economy")
    int32 PurchasePrice = 50000;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Economy")
    TArray<FName> RequiredReputation;  // Faction locks (deferred)
    
    // Progression
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progression")
    TArray<FShipUpgradePath> UpgradePaths;
};
```

---

## 🎮 MVP Ship Progression (3 Ships)

| Ship | Class | Cargo | Speed | Slots | Price | DataAsset |
|------|-------|-------|-------|-------|-------|-----------|
| **Starter** | Starter | 50 | 2000 | E:1, S:1, U:2 | 10,000 | `DA_Ship_Starter` |
| **Mid** | Freighter | 120 | 1500 | E:2, S:2, U:3 | 100,000 | `DA_Ship_Freighter` |
| **Advanced** | Corvette | 80 | 3000 | E:2, S:2, U:4, W:1 | 500,000 | `DA_Ship_Corvette` |

**Progression Logic**:
1. Start with Starter (given free)
2. Trade → Earn credits
3. Buy Freighter at any station with Shipyard module (deferred)
4. Trade more → Earn more
5. Buy Corvette

---

## ⌨️ Input Mapping (Enhanced Input)

### Input Actions (InputConfigDataAsset)
| Action | Type | Keys | Purpose |
|--------|------|------|---------|
| `IA_Thrust` | Axis1D | W / S | Forward/Backward |
| `IA_Strafe` | Axis2D | A/D, Q/E | Left/Right, Up/Down |
| `IA_Turn` | Axis2D | Mouse X/Y | Yaw/Pitch |
| `IA_Roll` | Axis1D | Z/C | Roll |
| `IA_Boost` | Digital | Left Shift | Temporary speed boost |
| `IA_Brake` | Digital | Space | Dampen velocity |
| `IA_Dock` | Digital | F | Request docking |

### Input Mapping Context
```cpp
// Priority: 0 (highest)
InputMappingContext_ShipFlight
  → IA_Thrust → Triggered → SpaceshipControlsComponent.Thrust()
  → IA_Strafe → Triggered → SpaceshipControlsComponent.Strafe()
  → IA_Turn → Triggered → SpaceshipControlsComponent.Turn()
  → IA_Boost → Started → SpaceshipControlsComponent.ActivateBoost()
  → IA_Brake → Started → SpaceshipControlsComponent.Brake()
  → IA_Dock → Started → PlayerController.RequestDock()
```

---

## 🎨 Visual Variants

```cpp
USTRUCT(BlueprintType)
struct FShipVisualVariant
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText VariantName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UStaticMesh> MeshOverride;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TPair<FName, UMaterialInterface*>> MaterialOverrides; // SlotName -> Material
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor EngineColor = FLinearColor::Blue;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor LightColor = FLinearColor::White;
};
```

---

## 🔧 Blueprint Setup (MVP)

### BP_TradingShip (Derived from ASpaceship)
```ini
; Components (auto-added from C++ constructor)
; ShipMesh (StaticMeshComponent)
; ControlsComponent (SpaceshipControlsComponent)
; CustomizationComponent (ShipCustomizationComponent)
; CargoComponent (CargoComponent)
; PlayerTraderComponent (PlayerTraderComponent)

; Class Defaults
ShipDataAsset = DA_Ship_Starter
AutoPossessPlayer = Player0
```

### BP_Ship_Freighter / BP_Ship_Corvette
```ini
; Derived from BP_TradingShip
ShipDataAsset = DA_Ship_Freighter  ; or DA_Ship_Corvette
; Mesh, stats, slots all come from DataAsset
```

---

## 🧪 Testing

```bash
# No dedicated ship test yet
# Test via trading system integration:
python test_trading_system.py  # Validates cargo capacity from ship
```

---

## 📋 MVP Checklist

- [x] ASpaceship C++ complete
- [x] USpaceshipControlsComponent C++ complete
- [x] UShipCustomizationComponent C++ complete
- [x] UShipModuleComponent C++ complete
- [x] USpaceshipDataAsset C++ complete
- [x] 3 Ship DataAssets created (DA_Ship_Starter, _Freighter, _Corvette)
- [x] BP_PlayerShip Blueprint exists
- [x] BP_Ship_Freighter Blueprint exists
- [x] BP_Ship_Corvette Blueprint exists
- [ ] BP_TradingShip Blueprint created & wired with components
- [ ] InputConfigDataAsset assigned
- [ ] Enhanced Input Mapping Context configured
- [ ] PIE test: flight controls responsive
- [ ] PIE test: CargoComponent accessible
- [ ] PIE test: PlayerTraderComponent accessible

---

*Back to [System Reference](../06-SYSTEM_REFERENCE.md) | [INDEX.md](../INDEX.md)*