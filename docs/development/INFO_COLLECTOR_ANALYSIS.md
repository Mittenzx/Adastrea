# Unreal Engine Info Collector - Analysis & Comparison

**Created**: January 8, 2026  
**Purpose**: Review info collection capabilities and align with Trade Simulator MVP goals  
**Status**: Analysis Document

---

## Executive Summary

This document analyzes information collection capabilities for Unreal Engine projects and compares them with Adastrea's Trade Simulator MVP requirements. The goal is to identify what information we need to collect to effectively build and validate the trading game.

### Key Findings

1. **Current Capabilities**: Adastrea has robust validation and testing scripts, but lacks MVP-focused info collection
2. **MVP Gap**: Need real-time gameplay metrics, economy data, and playtester behavior tracking
3. **Priority**: Info collection should support **Week 4, 8, and 12 validation gates** (60%/70%/75% fun ratings)

---

## 1. What an Info Collector Should Provide

### 1.1 Project Structure Information

**Purpose**: Understanding what exists and what's missing

**Essential Data**:
- C++ class inventory (header/implementation files)
- Blueprint asset count and organization
- Data Asset catalog (ships, stations, trade items)
- Map/Level inventory
- UI Widget inventory
- Content gaps analysis

**Current Adastrea Capability**: ✅ **GOOD**
- `analyze_project.py` - Basic structure analysis
- `AdastreaAssetValidator.py` - Asset validation
- `ContentValidator.py` - Content existence checks

**MVP Relevance**: **HIGH** - Need to know what trading content exists

---

### 1.2 Compilation and Build Status

**Purpose**: Ensure code compiles and runs

**Essential Data**:
- Compilation errors/warnings
- Missing dependencies
- Blueprint compilation status
- Plugin status
- Build configuration

**Current Adastrea Capability**: ✅ **GOOD**
- `SetupCheck.py` - Environment validation
- `check_uproperty.py` - GC tracking validation
- `check_null_safety.py` - Pointer safety checks
- Build scripts for UE tools

**MVP Relevance**: **MEDIUM** - Basic requirement, already working

---

### 1.3 Runtime Gameplay Metrics

**Purpose**: Validate gameplay is fun and progressing correctly

**Essential Data**:
- Player actions per minute
- Trade completion rate
- Average profit per trade
- Time between trades (should be 3-5 minutes)
- Station visit frequency
- Ship upgrade timing
- Session length
- Quit points (where players give up)

**Current Adastrea Capability**: ❌ **MISSING**
- No runtime metrics collection
- No gameplay telemetry
- No playtester behavior tracking

**MVP Relevance**: **CRITICAL** - Required for Week 4/8/12 validation gates

**Gap**: This is the HIGHEST PRIORITY missing capability

---

### 1.4 Economy Simulation Data

**Purpose**: Validate trading economy is balanced and engaging

**Essential Data**:
- Price fluctuations over time
- Supply/demand balance
- Profitable trade routes
- Average player credits over time
- Inflation/deflation trends
- Station inventory levels
- Trade route efficiency

**Current Adastrea Capability**: ❌ **MISSING**
- No economy monitoring
- No price tracking
- No trade route analysis

**MVP Relevance**: **CRITICAL** - Core gameplay loop validation

**Gap**: Essential for MVP success

---

### 1.5 Performance Profiling

**Purpose**: Ensure 60 FPS target for demo

**Essential Data**:
- Frame rate (FPS)
- Memory usage
- Asset loading times
- Blueprint execution time
- Garbage collection spikes
- Network replication (if applicable)

**Current Adastrea Capability**: ⚠️ **PARTIAL**
- `ScreenshotTester.py` - Visual regression testing
- `SmokeTest.py` - Load testing
- No real-time performance monitoring

**MVP Relevance**: **HIGH** - Week 12 requires 60 FPS

---

### 1.6 Playtester Feedback Collection

**Purpose**: Measure "fun" rating and identify confusion points

**Essential Data**:
- Fun rating (1-10 scale)
- Would play again? (Y/N)
- Would buy? (Y/N)
- Confusion points (free text)
- Time to first trade
- Number of completed trades
- Reasons for quitting

**Current Adastrea Capability**: ❌ **MISSING**
- No feedback collection system
- No survey integration
- Manual feedback only

**MVP Relevance**: **CRITICAL** - Required for GO/NO-GO decisions

---

## 2. Comparison with Trade Simulator MVP Goals

### 2.1 Week 4 Validation Gate (Prototype)

**Required Metrics**:
- [ ] 60%+ playtesters say "fun"
- [ ] Core loop clear to players
- [ ] Can complete trade in 3-5 minutes
- [ ] No critical bugs

**Info Collection Needs**:
1. **Playtester Survey**:
   - Post-play survey with fun rating
   - Confusion tracking
   - Bug reporting

2. **Gameplay Telemetry**:
   - Time to first trade
   - Trades completed
   - Session duration
   - Quit events

3. **Bug Tracking**:
   - Crash reports
   - Error logs
   - Warning counts

**Current Gap**: Need to build survey system and telemetry

---

### 2.2 Week 8 Validation Gate (Structured Version)

**Required Metrics**:
- [ ] 80%+ complete at least 3 trades
- [ ] 70%+ say "had fun"
- [ ] <3 major confusion points
- [ ] 50%+ "would buy" interest

**Info Collection Needs**:
1. **Enhanced Telemetry**:
   - Trade completion rate
   - Per-trade profit
   - Station visit patterns
   - Ship upgrade decisions

2. **Economy Monitoring**:
   - Price stability
   - Trade route profitability
   - Player credit growth curve

3. **Feedback Analysis**:
   - Confusion point frequency
   - Buy intent correlation
   - Fun rating distribution

**Current Gap**: All of the above

---

### 2.3 Week 12 Validation Gate (Demo)

**Required Metrics**:
- [ ] 90%+ complete full 30 minutes
- [ ] 75%+ say "fun"
- [ ] 50%+ "would buy"
- [ ] 60%+ "would play again"
- [ ] 0 crashes
- [ ] <5 known bugs
- [ ] 60 FPS
- [ ] <30 second load time

**Info Collection Needs**:
1. **Complete Telemetry Suite**:
   - All Week 4/8 metrics
   - Performance metrics (FPS, load times)
   - Stability metrics (crashes, errors)

2. **Advanced Analytics**:
   - Player progression curve
   - Retention prediction
   - Fun factor correlation analysis

3. **Publisher Pitch Data**:
   - Aggregate playtester stats
   - Success rate reports
   - Market validation evidence

**Current Gap**: Entire analytics pipeline

---

## 3. Existing Adastrea Information Collection Tools

### 3.1 Asset and Project Analysis

| Script | Purpose | MVP Relevance |
|--------|---------|---------------|
| `analyze_project.py` | Project structure analysis | **Medium** - Inventory check |
| `AdastreaAssetValidator.py` | Asset naming and validation | **Low** - Quality assurance |
| `ContentValidator.py` | Blueprint and content validation | **Medium** - Content completeness |
| `check_uproperty.py` | GC tracking validation | **Low** - Code quality |
| `check_null_safety.py` | Pointer safety checks | **Low** - Code quality |
| `validate_naming.py` | Naming convention checks | **Low** - Code style |

**Verdict**: Good for code quality, not suitable for gameplay validation

---

### 3.2 Testing and Automation

| Script | Purpose | MVP Relevance |
|--------|---------|---------------|
| `SmokeTest.py` | Map loading and error detection | **Medium** - Stability check |
| `ScreenshotTester.py` | Visual regression testing | **Low** - Visual quality |
| `AutomationRunner.py` | Test orchestration | **Medium** - CI/CD |
| `SetupCheck.py` | Environment validation | **Low** - Setup only |

**Verdict**: Good for stability, missing gameplay metrics

---

### 3.3 Content Generation

| Script | Purpose | MVP Relevance |
|--------|---------|---------------|
| `YAMLtoDataAsset.py` | Import YAML to Data Assets | **High** - Content creation |
| `SchemaValidator.py` | YAML schema validation | **Medium** - Data integrity |
| `ProceduralGenerators.py` | Generate ships/NPCs/quests | **Low** - Not for MVP (handcrafted) |
| `ScenePopulator.py` | Populate levels | **Medium** - Test scenario setup |

**Verdict**: Useful for content, but MVP uses hardcoded prototype first

---

## 4. Critical Gaps for MVP Development

### 4.1 **HIGHEST PRIORITY: Gameplay Telemetry System**

**What's Missing**:
- No way to track player actions during play
- No trade completion monitoring
- No time-to-trade tracking
- No quit event logging

**MVP Impact**: **CRITICAL**
- Cannot validate Week 4 GO/NO-GO decision
- Cannot measure "fun" objectively
- Cannot identify friction points

**Recommended Solution**:
Create `UMVPTelemetryComponent` (C++ component with Blueprint-callable functions):
```cpp
class UMVPTelemetryComponent : public UActorComponent
{
    GENERATED_BODY()
    
public:
    // Track events
    UFUNCTION(BlueprintCallable, Category="MVP Telemetry")
    void LogTradeStarted(FName StationID, FName ItemID, int32 Quantity);
    
    UFUNCTION(BlueprintCallable, Category="MVP Telemetry")
    void LogTradeCompleted(float Profit);
    
    UFUNCTION(BlueprintCallable, Category="MVP Telemetry")
    void LogStationVisit(FName StationID);
    
    UFUNCTION(BlueprintCallable, Category="MVP Telemetry")
    void LogQuitEvent(FString Reason);
    
    // Export data
    UFUNCTION(BlueprintCallable, Category="MVP Telemetry")
    void ExportSessionData();
};
```

**Deliverables**:
- C++ telemetry component
- JSON export to `Saved/Telemetry/`
- Python analysis script `analyze_telemetry.py`
- Dashboard visualization

---

### 4.2 **HIGH PRIORITY: Playtester Feedback System**

**What's Missing**:
- No in-game survey
- No fun rating collection
- No confusion tracking

**MVP Impact**: **CRITICAL**
- Primary success metric is "fun rating"
- Need structured feedback for iteration

**Recommended Solution**:
Create `WBP_PlaytestSurvey` widget:
- Post-session survey
- Fun rating (1-10)
- Would play again? (Y/N)
- Would buy? (Y/N)
- Confusion points (text input)
- Export to JSON

---

### 4.3 **HIGH PRIORITY: Economy Monitor**

**What's Missing**:
- No price tracking over time
- No trade route profitability analysis
- No supply/demand visualization

**MVP Impact**: **HIGH**
- Need to validate economy is balanced
- Need to identify best trade routes
- Need to ensure progression feels good

**Recommended Solution**:
Create `UEconomyMonitorSubsystem` (C++ subsystem with Blueprint-callable functions):
```cpp
class UEconomyMonitorSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
    
public:
    // Track prices
    UFUNCTION(BlueprintCallable, Category="Economy Monitor")
    void RecordPrice(FName ItemID, FName StationID, float Price);
    
    // Analyze trade routes
    UFUNCTION(BlueprintCallable, Category="Economy Monitor")
    TArray<FTradeRoute> GetMostProfitableRoutes();
    
    // Export data
    UFUNCTION(BlueprintCallable, Category="Economy Monitor")
    void ExportEconomyData();
};
```

---

### 4.4 **MEDIUM PRIORITY: Performance Monitor**

**What's Missing**:
- Real-time FPS tracking
- Load time measurement
- Memory profiling

**MVP Impact**: **MEDIUM**
- Week 12 requires 60 FPS
- Need to catch performance issues early

**Recommended Solution**:
Use Unreal's built-in profiling:
- `stat fps` - Frame rate
- `stat unit` - Frame time
- `stat memory` - Memory usage
- Create automated performance test with `ScreenshotTester.py` enhancement

---

## 5. Recommended Implementation Plan

### Phase 1: Week 1-2 (Immediate Needs)

**Goal**: Enable Week 4 validation gate

1. **Create MVP Telemetry System**:
   - [ ] C++ component: `UMVPTelemetryComponent`
   - [ ] Blueprint integration
   - [ ] JSON export functionality
   - [ ] Basic Python analyzer

2. **Create Playtester Survey**:
   - [ ] Widget: `WBP_PlaytestSurvey`
   - [ ] Fun rating collection
   - [ ] JSON export
   - [ ] Survey results analyzer

3. **Basic Performance Tracking**:
   - [ ] FPS logger
   - [ ] Load time tracker
   - [ ] Export to CSV

**Deliverable**: Can run Week 4 playtest with data collection

---

### Phase 2: Week 3-6 (Enhanced Tracking)

**Goal**: Enable Week 8 validation gate

1. **Economy Monitoring**:
   - [ ] C++ subsystem: `UEconomyMonitorSubsystem`
   - [ ] Price history tracking
   - [ ] Trade route analysis
   - [ ] Visualization dashboard

2. **Enhanced Telemetry**:
   - [ ] Player progression tracking
   - [ ] Session replay capability
   - [ ] Heatmap generation (station visits)

3. **Automated Analysis**:
   - [ ] `analyze_playtest_data.py` script
   - [ ] Generate success metrics report
   - [ ] Identify confusion points

**Deliverable**: Comprehensive playtest analysis for Week 8

---

### Phase 3: Week 7-12 (Full Analytics)

**Goal**: Enable Week 12 validation gate + publisher pitch

1. **Complete Analytics Suite**:
   - [ ] Retention prediction
   - [ ] Fun factor correlation analysis
   - [ ] Market validation report generator

2. **Publisher Pitch Package**:
   - [ ] Aggregate playtester stats
   - [ ] Success rate visualizations
   - [ ] Demo performance metrics
   - [ ] Market interest validation

**Deliverable**: Data-driven publisher pitch

---

## 6. Integration with Existing Workflow

### 6.1 How to Add Telemetry to Blueprint

**Prototype Phase (Weeks 1-4)**:
```
1. Add UMVPTelemetryComponent to BP_GameMode
2. In trading Blueprint:
   - OnBuyClicked: Call LogTradeStarted
   - OnSellClicked: Call LogTradeCompleted
3. On session end: Call ExportSessionData
```

**Structured Phase (Weeks 5-8)**:
```
1. Integrate with Data Assets
2. Track economy state changes
3. Export hourly economy snapshots
```

---

### 6.2 Playtest Workflow

**Before Playtest**:
1. Clear previous telemetry data
2. Reset economy to baseline
3. Set up screen recording

**During Playtest**:
1. Observer watches (no intervention)
2. Telemetry collects automatically
3. Playtest survey shown at end

**After Playtest**:
1. Run `analyze_playtest_data.py`
2. Generate metrics report
3. Review success criteria
4. Identify issues for next iteration

---

## 7. Comparison with Typical Info Collector

### 7.1 Standard UE Info Collector (General Purpose)

**What it typically provides**:
- Project structure inventory
- Asset catalog
- Class hierarchy
- Blueprint dependency graph
- Compilation status
- Plugin list
- Editor settings

**Use Case**: General development, large teams, complex projects

---

### 7.2 Adastrea MVP Info Collector (Focused)

**What we need instead**:
- ✅ Gameplay telemetry (time-to-trade, completion rate)
- ✅ Economy metrics (prices, routes, profitability)
- ✅ Playtester feedback (fun rating, buy intent)
- ✅ Performance metrics (FPS, load times)
- ✅ Session analytics (quit points, progression)
- ⚠️ Project structure (already have)
- ⚠️ Compilation status (already have)

**Use Case**: MVP validation, early-stage indie, market testing

---

### 7.3 Key Differences

| Feature | Standard Collector | MVP Collector | Priority for Adastrea |
|---------|-------------------|---------------|----------------------|
| Project inventory | ✅ Yes | ⚠️ Nice to have | **Low** (have it) |
| Compilation status | ✅ Yes | ⚠️ Nice to have | **Low** (have it) |
| Blueprint analysis | ✅ Yes | ⚠️ Nice to have | **Low** (have it) |
| **Gameplay telemetry** | ❌ No | ✅ **CRITICAL** | **CRITICAL** |
| **Economy tracking** | ❌ No | ✅ **CRITICAL** | **CRITICAL** |
| **Playtester surveys** | ❌ No | ✅ **CRITICAL** | **CRITICAL** |
| **Performance metrics** | ⚠️ Basic | ✅ **Detailed** | **HIGH** |
| Market validation | ❌ No | ✅ **Yes** | **HIGH** |

**Conclusion**: A general-purpose info collector is **NOT sufficient** for MVP validation. We need custom gameplay-focused telemetry.

---

## 8. Recommendations

### 8.1 Immediate Actions (This Week)

1. **Create Telemetry System** (Priority: **CRITICAL**)
   - Start with simple Blueprint-callable logging
   - Export to JSON
   - Build Python analyzer

2. **Create Survey Widget** (Priority: **CRITICAL**)
   - Fun rating
   - Buy intent
   - Confusion tracking

3. **Document Data Collection Plan** (Priority: **HIGH**)
   - Define all metrics for Week 4/8/12
   - Create telemetry specification
   - Plan data storage format

---

### 8.2 Week 1-2 Goals

- [ ] `UMVPTelemetryComponent` implemented
- [ ] `WBP_PlaytestSurvey` created
- [ ] `analyze_telemetry.py` script working
- [ ] Week 4 playtest data collection ready

---

### 8.3 Long-Term Strategy

**Month 1 (Weeks 1-4)**:
- Basic telemetry + surveys
- Validate prototype is fun

**Month 2 (Weeks 5-8)**:
- Economy monitoring
- Trade route analysis
- Enhanced analytics

**Month 3 (Weeks 9-12)**:
- Complete analytics suite
- Publisher pitch data
- Market validation evidence

---

## 9. Conclusion

### 9.1 Gap Analysis Summary

**What Adastrea Has**:
- ✅ Excellent code validation tools
- ✅ Strong testing infrastructure
- ✅ Good content generation scripts

**What Adastrea Needs for MVP**:
- ❌ Gameplay telemetry system (**CRITICAL**)
- ❌ Playtester feedback collection (**CRITICAL**)
- ❌ Economy monitoring (**HIGH**)
- ❌ Performance profiling (**MEDIUM**)

---

### 9.2 Key Takeaway

**A standard Unreal Engine info collector is NOT what we need.**

We need **gameplay validation tools** that measure:
1. Is it fun? (playtester ratings)
2. Is the economy balanced? (price tracking)
3. Are players completing trades? (telemetry)
4. Does it perform well? (FPS, load times)

**These tools don't exist yet.** They must be built as part of MVP development.

---

### 9.3 Success Criteria

**By Week 4**, we can answer:
- Is the prototype fun? (60%+ yes)
- How long does a trade take? (target: 3-5 minutes)
- Where do players get confused? (identify friction)

**By Week 8**, we can answer:
- Is progression rewarding? (credit growth curve)
- Are trade routes balanced? (profitability analysis)
- Would players buy this? (50%+ intent)

**By Week 12**, we can answer:
- Is this ready to pitch? (all metrics green)
- What's the market validation? (playtester aggregate)
- What's the retention prediction? (would play again %)

---

## Appendix A: Related Documents

**MVP Planning**:
- `.github/instructions/trade-simulator-mvp.instructions.md` - Complete MVP specification
- `CRITICAL_REVIEW_ACTION_PLAN.md` - 12-week recovery plan
- `.github/instructions/anti-patterns.instructions.md` - Lessons learned

**Current Tools**:
- `Tools/analyze_project.py` - Project structure analysis
- `Tools/SmokeTest.py` - Stability testing
- `Tools/ContentValidator.py` - Content validation

**Validation Gates**:
- Week 4: Prototype playtest (60% fun)
- Week 8: Structured playtest (70% fun)
- Week 12: Demo validation (75% fun, 50% buy intent)

---

## Appendix B: Quick Action Checklist

**For Developers**:
- [ ] Read Trade Simulator MVP instructions
- [ ] Understand Week 4/8/12 validation gates
- [ ] Prioritize telemetry system development
- [ ] Create playtester survey widget
- [ ] Build economy monitoring tools

**For Playtesters**:
- [ ] Complete post-session survey honestly
- [ ] Report confusion points specifically
- [ ] Track how long each trade takes
- [ ] Note when game stops being fun

**For Project Managers**:
- [ ] Schedule Week 4/8/12 playtests
- [ ] Recruit external playtesters (not team)
- [ ] Prepare GO/NO-GO decision criteria
- [ ] Plan pivot strategy if metrics fail

---

**Last Updated**: January 8, 2026  
**Next Review**: Week 4 playtest (early February 2026)  
**Document Owner**: Development Team
