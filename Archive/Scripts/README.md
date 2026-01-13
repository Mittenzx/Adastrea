# Archived Scripts

**Date Archived**: January 13, 2026  
**Reason**: One-time use or obsolete scripts

## Overview

This directory contains scripts that were used during development but are no longer needed for active development. They are preserved for historical reference.

## Files

### Migration Scripts

**move-and-update-includes.sh**
- **Purpose**: One-time script to move MyFirstSpaceshipDataAsset.h to Private/Ships
- **Status**: Task completed, no longer needed
- **Archived**: January 13, 2026

### Setup Scripts (AdastreaDirector Plugin)

**reingest_docs.bat**
- **Purpose**: Re-ingest documentation for AdastreaDirector plugin
- **Status**: AdastreaDirector plugin no longer exists in repository
- **Archived**: January 13, 2026

**setup_adastrea_director.bat**
- **Purpose**: Launch interactive setup for AdastreaDirector plugin
- **Status**: AdastreaDirector plugin no longer exists in repository
- **Archived**: January 13, 2026

### Project Generation Scripts

**regenerate_project.bat**
- **Purpose**: Regenerate Visual Studio project files
- **Status**: Contains hardcoded paths (C:\Adastrea\), not suitable for general use
- **Note**: Use Unreal Engine's native project file generation instead
- **Archived**: January 13, 2026

## Why Archive Instead of Delete?

These scripts are preserved because:
1. They provide historical context about project evolution
2. The migration script shows how code reorganization was done
3. The plugin scripts document that AdastreaDirector existed at one point
4. They may contain useful patterns for similar future tasks

## Current Alternatives

- **Project file generation**: Use Unreal Engine Editor's "Generate Visual Studio Project Files" option
- **Migration scripts**: Create new scripts as needed for specific tasks
- **Plugin setup**: No longer applicable

---

**Note**: These files should not be used for active development. They are kept for reference only.
