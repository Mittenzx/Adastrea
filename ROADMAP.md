# Adastrea - Project Roadmap

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.6.1-blue.svg)](https://www.unrealengine.com/)

> This roadmap outlines the current MVP development plan and long-term vision for the Adastrea space trading game.

**Last Updated**: January 21, 2026  
**Current Version**: 1.0.0-alpha  
**Project Status**: Trade Simulator MVP - Week 3-4 of 12 (Hardcoded Prototype Phase)  
**Timeline**: December 2025 - March 2026

---

## Table of Contents

- [🚨 Current Status](#-current-status-week-3-4-of-12-in-progress)
- [📅 12-Week MVP Timeline](#-12-week-mvp-timeline)
- [🎯 Post-MVP Roadmap](#-post-mvp-roadmap-conditional-on-success)
- [📖 Vision & Long-Term Goals](#-vision--long-term-goals)
- [💻 Current Development Status](#-current-development-status)
- [📜 Historical Development Context](#-historical-development-context)
- [🔗 Essential Documentation](#-essential-documentation)
- [💡 Contributing to MVP](#-contributing-to-mvp)
- [⚠️ Risk Management](#️-risk-management)
- [📚 Related Documentation](#-related-documentation)
- [📞 Contact & Support](#-contact--support)

---

## 🚨 CURRENT STATUS: Week 3-4 of 12 (In Progress)

> **Phase**: Trade Simulator MVP - Hardcoded Prototype Development  
> **Dates**: January 19 - January 25, 2026 (Week 3-4 transition)  
> **Goal**: Complete playable trading loop for Week 4 external playtest  
> **Critical Deadline**: Week 4 GO/NO-GO decision (January 26, 2026)

### Week 3-4 Mission-Critical Tasks (NOW)

**Context**: We are in the final days of hardcoded prototype development. Focus is on completing the core trading loop for Week 4 external playtest.

#### 🎯 Immediate Priorities (Next 48 Hours)

1. **✅ Complete Core Systems** (Should be done or nearly done)
   - Flyable ship with 3D movement
   - Two trading stations (Agricultural & Industrial)
   - Basic Trading HUD showing credits and cargo
   - Trading Interface for buy/sell operations
   - Test level with proper spacing

2. **🔧 Polish & Bug Fixes** (Critical for playtest)
   - [ ] Test complete trading loop (5 consecutive trades)
   - [ ] Fix any crashes or blocking bugs
   - [ ] Verify profit calculations are correct
   - [ ] Ensure docking trigger works reliably
   - [ ] Balance prices if needed (target: ~20% profit per trip)

3. **📦 Playtest Preparation** (Week 4 start)
   - [ ] Package standalone build (Windows)
   - [ ] Write 1-page playtest instructions
   - [ ] Create feedback form (Google Forms)
   - [ ] Recruit 5-10 external playtesters (NOT friends/family)
   - [ ] Schedule playtest sessions

#### 📝 Week 3-4 Success Criteria

By end of Week 4 (January 26, 2026), must demonstrate:
- ✅ Ship flies smoothly between stations in ~1 minute
- ✅ Docking trigger opens trading UI correctly
- ✅ Can complete full trade loop without bugs
- ✅ Profit shows correctly on HUD
- ✅ No crashes during 10-minute play session
- ✅ **60%+ external playtesters rate gameplay as "fun"**

**GO/NO-GO Decision Point**: If &lt;60% fun rating, must pivot or redesign core loop.

### 🚧 What We're NOT Doing (Defer to Later Phases)

- ❌ Data Assets - hardcode everything for now
- ❌ Supply/demand simulation - static prices only  
- ❌ Save/load system - session only
- ❌ Multiple ships - one ship only
- ❌ Ship upgrades - defer to Phase 2
- ❌ Visual polish - placeholder meshes are fine
- ❌ Quest system, combat, exploration - completely out of scope

---

## 📅 12-Week MVP Timeline

### Phase 1: Hardcoded Prototype (Weeks 1-4) ⏳ IN PROGRESS

**Goal**: Validate core trading gameplay loop is fun

#### Week 1-2 (Dec 29 - Jan 11) ✅ COMPLETE
- ✅ Project setup and planning
- ✅ Basic Blueprint structure
- ✅ Ship movement implementation
- ✅ Station placement

#### Week 3-4 (Jan 12 - Jan 26) 🔄 CURRENT WEEK
- 🔄 Complete hardcoded prototype
- 🔄 Bug fixes and polish
- 📦 Package for external playtest
- ⚠️ **GO/NO-GO Decision**: 60%+ must rate "fun"

**Deliverable**: Playable 5-minute trading loop

---

### Phase 2: Structured Version (Weeks 5-8) 📋 PLANNED

**Goal**: Convert prototype to scalable Data Asset architecture

#### Week 5-6 (Jan 27 - Feb 8)
- [ ] Create `UTradeItemDataAsset` with 10-15 items
- [ ] Create `UMarketDataAsset` for 5 stations
- [ ] Create `USpaceshipDataAsset` for starter ship
- [ ] Migrate hardcoded data to Data Assets
- [ ] Test all systems work with Data Assets

#### Week 7-8 (Feb 9 - Feb 22)
- [ ] Implement basic economy simulation (supply/demand)
- [ ] Add 3 more stations (total: 5)
- [ ] Add 10 more items (total: 15)
- [ ] Implement simple ship upgrades (cargo capacity)
- [ ] Balance testing and iteration

**Deliverable**: 10-minute gameplay with progression

---

### Phase 3: Polish & Demo (Weeks 9-12) ✨ PLANNED

**Goal**: 30-minute polished demo ready for publishers

#### Week 9-10 (Feb 23 - Mar 8)
- [ ] Expand to 10 stations with varied economies
- [ ] Expand to 20 trade items
- [ ] Add 3 ships (starter, mid-tier, advanced)
- [ ] Implement basic save/load
- [ ] Add tutorial/onboarding

#### Week 11 (Mar 9 - Mar 15)
- [ ] Visual polish (VFX, UI improvements)
- [ ] Sound effects and audio feedback
- [ ] Balance pass on all prices and progression
- [ ] Bug fixing sprint

#### Week 12 (Mar 16 - Mar 22)
- [ ] Final playtesting (20+ testers)
- [ ] Demo packaging and optimization
- [ ] Marketing materials (screenshots, video)
- [ ] Publisher pitch preparation

**Success Metrics**:
- 75%+ "had fun" rating
- 50%+ "would buy" interest
- No critical bugs
- 60 FPS on mid-range PC

**Deliverable**: 30-minute polished demo + pitch deck

---

## 🎯 Post-MVP Roadmap (Conditional on Success)

**Only proceed if MVP achieves:**
- ✅ 75%+ fun rating
- ✅ 50%+ buy interest
- ✅ Funding secured OR strong publisher interest

### Phase 4: Content Expansion (Q2 2026)
- Add 5 new ship classes
- Expand to 30+ stations
- Implement faction reputation system
- Add quest system (delivery missions)
- Multiplayer foundation

### Phase 5: Feature Expansion (Q3-Q4 2026)
- Combat system implementation
- Exploration mechanics
- Crew management
- Advanced diplomacy
- Procedural content generation

### Phase 6: Beta & Launch (2027)
- Beta testing program
- Content polish and balance
- Marketing campaign
- Platform optimization
- Launch preparation

**See**: [Full Architecture](docs/development/FULL_ARCHITECTURE.md) for complete system details

---

## 📖 Vision & Long-Term Goals

> **Note**: These represent the full vision AFTER successful MVP validation. Current focus is 100% on Trade Simulator MVP.

### Project Vision

**Adastrea** will become an open-world space flight game featuring exploration, combat, station building, crew management, faction diplomacy, and dynamic trading economies—all built on a designer-friendly, data-driven architecture.

### Core Pillars (Post-MVP)

1. **Trading** (MVP focus) - Dynamic markets and profitable trade routes
2. **Exploration** - Discover systems, planets, phenomena
3. **Combat** - Tactical space battles
4. **Construction** - Build modular space stations
5. **Diplomacy** - Navigate faction relationships
6. **Management** - Recruit and manage crew
7. **Narrative** - Procedural and handcrafted stories

### Design Philosophy

- **Gameplay First**: Fun gameplay before infrastructure
- **Data-Driven**: Content via Blueprint Data Assets
- **Designer-Friendly**: Minimal C++ knowledge required
- **MVP Validated**: Build incrementally based on player feedback
- **Community-Focused**: Open source with comprehensive docs

---

## 💻 Current Development Status

### Code Infrastructure (Complete)

The project has 33K+ lines of C++ code implementing 22 game systems. **However**, most systems are deferred pending MVP validation.

**Infrastructure Status**:
- ✅ Build system working (Unreal Engine 5.6)
- ✅ C++ compilation successful
- ✅ Blueprint integration functional
- ✅ Documentation comprehensive
- ⚠️ Content creation minimal (by design)

### Active Systems (MVP Only)

**Currently Being Built**:
- 🔄 Trading System (hardcoded prototype)
- 🔄 Basic Flight Controls
- 🔄 Station Docking
- 🔄 Simple Economy (static prices)
- 🔄 Trading UI

**Code Exists But Deferred**:
- ⏸️ Combat System (5,700+ LOC)
- ⏸️ Navigation/Autopilot (865 LOC)
- ⏸️ Quest System (1,327 LOC)
- ⏸️ Faction Diplomacy
- ⏸️ Personnel/Crew Management
- ⏸️ Save/Load System
- ⏸️ Advanced AI
- ⏸️ And 15+ other systems

**Rationale**: Code quality is excellent, but we need validated gameplay before expanding scope.

### Documentation Status

- ✅ 74K+ words of documentation
- ✅ 30+ system guides (deferred systems)
- ✅ MVP-specific documentation in `docs/mvp/`
- ✅ Organized file structure
- ✅ Clear scope separation (MVP vs Full Vision)

**Recent Cleanup** (January 21, 2026):
- Moved all non-README docs to `docs/` subdirectories
- Created focused MVP architecture document
- Updated roadmap to current week status

---
| Trading | ✅ Complete | 85% | ✅ Excellent | ⚠️ Moderate | Market simulation depth |
| Material | ✅ Complete | 80% | ✅ Good | ⚠️ Moderate | Crafting system |
| AI | ✅ Complete | 75% | ✅ Excellent | ⚠️ Moderate | Advanced decision trees |
| Homeworld | ✅ Complete | 90% | ✅ Good | ⚠️ Moderate | More templates |
| Combat | ✅ Complete | 100% | ✅ Excellent | ✅ Good | Polish & optimization |
| Navigation | ✅ Complete | 100% | ✅ Excellent | ✅ Good | Integration testing |
| Quest | ✅ Complete | 100% | ✅ Excellent | ✅ Good | Content creation |
| Enhanced Input | ✅ Complete | 100% | ✅ Excellent | ✅ Good | Key binding UI |
| Save System | ✅ Complete | 95% | ✅ Excellent | ✅ Good | Cloud save integration |
| Tutorial | ✅ Complete | 90% | ✅ Excellent | ✅ Good | Content creation |
| Audio | ✅ Complete | 85% | ✅ Excellent | ⚠️ Moderate | Asset creation |
| Performance | ✅ Complete | 80% | ✅ Good | ⚠️ Moderate | Profiling tools |
| Player Progression | ✅ Complete | 85% | ✅ Excellent | ⚠️ Moderate | Balance & testing |
| HUD | ✅ Complete | 95% | ✅ Excellent | ✅ Good | Polish & customization |
| Inventory | ✅ Complete | 85% | ✅ Excellent | ⚠️ Moderate | Item sorting features |
| Ship Customization | ✅ Complete | 85% | ✅ Excellent | ⚠️ Moderate | More options |
| Ship Status UI | ✅ Complete | 100% | ✅ Excellent | ✅ Good | Blueprint creation needed |
| Way System | ✅ Complete | 95% | ✅ Excellent | ✅ Good | Network expansion |
| Rivals/Antagonists | ✅ Complete | 90% | ⚠️ Moderate | ⚠️ Moderate | Documentation expansion |
| Exploration | 🔄 In Progress | 70% | ✅ Excellent | ✅ Good | Phase 2 ✅ Complete |
| Multiplayer | ❌ Planned | 0% | ❌ None | ❌ None | Technical research |

**Legend**:
- ✅ Complete/Excellent: Production-ready
- ⚠️ Partial/Moderate: Functional but needs work
- ❌ None/Limited: Not started or minimal progress
- 🔄 In Progress: Active development

---

## 📜 Historical Development Context

> **Note**: The sections below describe infrastructure built October-December 2025. While code exists, most systems are deferred pending MVP validation.

### Infrastructure Built (Oct-Dec 2025)

**Code Complete**:
- 33K+ lines of C++ across 22 game systems
- Spaceship, Station, Faction, Personnel systems
- Trading, Combat, Navigation, Quest systems
- Save, Tutorial, Audio, Performance systems
- Comprehensive Blueprint APIs

**Documentation Complete**:
- 74K+ words across 30+ system guides
- YAML templates for all systems
- Designer workflows and onboarding
- Automation and testing frameworks

**Quality Assessment** (December 2025 Critical Review):
- ✅ Code Quality: A- (excellent architecture, clean code)
- ✅ Documentation: A (comprehensive, well-organized)
- ⚠️ Gameplay: F (no playable content)
- ⚠️ Market Validation: F (zero player feedback)
- ⚠️ Timing: F (infrastructure before gameplay)

**Lesson Learned**: "Perfect infrastructure without gameplay = zero value"

### Why MVP Pivot (December 2025)

**Problem**: 3 months of development, 22 systems, zero playable gameplay
- ⏳ Community management
- ⏳ Initial patch readiness

**Post-Launch (March 2027+)**:
- ⏳ Community feedback collection
- ⏳ Bug fixes and hotfixes
- ⏳ Quality of life improvements
- ⏳ Balance adjustments based on player data

#### Ongoing Development

**Regular Updates**:
- ⏳ Monthly patches (bug fixes, balance)
- ⏳ Quarterly content updates
- ⏳ Community events and challenges
- ⏳ Seasonal events

**Expansion Content** (Post-Launch):
- ⏳ New ship classes and variants
- ⏳ New faction storylines
- ⏳ New star systems and sectors
- ⏳ New gameplay modes
- ⏳ Multiplayer features (co-op, PvP)

**Community & Modding**:
- ⏳ Active community management
- ⏳ Mod showcase and featured mods
- ⏳ Modding contests and events
- ⏳ Community content integration

**Long-Term Vision**:
- ⏳ Major expansions (1-2 per year)
- ⏳ Multiplayer enhancements
- ⏳ VR support exploration
- ⏳ Cross-platform play
- ⏳ Engine upgrades (UE 5.7+)

#### Key Deliverables
- Public release of Adastrea 1.0
- Post-launch support infrastructure
- Regular content updates
- Active community and modding ecosystem
- Expansion content (DLC)

---

## 🔗 Essential Documentation

### MVP Development
- **[Trade Simulator MVP Instructions](.github/instructions/trade-simulator-mvp.instructions.md)** - Complete 12-week plan
- **[MVP Guide](docs/mvp/README.md)** - MVP development guide
- **[Phase 1 Checklist](docs/mvp/PHASE_1_CHECKLIST.md)** - Daily task breakdown
- **[Architecture](ARCHITECTURE.md)** - MVP architecture overview

### Development Resources
- **[Getting Started](docs/GETTING_STARTED.md)** - Setup and quick start
- **[Contributing](docs/development/CONTRIBUTING.md)** - How to contribute
- **[Code Style](docs/development/CODE_STYLE.md)** - C++ and Blueprint standards
- **[Full Architecture](docs/development/FULL_ARCHITECTURE.md)** - Complete system details (deferred)

## 💡 Contributing to MVP

### Current Needs (Week 3-4)

**Critical Help Needed**:
1. **Playtesters** (Week 4) - NOT developers, just players
2. **Blueprint Developers** - Help complete trading UI
3. **Bug Testers** - Find issues in hardcoded prototype
4. **Documentation Writers** - Update guides as we build

### How to Help

1. **Star the Repository** - Show support
2. **Watch for Updates** - Follow MVP progress
3. **Test the Prototype** - Week 4 playtest sign-up coming
4. **Report Issues** - Use GitHub Issues for bugs
5. **Submit PRs** - Follow [Contributing Guide](docs/development/CONTRIBUTING.md)

### Development Setup

```bash
# Clone repository
git clone https://github.com/Mittenzx/Adastrea.git
cd Adastrea

# Open in Unreal Engine 5.6
# File → Open Project → Adastrea.uproject

# See docs/GETTING_STARTED.md for detailed setup
```

### Communication

- **GitHub Issues**: Bug reports and feature requests
- **GitHub Discussions**: Questions and ideas
- **Pull Requests**: Code contributions
- **Project Board**: Track MVP progress

---

## ⚠️ Risk Management

### MVP-Specific Risks

**High Risk**:
- ❌ Week 4 playtest fails (&lt;60% fun) → **Mitigation**: Pivot or redesign loop
- ❌ Can't recruit external testers → **Mitigation**: Use online communities
- ❌ Core loop not fun → **Mitigation**: Rapid iteration based on feedback

**Medium Risk**:
- ⚠️ Technical blockers in Blueprints → **Mitigation**: Simplify approach
- ⚠️ Balance issues (pricing) → **Mitigation**: Rapid iteration
- ⚠️ Scope creep → **Mitigation**: Strict MVP scope enforcement

**Low Risk**:
- ✅ Documentation incomplete → Already comprehensive
- ✅ Build system issues → Working reliably
- ✅ Code quality → High standards maintained

### Post-MVP Risks (If We Proceed)

- Market competition (Elite, X4, Star Citizen)
- Funding challenges
- Content creation bottleneck
- Team scaling issues
- Feature complexity management

**Mitigation**: Only worry about these IF MVP succeeds

---

### Engine & Infrastructure

#### Unreal Engine Updates
- **Current**: Unreal Engine 5.6.1
- **Q2 2026**: Evaluate UE 5.7 upgrade
- **Q3 2026**: Migration to UE 5.7 (if stable)
- **2027+**: Keep current with latest UE5 stable releases

---

## 📚 Related Documentation

### MVP Focus
- **[README.md](README.md)** - Project overview
- **[ARCHITECTURE.md](ARCHITECTURE.md)** - MVP architecture
- **[Trade Simulator MVP](.github/instructions/trade-simulator-mvp.instructions.md)** - 12-week plan
- **[Getting Started](docs/GETTING_STARTED.md)** - Setup guide

### Development
- **[CHANGELOG](docs/CHANGELOG.md)** - Version history
- **[Contributing](docs/development/CONTRIBUTING.md)** - How to contribute
- **[Code Style](docs/development/CODE_STYLE.md)** - Standards
- **[Full Architecture](docs/development/FULL_ARCHITECTURE.md)** - Complete system details (deferred)

### Post-MVP Reference (Deferred Systems)
- See `Assets/` directory for deferred system guides
- See `docs/archive/` for archived documentation
- See `.github/instructions/anti-patterns.instructions.md` for lessons learned

---

## 📞 Contact & Support

- **Repository**: [github.com/Mittenzx/Adastrea](https://github.com/Mittenzx/Adastrea)
- **Issues**: [GitHub Issues](https://github.com/Mittenzx/Adastrea/issues) - Bug reports
- **Discussions**: GitHub Discussions - Questions and ideas
- **MVP Progress**: Watch this repository for weekly updates

---

**Last Updated**: January 21, 2026  
**Current Phase**: Trade Simulator MVP - Week 3-4 of 12  
**Next Milestone**: Week 4 GO/NO-GO Decision (January 26, 2026)

**Built with ❤️ by the Adastrea community**

*"One gameplay loop at a time."*
