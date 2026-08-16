# Getting Started with Adastrea

**Unreal Engine 5.6 | Trade Simulator MVP | Last Updated: August 9, 2026**

---

## 🎯 Quick Start (5 Minutes)

### Prerequisites
- **Unreal Engine 5.6** (exact version in `.uproject`)
- **Visual Studio 2022** (Windows) or **Xcode** (Mac) for C++
- **Git** for version control

### 1. Clone & Open
```bash
git clone https://github.com/Mittenzx/Adastrea.git
cd Adastrea
# Double-click Adastrea.uproject to open in UE 5.6
```

### 2. Enable Required Plugins
In Unreal Editor:
```
Edit → Plugins → Search "Remote Control"
☑ Remote Control HTTP (Port 30010)
☑ Remote Control WebSocket (Port 30020)
Restart Editor
```

### 3. Build Project
**Option A: UE Build Tools Only (~500 MB, no editor needed)**
```bash
# Linux/Mac
./setup_ue_build_tools.sh
./build_with_ue_tools.sh Development Linux

# Windows
setup_ue_build_tools.bat
build_with_ue_tools.bat Development Win64
```

**Option B: Full Editor Build**
- Open `Adastrea.sln` in Visual Studio
- Build → Build Solution (Development Editor, Win64)
- Or open `.uproject` directly in Unreal Editor

### 4. Verify Installation
- Launch Editor
- Check Output Log for errors
- Open `Content/DataAssets/Trading/Items/` - should see 15 items
- Open `Content/Blueprints/Stations/Modules/` - should see station modules

---

## 🛠️ Development Setup

### Run Validation Scripts (Recommended)
```bash
# Cross-platform Python (Windows/Linux/Mac)
python Tools/SetupCheck.py

# Or shell script (Linux/Mac/WSL)
./Tools/SetupCheck.sh
```

Checks: UE version, compiler, project structure, required files, documentation.

### Generate Visual Studio Project Files
```
Right-click Adastrea.uproject → "Generate Visual Studio project files"
```
**Issues?** See `09-SETUP_GUIDES/PROJECT_GENERATION_QUICK_FIX.md`

### MCP Integration (For AI-Assisted Development)
The `unreal-engine-mcp-server` is configured. To use:
1. Complete steps 1-4 above
2. Enable Remote Control plugins (step 2)
3. Verify connection: I'll run `manage_tools(action="list_tools")` — should show 23 tools

---

## 🎮 First Playtest (PIE)

### Quick Test Without MCP
1. Open `Content/Maps/TestLevel` (the active test level)
2. Player ship `BP_Battleship` spawns via `BP_SpaceGameMode` (DefaultPawn)
3. `BP_SpaceStation` and market modules are already placed/wired
4. Press **Play** (PIE)
5. Fly to station, dock, trade!

### With MCP (Automated)
Once MCP is connected, I'll:
1. Create test level `MVP_TestLevel`
2. Spawn 3 stations in triangle formation
3. Wire trading UI
4. You fly/dock/trade in PIE while I capture screenshots

---

## 📁 Key Content Paths

| System | Path |
|--------|------|
| Trade Items | `Content/DataAssets/Trading/Items/*.uasset` |
| Markets | `Content/DataAssets/Trading/Markets/*.uasset` |
| Ships | `Content/Blueprints/Ships/*.uasset` |
| Stations | `Content/Blueprints/Stations/*.uasset` |
| Station Modules | `Content/Blueprints/Stations/Modules/*.uasset` |
| Trading UI | `Content/Blueprints/Trading/*.uasset` |

---

## 🔧 Common Issues & Fixes

| Issue | Solution |
|-------|----------|
| "SetEnv task failed" (Windows) | Use `build_with_ue_tools.bat` - bypasses MSBuild path limit |
| Project won't open | Check UE version matches `.uproject` (5.6) |
| Missing modules | Run `Generate Visual Studio project files` |
| Plugins not showing | Restart Editor after enabling |
| MCP connection fails | Verify Remote Control plugins enabled, ports 30010/30020 |

---

## 📚 Next Steps

| You Want To... | Read |
|----------------|------|
| Understand the MVP | [02-MVP_OVERVIEW.md](02-MVP_OVERVIEW.md) |
| See current status | [03-CURRENT_STATUS.md](03-CURRENT_STATUS.md) |
| Read architecture | [04-ARCHITECTURE.md](04-ARCHITECTURE.md) |
| View roadmap | [05-ROADMAP.md](05-ROADMAP.md) |
| Follow 3-day MCP plan | [07-MVP_EXECUTION_PLAN.md](07-MVP_EXECUTION_PLAN.md) |
| Build with UE tools only | [09-SETUP_GUIDES/BUILD_WITH_UE_TOOLS.md](09-SETUP_GUIDES/BUILD_WITH_UE_TOOLS.md) |

---

## ⚠️ Important Notes

1. **Code is 85% complete, content is 10% complete** - C++ systems exist but need editor validation
2. **Most systems are DEFERRED** - Only trading, basic flight, stations, economy are MVP scope
3. **MCP needs full Editor** - Build tools alone won't work for MCP
4. **Week 12 deadline**: March 22, 2026 - 30-min demo for publishers

---

*Back to [INDEX.md](INDEX.md)*