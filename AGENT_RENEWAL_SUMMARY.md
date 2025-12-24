# Agent Renewal Summary (Dec 24, 2025)

## What Changed

Based on comprehensive critical review analysis (CRITICAL_REVIEW_*.md files), all agent instructions have been renewed to reflect the project's pivot to a **Trade Simulator MVP** approach.

## New Instruction Files Created

### 1. `.github/instructions/trade-simulator-mvp.instructions.md` (16KB)
**Primary development mandate for Dec 2025 - Mar 2026**

Key Content:
- 12-week development roadmap (Weeks 1-4: Prototype, 5-8: Structure, 9-12: Polish)
- MVP scope definition (trading loop only)
- Explicitly out-of-scope features (combat, exploration, quests, crew)
- Success metrics and GO/NO-GO gates
- Technical guidelines for rapid iteration
- Content requirements (1 ship, 5-10 stations, 10-20 goods)
- Economy simulation basics
- 30-minute demo structure

**Purpose**: Ensure all development focuses on validating trading gameplay is fun.

---

### 2. `.github/instructions/anti-patterns.instructions.md` (12KB)
**Critical lessons learned from Oct-Dec 2025 mistakes**

Key Content:
- 18 documented anti-patterns to NEVER repeat
- Architecture before gameplay (❌)
- Over-engineering (❌)
- Documentation obsession (❌)
- Feature creep (❌)
- Working in isolation (❌)
- Untested "complete" code (❌)
- Prevention checklist and decision framework

**Purpose**: Prevent repeating the mistakes that led to 33K lines of code with 0 minutes of gameplay.

---

## Updated Files

### 3. `.github/copilot-instructions.md`
**Main project instructions updated**

Changes:
- Added critical section at top emphasizing trade simulator MVP
- Linked to new instruction files
- Added current mandate and out-of-scope features
- Emphasized gameplay-first philosophy change

---

### 4. `.github/agents/adastrea-developer.agent.md`
**Developer agent completely reoriented**

Changes:
- Added critical MVP mandate section at top
- Listed explicitly out-of-scope features
- Added anti-patterns section with decision framework
- Marked existing systems as "use", "defer", or "simplify" for MVP
- Updated "Your Role" section to reflect new priorities
- Reframed "Current Priorities" around 12-week roadmap

**Key Mindset Change**:
- OLD: Build perfect architecture for full-scope game
- NEW: Build minimal, fun trading game that validates market

---

### 5. `.github/agents/unreal-mcp.agent.md`
**MCP automation agent refocused**

Changes:
- Added MVP focus section
- Created trade-simulator specific automation workflows
- Trading station setup automation
- Trade item batch creation
- Economy test scenarios
- Ship progression setup
- Trading UI prototyping examples

**Key Principle**: Automate MVP tasks, avoid complexity not needed for trading validation.

---

### 6. `.github/instructions/README.md`
**Instruction directory guide updated**

Changes:
- Reorganized with "START HERE" section
- Prioritized trade simulator MVP and anti-patterns files
- Added reading order for new contributors
- Added context about the pivot
- Updated file purpose table with MVP relevance

---

## The Core Problem We're Solving

**What Happened (Oct-Dec 2025)**:
- Built 33,000+ lines of excellent C++ code
- Created 22 "complete" systems
- Wrote 74,127 lines of documentation
- **Result**: 0 minutes of playable gameplay
- **Assessment**: Not fundable, no market validation

**Why**:
1. Architecture before gameplay
2. Feature creep (22 systems before validating one)
3. Over-engineering (exposed 90%+ functions to Blueprints)
4. No player feedback (3 months in isolation)
5. Documentation obsession (2x code volume)
6. Assumed content would be "easy later"

**The Pivot**:
Focus on ONE core gameplay loop (trading), validate it's fun, then expand.

---

## New Development Philosophy

### Before (❌ Oct-Dec 2025)
1. Build all systems to "completion"
2. Document everything comprehensively
3. Perfect architecture first
4. Assume players will like it
5. Work in isolation

### After (✅ Dec 2025+)
1. Build ONE playable loop
2. Get player feedback every 2-4 weeks
3. Hardcode prototype, structure after validation
4. Test and iterate based on data
5. Ship playable content weekly

---

## 12-Week Trade Simulator Roadmap

### Weeks 1-2: Reality Check & Design
- Accept pivot to trade simulator
- Design core gameplay loop
- Archive non-MVP features

### Weeks 3-4: Hardcoded Prototype
- 10-minute playable experience
- 1 ship, 2 stations, 3 goods (hardcoded)
- **GO/NO-GO GATE**: 60%+ say "fun" or pivot

### Weeks 5-8: Structured Version
- Convert to Data Assets
- 5-10 stations, 10-20 goods
- Ship progression system
- Multiple trade routes

### Weeks 9-12: Polish & Demo
- 30-minute polished demo
- Tutorial and onboarding
- VFX and sound
- **TARGET**: 75% "fun", 50% "would buy"

---

## MVP Scope (What We're Building)

### IN SCOPE ✅
- Trading UI (buy/sell interface)
- Basic economy simulation (supply/demand)
- Station docking and navigation
- Cargo management
- Ship upgrades (cargo capacity progression)
- Profit tracking and display
- 1 playable ship → 3 ships (small, medium, large)
- 5-10 trading stations
- 10-20 trade goods
- Tutorial/onboarding

### OUT OF SCOPE ❌ (Until After MVP Validates)
- Combat system
- Exploration mechanics
- Faction diplomacy (beyond simple pricing)
- Crew management
- Quest system
- Character customization
- Multiplayer/networking
- Advanced AI (beyond basic economy)
- Procedural generation

---

## Success Metrics

### Week 4 Gate (Prototype)
- [ ] 60%+ playtesters say "this is fun"
- [ ] Core loop is clear to players
- [ ] Runs for 10 minutes without crashes
- **If NOT met**: Pivot or redesign core loop

### Week 8 Gate (Structured)
- [ ] 70%+ "had fun" rating
- [ ] 50%+ "would play more"
- [ ] Multiple trade routes working
- **If NOT met**: Extend polish

### Week 12 Gate (Demo)
- [ ] 75%+ "had fun" rating
- [ ] 50%+ "would buy" interest
- [ ] 30-minute polished demo
- [ ] 90%+ completion rate
- **If NOT met**: Additional polish before publisher pitch

---

## Key Takeaways for Agents

### Decision Framework
Before any task, ask:
1. Is this critical for trade simulator MVP? (If NO → defer)
2. Can players test this in next playtest? (If NO → reconsider)
3. Have we validated the need with players? (If NO → validate first)
4. Is this the simplest solution? (If NO → simplify)
5. Will this make the game more fun? (If UNSURE → prototype first)

### Anti-Pattern Checklist
Avoid these mistakes:
- [ ] Building architecture before gameplay
- [ ] Over-engineering solutions
- [ ] Feature creep ("just one more system")
- [ ] Working without player feedback
- [ ] Documenting non-existent features
- [ ] Claiming code is "complete" without gameplay testing

### MVP Mindset
- **Hardcode first**, structure later
- **Test weekly** with real players
- **Ship something playable** every week
- **Cut features ruthlessly**
- **Fun > Features**

---

## Impact on Existing Systems

All existing systems from Oct-Dec 2025 development are good quality but **untested in gameplay**:

**For MVP - Use (Simplified)**:
- ✅ Trading System (core to MVP)
- ✅ Spaceship System (1-3 ships for cargo progression)
- ✅ Station System (docking and trading only)

**For MVP - Minimal Use**:
- ⚠️ Faction System (station ownership and price modifiers only)
- ⚠️ AI System (basic economy simulation only)

**For MVP - Defer Completely**:
- ❌ Combat System (5,734 LOC - good work, but post-MVP)
- ❌ Quest System (1,327 LOC - trading IS the game)
- ❌ Navigation System (865 LOC - basic flight only)
- ❌ Personnel System (crew management - post-MVP)
- ❌ Enhanced Input (use minimal subset)
- ❌ HUD System (trading UI only)
- ❌ Inventory System (cargo only)

**These systems will be revisited post-MVP if trading validates.**

---

## Files Agents Should Read

### CRITICAL (Always Read First)
1. `.github/instructions/trade-simulator-mvp.instructions.md`
2. `.github/instructions/anti-patterns.instructions.md`

### Important Context
3. `CRITICAL_REVIEW_SUMMARY.md` (why we pivoted)
4. `CRITICAL_REVIEW_ACTION_PLAN.md` (12-week plan)
5. `CRITICAL_REVIEW_TECHNICAL_ISSUES.md` (technical debt)

### Technical Reference (As Needed)
6. `.github/instructions/data-assets.instructions.md` (simplified for MVP)
7. `.github/instructions/unreal-directive-best-practices.md` (don't premature optimize)
8. `.github/instructions/ai-system.instructions.md` (minimal for MVP)

### Existing Docs (Pre-Pivot - Use Cautiously)
9. `README.md`, `ARCHITECTURE.md`, `CODE_STYLE.md` (filter through MVP lens)
10. System guides in `Assets/` (reference only, most out of scope)

---

## Communication Guidelines

When agents are asked to add features:

**Trade Simulator Related? →** "Let's build it for MVP!"
- Trading UI improvements
- Economy simulation enhancements
- Cargo management features
- Ship progression mechanics

**Not Trade Simulator Related? →** "Great idea! Let's defer until post-MVP validation."
- Combat features
- Exploration mechanics
- Quest systems
- Crew management
- Advanced faction diplomacy

**Uncertain? →** "Is this critical for validating trading is fun? If not, let's defer."

---

## Verification Checklist

To verify agents are following new guidelines:

### Agent Responses Should:
- [ ] Reference trade simulator MVP scope
- [ ] Check feature against MVP requirements
- [ ] Recommend deferring out-of-scope features
- [ ] Suggest simple solutions over complex ones
- [ ] Advocate for player testing
- [ ] Question over-engineering
- [ ] Reference anti-patterns when relevant

### Agent Responses Should NOT:
- [ ] Suggest building 22 systems before validation
- [ ] Recommend comprehensive documentation before features work
- [ ] Over-engineer simple solutions
- [ ] Expose all functions to Blueprints "just in case"
- [ ] Build features without player validation plan
- [ ] Continue old patterns from Oct-Dec 2025

---

## Next Steps

### Immediate (This Week)
1. All agents read new instruction files
2. Understand MVP scope and anti-patterns
3. Begin Week 1 activities (design phase)
4. Archive non-MVP features

### Week 3-4
1. Begin hardcoded prototype development
2. Agents support rapid iteration
3. Prepare for Week 4 GO/NO-GO gate

### Ongoing
1. Weekly playtesting feedback
2. Agents adjust based on player data
3. Iterate toward 30-minute demo

---

## Success = Validated Fun

**Remember**: The goal is NOT perfect code. The goal is a fun game that validates market interest.

- Good code + No gameplay = Not fundable
- OK code + Fun gameplay = Fundable

**Ship playable content. Get feedback. Iterate. Validate.**

---

**Renewal Date**: 2025-12-24  
**Current Phase**: Trade Simulator MVP (Weeks 1-12)  
**Next Review**: After Week 12 validation gates  
**Purpose**: Prevent repeating Oct-Dec 2025 mistakes, focus on market validation
