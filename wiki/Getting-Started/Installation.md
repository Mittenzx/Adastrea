# Installation Guide

Comprehensive installation instructions for Adastrea development environment.

> **Quick Start Available**: For a faster setup, see the [Quick Start Guide](Quick-Start.md)

## System Requirements

### Minimum Requirements
- **OS**: Windows 10/11 64-bit, macOS 12+, or Linux (Ubuntu 20.04+)
- **CPU**: Quad-core processor (Intel i5 or AMD Ryzen 5)
- **RAM**: 16 GB
- **GPU**: DirectX 11/12 compatible with 4GB VRAM
- **Storage**: 50 GB free space (SSD recommended)

### Recommended Requirements
- **OS**: Windows 11 64-bit
- **CPU**: 8-core processor (Intel i7/i9 or AMD Ryzen 7/9)
- **RAM**: 32 GB
- **GPU**: RTX 2060 or better with 8GB+ VRAM
- **Storage**: 100 GB free space on NVMe SSD

## Prerequisites Installation

### 1. Unreal Engine 5.6

**Windows/Mac**:
1. Install [Epic Games Launcher](https://www.epicgames.com/store/download)
2. Sign in or create an Epic Games account
3. Go to the "Unreal Engine" tab
4. Click "Install Engine" and select version 5.6
5. Choose installation location (requires ~40GB)
6. Wait for download and installation

**Linux**:
```bash
# Follow Epic's official Linux installation guide
# https://docs.unrealengine.com/5.6/en-US/linux-development/
```

### 2. C++ Development Tools

**Windows - Visual Studio 2022**:
1. Download [Visual Studio 2022 Community](https://visualstudio.microsoft.com/downloads/)
2. Run installer
3. Select "Desktop development with C++" workload
4. Additional components to install:
   - C++ profiling tools
   - Windows 10/11 SDK
   - C++ CMake tools
5. Complete installation (requires ~10GB)

**Mac - Xcode**:
```bash
# Install Xcode from App Store or:
xcode-select --install
```

**Linux - Build Tools**:
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential clang

# Fedora/RHEL
sudo dnf install gcc gcc-c++ clang
```

### 3. Git

**Windows**:
1. Download [Git for Windows](https://git-scm.com/download/win)
2. Run installer with default options
3. Recommended: Enable "Git from the command line"

**Mac**:
```bash
# Install via Homebrew (recommended)
brew install git

# Or via Xcode command line tools
xcode-select --install
```

**Linux**:
```bash
# Ubuntu/Debian
sudo apt-get install git

# Fedora/RHEL
sudo dnf install git
```

### 4. Python 3.8+ (Optional but Recommended)

For automation scripts and testing:

**Windows**:
1. Download from [python.org](https://www.python.org/downloads/)
2. Run installer
3. ✓ Check "Add Python to PATH"
4. Complete installation

**Mac/Linux**:
```bash
# Usually pre-installed, verify version:
python3 --version

# If not installed:
# Mac (via Homebrew)
brew install python3

# Linux
sudo apt-get install python3 python3-pip  # Ubuntu/Debian
sudo dnf install python3 python3-pip      # Fedora/RHEL
```

## Project Installation

### 1. Clone Repository

```bash
# HTTPS (recommended for most users)
git clone https://github.com/Mittenzx/Adastrea.git
cd Adastrea

# SSH (for contributors with SSH keys)
git clone git@github.com:Mittenzx/Adastrea.git
cd Adastrea
```

### 2. Run Setup Validation

**Automated validation checks your environment**:

```bash
# Linux/Mac/WSL
./SetupCheck.sh

# Windows PowerShell / Cross-platform
python SetupCheck.py

# With verbose output
python SetupCheck.py --verbose

# With static analysis (requires cppcheck)
python SetupCheck.py --analyze
```

This validates:
- Unreal Engine installation
- Compiler availability  
- Project structure
- Required files
- Optional: Code quality checks

### 3. Generate Project Files

**Windows**:
1. Right-click `Adastrea.uproject`
2. Select "Generate Visual Studio project files"
3. Wait for generation to complete

**Mac**:
1. Right-click `Adastrea.uproject`
2. Select "Generate Xcode project files"
3. Wait for generation to complete

**Linux**:
```bash
# Use Unreal's GenerateProjectFiles script
cd /path/to/UnrealEngine
./GenerateProjectFiles.sh /path/to/Adastrea/Adastrea.uproject
```

### 4. Build the Project

**Method 1: Using IDE**

*Windows - Visual Studio*:
1. Open `Adastrea.sln`
2. Select "Development Editor" configuration
3. Build > Build Solution (Ctrl+Shift+B)
4. Wait for compilation (first build takes 10-30 minutes)

*Mac - Xcode*:
1. Open `Adastrea.xcworkspace`
2. Select "Development Editor" scheme
3. Product > Build (Cmd+B)
4. Wait for compilation

**Method 2: Using Unreal Engine**

1. Double-click `Adastrea.uproject`
2. Engine will detect missing binaries
3. Click "Yes" to rebuild modules
4. Wait for compilation
5. Editor launches automatically when ready

### 5. Post-Installation Verification

**Check Assets Load**:
1. In Unreal Editor, open Content Browser
2. Navigate to `Content/DataAssets/`
3. Verify example assets open without errors

**Run Validation Script**:
```python
# In Unreal Editor > Tools > Python Console
import unreal
exec(open('AdastreaAssetValidator.py').read())
```

**Test Play Mode**:
1. Open a test map (e.g., `Content/Maps/TestLevel`)
2. Click Play (Alt+P)
3. Verify no critical errors in Output Log
4. Test basic movement and systems

**Run Automated Tests**:
```bash
# Smoke tests
python AutomationRunner.py --smoke-test

# Screenshot tests
python AutomationRunner.py --screenshot-test

# All tests
python AutomationRunner.py --all
```

## Optional Components

### cppcheck (Static Analysis)
```bash
# Windows (via Chocolatey)
choco install cppcheck

# Mac
brew install cppcheck

# Linux
sudo apt-get install cppcheck  # Ubuntu/Debian
sudo dnf install cppcheck      # Fedora/RHEL
```

### Documentation Tools

For generating and viewing documentation:
```bash
# Python packages
pip install pyyaml markdown
```

## Troubleshooting

### "Unreal Engine not found"
- Verify installation path in Epic Games Launcher
- Check system PATH includes engine directory
- Restart computer after installation

### "Missing Visual Studio workload"
- Re-run Visual Studio Installer
- Verify "Desktop development with C++" is installed
- Install Windows SDK if prompted

### "Out of disk space during build"
- Minimum 50GB required (100GB recommended)
- Clear `Intermediate` and `Saved` folders if rebuilding
- Use disk cleanup tools

### "Build errors on first compile"
- Update Visual Studio to latest version
- Delete `Intermediate`, `Binaries`, `.vs` folders
- Regenerate project files
- Rebuild solution

### "Python scripts not working"
- Verify Python 3.8+ is installed: `python --version`
- Check Python is in PATH
- Install required packages: `pip install -r requirements.txt`

## Next Steps

✅ **Installation Complete!** Choose your path:

**For Designers**:
- [Designer Onboarding](Designer-Onboarding.md)
- [Content Creation Guide](Content-Creation.md)
- [Blueprint Beginners Guide](../Blueprint-Development/Blueprint-Beginners-Guide.md)

**For Developers**:
- [Architecture Overview](../Developer-Guides/Architecture.md)
- [Code Style Guide](../Developer-Guides/Code-Style.md)
- [Contributing Guidelines](../Developer-Guides/Contributing.md)

**For Everyone**:
- [First Steps Guide](First-Steps.md)
- [Core Systems Overview](../Core-Systems/README.md)
- [Testing Guide](../Automation-Testing/Testing-Guide.md)

## Getting Help

- **Setup Issues**: Check [SetupCheck.py output](../../SetupCheck.py)
- **Build Problems**: See [Build Troubleshooting](#troubleshooting)
- **Documentation**: [Documentation Index](../Reference/Documentation-Index.md)
- **Community**: [GitHub Discussions](https://github.com/Mittenzx/Adastrea/discussions)

---

*Estimated Installation Time: 1-3 hours (depending on download speeds and hardware)*

*See also: [Quick Start Guide](Quick-Start.md) for a faster setup*
