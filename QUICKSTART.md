# Quick Start Guide - Adastrea

Welcome to Adastrea! This guide will get you up and running in 15 minutes.

## Prerequisites Check

Before you begin, make sure you have:

- [ ] **Unreal Engine 5.5** installed ([Download here](https://www.unrealengine.com/))
- [ ] **Visual Studio 2022** (Windows) or **Xcode** (Mac)
- [ ] **Git** installed and configured
- [ ] At least **20GB of free disk space**
- [ ] Basic familiarity with Unreal Engine (if not, see [Learning Resources](#learning-resources))

## Step-by-Step Setup

### 1. Clone the Repository (2 minutes)

```bash
# Clone the project
git clone https://github.com/Mittenzx/Adastrea.git
cd Adastrea

# Verify you're on the correct branch
git status
```

### 2. Generate Project Files (1 minute)

**Windows:**
1. Right-click on `Adastrea.uproject` (or the .uproject file in the root)
2. Select "Generate Visual Studio project files"
3. Wait for the process to complete

**Mac:**
1. Right-click on `Adastrea.uproject`
2. Select "Generate Xcode project files"
3. Wait for the process to complete

### 3. Build the Project (5-10 minutes)

**Option A: Build from IDE**
1. Open `Adastrea.sln` (Windows) or `Adastrea.xcworkspace` (Mac)
2. Set configuration to "Development Editor"
3. Build the solution (Ctrl+Shift+B on Windows, Cmd+B on Mac)
4. Wait for compilation to complete (this may take a while the first time)

**Option B: Build from Editor**
1. Double-click `Adastrea.uproject`
2. Unreal Engine will prompt you to build
3. Click "Yes" and wait for compilation

### 4. Launch the Editor (1 minute)

1. Double-click `Adastrea.uproject` (if not already open)
2. Wait for the editor to load all assets
3. The main editor window should appear

### 5. Verify Installation (2 minutes)

Once in the editor:

1. **Check Content Browser**:
   - Navigate to `Content/` folder
   - You should see folders: Blueprints, Maps, SpaceStations, UI, etc.

2. **Open a Map**:
   - Go to `Content/Maps/`
   - Double-click any map to open it

3. **Test Play**:
   - Click the green "Play" button at the top
   - The game should launch in a new window
   - Press ESC to exit play mode

4. **Check Data Assets**:
   - Open Content Browser
   - Search for "DataAsset"
   - You should see various faction, ship, and personnel assets

## Your First Exploration

### For Designers (No Coding)

#### Create Your First Spaceship

1. **Navigate to Content Browser**:
   - Right-click in a folder
   - Select "Miscellaneous" > "Data Asset"
   - Choose "SpaceshipDataAsset"

2. **Configure the Ship**:
   - Name it: `DA_MyFirstShip`
   - Open it and fill in properties:
     - Ship Name: "Explorer One"
     - Ship Class: "Scout"
     - Hull Strength: 1000
     - Max Speed: 500
     - Cargo Capacity: 50

3. **Use the Template**:
   - Open `Assets/SpaceshipTemplates/PathfinderScout.yaml`
   - Copy values as reference
   - Adjust to your liking

4. **Save**: Ctrl+S

#### Create Your First Faction

1. **Create Data Asset**:
   - Right-click in Content Browser
   - "Miscellaneous" > "Data Asset"
   - Choose "FactionDataAsset"

2. **Configure**:
   - Name it: `DA_MyFaction`
   - Faction Name: "Star Traders"
   - Primary Color: Choose a color
   - Technology Level: 5
   - Economic Power: 7

3. **Add a Trait**:
   - In Traits section, click "+"
   - Trait Name: "Merchant Guild"
   - Modifier Value: 1.2

4. **Save**: Ctrl+S

### For Programmers

#### Explore the C++ Code

1. **Open Visual Studio** (or Xcode)
2. **Navigate to Source/Adastrea/**
3. **Start with**:
   - `Factions/FactionDataAsset.h` - Simple Data Asset example
   - `Ships/SpaceshipDataAsset.h` - Complex Data Asset
   - `Stations/SpaceStation.h` - Actor with module management

#### Create a Simple Blueprint Function

1. **Open any Data Asset header** (e.g., `FactionDataAsset.h`)
2. **Add a new function**:
   ```cpp
   UFUNCTION(BlueprintCallable, Category="My Category")
   int32 GetMyValue() const;
   ```
3. **Implement in .cpp file**:
   ```cpp
   int32 UFactionDataAsset::GetMyValue() const
   {
       return 42;
   }
   ```
4. **Compile**: Press Ctrl+Shift+B
5. **Test in Blueprint**: Right-click in Blueprint and search for your function

## Next Steps

### Essential Reading (30 minutes)

1. **[ARCHITECTURE.md](ARCHITECTURE.md)** - Understand project structure
2. **[Assets/DesignerOnboarding.md](Assets/DesignerOnboarding.md)** - Designer workflows
3. **[CONTRIBUTING.md](CONTRIBUTING.md)** - How to contribute

### Explore Systems (1-2 hours)

Pick one system to dive deep:

- **Spaceships**: [Assets/SpaceshipDataAssetGuide.md](Assets/SpaceshipDataAssetGuide.md)
- **Factions**: [Assets/BlueprintFactionAssetGuide.md](Assets/BlueprintFactionAssetGuide.md)
- **Trading**: [Assets/TradingSystemGuide.md](Assets/TradingSystemGuide.md)
- **AI**: [Assets/AI_QUICK_START.md](Assets/AI_QUICK_START.md)

### Join the Community

- **Found a bug?** Open an issue
- **Have an idea?** Start a discussion
- **Want to contribute?** Read [CONTRIBUTING.md](CONTRIBUTING.md)

## Common Issues

### Build Errors

**Problem**: "Cannot find Unreal Engine installation"
- **Solution**: Install Unreal Engine from Epic Games Launcher
- Verify installation in Engine Association settings

**Problem**: "Missing SDK"
- **Solution (Windows)**: Install Visual Studio 2022 with C++ game development workload
- **Solution (Mac)**: Install latest Xcode from App Store

**Problem**: "Out of memory during compilation"
- **Solution**: Close other applications, increase virtual memory

### Editor Issues

**Problem**: "Failed to load module"
- **Solution**: Right-click .uproject > "Generate project files" > Rebuild

**Problem**: "Cannot open map"
- **Solution**: Let the editor fully load all assets (check bottom-right progress)

**Problem**: "Shaders compiling forever"
- **Solution**: This is normal on first launch. Wait for completion (can take 10-30 minutes)

### Runtime Issues

**Problem**: "Crash on play"
- **Solution**: Check Output Log (Window > Developer Tools > Output Log)
- Look for error messages and report them

## Learning Resources

### Unreal Engine Basics
- [Official Unreal Engine Documentation](https://docs.unrealengine.com/)
- [Unreal Engine for Beginners](https://www.unrealengine.com/en-US/onlinelearning-courses)
- [Blueprint Quickstart Guide](https://docs.unrealengine.com/5.0/en-US/blueprints-quick-start-guide/)

### C++ with Unreal
- [Unreal Engine C++ Quick Start](https://docs.unrealengine.com/5.0/en-US/programming-quick-start-for-unreal-engine/)
- [UE4/UE5 C++ Tutorial](https://www.tomlooman.com/unreal-engine-cpp-tutorials/)

### Adastrea-Specific
- All guides in `Assets/` folder
- Example YAML templates for reference
- Blueprint examples in `Content/Blueprints/`

## Getting Help

1. **Check Documentation**: Most questions are answered in the Assets/ guides
2. **Search Issues**: Someone might have had the same problem
3. **Ask Questions**: Open an issue with the "question" label
4. **Join Discussions**: Participate in repository discussions

## Success Checklist

You're ready to start developing when you can:

- [ ] Launch the Unreal Editor without errors
- [ ] Open and edit a Data Asset
- [ ] Play the game in editor
- [ ] Create a simple Blueprint
- [ ] Build the project successfully
- [ ] Understand the project structure

## What's Next?

Choose your path:

**🎨 Artist/Designer Path**:
1. Read [Assets/DesignerOnboarding.md](Assets/DesignerOnboarding.md)
2. Create custom ships, factions, and personnel
3. Build Blueprint logic for gameplay
4. Test and iterate

**💻 Programmer Path**:
1. Read [ARCHITECTURE.md](ARCHITECTURE.md)
2. Explore the C++ source code
3. Add new features or fix bugs
4. Write tests and documentation

**📚 Documentation Path**:
1. Read [CONTRIBUTING.md](CONTRIBUTING.md)
2. Improve existing guides
3. Add new examples
4. Fix typos and clarity issues

---

**Welcome to Adastrea! Happy developing! 🚀✨**
