# Automation Setup Guide

This guide explains how to set up and use the automated code quality checks for the Adastrea project.

## Overview

The project includes several automated quality checks:

1. **GitHub Actions CI/CD** - Runs on every pull request and push
2. **Pre-commit Hooks** - Runs locally before each commit
3. **Manual Tools** - Can be run anytime during development

## GitHub Actions Workflow

### What It Checks

The CI/CD workflow (`.github/workflows/code-quality.yml`) runs:

- ✅ UPROPERTY usage validation
- ✅ Asset naming convention checks
- ✅ Hardcoded secrets detection
- ✅ TODO/FIXME comment tracking

### When It Runs

- Automatically on every pull request to `main` or `develop`
- Automatically on every push to `main` or `develop`
- Can be manually triggered from GitHub Actions tab

### Viewing Results

1. Go to your pull request on GitHub
2. Check the "Checks" tab
3. Click on "Code Quality Checks" to see details
4. Expand each step to see results

### Handling Failures

Most checks are set to `continue-on-error: true`, meaning they won't block merges. However, you should review and address issues:

- **UPROPERTY warnings**: All UObject* pointers need UPROPERTY for GC tracking
- **Naming violations**: Follow project naming conventions (BP_, DA_, etc.)
- **Secrets**: Never commit sensitive data - remove immediately
- **TODOs**: Consider completing or documenting for future work

## Pre-commit Hooks

### Installation

Install the pre-commit hook locally:

```bash
# From repository root
cp Tools/pre-commit-hook.sh .git/hooks/pre-commit
chmod +x .git/hooks/pre-commit
```

### What It Checks

The pre-commit hook runs before each commit:

- ✅ UPROPERTY usage (warning only)
- ✅ Asset naming conventions (warning only)
- ❌ Hardcoded secrets (blocks commit if found)
- ⚠️  Debug print statements (warning only)

### Bypassing the Hook

In emergencies, you can bypass the hook:

```bash
git commit --no-verify
```

**Note**: This is not recommended. Fix issues instead of bypassing.

### Uninstalling

Remove the hook:

```bash
rm .git/hooks/pre-commit
```

## Manual Tools

You can run quality checks manually anytime:

### Check UPROPERTY Usage

Validates that all UObject* pointers have UPROPERTY macro:

```bash
python Tools/check_uproperty.py
```

**Why This Matters**: Unreal Engine's garbage collector needs UPROPERTY to track object references. Missing UPROPERTY can cause crashes and memory leaks.

**When to Run**:
- After adding new C++ classes
- Before submitting pull requests
- When debugging memory issues

### Validate Asset Naming

Checks that assets follow naming conventions:

```bash
python Tools/validate_naming.py
```

**Naming Conventions**:
- `BP_` - Blueprint
- `DA_` - Data Asset
- `M_` - Material
- `MI_` - Material Instance
- `T_` - Texture
- `SM_` - Static Mesh
- `SK_` - Skeletal Mesh
- `WBP_` - Widget Blueprint

**When to Run**:
- After creating new assets
- Before organizing assets
- During cleanup sprints

### Check Null Safety

Validates null pointer checks:

```bash
python Tools/check_null_safety.py
```

**When to Run**:
- After modifying pointer dereferences
- When addressing crash bugs
- Before major releases

## Best Practices

### Development Workflow

1. **Before Starting Work**:
   - Pull latest changes
   - Run manual tools to check current state

2. **During Development**:
   - Follow coding standards
   - Add UPROPERTY to all UObject* pointers
   - Use proper naming conventions
   - Write clean, documented code

3. **Before Committing**:
   - Pre-commit hook runs automatically
   - Address any errors immediately
   - Review warnings and fix if reasonable

4. **Before Creating PR**:
   - Run all manual tools
   - Fix critical issues
   - Document known issues in PR description

5. **After PR Review**:
   - Address CI/CD failures
   - Respond to code review comments
   - Ensure all checks pass

### Handling Common Issues

#### "Found UObject* without UPROPERTY"

**Fix**: Add UPROPERTY macro:

```cpp
// ❌ Wrong
UMyClass* MyPointer;

// ✅ Correct
UPROPERTY()
UMyClass* MyPointer;
```

Even private pointers need UPROPERTY for GC tracking.

#### "Asset name does not follow convention"

**Fix**: Rename asset with proper prefix:

```
❌ MyBlueprint.uasset
✅ BP_MyBlueprint.uasset

❌ CustomDataAsset.uasset
✅ DA_CustomDataAsset.uasset
```

#### "Found potential hardcoded secret"

**Fix**: Use configuration files or environment variables:

```cpp
// ❌ Wrong
FString APIKey = "1234567890abcdef";

// ✅ Correct
FString APIKey = GConfig->GetString(TEXT("API"), TEXT("Key"));
```

#### "Found debug print statements"

**Fix**: Use UE_LOG instead:

```cpp
// ❌ Wrong
printf("Debug: %d\n", Value);
std::cout << "Value: " << Value << std::endl;

// ✅ Correct
UE_LOG(LogAdastrea, Log, TEXT("Value: %d"), Value);
```

## Configuration

### GitHub Actions

Edit `.github/workflows/code-quality.yml` to:
- Add new check steps
- Change when workflow runs
- Modify error handling behavior

### Pre-commit Hook

Edit `Tools/pre-commit-hook.sh` to:
- Add new validation checks
- Change which checks block commits
- Customize error messages

### Python Tools

Edit individual tools in `Tools/` directory to:
- Adjust validation rules
- Change severity levels
- Add new check types

## Troubleshooting

### "Python not found"

Install Python 3.11+:
- **Windows**: Download from python.org
- **Mac**: `brew install python3`
- **Linux**: `sudo apt install python3`

### "Permission denied" on pre-commit hook

Make it executable:

```bash
chmod +x .git/hooks/pre-commit
```

### "Module not found" errors

Install dependencies:

```bash
pip install pyyaml
```

### GitHub Actions not running

Check:
- Workflow file is in `.github/workflows/`
- File has `.yml` extension
- Syntax is valid YAML
- Branch matches trigger conditions

## Contributing

To add new quality checks:

1. Create Python script in `Tools/`
2. Add to `.github/workflows/code-quality.yml`
3. Add to `Tools/pre-commit-hook.sh`
4. Update this documentation
5. Test thoroughly before merging

## Resources

- [Unreal Directive Best Practices](../.github/instructions/unreal-directive-best-practices.md)
- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [Git Hooks Documentation](https://git-scm.com/book/en/v2/Customizing-Git-Git-Hooks)
- [Code Style Guide](../CODE_STYLE.md)

---

**Last Updated**: 2025-12-08  
**Maintained By**: Adastrea Development Team
