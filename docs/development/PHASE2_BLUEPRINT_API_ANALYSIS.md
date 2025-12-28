# Phase 2: Blueprint API Analysis Report

**Generated**: 2025-12-28  
**Status**: ✅ Complete (All MVP Systems)  
**Scope**: MVP-Focused (Trade Simulator)

---

## Executive Summary

Current codebase has **1,014 BlueprintCallable functions** across all systems. Phase 2 target is ~200 functions (80% reduction).

**MVP-Focused Strategy**: Prioritize Trading, Ships, and Stations systems. Defer Combat, AI, and other non-MVP systems until after MVP validation.

---

## System Breakdown

### MVP-Critical Systems (Prioritize for Phase 2)

| System | Functions | Status | Priority |
|--------|-----------|--------|----------|
| **Trading** | 70 | Review needed | HIGH |
| **Ships** | 106 | Review needed | HIGH |
| **Stations** | 10 | Review needed | HIGH |
| **UI** | 214 | Review needed | MEDIUM |

**Total MVP-Critical**: 400 functions

### Non-MVP Systems (Defer)

| System | Functions | Status | Priority |
|--------|-----------|--------|----------|
| Combat | 87 | Defer | LOW |
| AI | 36 | Defer | LOW |
| Factions | 3 | Defer | LOW |
| Characters/Personnel | 21 | Defer | LOW |
| Other | 453 | Defer | LOW |

**Total Non-MVP**: 600 functions

---

## Phase 2.1: Blueprint API Audit (MVP Focus)

### Step 1: ✅ Generate Report (Complete)

Generated full Blueprint API report with 1,014 functions identified.

### Step 2: ✅ Categorize Functions (Complete - 2025-12-28)

#### All MVP Systems Analyzed ✅

**Trading System (70 functions)** - ✅ COMPLETE
- Created: PHASE2_TRADING_SYSTEM_CATEGORIZATION.md
- Result: 70 → 34-38 functions (46% reduction)
- Status: Ready for implementation

**Ships System (106 functions)** - ✅ COMPLETE
- Created: PHASE2_SHIPS_SYSTEM_CATEGORIZATION.md
- Result: 106 → 25-30 functions (72% reduction)
- Status: Ready for implementation

**Stations System (10 functions)** - ✅ COMPLETE
- Created: PHASE2_STATIONS_SYSTEM_CATEGORIZATION.md
- Result: 10 → 5-7 functions (30% reduction)
- Status: Ready for implementation

**Total MVP Reduction**: 186 → 64-75 functions (60-66% reduction) ✅

#### UI System (214 functions)

**Review Needed**: Large surface area. Likely includes many widget helpers and internal functions that don't need Blueprint exposure.

---

## Phase 2.2: Property Modifier Audit (MVP Focus)

### Current State

**418 EditAnywhere+BlueprintReadWrite properties** across all systems.

### Target

Reduce to ~100 properties (76% reduction).

### Strategy

1. Identify runtime-mutable properties (keep BlueprintReadWrite)
2. Convert config-only properties to BlueprintReadOnly
3. Add UFUNCTION setters for controlled modifications

### Systems to Audit (MVP Priority)

1. Trading system properties
2. Ships system properties
3. Stations system properties
4. UI system properties

---

## Phase 2.3: TWeakObjectPtr for Optional References

### Strategy

1. Identify actor references that may be destroyed (targets, cached actors)
2. Convert to `TWeakObjectPtr<AActor>`
3. Add validity checks before use

### Files to Review

Focus on MVP-critical systems:
- Trading components (cached station references?)
- Ship components (target references?)
- UI components (cached actor references?)

---

## Migration Plan

### Week 1: Trading System Refactoring

**Goals**:
- Reduce Trading system from 70 to ~15-20 functions
- Focus on designer-facing trade operations
- Remove internal helpers and analytics

**Steps**:
1. Review each Trading file
2. Categorize: Keep / Remove / Make Private
3. Create migration guide for affected Blueprints
4. Implement changes incrementally
5. Test trading gameplay loop

### Week 2: Ships & Stations Refactoring

**Goals**:
- Reduce Ships from 106 to ~25-30 functions
- Reduce Stations from 10 to ~5-7 functions
- Focus on MVP gameplay needs

**Steps**:
1. Audit ship functions for MVP relevance
2. Keep essential: Get cargo capacity, Get current cargo, Docking status
3. Remove: Internal calculations, debug helpers, non-MVP features
4. Test ship trading workflow

### Week 3-4: UI & Polish (If Time Permits)

**Goals**:
- Reduce UI from 214 to ~50 functions
- Focus on trade UI essentials
- Polish and documentation

---

## Success Metrics

### Phase 2 Targets

- [ ] Reduce to ~200 total BlueprintCallable functions (currently 1,014)
- [ ] Reduce to ~100 BlueprintReadWrite properties (currently 418)
- [ ] All MVP gameplay loops functional
- [ ] No broken Blueprint references
- [ ] Migration guide complete

### MVP-Specific Targets

- [ ] Trading system: 70 → 15-20 functions
- [ ] Ships system: 106 → 25-30 functions
- [ ] Stations system: 10 → 5-7 functions
- [ ] UI system: 214 → 50 functions (if time)

---

## Risk Mitigation

### Potential Issues

1. **Breaking existing Blueprints**: Create migration guide before changes
2. **Designer workflow disruption**: Focus on internal functions first
3. **MVP timeline impact**: Prioritize ruthlessly, defer non-MVP work

### Safety Measures

1. Work in feature branch
2. Incremental commits with clear messages
3. Test trading loop after each change
4. Document all changes in migration guide
5. Keep changes reversible

---

## Next Steps

1. ✅ Generate Blueprint API report (DONE)
2. ✅ Create detailed function categorization (DONE - 2025-12-28)
3. 🔄 Complete migration guide (Trading done, Ships/Stations in progress)
4. ⏳ Start implementation (Week 1-2)
5. ⏳ Test and validate changes (Week 2-3)

---

## Notes

- **Anti-Pattern Warning**: Phase 2 involves significant refactoring. Must balance with MVP timeline.
- **Prioritization**: Trade gameplay loop is #1 priority. Don't break it.
- **Deferral Strategy**: Non-MVP systems (Combat, AI, Factions) explicitly deferred until post-MVP.
- **Testing**: Test trading gameplay after each change. No broken game loops.

---

**Last Updated**: 2025-12-28  
**Owner**: Development Team  
**Status**: ✅ All MVP Systems Categorized, Ready for Implementation
