# Adastrea - Critical Review Action Plan

**Date**: December 24, 2025 (Created)  
**Updated**: December 29, 2025 (Week 1 in Progress)  
**Status**: Active - Trade Simulator MVP Phase 1  
**Purpose**: Concrete Steps to Recovery  

---

## 🎯 EXECUTIVE SUMMARY

This document provides **specific, actionable steps** to transform Adastrea from an over-engineered code project into a fundable, shippable game.

**Current State**: 85% code, 10% playable content, 0% market validation  
**Target State**: 40% focused code, 80% polished content, validated gameplay  
**Timeline**: 6 months to fundable demo  
**Investment Required**: $50,000

---

## 📅 6-MONTH RECOVERY PLAN

> **STATUS UPDATE (Dec 29, 2025)**: Phase 1 Week 1 in progress. Scope defined, analysis complete, hardcoded prototype development starting.

### Phase 1: Reality Check & Scope Cut (Weeks 1-2)

#### Week 1: Accept Reality ✅ COMPLETE (Dec 24-28)

**Day 1-2: Team Meeting** ✅
- ✅ Read critical review documents
- ✅ Accepted current approach isn't working
- ✅ Committed to Trade Simulator MVP

**Day 3-5: Scope Definition** ✅
- ✅ Picked **Option A**: Space Trading Simulator (Merchant focus)
- ✅ Documented in `.github/instructions/trade-simulator-mvp.instructions.md`
- ✅ Team/solo developer aligned on 12-week plan

**Day 6-7: Create Vision Document** ✅
- ✅ 2-page MVP vision created (see trade-simulator-mvp.instructions.md)
- ✅ Core loop defined: Buy low, sell high, upgrade ship, repeat
- ✅ Success metric: 75% "fun" rating, 50% "would buy" interest

**Deliverable**: ✅ 2-page MVP vision document with ONE core loop defined

#### Week 2: Ruthless Deletion ✅ COMPLETE (Dec 28-29)

**Delete/Archive**: ✅ ANALYSIS COMPLETE
- ✅ Phase 2.1: Analyzed 186 functions → identified 64-75 MVP-critical
- ✅ Trading cleanup: Removed 12 non-MVP functions (Dec 27)
- ✅ Systems categorized: Keep (MVP), Defer (Post-MVP), Remove (bloat)
- ✅ Anti-patterns documented for future prevention
- ⏳ Physical archiving: Will be done incrementally during refactor

**Keep Only**: ✅ IDENTIFIED
- ✅ Core flight system (Spaceship: 5-6 functions)
- ✅ Trading system (PlayerTraderComponent, EconomyManager, CargoComponent)
- ✅ Essential UI (HUD + trading screen)
- ✅ Input system (basic controls)
- ⏳ Save/load (minimal - defer to Week 9+)

**Deliverable**: ✅ Reduced scope defined, MVP-critical code identified

---

### Phase 2: Hardcoded Prototype (Weeks 3-4)

> **STATUS**: Week 3 starting (Dec 29, 2025). This is the CURRENT PHASE.

#### Week 3: Build Something Playable 🔄 IN PROGRESS (Dec 29 - Jan 4)

**Goal**: 10-minute hardcoded experience that's playable start-to-finish

**Hardcode Everything**:
```cpp
// Don't create Data Assets yet, just hardcode
void ASpaceship::BeginPlay()
{
    // Hardcoded ship stats
    MaxSpeed = 1000.0f;
    Acceleration = 500.0f;
    TurnRate = 90.0f;
    
    // This is TEMPORARY and that's OK
}
```

**Minimal Content Checklist**:
- [ ] One ship that flies smoothly (hardcoded stats) ← **IN PROGRESS**
- [ ] One sector with 5-10 objects (static spawned) ← **IN PROGRESS**
- [ ] One gameplay interaction (dock at station) ← **PLANNED**
- [ ] Basic HUD showing relevant info ← **PLANNED**
- [ ] Input that works (keyboard/mouse) ← **PLANNED**
- [ ] No crashes for 10 minutes ← **TESTING PHASE**

**Current Week Goals** (Dec 29 - Jan 4):
- Set up BP_SimpleTradingShip with basic flight
- Create 2 station actors (Agricultural, Industrial)
- Implement simple docking proximity check
- Hardcode 3 trade goods (Water, Food, Fuel)

**Deliverable**: Ugly but playable 10-minute prototype (by end of Week 3)

#### Week 4: Internal Playtesting ⏳ PLANNED (Jan 5-11)

**Test With 3-5 People**:
- Friend/colleague tests (strangers better)
- Give them NO instructions
- Watch them play silently
- Record their struggles
- Ask: "Is this fun? Would you play more?"

**Key Questions**:
1. Can they figure out controls in 2 minutes?
2. Do they understand the goal?
3. Do they have fun?
4. Do they want to keep playing?
5. Would they recommend it?

**Iterate Based on Feedback**:
- Fix critical confusion points
- Add missing feedback
- Improve core feel
- Make it 20% more fun

**Deliverable**: Validated prototype, feedback documented

**GO/NO-GO DECISION**: 
- ✅ If people have fun: Continue to Phase 3
- ❌ If people don't have fun: Pivot or kill project

---

### Phase 3: Content Creation (Weeks 5-12)

#### Weeks 5-6: Core Content

**If Hardcoded Prototype Was Fun, Now Add Structure**:

**Convert Hardcoded → Data Assets**:
- Create 3 ship Data Assets (small, medium, large)
- Create 1 faction Data Asset
- Create 5-10 station/object Data Assets
- Create 1 sector configuration

**Build Blueprints**:
- BP_Spaceship (references Data Asset)
- BP_Station (or BP_Target, or BP_Planet)
- BP_GameMode (spawn logic)
- BP_PlayerController (input routing)

**Basic UI**:
- WBP_HUD_Main (extend AdastreaHUDWidget)
- One gameplay screen (trading, OR targeting, OR scanning)

**Deliverable**: Structured version of hardcoded prototype

#### Weeks 7-8: Content Expansion

**Add Variety**:
- 3 more ships (6 total)
- 5 more locations (10-15 total)
- 3 variations of core loop
- Basic progression (if applicable)

**Polish Core Loop**:
- VFX for key actions
- Sound effects (basic)
- UI feedback improvements
- Control refinement

**Deliverable**: 20-minute experience with variety

#### Weeks 9-10: Systems Integration

**Add Supporting Systems**:
- Save/Load (basic)
- Progression (if needed)
- Tutorial/Instructions
- Settings/Options

**Test Intensively**:
- 20+ external playtesters
- Fix all critical bugs
- Balance difficulty
- Improve clarity

**Deliverable**: Polished 30-minute demo

#### Weeks 11-12: Demo Package

**Create Pitch Materials**:
1. **Gameplay Video** (2-3 minutes)
   - Show core loop
   - Highlight unique hook
   - Show polish
   - End with "Coming Soon"

2. **Pitch Deck** (10 slides)
   - Problem/Opportunity
   - Solution (your game)
   - Market size
   - Competition
   - Unique advantage
   - Traction (playtester feedback)
   - Team
   - Timeline
   - Budget
   - Ask

3. **Playable Demo** (30 minutes)
   - Polished experience
   - No crashes
   - Clear instructions
   - Fun gameplay

**Deliverable**: Complete pitch package

---

### Phase 4: Funding Pursuit (Weeks 13-24)

#### Weeks 13-16: Publisher Outreach

**Target Publishers**:
- Focus on indie-friendly publishers
- Those with similar games in portfolio
- Realistic tier (not AAA, not micro)

**Approach**:
1. Cold email with video link
2. Follow up with demo if interested
3. Pitch meeting if requested
4. Negotiate if offer extended

**Parallel Activities**:
- Continue dev (add content)
- Build community (Discord, dev blog)
- Social media presence
- Consider Kickstarter/crowdfunding

#### Weeks 17-24: Negotiation or Pivot

**If Funded**: 
- Hire team
- Expand content
- Build toward 1.0

**If Not Funded**:
- Crowdfunding campaign
- Early Access release
- Open source pivot
- Scale down further

---

## 🎮 RECOMMENDED MVP SCOPE (Pick One)

### Option A: Space Trading Simulator

**Core Loop**:
1. Buy cargo at station
2. Fly to another station
3. Sell cargo at profit
4. Upgrade ship
5. Repeat with better margins

**Unique Hook**: Dynamic economy that feels alive

**Content Needed**:
- 1 flyable ship (expandable to 3)
- 5-10 stations
- 10-20 trade goods
- Basic economy simulation
- Trading UI

**Timeline**: 2-3 months  
**Comparable**: Space Rangers trading loop

---

### Option B: Tactical Space Combat

**Core Loop**:
1. Enter combat arena
2. Fight enemy ships
3. Use tactics to win
4. Earn credits/XP
5. Upgrade loadout
6. Harder challenges

**Unique Hook**: Tactical depth in combat

**Content Needed**:
- 2-3 flyable ships
- 5-10 enemy variants
- Combat arena map
- Weapon systems
- Upgrade system

**Timeline**: 2-3 months  
**Comparable**: House of the Dying Sun

---

### Option C: Exploration & Discovery

**Core Loop**:
1. Scan for anomalies
2. Investigate discoveries
3. Catalog findings
4. Unlock story/lore
5. Upgrade scanner
6. Find bigger mysteries

**Unique Hook**: Compelling narrative in discoveries

**Content Needed**:
- 1 exploration ship
- 10-20 discoverable objects
- Scanning mechanics
- Lore/story system
- Progress tracking

**Timeline**: 3-4 months  
**Comparable**: Outer Wilds (simplified)

---

## 💰 BUDGET ALLOCATION

### Total Budget: $50,000 for 6 Months

| Category | Amount | Purpose |
|----------|--------|---------|
| **Content Creator** | $25,000 | 3 months full-time: Blueprints, Data Assets, level design |
| **UI/UX Designer** | $12,000 | 2 months part-time: UI design and implementation |
| **Audio** | $3,000 | Contract: Sound effects and basic music |
| **Art/Models** | $5,000 | Contract: Essential 3D models and textures |
| **Marketing** | $3,000 | Trailer production, pitch deck, website |
| **Misc/Buffer** | $2,000 | Unexpected costs, tools, software |

### Alternative: $0 Budget Solo Dev

**If No Funding Available**:
- Solo developer continues
- Use free assets (Quixel, Sketchfab CC)
- Use free tools (Blender, GIMP)
- Community help (Reddit, Discord)
- Timeline: 12-18 months
- Quality: Lower but shippable

---

## 📊 SUCCESS METRICS

### Week 4 (Prototype) Success Criteria

- [ ] 80% of playtesters finish 10-minute experience
- [ ] 60% say they had fun
- [ ] 40% want to play more
- [ ] 0 critical bugs/crashes
- [ ] Core loop is clear to players

**If Not Met**: Pivot or kill

### Week 12 (Demo) Success Criteria

- [ ] 30-minute polished demo
- [ ] 90% completion rate
- [ ] 75% "had fun" rating
- [ ] 50% "would buy" interest
- [ ] <5 known bugs
- [ ] Trailer gets positive responses

**If Not Met**: Extend polish phase

### Week 24 (Funding) Success Criteria

- [ ] 5+ publisher meetings
- [ ] 1+ funding offer
- [ ] OR successful crowdfunding
- [ ] OR viable early access path

**If Not Met**: Reevaluate or pivot

---

## 🚨 RISK MITIGATION

### Risk 1: Can't Cut Scope (Emotional Attachment)

**Mitigation**:
- Archive, don't delete
- Frame as "future DLC"
- Focus on "version 1.0" not "full vision"
- Remember: Ship v1.0, iterate to v2.0

### Risk 2: Prototype Isn't Fun

**Mitigation**:
- Playtest EARLY (week 4)
- Pivot quickly if needed
- Try alternative core loops
- Get brutal honest feedback

### Risk 3: Solo Developer Burnout

**Mitigation**:
- Set realistic daily hours (6-8, not 16)
- Take weekends off
- Use contractors for specialties
- Find accountability partner
- Join game dev community

### Risk 4: Runs Out of Money

**Mitigation**:
- Part-time dev + day job
- Use only free assets
- Extend timeline
- Seek grants/competitions
- Consider open source pivot

### Risk 5: Can't Find Funding

**Mitigation**:
- Early Access on Steam
- Crowdfunding (Kickstarter)
- Itch.io pay-what-you-want
- Free/ad-supported model
- Open source + Patreon

---

## ✅ WEEKLY CHECKLIST

### Every Week:

- [ ] Ship something playable (even if small)
- [ ] Get external feedback
- [ ] Fix top 3 bugs/issues
- [ ] Update one-page status doc
- [ ] Share progress (blog/social)
- [ ] Reflect: still fun? still achievable?

### Every Month:

- [ ] Major milestone review
- [ ] Update roadmap
- [ ] Financial check-in
- [ ] Mental health check
- [ ] Celebrate wins

---

## 🎯 THE ONE-PAGE PLAN

If you read nothing else, do this:

**Weeks 1-2**: Pick one core loop, document it, cut everything else

**Weeks 3-4**: Hardcode prototype, playtest, iterate

**Week 4**: GO/NO-GO decision based on fun factor

**Weeks 5-12**: Build structured demo around validated prototype

**Week 12**: Pitch-ready demo package

**Weeks 13-24**: Seek funding or launch early access

**Success = Something people want to play**

---

## 💬 FINAL WORDS

### The Hard Truth

You've spent 3 months building infrastructure for a game that doesn't exist. That's a sunk cost. Don't throw good time after bad.

### The Good News

You have strong technical skills. You CAN build a great game. You just need to:
1. Cut scope by 80%
2. Focus on fun, not features
3. Ship something playable NOW
4. Iterate based on feedback

### The Choice

**Path A**: Continue current trajectory → 0% chance of success

**Path B**: Follow this plan → 30-50% chance of success

**Path C**: Give up → 100% chance of failure

### The Ask

Give yourself 6 months following this plan. If it doesn't work, at least you tried a different approach. If it does work, you'll have a fundable game.

**Stop building the perfect foundation. Start building the fun game.**

---

**Next Step**: Read this plan, pick an option, start Week 1 Day 1 tomorrow.

Good luck. 🚀
