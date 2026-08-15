# Data Asset Guide

**Trade Simulator MVP | Creating & Managing DataAssets | Last Updated: August 9, 2026**

---

## 🎯 Overview

**All game content lives in DataAssets** — not hardcoded values. This enables designers to balance without C++ changes.

**Asset Manager Registration Required**: All PrimaryDataAssets must be registered in Project Settings → Game → Asset Manager.

---

## 📦 DataAsset Types (MVP)

| DataAsset | Module | Purpose | Count |
|-----------|--------|---------|-------|
| `UTradeItemDataAsset` | Trading | Tradeable goods definitions | 20 target |
| `UMarketDataAsset` | Trading | Station market configurations | 10 target |
| `USpaceshipDataAsset` | Ships | Ship class definitions | 3 target |
| `UInputConfigDataAsset` | Input | Control schemes | 1 |

---

## 🛠️ Creating DataAssets

### In Editor
```
Content Browser → Right-click → Data Assets → [Select Type]
```

### Naming Convention
```
DA_TradeItem_[Name]          // e.g., DA_TradeItem_FoodRations
DA_Market_[StationName]      // e.g., DA_Market_Agricultural
DA_Ship_[ClassName]          // e.g., DA_Ship_Starter
```

### Folder Structure
```
Content/DataAssets/
├── Trading/
│   ├── Items/           # TradeItemDataAsset
│   └── Markets/         # MarketDataAsset
├── Ships/               # SpaceshipDataAsset
└── Input/               # InputConfigDataAsset
```

---

## 📝 TradeItemDataAsset — Field Guide

| Field | Type | Required | Description | Example |
|-------|------|----------|-------------|---------|
| ItemName | FText | ✅ | Display name | "Food Rations" |
| ItemID | FName | ✅ | Stable save ID | "food_rations" |
| Category | Enum | ✅ | Food/Tech/Industrial/Luxury/Resources | Food |
| BasePrice | float | ✅ | Base credits/unit | 25.0 |
| PriceVolatility | Struct | ✅ | Min/Max deviation, multiplier | 0.5-2.0x, 1.3x |
| VolumePerUnit | float | ✅ | Cargo space per unit | 1.0 |
| MassPerUnit | float | ✅ | Physics mass | 1.0 |
| StandardLotSize | int32 | ✅ | Default trade qty | 100 |
| TypicalMarketStock | int32 | ✅ | Baseline supply | 1000 |
| ReplenishmentRate | int32 | ✅ | Per refresh cycle | 100 |
| AITradePriority | int32 | ✅ | 1-10 | 5 |
| BehaviorTags | Array<FName> | ❌ | "Perishable", "Illegal" | - |
| bAffectedBySupplyDemand | bool | ✅ | Enable S/D pricing | true |
| bAffectedByMarketEvents | bool | ✅ | Enable event pricing | true |

### PriceVolatility Struct
```cpp
// In DataAsset defaults
PriceVolatility.MinPriceDeviation = 0.5    // 50% of base minimum
PriceVolatility.MaxPriceDeviation = 2.0    // 200% of base maximum
PriceVolatility.VolatilityMultiplier = 1.3 // Volatility intensity
```

### Validation Rules (Editor)
- ItemName not empty
- ItemID not "None"
- BasePrice > 0
- VolumePerUnit > 0
- MassPerUnit > 0
- StandardLotSize > 0
- AITradePriority 1-10

---

## 🏪 MarketDataAsset — Field Guide

| Field | Type | Required | Description | Example |
|-------|------|----------|-------------|---------|
| MarketName | FText | ✅ | Display name | "Agricultural Exchange" |
| MarketID | FName | ✅ | Stable ID | "market_agricultural" |
| MarketType | Enum | ✅ | OpenMarket/LuxuryBazaar/IndustrialDepot/ResearchHub | OpenMarket |
| MarketSize | Enum | ✅ | Small/Medium/Large/Massive | Medium |
| SellPriceMarkup | float | ✅ | Player buys multiplier | 1.2 |
| BuyPriceMarkdown | float | ✅ | Player sells multiplier | 0.8 |
| TransactionTaxRate | float | ✅ | Tax on all trades | 0.05 |
| bAllowPlayerBuying | bool | ✅ | Player can buy | true |
| bAllowPlayerSelling | bool | ✅ | Player can sell | true |
| StockRefreshRate | float | ✅ | Hours between refresh | 24.0 |
| RandomEventChance | float | ✅ | Per-refresh event chance | 0.1 |
| Inventory | Array | ✅ | Per-item supply/demand/stock | See below |

### Inventory Entry (Per Item)
| Field | Type | Description |
|-------|------|-------------|
| TradeItem | Object | Reference to TradeItemDataAsset |
| CurrentStock | int32 | Available units |
| MaxStock | int32 | Capacity |
| SupplyLevel | float | 0.0-2.0 (affects price) |
| DemandLevel | float | 0.0-2.0 (affects price) |
| bInStock | bool | Auto-calculated |

### Market Type Behaviors

| Type | Markup | Markdown | Refresh | Event Chance | Focus |
|------|--------|----------|---------|--------------|-------|
| OpenMarket | 1.2 | 0.8 | 24h | 10% | General |
| LuxuryBazaar | 1.5 | 0.6 | 72h | 20% | Luxury goods |
| IndustrialDepot | 1.1 | 0.9 | 12h | 15% | Industrial |
| ResearchHub | 1.2 | 0.8 | 48h | 5% | Technology |

---

## 🚀 SpaceshipDataAsset — Field Guide

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| ShipName | FText | ✅ | "Starter Freighter" |
| ShipID | FName | ✅ | "ship_starter" |
| ShipClass | Enum | ✅ | Starter/Freighter/Corvette/... |
| TechLevel | int32 | ✅ | 1-5 |
| BaseMaxSpeed | float | ✅ | 2000.0 |
| BaseAcceleration | float | ✅ | 500.0 |
| BaseTurnRate | float | ✅ | 90.0 |
| BaseCargoCapacity | int32 | ✅ | 50 |
| BaseShieldCapacity | int32 | ✅ | 100 |
| BaseHullIntegrity | int32 | ✅ | 1000 |
| EngineSlots | int32 | ✅ | 1 |
| ShieldSlots | int32 | ✅ | 1 |
| WeaponSlots | int32 | ✅ | 0 (MVP) |
| UtilitySlots | int32 | ✅ | 2 |
| DefaultModules | Array | ❌ | Pre-installed modules |
| ShipMesh | SoftObjectPtr | ✅ | StaticMesh reference |
| VisualVariants | Array | ❌ | Material/mesh overrides |
| PurchasePrice | int32 | ✅ | 10000 |
| UpgradePaths | Array | ❌ | Upgrade progression |

---

## 🎮 InputConfigDataAsset

```cpp
// Single instance for project
UCLASS(Blueprintable)
class UInputConfigDataAsset : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UInputMappingContext* ShipFlightContext;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UInputAction* IA_Thrust;
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UInputAction* IA_Strafe;
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UInputAction* IA_Turn;
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UInputAction* IA_Roll;
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UInputAction* IA_Boost;
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UInputAction* IA_Brake;
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UInputAction* IA_Dock;
};
```

---

## ⚙️ Asset Manager Registration

**Required for PrimaryDataAssets to load at runtime**

### Project Settings → Game → Asset Manager
```ini
Primary Asset Types to Scan:
  - TradeItemDataAsset
    - Directories: /Game/DataAssets/Trading/Items
    - Rules: Cook, Load
  - MarketDataAsset
    - Directories: /Game/DataAssets/Trading/Markets
    - Rules: Cook, Load
  - SpaceshipDataAsset
    - Directories: /Game/DataAssets/Ships
    - Rules: Cook, Load
```

### In Code (Auto-registration)
```cpp
// In AdastreaGameInstance::Init()
void UAdastreaGameInstance::Init()
{
    Super::Init();
    
    UAssetManager& AssetManager = UAssetManager::GetIfInitialized();
    if (AssetManager.IsValid())
    {
        // Scan paths on startup
        AssetManager.ScanPathsForPrimaryAssets(
            {TEXT("/Game/DataAssets/Trading/Items"),
             TEXT("/Game/DataAssets/Trading/Markets"),
             TEXT("/Game/DataAssets/Ships")},
            true  // bForceRescan
        );
    }
}
```

---

## 🔄 DataAsset Workflow

### For Designers
1. **Create**: Right-click → Data Asset → Select type
2. **Configure**: Fill all required fields
3. **Validate**: Editor shows errors in tooltip (IsDataValid)
4. **Assign**: Reference in Blueprints (MarketDataAsset on MarketplaceModule)
5. **Test**: PIE → verify prices, cargo, progression

### For Programmers
1. **Add Fields**: Edit `.h` → recompile → Editor picks up
2. **Validation**: Implement `IsDataValid()` for Editor checks
3. **Blueprint Events**: Use `BlueprintNativeEvent` for overrides
3. **Save Compatibility**: Never rename ItemID/MarketID/ShipID after release

---

## 📋 MVP Checklist

### Trade Items (20 Target)
- [ ] DA_TradeItem_FoodRations
- [ ] DA_TradeItem_Water
- [ ] DA_TradeItem_MedicalSupplies
- [ ] DA_TradeItem_Electronics
- [ ] DA_TradeItem_ShipComponents
- [ ] DA_TradeItem_IronOre
- [ ] DA_TradeItem_CopperOre
- [ ] DA_TradeItem_TitaniumOre
- [ ] DA_TradeItem_Platinum
- [ ] DA_TradeItem_RareEarthElements
- [ ] DA_TradeItem_Helium3
- [ ] DA_TradeItem_AntimatterPowerCores
- [ ] DA_TradeItem_QuantumProcessor
- [ ] DA_TradeItem_AICores
- [ ] DA_TradeItem_LuxuryBeverages
- [ ] DA_TradeItem_Artwork
- [ ] DA_TradeItem_Jewelry
- [ ] DA_TradeItem_ConstructionMaterials
- [ ] DA_TradeItem_SteelAlloy
- [ ] DA_TradeItem_SyntheticProtein

### Markets (10 Target)
- [ ] DA_Market_Agricultural (OpenMarket)
- [ ] DA_Market_Industrial (IndustrialDepot)
- [ ] DA_Market_Mining (OpenMarket)
- [ ] DA_Market_TechHub (ResearchHub)
- [ ] DA_Market_Luxury (LuxuryBazaar)
- [ ] DA_Market_Research (ResearchHub)
- [ ] DA_Market_Military (IndustrialDepot)
- [ ] DA_Market_Medical (OpenMarket)
- [ ] DA_Market_Commodity (OpenMarket)
- [ ] DA_Market_Freeport (OpenMarket)

### Ships (3 Target)
- [ ] DA_Ship_Starter
- [ ] DA_Ship_Freighter
- [ ] DA_Ship_Corvette

### Input (1 Target)
- [ ] DA_InputConfig

### Asset Manager
- [ ] All 4 types registered in Project Settings
- [ ] Scan paths configured
- [ ] Verified loading in PIE

---

## 🧪 Validation Script

```bash
python test_trading_system.py
# Validates all TradeItem and Market DataAssets
# Outputs: trading_system_report.json
```

---

*Back to [System Reference](../06-SYSTEM_REFERENCE.md) | [INDEX.md](../INDEX.md)*