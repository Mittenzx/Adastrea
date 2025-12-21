# Adastrea Build Errors & Warnings

This document lists the build errors and warnings encountered during compilation of the Adastrea project.

---

## Visual Studio Project Generation Errors

### CS0101: Duplicate Namespace Error in AdastreaDirectorEditor.Build.cs

**Error Message:**
```
Compiled assembly file 'C:\Unreal Projects\Adastrea\Intermediate\Build\BuildRules\AdastreaModuleRules.dll' appears to be for a newer CLR version or is otherwise invalid.
Unreal Build Tool will try to recompile this assembly now.
(Exception: Bad IL format. The format of the file 'C:\Unreal Projects\Adastrea\Intermediate\Build\BuildRules\AdastreaModuleRules.dll' is invalid.)

C:\Unreal Projects\Adastrea\Plugins\AdastreaDirector_Built\HostProject\Plugins\AdastreaDirector\Source\AdastreaDirectorEditor\AdastreaDirectorEditor.Build.cs(5,14): error CS0101: The namespace...
```

**Root Cause:**
This error occurs when there are duplicate or conflicting plugin structures in your project directory. The error message shows a path like `Plugins\AdastreaDirector_Built\HostProject\Plugins\AdastreaDirector`, which indicates a nested plugin structure where:
1. A built/packaged version of the AdastreaDirector plugin exists (AdastreaDirector_Built)
2. This conflicts with the source version of the plugin

**Solutions:**

1. **Clean Intermediate Build Files (Recommended First Step):**
   ```batch
   :: Windows
   del /s /q Intermediate\Build\BuildRules\*.dll
   del /s /q Binaries\Win64\*.dll
   del /s /q Plugins\AdastreaDirector\Binaries\*.dll
   del /s /q Plugins\AdastreaDirector\Intermediate\*.dll
   ```

2. **Remove Duplicate Plugin Folders:**
   - Check for and remove any folders named `AdastreaDirector_Built` or similar
   - Only keep the source plugin in `Plugins/AdastreaDirector/`
   - Delete any `HostProject` folders inside plugin directories

3. **Regenerate Project Files:**
   ```batch
   :: Right-click on Adastrea.uproject → "Generate Visual Studio project files"
   :: Or use command line:
   "C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" AdastreaEditor Win64 Development -project="C:\Path\To\Adastrea.uproject" -WaitMutex -FromMsBuild
   ```

4. **Clear Additional Build Artifacts:**
   - Delete the entire `Intermediate` folder
   - Delete the entire `Binaries` folder  
   - Delete `.vs` folder (Visual Studio user settings)
   - Regenerate project files

5. **Verify Plugin Structure:**
   Correct structure should be:
   ```
   Adastrea/
   ├── Plugins/
   │   ├── AdastreaDirector/          ← Source plugin (keep this)
   │   │   ├── AdastreaDirector.uplugin
   │   │   ├── Source/
   │   │   └── Content/
   │   ├── UnrealMCP/
   │   └── vc-ue-extensions/
   ```

   **Remove** any of these if present:
   - `Plugins/AdastreaDirector_Built/`
   - `Plugins/*/HostProject/`
   - Any duplicate plugin folders

**Prevention:**
- Add these entries to your `.gitignore`:
  ```
  # Plugin build artifacts
  *_Built/
  HostProject/
  Intermediate/Build/BuildRules/
  ```

---

## Errors

| Severity | Code   | Description                                                                                                                            | Project   | File                                                                                                                                  | Line | Details |
|----------|--------|----------------------------------------------------------------------------------------------------------------------------------------|-----------|---------------------------------------------------------------------------------------------------------------------------------------|------|---------|
| Error    | C4430  | missing type specifier - int assumed. Note: C++ does not support default-int                                                           | Adastrea  | C:\Unreal Projects\Adastrea\Intermediate\Build\Win64\UnrealEditor\Inc\Adastrea\UHT\AdastreaPlayerController.gen.cpp                    | 24   |         |
| Error    | C2146  | syntax error: missing ';' before identifier 'UClass'                                                                                   | Adastrea  | C:\Unreal Projects\Adastrea\Intermediate\Build\Win64\UnrealEditor\Inc\Adastrea\UHT\AdastreaPlayerController.gen.cpp                    | 24   |         |
| Error    | C4430  | missing type specifier - int assumed. Note: C++ does not support default-int                                                           | Adastrea  | C:\Unreal Projects\Adastrea\Intermediate\Build\Win64\UnrealEditor\Inc\Adastrea\UHT\AdastreaPlayerController.gen.cpp                    | 25   |         |
| Error    | C2086  | 'int STATIONEDITOR_API': redefinition                                                                                                 | Adastrea  | C:\Unreal Projects\Adastrea\Intermediate\Build\Win64\UnrealEditor\Inc\Adastrea\UHT\AdastreaPlayerController.gen.cpp                    | 25   |         |
| Error    | C2146  | syntax error: missing ';' before identifier 'UClass'                                                                                   | Adastrea  | C:\Unreal Projects\Adastrea\Intermediate\Build\Win64\UnrealEditor\Inc\Adastrea\UHT\AdastreaPlayerController.gen.cpp                    | 25   |         |
| Error    | C2065  | 'Z_Construct_UClass_UStationEditorWidget_NoRegister': undeclared identifier                                                            | Adastrea  | C:\Unreal Projects\Adastrea\Intermediate\Build\Win64\UnrealEditor\Inc\Adastrea\UHT\AdastreaPlayerController.gen.cpp                    | 234  |         |
| Error    | C2737  | 'NewProp_ReturnValue': const object must be initialized                                                                               | Adastrea  | C:\Unreal Projects\Adastrea\Intermediate\Build\Win64\UnrealEditor\Inc\Adastrea\UHT\AdastreaPlayerController.gen.cpp                    | 234  |         |
| Error    | C2065  | 'Z_Construct_UClass_UStationEditorWidget_NoRegister': undeclared identifier                                                            | Adastrea  | C:\Unreal Projects\Adastrea\Intermediate\Build\Win64\UnrealEditor\Inc\Adastrea\UHT\AdastreaPlayerController.gen.cpp                    | 739  |         |
| Error    | C2737  | 'NewProp_StationEditorWidgetClass': const object must be initialized                                                                  | Adastrea  | C:\Unreal Projects\Adastrea\Intermediate\Build\Win64\UnrealEditor\Inc\Adastrea\UHT\AdastreaPlayerController.gen.cpp                    | 739  |         |
| Error    | C2065  | 'Z_Construct_UClass_UStationModuleCatalog_NoRegister': undeclared identifier                                                           | Adastrea  | C:\Unreal Projects\Adastrea\Intermediate\Build\Win64\UnrealEditor\Inc\Adastrea\UHT\AdastreaPlayerController.gen.cpp                    | 740  |         |
| Error    | C2737  | 'NewProp_ModuleCatalog': const object must be initialized                                                                             | Adastrea  | C:\Unreal Projects\Adastrea\Intermediate\Build\Win64\UnrealEditor\Inc\Adastrea\UHT\AdastreaPlayerController.gen.cpp                    | 740  |         |
| Error    | C2065  | 'Z_Construct_UClass_UStationEditorWidget_NoRegister': undeclared identifier                                                            | Adastrea  | C:\Unreal Projects\Adastrea\Intermediate\Build\Win64\UnrealEditor\Inc\Adastrea\UHT\AdastreaPlayerController.gen.cpp                    | 747  |         |
| Error    | C2737  | 'NewProp_StationEditorWidget': const object must be initialized                                                                       | Adastrea  | C:\Unreal Projects\Adastrea\Intermediate\Build\Win64\UnrealEditor\Inc\Adastrea\UHT\AdastreaPlayerController.gen.cpp                    | 747  |         |
| Error    | C2065  | 'FireRate': undeclared identifier                                                                                                     | Adastrea  | C:\Unreal Projects\Adastrea\Source\Adastrea\Combat\WeaponDataAsset.cpp                                                                 | 184  |         |
| Error    | C2059  | syntax error: ')'                                                                                                                     | Adastrea  | C:\Unreal Projects\Adastrea\Source\Adastrea\Combat\WeaponDataAsset.cpp                                                                 | 191  |         |
| Error    | C2143  | syntax error: missing ';' before '{'                                                                                                  | Adastrea  | C:\Unreal Projects\Adastrea\Source\Adastrea\Combat\WeaponDataAsset.cpp                                                                 | 192  |         |
| Error    | C2143  | syntax error: missing ';' before '}'                                                                                                  | Adastrea  | C:\Unreal Projects\Adastrea\Source\Adastrea\Combat\WeaponDataAsset.cpp                                                                 | 195  |         |
| Error    | C2143  | syntax error: missing ';' before '{'                                                                                                  | Adastrea  | C:\Unreal Projects\Adastrea\Source\Adastrea\Combat\WeaponDataAsset.cpp                                                                 | 201  |         |
| Error    | C2947  | expecting '>' to terminate template-argument-list, found '<='                                                                         | Adastrea  | C:\Unreal Projects\Adastrea\Source\Adastrea\Combat\WeaponDataAsset.cpp                                                                 | 202  |         |
| Error    | C2059  | syntax error: 'constant'                                                                                                              | Adastrea  | C:\Unreal Projects\Adastrea\Source\Adastrea\Combat\WeaponDataAsset.cpp                                                                 | 202  |         |
| Error    | C2143  | syntax error: missing ';' before '{'                                                                                                  | Adastrea  | C:\Unreal Projects\Adastrea\Source\Adastrea\Combat\WeaponDataAsset.cpp                                                                 | 203  |         |
| Error    | C2059  | syntax error: 'if'                                                                                                                    | Adastrea  | C:\Unreal Projects\Adastrea\Source\Adastrea\Combat\WeaponDataAsset.cpp                                                                 | 210  |         |
| Error    | C2143  | syntax error: missing ';' before '{'                                                                                                  | Adastrea  | C:\Unreal Projects\Adastrea\Source\Adastrea\Combat\WeaponDataAsset.cpp                                                                 | 211  |         |
| Error    | C2447  | '{': missing function header (old-style formal list?)                                                                                 | Adastrea  | C:\Unreal Projects\Adastrea\Source\Adastrea\Combat\WeaponDataAsset.cpp                                                                 | 211  |         |
| Error    | C2059  | syntax error: 'if'                                                                                                                    | Adastrea  | C:\Unreal Projects\Adastrea\Source\Adastrea\Combat\WeaponDataAsset.cpp                                                                 | 217  |         |
| Error    | C2143  | syntax error: missing ';' before '{'                                                                                                  | Adastrea  | C:\Unreal Projects\Adastrea\Source\Adastrea\Combat\WeaponDataAsset.cpp                                                                 | 218  |         |
| Error    | C2447  | '{': missing function header (old-style formal list?)                                                                                 | Adastrea  | C:\Unreal Projects\Adastrea\Source\Adastrea\Combat\WeaponDataAsset.cpp                                                                 | 218  |         |
| Error    | C2059  | syntax error: 'if'                                                                                                                    | Adastrea  | C:\Unreal Projects\Adastrea\Source\Adastrea\Combat\WeaponDataAsset.cpp                                                                 | 224  |         |
| Error    | C2143  | syntax error: missing ';' before '{'                                                                                                  | Adastrea  | C:\Unreal Projects\Adastrea\Source\Adastrea\Combat\WeaponDataAsset.cpp                                                                 | 225  |         |
| Error    | C2447  | '{': missing function header (old-style formal list?)                                                                                 | Adastrea  | C:\Unreal Projects\Adastrea\Source\Adastrea\Combat\WeaponDataAsset.cpp                                                                 | 225  |         |
| Error    | C2059  | syntax error: 'else'                                                                                                                  | Adastrea  | C:\Unreal Projects\Adastrea\Source\Adastrea\Combat\WeaponDataAsset.cpp                                                                 | 228  |         |
| Error    | C2143  | syntax error: missing ';' before '{'                                                                                                  | Adastrea  | C:\Unreal Projects\Adastrea\Source\Adastrea\Combat\WeaponDataAsset.cpp                                                                 | 229  |         |
| Error    | C2447  | '{': missing function header (old-style formal list?)                                                                                 | Adastrea  | C:\Unreal Projects\Adastrea\Source\Adastrea\Combat\WeaponDataAsset.cpp                                                                 | 229  |         |
| Error    | C2059  | syntax error: 'return'                                                                                                                | Adastrea  | C:\Unreal Projects\Adastrea\Source\Adastrea\Combat\WeaponDataAsset.cpp                                                                 | 233  |         |
| Error    | C2059  | syntax error: '}'                                                                                                                     | Adastrea  | C:\Unreal Projects\Adastrea\Source\Adastrea\Combat\WeaponDataAsset.cpp                                                                 | 234  |         |
| Error    | C2143  | syntax error: missing ';' before '}'                                                                                                  | Adastrea  | C:\Unreal Projects\Adastrea\Source\Adastrea\Combat\WeaponDataAsset.cpp                                                                 | 234  |         |
| Error    | C1083  | Cannot open include file: 'StationEditorManager.h': No such file or directory                                                         | Adastrea  | C:\Unreal Projects\Adastrea\Source\StationEditor\UI\StationEditorWidget.h                                                              | 8    |         |

---

## Warnings

| Severity | Code   | Description                                                                                                                                                | Project   | File                                                                                                                                    | Line | Details |
|----------|--------|------------------------------------------------------------------------------------------------------------------------------------------------------------|-----------|-----------------------------------------------------------------------------------------------------------------------------------------|------|---------|
| Warning  | C4996  | 'UClass::ClassDefaultObject': ClassDefaultObject will be made private in the next release. Use GetDefault<>() or GetMutableDefault<>() functions.         | Adastrea  | C:\Unreal Projects\Adastrea\Plugins\vc-ue-extensions\Source\VisualStudioTools\Private\VisualStudioToolsCommandlet.cpp                    | 118  |         |
| Warning  | C4996  | 'UClass::ClassDefaultObject': ClassDefaultObject will be made private in the next release. Use GetDefault<>() or GetMutableDefault<>() functions.         | Adastrea  | C:\Unreal Projects\Adastrea\Plugins\vc-ue-extensions\Source\VisualStudioTools\Private\VisualStudioToolsCommandlet.cpp                    | 257  |         |
| Warning  | C4996  | 'FMessageDialog::Open': Use the overload of Open that takes the Title by-value.                                                                            | Adastrea  | C:\Unreal Projects\Adastrea\Plugins\AdastreaDirector\Source\AdastreaDirectorEditor\Private\SAdastreaDirectorPanel.cpp                    | 821  |         |
| Warning  | C4996  | ANY_PACKAGE has been deprecated. Use the full path name of objects (including classes) or provide a valid Outer for *FindObject* functions.                | Adastrea  | C:\Unreal Projects\Adastrea\Plugins\UnrealMCP\Source\UnrealMCP\Private\Commands\UnrealMCPBlueprintCommands.cpp                           | 197  |         |
| Warning  | C4996  | ANY_PACKAGE has been deprecated. Use the full path name of objects (including classes) or provide a valid Outer for *FindObject* functions.                | Adastrea  | C:\Unreal Projects\Adastrea\Plugins\UnrealMCP\Source\UnrealMCP\Private\Commands\UnrealMCPBlueprintCommands.cpp                           | 203  |         |
| Warning  | C4996  | ANY_PACKAGE has been deprecated. Use the full path name of objects (including classes) or provide a valid Outer for *FindObject* functions.                | Adastrea  | C:\Unreal Projects\Adastrea\Plugins\UnrealMCP\Source\UnrealMCP\Private\Commands\UnrealMCPBlueprintCommands.cpp                           | 210  |         |
| Warning  | C4996  | ANY_PACKAGE has been deprecated. Use the full path name of objects (including classes) or provide a valid Outer for *FindObject* functions.                | Adastrea  | C:\Unreal Projects\Adastrea\Plugins\UnrealMCP\Source\UnrealMCP\Private\Commands\UnrealMCPBlueprintCommands.cpp                           | 216  |         |
| Warning  | C4996  | ANY_PACKAGE has been deprecated. Use the full path name of objects (including classes) or provide a valid Outer for *FindObject* functions.                | Adastrea  | C:\Unreal Projects\Adastrea\Plugins\UnrealMCP\Source\UnrealMCP\Private\Commands\UnrealMCPBlueprintNodeCommands.cpp                       | 323  |         |
| Warning  | C4996  | ANY_PACKAGE has been deprecated. Use the full path name of objects (including classes) or provide a valid Outer for *FindObject* functions.                | Adastrea  | C:\Unreal Projects\Adastrea\Plugins\UnrealMCP\Source\UnrealMCP\Private\Commands\UnrealMCPBlueprintNodeCommands.cpp                       | 331  |         |
| Warning  | C4996  | ANY_PACKAGE has been deprecated. Use the full path name of objects (including classes) or provide a valid Outer for *FindObject* functions.                | Adastrea  | C:\Unreal Projects\Adastrea\Plugins\UnrealMCP\Source\UnrealMCP\Private\Commands\UnrealMCPBlueprintNodeCommands.cpp                       | 346  |         |
| Warning  | C4996  | ANY_PACKAGE has been deprecated. Use the full path name of objects (including classes) or provide a valid Outer for *FindObject* functions.                | Adastrea  | C:\Unreal Projects\Adastrea\Plugins\UnrealMCP\Source\UnrealMCP\Private\Commands\UnrealMCPBlueprintNodeCommands.cpp                       | 359  |         |
| Warning  | C4996  | ANY_PACKAGE has been deprecated. Use the full path name of objects (including classes) or provide a valid Outer for *FindObject* functions.                | Adastrea  | C:\Unreal Projects\Adastrea\Plugins\UnrealMCP\Source\UnrealMCP\Private\Commands\UnrealMCPBlueprintNodeCommands.cpp                       | 506  |         |
| Warning  | C4996  | 'FImageUtils::CompressImageArray': Please use PNGCompressImageArray or ThumbnailCompressImageArray                                                        | Adastrea  | C:\Unreal Projects\Adastrea\Plugins\UnrealMCP\Source\UnrealMCP\Private\Commands\UnrealMCPEditorCommands.cpp                              | 588  |         |

---

## Notes

- Errors will prevent successful compilation. Address all errors before attempting to build again.
- Warnings indicate deprecated features or recommended code changes, especially for compatibility with future Unreal Engine releases.
- Pay special attention to deprecated APIs: update your code to use recommended replacements to avoid breakage in upcoming Unreal versions.

```