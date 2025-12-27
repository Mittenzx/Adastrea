# UE5 Best Practices Implementation Checklist

**Status**: Ready for Team Review  
**Priority**: High  
**Estimated Timeline**: 8-12 weeks

This document provides a step-by-step implementation plan for addressing the non-standard UE5 practices identified in [NON_STANDARD_UE5_PRACTICES.md](../development/NON_STANDARD_UE5_PRACTICES.md).

---

## Phase 1: Critical Fixes (Weeks 1-2)

**Goal**: Fix garbage collection safety issues  
**Risk**: Low (adds safety without breaking changes)  
**Effort**: Medium

### 1.1 Audit and Fix Missing UPROPERTY Macros

- [ ] **Step 1**: Run automated check
  ```bash
  python Tools/check_uproperty.py
  ```
  
- [ ] **Step 2**: Review output and identify all UObject* without UPROPERTY()

- [ ] **Step 3**: Add UPROPERTY() to identified pointers
  - Start with critical files: Combat/, AI/, Ships/
  - Pattern: Add `UPROPERTY()` above each UObject* declaration
  - Even private pointers MUST have UPROPERTY()

- [ ] **Step 4**: Test compilation
  ```bash
  # Build in Development Editor mode
  # Ensure no new warnings or errors
  ```

- [ ] **Step 5**: Add CI validation
  - Update `.github/workflows/` to run check_uproperty.py
  - Fail PR if violations found

- [ ] **Step 6**: Update documentation
  - Add rule to CODE_STYLE.md
  - Add to `.github/copilot-instructions.md`

**Acceptance Criteria**:
- ✅ All UObject* pointers have UPROPERTY()
- ✅ CI check passes
- ✅ Documentation updated
- ✅ No compilation errors

---

## Phase 2: High Priority Refactoring (Weeks 3-4)

**Goal**: Reduce API complexity and improve safety  
**Risk**: Medium (changes public API)  
**Effort**: High

### 2.1 Blueprint API Audit

- [ ] **Step 1**: Generate Blueprint API report
  ```bash
  grep -r "BlueprintCallable" Source/Adastrea/Public --include="*.h" > blueprint_api.txt
  ```

- [ ] **Step 2**: Categorize functions
  - Designer-facing: Keep BlueprintCallable
  - Internal helpers: Remove BlueprintCallable, make private
  - Utilities: Keep but document clearly

- [ ] **Step 3**: Create migration guide
  - Document which functions are being changed
  - Provide alternatives where needed
  - Notify content team

- [ ] **Step 4**: Implement changes in phases
  - Week 3: Combat and AI systems
  - Week 4: Ships, Stations, Trading systems

- [ ] **Step 5**: Test Blueprints
  - Verify all existing Blueprints still compile
  - Fix any broken Blueprint references
  - Update Blueprint tutorials

**Target**: Reduce from 1,041 to ~200 BlueprintCallable functions (80% reduction)

### 2.2 Property Modifier Audit

- [ ] **Step 1**: Find all EditAnywhere+BlueprintReadWrite
  ```bash
  grep -r "EditAnywhere.*BlueprintReadWrite" Source/Adastrea/Public --include="*.h" > writable_props.txt
  ```

- [ ] **Step 2**: Review each property
  - Should it be modified at runtime? → Keep BlueprintReadWrite
  - Is it configuration only? → Change to BlueprintReadOnly
  - Create setter functions for controlled modifications

- [ ] **Step 3**: Update properties
  - Pattern: EditAnywhere, BlueprintReadOnly for config
  - Add UFUNCTION setters for valid runtime changes

- [ ] **Step 4**: Update Blueprints
  - Find and fix any Blueprint writes to changed properties
  - Use new setter functions

**Target**: Reduce from 537 to ~100 BlueprintReadWrite properties

### 2.3 Add TWeakObjectPtr for Optional References

- [ ] **Step 1**: Identify optional actor references
  - CurrentTarget, LastTarget, CachedTarget, etc.
  - References to actors that may be destroyed

- [ ] **Step 2**: Convert to TWeakObjectPtr
  ```cpp
  // Before
  UPROPERTY()
  AActor* CurrentTarget;
  
  // After
  UPROPERTY()
  TWeakObjectPtr<AActor> CurrentTarget;
  ```

- [ ] **Step 3**: Add validity checks
  ```cpp
  if (CurrentTarget.IsValid())
  {
      AActor* Target = CurrentTarget.Get();
      // Use Target...
  }
  ```

- [ ] **Step 4**: Test thoroughly
  - Verify no crashes when targets are destroyed
  - Test with aggressive garbage collection

**Files to Update**:
- Combat/WeaponComponent.h
- Combat/TargetingComponent.h
- AI/NPCLogicBase.h
- Others as identified

---

## Phase 3: Medium Priority Modernization (Weeks 5-8)

**Goal**: Modernize to UE5 patterns  
**Risk**: Low (incremental changes)  
**Effort**: High

### 3.1 Migrate to TObjectPtr

**Strategy**: Incremental migration, new code first

- [ ] **Week 5**: Data Assets
  - Update SpaceshipDataAsset
  - Update FactionDataAsset
  - Update WeaponDataAsset
  - Others...

- [ ] **Week 6**: Components
  - Combat components
  - Trading components
  - AI components

- [ ] **Week 7**: Actors
  - Spaceship
  - SpaceStation
  - SpaceStationModule

- [ ] **Week 8**: Remaining files
  - UI widgets
  - Game mode/state
  - Controllers

**Pattern**:
```cpp
// Before
UPROPERTY()
UDataAsset* MyData;

// After
UPROPERTY()
TObjectPtr<UDataAsset> MyData;
```

**Testing**: Ensure no behavioral changes, only type system improvements

### 3.2 Convert Tick to Timer-Based Updates

- [ ] **Step 1**: Profile Tick usage
  - Use Unreal Insights
  - Identify components with low-frequency updates

- [ ] **Step 2**: Convert identified components
  ```cpp
  // Remove Tick, add timer
  void BeginPlay()
  {
      GetWorld()->GetTimerManager().SetTimer(
          UpdateTimer, this, &UMyComponent::Update,
          1.0f, true  // 1 second, looping
      );
  }
  ```

- [ ] **Step 3**: Implement AI update staggering
  ```cpp
  // Random offset to spread CPU load
  float Offset = FMath::RandRange(0.0f, UpdateInterval);
  SetTimer(..., UpdateInterval, true, Offset);
  ```

- [ ] **Step 4**: Profile again
  - Measure CPU time savings
  - Ensure gameplay still feels responsive

**Candidates**:
- AI logic updates
- Health regeneration
- Economy updates
- Distant actor updates

### 3.3 Add const Correctness

- [ ] **Step 1**: Find all BlueprintPure without const
  ```bash
  grep -r "BlueprintPure" Source/Adastrea/Public --include="*.h" | grep -v "const"
  ```

- [ ] **Step 2**: Add const to getters
  ```cpp
  // Before
  float GetMaxHealth();
  
  // After
  float GetMaxHealth() const;
  ```

- [ ] **Step 3**: Update implementations
  - Add const to .cpp definitions
  - Ensure no state modifications

- [ ] **Step 4**: Verify compilation

### 3.4 Standardize ClassGroup

- [ ] **Step 1**: List all components without ClassGroup
  ```bash
  grep -r "class.*: public UActorComponent" Source/Adastrea/Public --include="*.h"
  ```

- [ ] **Step 2**: Add ClassGroup to all
  ```cpp
  UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
  ```

- [ ] **Step 3**: Standardize category names
  - Combat, Trading, AI, Navigation, Exploration, UI, Custom

---

## Phase 4: Ongoing Improvements

**Goal**: Continuous improvement  
**Timeline**: Ongoing

### 4.1 New Code Standards

- [ ] Update `.github/copilot-instructions.md` with new standards
- [ ] Update CODE_STYLE.md with patterns
- [ ] Create code review checklist
- [ ] Add pre-commit hooks for validation

### 4.2 Documentation

- [ ] Keep NON_STANDARD_UE5_PRACTICES.md updated
- [ ] Update UE5_BEST_PRACTICES_QUICK_REF.md
- [ ] Add examples to style guide
- [ ] Record video tutorials for team

### 4.3 Validation

- [ ] Expand automated checking
- [ ] Add more CI validations
- [ ] Schedule quarterly audits
- [ ] Track metrics over time

---

## Testing Strategy

### For Each Phase

1. **Unit Testing**
   - Compile in Development Editor
   - Run automated tests (if available)
   - Check for new warnings

2. **Integration Testing**
   - Load all maps
   - Test gameplay systems
   - Verify Blueprint compilation

3. **Performance Testing**
   - Profile with Unreal Insights
   - Measure GC frequency/duration
   - Check frame rate impact

4. **Validation**
   - Run automated checkers
   - Code review changes
   - Update documentation

---

## Rollback Plan

If issues arise:

1. **Immediate**: Revert specific commits
2. **Branch Protection**: Keep main branch stable
3. **Feature Branches**: Test thoroughly before merge
4. **Incremental**: Small PRs easier to revert

---

## Success Metrics

### Phase 1
- [ ] 100% of UObject* pointers have UPROPERTY()
- [ ] CI validation passing
- [ ] Zero GC-related crashes

### Phase 2
- [ ] 80% reduction in BlueprintCallable functions
- [ ] 80% reduction in BlueprintReadWrite properties
- [ ] TWeakObjectPtr used for all optional references

### Phase 3
- [ ] 50%+ of codebase using TObjectPtr
- [ ] 50% reduction in Tick usage
- [ ] All BlueprintPure functions are const
- [ ] All components have ClassGroup

### Phase 4
- [ ] New code follows all standards
- [ ] Documentation complete and current
- [ ] Team trained on new patterns

---

## Team Responsibilities

### Lead Developer
- Review and approve implementation plan
- Coordinate phases with team
- Monitor metrics

### Developers
- Implement changes in assigned areas
- Test thoroughly
- Update documentation

### Content Team
- Test Blueprint changes
- Report issues
- Update tutorials

### QA
- Verify no regressions
- Performance testing
- Automation support

---

## Communication Plan

### Week 1
- Present plan to team
- Get feedback
- Assign responsibilities

### Weekly
- Status updates
- Blocker resolution
- Metric tracking

### After Each Phase
- Retrospective
- Documentation update
- Next phase kickoff

---

## Resources

### Documentation
- [NON_STANDARD_UE5_PRACTICES.md](../development/NON_STANDARD_UE5_PRACTICES.md) - Full analysis
- [UE5_BEST_PRACTICES_QUICK_REF.md](../reference/UE5_BEST_PRACTICES_QUICK_REF.md) - Quick reference
- [unreal-directive-best-practices.md](../../.github/instructions/unreal-directive-best-practices.md) - Advanced patterns

### Tools
- `Tools/check_uproperty.py` - UPROPERTY validation
- `Tools/check_null_safety.py` - Null safety checks
- `Tools/validate_naming.py` - Naming convention checks

### External
- [Epic C++ Coding Standard](https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine)
- [Unreal Directive](https://unrealdirective.com/)
- [TObjectPtr Documentation](https://docs.unrealengine.com/5.0/en-US/API/Runtime/CoreUObject/UObject/TObjectPtr/)

---

**Status**: Ready for Implementation  
**Next Action**: Team Review and Phase 1 Kickoff  
**Last Updated**: 2025-12-27  
**Version**: 1.0
