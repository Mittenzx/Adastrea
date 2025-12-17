# Widget Initialization Guide for Adastrea

## Overview

This guide explains how widgets are initialized and managed in the Adastrea project. It covers the patterns used in recent C++ widget implementations and provides step-by-step instructions for adding new widgets to the system.

## Table of Contents

1. [Widget Architecture](#widget-architecture)
2. [Initialization Patterns](#initialization-patterns)
3. [Existing Widget Types](#existing-widget-types)
4. [Adding New Widgets](#adding-new-widgets)
5. [Blueprint Widget Creation](#blueprint-widget-creation)
6. [Troubleshooting](#troubleshooting)

---

## Widget Architecture

### Core Components

Adastrea uses Unreal Engine's UMG (Unreal Motion Graphics) system with a C++ backend and Blueprint frontend approach:

- **C++ Base Classes**: Define functionality, data, and BlueprintNativeEvents
- **Blueprint Widgets**: Handle visual layout and can override C++ behavior
- **PlayerController**: Manages widget lifecycle and viewport integration
- **Native Events**: Allow Blueprint customization while maintaining C++ defaults

### Widget Lifecycle

```
1. Widget Class Configuration (EditDefaultsOnly in PlayerController)
2. Widget Creation (CreateWidget<T>)
3. Widget Initialization (Custom Init functions like InitializeHUD, SetStation, etc.)
4. Add to Viewport (AddToViewport)
5. Native Events (NativeConstruct, NativeTick)
6. Widget Usage
7. Remove from Viewport (RemoveFromParent)
8. Widget Destruction (Garbage Collection)
```

---

## Initialization Patterns

### Pattern 1: Automatic Creation on BeginPlay (HUD Widget)

**Use Case**: Widgets that should always be visible during gameplay.

**Example: AdastreaHUDWidget**

```cpp
// In PlayerController.h
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD")
TSubclassOf<UAdastreaHUDWidget> HUDWidgetClass;

UPROPERTY(BlueprintReadOnly, Category="HUD")
UAdastreaHUDWidget* HUDWidget;

// In PlayerController.cpp - BeginPlay()
void AAdastreaPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Create HUD widget if class is set
    if (HUDWidgetClass)
    {
        HUDWidget = CreateWidget<UAdastreaHUDWidget>(this, HUDWidgetClass);
        if (HUDWidget)
        {
            HUDWidget->AddToViewport(0);  // Z-order 0 (bottom layer)
            HUDWidget->InitializeHUD();   // Custom initialization
            UE_LOG(LogAdastrea, Log, TEXT("Created and initialized HUD widget"));
        }
    }
}
```

**Key Points**:
- Widget is created immediately when player controller begins play
- Always visible (not toggled)
- Added to viewport at lowest Z-order (background layer)
- Custom `InitializeHUD()` function sets up initial state
- Widget class must be set in Blueprint or editor

---

### Pattern 2: On-Demand Creation with Data (Station Editor, Ship Status)

**Use Case**: Widgets that need specific data and are toggled on/off.

**Example: StationEditorWidget**

```cpp
// In PlayerController.h
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Player|Station Editor")
TSubclassOf<UUserWidget> StationEditorWidgetClass;

UPROPERTY()
UUserWidget* StationEditorWidget;

bool bIsStationEditorOpen;

// In PlayerController.cpp
void AAdastreaPlayerController::ToggleStationEditor()
{
    if (bIsStationEditorOpen)
    {
        HideStationEditor();
    }
    else
    {
        ASpaceStation* NearestStation = FindNearestStation();
        if (NearestStation)
        {
            ShowStationEditor(NearestStation);
        }
    }
}

UUserWidget* AAdastreaPlayerController::CreateStationEditorWidget()
{
    if (StationEditorWidget)
    {
        return StationEditorWidget;  // Return existing
    }

    if (!StationEditorWidgetClass)
    {
        UE_LOG(LogAdastrea, Error, TEXT("StationEditorWidgetClass not set!"));
        return nullptr;
    }

    StationEditorWidget = CreateWidget<UUserWidget>(this, StationEditorWidgetClass);
    return StationEditorWidget;
}

void AAdastreaPlayerController::ShowStationEditor(ASpaceStation* Station)
{
    if (!Station || !CreateStationEditorWidget())
    {
        return;
    }

    // Initialize widget with station data using reflection (Blueprint-safe)
    UFunction* SetStationFunc = StationEditorWidget->FindFunction(FName("SetStation"));
    if (SetStationFunc)
    {
        FSetStationParams Params;
        Params.Station = Station;
        StationEditorWidget->ProcessEvent(SetStationFunc, &Params);
    }

    // Add to viewport
    if (!StationEditorWidget->IsInViewport())
    {
        StationEditorWidget->AddToViewport();
    }

    // Switch to UI mode
    SetInputMode(FInputModeGameAndUI());
    bShowMouseCursor = true;
    bIsStationEditorOpen = true;
}

void AAdastreaPlayerController::HideStationEditor()
{
    if (StationEditorWidget && StationEditorWidget->IsInViewport())
    {
        StationEditorWidget->RemoveFromParent();
    }

    SetInputMode(FInputModeGameOnly());
    bShowMouseCursor = false;
    bIsStationEditorOpen = false;
}
```

**Key Points**:
- Widget is created only when first needed (lazy initialization)
- Persists after creation (reused on subsequent opens)
- Requires initialization with game data (station, spaceship, etc.)
- Uses reflection (`FindFunction`, `ProcessEvent`) to call Blueprint functions safely
- Manages input mode (Game vs UI)
- Tracks open/closed state with boolean flag

---

### Pattern 3: Component-Based Initialization (Inventory Widget)

**Use Case**: Widgets that display data from a component.

**Example: InventoryWidget**

```cpp
// In InventoryWidget.h
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API UInventoryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Inventory Widget")
    void InitializeInventory(UInventoryComponent* InInventoryComponent);
    virtual void InitializeInventory_Implementation(UInventoryComponent* InInventoryComponent);

protected:
    UPROPERTY(BlueprintReadOnly, Category="Inventory Widget")
    UInventoryComponent* InventoryComponent;

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
};

// In InventoryWidget.cpp
void UInventoryWidget::InitializeInventory_Implementation(UInventoryComponent* InInventoryComponent)
{
    // Unbind from previous component
    if (InventoryComponent)
    {
        InventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::OnInventoryChanged);
    }

    InventoryComponent = InInventoryComponent;

    // Bind to new component events
    if (InventoryComponent)
    {
        InventoryComponent->OnInventoryChanged.AddDynamic(this, &UInventoryWidget::OnInventoryChanged);
        RefreshInventory_Implementation();
    }
}

void UInventoryWidget::NativeDestruct()
{
    // Clean up event bindings
    if (InventoryComponent)
    {
        InventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::OnInventoryChanged);
    }
    Super::NativeDestruct();
}
```

**Usage in PlayerController**:
```cpp
// Create inventory widget
UInventoryWidget* InventoryWidget = CreateWidget<UInventoryWidget>(this, InventoryWidgetClass);

// Get player's inventory component
UInventoryComponent* PlayerInventory = GetPawn()->FindComponentByClass<UInventoryComponent>();

// Initialize with component
if (InventoryWidget && PlayerInventory)
{
    InventoryWidget->InitializeInventory(PlayerInventory);
    InventoryWidget->AddToViewport();
}
```

**Key Points**:
- Widget is initialized with a component reference
- Automatically binds to component events for updates
- Unbinds events in `NativeDestruct` to prevent memory leaks
- Component provides all data the widget needs

---

## Existing Widget Types

### 1. AdastreaHUDWidget
- **Purpose**: Main heads-up display during gameplay
- **Creation**: Automatic on BeginPlay
- **Initialization**: `InitializeHUD()`
- **Location**: `Source/Adastrea/UI/AdastreaHUDWidget.h`
- **Features**:
  - Ship health/shield display
  - Speed and navigation info
  - Target information
  - Weapon status
  - Alert messages
  - Player and ship info
  - Weapon aim crosshair

**Key Functions**:
```cpp
void InitializeHUD();
void UpdateHealth(float CurrentHealth, float MaxHealth);
void UpdateShield(float CurrentShield, float MaxShield);
void UpdateSpeed(float CurrentSpeed, float MaxSpeed);
void UpdateTargetInfo(const FText& TargetName, float TargetDistance, bool bIsHostile);
void ShowAlert(const FText& Message, float Duration, bool bIsWarning);
```

---

### 2. StationEditorWidget
- **Purpose**: UI for editing and managing space stations
- **Creation**: On-demand (via Toggle function)
- **Initialization**: `SetStation(ASpaceStation*)` or `InitializeEditor(ASpaceStation*, UDataAsset*)`
- **Location**: `Source/StationEditor/UI/StationEditorWidget.h`
- **Features**:
  - Module placement/removal/movement
  - Construction queue
  - Faction integration
  - Undo/Redo support
  - Statistics display
  - Module connections
  - Notifications

**Key Functions**:
```cpp
void SetStation(ASpaceStation* Station);
ASpaceStationModule* AddModule(TSubclassOf<ASpaceStationModule> ModuleClass, FVector RelativeLocation);
bool RemoveModule(ASpaceStationModule* Module);
bool MoveModule(ASpaceStationModule* Module, FVector NewRelativeLocation);
TArray<ASpaceStationModule*> GetAllModules();
```

---

### 3. ShipStatusWidget
- **Purpose**: Detailed view of spaceship statistics and configuration
- **Creation**: On-demand (via Toggle function)
- **Initialization**: `InitializeWithSpaceship(ASpaceship*)`
- **Location**: `Source/Adastrea/UI/ShipStatusWidget.h`
- **Features**:
  - Ship statistics display
  - Component information
  - Configuration options

**Key Functions**:
```cpp
void InitializeWithSpaceship(ASpaceship* Spaceship);
void RefreshDisplay();
```

---

### 4. MainMenuWidget
- **Purpose**: In-game pause menu
- **Creation**: On-demand (via Toggle function)
- **Initialization**: None required (simple menu)
- **Location**: `Source/Adastrea/UI/MainMenuWidget.h`
- **Features**:
  - Pause/resume game
  - Settings access
  - Return to main menu
  - Quit game

---

### 5. InventoryWidget
- **Purpose**: Player inventory management
- **Creation**: On-demand
- **Initialization**: `InitializeInventory(UInventoryComponent*)`
- **Location**: `Source/Adastrea/UI/InventoryWidget.h`
- **Features**:
  - Item grid/list display
  - Item details and tooltips
  - Drag and drop
  - Sorting and filtering
  - Item usage

**Key Functions**:
```cpp
void InitializeInventory(UInventoryComponent* InInventoryComponent);
void RefreshInventory();
void OnSlotSelected(int32 SlotIndex);
void OnUseItem(int32 SlotIndex);
void OnDropItem(int32 SlotIndex, int32 Quantity);
void SortInventory();
```

**Integration**: Currently not integrated into PlayerController - needs to be added.

---

### 6. TradingInterfaceWidget
- **Purpose**: Enhanced trading UI with price tracking and bulk operations
- **Creation**: On-demand
- **Initialization**: `SetTradePartner(UFactionDataAsset*)`
- **Location**: `Source/Adastrea/Public/UI/TradingInterfaceWidget.h`
- **Features**:
  - Price comparison
  - Bulk trading
  - Filters and sorting
  - Trade route suggestions
  - Market trends

**Key Functions**:
```cpp
void SetTradePartner(UFactionDataAsset* Faction);
TArray<UTradeItemDataAsset*> GetFilteredItems(const FString& Category);
float GetItemPrice(const UTradeItemDataAsset* Item, int32 Quantity, bool bBuying);
void AddToCart(UTradeItemDataAsset* Item, int32 Quantity);
bool ExecuteTrade();
```

**Integration**: Currently not integrated into PlayerController - needs to be added.

---

### 7. StationManagementWidget
- **Purpose**: Comprehensive station management interface
- **Creation**: On-demand
- **Initialization**: `SetManagedStation(ASpaceStation*)`
- **Location**: `Source/Adastrea/Public/UI/StationManagementWidget.h`
- **Features**:
  - Station status display
  - Resource management
  - Module building/upgrading
  - Station configuration

**Key Functions**:
```cpp
void SetManagedStation(ASpaceStation* Station);
void RefreshStationData();
TArray<ASpaceStationModule*> GetModulesByGroup(EStationModuleGroup Group);
bool RequestBuildModule(TSubclassOf<ASpaceStationModule> ModuleClass);
```

**Integration**: Currently not integrated into PlayerController - needs to be added.

---

## Adding New Widgets

### Step 1: Create C++ Widget Class

```cpp
// In YourWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "YourWidget.generated.h"

UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API UYourWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UYourWidget(const FObjectInitializer& ObjectInitializer);

    // Initialization function
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Your Widget")
    void InitializeWidget(/* parameters */);
    virtual void InitializeWidget_Implementation(/* parameters */);

    // Other widget functions
    UFUNCTION(BlueprintCallable, Category="Your Widget")
    void SomeFunction();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // Widget state
    UPROPERTY(BlueprintReadOnly, Category="Your Widget")
    bool bIsInitialized;
};
```

```cpp
// In YourWidget.cpp
#include "UI/YourWidget.h"

UYourWidget::UYourWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , bIsInitialized(false)
{
}

void UYourWidget::NativeConstruct()
{
    Super::NativeConstruct();
    // Called when widget is added to viewport
}

void UYourWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    // Called every frame when widget is visible
}

void UYourWidget::InitializeWidget_Implementation(/* parameters */)
{
    // Custom initialization logic
    bIsInitialized = true;
}
```

---

### Step 2: Add to PlayerController

**For Automatic Creation (Always Visible):**

```cpp
// In AdastreaPlayerController.h
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Your Widget")
TSubclassOf<UYourWidget> YourWidgetClass;

UPROPERTY(BlueprintReadOnly, Category="Your Widget")
UYourWidget* YourWidget;

// In AdastreaPlayerController.cpp - BeginPlay()
if (YourWidgetClass)
{
    YourWidget = CreateWidget<UYourWidget>(this, YourWidgetClass);
    if (YourWidget)
    {
        YourWidget->AddToViewport();
        YourWidget->InitializeWidget(/* params */);
    }
}
```

**For On-Demand Creation (Toggled):**

```cpp
// In AdastreaPlayerController.h
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Your Widget")
TSubclassOf<UYourWidget> YourWidgetClass;

UPROPERTY(BlueprintReadOnly, Category="Your Widget")
UYourWidget* YourWidget;

UFUNCTION(BlueprintCallable, Category="Your Widget")
void ToggleYourWidget();

UFUNCTION(BlueprintCallable, BlueprintPure, Category="Your Widget")
bool IsYourWidgetOpen() const;

private:
    bool bIsYourWidgetOpen;
    UYourWidget* CreateYourWidget();
    void ShowYourWidget();
    void HideYourWidget();

// In AdastreaPlayerController.cpp
void AAdastreaPlayerController::ToggleYourWidget()
{
    if (bIsYourWidgetOpen)
    {
        HideYourWidget();
    }
    else
    {
        ShowYourWidget();
    }
}

UYourWidget* AAdastreaPlayerController::CreateYourWidget()
{
    if (YourWidget)
    {
        return YourWidget;
    }

    if (!YourWidgetClass)
    {
        UE_LOG(LogAdastrea, Error, TEXT("YourWidgetClass not set!"));
        return nullptr;
    }

    YourWidget = CreateWidget<UYourWidget>(this, YourWidgetClass);
    return YourWidget;
}

void AAdastreaPlayerController::ShowYourWidget()
{
    if (!CreateYourWidget())
    {
        return;
    }

    // Initialize if needed
    // YourWidget->InitializeWidget(/* params */);

    if (!YourWidget->IsInViewport())
    {
        YourWidget->AddToViewport();
    }

    SetInputMode(FInputModeGameAndUI());
    bShowMouseCursor = true;
    bIsYourWidgetOpen = true;
}

void AAdastreaPlayerController::HideYourWidget()
{
    if (YourWidget && YourWidget->IsInViewport())
    {
        YourWidget->RemoveFromParent();
    }

    SetInputMode(FInputModeGameOnly());
    bShowMouseCursor = false;
    bIsYourWidgetOpen = false;
}

bool AAdastreaPlayerController::IsYourWidgetOpen() const
{
    return bIsYourWidgetOpen && YourWidget && YourWidget->IsInViewport();
}
```

---

### Step 3: Create Blueprint Widget

1. **In Unreal Editor**, navigate to `Content/UI/` (or appropriate folder)
2. **Right-click** and select **User Interface > Widget Blueprint**
3. **Name** the widget: `WBP_YourWidget`
4. **Open** the widget blueprint
5. **Go to Graph** view
6. **Click "Class Settings"** in the toolbar
7. **In Details panel**, find **"Parent Class"**
8. **Set Parent Class** to your C++ class (e.g., `YourWidget`)
9. **Switch to Designer** view
10. **Design your UI** layout using UMG components
11. **Compile and Save**

---

### Step 4: Configure in PlayerController Blueprint

1. **Open** your PlayerController Blueprint (e.g., `BP_AdastreaPlayerController`)
2. **In Details panel**, find **"Your Widget"** category
3. **Set "Your Widget Class"** to the Widget Blueprint you created (e.g., `WBP_YourWidget`)
4. **Compile and Save**

---

### Step 5: Bind Input (if toggled)

**In your Input Action Asset** (e.g., `DA_InputConfig`):
1. Add a new Input Action (e.g., `IA_ToggleYourWidget`)
2. Bind a key (e.g., `I` key)

**In your Input Mapping Context or Blueprint**:
```cpp
// Bind in C++ (if using Enhanced Input)
EnhancedInputComponent->BindAction(ToggleYourWidgetAction, ETriggerEvent::Triggered, this, &AAdastreaPlayerController::ToggleYourWidget);
```

Or in Blueprint:
- Add event for Input Action
- Call `ToggleYourWidget` function

---

## Blueprint Widget Creation

### Basic Blueprint Widget Workflow

1. **Create Widget Blueprint** as described above
2. **Design Layout** in Designer view:
   - Add panels (Canvas, Vertical Box, Horizontal Box, etc.)
   - Add components (Text, Image, Button, Progress Bar, etc.)
   - Set anchors for responsive layout
   - Name components for easy reference

3. **Override Native Events** in Graph view:
   - `Event Construct`: Called when widget is added to viewport
   - `Event Tick`: Called every frame (use sparingly for performance)
   - `Event Destruct`: Called when widget is removed

4. **Override C++ Functions**:
   - Find your C++ function (e.g., `InitializeWidget`)
   - Right-click > **Add Event > Event InitializeWidget**
   - Implement Blueprint logic
   - Optionally call **Parent: InitializeWidget** to keep C++ behavior

5. **Bind UI Events**:
   - Select a button in Designer
   - In Details > Events > OnClicked, click the **+** button
   - Implement button click logic in Graph

---

### Example: Creating a Simple Status Widget

**C++ Base Class** (`SimpleStatusWidget.h`):
```cpp
UCLASS()
class ADASTREA_API USimpleStatusWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Status")
    void UpdateStatus(const FText& StatusText, float HealthPercent);
    virtual void UpdateStatus_Implementation(const FText& StatusText, float HealthPercent);

protected:
    UPROPERTY(BlueprintReadOnly, Category="Status")
    FText CurrentStatus;

    UPROPERTY(BlueprintReadOnly, Category="Status")
    float CurrentHealth;
};
```

**Blueprint Widget** (`WBP_SimpleStatus`):
1. Set **Parent Class** to `SimpleStatusWidget`
2. **Add components**:
   - Text Block named "StatusText"
   - Progress Bar named "HealthBar"
3. **Override UpdateStatus** in Graph:
   - Get "StatusText" component > Set Text to `StatusText` parameter
   - Get "HealthBar" component > Set Percent to `HealthPercent` parameter
   - Call Parent: UpdateStatus

---

## Troubleshooting

### Issue: Widget Class Not Set Error

**Symptom**: Log shows `"YourWidgetClass is not set! Assign it in Blueprint."`

**Solution**:
1. Open your PlayerController Blueprint
2. Find the widget class property in Details panel
3. Set it to your Widget Blueprint (e.g., `WBP_YourWidget`)
4. Compile and save

---

### Issue: Widget Not Appearing on Screen

**Possible Causes**:
1. Widget class not set in PlayerController Blueprint
2. Widget not added to viewport
3. Widget Z-order behind other widgets
4. Widget size or anchors set incorrectly

**Solutions**:
- Verify widget class is set
- Check that `AddToViewport()` is called
- Try different Z-order: `AddToViewport(100)` for higher layer
- In Designer, check widget's Canvas Panel settings

---

### Issue: Input Not Working in Widget

**Possible Causes**:
1. Input mode not set to UI or GameAndUI
2. Widget not receiving focus
3. Button "Is Enabled" is unchecked

**Solutions**:
```cpp
// Set input mode to allow UI interaction
SetInputMode(FInputModeGameAndUI());
bShowMouseCursor = true;

// Or for UI only:
FInputModeUIOnly InputMode;
InputMode.SetWidgetToFocus(YourWidget->TakeWidget());
SetInputMode(InputMode);
```

---

### Issue: Widget Updates Not Reflecting in UI

**Possible Causes**:
1. Blueprint not overriding C++ function
2. UI components not bound to variables
3. Not calling Refresh/Update functions

**Solutions**:
- Override the BlueprintNativeEvent in Blueprint
- In Designer, check "Is Variable" for components you need to reference
- Call refresh functions after data changes
- Use Blueprint's Event Tick for continuous updates (sparingly)

---

### Issue: Widget Persists After Removal

**Possible Causes**:
1. Not calling `RemoveFromParent()`
2. Multiple references to widget
3. Widget still referenced elsewhere

**Solutions**:
```cpp
if (Widget && Widget->IsInViewport())
{
    Widget->RemoveFromParent();
}
```

---

### Issue: Memory Leaks or Crashes

**Possible Causes**:
1. Not unbinding delegates in `NativeDestruct`
2. Accessing null pointers
3. Widget outliving its referenced objects

**Solutions**:
- Always unbind delegates in `NativeDestruct`:
  ```cpp
  void UYourWidget::NativeDestruct()
  {
      if (Component)
      {
          Component->OnEvent.RemoveDynamic(this, &UYourWidget::OnEventHandler);
      }
      Super::NativeDestruct();
  }
  ```
- Use `IsValid()` or null checks before accessing pointers
- Mark widget pointers with `UPROPERTY()` for garbage collection

---

## Best Practices

### 1. Use BlueprintNativeEvent for Extensibility
```cpp
// Allows Blueprint to override while providing C++ default
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Widget")
void UpdateDisplay();
virtual void UpdateDisplay_Implementation();
```

### 2. Initialize State in Constructor
```cpp
UYourWidget::UYourWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , bIsInitialized(false)
    , CurrentValue(0.0f)
{
}
```

### 3. Clean Up in NativeDestruct
```cpp
void UYourWidget::NativeDestruct()
{
    // Unbind delegates
    // Clear timers
    // Release references
    Super::NativeDestruct();
}
```

### 4. Use UPROPERTY for All Widget Pointers
```cpp
// Ensures garbage collection works correctly
UPROPERTY()
UUserWidget* SubWidget;
```

### 5. Log Errors for Missing Configuration
```cpp
if (!WidgetClass)
{
    UE_LOG(LogAdastrea, Error, TEXT("WidgetClass not set! Assign in Blueprint."));
    return nullptr;
}
```

### 6. Manage Input Modes Properly
```cpp
// When showing UI:
SetInputMode(FInputModeGameAndUI());
bShowMouseCursor = true;

// When hiding UI:
SetInputMode(FInputModeGameOnly());
bShowMouseCursor = false;
```

### 7. Use Lazy Initialization for On-Demand Widgets
```cpp
// Create only when first needed
UYourWidget* AYourController::GetOrCreateWidget()
{
    if (!Widget && WidgetClass)
    {
        Widget = CreateWidget<UYourWidget>(this, WidgetClass);
    }
    return Widget;
}
```

### 8. Provide Clear Documentation
- Document all public functions with Doxygen comments
- Explain parameters and return values
- Note required initialization steps
- Provide usage examples

---

## Summary

### Quick Reference: Adding a Widget

1. **Create C++ class** derived from `UUserWidget`
2. **Add initialization function** (BlueprintNativeEvent)
3. **Add to PlayerController**:
   - Property for widget class
   - Property for widget instance
   - Create/Show/Hide functions
   - Toggle function (if on-demand)
4. **Create Blueprint Widget** based on C++ class
5. **Design UI** in UMG Designer
6. **Set widget class** in PlayerController Blueprint
7. **Bind input** (if toggled)
8. **Test** in editor

### Widget Integration Checklist

- [ ] C++ class created with proper UCLASS macros
- [ ] Initialization function implemented
- [ ] PlayerController properties added
- [ ] CreateWidget logic implemented
- [ ] AddToViewport/RemoveFromParent logic implemented
- [ ] Input mode management implemented
- [ ] Blueprint Widget created and configured
- [ ] UI designed in Designer view
- [ ] Widget class set in PlayerController Blueprint
- [ ] Input actions bound (if applicable)
- [ ] Tested in Play-In-Editor
- [ ] Documentation updated

---

## Next Steps

### Widgets Needing Integration

The following widgets are implemented in C++ but not yet integrated into the PlayerController:

1. **InventoryWidget** - Needs toggle function and initialization with player's inventory component
2. **TradingInterfaceWidget** - Needs toggle function and initialization with trade partner
3. **StationManagementWidget** - Needs toggle function and initialization with station reference

See [Adding Missing Widget Integration](#adding-missing-widget-integration) section for implementation details.

---

## Adding Missing Widget Integration

### 1. Integrating InventoryWidget

**Add to PlayerController.h**:
```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
TSubclassOf<UInventoryWidget> InventoryWidgetClass;

UPROPERTY(BlueprintReadOnly, Category="Inventory")
UInventoryWidget* InventoryWidget;

UFUNCTION(BlueprintCallable, Category="Inventory")
void ToggleInventory();

UFUNCTION(BlueprintCallable, BlueprintPure, Category="Inventory")
bool IsInventoryOpen() const;

private:
    bool bIsInventoryOpen;
    UInventoryWidget* CreateInventoryWidget();
    void ShowInventory();
    void HideInventory();
```

**Implementation** (see Step 2 pattern above for full code)

---

### 2. Integrating TradingInterfaceWidget

**Add to PlayerController.h**:
```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trading")
TSubclassOf<UTradingInterfaceWidget> TradingWidgetClass;

UPROPERTY(BlueprintReadOnly, Category="Trading")
UTradingInterfaceWidget* TradingWidget;

UFUNCTION(BlueprintCallable, Category="Trading")
void OpenTrading(UFactionDataAsset* TradePartner);

UFUNCTION(BlueprintCallable, Category="Trading")
void CloseTrading();

UFUNCTION(BlueprintCallable, BlueprintPure, Category="Trading")
bool IsTradingOpen() const;

private:
    bool bIsTradingOpen;
    UTradingInterfaceWidget* CreateTradingWidget();
```

**Implementation**: Similar to Station Editor pattern, but initialize with faction

---

### 3. Integrating StationManagementWidget

**Add to PlayerController.h**:
```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Station Management")
TSubclassOf<UStationManagementWidget> StationManagementWidgetClass;

UPROPERTY(BlueprintReadOnly, Category="Station Management")
UStationManagementWidget* StationManagementWidget;

UFUNCTION(BlueprintCallable, Category="Station Management")
void OpenStationManagement(ASpaceStation* Station);

UFUNCTION(BlueprintCallable, Category="Station Management")
void CloseStationManagement();

UFUNCTION(BlueprintCallable, BlueprintPure, Category="Station Management")
bool IsStationManagementOpen() const;

private:
    bool bIsStationManagementOpen;
    UStationManagementWidget* CreateStationManagementWidget();
```

**Implementation**: Similar to Station Editor pattern, but for management instead of editing

---

## Additional Resources

- **Unreal Engine Documentation**: [UMG UI Designer](https://docs.unrealengine.com/en-US/umg-ui-designer/)
- **Adastrea Code Style**: `CODE_STYLE.md`
- **Widget Usage Guides**: `Source/Adastrea/UI/*_Usage.md`
- **Example Widgets**: Browse `Source/Adastrea/UI/` and `Source/StationEditor/UI/`

---

**Last Updated**: 2025-12-17  
**Version**: 1.0  
**Author**: Adastrea Development Team
