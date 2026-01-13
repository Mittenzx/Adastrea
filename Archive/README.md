# Archive Directory

**Purpose**: Storage for non-MVP files that will be needed after Trade Simulator MVP validation

**Date Archived**: December 30, 2025  
**Reason**: Focus development on Trade Simulator MVP (12-week timeline)  
**Based on**: Critical Review Analysis and Trade Simulator MVP Instructions

---

## 📋 What's Archived Here

This directory contains code, documentation, and assets for systems that are **explicitly deferred** until after MVP validation succeeds. These are not deleted because they represent valuable work that will be reintegrated when appropriate.

---

## 🗂️ Directory Structure

### `/Source`
Non-MVP C++ source code for deferred game systems:
- **Combat**: Weapons, targeting, shields, point defense (13 files)
- **Exploration**: Scanning, discovery, anomalies (7 files)
- **Quest**: Quest management and data assets (2 files)
- **Personnel/Characters**: Crew management AI (deferred until crew needed)
- **Rivals**: Antagonist system
- **Procedural**: Procedural generation tools
- **Way**: Advanced navigation networks
- **Materials**: Advanced material resource system
- **Tutorial**: Tutorial system (will return in Week 9-12)
- **Audio**: Audio system (will return in Week 7-9)

### `/Documentation`
Guides and documentation for deferred systems:
- Combat guides
- Exploration guides
- Personnel/AI setup guides
- Way system documentation
- Material system guides
- Advanced workflow templates

### `/Scripts`
Python automation scripts and one-time migration scripts:
- **Archived January 13, 2026**: One-time migration and setup scripts
- Content generators (20+ files)
- Blueprint generators
- Procedural generation tools
- Mass content creation utilities
- Advanced automation scripts

**See**: `Scripts/README.md` for details on archived scripts

### `/Assets`
Non-MVP assets and templates:
- HTML mockups for deferred features
- YAML templates for combat, exploration, quests
- Blueprint templates for unused systems
- Obsolete documentation

### `/Reports`
Generated reports from automated analysis:
- **Archived January 13, 2026**: One-time generated reports
- Content organization reports
- YAML template inventories

**See**: `Reports/README.md` for details on archived reports

### `/Content`
Unreal Content Browser assets (if any):
- Blueprints for deferred systems
- Data Assets not used in MVP
- VFX for combat
- UI for unused features

---

## ⚠️ Important Notes

### Why Archive Instead of Delete?

1. **Preserve Work**: These systems represent months of quality development
2. **Future Use**: Will be reintegrated post-MVP if validation succeeds
3. **Learning**: Shows architectural patterns and design decisions
4. **Audit Trail**: Demonstrates scope management discipline

### When to Restore Files

**After MVP Success** (75% fun rating, 50% buy intent):
1. Review archived systems
2. Prioritize based on player feedback
3. Reintegrate one system at a time
4. Test integration thoroughly
5. Validate each addition with playtesters

**If MVP Fails**:
- May need different systems
- Or may pivot to different core gameplay
- Archive may remain archived indefinitely

---

## 🎯 MVP-Critical Systems (NOT Archived)

These systems remain in active development:

**✅ Keep Active:**
1. **Spaceship System** - Basic flight, cargo capacity
2. **Trading System** - Buy/sell, prices, inventory
3. **Space Station System** - Docking, trading UI access
4. **HUD/UI System** - Basic HUD, trading interface
5. **Save System** - Basic save/load
6. **Faction System** (simplified) - Price modifiers only
7. **Navigation System** (simplified) - Waypoint markers only
8. **Inventory System** (simplified) - Cargo holds only

---

## 📊 Archive Statistics

**Systems Archived**: 14 complete game systems  
**Files Archived**: ~100+ C++ files, ~50+ documentation files, ~20+ Python scripts  
**Deferred Until**: Post-MVP validation (March 2026 or later)

---

## 🔄 Restoration Process

When ready to restore a system:

1. **Copy files back** to appropriate locations:
   - `Archive/Source/[System]/*.h` → `Source/Adastrea/Public/[System]/`
   - `Archive/Source/[System]/*.cpp` → `Source/Adastrea/Private/[System]/`
   - `Archive/Documentation/*.md` → `Assets/` or `docs/systems/`

2. **Update build files** if needed:
   - Check `Adastrea.Build.cs` for module dependencies
   - Recompile and test

3. **Integrate gradually**:
   - Don't restore everything at once
   - Test each system individually
   - Validate with playtesters

4. **Update documentation**:
   - Mark system as active in SYSTEM_CLASSIFICATION.md
   - Update CHANGELOG.md with restoration

---

## 📞 Questions?

See primary documentation:
- `.github/instructions/trade-simulator-mvp.instructions.md` - MVP scope
- `.github/instructions/anti-patterns.instructions.md` - Lessons learned
- `docs/mvp/SYSTEM_CLASSIFICATION.md` - System classifications
- `CRITICAL_REVIEW_ACTION_PLAN.md` - 12-week recovery plan

---

**Remember**: One polished system beats ten half-finished systems.

**Last Updated**: December 30, 2025  
**Next Review**: After Week 4 playtest (late January 2026)
