# TestSettingsWidget Enhancement Suggestions

**Status**: Optional improvements for future iterations  
**Priority**: Low to Medium  
**Current Implementation**: Production-ready as-is

These enhancements would make the TestSettingsWidget even more robust, but are **not required** for merging.

---

## 1. GameInstance Integration (Priority: Medium)

### Problem
Settings are currently stored only in the widget instance and are lost when the widget is destroyed.

### Solution
Store settings in `UAdastreaGameInstance` for persistence across level transitions.

### Implementation

**Step 1: Add properties to AdastreaGameInstance**

```cpp
// In AdastreaGameInstance.h
UPROPERTY(BlueprintReadWrite, Category="Test Settings")
FString TestShipType;

UPROPERTY(BlueprintReadWrite, Category="Test Settings")
FString TestDifficulty;

UPROPERTY(BlueprintReadWrite, Category="Test Settings")
FString TestDebugMode;

UPROPERTY(BlueprintReadWrite, Category="Test Settings")
bool bTestAutoSpawn;
```

**Step 2: Update ApplySettings_Implementation**

```cpp
void UTestSettingsWidget::ApplySettings_Implementation()
{
    UE_LOG(LogAdastrea, Log, TEXT("TestSettingsWidget: Applying settings..."));
    
    // Store in Game Instance for persistence
    if (UAdastreaGameInstance* GameInstance = Cast<UAdastreaGameInstance>(GetGameInstance()))
    {
        GameInstance->TestShipType = SelectedShipType;
        GameInstance->TestDifficulty = SelectedDifficulty;
        GameInstance->TestDebugMode = SelectedDebugMode;
        GameInstance->bTestAutoSpawn = bAutoSpawnShip;
        
        UE_LOG(LogAdastrea, Log, TEXT("  Settings stored in Game Instance"));
    }
    
    UE_LOG(LogAdastrea, Log, TEXT("  Applied Settings:"));
    UE_LOG(LogAdastrea, Log, TEXT("    - Ship Type: %s"), *SelectedShipType);
    UE_LOG(LogAdastrea, Log, TEXT("    - Difficulty: %s"), *SelectedDifficulty);
    UE_LOG(LogAdastrea, Log, TEXT("    - Debug Mode: %s"), *SelectedDebugMode);
    UE_LOG(LogAdastrea, Log, TEXT("    - Auto Spawn: %s"), bAutoSpawnShip ? TEXT("Yes") : TEXT("No"));
}
```

**Step 3: Update LoadDefaultSettings_Implementation**

```cpp
void UTestSettingsWidget::LoadDefaultSettings_Implementation()
{
    UE_LOG(LogAdastrea, Log, TEXT("TestSettingsWidget: Loading default settings"));
    
    // Try to load from Game Instance first
    if (UAdastreaGameInstance* GameInstance = Cast<UAdastreaGameInstance>(GetGameInstance()))
    {
        if (!GameInstance->TestShipType.IsEmpty())
        {
            SelectedShipType = GameInstance->TestShipType;
            SelectedDifficulty = GameInstance->TestDifficulty;
            SelectedDebugMode = GameInstance->TestDebugMode;
            bAutoSpawnShip = GameInstance->bTestAutoSpawn;
            
            UE_LOG(LogAdastrea, Log, TEXT("  Settings loaded from Game Instance"));
            return;
        }
    }
    
    // Fall back to defaults
    if (AvailableShipTypes.Num() > 0)
    {
        SelectedShipType = AvailableShipTypes[0];
    }
    
    if (AvailableDifficulties.Num() > 2)
    {
        SelectedDifficulty = AvailableDifficulties[2];
    }
    
    if (AvailableDebugModes.Num() > 0)
    {
        SelectedDebugMode = AvailableDebugModes[0];
    }
    
    bAutoSpawnShip = true;
    
    UE_LOG(LogAdastrea, Log, TEXT("  Default settings loaded"));
}
```

### Benefits
- Settings persist across level transitions
- Can be accessed from any GameMode
- Easy to integrate with save/load system
- No additional dependencies

---

## 2. Dedicated UI Log Category (Priority: Low)

### Problem
Currently uses generic `LogAdastrea` category, making UI-specific filtering harder.

### Solution
Add `LogAdastreaUI` category to `AdastreaLog.h`.

### Implementation

**In AdastreaLog.h:**
```cpp
// UI system
ADASTREA_API DECLARE_LOG_CATEGORY_EXTERN(LogAdastreaUI, Log, All);
```

**In AdastreaLog.cpp:**
```cpp
DEFINE_LOG_CATEGORY(LogAdastreaUI);
```

**In TestSettingsWidget.cpp:**
```cpp
#include "AdastreaLog.h"

// Change all logging statements:
UE_LOG(LogAdastreaUI, Log, TEXT("TestSettingsWidget: Widget constructed"));
UE_LOG(LogAdastreaUI, Log, TEXT("TestSettingsWidget: Settings initialized"));
// etc.
```

### Benefits
- Better log filtering: `Log LogAdastreaUI Verbose`
- Cleaner separation of UI vs gameplay logs
- Consistent with other system categories
- Easy to disable UI logs in shipping builds

---

## 3. Editor Validation Functions (Priority: Low)

### Problem
Configuration errors (empty arrays, invalid selections) only discovered at runtime.

### Solution
Add editor-time validation to catch issues early.

### Implementation

**In header (TestSettingsWidget.h):**
```cpp
#if WITH_EDITOR
    /**
     * Validate widget configuration in editor
     * Called when properties are changed or before cooking
     * @param ValidationErrors Output array for validation error messages
     * @return True if validation passed, false if errors found
     */
    UFUNCTION(BlueprintCallable, Category="Test Settings|Validation")
    bool ValidateConfiguration(TArray<FString>& ValidationErrors);
    
    /** Editor-only: PostEditChangeProperty override */
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
```

**In implementation (TestSettingsWidget.cpp):**
```cpp
#if WITH_EDITOR
bool UTestSettingsWidget::ValidateConfiguration(TArray<FString>& ValidationErrors)
{
    bool bIsValid = true;
    
    // Validate ship types
    if (AvailableShipTypes.Num() == 0)
    {
        ValidationErrors.Add(TEXT("No ship types configured. Add at least one ship type."));
        bIsValid = false;
    }
    
    // Check for duplicate ship types
    TSet<FString> UniqueShips;
    for (const FString& Ship : AvailableShipTypes)
    {
        if (Ship.IsEmpty())
        {
            ValidationErrors.Add(TEXT("Empty ship type in AvailableShipTypes array."));
            bIsValid = false;
        }
        else if (UniqueShips.Contains(Ship))
        {
            ValidationErrors.Add(FString::Printf(TEXT("Duplicate ship type: %s"), *Ship));
            bIsValid = false;
        }
        UniqueShips.Add(Ship);
    }
    
    // Validate difficulties
    if (AvailableDifficulties.Num() == 0)
    {
        ValidationErrors.Add(TEXT("No difficulty levels configured. Add at least one difficulty."));
        bIsValid = false;
    }
    
    // Validate debug modes
    if (AvailableDebugModes.Num() == 0)
    {
        ValidationErrors.Add(TEXT("No debug modes configured. Add at least one debug mode."));
        bIsValid = false;
    }
    
    // Validate default selections
    if (!SelectedShipType.IsEmpty() && !AvailableShipTypes.Contains(SelectedShipType))
    {
        ValidationErrors.Add(FString::Printf(
            TEXT("Default ship type '%s' not in AvailableShipTypes array."), 
            *SelectedShipType));
        bIsValid = false;
    }
    
    if (!SelectedDifficulty.IsEmpty() && !AvailableDifficulties.Contains(SelectedDifficulty))
    {
        ValidationErrors.Add(FString::Printf(
            TEXT("Default difficulty '%s' not in AvailableDifficulties array."), 
            *SelectedDifficulty));
        bIsValid = false;
    }
    
    if (bIsValid)
    {
        UE_LOG(LogAdastrea, Log, TEXT("TestSettingsWidget: Configuration validation passed"));
    }
    else
    {
        UE_LOG(LogAdastrea, Warning, TEXT("TestSettingsWidget: Configuration validation failed with %d errors"), 
               ValidationErrors.Num());
    }
    
    return bIsValid;
}

void UTestSettingsWidget::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    
    // Auto-validate when properties change in editor
    TArray<FString> ValidationErrors;
    if (!ValidateConfiguration(ValidationErrors))
    {
        for (const FString& Error : ValidationErrors)
        {
            UE_LOG(LogAdastrea, Warning, TEXT("  Validation Error: %s"), *Error);
        }
    }
}
#endif
```

### Benefits
- Catch configuration errors in editor
- Prevent invalid Widget Blueprints
- Better developer experience
- No runtime cost (editor-only)

---

## 4. Enhanced Input Validation (Priority: Low)

### Problem
Empty or invalid strings from dropdown selections aren't validated.

### Solution
Add input sanitization to all change handlers.

### Implementation

```cpp
void UTestSettingsWidget::OnShipTypeChanged_Implementation(const FString& SelectedShip, int32 SelectionIndex)
{
    // Validate input
    if (SelectedShip.IsEmpty())
    {
        UE_LOG(LogAdastrea, Warning, TEXT("TestSettingsWidget: Empty ship type selected, ignoring"));
        return;
    }
    
    if (SelectionIndex < 0 || SelectionIndex >= AvailableShipTypes.Num())
    {
        UE_LOG(LogAdastrea, Warning, TEXT("TestSettingsWidget: Invalid selection index %d, clamping"), 
               SelectionIndex);
        return;
    }
    
    if (!AvailableShipTypes.Contains(SelectedShip))
    {
        UE_LOG(LogAdastrea, Warning, 
               TEXT("TestSettingsWidget: Ship type '%s' not in available list, ignoring"), 
               *SelectedShip);
        return;
    }
    
    SelectedShipType = SelectedShip;
    UE_LOG(LogAdastrea, Log, TEXT("TestSettingsWidget: Ship type changed to: %s (index: %d)"), 
           *SelectedShip, SelectionIndex);
}

// Similar validation for OnDifficultyChanged and OnDebugModeChanged
```

### Benefits
- Prevents invalid selections
- Better error messages
- More robust against UI bugs
- Graceful degradation

---

## 5. MainMenuWidget Class Reference (Priority: Low)

### Problem
TransitionToMainMenu requires Blueprint override to specify main menu class.

### Solution
Add optional C++ integration via class reference.

### Implementation

**In header:**
```cpp
/**
 * Optional: Main menu widget class to display after test settings
 * If not set, TransitionToMainMenu must be overridden in Blueprint
 */
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Test Settings|Configuration")
TSubclassOf<UUserWidget> MainMenuWidgetClass;
```

**In TransitionToMainMenu_Implementation:**
```cpp
void UTestSettingsWidget::TransitionToMainMenu_Implementation()
{
    UE_LOG(LogAdastrea, Log, TEXT("TestSettingsWidget: Transitioning to main menu"));
    
    // Remove this widget from viewport
    RemoveFromParent();
    
    // Create main menu if class is specified
    if (MainMenuWidgetClass)
    {
        UUserWidget* MainMenu = CreateWidget<UUserWidget>(GetOwningPlayer(), MainMenuWidgetClass);
        if (MainMenu)
        {
            MainMenu->AddToViewport();
            UE_LOG(LogAdastrea, Log, TEXT("TestSettingsWidget: Main menu displayed"));
        }
        else
        {
            UE_LOG(LogAdastrea, Error, TEXT("TestSettingsWidget: Failed to create main menu widget"));
        }
    }
    else
    {
        // Blueprint must override to create main menu
        UE_LOG(LogAdastrea, Log, TEXT("TestSettingsWidget: MainMenuWidgetClass not set, expecting Blueprint override"));
    }
}
```

### Benefits
- C++ projects can use without Blueprint override
- More flexible integration
- Still supports Blueprint customization
- Better for automated testing

---

## 6. Settings Profiles System (Priority: Low)

### Problem
Can't save/load different test configurations.

### Solution
Add settings profiles feature.

### Implementation

**Add to header:**
```cpp
USTRUCT(BlueprintType)
struct FTestSettingsProfile
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ProfileName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ShipType;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Difficulty;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DebugMode;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoSpawn;
};

// In UTestSettingsWidget:
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Test Settings|Profiles")
TArray<FTestSettingsProfile> SavedProfiles;

UFUNCTION(BlueprintCallable, Category="Test Settings|Profiles")
void SaveCurrentProfile(const FString& ProfileName);

UFUNCTION(BlueprintCallable, Category="Test Settings|Profiles")
void LoadProfile(const FString& ProfileName);

UFUNCTION(BlueprintCallable, Category="Test Settings|Profiles")
TArray<FString> GetAvailableProfiles() const;
```

### Benefits
- Quick testing of different configurations
- Save common test scenarios
- Share test profiles with team
- Faster iteration

---

## 7. Debug Mode Application (Priority: Medium)

### Problem
Debug modes are selected but not actually applied.

### Solution
Implement debug mode application logic.

### Implementation

```cpp
void UTestSettingsWidget::OnDebugModeChanged_Implementation(const FString& SelectedMode, int32 SelectionIndex)
{
    SelectedDebugMode = SelectedMode;
    UE_LOG(LogAdastrea, Log, TEXT("TestSettingsWidget: Debug mode changed to: %s (index: %d)"), 
           *SelectedMode, SelectionIndex);
    
    // Apply debug mode immediately
    ApplyDebugMode(SelectedMode);
}

void UTestSettingsWidget::ApplyDebugMode(const FString& DebugMode)
{
    // Clear previous debug modes
    if (APlayerController* PC = GetOwningPlayer())
    {
        // Show FPS
        if (DebugMode == TEXT("Show FPS"))
        {
            PC->ConsoleCommand(TEXT("stat fps"));
        }
        // Show Collision
        else if (DebugMode == TEXT("Show Collision"))
        {
            PC->ConsoleCommand(TEXT("show collision"));
        }
        // Show Navigation
        else if (DebugMode == TEXT("Show Navigation"))
        {
            PC->ConsoleCommand(TEXT("show navigation"));
        }
        // Show All Stats
        else if (DebugMode == TEXT("Show All Stats"))
        {
            PC->ConsoleCommand(TEXT("stat unit"));
            PC->ConsoleCommand(TEXT("stat fps"));
        }
        // God Mode
        else if (DebugMode == TEXT("God Mode"))
        {
            PC->ConsoleCommand(TEXT("god"));
        }
        // Infinite Resources
        else if (DebugMode == TEXT("Infinite Resources"))
        {
            // Set cheat flag or call custom function
        }
        // None - clear all
        else if (DebugMode == TEXT("None"))
        {
            PC->ConsoleCommand(TEXT("stat fps 0"));
            PC->ConsoleCommand(TEXT("show collision 0"));
            PC->ConsoleCommand(TEXT("god 0"));
        }
    }
}
```

### Benefits
- Debug modes actually work
- Immediate visual feedback
- Better testing experience
- No need to manually enable debug modes

---

## 8. Automated Testing Support (Priority: Low)

### Problem
No automated tests to verify functionality.

### Solution
Add test fixtures and automation-friendly functions.

### Implementation

**Create TestSettingsWidget.spec.cpp:**
```cpp
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "UI/TestSettingsWidget.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestSettingsWidgetBasicTest, 
    "Adastrea.UI.TestSettingsWidget.Basic", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTestSettingsWidgetBasicTest::RunTest(const FString& Parameters)
{
    // Create widget
    UTestSettingsWidget* Widget = NewObject<UTestSettingsWidget>();
    TestNotNull(TEXT("Widget created"), Widget);
    
    // Test default values
    TestEqual(TEXT("Default ship type"), Widget->GetSelectedShipType(), FString(TEXT("Default Fighter")));
    TestEqual(TEXT("Default difficulty"), Widget->GetSelectedDifficulty(), FString(TEXT("Normal")));
    TestEqual(TEXT("Default debug mode"), Widget->GetSelectedDebugMode(), FString(TEXT("None")));
    TestTrue(TEXT("Auto spawn enabled by default"), Widget->GetAutoSpawnEnabled());
    
    // Test ship type change
    Widget->OnShipTypeChanged(TEXT("Gunship"), 3);
    TestEqual(TEXT("Ship type changed"), Widget->GetSelectedShipType(), FString(TEXT("Gunship")));
    
    // Test reset
    Widget->OnResetClicked();
    TestEqual(TEXT("Reset to default ship"), Widget->GetSelectedShipType(), FString(TEXT("Default Fighter")));
    
    return true;
}
```

### Benefits
- Automated regression testing
- CI/CD integration
- Catch breakages early
- Documentation via tests

---

## Implementation Priority

1. **High Priority (Implement Soon)**
   - GameInstance Integration (improves usability)
   - Debug Mode Application (makes feature actually work)

2. **Medium Priority (Nice to Have)**
   - Dedicated UI Log Category (better debugging)
   - Editor Validation (better DX)

3. **Low Priority (Future Enhancement)**
   - Enhanced Input Validation (defensive)
   - MainMenuWidget Class Reference (optional convenience)
   - Settings Profiles (power user feature)
   - Automated Testing (long-term quality)

---

## Conclusion

The current TestSettingsWidget implementation is **production-ready** without any of these enhancements. These suggestions represent potential future improvements that would make the widget more robust, but are **not blockers** for merging.

**Recommendation**: Merge current implementation, then create follow-up tasks for high-priority enhancements.
