# Adastrea Documentation Index

**Version**: 1.0 | **Last Updated**: August 9, 2026 | **Project Status**: Trade Simulator MVP - Week 12 of 12 (Final Polish)

---

## 🚀 Quick Navigation

| You Want To... | Go To |
|----------------|-------|
| **Start developing** | [01-GETTING_STARTED.md](01-GETTING_STARTED.md) |
| **Understand the MVP** | [02-MVP_OVERVIEW.md](02-MVP_OVERVIEW.md) |
| **See current status** | [03-CURRENT_STATUS.md](03-CURRENT_STATUS.md) |
| **Read architecture** | [04-ARCHITECTURE.md](04-ARCHITECTURE.md) |
| **View roadmap** | [05-ROADMAP.md](05-ROADMAP.md) |
| **Check systems** | [06-SYSTEM_REFERENCE.md](06-SYSTEM_REFERENCE.md) |
| **Follow execution plan** | [07-MVP_EXECUTION_PLAN.md](07-MVP_EXECUTION_PLAN.md) |
| **Contribute** | [08-CONTRIBUTING.md](08-CONTRIBUTING.md) |

---

## 📁 Documentation Structure

```
docs/
├── INDEX.md                    # This file - master navigation
├── 01-GETTING_STARTED.md       # Setup, build, first run
├── 02-MVP_OVERVIEW.md          # What is the Trade Simulator MVP
├── 03-CURRENT_STATUS.md        # Real status: code vs content
├── 04-ARCHITECTURE.md          # Technical architecture
├── 05-ROADMAP.md               # Timeline & milestones
├── 06-SYSTEM_REFERENCE.md      # All 22 systems summary
├── 07-MVP_EXECUTION_PLAN.md    # 3-day MCP execution plan
├── 08-CONTRIBUTING.md          # How to contribute
├── 09-SETUP_GUIDES/
│   ├── BUILD_WITH_UE_TOOLS.md
│   ├── VISUAL_STUDIO_PROJECT_GENERATION.md
│   ├── PROJECT_GENERATION_QUICK_FIX.md
│   └── WINDOWS_BUILD_SCRIPTS.md
├── 10-MVP_PHASES/
│   ├── PHASE_1_HARDCODED_PROTOTYPE.md
│   ├── PHASE_2_STRUCTURED_VERSION.md
│   └── PHASE_3_POLISH_DEMO.md
├── 11-TECHNICAL_SPECS/
│   ├── TRADING_SYSTEM.md
│   ├── STATION_SYSTEM.md
│   ├── SHIP_SYSTEM.md
│   ├── ECONOMY_SYSTEM.md
│   └── DATA_ASSET_GUIDE.md
├── 12-ANTI_PATTERNS.md
├── 13-CRITICAL_REVIEW_SUMMARY.md
└── 14-ARCHIVE/                 # All legacy docs (500+ files)
    ├── mvp_old/
    ├── archive_old/
    ├── development/
    ├── reference/
    ├── ...
    └── (everything else)
```

---

## 🎯 Current Focus: Trade Simulator MVP

**Goal**: Build a playable, fun trading game in 12 weeks that validates market interest.

**Core Loop**: Buy low → Fly → Sell high → Upgrade ship → Repeat with better margins

**Success Metrics**:
- Week 4: 60%+ "had fun" rating from playtesters
- Week 8: 70%+ "had fun", 50%+ "would buy"
- Week 12: 75%+ "fun", 50%+ "would buy" → Publisher ready

**Current Week**: 12 of 12 (Final Polish & Demo Preparation)
**Target**: March 22, 2026 - 30-minute polished demo for publishers

---

## 🔧 Key Technical Facts

| Aspect | Detail |
|--------|--------|
| **Engine** | Unreal Engine 5.6 |
| **Code** | 33K+ lines C++, 22 systems |
| **Trading MVP** | Complete C++ infrastructure |
| **DataAssets** | 15 trade items, 5 markets (validated) |
| **Blueprints** | 10+ ships, station modules, trading UI templates |
| **Tests** | 3 Python test suites passing |
| **MCP Ready** | unreal-engine-mcp-server configured (23 tools) |

---

## 📋 Out of Scope (Until MVP Validates)

- ❌ Combat system
- ❌ Exploration mechanics  
- ❌ Crew management
- ❌ Quest system
- ❌ Faction diplomacy
- ❌ Multiplayer/networking
- ❌ Procedural generation

> "Great idea for v2.0! Let's validate the trading loop is fun first."

---

## 🔗 External References

- **GitHub**: https://github.com/Mittenzx/Adastrea
- **MVP Spec**: `.github/instructions/trade-simulator-mvp.instructions.md`
- **Anti-Patterns**: `.github/instructions/anti-patterns.instructions.md`
- **Unreal MCP Setup**: `docs/14-ARCHIVE/setup/UNREAL_MCP_SETUP_STEPS.md`

---

*Start here: [01-GETTING_STARTED.md](01-GETTING_STARTED.md)*