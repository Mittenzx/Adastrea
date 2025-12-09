# Adastrea Code Quality Scripts

This directory contains automated code quality and validation scripts for the Adastrea project.

## Scripts

### check_module_dependencies.py

**Purpose**: Detect circular dependencies between Unreal Engine modules

**Usage**:
```bash
python3 .github/scripts/check_module_dependencies.py
```

**What it checks**:
1. **Module Declaration Dependencies**: Analyzes `.Build.cs` files to build a dependency graph
2. **Circular Dependencies**: Detects cycles in the module dependency graph
3. **Include Statement Issues**: Warns about potential circular dependencies in `#include` statements

**Exit codes**:
- `0`: No circular dependencies detected (success)
- `1`: Circular dependencies found or errors occurred (failure)

**Example output**:
```
Checking module dependencies...
Source directory: /path/to/Adastrea/Source

Found 3 modules:
  - Adastrea
  - PlayerMods
  - StationEditor

✅ No circular dependencies detected in module declarations
✅ No include-based circular dependency warnings

Module dependency check complete!
```

**If circular dependencies are found**:
```
❌ CIRCULAR DEPENDENCIES DETECTED:

  Cycle 1: Adastrea -> StationEditor -> Adastrea

Circular dependencies must be fixed before the project can build correctly.
See .github/MODULE_DEPENDENCY_FIX.md for resolution strategies.
```

**Resolution**:
See `.github/MODULE_DEPENDENCY_FIX.md` for detailed instructions on how to fix circular dependencies.

## CI/CD Integration

These scripts are automatically run by GitHub Actions:

### check-module-dependencies.yml
- **Trigger**: Pull requests and pushes affecting `.Build.cs`, `.h`, or `.cpp` files
- **Action**: Runs `check_module_dependencies.py`
- **Failure**: Blocks merge if circular dependencies are detected

## Adding New Scripts

When adding new scripts to this directory:

1. **Make them executable**: `chmod +x script_name.py`
2. **Add shebang**: `#!/usr/bin/env python3` or `#!/usr/bin/env bash`
3. **Document in this README**: Add a section describing the script
4. **Add CI workflow**: Create a GitHub Actions workflow if appropriate
5. **Exit codes**: Use `0` for success, non-zero for failure
6. **Clear output**: Use emoji and formatting for easy scanning (✅ ❌ ⚠️)

## Best Practices

### Python Scripts
- Use type hints for better code clarity
- Add docstrings to all functions
- Handle errors gracefully
- Provide clear, actionable error messages
- Use `pathlib.Path` for file system operations

### Shell Scripts
- Use `set -euo pipefail` for safety
- Quote all variables: `"$VAR"`
- Check exit codes: `if [ $? -eq 0 ]; then`
- Provide verbose output with `set -x` option

### Error Messages
- Be specific about what failed
- Suggest how to fix the issue
- Link to relevant documentation
- Use formatting to highlight important info

## Testing Scripts Locally

Before committing a new script:

1. **Test success case**: Verify it passes on clean code
2. **Test failure case**: Verify it catches errors correctly
3. **Test edge cases**: Empty input, missing files, etc.
4. **Check output**: Ensure messages are clear and helpful
5. **Verify exit codes**: Correct codes for success/failure

Example:
```bash
# Test the script
python3 .github/scripts/check_module_dependencies.py
echo "Exit code: $?"

# Test with deliberate error (if applicable)
# Verify appropriate error message and exit code
```

## Dependencies

### Python Scripts
- **Python version**: 3.11+
- **Standard library only**: No external dependencies required
- **Type hints**: Use for better code quality

### Shell Scripts
- **Shell**: bash 4.0+
- **Common utilities**: grep, sed, awk (assume GNU versions)

## Contributing

When modifying scripts:
1. Test thoroughly before committing
2. Update this README with any changes
3. Maintain backward compatibility if possible
4. Update CI workflows if script interface changes

## Related Documentation

- `.github/MODULE_DEPENDENCY_FIX.md` - Circular dependency resolution guide
- `MODULE_DEPENDENCY_GUIDE.md` - Module architecture guidelines
- `.github/workflows/` - CI/CD workflow definitions

---

**Maintained by**: Adastrea Development Team  
**Last updated**: 2025-12-09
