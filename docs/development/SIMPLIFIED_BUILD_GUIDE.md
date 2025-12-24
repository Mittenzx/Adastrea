# Adastrea - Simplified Build Guide (MVP Focus)

**Purpose**: ONE recommended build path to reduce complexity  
**Target Audience**: MVP developers  
**Last Updated**: 2025-12-24  
**Based on**: Anti-Pattern #9 (Build System Complexity)

---

## 🎯 Philosophy Change

**OLD Approach**: Support 5 different build methods  
**NEW Approach**: ONE recommended path, document only that

**Why**: Build complexity killed productivity. Keep it simple.

---

## ✅ THE ONE RECOMMENDED BUILD PATH

### Prerequisites

**Required**:
1. **Unreal Engine 5.6** (full editor installation)
2. **Visual Studio 2022** (Community Edition or better)
3. **Git** (for version control)

**Platforms Supported**: Windows (primary), Mac, Linux

---

## 📋 Step-by-Step Build Instructions

### Step 1: Clone Repository

```bash
git clone https://github.com/Mittenzx/Adastrea.git
cd Adastrea
```

### Step 2: Generate Project Files

**Windows**:
```bash
# Right-click Adastrea.uproject
# Select "Generate Visual Studio project files"
```

**Alternative (Command Line)**:
```bash
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" AdastreaEditor Win64 Development -project="%CD%\Adastrea.uproject"
```

### Step 3: Open in Visual Studio

1. Open `Adastrea.sln` in Visual Studio 2022
2. Set build configuration to **"Development Editor"**
3. Set platform to **Win64**

### Step 4: Build

**Method 1 (Recommended)**: 
- Press `Ctrl+Shift+B` or select "Build Solution"

**Method 2**: 
- Right-click Adastrea project → Build

**Expected Result**: 
- Build completes in 5-15 minutes (first time)
- 0 errors, minimal warnings
- If errors occur, see Troubleshooting section

### Step 5: Launch Editor

**Method 1**: 
- Double-click `Adastrea.uproject`

**Method 2**: 
- Press `F5` in Visual Studio (starts with debugger)

---

## 🛠️ Common Build Issues

### Issue 1: "Unreal Engine 5.6 not found"

**Solution**:
1. Install UE 5.6 from Epic Games Launcher
2. Or update path in `Adastrea.uproject` to your UE installation

### Issue 2: "Missing Visual Studio components"

**Solution**:
Install these VS 2022 workloads:
- "Game development with C++"
- ".NET desktop development"
- "Desktop development with C++"

### Issue 3: "Build errors about missing includes"

**Solution**:
```bash
# Regenerate project files
Right-click Adastrea.uproject → "Generate Visual Studio project files"
```

### Issue 4: "Out of memory during compile"

**Solution**:
- Close other applications
- Reduce parallel builds in VS: Tools → Options → Projects and Solutions → Build and Run → Set "maximum number of parallel project builds" to 2-4

---

## ⚙️ Build Configurations

### Development Editor (Recommended)
- **Use For**: Daily development
- **Optimization**: Minimal
- **Debug Info**: Full
- **Speed**: Fast compile, slower runtime

### DebugGame Editor
- **Use For**: Deep debugging
- **Optimization**: None
- **Debug Info**: Maximum
- **Speed**: Very slow

### Shipping
- **Use For**: Final builds only (Week 12+)
- **Optimization**: Maximum
- **Debug Info**: None
- **Speed**: Slow compile, fast runtime

**For MVP Development**: Use "Development Editor" only.

---

## 🚫 Deprecated Build Methods

The following build methods are **NO LONGER SUPPORTED** to reduce complexity:

❌ **Docker Builds** - Requires Epic membership, adds complexity  
❌ **UE Build Tools Only** - Incomplete, causes confusion  
❌ **Alternative Build Scripts** - Too many variations  
❌ **GitHub Actions Full Build** - Use for CI only, not local dev  

**Removed Files** (if they exist):
- `build_with_ue_tools.sh` - Deprecated
- `setup_ue_build_tools.sh` - Deprecated
- Multiple "QUICK FIX" guides - Consolidated here

---

## 📊 Build Performance

**Expected Build Times** (on typical dev machine):

| Build Type | First Build | Incremental |
|-----------|-------------|-------------|
| Clean build | 10-15 min | - |
| Single file change | - | 10-30 sec |
| Full rebuild | 8-12 min | - |

**If your builds are slower**: 
- Check VS parallel build settings
- Close other applications
- Ensure SSD (not HDD)
- Check antivirus exclusions

---

## 🎯 MVP-Specific Build Notes

### What to Build For MVP

**Week 1-4 (Prototype)**:
- Build once, iterate in Blueprints
- Minimize C++ changes
- Use hot reload when possible

**Week 5-8 (Structured)**:
- Add Data Asset classes only
- Minimal new C++ code
- Focus on Blueprint content

**Week 9-12 (Polish)**:
- No new systems
- Bug fixes only
- Polish existing code

### Performance Monitoring

Add FPS counter during development:
```cpp
// In console (~ key):
stat fps
stat unit
```

Track memory:
```cpp
stat memory
memreport
```

---

## 🔧 Quick Commands Reference

### Regenerate Project Files
```bash
# Right-click .uproject → "Generate Visual Studio project files"
```

### Clean Build
```bash
# Delete these folders:
rm -rf Binaries/ Intermediate/ Saved/
# Then regenerate and rebuild
```

### Verify Install
```bash
# Launch editor and check console for errors
# If editor opens → build successful
```

---

## 📚 Additional Resources

**Unreal Engine Documentation**:
- [Building Unreal Engine from Source](https://dev.epicgames.com/documentation/en-us/unreal-engine/building-unreal-engine-from-source)

**Visual Studio**:
- [Visual Studio 2022 Installation](https://visualstudio.microsoft.com/downloads/)

---

## ✅ Build Checklist

Before starting development:
- [ ] UE 5.6 installed
- [ ] Visual Studio 2022 installed with game dev workload
- [ ] Repository cloned
- [ ] Project files generated
- [ ] Solution builds successfully (0 errors)
- [ ] Editor launches without errors

---

## 🚨 When to Ask for Help

**Ask for help if**:
- Build fails with 10+ errors
- Build takes >30 minutes
- Editor crashes on launch
- Consistent crashes in gameplay

**Don't spend >1 hour troubleshooting alone** - ask the team.

---

## 📝 Anti-Pattern Notes

This guide addresses:
- **Anti-Pattern #9**: Build System Complexity
- **Anti-Pattern #13**: Multiple guides for same task

**Key Improvements**:
- ONE build path (not 5)
- ONE guide (not 3-4 different guides)
- Clear, step-by-step instructions
- Consolidated troubleshooting

---

**Next Steps**: Once build works, proceed to [MVP Development](README.md)

---

**Last Updated**: 2025-12-24  
**Maintained By**: Adastrea MVP Team
