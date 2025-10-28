# Contributing to Adastrea

First off, thank you for considering contributing to Adastrea! It's people like you that make Adastrea such a great project.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Workflow](#development-workflow)
- [Coding Standards](#coding-standards)
- [Submitting Changes](#submitting-changes)
- [Reporting Bugs](#reporting-bugs)
- [Feature Requests](#feature-requests)

## Code of Conduct

This project and everyone participating in it is governed by our commitment to fostering an open and welcoming environment. Please be respectful and constructive in all interactions.

## Getting Started

### Prerequisites

- **Unreal Engine 5.5** (check the .uproject file for the exact version)
- **Visual Studio 2022** (Windows) or **Xcode** (Mac) for C++ development
- **Git** for version control
- Basic knowledge of Unreal Engine, C++, and Blueprint scripting

### Setting Up Your Development Environment

1. **Fork and Clone**
   ```bash
   git clone https://github.com/YOUR_USERNAME/Adastrea.git
   cd Adastrea
   ```

2. **Validate Environment** (Recommended)
   ```bash
   # Use the automated setup validation scripts
   ./SetupCheck.sh              # Linux/Mac/WSL
   # or
   python SetupCheck.py         # Cross-platform
   ```
   
   These scripts verify:
   - Unreal Engine version compatibility
   - C++ compiler availability
   - Required project folders and files
   - Optional: Static code analysis with cppcheck
   
   See [README.md](README.md#robust-setup-validation) for detailed usage.

3. **Generate Project Files**
   - Right-click on the `.uproject` file and select "Generate Visual Studio project files"

4. **Build the Project**
   - Open the generated `.sln` file in Visual Studio
   - Build the project in Development Editor configuration
   - Or open the `.uproject` file directly in Unreal Engine and let it compile

5. **Verify Setup**
   - Launch the editor and ensure all systems load correctly
   - Check that all example Blueprints and Data Assets work
   - Refer to [Assets/PlaytestingChecklist.md](Assets/PlaytestingChecklist.md) for comprehensive verification

## Development Workflow

### Branching Strategy

- `main` - Stable, production-ready code
- `develop` - Integration branch for features
- `feature/*` - New features (e.g., `feature/new-ship-system`)
- `bugfix/*` - Bug fixes (e.g., `bugfix/faction-relationship-crash`)
- `docs/*` - Documentation updates

### Making Changes

1. **Create a Branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **Make Your Changes**
   - Write clean, well-documented code
   - Follow the coding standards below
   - Add or update tests as needed
   - Update documentation

3. **Test Your Changes**
   - Build the project without errors
   - Test in-editor functionality
   - Verify Blueprint integration works
   - Check that example Data Assets still work

4. **Commit Your Changes**
   ```bash
   git add .
   git commit -m "feat: Add detailed description of your changes"
   ```
   
   Use conventional commit messages:
   - `feat:` - New feature
   - `fix:` - Bug fix
   - `docs:` - Documentation changes
   - `refactor:` - Code refactoring
   - `test:` - Adding tests
   - `chore:` - Maintenance tasks

## Coding Standards

### C++ Code

#### Header Files (.h)

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "YourClassName.generated.h"

/**
 * Brief description of the class.
 * Detailed explanation of what this class does and when to use it.
 */
UCLASS(BlueprintType)
class ADASTREA_API UYourClassName : public UDataAsset
{
    GENERATED_BODY()

public:
    // Properties with clear descriptions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Your Category")
    float SomeValue;

    /**
     * Function description explaining what it does
     * @param ParamName Description of parameter
     * @return Description of return value
     */
    UFUNCTION(BlueprintCallable, Category="Your Category")
    bool YourFunction(int32 ParamName);
};
```

#### Best Practices

- **Use `UPROPERTY` macros properly**:
  - `EditAnywhere, BlueprintReadWrite` for designer-editable values
  - `BlueprintReadOnly` for values that shouldn't be changed in Blueprints
  - Always specify a `Category`
  - Use `meta=` for constraints (ClampMin, ClampMax, MultiLine, etc.)

- **Use `UFUNCTION` for Blueprint exposure**:
  - Mark functions `BlueprintCallable` if Blueprints should call them
  - Mark functions `BlueprintPure` for getter functions with no side effects
  - Add clear parameter and return value documentation

- **Naming Conventions**:
  - Classes: `UClassName` (U for UObject-derived), `AClassName` (A for Actor-derived), `FClassName` (F for structs)
  - Member variables: `CamelCase` (e.g., `MaxHealth`)
  - Boolean properties: Prefix with `b` (e.g., `bIsEnabled`)
  - Functions: `CamelCase` (e.g., `CalculateDamage()`)
  - Constants: `ALL_CAPS` or `kConstantName`

- **Comments**:
  - Document all public APIs with `/** */` Doxygen-style comments
  - Explain WHY, not WHAT (code should be self-documenting)
  - Add inline comments for complex logic

### Blueprint Organization

- **Naming Convention**: `BP_SystemName_Purpose` (e.g., `BP_Faction_TradeStation`)
- **Folder Structure**: Organize by system (Ships, Stations, UI, etc.)
- **Comments**: Add comment boxes in complex Blueprint graphs
- **Functions**: Keep Blueprint functions small and focused (max 20-30 nodes)
- **Variables**: Use descriptive names and categories

### Data Assets

- **Naming**: Use descriptive names indicating the asset type (e.g., `DA_Ship_Pathfinder`, `DA_Faction_SolarisUnion`)
- **Organization**: Group by system in Content Browser
- **Documentation**: Add descriptions to all properties
- **Templates**: Use YAML templates from `Assets/` folder as reference

## Submitting Changes

### Pull Request Process

1. **Update Documentation**
   - Update README.md if you add/change features
   - Add/update guides in `Assets/` folder
   - Include code comments and docstrings

2. **Ensure Quality**
   - Code compiles without errors or warnings
   - All systems work in-editor
   - Blueprint integration is tested
   - No crashes or critical bugs

3. **Create Pull Request**
   - Push your branch to your fork
   - Open a PR against the `develop` branch (not `main`)
   - Use a clear, descriptive title
   - Fill out the PR template completely
   - Reference any related issues

4. **PR Description Should Include**:
   - Summary of changes
   - Motivation and context
   - Type of change (bug fix, new feature, breaking change, etc.)
   - Testing performed
   - Screenshots (if UI changes)
   - Checklist of completed items

5. **Review Process**
   - Respond to feedback promptly
   - Make requested changes in new commits
   - Be open to suggestions and discussion

## Reporting Bugs

### Before Submitting a Bug Report

- Check the issue tracker for existing reports
- Verify you're using the latest version
- Test if the bug occurs in a clean project

### How to Submit a Good Bug Report

Include:
- **Title**: Clear, concise description
- **Environment**: OS, Unreal Engine version, hardware
- **Steps to Reproduce**: Exact steps to trigger the bug
- **Expected Behavior**: What should happen
- **Actual Behavior**: What actually happens
- **Screenshots/Logs**: Visual evidence or error logs
- **Reproducibility**: Always, sometimes, or once?

## Feature Requests

We love feature requests! Please provide:

- **Use Case**: What problem does this solve?
- **Proposed Solution**: How would you implement it?
- **Alternatives**: Other approaches you've considered
- **Impact**: Who benefits from this feature?
- **Examples**: Similar features in other projects

## Questions?

- **Documentation**: Check the `Assets/` folder for comprehensive guides
- **Issues**: Open an issue with the "question" label
- **Community**: Engage with other contributors

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

---

Thank you for contributing to Adastrea! 🚀✨
