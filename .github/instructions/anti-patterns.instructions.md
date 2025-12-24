# Anti-Patterns and Lessons Learned

## Purpose

This document captures critical lessons learned from Adastrea's initial development phase (Oct-Dec 2025) to prevent repeating mistakes. These anti-patterns MUST be avoided in all future development.

---

## 🚨 CRITICAL ANTI-PATTERNS (Never Do This)

### 1. Architecture Before Gameplay

**❌ WRONG APPROACH:**
- Build 22 complete systems before any gameplay
- Write 74K lines of documentation before playable content
- Create 100+ blueprints in isolation
- Design everything "perfectly" upfront

**✅ CORRECT APPROACH:**
- Build ONE core gameplay loop first
- Make it playable in 2 weeks (hardcoded is fine)
- Get playtester feedback early
- Iterate based on actual play, not theory
- Add architecture incrementally as needed

**Why This Matters:**
You can't validate fun without players. All the perfect architecture in the world is worthless if the game isn't enjoyable.

---

### 2. Over-Engineering

**❌ WRONG APPROACH:**
- Expose 90%+ of functions to Blueprints "just in case"
- Create 3 separate C++ modules before 50K lines
- Support multiple build paths simultaneously
- Premature optimization before profiling
- Data Assets with 50+ properties

**✅ CORRECT APPROACH:**
- Expose 10-20% of functions (what designers actually need)
- Start with ONE module, split only when necessary
- One recommended build path only
- Profile first, optimize second
- Data Assets with 10-20 core properties

**Why This Matters:**
Over-engineering adds complexity without value. Keep it simple until proven otherwise.

---

### 3. Documentation Obsession

**❌ WRONG APPROACH:**
- Writing documentation for non-existent features
- 2x as much documentation as code
- Multiple guides for the same task
- Documenting before validating
- 500+ files in root directory
- Creating new summary/status files in root for every change

**✅ CORRECT APPROACH:**
- Document only after feature is tested
- Documentation equals or less than code volume
- One authoritative guide per topic
- Validate instructions work before publishing
- Organized file structure (use `docs/` folder)
- **Update CHANGELOG.md instead of creating new root files**
- Put documentation in appropriate subdirectories

**Why This Matters:**
Outdated documentation is worse than no documentation. Focus on working code first. Root directory clutter makes the project harder to navigate.

---

### 4. Feature Creep Infinity

**❌ WRONG APPROACH:**
- Building 22 systems simultaneously
- "Just one more feature" syndrome
- Scope expansion without validation
- Every system "complete" but untested
- AAA ambition on indie resources

**✅ CORRECT APPROACH:**
- Build ONE system at a time
- Ship and validate before expanding
- Cut features ruthlessly
- Test each system in actual gameplay
- Realistic scope for available resources

**Why This Matters:**
A complete, polished simple game beats an incomplete complex game every time.

---

### 5. Working in Isolation

**❌ WRONG APPROACH:**
- 3 months without player feedback
- Assuming "content will be easy later"
- No external playtesting
- Building for imaginary users
- "Trust me, it'll be fun"

**✅ CORRECT APPROACH:**
- Get feedback every 2 weeks minimum
- Validate assumptions with real players
- External playtesters (not just friends)
- Build for actual user needs
- Data-driven decisions on fun factor

**Why This Matters:**
Your assumptions about fun are probably wrong. Let players guide you.

---

## ⚠️ TECHNICAL ANTI-PATTERNS

### 6. Untested "Complete" Code

**❌ WRONG PATTERN:**
```cpp
// Class compiles = "complete"
class UMarketDataAsset : public UDataAsset
{
    // Looks good, ships it!
};
```

**✅ CORRECT PATTERN:**
```cpp
// Class compiles + tested in gameplay + has tests
class UMarketDataAsset : public UDataAsset
{
    // Properties validated
    // Used in actual trading loop
    // Edge cases handled
    // Performance profiled
};
```

**Why:**
Code that compiles ≠ code that works. Test in actual gameplay before marking "complete".

---

### 7. Suspiciously Low Bug Count

**❌ RED FLAG:**
- 29 TODOs in 33K lines of code
- "Everything works perfectly"
- No known issues list
- Zero integration bugs

**✅ HEALTHY REALITY:**
- 100-200 known issues during development
- Honest bug tracking
- Issue backlog with priorities
- Regular bug hunts

**Why:**
If you're not finding bugs, you're not testing enough. Embrace finding issues early.

---

### 8. Blueprint Exposure Overkill

**❌ WRONG APPROACH:**
```cpp
// Every function exposed
UFUNCTION(BlueprintCallable)
float CalculateInternalThingDesignersNeverNeed();
```

**✅ CORRECT APPROACH:**
```cpp
// Only designer-facing functions exposed
UFUNCTION(BlueprintCallable)
float GetTotalPrice(); // Designers need this

private:
    float CalculateInternalThing(); // Keep internal
```

**Why:**
Too many exposed functions = confusing API. Expose only what designers actually use.

---

### 9. Data Asset Bloat

**❌ WRONG PATTERN:**
```cpp
UCLASS()
class USpaceshipDataAsset : public UDataAsset
{
    // 50+ properties, all required, no validation
    UPROPERTY(EditAnywhere)
    float Property1;
    // ... 49 more
};
```

**✅ CORRECT PATTERN:**
```cpp
UCLASS()
class USpaceshipDataAsset : public UDataAsset
{
    // 10-15 core properties with validation
    UPROPERTY(EditAnywhere, meta=(ClampMin=1, ClampMax=100))
    float CoreStat1;
    
    #if WITH_EDITOR
    virtual void PostEditChangeProperty(...) override
    {
        ValidateProperties();
    }
    #endif
};
```

**Why:**
Smaller Data Assets = easier to configure correctly. Add validation to prevent errors.

---

### 10. Circular Dependency "Fixes"

**❌ WRONG FIX:**
```cpp
// Just remove the dependency and move on
// Problem: Architectural issue not addressed
```

**✅ CORRECT FIX:**
```cpp
// Redesign to eliminate circular dependency
// Use interfaces or event-driven communication
// Fix root cause, not symptoms
```

**Why:**
Quick fixes create technical debt. Fix architecture properly or it'll bite you later.

---

## 📊 METRICS ANTI-PATTERNS

### 11. No Performance Baseline

**❌ WRONG APPROACH:**
- Build everything without profiling
- "We'll optimize later"
- No FPS targets
- No memory budgets
- Hope for the best

**✅ CORRECT APPROACH:**
- Profile early and often
- Set performance budgets upfront
- Measure every build
- Track metrics over time
- Catch issues early

**Why:**
Performance issues discovered late are expensive to fix.

---

### 12. Missing Test Coverage

**❌ WRONG STATE:**
- 5% test coverage
- "Manual testing is enough"
- No regression tests
- Tests that only check scripts run, not correctness

**✅ CORRECT STATE:**
- 60-80% test coverage
- Automated testing in CI
- Regression test suite
- Tests validate actual functionality

**Why:**
Without tests, you're constantly breaking things and not knowing it.

---

## 🎯 PROCESS ANTI-PATTERNS

### 13. Git Repository Chaos

**❌ WRONG STRUCTURE:**
```
/ProjectRoot
├── 500+ files in root
├── README1.md
├── README2.md
├── QUICKSTART_V1.md
├── QUICKSTART_V2.md
├── NEW_SUMMARY.md (created for every change)
└── ... chaos
```

**✅ CORRECT STRUCTURE:**
```
/ProjectRoot
├── README.md (one, authoritative)
├── CHANGELOG.md (update this, don't create new files)
├── docs/
│   ├── development/
│   ├── reference/
│   └── systems/
├── scripts/
├── Source/
└── Content/
```

**Project Standard:**
- **Use CHANGELOG.md for documenting changes** - don't create summary files in root
- New documentation goes in `docs/` subdirectories
- One README per directory maximum
- Keep root directory minimal and organized

**Why:**
Repository chaos = contributor confusion. Keep it organized from day one. The critical review identified "500+ files in root" as a major problem.

---

### 14. Premature Modularization

**❌ WRONG TIMING:**
- Split into 3 modules at 33K lines
- Before content exists
- Creates circular dependency issues

**✅ CORRECT TIMING:**
- Start with ONE module
- Split at 50K+ lines
- Only when clear runtime benefit
- After proving architecture works

**Why:**
Modules add complexity. Only split when benefits outweigh costs.

---

### 15. Build System Fragility

**❌ WRONG APPROACH:**
- Multiple build paths
- Frequent build breaks
- Multiple guides to fix same issue
- `bWarningsAsErrors = false` to hide problems

**✅ CORRECT APPROACH:**
- One recommended build path
- Stable builds
- Single comprehensive build guide
- Fix warnings, don't disable them

**Why:**
Build breaks kill productivity. Invest in stable build infrastructure.

---

## 🎮 GAME DESIGN ANTI-PATTERNS

### 16. Assuming Content Is Easy

**❌ WRONG ASSUMPTION:**
"We have all the systems, content will be quick"

**✅ REALITY:**
- Content takes 2-3x longer than systems
- Requires different skills (art, design, writing)
- Quality content is hard
- Iteration is time-consuming

**Why:**
Many projects fail in content phase. Don't underestimate it.

---

### 17. No Unique Value Proposition

**❌ WRONG APPROACH:**
"We have everything Elite/X4/Star Citizen has"

**✅ CORRECT APPROACH:**
"We do ONE thing better than anyone"

**Why:**
Can't compete with $100M games on breadth. Win on focused depth.

---

### 18. Ignoring Market Validation

**❌ WRONG APPROACH:**
- Build for 3+ months without player feedback
- "We know what players want"
- No market research
- Assume players will come

**✅ CORRECT APPROACH:**
- Get feedback in week 4
- Build what players actually want
- Study competitor reception
- Validate demand exists

**Why:**
You can build the perfect game for no one. Validate market fit early.

---

## 🛡️ PREVENTION CHECKLIST

Before starting any significant feature work, ask:

### Gameplay First
- [ ] Will this make the game more fun?
- [ ] Can players experience this in next playtest?
- [ ] Have we validated this need with players?

### Scope Management
- [ ] Is this MVP-critical or nice-to-have?
- [ ] Can we cut this and still ship?
- [ ] Are we solving real problems or imaginary ones?

### Technical Discipline
- [ ] Will this be tested in actual gameplay?
- [ ] Have we profiled before optimizing?
- [ ] Is this the simplest solution?

### Process Health
- [ ] Are we getting regular player feedback?
- [ ] Is our build stable?
- [ ] Is documentation current?

### Reality Check
- [ ] Would a publisher fund this?
- [ ] Can we ship this in 6 months?
- [ ] Are we building a game or infrastructure?

---

## 📖 LEARNING FROM FAILURES

### What Went Wrong (Oct-Dec 2025)

1. ✅ **Code Quality**: Excellent
2. ✅ **Architecture**: Well-designed
3. ✅ **Documentation**: Comprehensive
4. ❌ **Gameplay**: Non-existent
5. ❌ **Market Validation**: Zero
6. ❌ **Playable Content**: None

### The Core Problem

Built infrastructure for a game that doesn't exist. Like building a car factory before designing the car.

### The Lesson

**Game first, infrastructure second.**

You can't sell code. You can't sell documentation. You can only sell a fun game.

---

## 🎯 CURRENT MANDATE (2025-2026)

### Primary Focus: Trade Simulator MVP

Based on critical review analysis, we are focused on:

**Option A: Space Trading Simulator**
- Core loop: Buy low, sell high, upgrade ship
- Target: 2-3 months to playable demo
- Content needed: 1 ship, 5-10 stations, 10-20 goods
- Hook: Dynamic living economy

### What This Means

**DO BUILD:**
- ✅ Trading UI
- ✅ Basic economy simulation
- ✅ Station docking
- ✅ Ship upgrades
- ✅ Profit/loss tracking

**DON'T BUILD (Yet):**
- ❌ Combat system
- ❌ Exploration mechanics
- ❌ Faction diplomacy (beyond pricing)
- ❌ Crew management
- ❌ Quest system

**Remember:**
One polished system beats ten half-finished systems.

---

## 🚀 MOVING FORWARD

### New Development Process (Mandatory)

1. **Week 1-2**: Design core loop on paper
2. **Week 3-4**: Build hardcoded prototype
3. **Week 4**: GO/NO-GO based on playtester fun
4. **Week 5-8**: Structure + content if GO
5. **Week 9-12**: Polish + demo

### Quality Gates

**Week 4 Gate:**
- [ ] 60%+ playtesters say "this is fun"
- [ ] Core loop is clear
- [ ] No crashes in 10-minute session

**Week 12 Gate:**
- [ ] 75%+ "had fun" rating
- [ ] 50%+ "would buy" interest
- [ ] 30-minute polished demo

**If Gates Not Met:**
Pivot or kill. Don't continue hoping it gets better.

---

## 📚 REQUIRED READING

Before making any significant architectural decisions, read:

1. **CRITICAL_REVIEW_SUMMARY.md** - Current state assessment
2. **CRITICAL_REVIEW_ACTION_PLAN.md** - Recovery roadmap
3. **CRITICAL_REVIEW_TECHNICAL_ISSUES.md** - Technical debt analysis
4. **This file** - Anti-patterns to avoid

**Don't repeat the mistakes documented here.**

---

**Last Updated**: 2025-12-24  
**Based on**: Critical Review Analysis (Oct-Dec 2025)  
**Purpose**: Prevent repeating architectural and process mistakes  
**Mandate**: Focus on trade simulator MVP with player validation
