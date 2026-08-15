# Project Roadmap

**Trade Simulator MVP | Updated: August 9, 2026**

---

## 🚨 Current Status: Week 12 of 12 — Final Polish

**Timeline**: December 2025 – March 2026 (Original) | **Actual**: August 2026 (Extended)
**Goal**: 30-minute polished demo for publishers
**Status**: Code complete, content incomplete — **Need PIE validation now**

---

## 📅 12-Week MVP Timeline (Original Plan)

| Phase | Weeks | Dates | Goal | Status |
|-------|-------|-------|------|--------|
| **Phase 1** | 1-4 | Dec 29 – Jan 26 | Hardcoded prototype, validate fun | ✅ DONE |
| **Phase 2** | 5-8 | Jan 27 – Feb 22 | DataAsset architecture, economy | ✅ DONE |
| **Phase 3** | 9-12 | Feb 23 – Mar 22 | Polish, 30-min demo | 🔄 **ACTIVE** |

---

## 🎯 Phase 3 Detail (Weeks 9-12)

| Week | Dates | Focus | Original Status | **Actual Status (Aug 2026)** |
|------|-------|-------|-----------------|------------------------------|
| 9-10 | Feb 23 – Mar 8 | 10 stations, 20 items, 3 ships, save/load | ✅ Done | **Code done, content gaps** |
| 11 | Mar 9 – Mar 15 | VFX, audio, balance, bugs | ✅ Done | **VFX/Audio missing** |
| **12** | **Mar 16 – Mar 22** | **Playtest, package, pitch** | 🔄 Active | **🔄 THIS WEEK** |

---

## 📋 Week 12 Success Criteria (Must Hit)

| Metric | Target | Current |
|--------|--------|---------|
| Fun Rating | 75%+ | Untested |
| Buy Intent | 50%+ | Untested |
| Critical Bugs | 0 | Unknown |
| Performance | 60 FPS (GTX 1060) | Untested |
| Demo Length | 30 min | 0 min |
| Pitch Package | Complete | Not started |

---

## 🚀 Revised Execution Plan (August 2026)

### Week 1: MCP Foundation (Days 1-3)
- [ ] Verify all 25 DataAssets via MCP
- [ ] Create 3 core Blueprints (TradingShip, TradeStation, TradingUI)
- [ ] Wire trading logic in Blueprint graphs
- [ ] Build test level with 3 stations
- [ ] **First PIE test** — you fly/dock/trade

### Week 2: Content & Polish (Days 4-7)
- [ ] Create 5 missing MarketDataAssets via MCP
- [ ] Create 5 missing TradeItemDataAssets via MCP
- [ ] Add VFX (docking ring, trade sparkle)
- [ ] Add Audio (docking chime, cash register)
- [ ] UI animations (profit pulse, button feedback)
- [ ] Save/Load test in PIE

### Week 3: Validation & Package (Days 8-10)
- [ ] Performance profile (stat fps/unit/gpu)
- [ ] Bug fix sprint
- [ ] Balance pass on all prices
- [ ] Recruit 5→10→20 playtesters
- [ ] Package Shipping build
- [ ] Create pitch materials (screenshots, video, deck)

---

## 🎯 Post-MVP Roadmap (Conditional)

**Only if MVP hits: 75% fun + 50% buy interest + funding/publisher interest**

| Phase | Timeline | Focus |
|-------|----------|-------|
| **Phase 4** | Q2 2026 | 5 new ships, 30+ stations, faction reputation, delivery quests, multiplayer foundation |
| **Phase 5** | Q3-Q4 2026 | Combat, exploration, crew management, advanced diplomacy, procedural generation |
| **Phase 6** | 2027 | Beta, polish, marketing, launch, post-launch support |

---

## 💻 Deferred Systems (Code Exists, Not Wired)

| System | LOC | Status |
|--------|-----|--------|
| Combat | 5,700+ | Complete, disabled |
| Navigation/Autopilot | 865 | Complete, disabled |
| Quest System | 1,327 | Complete, disabled |
| Faction Diplomacy | 2,000+ | Complete, disabled |
| Personnel/Crew | 3,000+ | Complete, disabled |
| Save/Load (full) | 2,500+ | Partial |
| Advanced AI | 4,000+ | Complete, disabled |
| Exploration/Scanning | 3,500+ | Partial |
| Way Network | 2,800+ | Complete, disabled |
| Rivals/Antagonists | 2,200+ | Complete, disabled |
| Battle Pets | 1,800+ | Complete, disabled |
| Building/Crafting | 3,000+ | Complete, disabled |
| Multiplayer | 0 | Planned only |

**Total Deferred**: ~35K lines (50% of codebase)

---

## ⚠️ Risk Management

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| No playtesters | High | Critical | Recruit NOW (Discord, Reddit, indie dev) |
| Blueprint wiring breaks C++ API | Medium | High | Test each component incrementally |
| Performance < 60 FPS | Medium | High | Profile early, optimize draw calls |
| Scope creep | High | Medium | Strict MVP scope enforcement |
| Content creation bottleneck | High | High | MCP automation for DataAssets |

---

## 🔧 Engine & Infrastructure

| Item | Current | Plan |
|------|---------|------|
| Unreal Engine | 5.6 | Evaluate 5.7 in Q2 2026 |
| Build System | Working | Keep working |
| CI/CD | GitHub Actions | Add automated PIE tests |
| MCP | Configured (23 tools) | Use for content creation |

---

## 📞 Communication

- **Issues**: GitHub Issues — bugs, blockers
- **Discussions**: GitHub Discussions — questions, ideas
- **Progress**: Watch repo for weekly updates
- **MVP Decisions**: Document in CHANGELOG.md

---

*Back to [INDEX.md](INDEX.md) | Next: [06-SYSTEM_REFERENCE.md](06-SYSTEM_REFERENCE.md)*