# Adastrea Development Tools

## Overview

The `/Tools/` directory contains validation scripts and development utilities for the Adastrea project. These tools help maintain code quality, catch common errors, and enforce project standards.

## Validation Scripts

### check_uproperty.py
**Purpose**: Validates that all UObject* pointers have the UPROPERTY() macro for garbage collection tracking.

**Why It's Important**:
- UObject pointers without UPROPERTY() are invisible to Unreal's garbage collector
- Can cause memory leaks and crashes
- Critical for game stability

**Usage**:
```bash
# Basic scan
python Tools/check_uproperty.py

# Verbose output with detailed analysis
python Tools/check_uproperty.py --verbose
```

**What It Checks**:
- UObject* pointers (UClass*, UComponent*, etc.)
- AActor* pointers
- Common pointer patterns that need UPROPERTY

**Example Output**:
```
=== UPROPERTY Validation Report ===
Found 3 potential issues in 2 files:

Source/Adastrea/Combat/WeaponComponent.h:45
  UProjectile* CurrentProjectile;
  ^ Missing UPROPERTY() macro

Total files scanned: 156
Files with issues: 2
```

**When to Run**:
- Before committing C++ changes
- After adding new UObject pointer members
- As part of CI/CD pipeline (automated)

---

### check_null_safety.py
**Purpose**: Checks for potential null pointer dereferences and unsafe pointer usage.

**Why It's Important**:
- Null pointer crashes are common in C++
- Prevents runtime crashes
- Improves code reliability

**Usage**:
```bash
# Scan all source files
python Tools/check_null_safety.py

# Check specific file
python Tools/check_null_safety.py Source/Adastrea/Combat/WeaponComponent.cpp
```

**What It Checks**:
- Pointer dereferences without null checks
- IsValid() usage for Actors
- Null checks before pointer operations

**Example Issues Detected**:
```cpp
// Bad: No null check
void UWeaponComponent::FireWeapon()
{
    CurrentTarget->TakeDamage(...);  // ❌ Crash if CurrentTarget is null
}

// Good: Proper null check
void UWeaponComponent::FireWeapon()
{
    if (IsValid(CurrentTarget))
    {
        CurrentTarget->TakeDamage(...);  // ✅ Safe
    }
}
```

**When to Run**:
- Before committing C++ code
- After modifying pointer-heavy code
- When debugging crashes

---

### validate_naming.py
**Purpose**: Enforces Unreal Engine and project naming conventions.

**Why It's Important**:
- Consistent naming improves code readability
- Follows Unreal Engine standards
- Makes code easier to maintain

**Usage**:
```bash
# Validate all source files
python Tools/validate_naming.py

# Check specific directory
python Tools/validate_naming.py Source/Adastrea/Combat/
```

**What It Checks**:
- Class prefixes (U for UObject, A for AActor, F for structs)
- Variable naming (PascalCase for members, bPrefix for booleans)
- Function naming (PascalCase, verb-noun patterns)
- File naming (matches class name)

**Naming Rules Enforced**:

| Type | Prefix | Example | Notes |
|------|--------|---------|-------|
| UObject class | U | UWeaponComponent | Subclass of UObject |
| AActor class | A | ASpaceship | Subclass of AActor |
| Struct | F | FWeaponStats | Plain structs |
| Enum | E | EWeaponType | Enumerations |
| Interface (class) | I | ITargetable | Interface implementations |
| Boolean property | b | bIsActive | Always prefix with 'b' |
| Member variable | - | MaxAmmo | PascalCase |
| Function | - | FireWeapon() | PascalCase, verb-noun |

**Example Issues**:
```cpp
// ❌ Bad naming
class WeaponComponent : public UActorComponent { };  // Missing U prefix
bool isActive;  // Should be bIsActive
void fire();    // Should be Fire() or FireWeapon()

// ✅ Good naming
class UWeaponComponent : public UActorComponent { };
bool bIsActive;
void FireWeapon();
```

**When to Run**:
- Before committing new classes
- After refactoring
- During code review

---

### organize_content.py
**Purpose**: Organizes Unreal content files and maintains directory structure.

**Why It's Important**:
- Keeps Content Browser organized
- Enforces project structure
- Prevents asset clutter

**Usage**:
```bash
# Organize all content
python Tools/organize_content.py

# Dry run (show what would change)
python Tools/organize_content.py --dry-run

# Organize specific directory
python Tools/organize_content.py Content/DataAssets/
```

**What It Does**:
- Moves misplaced assets to correct directories
- Validates asset naming conventions
- Reports duplicate assets
- Checks for unused assets

**Directory Structure**:
```
Content/
├── Blueprints/
│   ├── GameModes/
│   ├── Ships/
│   └── UI/
├── DataAssets/
│   ├── Ships/
│   ├── Personnel/
│   ├── Weapons/
│   └── Quests/
├── Materials/
├── Textures/
└── Fab/
    └── [Marketplace assets]
```

**When to Run**:
- After importing new assets
- Before committing Content changes
- Periodically to maintain organization

---

## Integration with CI/CD

These tools are automatically run in the CI/CD pipeline (see `.github/workflows/code-quality.yml`):

```yaml
# Automated checks on every PR
- check_uproperty.py
- check_null_safety.py
- validate_naming.py
```

If any tool reports errors, the CI build fails and requires fixes before merging.

## Running All Validation Tools

To run all validation tools at once:

```bash
# Run from your project root directory
cd <project_root>  # Change to your local Adastrea project root

# Run all tools
python Tools/check_uproperty.py --verbose && \
python Tools/check_null_safety.py && \
python Tools/validate_naming.py
```

Or use the provided script (if available):
```bash
./Tools/run_all_validations.sh
```

## Best Practices

1. **Run Before Committing**
   - Always run relevant validation scripts before committing
   - Fix all reported issues before pushing

2. **CI Integration**
   - Tools run automatically in CI
   - Don't ignore CI failures

3. **Regular Maintenance**
   - Run organize_content.py weekly
   - Run all validation tools before releases

4. **Clean Code**
   - Address warnings, not just errors
   - Use tools to learn best practices

## Troubleshooting

### Tool Reports False Positives

Some patterns may trigger false positives:

**For check_uproperty.py**:
- Forward declarations are excluded automatically
- Function parameters are excluded
- If you have a legitimate reason not to use UPROPERTY, add a comment explaining why

**For check_null_safety.py**:
- Some code paths guarantee non-null (e.g., in constructors)
- Add assertion or comment to document the guarantee

**For validate_naming.py**:
- Some third-party code may not follow conventions
- Exclude third-party directories if needed

### Tool Won't Run

1. **Check Python version**: Requires Python 3.7+
   ```bash
   python --version
   ```

2. **Check you're in project root**:
   ```bash
   pwd  # Should show /path/to/Adastrea
   ```

3. **Check file paths**:
   ```bash
   ls Tools/check_uproperty.py  # Should exist
   ```

### Understanding Results

Each tool provides:
- **File path**: Where the issue is
- **Line number**: Exact location
- **Issue description**: What's wrong
- **Suggestion**: How to fix it (when applicable)

## Contributing

### Adding New Validation Tools

1. Create script in `/Tools/` directory
2. Follow existing script structure:
   - Docstring at top
   - Command-line argument parsing
   - Clear error messages
   - Return non-zero exit code on errors

3. Add to CI pipeline (`.github/workflows/code-quality.yml`)
4. Document in this README

### Improving Existing Tools

1. Test changes thoroughly
2. Update documentation
3. Ensure backward compatibility
4. Add examples of new patterns detected

## Related Documentation

- `.github/instructions/` - Project coding standards
- `CODE_STYLE.md` - Detailed style guide
- `CONTRIBUTING.md` - Contribution guidelines
- `COMPLETE_REVIEW_SUMMARY.md` - Code review findings

---

## Quick Reference

| Tool | Primary Purpose | Run When |
|------|----------------|----------|
| check_uproperty.py | GC tracking validation | Before committing C++ |
| check_null_safety.py | Null pointer safety | Before committing C++ |
| validate_naming.py | Naming conventions | Before committing new classes |
| organize_content.py | Content organization | After importing assets |

---

**Location**: `/Tools/`  
**Maintained By**: Adastrea Development Team  
**Last Updated**: 2025-12-20  
**Status**: Active - All tools ready for use

## Migration Note

**Previous Duplicate Files Removed**:
- ❌ `/check_uproperty.py` (duplicate - removed)
- ❌ `/check_uproperty_final.py` (duplicate - removed)
- ❌ `/check_uproperty_fixed.py` (duplicate - removed)
- ✅ `/Tools/check_uproperty.py` (kept - most comprehensive)

All validation tools are now consolidated in the `/Tools/` directory for consistency.
