# Code Review Action Items - Prioritized

**Generated:** 2025-12-23  
**Source:** Comprehensive C++ code review of Adastrea UE 5.6 codebase

---

## 🔴 CRITICAL PRIORITY (Must Fix Before Next Release)

### Issue #1: Missing FactionLogic Implementation
**Severity:** CRITICAL  
**Effort:** 2-3 days  
**Assignee:** TBD

**Description:**
FactionLogic is extensively documented throughout the project but the implementation files are missing. This prevents strategic faction AI from being implemented.

**Files to Create:**
- `Source/Adastrea/AI/FactionLogic.h`
- `Source/Adastrea/AI/FactionLogic.cpp`

**Implementation Guide:**
1. Create `UFactionLogic` class inheriting from `UNPCLogicBase`
2. Follow the pattern established by `UPersonnelLogic`
3. Implement strategic decision-making functions:
   - `DecideDiplomaticAction()`
   - `DecideEconomicAction()`
   - `DecideMilitaryAction()`
4. Add faction-level priorities and behaviors
5. Use BlueprintNativeEvent pattern for extensibility
6. Document with usage examples

**References:**
- `Source/Adastrea/AI/NPCLogicBase.h` - Base class
- `Source/Adastrea/AI/PersonnelLogic.h` - Example implementation
- `.github/instructions/ai-system.instructions.md` - Documentation
- `ARCHITECTURE.md` - System overview

**Acceptance Criteria:**
- [ ] Header and implementation files created
- [ ] Compiles without errors
- [ ] Follows project coding standards
- [ ] All BlueprintNativeEvent functions implemented
- [ ] Documentation comments added
- [ ] Example Blueprint can be created from UFactionLogic

---

## 🟠 HIGH PRIORITY (Should Fix This Sprint)

### Issue #2: UPROPERTY Declarations Without Specifiers
**Severity:** MEDIUM  
**Effort:** 1-2 hours  
**Assignee:** TBD

**Description:**
Several UPROPERTY declarations lack access specifiers, which can lead to unclear intent and potential Blueprint exposure issues.

**Files to Fix:**

#### File 1: `Source/Adastrea/Combat/ProjectilePoolComponent.h`
**Lines:** 89-94

**Current:**
```cpp
UPROPERTY()
TArray<AProjectile*> PooledProjectiles;

UPROPERTY()
TArray<AProjectile*> ActiveProjectiles;
```

**Fix to:**
```cpp
/** Pool of inactive projectiles ready for reuse (GC tracked) */
UPROPERTY(Transient)
TArray<AProjectile*> PooledProjectiles;

/** Currently active projectiles in the world (GC tracked) */
UPROPERTY(Transient)
TArray<AProjectile*> ActiveProjectiles;
```

#### File 2: `Source/Adastrea/Ships/SimpleAIMovementComponent.h`
**Lines:** TBD (grep for `UPROPERTY()`)

**Pattern:**
```cpp
UPROPERTY()
UFloatingPawnMovement* MovementComponent;

UPROPERTY()
AActor* TargetActor;
```

**Fix to:**
```cpp
/** Movement component reference (GC tracked) */
UPROPERTY(Transient)
UFloatingPawnMovement* MovementComponent;

/** Current target actor for AI navigation (GC tracked) */
UPROPERTY(Transient)
AActor* TargetActor;
```

#### File 3: `Source/Adastrea/Ships/Spaceship.h`
**Count:** 6 instances (related to Input Actions)

**Pattern:**
```cpp
UPROPERTY()
UInputAction* MoveForwardAction;
```

**Fix to:**
```cpp
/** Input action for forward movement */
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
UInputAction* MoveForwardAction;
```

**Acceptance Criteria:**
- [ ] All 10 UPROPERTY declarations have appropriate specifiers
- [ ] Code compiles without warnings
- [ ] No unintended Blueprint exposure
- [ ] Comments added explaining purpose

---

### Issue #3: Debug Messages in Production Code
**Severity:** LOW  
**Effort:** 30 minutes  
**Assignee:** TBD

**Description:**
Two `GEngine->AddOnScreenDebugMessage` calls in production code should be replaced with proper logging.

**Files to Fix:**

#### File: `Source/Adastrea/AI/PersonnelLogic.cpp`
**Lines:** 217, 309

**Current (Line 217):**
```cpp
if (GEngine)
{
    FColor MessageColor = Success ? FColor::Green : FColor::Yellow;
    GEngine->AddOnScreenDebugMessage(-1, 3.0f, MessageColor,
        FString::Printf(TEXT("%s: Completed %s - %s"),
            PersonnelData ? *PersonnelData->PersonnelName.ToString() : TEXT("Unknown"),
            *UEnum::GetValueAsString(CompletedTask),
            Success ? TEXT("Success") : TEXT("Partial Success")));
}
```

**Fix to:**
```cpp
UE_LOG(LogAdastreaAI, Verbose, TEXT("%s: Completed %s - %s"),
    PersonnelData ? *PersonnelData->PersonnelName.ToString() : TEXT("Unknown"),
    *UEnum::GetValueAsString(CompletedTask),
    Success ? TEXT("Success") : TEXT("Partial Success"));

#if !UE_BUILD_SHIPPING
// Optional: Keep debug message for development builds only
if (GEngine)
{
    FColor MessageColor = Success ? FColor::Green : FColor::Yellow;
    GEngine->AddOnScreenDebugMessage(-1, 3.0f, MessageColor,
        FString::Printf(TEXT("%s: Completed task %s"),
            PersonnelData ? *PersonnelData->PersonnelName.ToString() : TEXT("Unknown"),
            *UEnum::GetValueAsString(CompletedTask)));
}
#endif
```

**Current (Line 309):**
```cpp
if (GEngine)
{
    GEngine->AddOnScreenDebugMessage(-1, 3.0f, 
        ModifiedStress > 0 ? FColor::Orange : FColor::Green,
        FString::Printf(TEXT("%s: Stress %s by %.1f"),
            *PersonnelData->PersonnelName.ToString(),
            ModifiedStress > 0 ? TEXT("increased") : TEXT("decreased"),
            FMath::Abs(ModifiedStress)));
}
```

**Fix to:**
```cpp
UE_LOG(LogAdastreaAI, Verbose, TEXT("%s: Stress %s by %.1f"),
    *PersonnelData->PersonnelName.ToString(),
    ModifiedStress > 0 ? TEXT("increased") : TEXT("decreased"),
    FMath::Abs(ModifiedStress));

#if !UE_BUILD_SHIPPING
// Optional: Keep debug message for development builds only
if (GEngine)
{
    GEngine->AddOnScreenDebugMessage(-1, 3.0f, 
        ModifiedStress > 0 ? FColor::Orange : FColor::Green,
        FString::Printf(TEXT("%s: Stress change %.1f"),
            *PersonnelData->PersonnelName.ToString(),
            ModifiedStress));
}
#endif
```

**Acceptance Criteria:**
- [ ] Both debug messages replaced with UE_LOG
- [ ] Code compiles without errors
- [ ] No debug messages in Shipping builds
- [ ] Optional: Development-only messages gated with `#if !UE_BUILD_SHIPPING`

---

## 🟡 MEDIUM PRIORITY (Next Sprint)

### Issue #4: Review Tick Usage for Performance
**Severity:** MEDIUM  
**Effort:** 4-8 hours  
**Assignee:** TBD

**Description:**
43 files still use Tick implementations. While AI system correctly uses timer-based updates, other components may benefit from similar optimization.

**Known Good:**
- AI system uses `UpdateInterval` and `UpdateAI()` ✓
- Combat components likely need per-frame updates (fine)
- Movement components need smooth interpolation (fine)

**Candidates for Review:**
1. Station module systems (periodic checks)
2. Economy/trading systems (can be event-driven)
3. Sensor systems (can use timers)
4. AI movement components (may benefit from timers)

**Recommended Approach:**
1. Profile game with 100+ NPCs and 10+ stations
2. Identify Tick hotspots using Unreal Insights
3. Convert highest-impact Ticks to timers
4. Test performance improvements
5. Document findings

**Acceptance Criteria:**
- [ ] Performance profiling completed
- [ ] Top 5 Tick hotspots identified
- [ ] At least 3 high-impact systems converted to timers
- [ ] Performance improvements measured and documented
- [ ] No regression in gameplay functionality

---

### Issue #5: Improve Null Safety in PersonnelLogic
**Severity:** LOW  
**Effort:** 1 hour  
**Assignee:** TBD

**Description:**
Some functions in PersonnelLogic access `PersonnelData` without prior null checks.

**Files to Fix:**

#### File: `Source/Adastrea/AI/PersonnelLogic.cpp`

**Locations to Add Checks:**

**Lines 373-377 (MakeFriend):**
```cpp
// Current:
UE_LOG(LogAdastreaAI, Log, TEXT("%s: Made friends with %s"),
    *PersonnelData->PersonnelName.ToString(),  // No null check
    *OtherPersonnelID.ToString());

// Fix to:
if (PersonnelData)
{
    UE_LOG(LogAdastreaAI, Log, TEXT("%s: Made friends with %s"),
        *PersonnelData->PersonnelName.ToString(),
        *OtherPersonnelID.ToString());
}
```

**Lines 391-394 (AddConflict):**
```cpp
// Current:
UE_LOG(LogAdastreaAI, Log, TEXT("%s: Conflict with %s"),
    *PersonnelData->PersonnelName.ToString(),  // No null check
    *OtherPersonnelID.ToString());

// Fix to:
if (PersonnelData)
{
    UE_LOG(LogAdastreaAI, Log, TEXT("%s: Conflict with %s"),
        *PersonnelData->PersonnelName.ToString(),
        *OtherPersonnelID.ToString());
}
```

**Lines 400-403 (ResolveConflict):**
```cpp
// Current:
UE_LOG(LogAdastreaAI, Log, TEXT("%s: Resolved conflict with %s"),
    *PersonnelData->PersonnelName.ToString(),  // No null check
    *OtherPersonnelID.ToString());

// Fix to:
if (PersonnelData)
{
    UE_LOG(LogAdastreaAI, Log, TEXT("%s: Resolved conflict with %s"),
        *PersonnelData->PersonnelName.ToString(),
        *OtherPersonnelID.ToString());
}
```

**Acceptance Criteria:**
- [ ] All PersonnelData accesses have null checks
- [ ] Code compiles without warnings
- [ ] No crashes if PersonnelData is null
- [ ] Log messages handled gracefully

---

## 🟢 LOW PRIORITY (Future Improvements)

### Issue #6: Convert TODO Comments to GitHub Issues
**Severity:** LOW  
**Effort:** 2 hours  
**Assignee:** TBD

**Description:**
27 TODO/FIXME comments exist in the codebase. These should be tracked in GitHub Issues for better visibility.

**Approach:**
1. Find all TODO comments: `grep -r "TODO\|FIXME\|HACK" Source/Adastrea --include="*.h" --include="*.cpp"`
2. Create GitHub Issues for each
3. Categorize by priority (P0, P1, P2)
4. Assign to appropriate team members
5. Add milestone/sprint planning
6. Replace TODO comments with issue references:
   ```cpp
   // TODO: Implement faction relationship modifiers
   // becomes:
   // Issue #123: Implement faction relationship modifiers
   ```

**Acceptance Criteria:**
- [ ] All 27 TODOs cataloged
- [ ] GitHub Issues created with proper labels
- [ ] Issues assigned to appropriate sprints
- [ ] Code comments updated with issue references

---

### Issue #7: Add Unit Tests for Core Systems
**Severity:** LOW  
**Effort:** 1-2 weeks  
**Assignee:** TBD

**Description:**
Add unit tests for critical gameplay systems to catch regressions early.

**Priority Systems to Test:**
1. Data Asset validation (SpaceshipDataAsset, PersonnelDataAsset, FactionDataAsset)
2. Combat system (damage calculation, armor penetration, shield facings)
3. AI system (priority evaluation, decision making, personality modifiers)
4. Interface implementations (IDamageable, ITargetable, IFactionMember)

**Example Test Structure:**
```cpp
// Tests/Combat/CombatHealthComponentTests.cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatHealthArmorTest, 
    "Adastrea.Combat.ArmorReduction", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCombatHealthArmorTest::RunTest(const FString& Parameters)
{
    // Test armor reduction calculation
    float Damage = 100.0f;
    float Armor = 50.0f;
    float Penetration = 25.0f;
    
    float ExpectedDamage = CalculateExpectedDamage(Damage, Armor, Penetration);
    float ActualDamage = TestComponent->ApplyTypedDamage(...);
    
    TestEqual("Armor reduction correct", ActualDamage, ExpectedDamage, 0.01f);
    
    return true;
}
```

**Acceptance Criteria:**
- [ ] Test framework set up
- [ ] 20+ unit tests covering core systems
- [ ] All tests passing
- [ ] CI/CD integration configured
- [ ] Code coverage above 60% for tested systems

---

### Issue #8: Performance Profiling and Optimization
**Severity:** LOW  
**Effort:** 1 week  
**Assignee:** TBD

**Description:**
Comprehensive performance profiling to validate optimization decisions and identify bottlenecks.

**Test Scenarios:**
1. 100 NPCs with active AI (PersonnelLogic)
2. 10 space stations with modules
3. Active combat with 50+ projectiles
4. Complex faction relationships (10+ factions)
5. Large-scale trading operations

**Tools:**
- Unreal Insights
- Visual Studio Profiler
- In-engine stat commands

**Metrics to Capture:**
- Frame time
- AI update cost
- GC pressure
- Memory usage
- Blueprint vs C++ performance

**Acceptance Criteria:**
- [ ] Performance baseline documented
- [ ] Bottlenecks identified and prioritized
- [ ] Optimization plan created
- [ ] Target performance metrics met (60 FPS with 100 NPCs)
- [ ] Results documented for future reference

---

## Summary

| Priority | Count | Total Effort | Status |
|----------|-------|--------------|--------|
| 🔴 Critical | 1 | 2-3 days | Not Started |
| 🟠 High | 3 | 3-4 hours | Not Started |
| 🟡 Medium | 2 | 5-9 hours | Not Started |
| 🟢 Low | 3 | 2-3 weeks | Not Started |
| **TOTAL** | **9** | **~4 weeks** | **0% Complete** |

---

## Recommended Sprint Planning

### Sprint 1 (This Week)
- Issue #1: Create FactionLogic (Critical)
- Issue #2: Fix UPROPERTY specifiers (High)
- Issue #3: Remove debug messages (High)

### Sprint 2 (Next Week)
- Issue #5: Improve null safety (Medium)
- Issue #6: Convert TODOs to issues (Low)
- Start Issue #4: Tick usage review

### Sprint 3 (Future)
- Complete Issue #4: Tick optimization
- Issue #7: Begin unit testing
- Issue #8: Performance profiling

---

**For detailed analysis, see:**
- `CODE_REVIEW_REPORT.md` - Full 700+ line comprehensive review
- `CODE_REVIEW_SUMMARY.md` - Executive summary and grades

**Questions?** Contact the code review team or create a GitHub Discussion.
