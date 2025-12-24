# Developer Quick Reference - Editor Setup

## Cursor/VS Code MCP Configuration

### First Time Setup

1. **Copy the template file:**
   ```bash
   cp .cursor/mcp.json.template .cursor/mcp.json
   ```

2. **The template uses a workspace-relative path** that works automatically:
   ```json
   {
     "mcpServers": {
       "unreal-engine": {
         "command": "npx",
         "args": ["-y", "@chongdashu/unreal-mcp-server"],
         "env": {
           "UNREAL_PROJECT_PATH": "${workspaceFolder}/Adastrea.uproject"
         }
       }
     }
   }
   ```

3. **No changes needed!** The `${workspaceFolder}` variable automatically resolves to your project location.

### Custom Configuration (Optional)

If you need a different configuration:

1. Edit `.cursor/mcp.json` (this file is gitignored)
2. Modify the MCP server settings as needed
3. Your changes won't be committed to the repository

## What Was Fixed

### ❌ Before (Broken)
```json
"UNREAL_PROJECT_PATH": "c:/Unreal Projects/Adastrea/Adastrea.uproject"
```
**Problems:**
- Only works on Windows
- Only works if project is at exact path `c:/Unreal Projects/Adastrea/`
- Breaks for other developers
- Breaks in CI/CD

### ✅ After (Fixed)
```json
"UNREAL_PROJECT_PATH": "${workspaceFolder}/Adastrea.uproject"
```
**Benefits:**
- Works on Windows, macOS, and Linux
- Works regardless of where you clone the project
- Works for all developers
- Works in CI/CD environments

## Why .cursor/ is Gitignored

The `.cursor/` directory contains **user-specific editor settings**:
- MCP server configurations
- User preferences
- Workspace state

Different developers may have different:
- MCP server versions
- Custom extensions
- Preferred settings

**Solution:** 
- `.cursor/` is in `.gitignore` (not tracked)
- `.cursor/mcp.json.template` is tracked (shared template)

## Troubleshooting

### "MCP server won't connect"

1. **Check if template was copied:**
   ```bash
   ls -la .cursor/mcp.json
   ```
   If missing, copy the template:
   ```bash
   cp .cursor/mcp.json.template .cursor/mcp.json
   ```

2. **Verify the path in mcp.json:**
   ```bash
   cat .cursor/mcp.json
   ```
   Should show: `"${workspaceFolder}/Adastrea.uproject"`

3. **Restart Cursor/VS Code** after creating/modifying the file

### "Cannot find Adastrea.uproject"

Make sure you're opening the **root directory** of the project in Cursor/VS Code, not a subdirectory.

**Correct:**
```
/path/to/Adastrea/          ← Open this directory
  ├── Adastrea.uproject
  ├── Source/
  ├── Content/
  └── .cursor/
```

**Incorrect:**
```
/path/to/Adastrea/
  ├── Source/                ← Don't open this
  ├── Content/               ← Don't open this
```

### "Path uses backslashes on Windows"

The `${workspaceFolder}` variable automatically handles path separators for your OS. No manual conversion needed!

## Related Files

- `.cursor/mcp.json` - Your personal config (gitignored)
- `.cursor/mcp.json.template` - Shared template (tracked in git)
- `.gitignore` - Excludes `.cursor/` from version control

## Review Documents

For more details on the changes made:
- `EDITOR_CONFIG_REVIEW.md` - Review of editor configuration
- `COMMIT_C35BCA0_REVIEW.md` - Review of content additions
- `COMPLETE_REVIEW_SUMMARY.md` - Complete review summary

---

**Last Updated:** 2025-12-20  
**Issue:** Fixed hardcoded Windows path in MCP configuration  
**Status:** ✅ Resolved - Cross-platform compatible
