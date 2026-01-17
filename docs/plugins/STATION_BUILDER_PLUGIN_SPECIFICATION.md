# Station Builder Plugin - Development Specification

**Document Version**: 1.0  
**Created**: 2026-01-17  
**Target**: Agent-driven development in separate repository  
**Project**: Modular Station Designer Plugin for Adastrea

---

## Executive Summary

This document provides complete specifications for an agent to develop a **visual station builder plugin** for Unreal Engine that enables drag-and-drop modular station construction with connection points, snapping, and direct export to Adastrea's editor.

**Plugin Purpose**: 
- Visual tool for designing space stations by dragging modules into connection points
- Automatic snapping and validation of module connections
- Export finished stations directly to Adastrea project as Blueprint actors
- Streamline station creation workflow for designers

**Target Audience**: 
- AI development agent (autonomous implementation)
- Level designers building space stations
- Content creators designing station layouts

---

## Quick Reference

### Key Features
- **Drag-and-Drop**: Intuitive module placement from palette
- **Connection Points**: Automatic snapping with validation
- **3D Preview**: Real-time visualization in workspace
- **Export**: Direct Blueprint generation for Adastrea
- **Validation**: Check for required modules and connectivity

### Implementation Timeline
- **Phase 1** (Weeks 1-4): Core functionality
- **Phase 2** (Weeks 5-8): Export & polish
- **Phase 3** (Weeks 9-12): Advanced features

### Success Metrics
- 5-10x faster than manual module placement
- 60 FPS performance with 50+ modules
- < 5 second export time

---

## Table of Contents

### Part 1: Architecture & Design
1. [Plugin Architecture](#plugin-architecture)
2. [Core Features](#core-features)
3. [User Workflow](#user-workflow)
4. [Technical Requirements](#technical-requirements)

### Part 2: Integration with Adastrea
5. [Adastrea Module System](#adastrea-module-system)
6. [Connection Point System](#connection-point-system)
7. [Export Format](#export-format)
8. [Data Compatibility](#data-compatibility)

### Part 3: Implementation Guide
9. [C++ Classes to Create](#c-classes-to-create)
10. [Editor UI Components](#editor-ui-components)
11. [Build System](#build-system)
12. [Testing Strategy](#testing-strategy)

### Part 4: Reference
13. [Module Types Reference](#module-types-reference)
14. [Connection Rules](#connection-rules)
15. [Validation Rules](#validation-rules)
16. [Export Examples](#export-examples)

---

## Part 1: Architecture & Design

### Plugin Architecture

#### High-Level Architecture

```
Station Builder Plugin
├── Editor Module (UE5 Editor Tools)
│   ├── Station Designer Window (Slate UI)
│   ├── Module Palette (Drag-and-drop library)
│   ├── Connection Point System
│   ├── Validation Engine
│   └── Export System
│
├── Runtime Module (Game-time support)
│   ├── Connection Point Components
│   ├── Module Attachment Logic
│   └── Station Assembly System
│
└── Content (Assets)
    ├── Module Thumbnails
    ├── Connection Point Visuals
    └── UI Icons
```

#### Plugin File Structure

```
ModularStationDesigner/
├── Source/
│   ├── ModularStationDesigner/           # Runtime module
│   │   ├── Public/
│   │   │   ├── StationDesignerTypes.h
│   │   │   ├── ConnectionPoint.h
│   │   │   └── ModuleAttachment.h
│   │   └── Private/
│   │       ├── ConnectionPoint.cpp
│   │       └── ModuleAttachment.cpp
│   │
│   └── ModularStationDesignerEditor/     # Editor module
│       ├── Public/
│       │   ├── StationDesignerWindow.h
│       │   ├── ModulePalette.h
│       │   ├── StationValidator.h
│       │   └── StationExporter.h
│       └── Private/
│           ├── StationDesignerWindow.cpp
│           ├── ModulePalette.cpp
│           ├── StationValidator.cpp
│           └── StationExporter.cpp
│
├── Content/
│   ├── Icons/                            # UI icons
│   ├── ModuleThumbnails/                 # Preview images
│   └── EditorAssets/                     # Editor-only assets
│
├── Config/
│   └── FilterPlugin.ini
│
└── ModularStationDesigner.uplugin        # Plugin descriptor
```

---

### Core Features

#### Feature List (Priority Order)

**Phase 1: MVP (Essential Features)**

1. **Module Library**
   - Display all available Adastrea station modules
   - Thumbnail previews with module info
   - Filter by module group (Docking, Storage, Public, etc.)
   - Drag-and-drop to workspace

2. **3D Workspace**
   - Visual 3D station preview
   - Camera controls (orbit, pan, zoom)
   - Grid overlay for alignment
   - Module placement with transform gizmos

3. **Connection Point System**
   - Visual connection point indicators
   - Automatic snapping when modules are nearby
   - Valid/invalid connection feedback (green/red)
   - Connection compatibility validation

4. **Export Functionality**
   - Export as Adastrea-compatible Blueprint
   - Child Actor Component generation
   - Preserve module positions and rotations
   - Save/load station designs

**Phase 2: Enhanced Features**

5. **Validation System**
   - Check for required modules (must have docking bay)
   - Verify all modules are connected
   - Warn about power balance issues
   - Suggest missing functionality

6. **Templates & Presets**
   - Save station templates
   - Load from template library
   - Quick-start templates (trade hub, outpost, etc.)

7. **Advanced Tools**
   - Copy/paste modules
   - Mirror/rotate entire sections
   - Symmetry mode for balanced designs
   - Auto-arrange helpers

**Phase 3: Polish & Quality of Life**

8. **UI Enhancements**
   - Property editor for selected modules
   - Module search/filter
   - Recently used modules
   - Favorites system

9. **Visualization**
   - Power flow visualization
   - Traffic flow simulation
   - Module group color coding
   - Connection wire display

---

### User Workflow

#### Typical Design Session

```
1. Open Station Builder
   ↓
2. Select "New Station" or load template
   ↓
3. Choose station core/base module
   ↓
4. Drag modules from palette to workspace
   ↓
5. Connect modules via connection points
   - Modules snap automatically when close
   - Green highlight = valid connection
   - Red highlight = invalid connection
   ↓
6. Position and rotate modules as desired
   ↓
7. Validate design (check for issues)
   ↓
8. Preview in 3D (optional)
   ↓
9. Export to Adastrea project
   - Plugin generates BP_StationDesign_Custom
   - Includes all modules as Child Actor Components
   - Preserves all transforms and connections
   ↓
10. Open in Adastrea editor
    - Station ready to use immediately
    - All modules visible in Components panel
    - Drag into level and press Play
```

#### Example Workflow Timeline

**Beginner Designer (First Station):**
- Learn tool: 5-10 minutes
- Design station: 10-15 minutes
- Export and test: 5 minutes
- **Total: 20-30 minutes**

**Experienced Designer:**
- Load template: 1 minute
- Customize layout: 5-10 minutes
- Export: 2 minutes
- **Total: 8-13 minutes**

**Goal**: Enable designers to create functional stations 5-10x faster than manual placement.

---

### Technical Requirements

#### Unreal Engine Version

- **Target**: Unreal Engine 5.6 (Adastrea's version)
- **Minimum**: Unreal Engine 5.4
- **Architecture**: C++ plugin with Slate UI

#### Dependencies

**Core Unreal Modules:**
- `Core` - Basic engine functionality
- `CoreUObject` - Object system
- `Engine` - Game engine
- `UnrealEd` - Editor functionality
- `Slate` - UI framework
- `SlateCore` - Core UI primitives
- `EditorStyle` - Editor styling
- `LevelEditor` - Level editing integration
- `PropertyEditor` - Property editing
- `AssetTools` - Asset management

**Optional Modules:**
- `InputCore` - Input handling
- `EditorWidgets` - Advanced editor widgets

#### Platform Requirements

- **Windows**: Primary development platform
- **Mac**: Secondary support (optional)
- **Linux**: Optional support

#### Performance Targets

- **Load Time**: < 2 seconds to open plugin window
- **Responsiveness**: 60 FPS in 3D workspace with 50+ modules
- **Export Time**: < 5 seconds for typical station (20-30 modules)
- **Memory**: < 500 MB additional RAM usage

---

## Part 2: Integration with Adastrea

### Adastrea Module System

#### Module Base Class

All Adastrea station modules inherit from `ASpaceStationModule`:

```cpp
// From Adastrea: Source/Adastrea/Public/Stations/SpaceStationModule.h
class ADASTREA_API ASpaceStationModule : public AActor, public IDamageable, public ITargetable
{
    GENERATED_BODY()

public:
    // Type identifier (e.g., "Docking Bay", "Reactor Core")
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module")
    FString ModuleType;

    // Power consumption/generation (positive = consume, negative = generate)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module")
    float ModulePower;

    // Functional group (Docking, Storage, Public, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module")
    EStationModuleGroup ModuleGroup;
    
    // ... additional functionality
};
```

#### Module Groups (EStationModuleGroup)

```cpp
enum class EStationModuleGroup : uint8
{
    All,          // All modules (no filtering)
    Docking,      // Ship docking bays (REQUIRED for trading)
    Power,        // Reactors and generators
    Storage,      // Cargo and material storage
    Processing,   // Manufacturing and refinement
    Defence,      // Weapons and shields
    Habitation,   // Living quarters
    Public,       // Markets and public spaces (marketplace for trading)
    Connection,   // Corridors and connectors
    Other         // Miscellaneous
};
```

**MVP Required Groups:**
- `Docking` - At least one required (enables trading)
- `Public` - Marketplace module (trading interface)
- `Storage` - Optional but recommended (cargo management)

#### Available Module Types

**Adastrea provides these module Blueprints:**

| Module Blueprint | Type | Group | Power | Description |
|------------------|------|-------|-------|-------------|
| `BP_SpaceStationModule_DockingBay` | Large Bay | Docking | +50 | 4-6 docking points |
| `BP_SpaceStationModule_DockingPort` | Small Port | Docking | +10 | 1 docking point |
| `BP_SpaceStationModule_Marketplace` | Market | Public | +40 | Trading interface |
| `BP_CargoBayModule` | Storage | Storage | +20 | Cargo storage |
| `BP_SpaceStationModule_Habitation` | Living | Habitation | +30 | Crew quarters |
| `BP_SpaceStationModule_Corridor` | Connector | Connection | +5 | Passage between modules |
| `BP_ReactorModule` | Generator | Power | -500 | Power generation |
| `BP_SolarArrayModule` | Generator | Power | -200 | Solar power |
| `BP_FabricationModule` | Factory | Processing | +80 | Manufacturing |
| `BP_TurretModule` | Defense | Defence | +60 | Weapons |
| `BP_ShieldGeneratorModule` | Defense | Defence | +100 | Shields |

**Plugin should scan Adastrea project and auto-discover all module types.**

---

### Connection Point System

#### Connection Point Concept

Connection points are standardized attachment locations on modules where other modules can connect.

**Properties:**
- **Position** - 3D location relative to module origin
- **Rotation** - Facing direction of connection
- **Type** - Connection socket type (standard, large, small)
- **Occupied** - Whether another module is connected

#### Connection Point Implementation

```cpp
// Plugin Runtime Class
UCLASS()
class UConnectionPointComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    // Connection type (for compatibility checking)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Connection")
    EConnectionType ConnectionType;
    
    // Size class (modules of compatible size can connect)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Connection")
    EConnectionSize ConnectionSize;
    
    // Whether this point is currently occupied
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Connection")
    bool bIsOccupied;
    
    // The module currently connected (if any)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Connection")
    AActor* ConnectedModule;
    
    // Visual indicators
    UPROPERTY(EditAnywhere, Category="Visuals")
    UStaticMeshComponent* SocketMesh;
    
    // Connection validation
    UFUNCTION(BlueprintCallable, Category="Connection")
    bool CanConnectTo(UConnectionPointComponent* OtherPoint) const;
    
    UFUNCTION(BlueprintCallable, Category="Connection")
    bool ConnectTo(UConnectionPointComponent* OtherPoint);
    
    UFUNCTION(BlueprintCallable, Category="Connection")
    void Disconnect();
};
```

#### Connection Types

```cpp
UENUM(BlueprintType)
enum class EConnectionType : uint8
{
    Standard,      // General purpose connection
    Docking,       // Docking bay specific
    Corridor,      // Corridor/passage connection
    Power,         // Power transfer connection
    Data           // Data/control connection
};

UENUM(BlueprintType)
enum class EConnectionSize : uint8
{
    Small,         // 1-2 meter diameter
    Medium,        // 3-5 meter diameter
    Large,         // 6+ meter diameter
    Universal      // Connects to any size
};
```

#### Connection Rules

**Compatibility Matrix:**

| Type A | Type B | Compatible? | Notes |
|--------|--------|-------------|-------|
| Standard | Standard | ✅ Yes | Basic connection |
| Standard | Corridor | ✅ Yes | Corridor accepts all |
| Corridor | Corridor | ✅ Yes | Corridor to corridor |
| Docking | Standard | ❌ No | Docking needs open space |
| Docking | Docking | ❌ No | Can't dock to dock |
| Power | Power | ✅ Yes | Power transfer |
| Universal | Any | ✅ Yes | Universal adapter |

**Size Compatibility:**
- Small ↔ Small: ✅ Yes
- Medium ↔ Medium: ✅ Yes
- Large ↔ Large: ✅ Yes
- Small ↔ Medium: ⚠️ Warning (works but suboptimal)
- Small ↔ Large: ❌ No
- Universal ↔ Any: ✅ Yes

---

### Export Format

#### Blueprint Generation

The plugin exports stations as Unreal Blueprint assets that Adastrea can directly use.

**Generated Blueprint Structure:**

```
BP_StationDesign_CustomName (Blueprint Class)
├── Class: ASpaceStation (Adastrea's station class)
├── Components:
│   ├── DefaultSceneRoot
│   ├── StaticMesh_StationCore (optional visual core)
│   ├── DockingBay_North (Child Actor Component)
│   │   └─ Child Actor Class: BP_SpaceStationModule_DockingBay
│   │   └─ Transform: (X=500, Y=0, Z=0, Rotation...)
│   ├── Marketplace_Central (Child Actor Component)
│   │   └─ Child Actor Class: BP_SpaceStationModule_Marketplace
│   │   └─ Transform: (X=0, Y=0, Z=0, Rotation...)
│   ├── CargoStorage_1 (Child Actor Component)
│   │   └─ Child Actor Class: BP_CargoBayModule
│   │   └─ Transform: (X=-500, Y=200, Z=0, Rotation...)
│   └── CargoStorage_2 (Child Actor Component)
│       └─ Child Actor Class: BP_CargoBayModule
│       └─ Transform: (X=-500, Y=-200, Z=0, Rotation...)
│
└── Class Defaults:
    ├── StationName: "Custom Trading Hub"
    ├── MaxStructuralIntegrity: 10000
    └── DefaultModuleClasses: (empty - using Child Actor Components)
```

#### Export Process

```cpp
// Plugin Editor Class
class FStationExporter
{
public:
    // Main export function
    static bool ExportStation(
        const FStationDesign& Design,
        const FString& TargetPath,
        const FString& BlueprintName
    );

private:
    // Create Blueprint asset
    static UBlueprint* CreateStationBlueprint(
        const FString& AssetPath,
        const FString& AssetName
    );
    
    // Add Child Actor Components for each module
    static void AddModuleComponents(
        UBlueprint* Blueprint,
        const TArray<FModulePlacement>& Modules
    );
    
    // Set component transforms
    static void SetComponentTransforms(
        UActorComponent* Component,
        const FTransform& Transform
    );
    
    // Validate exported Blueprint
    static bool ValidateExport(UBlueprint* Blueprint);
};
```

#### Export File Format

**Blueprint Asset:**
- Path: `Content/Blueprints/Stations/Generated/BP_StationDesign_[Name].uasset`
- Parent Class: `ASpaceStation`
- Format: Unreal Engine Blueprint (binary `.uasset`)

**Design File (Optional Save/Load):**
- Path: `Saved/StationDesigns/[Name].stationdesign`
- Format: JSON for human readability
- Purpose: Re-edit stations in plugin later

**Design JSON Schema:**
```json
{
  "version": "1.0",
  "stationName": "Custom Trading Hub",
  "modules": [
    {
      "id": "module_001",
      "blueprintPath": "/Game/Blueprints/Stations/Modules/BP_SpaceStationModule_DockingBay",
      "position": {"x": 500.0, "y": 0.0, "z": 0.0},
      "rotation": {"pitch": 0.0, "yaw": 90.0, "roll": 0.0},
      "scale": {"x": 1.0, "y": 1.0, "z": 1.0},
      "componentName": "DockingBay_North"
    },
    {
      "id": "module_002",
      "blueprintPath": "/Game/Blueprints/Stations/Modules/BP_SpaceStationModule_Marketplace",
      "position": {"x": 0.0, "y": 0.0, "z": 0.0},
      "rotation": {"pitch": 0.0, "yaw": 0.0, "roll": 0.0},
      "scale": {"x": 1.0, "y": 1.0, "z": 1.0},
      "componentName": "Marketplace_Central"
    }
  ],
  "connections": [
    {
      "moduleA": "module_001",
      "pointA": "connection_01",
      "moduleB": "module_002",
      "pointB": "connection_01"
    }
  ]
}
```

---

### Data Compatibility

#### Reading Adastrea Module Data

The plugin needs to read Adastrea's module Blueprints to populate the palette.

**Discovery Process:**
1. Scan Adastrea project for all Blueprint assets
2. Filter Blueprints that inherit from `ASpaceStationModule`
3. Load module metadata (name, type, group, power, thumbnail)
4. Populate module palette in plugin UI

**Code Example:**
```cpp
void FModulePalette::DiscoverAdastreaModules()
{
    // Get asset registry
    FAssetRegistryModule& AssetRegistryModule = 
        FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
    
    // Find all Blueprint assets
    TArray<FAssetData> BlueprintAssets;
    AssetRegistry.GetAssetsByClass(UBlueprint::StaticClass()->GetFName(), BlueprintAssets);
    
    // Filter for SpaceStationModule descendants
    for (const FAssetData& AssetData : BlueprintAssets)
    {
        UBlueprint* Blueprint = Cast<UBlueprint>(AssetData.GetAsset());
        if (Blueprint && Blueprint->GeneratedClass)
        {
            if (Blueprint->GeneratedClass->IsChildOf(ASpaceStationModule::StaticClass()))
            {
                // Found a module! Add to palette
                AddModuleToPalette(Blueprint);
            }
        }
    }
}
```

#### Maintaining Compatibility

**Version Compatibility:**
- Plugin should check Adastrea project version
- Warn if versions don't match
- Gracefully handle missing module types

**Forward Compatibility:**
- Save extra module data in JSON (for future features)
- Ignore unknown module properties when loading
- Support older design files

---

## Part 3: Implementation Guide

### C++ Classes to Create

#### Runtime Module Classes

**1. UConnectionPointComponent** (Connection point visualization)

```cpp
// Header: ModularStationDesigner/Public/ConnectionPoint.h
#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ConnectionPoint.generated.h"

UENUM(BlueprintType)
enum class EConnectionType : uint8
{
    Standard     UMETA(DisplayName="Standard"),
    Docking      UMETA(DisplayName="Docking"),
    Corridor     UMETA(DisplayName="Corridor"),
    Power        UMETA(DisplayName="Power"),
    Universal    UMETA(DisplayName="Universal")
};

UENUM(BlueprintType)
enum class EConnectionSize : uint8
{
    Small        UMETA(DisplayName="Small"),
    Medium       UMETA(DisplayName="Medium"),
    Large        UMETA(DisplayName="Large"),
    Universal    UMETA(DisplayName="Universal")
};

UCLASS(ClassGroup=(StationBuilder), meta=(BlueprintSpawnableComponent))
class MODULARSTATIONDESIGNER_API UConnectionPointComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UConnectionPointComponent();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Connection")
    EConnectionType ConnectionType;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Connection")
    EConnectionSize ConnectionSize;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Connection")
    bool bIsOccupied;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Connection")
    AActor* ConnectedModule;
    
    UFUNCTION(BlueprintCallable, Category="Connection")
    bool CanConnectTo(UConnectionPointComponent* OtherPoint) const;
    
    UFUNCTION(BlueprintCallable, Category="Connection")
    bool ConnectTo(UConnectionPointComponent* OtherPoint);
    
    UFUNCTION(BlueprintCallable, Category="Connection")
    void Disconnect();

protected:
    virtual void BeginPlay() override;
    
    UPROPERTY(EditAnywhere, Category="Visuals")
    UStaticMeshComponent* SocketMesh;
};
```

**2. FModulePlacement** (Module placement data)

```cpp
// Header: ModularStationDesigner/Public/StationDesignerTypes.h
#pragma once

#include "CoreMinimal.h"
#include "StationDesignerTypes.generated.h"

USTRUCT(BlueprintType)
struct FModulePlacement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ModuleID;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FSoftClassPath ModuleBlueprintPath;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform Transform;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ComponentName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> ConnectedModuleIDs;
};

USTRUCT(BlueprintType)
struct FStationDesign
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString StationName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FModulePlacement> Modules;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DesignVersion;
};
```

#### Editor Module Classes

**3. SStationDesignerWindow** (Main plugin window - Slate UI)

```cpp
// Header: ModularStationDesignerEditor/Public/StationDesignerWindow.h
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SStationDesignerWindow : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SStationDesignerWindow) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    // UI sections
    TSharedPtr<class SModulePalette> ModulePalette;
    TSharedPtr<class SStationViewport> Viewport;
    TSharedPtr<class SPropertiesPanel> Properties;
    
    // Layout structure
    TSharedRef<SWidget> CreateToolbar();
    TSharedRef<SWidget> CreateMainContent();
    
    // Button handlers
    FReply OnNewStation();
    FReply OnLoadStation();
    FReply OnSaveStation();
    FReply OnExportStation();
    FReply OnValidateStation();
    
    // Current design
    FStationDesign CurrentDesign;
};
```

**4. SModulePalette** (Module library panel)

```cpp
// Header: ModularStationDesignerEditor/Public/ModulePalette.h
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

struct FModuleInfo
{
    FString Name;
    FString BlueprintPath;
    FString ModuleType;
    FString ModuleGroup;
    float PowerConsumption;
    TSharedPtr<FSlateBrush> Thumbnail;
};

class SModulePalette : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SModulePalette) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    
    // Discover modules from Adastrea project
    void DiscoverModules();
    
    // Filter modules
    void SetGroupFilter(EStationModuleGroup Group);
    void SetSearchFilter(const FString& SearchText);

private:
    TArray<FModuleInfo> AllModules;
    TArray<FModuleInfo> FilteredModules;
    
    TSharedRef<ITableRow> OnGenerateModuleRow(
        TSharedPtr<FModuleInfo> Item,
        const TSharedRef<STableViewBase>& OwnerTable
    );
    
    void OnModuleSelected(TSharedPtr<FModuleInfo> SelectedModule);
    void OnModuleDragStart(TSharedPtr<FModuleInfo> DraggedModule);
};
```

**5. FStationValidator** (Validation logic)

```cpp
// Header: ModularStationDesignerEditor/Public/StationValidator.h
#pragma once

#include "CoreMinimal.h"

enum class EValidationSeverity : uint8
{
    Info,
    Warning,
    Error
};

struct FValidationMessage
{
    EValidationSeverity Severity;
    FString Message;
    FString ModuleID;  // Optional: specific module with issue
};

class FStationValidator
{
public:
    // Validate entire station design
    static TArray<FValidationMessage> ValidateStation(const FStationDesign& Design);
    
private:
    // Individual validation checks
    static void CheckRequiredModules(const FStationDesign& Design, TArray<FValidationMessage>& OutMessages);
    static void CheckConnectivity(const FStationDesign& Design, TArray<FValidationMessage>& OutMessages);
    static void CheckPowerBalance(const FStationDesign& Design, TArray<FValidationMessage>& OutMessages);
    static void CheckModuleCompatibility(const FStationDesign& Design, TArray<FValidationMessage>& OutMessages);
};
```

**6. FStationExporter** (Export to Blueprint)

```cpp
// Header: ModularStationDesignerEditor/Public/StationExporter.h
#pragma once

#include "CoreMinimal.h"

class FStationExporter
{
public:
    // Export station design to Adastrea Blueprint
    static bool ExportToBlueprintAsset(
        const FStationDesign& Design,
        const FString& TargetPackagePath,
        const FString& AssetName,
        FString& OutErrorMessage
    );
    
    // Save design to JSON file
    static bool SaveDesignToFile(
        const FStationDesign& Design,
        const FString& FilePath
    );
    
    // Load design from JSON file
    static bool LoadDesignFromFile(
        const FString& FilePath,
        FStationDesign& OutDesign
    );

private:
    static UBlueprint* CreateBlueprintAsset(const FString& PackagePath, const FString& AssetName);
    static void AddChildActorComponents(UBlueprint* Blueprint, const FStationDesign& Design);
    static void ConfigureStationDefaults(UBlueprint* Blueprint, const FStationDesign& Design);
};
```

---

### Editor UI Components

#### Main Window Layout

```
┌─────────────────────────────────────────────────────────────────┐
│  Station Builder                                        [-][□][X]│
├─────────────────────────────────────────────────────────────────┤
│  [New] [Load] [Save] [Export] [Validate]          [?] Help      │
├──────────────┬──────────────────────────────────┬───────────────┤
│              │                                  │               │
│  Module      │                                  │  Properties   │
│  Palette     │         3D Viewport              │               │
│              │                                  │  Module: (none)│
│ [All]        │                                  │               │
│  Docking     │                                  │  Transform:   │
│  Storage     │                                  │   X: 0.0      │
│  Public      │        [Camera Controls]         │   Y: 0.0      │
│  Power       │                                  │   Z: 0.0      │
│  Habitation  │                                  │               │
│  Connection  │                                  │  Rotation:    │
│              │                                  │   Pitch: 0.0  │
│ [Search...]  │                                  │   Yaw: 0.0    │
│              │                                  │   Roll: 0.0   │
│ ┌──────────┐ │                                  │               │
│ │DockingBay│ │                                  │  [Delete]     │
│ │ +50 pow  │ │                                  │  [Duplicate]  │
│ └──────────┘ │                                  │               │
│ ┌──────────┐ │                                  │               │
│ │Marketplace│ │                                  │               │
│ │ +40 pow  │ │                                  │               │
│ └──────────┘ │                                  │               │
│              │                                  │               │
├──────────────┴──────────────────────────────────┴───────────────┤
│  Status: Ready    Modules: 5    Power Balance: +50 MW      [>] │
└─────────────────────────────────────────────────────────────────┘
```

#### Module Palette Design

**Visual Structure:**
- Grid layout of module tiles
- Each tile shows:
  - Module thumbnail (top-down view)
  - Module name
  - Power consumption/generation
  - Module group icon
- Drag-and-drop enabled
- Filter buttons at top
- Search bar

**Example Module Tile:**
```
┌─────────────┐
│  [Image]    │  ← Thumbnail
│  --------   │
│ Docking Bay │  ← Name
│   +50 MW    │  ← Power (red = consume, green = generate)
│ [Docking]   │  ← Group badge
└─────────────┘
```

#### 3D Viewport Features

**Camera Controls:**
- **Mouse Right-Click + Drag**: Orbit camera
- **Mouse Middle-Click + Drag**: Pan camera
- **Mouse Wheel**: Zoom in/out
- **F Key**: Frame selected module

**Module Manipulation:**
- **Click**: Select module
- **Click + Drag**: Move module
- **Ctrl + Click + Drag**: Duplicate and move
- **R Key**: Rotate mode
- **Delete Key**: Delete selected module

**Visual Feedback:**
- Selected module: Yellow outline
- Connection points: Sphere gizmos (green when valid, red when invalid)
- Snapping: Modules snap to nearby connection points
- Grid: Optional grid overlay for alignment

---

### Build System

#### Plugin Descriptor (ModularStationDesigner.uplugin)

```json
{
  "FileVersion": 3,
  "Version": 1,
  "VersionName": "1.0",
  "FriendlyName": "Modular Station Designer",
  "Description": "Visual station builder plugin for Adastrea - drag and drop modules with connection points",
  "Category": "Level Design",
  "CreatedBy": "Adastrea Development Team",
  "CreatedByURL": "https://github.com/Mittenzx/Adastrea",
  "DocsURL": "",
  "MarketplaceURL": "",
  "SupportURL": "",
  "EngineVersion": "5.6.0",
  "CanContainContent": true,
  "Installed": false,
  "Modules": [
    {
      "Name": "ModularStationDesigner",
      "Type": "Runtime",
      "LoadingPhase": "Default",
      "WhitelistPlatforms": [
        "Win64",
        "Mac",
        "Linux"
      ]
    },
    {
      "Name": "ModularStationDesignerEditor",
      "Type": "Editor",
      "LoadingPhase": "Default",
      "WhitelistPlatforms": [
        "Win64",
        "Mac",
        "Linux"
      ]
    }
  ],
  "Plugins": []
}
```

#### Build Configuration (ModularStationDesigner.Build.cs)

```csharp
// Source/ModularStationDesigner/ModularStationDesigner.Build.cs
using UnrealBuildTool;

public class ModularStationDesigner : ModuleRules
{
    public ModularStationDesigner(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            // Add additional dependencies here
        });
    }
}
```

#### Editor Build Configuration

```csharp
// Source/ModularStationDesignerEditor/ModularStationDesignerEditor.Build.cs
using UnrealBuildTool;

public class ModularStationDesignerEditor : ModuleRules
{
    public ModularStationDesignerEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "ModularStationDesigner"  // Runtime module
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore",
            "UnrealEd",
            "LevelEditor",
            "PropertyEditor",
            "AssetTools",
            "EditorStyle",
            "EditorWidgets",
            "InputCore"
        });
    }
}
```

---

### Testing Strategy

#### Unit Tests

**Test Connection Logic:**
```cpp
// Test connection compatibility
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConnectionCompatibilityTest, 
    "StationBuilder.Connection.Compatibility", 
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FConnectionCompatibilityTest::RunTest(const FString& Parameters)
{
    // Test Standard + Standard = Valid
    UConnectionPointComponent* PointA = NewObject<UConnectionPointComponent>();
    PointA->ConnectionType = EConnectionType::Standard;
    
    UConnectionPointComponent* PointB = NewObject<UConnectionPointComponent>();
    PointB->ConnectionType = EConnectionType::Standard;
    
    TestTrue("Standard-Standard connection should be valid", 
        PointA->CanConnectTo(PointB));
    
    // Test Docking + Docking = Invalid
    PointA->ConnectionType = EConnectionType::Docking;
    PointB->ConnectionType = EConnectionType::Docking;
    
    TestFalse("Docking-Docking connection should be invalid", 
        PointA->CanConnectTo(PointB));
    
    return true;
}
```

**Test Validation Logic:**
```cpp
// Test station validation
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStationValidationTest, 
    "StationBuilder.Validation.RequiredModules", 
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStationValidationTest::RunTest(const FString& Parameters)
{
    FStationDesign Design;
    Design.StationName = "Test Station";
    
    // Station with no modules should fail validation
    TArray<FValidationMessage> Messages = FStationValidator::ValidateStation(Design);
    
    TestTrue("Empty station should have validation errors", 
        Messages.Num() > 0);
    
    TestTrue("Should warn about missing docking bay",
        Messages.ContainsByPredicate([](const FValidationMessage& Msg) {
            return Msg.Message.Contains("docking");
        }));
    
    return true;
}
```

#### Integration Tests

**Test Export Process:**
- Create simple station design (2-3 modules)
- Export to Blueprint
- Load Blueprint in Adastrea editor
- Verify all modules present
- Verify transforms correct
- Test in-game functionality

**Manual Test Checklist:**
- [ ] Open plugin window
- [ ] Module palette loads all Adastrea modules
- [ ] Drag module to viewport
- [ ] Module appears in 3D view
- [ ] Select and move module
- [ ] Connect two modules via connection points
- [ ] Export station
- [ ] Open exported Blueprint in Adastrea
- [ ] Drag station into level
- [ ] Press Play - station works correctly

---

## Part 4: Reference

### Module Types Reference

#### Complete Module List (from Adastrea)

| Module | Blueprint Path | Group | Power | Docking Points | Special Features |
|--------|---------------|-------|-------|----------------|------------------|
| Docking Bay (Large) | `BP_SpaceStationModule_DockingBay` | Docking | +50 | 4-6 | Large ship docking |
| Docking Port (Small) | `BP_SpaceStationModule_DockingPort` | Docking | +10 | 1 | Small ship docking |
| Marketplace | `BP_SpaceStationModule_Marketplace` | Public | +40 | 0 | Trading UI access |
| Cargo Bay | `BP_CargoBayModule` | Storage | +20 | 0 | Inventory storage |
| Habitation | `BP_SpaceStationModule_Habitation` | Habitation | +30 | 0 | Crew quarters |
| Corridor | `BP_SpaceStationModule_Corridor` | Connection | +5 | 0 | Module passage |
| Barracks | `BP_BarracksModule` | Habitation | +40 | 0 | Military housing |
| Reactor | `BP_ReactorModule` | Power | -500 | 0 | Power generation |
| Solar Array | `BP_SolarArrayModule` | Power | -200 | 0 | Solar power |
| Fuel Depot | `BP_FuelDepotModule` | Storage | +15 | 0 | Fuel storage |
| Fabrication | `BP_FabricationModule` | Processing | +80 | 0 | Manufacturing |
| Processing | `BP_ProcessingModule` | Processing | +60 | 0 | Resource refining |
| Science Lab | `BP_ScienceLabModule` | Other | +50 | 0 | Research |
| Turret | `BP_TurretModule` | Defence | +60 | 0 | Weapons |
| Shield Generator | `BP_ShieldGeneratorModule` | Defence | +100 | 0 | Defensive shields |

---

### Connection Rules

#### Compatibility Matrix (Full)

| Connection A | Connection B | Compatible | Notes |
|--------------|--------------|------------|-------|
| Standard | Standard | ✅ Yes | General purpose |
| Standard | Corridor | ✅ Yes | Corridor accepts all standard |
| Standard | Docking | ❌ No | Docking needs open space |
| Standard | Power | ⚠️ Maybe | Only if power transfer needed |
| Standard | Universal | ✅ Yes | Universal accepts all |
| Corridor | Corridor | ✅ Yes | Corridor connections |
| Corridor | Docking | ⚠️ Maybe | Only if corridor leads to docking |
| Corridor | Power | ✅ Yes | Power can run through corridor |
| Corridor | Universal | ✅ Yes | Universal accepts all |
| Docking | Docking | ❌ No | Docking bays can't connect |
| Docking | Power | ❌ No | Incompatible |
| Docking | Universal | ⚠️ Maybe | Adapter required |
| Power | Power | ✅ Yes | Power transfer |
| Power | Universal | ✅ Yes | Universal accepts all |
| Universal | Universal | ✅ Yes | Universal adapter |

#### Size Compatibility

| Size A | Size B | Compatible | Connection Quality |
|--------|--------|------------|-------------------|
| Small | Small | ✅ Yes | Perfect fit |
| Small | Medium | ⚠️ Warning | Works but loose |
| Small | Large | ❌ No | Too different |
| Small | Universal | ✅ Yes | Adapter |
| Medium | Medium | ✅ Yes | Perfect fit |
| Medium | Large | ⚠️ Warning | Works but tight |
| Medium | Universal | ✅ Yes | Adapter |
| Large | Large | ✅ Yes | Perfect fit |
| Large | Universal | ✅ Yes | Adapter |
| Universal | Universal | ✅ Yes | Any size |

---

### Validation Rules

#### Required Module Checks

**Critical (ERROR - Cannot Export):**
1. ✅ **At least one Docking module** required
   - Station must have `DockingBay` or `DockingPort`
   - Error: "Station must have at least one docking facility"

2. ✅ **All modules connected** (no orphans)
   - Every module must connect to the network
   - Error: "Disconnected modules detected: [Module Names]"

**Recommended (WARNING - Can Export):**
3. ⚠️ **Marketplace module** recommended for trading
   - Warning: "No marketplace found - station cannot trade"

4. ⚠️ **Power balance positive or neutral**
   - Total power consumption ≤ total power generation
   - Warning: "Power deficit: -150 MW (add reactors or solar arrays)"

5. ⚠️ **Storage module** recommended
   - Warning: "No cargo storage - station has limited inventory"

6. ⚠️ **Habitation module** recommended
   - Warning: "No habitation - station cannot house crew"

**Suggestions (INFO - Nice to Have):**
7. ℹ️ **Multiple docking bays** for large stations
   - Info: "Consider adding more docking bays for busy trade hubs"

8. ℹ️ **Corridor modules** for aesthetics
   - Info: "Add corridors for more realistic station layout"

#### Structural Checks

**Connection Validation:**
- All connection points properly paired
- No overlapping modules
- No floating modules (must be grounded to core)
- Connection types compatible

**Layout Validation:**
- Modules not intersecting
- Reasonable spacing between modules
- Symmetric layout (optional suggestion)
- Station within size limits (if defined)

---

### Export Examples

#### Example 1: Simple Trade Outpost

**Design:**
- 1 Docking Port
- 1 Marketplace
- 1 Cargo Bay

**Generated Blueprint:**
```cpp
// BP_Station_TradeOutpost
Components:
  - DockingPort_Main (Child Actor: BP_SpaceStationModule_DockingPort)
  - Marketplace_Central (Child Actor: BP_SpaceStationModule_Marketplace)
  - CargoStorage (Child Actor: BP_CargoBayModule)

Class Defaults:
  StationName: "Trade Outpost"
  MaxStructuralIntegrity: 5000
  Power Balance: +70 MW (consuming +70, generating 0)
```

**Validation:**
- ✅ Has docking: Yes
- ⚠️ Power deficit: -70 MW
- ✅ Has marketplace: Yes
- ✅ All connected: Yes

#### Example 2: Medium Trading Station

**Design:**
- 1 Docking Bay (large)
- 1 Marketplace
- 3 Cargo Bays
- 1 Habitation
- 2 Corridors
- 1 Reactor (power)

**Generated Blueprint:**
```cpp
// BP_Station_TradingHub
Components:
  - DockingBay_North (Child Actor: BP_SpaceStationModule_DockingBay)
  - Marketplace_Central (Child Actor: BP_SpaceStationModule_Marketplace)
  - CargoStorage_1 (Child Actor: BP_CargoBayModule)
  - CargoStorage_2 (Child Actor: BP_CargoBayModule)
  - CargoStorage_3 (Child Actor: BP_CargoBayModule)
  - Habitation_Crew (Child Actor: BP_SpaceStationModule_Habitation)
  - Corridor_East (Child Actor: BP_SpaceStationModule_Corridor)
  - Corridor_West (Child Actor: BP_SpaceStationModule_Corridor)
  - Reactor_Core (Child Actor: BP_ReactorModule)

Class Defaults:
  StationName: "Trading Hub Alpha"
  MaxStructuralIntegrity: 10000
  Power Balance: +320 MW surplus
```

**Validation:**
- ✅ Has docking: Yes (large bay)
- ✅ Power surplus: +320 MW
- ✅ Has marketplace: Yes
- ✅ Has storage: Yes (3 bays)
- ✅ Has habitation: Yes
- ✅ All connected: Yes
- ℹ️ Well-designed station!

---

## Implementation Roadmap

### Phase 1: Core Functionality (Weeks 1-4)

**Week 1: Project Setup**
- [ ] Create plugin structure
- [ ] Set up build system
- [ ] Create basic module classes
- [ ] Test plugin loads in Unreal

**Week 2: Module Discovery**
- [ ] Implement Adastrea module scanning
- [ ] Load module metadata
- [ ] Create module info structures
- [ ] Test with real Adastrea modules

**Week 3: Basic UI**
- [ ] Create main plugin window
- [ ] Implement module palette
- [ ] Add 3D viewport
- [ ] Basic drag-and-drop

**Week 4: Connection System**
- [ ] Implement connection point logic
- [ ] Add snapping behavior
- [ ] Visual connection indicators
- [ ] Test module connections

### Phase 2: Export & Polish (Weeks 5-8)

**Week 5: Export System**
- [ ] Implement Blueprint generation
- [ ] Add Child Actor Components
- [ ] Set transforms correctly
- [ ] Test exported Blueprints

**Week 6: Validation**
- [ ] Implement validation rules
- [ ] Add validation UI feedback
- [ ] Test all validation scenarios

**Week 7: Save/Load**
- [ ] JSON serialization
- [ ] Save design to file
- [ ] Load design from file
- [ ] Version compatibility

**Week 8: Polish & Testing**
- [ ] UI improvements
- [ ] Bug fixes
- [ ] Performance optimization
- [ ] Documentation

### Phase 3: Advanced Features (Weeks 9-12)

**Week 9: Templates**
- [ ] Save station templates
- [ ] Template library
- [ ] Quick-start templates

**Week 10: Advanced Tools**
- [ ] Copy/paste modules
- [ ] Mirror/rotate
- [ ] Symmetry mode

**Week 11: Visualization**
- [ ] Power flow display
- [ ] Module group coloring
- [ ] Connection wires

**Week 12: Final Polish**
- [ ] User testing
- [ ] Bug fixes
- [ ] Documentation
- [ ] Release preparation

---

## Success Criteria

### MVP Success (End of Phase 1)

- [ ] Plugin loads in Unreal Engine 5.6
- [ ] Discovers all Adastrea station modules
- [ ] Module palette displays thumbnails
- [ ] Can drag modules to 3D workspace
- [ ] Modules snap to connection points
- [ ] Basic export to Blueprint works
- [ ] Exported Blueprints work in Adastrea

### Full Success (End of Phase 2)

- [ ] All validation rules implemented
- [ ] Save/load station designs
- [ ] Professional UI polish
- [ ] Comprehensive error handling
- [ ] Documentation complete
- [ ] 5-10x faster than manual placement

### Stretch Goals (Phase 3)

- [ ] Template library with 10+ presets
- [ ] Advanced visualization features
- [ ] Symmetry and mirroring tools
- [ ] Integration with Adastrea editor menus
- [ ] Automated station generation

---

## Documentation Deliverables

### For Users (Designers)

1. **Quick Start Guide** - 2-page getting started
2. **User Manual** - Complete feature documentation
3. **Video Tutorials** - Screen recordings of workflows
4. **Template Gallery** - Pre-built station examples

### For Developers (Maintenance)

1. **API Documentation** - All classes and functions
2. **Architecture Guide** - System design overview
3. **Build Instructions** - How to compile plugin
4. **Contribution Guide** - How to add features

### For Integration

1. **Adastrea Integration Guide** - How plugin connects
2. **Module Creation Guide** - Adding new module types
3. **Export Format Specification** - Blueprint structure
4. **Troubleshooting Guide** - Common issues

---

## Conclusion

This specification provides everything an AI agent needs to develop the Modular Station Designer plugin for Adastrea.

**Key Points:**
- ✅ Complete architecture defined
- ✅ All classes specified with code examples
- ✅ UI layout and workflow documented
- ✅ Integration with Adastrea fully detailed
- ✅ Testing strategy included
- ✅ 12-week implementation roadmap

**Next Steps for Agent:**
1. Create new repository: `ModularStationDesigner`
2. Initialize Unreal Engine 5.6 plugin structure
3. Follow implementation roadmap phase-by-phase
4. Test against real Adastrea project
5. Iterate based on user feedback

**Questions? Reference:**
- Adastrea architecture: `docs/reference/STATION_MODULE_ARCHITECTURE.md`
- Editor placement: `docs/reference/EDITOR_PLACED_MODULES_GUIDE.md`
- Module system: `Source/Adastrea/Public/Stations/SpaceStationModule.h`

---

**Document Status**: COMPLETE - Ready for Agent Implementation  
**Last Updated**: 2026-01-17  
**Version**: 1.0  
**Author**: Adastrea Development Team
