# Plugins Directory

This directory is for custom Unreal Engine plugins that extend the Adastrea project.

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
