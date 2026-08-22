# UE 5.6 → 5.8 Migration Plan

**Adastrea | Generated: August 9, 2026**

---

## 📋 Migration Overview

| Component | Current | Target | Action |
|-----------|---------|--------|--------|
| Engine | 5.6 | 5.8 | Update `.uproject` EngineAssociation |
| Project Config | 5.6 | 5.8 | Update plugin `.uplugin` EngineVersion |
| Build Settings | V5 | V6 | Update TargetRules `DefaultBuildSettings` |
| C++ Standard | C++20 | C++20 | Likely unchanged |
| Modules | 5.6 API | 5.8 API | Audit deprecated APIs |
| Plugins | 5.6 | 5.8 | Update plugin descriptors |

---

## 🎯 Phase 1: Project Configuration (Immediate)

### 1.1 Adastrea.uproject
- Update `EngineAssociation` to `"5.8"` (or GUID for 5.8)
- Verify all listed plugins are compatible with 5.8

### 1.2 Plugin Descriptors
- `Plugins/AdastreaDirector/AdastreaDirector.uplugin` → `"EngineVersion": "5.8.0"`
- `Plugins/VisualStudioTools/VisualStudioTools.uplugin` → Check marketplace for 5.8 version

### 1.3 TargetRules (Adastrea.Target.cs, AdastreaEditor.Target.cs)
- `DefaultBuildSettings = BuildSettingsVersion.V6` (UE 5.8 uses V6)
- `IncludeOrderVersion = EngineIncludeOrderVersion.Latest` (keep)
- Review `AdditionalCompilerArguments` for 5.8 compatibility

---

## 🎯 Phase 2: C++ API Migration (Code Audit)

### Common UE 5.6 → 5.8 Breaking Changes

| Area | 5.6 API | 5.8 API | Migration |
|------|---------|---------|-----------|
| **Enhanced Input** | `UEnhancedInputComponent` | Same | Check for new action value types |
| **Gameplay Abilities** | `UGameplayAbility` | Same | Check GAS updates |
| **Niagara** | `UNiagaraComponent` | Same | Parameter updates |
| **UMG/Slate** | `SWidget` | Same | Slate brush changes |
| **Physics** | `Chaos` | Chaos 2.0 | Collision profile updates |
| **Animation** | `Control Rig` | Control Rig v2 | Rig hierarchy changes |
| **Networking** | `Replication` | Same | NetDriver updates |
| **Asset Manager** | `PrimaryAssetId` | Same | Scan path changes |

### Specific to Adastrea Codebase

| File | Potential Issues |
|------|------------------|
| `Source/Adastrea/Private/Trading/` | DataAsset loading, AssetManager scan paths |
| `Source/Adastrea/Private/Stations/` | Modular actor spawning, component attachment |
| `Source/Adastrea/Private/Ships/` | EnhancedInput bindings, physics handling |
| `Source/Adastrea/Private/Player/` | SaveGame, GameInstanceSubsystem |
| `Source/Adastrea/Private/UI/` | UMG widget construction, MVVM bindings |
| `Plugins/AdastreaDirector/` | PythonScriptPlugin, EditorScriptingUtilities APIs |

---

## 🎯 Phase 3: Build & Test

### 3.1 Build Scripts
- `build_with_ue_tools.sh/.bat` - Verify UE 5.8 paths
- `setup_ue_build_tools.sh/.bat` - Download 5.8 build tools

### 3.2 Project Generation
- Regenerate VS project files with UE 5.8
- Verify all modules compile

### 3.3 Automated Tests
- Run Python test suite (should be engine-agnostic)
- Verify DataAsset loading in 5.8

---

## 🚀 Implementation Order

1. **Config files** (`.uproject`, `.uplugin`, TargetRules) — 5 mins
2. **C++ compilation fixes** — 30-60 mins
3. **Build & test** — 15 mins
4. **PIE validation** — 10 mins

---

## ⚠️ Risk Areas

| Risk | Likelihood | Mitigation |
|------|------------|------------|
| Plugin incompatibility | Medium | Check VisualStudioTools marketplace for 5.8 |
| EnhancedInput changes | Low | Test input bindings |
| Niagara parameter changes | Low | Verify VFX |
| AssetManager scan paths | Medium | Test DataAsset loading |
| AdastreaDirector plugin | High | Most custom code, needs audit |

---

## 📝 Next Steps

Run the migration script to update all config files, then compile and fix errors.