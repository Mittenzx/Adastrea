# AdastreaEditor: Next Steps Implementation Guide

**Quick Start Guide for Implementing Editor Customizations**

---

## TL;DR - What to Do Next

The AdastreaEditor module is a skeleton waiting for implementation. Start by implementing a detail customization for `USpaceshipDataAsset` to dramatically improve the designer experience.

**Estimated Time to First Working Feature:** 4-8 hours  
**Impact:** Immediate improvement to spaceship Data Asset editing workflow

---

## Quick Win: First Detail Customization (SpaceshipDataAsset)

### Why Start Here?

1. **High Impact** - Spaceships have 50+ properties across 7 categories
2. **Clear Benefit** - Current editing experience is overwhelming
3. **Learning Opportunity** - Demonstrates all key concepts
4. **Designer Feedback** - Already identified as pain point

### Step-by-Step Implementation

#### Step 1: Create the Detail Customization Class

**File:** `Source/AdastreaEditor/Public/Customizations/SpaceshipDataAssetDetails.h`

```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class USpaceshipDataAsset;

/**
 * Custom detail panel for USpaceshipDataAsset
 * Provides organized categories, real-time calculations, and validation
 */
class FSpaceshipDataAssetDetails : public IDetailCustomization
{
public:
    /** Create an instance of this customization */
    static TSharedRef<IDetailCustomization> MakeInstance();

    /** IDetailCustomization interface */
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
    /** Customize the Core Stats category */
    void CustomizeCoreStats(IDetailLayoutBuilder& DetailBuilder);
    
    /** Customize the Combat Stats category */
    void CustomizeCombatStats(IDetailLayoutBuilder& DetailBuilder);
    
    /** Calculate and display real-time combat rating */
    FText GetCombatRatingText() const;
    
    /** Calculate and display real-time mobility rating */
    FText GetMobilityRatingText() const;
    
    /** Cache the spaceship being edited */
    TWeakObjectPtr<USpaceshipDataAsset> CachedSpaceship;
};
```

#### Step 2: Implement the Detail Customization

**File:** `Source/AdastreaEditor/Private/Customizations/SpaceshipDataAssetDetails.cpp`

```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#include "Customizations/SpaceshipDataAssetDetails.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Text/STextBlock.h"
#include "Ships/SpaceshipDataAsset.h"

#define LOCTEXT_NAMESPACE "SpaceshipDataAssetDetails"

TSharedRef<IDetailCustomization> FSpaceshipDataAssetDetails::MakeInstance()
{
    return MakeShareable(new FSpaceshipDataAssetDetails);
}

void FSpaceshipDataAssetDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    // Get the object being customized
    TArray<TWeakObjectPtr<UObject>> Objects;
    DetailBuilder.GetObjectsBeingCustomized(Objects);
    
    if (Objects.Num() > 0)
    {
        CachedSpaceship = Cast<USpaceshipDataAsset>(Objects[0].Get());
    }
    
    // Customize categories
    CustomizeCoreStats(DetailBuilder);
    CustomizeCombatStats(DetailBuilder);
    
    // Add calculated stats display
    IDetailCategoryBuilder& CalculatedStatsCategory = 
        DetailBuilder.EditCategory("Calculated Stats", 
                                   FText::GetEmpty(), 
                                   ECategoryPriority::Important);
    
    // Add combat rating display
    CalculatedStatsCategory.AddCustomRow(LOCTEXT("CombatRating", "Combat Rating"))
        .NameContent()
        [
            SNew(STextBlock)
            .Text(LOCTEXT("CombatRating", "Combat Rating"))
            .Font(IDetailLayoutBuilder::GetDetailFontBold())
        ]
        .ValueContent()
        [
            SNew(STextBlock)
            .Text(this, &FSpaceshipDataAssetDetails::GetCombatRatingText)
            .Font(IDetailLayoutBuilder::GetDetailFont())
        ];
    
    // Add mobility rating display
    CalculatedStatsCategory.AddCustomRow(LOCTEXT("MobilityRating", "Mobility Rating"))
        .NameContent()
        [
            SNew(STextBlock)
            .Text(LOCTEXT("MobilityRating", "Mobility Rating"))
            .Font(IDetailLayoutBuilder::GetDetailFontBold())
        ]
        .ValueContent()
        [
            SNew(STextBlock)
            .Text(this, &FSpaceshipDataAssetDetails::GetMobilityRatingText)
            .Font(IDetailLayoutBuilder::GetDetailFont())
        ];
}

void FSpaceshipDataAssetDetails::CustomizeCoreStats(IDetailLayoutBuilder& DetailBuilder)
{
    // Core stats are already organized in the default layout
    // This is a placeholder for future enhancements
}

void FSpaceshipDataAssetDetails::CustomizeCombatStats(IDetailLayoutBuilder& DetailBuilder)
{
    // Combat stats are already organized in the default layout
    // This is a placeholder for future enhancements
}

FText FSpaceshipDataAssetDetails::GetCombatRatingText() const
{
    if (CachedSpaceship.IsValid())
    {
        float Rating = CachedSpaceship->GetCombatRating();
        return FText::Format(LOCTEXT("CombatRatingValue", "{0} / 100"), 
                            FText::AsNumber(FMath::RoundToInt(Rating)));
    }
    return LOCTEXT("InvalidSpaceship", "N/A");
}

FText FSpaceshipDataAssetDetails::GetMobilityRatingText() const
{
    if (CachedSpaceship.IsValid())
    {
        float Rating = CachedSpaceship->GetMobilityRating();
        return FText::Format(LOCTEXT("MobilityRatingValue", "{0} / 100"), 
                            FText::AsNumber(FMath::RoundToInt(Rating)));
    }
    return LOCTEXT("InvalidSpaceship", "N/A");
}

#undef LOCTEXT_NAMESPACE
```

#### Step 3: Register the Customization

**Update:** `Source/AdastreaEditor/AdastreaEditor.cpp`

```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#include "AdastreaEditor.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Ships/SpaceshipDataAsset.h"
#include "Customizations/SpaceshipDataAssetDetails.h"

void FAdastreaEditorModule::StartupModule()
{
    // Register detail customizations
    FPropertyEditorModule& PropertyModule = 
        FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    
    PropertyModule.RegisterCustomClassLayout(
        USpaceshipDataAsset::StaticClass()->GetFName(),
        FOnGetDetailCustomizationInstance::CreateStatic(
            &FSpaceshipDataAssetDetails::MakeInstance
        )
    );
}

void FAdastreaEditorModule::ShutdownModule()
{
    // Unregister customizations
    if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
    {
        FPropertyEditorModule& PropertyModule = 
            FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
        
        PropertyModule.UnregisterCustomClassLayout(
            USpaceshipDataAsset::StaticClass()->GetFName()
        );
    }
}

IMPLEMENT_MODULE(FAdastreaEditorModule, AdastreaEditor);
```

#### Step 4: Build and Test

1. **Rebuild the project** in Visual Studio (Development Editor configuration)
2. **Open Unreal Editor**
3. **Navigate to a SpaceshipDataAsset** in the Content Browser
4. **Open the asset** - you should see the calculated stats display
5. **Edit properties** - watch the calculated ratings update in real-time

### Expected Result

When you open a SpaceshipDataAsset, you'll see:
- A new "Calculated Stats" section at the top
- Real-time Combat Rating display (e.g., "75 / 100")
- Real-time Mobility Rating display (e.g., "62 / 100")
- These values update as you change the underlying stats

---

## Next 5 Implementation Steps

### 1. Add Visual Feedback for Stats (Week 1)

**Goal:** Add color-coding and progress bars for ratings

**Changes:**
- Add color coding (Green: 80-100, Yellow: 50-79, Red: 0-49)
- Add progress bars using `SProgressBar`
- Add validation warnings for stats out of reasonable ranges

**Impact:** Visual feedback makes it easier to understand ship capabilities at a glance.

### 2. Implement Property Type Customization for FFactionTrait (Week 2)

**Goal:** Make trait editing visual and intuitive

**Implementation:**
- Create `FFactionTraitCustomization` class
- Add category icon display
- Add visual modifier value editor
- Add quick preset buttons

**Impact:** Traits are easier to understand and edit.

### 3. Add Asset Validators (Week 2)

**Goal:** Catch data errors at save time

**Implementation:**
- Create `USpaceshipDataAssetValidator`
- Check required fields (DisplayName, ShipClass)
- Validate ranges (Mass > 0, Crew >= 0)
- Suggest fixes for common errors

**Impact:** Fewer broken Data Assets in content.

### 4. Implement FactionDataAsset Detail Customization (Week 3)

**Goal:** Improve faction editing experience

**Features:**
- Visual relationship graph preview
- Trait summary with category grouping
- Color picker for faction colors
- Validation for relationship targets

**Impact:** Faction creation is streamlined and less error-prone.

### 5. Add Custom Asset Actions (Week 3)

**Goal:** Streamline common workflows

**Actions:**
- "Duplicate and Modify" - Quick variant creation
- "Export to YAML" - Export for documentation
- "Validate References" - Check for broken links
- "Preview Calculated Stats" - Open preview window

**Impact:** Designer workflows are faster and more efficient.

---

## Common Pitfalls to Avoid

### 1. Forgetting to Unregister Customizations

**Problem:** Causes crashes on hot reload or editor shutdown.

**Solution:** Always implement proper cleanup in `ShutdownModule()` and check if module is loaded.

### 2. Caching Stale Object Pointers

**Problem:** Using cached pointers after objects are deleted.

**Solution:** Use `TWeakObjectPtr` and always check `IsValid()` before dereferencing.

### 3. Expensive Operations in Detail Refresh

**Problem:** Detail panels can refresh frequently, causing lag.

**Solution:** Cache expensive calculations, use async operations for complex work.

### 4. Not Testing Hot Reload

**Problem:** Customizations work initially but crash on hot reload.

**Solution:** Test hot reload workflow extensively during development.

### 5. Forgetting Module Dependencies

**Problem:** Compilation errors due to missing headers or modules.

**Solution:** Verify `AdastreaEditor.Build.cs` has all required dependencies.

---

## Development Workflow Tips

### Fast Iteration with Hot Reload

1. **Make code changes** in Visual Studio
2. **Save all files**
3. **In Unreal Editor:** Press **Ctrl + Alt + F11** (or use Live Coding)
4. **Wait for compilation**
5. **Test changes immediately** (no editor restart needed)

### Debugging Detail Customizations

```cpp
// Add logging to understand execution flow
UE_LOG(LogTemp, Log, TEXT("Customizing SpaceshipDataAsset: %s"), 
       *CachedSpaceship->GetName());

// Use breakpoints in Visual Studio
// Set breakpoint in CustomizeDetails() to step through code

// Check object validity
if (!CachedSpaceship.IsValid())
{
    UE_LOG(LogTemp, Warning, TEXT("Invalid spaceship object!"));
    return;
}
```

### Testing Checklist

- [ ] Customization appears when opening Data Asset
- [ ] Properties can still be edited normally
- [ ] Calculated values update in real-time
- [ ] Hot reload works without crashes
- [ ] Editor shutdown is clean (no warnings)
- [ ] Multiple assets can be open simultaneously
- [ ] Undo/Redo works correctly

---

## Resources and References

### Unreal Engine Documentation

- **Detail Customization:** https://docs.unrealengine.com/5.0/en-US/customizing-detail-panels-in-unreal-engine/
- **Slate Widgets:** https://docs.unrealengine.com/5.0/en-US/slate-ui-framework-in-unreal-engine/
- **Editor Modules:** https://docs.unrealengine.com/5.0/en-US/editor-extensions-in-unreal-engine/

### Code Examples

- **Engine Source:** `Engine/Source/Editor/DetailCustomizations/Private/`
- Look at existing customizations like `ActorDetails.cpp` for patterns
- Study `SlateFwd.h` for available widgets

### Getting Help

- **Unreal Slackers Discord:** #cpp channel for editor questions
- **UE Forums:** Editor Extensions section
- **Engine Source:** Best reference for how Epic does it

---

## Measuring Success

### Key Metrics

1. **Designer Feedback**
   - "This is much easier to use now"
   - Time saved per asset creation
   - Reduction in data entry errors

2. **Error Reduction**
   - Fewer invalid Data Assets
   - Fewer bug reports related to bad data
   - Faster QA cycles

3. **Workflow Speed**
   - Time to create a new ship: Before vs After
   - Number of steps reduced by custom actions
   - Designer satisfaction scores

### Success Criteria for Phase 1

- [ ] All SpaceshipDataAssets use new custom editor
- [ ] Designers report improved workflow
- [ ] Calculated stats visible without manual calculation
- [ ] Zero crashes related to editor customizations
- [ ] Documentation complete and designers trained

---

## Long-Term Vision

### 6 Months

- ✅ Detail customizations for all major Data Assets
- ✅ Property type customizations for complex structs
- ✅ Asset validators preventing data errors
- ✅ Custom asset actions for common workflows

### 12 Months

- ✅ Advanced editor tools (Faction Relationship Graph, Trade Route Designer)
- ✅ Thumbnail renderers for visual asset identification
- ✅ Performance optimization and polish
- ✅ Comprehensive documentation and training materials

### 18 Months

- ✅ Full-featured content creation pipeline
- ✅ Automated testing for editor tools
- ✅ Community contributions and extensions
- ✅ Best-in-class designer experience

---

## Get Started Now

**Action Items for Next Session:**

1. [ ] Create folder structure: `Source/AdastreaEditor/Public/Customizations/`
2. [ ] Create folder structure: `Source/AdastreaEditor/Private/Customizations/`
3. [ ] Implement `SpaceshipDataAssetDetails.h` (copy code from Step 1 above)
4. [ ] Implement `SpaceshipDataAssetDetails.cpp` (copy code from Step 2 above)
5. [ ] Update `AdastreaEditor.cpp` with registration (copy code from Step 3 above)
6. [ ] Build project (Development Editor)
7. [ ] Test in Unreal Editor with a SpaceshipDataAsset
8. [ ] Iterate based on results

**Time Estimate:** 2-4 hours for first working implementation

**Difficulty:** Intermediate (requires C++ and Slate knowledge)

**Support:** Refer to this guide and Unreal documentation

---

**Ready to Start?** Follow Step 1 above to create your first detail customization!

**Questions?** Review the full analysis document: `ADASTREA_EDITOR_ANALYSIS.md`

**Stuck?** Check the Common Pitfalls section above, or refer to engine source examples.

---

**Document Version:** 1.0  
**Last Updated:** 2025-11-05  
**Next Review:** After Phase 1 Implementation
