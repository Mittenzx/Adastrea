# Getting Started with Adastrea

**Last Updated**: January 14, 2026  
**Project Status**: Trade Simulator MVP Development (Week 3-4 of 12)  
**Engine**: Unreal Engine 5.6  
**Language**: C++ with Blueprint integration

---

## 🎯 Current Focus: Trade Simulator MVP

Adastrea is building a **space trading game MVP** to validate market interest and secure funding. We're focused on ONE core gameplay loop: **buy low, sell high, upgrade ship, repeat**.

**Target**: Playable 30-minute demo by March 2026

---

## 🚀 Quick Start Paths

Choose your role:

### 🎮 Want to Play the MVP?
*Available: March 2026 (Week 12)*

We're building a playable demo. Check back in March 2026, or:
- Star/watch this repo for updates
- See `docs/mvp/README.md` for development progress
- Follow `.github/instructions/trade-simulator-mvp.instructions.md`

### 💻 Want to Contribute Code?
*C++ and Blueprint developers*

1. **Prerequisites**: UE 5.6, Visual Studio 2022, Git
2. **Clone**: `git clone https://github.com/Mittenzx/Adastrea.git`
3. **Build**: See `QUICKSTART.md` for detailed setup
4. **Read**: `.github/instructions/README.md` for coding guidelines
5. **Focus**: MVP systems only (trading, docking, economy)

**Key Documents:**
- `ARCHITECTURE.md` - System architecture
- `CODE_STYLE.md` - C++ and Blueprint standards
- `CONTRIBUTING.md` - Contribution workflow
- `.github/instructions/trade-simulator-mvp.instructions.md` - MVP plan

### 🎨 Want to Create Content?
*Designers and content creators*

1. **Setup**: Install Unreal Engine 5.6
2. **Open**: `Adastrea.uproject`
3. **Learn**: `docs/mvp/GETTING_STARTED.md` - MVP content guide
4. **Create**: Ships, stations, and trade goods using Data Assets

**Key Documents:**
- `Assets/SpaceshipTemplates/` - Ship templates and examples
- `Assets/TradingSystemGuide.md` - Trading system overview
- `.github/instructions/data-assets.instructions.md` - Data Asset patterns

### 📚 Want to Understand the Project?
*Researchers, reviewers, or curious developers*

**Start Here:**
1. `README.md` - Project overview (this file)
2. `ARCHITECTURE.md` - Technical architecture and design patterns
3. `docs/archive/critical-review-2025/README.md` - Why we pivoted to MVP
4. `.github/instructions/anti-patterns.instructions.md` - Lessons learned

**Explore:**
- `docs/` - All technical documentation (organized by category)
- `wiki/` - GitHub wiki pages
- `CHANGELOG.md` - What's changed recently

### 📝 Want to Help with Documentation?
*Technical writers*

1. **Read**: `docs/README.md` - Documentation structure
2. **Follow**: `CODE_STYLE.md` - Documentation standards
3. **Focus**: MVP documentation gaps
4. **Update**: Keep cross-references current

---

## 📁 Repository Structure

```
Adastrea/
├── .github/              # GitHub configuration and AI instructions
│   └── instructions/     # ⭐ Copilot and agent guidelines
├── Source/               # C++ source code
│   └── Adastrea/         # Main game module
├── Content/              # Unreal Engine content (Blueprints, assets)
├── docs/                 # ⭐ All documentation (lowercase!)
│   ├── mvp/              # ⭐ Trade Simulator MVP documentation
│   ├── development/      # Developer guides
│   ├── reference/        # Quick references
│   └── archive/          # Historical documents
├── Assets/               # YAML templates and system guides
├── Tools/                # Python automation scripts
├── tests/                # Test suite
├── README.md             # Project overview
├── ARCHITECTURE.md       # System architecture
├── CODE_STYLE.md         # Coding standards
├── CONTRIBUTING.md       # Contribution guide
└── CHANGELOG.md          # Version history
```

**⭐ Key Locations:**
- **MVP Documentation**: `docs/mvp/`
- **AI Instructions**: `.github/instructions/`
- **Developer Docs**: `docs/development/`
- **Code Standards**: `CODE_STYLE.md`

---

## 🎮 What We're Building (MVP Scope)

### Core Gameplay Loop (3-5 minutes)
```
Dock at Station A
    ↓
Buy cargo (cheap goods)
    ↓
Fly to Station B
    ↓
Sell cargo (expensive there)
    ↓
Profit! 💰
    ↓
Upgrade ship capacity
    ↓
Repeat with bigger profits
```

### MVP Features (March 2026)
- ✅ 1 flyable ship (with upgrades)
- ✅ 5-10 trading stations
- ✅ 10-20 trade goods
- ✅ Dynamic supply/demand economy
- ✅ Simple docking
- ✅ Trading UI
- ✅ Profit tracking
- ✅ Ship upgrades

### NOT in MVP (Future)
- ❌ Combat system
- ❌ Exploration mechanics
- ❌ Faction diplomacy (beyond pricing)
- ❌ Crew management
- ❌ Quest system
- ❌ Multiplayer

**Why?** We're validating ONE core loop is fun before expanding. See `.github/instructions/anti-patterns.instructions.md` for lessons learned.

---

## 📖 Essential Documentation

### For Everyone
- **README.md** (this file) - Project overview and getting started
- **CHANGELOG.md** - What's new and what changed
- **docs/README.md** - Complete documentation index

### For Developers
- **ARCHITECTURE.md** - System design and patterns
- **CODE_STYLE.md** - C++ and Blueprint standards
- **CONTRIBUTING.md** - How to contribute
- **QUICKSTART.md** - Detailed setup instructions

### For MVP Work
- **docs/mvp/README.md** - MVP documentation hub
- **.github/instructions/trade-simulator-mvp.instructions.md** - Week-by-week plan
- **.github/instructions/anti-patterns.instructions.md** - What NOT to do

### For Understanding the Pivot
- **docs/archive/critical-review-2025/README.md** - December 2025 assessment
- **.github/instructions/anti-patterns.instructions.md** - Lessons learned

---

## 🛠️ Development Setup (Quick)

### Prerequisites
- Windows 10/11, macOS, or Linux
- Unreal Engine 5.6
- Visual Studio 2022 (Windows) or Xcode (Mac)
- Git
- 20GB+ free disk space

### Installation
```bash
# Clone repository
git clone https://github.com/Mittenzx/Adastrea.git
cd Adastrea

# Generate project files
# Windows: Right-click Adastrea.uproject → "Generate Visual Studio project files"
# Mac: Right-click Adastrea.uproject → "Generate Xcode project"

# Open in IDE and build
# Windows: Open Adastrea.sln in Visual Studio 2022
# Mac: Open Adastrea.xcworkspace in Xcode

# Build configuration: "Development Editor"
```

**Detailed setup**: See `QUICKSTART.md`

---

## 🧪 Running Tests

```bash
# Python tests (automation and validation)
cd tests
python3 run_all_tests.py

# Unreal Engine tests
# Open Adastrea.uproject → Tools → Test Automation
```

---

## 📊 Project Status

### Current Phase: MVP Development
- **Week**: 3-4 of 12 (Prototype phase)
- **Goal**: Hardcoded playable prototype by Week 4
- **Next**: External playtesting, GO/NO-GO decision

### Success Metrics
- **Week 4**: 60% of playtesters say "this is fun"
- **Week 8**: 70% "had fun", multiple trade routes
- **Week 12**: 75% "had fun", 50% "would buy", 30-min polished demo

### Timeline
- **Jan 2026**: Weeks 1-4 (Prototype)
- **Feb 2026**: Weeks 5-8 (Structure and content)
- **Mar 2026**: Weeks 9-12 (Polish and demo)

**Track Progress**: See `CHANGELOG.md` for updates

---

## 🤝 Contributing

We welcome contributions focused on the **Trade Simulator MVP**!

### Before Contributing
1. Read `CONTRIBUTING.md` - Contribution workflow
2. Read `.github/instructions/trade-simulator-mvp.instructions.md` - Current focus
3. Read `.github/instructions/anti-patterns.instructions.md` - What to avoid
4. Check open issues for "MVP" label

### Contribution Guidelines
- **Focus on MVP** - Trading, docking, economy only
- **Keep it simple** - Minimal viable changes
- **Test your changes** - Don't break existing features
- **Follow standards** - See `CODE_STYLE.md`
- **Update docs** - If you change APIs

### MVP Priorities
1. ✅ Trading UI improvements
2. ✅ Economy simulation
3. ✅ Ship docking mechanics
4. ✅ Cargo management
5. ✅ Station variety

### NOT Priorities (Yet)
- ❌ Combat features
- ❌ AI improvements (beyond economy)
- ❌ Faction systems
- ❌ Quest systems
- ❌ Multiplayer

---

## 📞 Getting Help

### Documentation
- **Quick Start**: See `QUICKSTART.md`
- **Full Docs**: See `docs/README.md`
- **MVP Docs**: See `docs/mvp/README.md`
- **Troubleshooting**: See `docs/reference/` quick references

### Community
- **Issues**: GitHub Issues (tag with "help wanted")
- **Discussions**: GitHub Discussions
- **Wiki**: `wiki/Home.md`

### For Developers
- **Architecture Questions**: See `ARCHITECTURE.md`
- **Coding Standards**: See `CODE_STYLE.md`
- **AI Instructions**: See `.github/instructions/README.md`

---

## 📚 Learning Resources

### Unreal Engine
- [Official UE5 Documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-5-6-documentation)
- [Unreal Directive Best Practices](https://unrealdirective.com/)
- `.github/instructions/unreal-directive-best-practices.md`

### Adastrea Specific
- **System Guides**: `Assets/` directory (system-specific documentation)
- **Code Examples**: `Source/Adastrea/` (reference implementations)
- **Templates**: `Assets/SpaceshipTemplates/`, `Assets/SectorTemplates/`

---

## 🎯 Project Vision

### Short-Term (Q1 2026)
Build a **fun, polished 30-minute trading demo** that validates market interest.

### Medium-Term (2026)
If MVP succeeds:
- Secure funding (publisher, crowdfunding, or early access)
- Expand trade simulator (more ships, stations, goods)
- Add progression systems (reputation, unlocks)

### Long-Term (2027+)
If funded:
- Add exploration gameplay
- Add tactical combat
- Add faction diplomacy
- Add crew management
- Build towards full open-world space game

**But first**: Prove trading is fun. Everything else depends on that.

---

## ⚠️ Important Notes

### What Changed (December 2025)
Adastrea underwent a **strategic pivot** in December 2025:
- **Before**: Building 22 systems simultaneously, no playable content
- **After**: Focus on ONE system (trading), playable in 12 weeks

**Why?** See `docs/archive/critical-review-2025/README.md` for full analysis.

### Development Philosophy
**Gameplay First**: Fun, validated gameplay before infrastructure  
**Keep It Simple**: Minimal viable features, iterate based on feedback  
**Test Early**: Player feedback every 2 weeks  
**Focus Wins**: One polished system beats ten half-finished systems

See `.github/instructions/anti-patterns.instructions.md` for lessons learned.

---

## 📜 License

See `LICENSE` file for details.

---

## 🙏 Acknowledgments

- **Unreal Engine** by Epic Games
- **Community Contributors** - See `CONTRIBUTING.md`
- **Critical Review** (Dec 2025) - For honest assessment that led to MVP pivot

---

**Ready to contribute?** Start with `CONTRIBUTING.md`  
**Want to understand the project?** Read `ARCHITECTURE.md`  
**Looking for MVP docs?** See `docs/mvp/README.md`

**Questions?** Open a GitHub Issue or Discussion.

---

*Last updated: January 14, 2026 - Repository Modernization*
