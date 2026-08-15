# Contributing to Adastrea

**Trade Simulator MVP | How to Help | Last Updated: August 9, 2026**

---

## 🎯 Current Needs (Week 12 - Final Polish)

### Critical Help Needed
1. **Playtesters** (Week 12) — NOT developers, just players
2. **Blueprint Developers** — Wire trading UI, station Blueprints
3. **Bug Testers** — Find issues in PIE
4. **Artists** — VFX for docking/trade, UI icons
5. **Audio** — Docking chime, trade confirmation sounds

---

## 🚀 How to Help

### 1. Star & Watch
```bash
# GitHub: Click Star + Watch → "All Activity"
```

### 2. Test the Prototype (When Ready)
- Week 12 playtest sign-up: [GitHub Discussions](https://github.com/Mittenzx/Adastrea/discussions)
- No instructions given — we watch you play
- Feedback form: Fun rating, confusion points, what felt good

### 3. Report Issues
```bash
# GitHub Issues → New Issue
# Template: Bug Report
# Include: UE version, steps to reproduce, Output Log, screenshot
```

### 4. Submit PRs
```bash
# 1. Fork repository
# 2. Create feature branch: git checkout -b feature/trading-ui-fix
# 3. Make changes (follow Code Style below)
# 4. Run tests: python test_trading_system.py
# 5. Submit PR → reference issue number
```

---

## 💻 Development Setup

### Prerequisites
- Unreal Engine 5.6
- Visual Studio 2022 (Windows) / Xcode (Mac)
- Git
- Python 3.8+ (for test scripts)

### Quick Start
```bash
git clone https://github.com/Mittenzx/Adastrea.git
cd Adastrea

# Run validation
python Tools/SetupCheck.py

# Open in UE 5.6
# Double-click Adastrea.uproject

# Enable Remote Control plugins for MCP
# Edit → Plugins → Remote Control HTTP + WebSocket
```

### Build
```bash
# UE Build Tools only (CI, no editor)
./build_with_ue_tools.sh Development Linux

# Full editor build
# Open Adastrea.sln in VS → Build Development Editor Win64
```

---

## 📝 Code Style

### C++ Standards
- **Standard**: C++20
- **Warnings**: `bWarningsAsErrors = true` — fix all warnings
- **Naming**: UE conventions (PascalCase classes, camelCase variables, `b` prefix for bools)
- **Headers**: Minimal includes, forward declare where possible
- **Documentation**: Doxygen-style for public APIs

```cpp
// Good
UCLASS(Blueprintable, BlueprintType)
class UMyComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="MyCategory")
    void DoSomething(int32 Count);
    
private:
    UPROPERTY(EditAnywhere, Category="Config")
    float Threshold = 1.0f;
};
```

### Blueprint Standards
- **Naming**: `BP_` prefix for Actor Blueprints, `WBP_` for Widgets
- **Events**: Use Custom Events over Tick where possible
- **Variables**: Group by category, tooltip all exposed pins
- **Graphs**: Comment boxes for logic sections, straight wires

### DataAssets
- All content via `UPrimaryDataAsset` or `UDataAsset`
- Stable `FName` IDs for save/load compatibility
- Validation in `IsDataValid()` (Editor only)

---

## 🧪 Testing Requirements

### Before PR
```bash
# Run all Python tests
python test_trading_system.py
python test_station_systems.py
python test_damage_calculations.py
python test_todo_verification.py

# Repository health
python repository_health_check_enhanced.py
```

### In Editor
- [ ] PIE starts without errors
- [ ] No red output log errors
- [ ] DataAssets load correctly
- [ ] Blueprints compile

---

## 📋 PR Checklist

- [ ] Tests pass
- [ ] Code follows style guide
- [ ] No new warnings
- [ ] Documentation updated (if API changed)
- [ ] CHANGELOG.md entry added
- [ ] Linked to issue

---

## 🎯 MVP Scope Enforcement

**If your PR adds any of these, it will be rejected:**
- ❌ Combat system (weapons, damage, combat AI)
- ❌ Exploration mechanics (scanning, discovery)
- ❌ Crew management (personnel, assignments)
- ❌ Quest system (objectives, rewards)
- ❌ Faction diplomacy (reputation, alliances)
- ❌ Multiplayer/networking
- ❌ Procedural generation (galaxy, missions)
- ❌ Character customization
- ❌ Story/narrative content

**Standard Response**: "Great idea! Let's add it AFTER we validate the trading loop is fun."

---

## 📚 Resources

| Document | Purpose |
|----------|---------|
| [01-GETTING_STARTED.md](01-GETTING_STARTED.md) | Setup & first run |
| [02-MVP_OVERVIEW.md](02-MVP_OVERVIEW.md) | What we're building |
| [04-ARCHITECTURE.md](04-ARCHITECTURE.md) | Technical architecture |
| [06-SYSTEM_REFERENCE.md](06-SYSTEM_REFERENCE.md) | All systems summary |
| [07-MVP_EXECUTION_PLAN.md](07-MVP_EXECUTION_PLAN.md) | 3-day MCP plan |
| `.github/instructions/trade-simulator-mvp.instructions.md` | Full MVP spec |
| `.github/instructions/anti-patterns.instructions.md` | Lessons learned |

---

## 📞 Communication

- **GitHub Issues**: Bugs, feature requests (MVP scope only)
- **GitHub Discussions**: Questions, ideas, playtest sign-up
- **Discord**: [Link in repo description] — real-time chat
- **Weekly Updates**: Watch repo for Sunday summary posts

---

## 🙏 Recognition

Contributors listed in:
- README.md acknowledgments
- CHANGELOG.md per release
- Credits screen in final demo

---

*Back to [INDEX.md](INDEX.md)*