# Unreal MCP Setup - Step by Step

## Current Status: Setting Up Full MCP Connection

This guide will enable direct AI-to-Unreal Engine communication for automation.

---

## Step 1: Install Unreal MCP Plugin ⏳

### Option A: Via Git (Recommended)
```powershell
# Open PowerShell in project root
cd "c:\Unreal Projects\Adastrea"

# Clone the plugin
git clone https://github.com/chongdashu/unreal-mcp.git temp-mcp

# Copy plugin to Plugins folder
New-Item -ItemType Directory -Force -Path "Plugins\UnrealMCP"
Copy-Item -Recurse -Force "temp-mcp\UnrealMCP\*" "Plugins\UnrealMCP\"

# Clean up
Remove-Item -Recurse -Force temp-mcp
```

### Option B: Manual Download
1. Go to: https://github.com/chongdashu/unreal-mcp
2. Click **Code** → **Download ZIP**
3. Extract the ZIP
4. Copy `UnrealMCP` folder to `c:\Unreal Projects\Adastrea\Plugins\`

---

## Step 2: Enable Python Plugin in Unreal ⏳

1. **Open Unreal Editor** with Adastrea project
2. **Edit** → **Plugins**
3. Search: **"Python Editor Script Plugin"**
4. Check **Enabled**
5. Click **Restart Now**

**Verify:**
- After restart, open **Output Log**
- Look for: `LogPython: Python 3.x initialized`

---

## Step 3: Configure Cursor for MCP ✅

**Already Done!** Created `.cursor/mcp.json` with Unreal MCP server config.

**What it does:**
- Connects Cursor to Unreal Engine via MCP protocol
- Enables AI to execute Python commands in UE
- Allows real-time Blueprint manipulation

---

## Step 4: Install Node.js (If Not Installed) ⏳

MCP server requires Node.js/npx to run.

**Check if installed:**
```powershell
node --version
```

**If not installed:**
1. Download from: https://nodejs.org/ (LTS version)
2. Run installer (default settings are fine)
3. Restart PowerShell/Cursor after installation

---

## Step 5: Restart Cursor ⏳

1. **Close Cursor completely** (not just window - exit from system tray)
2. **Reopen Cursor**
3. MCP server will auto-start when needed

**Verify Connection:**
- MCP server should appear in Cursor's status bar
- Or check Cursor's developer console for MCP logs

---

## Step 6: Test the Connection 🧪

Once everything is set up, test with:

```
"List all Blueprints in the project"
```

or

```
"Execute: print('Hello from Unreal!')"
```

**Expected Result:**
- AI will execute Python code directly in Unreal Engine
- No more copy/paste needed!
- Full automation capabilities

---

## Verification Checklist

- [ ] UnrealMCP plugin in `Plugins/UnrealMCP/` folder
- [ ] Python Editor Script Plugin enabled in Unreal
- [ ] Unreal Editor restarted after enabling Python
- [ ] Node.js installed (check: `node --version`)
- [ ] `.cursor/mcp.json` exists (already created ✅)
- [ ] Cursor restarted
- [ ] Unreal Editor running with Adastrea project open
- [ ] Test command successful

---

## Troubleshooting

### "Cannot find module @chongdashu/unreal-mcp-server"
**Solution:** 
```powershell
npm install -g @chongdashu/unreal-mcp-server
```

### "MCP server not starting"
**Solution:**
1. Check Node.js is installed: `node --version`
2. Check Cursor's Output panel for MCP errors
3. Try manual start: `npx -y @chongdashu/unreal-mcp-server`

### "Python not initialized in Unreal"
**Solution:**
1. Edit → Plugins → Enable "Python Editor Script Plugin"
2. Restart Unreal Editor
3. Check Output Log for Python initialization

### "Connection timeout"
**Solution:**
1. Ensure Unreal Editor is running
2. Ensure project (Adastrea.uproject) is open
3. Check Windows Firewall isn't blocking connection
4. Try restarting both Unreal and Cursor

---

## What You'll Be Able to Do

Once connected:

✅ **Automate Blueprint Configuration**
```
"Set BP_PlayerShip's MaxSpeed to 5000"
```

✅ **Spawn and Manipulate Actors**
```
"Create 10 test cubes in a line"
```

✅ **Query Project State**
```
"Show me all spaceships in the project"
```

✅ **Batch Operations**
```
"Update all station modules to use new material"
```

✅ **Level Population**
```
"Add lighting and player start to TestLevel"
```

✅ **Asset Management**
```
"Create a new faction data asset for the Solar Union"
```

---

## Next Steps

1. **Complete steps 1-6 above**
2. **Test connection** with simple command
3. **Continue PIE setup** - I'll be able to complete the remaining Blueprint configurations automatically!

---

**Ready to start?** Begin with **Step 1** (installing the plugin), then work through each step. Let me know when you complete each step and I'll help verify and troubleshoot if needed!

**Estimated Total Time:** 10-15 minutes

**Last Updated:** 2025-11-29  
**Status:** Setup in Progress
