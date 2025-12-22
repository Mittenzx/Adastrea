# C++ API Quick Reference

Quick reference for the Adastrea Director C++ API. For detailed documentation, see [CPP_IMPLEMENTATION_GUIDE.md](CPP_IMPLEMENTATION_GUIDE.md).

## Including Headers

```cpp
#include "UEBridge.h"              // Core UE operations
#include "AssetHelpers.h"          // Asset import/creation
#include "StandardResult.h"        // Result handling
#include "AdastreaDirectorBlueprintLibrary.h"  // Blueprint library
```

## Common Operations

### Console Commands
```cpp
// Execute command
UUEBridge::ExecuteConsoleCommand("stat fps");

// Log message
UUEBridge::LogMessage("Hello World");
UUEBridge::LogMessage("Warning!", false, true);  // Warning
UUEBridge::LogMessage("Error!", true, false);    // Error
```

### Working with Results
```cpp
FAdastreaResult Result = UUEBridge::SpawnActor("Actor", Location, Rotation);

// Check status
if (Result.IsSuccess()) { /* ... */ }
if (Result.IsError()) { /* ... */ }

// Get details
FString Value = Result.Details["key"];
FString Message = Result.Message;
```

### Asset Operations
```cpp
// Get selected assets
TArray<FUEAssetInfo> Assets;
UUEBridge::GetSelectedAssets(Assets);

// Find assets by class
TArray<FUEAssetInfo> Materials;
UUEBridge::FindAssetsByClass("Material", "/Game", Materials);

// Load asset
UUEBridge::LoadAsset("/Game/Materials/M_MyMaterial");

// Save asset
UUEBridge::SaveAsset("/Game/Materials/M_MyMaterial");
```

### Actor Operations
```cpp
// Get all actors of class
TArray<FUEActorInfo> Actors;
UUEBridge::GetAllActorsOfClass("StaticMeshActor", Actors);

// Get selected actors
TArray<FUEActorInfo> Selected;
UUEBridge::GetSelectedActors(Selected);

// Spawn actor
FVector Loc(0, 0, 0);
FRotator Rot = FRotator::ZeroRotator;
UUEBridge::SpawnActor("StaticMeshActor", Loc, Rot, "MyActor");

// Delete actor
UUEBridge::DeleteActor("MyActor");
```

### Level Operations
```cpp
// Get current level
FAdastreaResult Result = UUEBridge::GetCurrentLevelName();
FString LevelName = Result.Details["level_name"];

// Load level
UUEBridge::LoadLevel("/Game/Maps/MyLevel");

// Save level
UUEBridge::SaveCurrentLevel();
```

### Asset Import
```cpp
// Import texture
UAssetHelpers::ImportTexture(
    "C:/Assets/texture.png",
    "/Game/Textures",
    "MyTexture"
);

// Import mesh
UAssetHelpers::ImportStaticMesh(
    "C:/Assets/mesh.fbx",
    "/Game/Meshes",
    "MyMesh"
);

// Import audio
UAssetHelpers::ImportAudio(
    "C:/Assets/sound.wav",
    "/Game/Audio",
    "MySound"
);
```

### Asset Creation
```cpp
// Create blueprint
UAssetHelpers::CreateBlueprint(
    "BP_MyActor",
    "Actor",
    "/Game/Blueprints"
);

// Create material
UAssetHelpers::CreateMaterial(
    "M_MyMaterial",
    "/Game/Materials"
);
```

### Editor Utilities
```cpp
// Get project directory
FAdastreaResult Result = UUEBridge::GetProjectDirectory();
FString ProjectDir = Result.Details["project_dir"];

// Get engine version
Result = UUEBridge::GetEngineVersion();
FString Version = Result.Details["engine_version"];
```

## Blueprint Usage

All functions are available in Blueprints under the **Adastrea** category:

### Common Nodes
- `Execute Console Command` - Run console commands
- `Get Selected Actors` - Get selected actors in level
- `Spawn Actor` - Create new actor
- `Import Texture` - Import texture file
- `Create Blueprint` - Create new blueprint
- `Is Result Success` - Check if operation succeeded
- `Get Result Detail` - Get detail from result

### Searching in Blueprint
1. Right-click in Blueprint graph
2. Type "Adastrea" to see all nodes
3. Or search for specific function (e.g., "spawn", "import")

## Error Handling Pattern

```cpp
FAdastreaResult Result = UUEBridge::SpawnActor(...);

if (Result.IsSuccess())
{
    // Success path
    UE_LOG(LogTemp, Log, TEXT("Success: %s"), *Result.Message);
    FString ActorName = Result.Details["actor_name"];
}
else
{
    // Error path
    UE_LOG(LogTemp, Error, TEXT("Failed: %s"), *Result.Message);
    
    // Log all error details
    for (const auto& Detail : Result.Details)
    {
        UE_LOG(LogTemp, Error, TEXT("%s: %s"), 
            *Detail.Key, *Detail.Value);
    }
}
```

## Batch Operations Pattern

```cpp
TArray<FVector> SpawnLocations = {
    FVector(0, 0, 0),
    FVector(100, 0, 0),
    FVector(200, 0, 0)
};

int32 SuccessCount = 0;
for (int32 i = 0; i < SpawnLocations.Num(); i++)
{
    FString Name = FString::Printf(TEXT("Actor_%d"), i);
    FAdastreaResult Result = UUEBridge::SpawnActor(
        "StaticMeshActor",
        SpawnLocations[i],
        FRotator::ZeroRotator,
        Name
    );
    
    if (Result.IsSuccess())
    {
        SuccessCount++;
    }
}

UE_LOG(LogTemp, Log, TEXT("Spawned %d/%d actors"), 
    SuccessCount, SpawnLocations.Num());
```

## Data Structures

### FAdastreaResult
```cpp
struct FAdastreaResult
{
    EAdastreaResultStatus Status;  // Success or Error
    FString Message;               // Human-readable message
    TMap<FString, FString> Details; // Additional data
    
    // Helpers
    bool IsSuccess() const;
    bool IsError() const;
};
```

### FUEAssetInfo
```cpp
struct FUEAssetInfo
{
    FString AssetName;   // Asset name
    FString AssetPath;   // Full path
    FString AssetClass;  // Class name
    int64 AssetSize;     // Size in bytes
};
```

### FUEActorInfo
```cpp
struct FUEActorInfo
{
    FString ActorName;   // Actor name
    FString ActorClass;  // Actor class
    FVector Location;    // World location
    FRotator Rotation;   // World rotation
    FVector Scale;       // World scale
};
```

## Performance Tips

1. **Use C++ for UE operations** - 10-50x faster than IPC
2. **Batch operations when possible** - Reduces overhead
3. **Check results** - Handle errors gracefully
4. **Cache frequently used data** - Avoid repeated queries

## Common Patterns

### Query and Process Pattern
```cpp
// Get all materials
TArray<FUEAssetInfo> Materials;
UUEBridge::FindAssetsByClass("Material", "/Game", Materials);

// Process each material
for (const FUEAssetInfo& Material : Materials)
{
    // Load and process
    UUEBridge::LoadAsset(Material.AssetPath);
    // ... do work ...
}
```

### Create and Configure Pattern
```cpp
// Create blueprint
FAdastreaResult Result = UAssetHelpers::CreateBlueprint(
    "BP_MyActor",
    "Actor",
    "/Game/Blueprints"
);

if (Result.IsSuccess())
{
    FString AssetPath = Result.Details["asset_path"];
    // Now load and configure it
    UUEBridge::LoadAsset(AssetPath);
    // ... configure blueprint ...
}
```

### Safe Import Pattern
```cpp
FString FilePath = "C:/Assets/texture.png";

// Check if file exists (optional)
IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
if (FileManager.FileExists(*FilePath))
{
    // Import
    FAdastreaResult Result = UAssetHelpers::ImportTexture(
        FilePath,
        "/Game/Textures",
        "ImportedTexture"
    );
    
    if (Result.IsSuccess())
    {
        FString AssetPath = Result.Details["asset_path"];
        UE_LOG(LogTemp, Log, TEXT("Imported: %s"), *AssetPath);
    }
}
```

## Blueprint Library Wrapper

For Blueprint-optimized access, use the library wrapper:

```cpp
#include "AdastreaDirectorBlueprintLibrary.h"

// Same functions, Blueprint-friendly
UAdastreaDirectorBlueprintLibrary::ExecuteConsoleCommand("stat fps");
UAdastreaDirectorBlueprintLibrary::SpawnActor(...);

// Helper functions
bool bSuccess = UAdastreaDirectorBlueprintLibrary::IsResultSuccess(Result);
FString Detail = UAdastreaDirectorBlueprintLibrary::GetResultDetail(Result, "key");
```

## Further Reading

- [CPP_IMPLEMENTATION_GUIDE.md](CPP_IMPLEMENTATION_GUIDE.md) - Complete API documentation
- [CPP_IMPLEMENTATION_SUMMARY.md](CPP_IMPLEMENTATION_SUMMARY.md) - Implementation overview
- [ExampleUsage.h](Source/AdastreaDirector/Public/ExampleUsage.h) - 10 complete examples
- [README.md](README.md) - Plugin overview

## Support

For issues or questions:
- GitHub Issues: https://github.com/Mittenzx/Adastrea-Director/issues
- Documentation: https://github.com/Mittenzx/Adastrea-Director/wiki
