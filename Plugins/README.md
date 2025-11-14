# Plugins Directory

This directory is for custom Unreal Engine plugins that extend the Adastrea project.

> **🚀 Upcoming:** The **Adastrea-Director Plugin** is planned for Q1-Q3 2026, which will provide AI-powered development assistance directly within the Unreal Editor. See the [Plugin Development Guide](../PLUGIN_DEVELOPMENT_GUIDE.md) and [UE Plugin Feasibility Report](../UE_PLUGIN_FEASIBILITY_REPORT.md) for details.

## Purpose

The `Plugins` folder is a standard Unreal Engine directory structure where you can place:
- Custom game-specific plugins
- Third-party plugins
- Community-created plugins
- Modding plugins

## Plugin Structure

Each plugin should follow the standard Unreal Engine plugin structure:

```
Plugins/
├── MyPlugin/
│   ├── MyPlugin.uplugin        # Plugin descriptor file
│   ├── Resources/              # Plugin icons and resources
│   ├── Source/                 # C++ source code
│   │   ├── MyPlugin/
│   │   │   ├── Private/
│   │   │   ├── Public/
│   │   │   └── MyPlugin.Build.cs
│   └── Content/                # Plugin content (Blueprints, assets)
```

## Creating a New Plugin

To create a new plugin:

1. Open the Unreal Editor
2. Go to **Edit → Plugins**
3. Click **New Plugin**
4. Follow the plugin creation wizard
5. The plugin will be automatically created in this directory

Alternatively, you can use the Unreal Engine command line tools or manually create the plugin structure.

## Documentation

For more information about Unreal Engine plugins, see:
- [Unreal Engine Plugin Documentation](https://docs.unrealengine.com/5.6/en-US/plugins-in-unreal-engine/)
- [Creating Plugins](https://docs.unrealengine.com/5.6/en-US/creating-plugins-in-unreal-engine/)

## .gitignore

The project's `.gitignore` is configured to:
- ✅ Track plugin source code (`.uplugin`, `.cpp`, `.h` files)
- ✅ Track plugin content (`.uasset`, `.umap` files)
- ❌ Ignore plugin binaries (`/Plugins/*/Binaries/`)
- ❌ Ignore plugin intermediate files (`/Plugins/*/Intermediate/`)

This ensures that plugin source code and content are version controlled while excluding generated files.

---

## Planned Plugins

### Adastrea-Director Plugin (Q1-Q3 2026)

**Status:** Planning Phase  
**Documentation:** [Plugin Development Guide](../PLUGIN_DEVELOPMENT_GUIDE.md) | [Feasibility Report](../UE_PLUGIN_FEASIBILITY_REPORT.md)

An AI-powered development assistant that provides:
- **Documentation Q&A:** Context-aware search across project documentation
- **Planning System:** Task decomposition and code generation
- **Autonomous Agents:** Performance profiling, bug detection, code quality monitoring
- **Seamless Integration:** Native Slate UI within Unreal Editor

**Architecture:**
- Hybrid C++/Python design
- Lightweight C++ plugin shell
- Python subprocess for AI processing
- Local socket IPC for communication
- Reuses 95% of existing [Adastrea-Director](https://github.com/Mittenzx/Adastrea-Director) codebase

**Timeline:**
- Q2 2026: Proof of Concept (6 weeks)
- Q2 2026: Foundation & Core Integration
- Q2 2026: Planning System
- Q2-Q3 2026: Autonomous Agents
- Q3 2026: Beta & Release

**Benefits:**
- ✅ Seamless editor workflow
- ✅ AI-assisted development
- ✅ Automated quality monitoring
- ✅ Direct engine access
- ✅ Improved developer productivity

For current external tool documentation, see [ADASTREA_DIRECTOR_ANALYSIS.md](../ADASTREA_DIRECTOR_ANALYSIS.md).
