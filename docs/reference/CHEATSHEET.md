# Developer Cheat Sheet

Quick reference for common Adastrea development tasks.

## Quick Links

- [QUICKSTART.md](QUICKSTART.md) - Setup guide
- [ARCHITECTURE.md](ARCHITECTURE.md) - Project structure
- [CODE_STYLE.md](CODE_STYLE.md) - Coding standards
- [CONTRIBUTING.md](CONTRIBUTING.md) - How to contribute

## Common Commands

### Git Workflow
```bash
# Create feature branch
git checkout -b feature/my-feature

# Check status
git status

# Stage and commit
git add .
git commit -m "feat: Add new feature"

# Push to remote
git push origin feature/my-feature

# Update from main
git checkout main
git pull
git checkout feature/my-feature
git merge main
```

### Building

**UnrealBuildTool**:
```bash
# Windows (from project root)
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" AdastreaEditor Win64 Development -Project="%CD%\Adastrea.uproject"

# Mac
"/Users/Shared/Epic Games/UE_5.6/Engine/Build/BatchFiles/Mac/Build.sh" AdastreaEditor Mac Development -Project="$(pwd)/Adastrea.uproject"

# Linux
"~/UnrealEngine/Engine/Build/BatchFiles/Linux/Build.sh" AdastreaEditor Linux Development -Project="$(pwd)/Adastrea.uproject"
```

See [BUILD_FILES_MOVED.md](BUILD_FILES_MOVED.md) for information about build scripts.

### Cleaning Build
```bash
# Delete these folders to force a clean build:
Binaries/
Intermediate/
Saved/
.vs/

# Then regenerate project files
```

## Data Asset Creation

### In C++

**1. Create Header (e.g., MyDataAsset.h)**
```cpp
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MyDataAsset.generated.h"

UCLASS(BlueprintType)
class ADASTREA_API UMyDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
    FText DisplayName;

    UFUNCTION(BlueprintCallable, Category="My Category")
    void MyFunction();
};
```

**2. Implement (e.g., MyDataAsset.cpp)**
```cpp
#include "MyDataAsset.h"

void UMyDataAsset::MyFunction()
{
    // Implementation
}
```

**3. Compile** (Ctrl+Shift+B in Visual Studio)

**4. Create in Editor**
- Right-click in Content Browser
- Miscellaneous > Data Asset
- Select your class

### In Blueprint

**Create Blueprint based on C++ Class**:
1. Right-click in Content Browser
2. Blueprint Class
3. Select parent class (e.g., ASpaceStation)
4. Name it (e.g., BP_Station_Military)

## Common Tasks

### Add a New System

1. **Create C++ Classes**:
   - Header in `Source/Adastrea/NewSystem/`
   - Implementation in same folder
   - Follow Data Asset pattern

2. **Document**:
   - Create `Assets/NewSystemGuide.md`
   - Add templates to `Assets/NewSystemTemplates/`
   - Update README.md

3. **Test**:
   - Create example Data Assets
   - Test Blueprint integration
   - Verify all functions work

### Expose Function to Blueprint

```cpp
// In header
UFUNCTION(BlueprintCallable, Category="My Category")
ReturnType FunctionName(ParamType Param);

// In cpp
ReturnType UMyClass::FunctionName(ParamType Param)
{
    // Implementation
    return Result;
}
```

### Add Property to Data Asset

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats",
          meta=(ClampMin="0", ClampMax="100"))
float MyProperty;
```

### Create Custom Enum

```cpp
UENUM(BlueprintType)
enum class EMyEnum : uint8
{
    Option1 UMETA(DisplayName="Option One"),
    Option2 UMETA(DisplayName="Option Two"),
    Option3 UMETA(DisplayName="Option Three")
};
```

### Create Custom Struct

```cpp
USTRUCT(BlueprintType)
struct FMyStruct
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Name;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Value;

    FMyStruct()
        : Name(FText::FromString(TEXT("Default")))
        , Value(0.0f)
    {}
};
```

> **Note**: Use `BlueprintReadOnly` for configuration properties. Only use `BlueprintReadWrite` when Blueprints need to modify values at runtime.

## Blueprint Patterns

### Get Data from Data Asset
```
Get [Property] → Result
```

### Call C++ Function from Blueprint
```
[Object Reference] → [Function Name] → Result
```

### Check Trait/Relationship
```
[Faction] → Has Trait → TraitID → Result (bool)
[Faction] → Is Allied With → OtherFactionID → Result (bool)
```

### Rating Calculations
```
[Ship Data Asset] → Get Combat Rating → Result (float)
[Ship Data Asset] → Is Suitable For Role → Role (string) → Result (bool)
```

## File Locations

### Source Code
```
Source/Adastrea/
├── AI/              - AI logic
├── Characters/      - Personnel
├── Factions/        - Faction system
├── Materials/       - Materials
├── Player/          - Homeworlds
├── Ships/           - Spaceships
├── Stations/        - Space stations
├── Trading/         - Trading economy
└── UI/              - User interface
```

### Documentation
```
Assets/
├── *Guide.md            - System guides
├── *Workflow.md         - Designer workflows
├── *Templates.md        - Reference docs
├── *Templates/          - YAML templates
└── *SUMMARY.md          - Tech summaries
```

### Content
```
Content/
├── Blueprints/      - Blueprint classes
├── DataAssets/      - Data Asset instances
├── Maps/            - Level maps
├── Materials/       - Materials and instances
├── Meshes/          - Static/skeletal meshes
├── Textures/        - Texture assets
└── UI/              - Widget blueprints
```

## Common Issues & Solutions

### Compile Errors

**"Cannot find include file"**
- Check #include paths
- Ensure module dependencies in .Build.cs

**"Unresolved external symbol"**
- Check if function is declared in header
- Check if function is implemented in .cpp
- Verify module is linked

### Blueprint Issues

**"Function not found"**
- Ensure function has UFUNCTION macro
- Check BlueprintCallable/BlueprintPure
- Rebuild C++ code

**"Property not editable"**
- Check UPROPERTY macro
- Verify EditAnywhere or VisibleAnywhere
- Check access specifier (public/protected)

### Editor Crashes

**On Launch**:
- Delete Intermediate/ and Binaries/
- Regenerate project files
- Rebuild from IDE

**During Play**:
- Check Output Log for errors
- Look for null pointer access
- Verify all references are valid

## Performance Tips

### C++
- Mark const functions with `const`
- Use references for large parameters: `const FMyStruct& Param`
- Cache expensive calculations
- Use `TArray::Reserve()` for known sizes

### Blueprint
- Avoid Tick when possible (use Timers)
- Cache node results in variables
- Use Event-driven design
- Minimize Blueprint complexity

## Testing Checklist

- [ ] Code compiles without warnings
- [ ] All UPROPERTYs are accessible in editor
- [ ] All UFUNCTIONs work in Blueprint
- [ ] Data Assets can be created and configured
- [ ] Changes don't break existing functionality
- [ ] Documentation is updated
- [ ] Example assets are created

## Debugging Tools

### C++ Debugging
```cpp
// Print to log
UE_LOG(LogTemp, Warning, TEXT("Value: %f"), MyFloat);
UE_LOG(LogTemp, Error, TEXT("Error occurred!"));

// Print to screen
if (GEngine)
{
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Debug Message"));
}

// Check conditions
check(Condition); // Hard assert
checkSlow(Condition); // Debug builds only
ensure(Condition); // Warning if false
```

### Blueprint Debugging
- Set breakpoints (F9)
- Watch variables in Debugger panel
- Print String nodes
- Enable "Show Debug Info" on actors

## Useful Shortcuts

### Unreal Editor
- **Play**: Alt+P
- **Simulate**: Alt+S
- **Stop**: Esc
- **Compile Blueprints**: Ctrl+S (in BP editor)
- **Content Browser**: Ctrl+Space
- **Place Actor**: Drag from Content Browser

### Visual Studio
- **Build**: Ctrl+Shift+B
- **Start Debugging**: F5
- **Toggle Breakpoint**: F9
- **Step Over**: F10
- **Step Into**: F11
- **Find in Files**: Ctrl+Shift+F

### VS Code
- **Build**: Ctrl+Shift+B
- **Command Palette**: Ctrl+Shift+P
- **Go to Definition**: F12
- **Find References**: Shift+F12

## Resources

### Documentation
- [Unreal Engine Docs](https://docs.unrealengine.com/)
- [C++ API Reference](https://docs.unrealengine.com/5.6/en-US/API/)
- [Blueprint API](https://docs.unrealengine.com/5.6/en-US/BlueprintAPI/)

### Learning
- [Unreal Engine Learning Portal](https://www.unrealengine.com/en-US/onlinelearning-courses)
- [Tom Looman's C++ Tutorials](https://www.tomlooman.com/)
- [UE Community Wiki](https://unrealcommunity.wiki/)

### Community
- [Unreal Engine Forums](https://forums.unrealengine.com/)
- [r/unrealengine](https://www.reddit.com/r/unrealengine/)
- [Unreal Slackers Discord](https://unrealslackers.org/)

---

**Keep this cheat sheet handy while developing!** 📚✨
