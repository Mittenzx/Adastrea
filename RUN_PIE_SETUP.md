# How to Run Auto-Configuration Script

## Method 1: Execute Python Script (Easiest)

1. **Open Unreal Editor** with the Adastrea project
2. **Menu Bar** → Click **Tools**
3. Select **Execute Python Script...**
4. Browse to: `c:\Unreal Projects\Adastrea\AutoConfigurePIE.py`
5. Click **Open**
6. Watch the **Output Log** for results

## Method 2: Python Console

1. **Open Unreal Editor**
2. **Menu Bar** → **Window** → **Developer Tools** → **Output Log**
3. Click the **Cmd** dropdown → Select **Python**
4. Paste this command:
   ```python
   execfile(r"c:\Unreal Projects\Adastrea\AutoConfigurePIE.py")
   ```
5. Press **Enter**

## Method 3: Copy-Paste into Python Console

1. **Open Unreal Editor**
2. **Window** → **Developer Tools** → **Output Log**
3. Switch console type to **Python**
4. Open `AutoConfigurePIE.py` in a text editor
5. Copy the entire contents
6. Paste into Python console
7. Press **Enter**

---

## What the Script Does

### ✅ Automatically Configures:
- **BP_TestGameMode**
  - Sets Default Pawn Class to BP_PlayerShip
  - Sets Player Controller Class to BP_PlayerController
  
- **BP_PlayerController**
  - Sets HUD Widget Class to WBP_SpaceShipHUD
  - Disables mouse cursor
  
- **Verifies TestLevel**
  - Checks for Directional Light
  - Checks for Player Start

### ⚠️ Manual Steps Still Required:
- **BP_PlayerShip** component setup (script can't edit component hierarchy):
  - Add Static Mesh Component
  - Add Camera Component
  - Add SpaceshipControlsComponent
  - Set Auto Possess Player to "Player 0"

---

## Expected Output

When successful, you'll see in the Output Log:

```
[PIE Setup] ============================================================
[PIE Setup] Auto-Configure PIE Setup for Adastrea
[PIE Setup] ============================================================
[PIE Setup] 
[PIE Setup] Configuring BP_TestGameMode...
[PIE Setup]   ✓ Set Default Pawn Class to BP_PlayerShip
[PIE Setup]   ✓ Set Player Controller Class to BP_PlayerController
[PIE Setup]   ✓ Saved BP_TestGameMode
[PIE Setup] 
[PIE Setup] Configuring BP_PlayerShip...
[PIE Setup]   Note: Component configuration requires Blueprint editor access
[PIE Setup]   Please manually verify in Unreal Editor that BP_PlayerShip has:
[PIE Setup]     - Static Mesh Component (visible mesh)
[PIE Setup]     - Camera Component (X=-500, Y=0, Z=200)
[PIE Setup]     - SpaceshipControlsComponent
[PIE Setup]     - Auto Possess Player = Player 0
[PIE Setup] 
[PIE Setup] Configuring BP_PlayerController...
[PIE Setup]   ✓ Set HUD Widget Class to WBP_SpaceShipHUD
[PIE Setup]   ✓ Disabled mouse cursor
[PIE Setup]   ✓ Saved BP_PlayerController
[PIE Setup] 
[PIE Setup] Verifying TestLevel...
[PIE Setup]   ✓ Lighting found
[PIE Setup]   ✓ Player Start found
[PIE Setup] 
[PIE Setup] ============================================================
[PIE Setup] ✅ Auto-configuration completed!
[PIE Setup] 
[PIE Setup] NEXT STEPS:
[PIE Setup] 1. Open BP_PlayerShip in Blueprint editor
[PIE Setup] 2. Verify it has: Static Mesh, Camera, SpaceshipControlsComponent
[PIE Setup] 3. Set 'Auto Possess Player' to 'Player 0'
[PIE Setup] 4. Compile and save
[PIE Setup] 5. Press Alt+P to test PIE!
[PIE Setup] ============================================================
```

---

## Troubleshooting

### "Python is not enabled"
1. **Edit** → **Project Settings**
2. **Plugins** → Search "Python"
3. Enable **Python Editor Script Plugin**
4. Restart Unreal Editor

### "Failed to load Blueprint"
→ Blueprint paths might be different  
→ Check the paths match your project structure  
→ Edit `AutoConfigurePIE.py` to fix paths

### Script runs but nothing changes
→ Check **Output Log** for error messages  
→ Blueprints might be checked out in version control  
→ Try manual configuration using `PIE_FIX_CHECKLIST.md`

### "No module named unreal"
→ Make sure you're running this IN Unreal Editor, not external Python  
→ Use **Tools → Execute Python Script** method

---

## After Running Script

### Still Need to Do Manually:

1. **Open BP_PlayerShip** in Blueprint editor
2. **Components** panel → **+ Add** button:
   - Add **Static Mesh Component** (set to visible mesh)
   - Add **Camera Component** (Location: X=-500, Y=0, Z=200)
   - Add **SpaceshipControlsComponent** (search in components list)
3. **Class Defaults** → **Auto Possess Player**: Set to **Player 0**
4. Click **Compile** and **Save**
5. Close Blueprint editor
6. Press **Alt+P** to test!

---

## Alternative: Full Manual Setup

If the script doesn't work, follow:
- **PIE_FIX_CHECKLIST.md** - Simple checklist
- **VISUAL_BLUEPRINT_SETUP.md** - Step-by-step with diagrams
- **QUICK_PIE_FIX.md** - 10-minute guide

---

**Script Location**: `c:\Unreal Projects\Adastrea\AutoConfigurePIE.py`  
**Time to Run**: < 1 minute  
**Manual Steps After**: ~5 minutes (BP_PlayerShip setup)  
**Total Time**: ~6 minutes to working PIE
