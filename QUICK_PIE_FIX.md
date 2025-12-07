# QUICK PIE FIX - Get Flying in 10 Minutes

## Problem
You click PIE but can't fly spaceships or see HUD.

## Root Cause
Game mode doesn't know what pawn to spawn or which controller to use.

---

## 🚀 FASTEST FIX (Do This First)

### 1. Open Unreal Editor
Launch `Adastrea.uproject`

### 2. Open Game Mode Blueprint (2 min)
1. **Content Browser** → `Content/Blueprints/`
2. Double-click `BP_TestGameMode`
3. In **Class Defaults** panel (right side):
   - **Default Pawn Class**: Click dropdown → Select `BP_PlayerShip`
   - **Player Controller Class**: Click dropdown → Select `BP_PlayerController`
   - **HUD Class**: Leave as default (None) for now
4. Click **Compile** button (top-left)
5. Click **Save** button

### 3. Open Player Ship Blueprint (3 min)
1. **Content Browser** → `Content/Blueprints/Ships/`
2. Double-click `BP_PlayerShip`
3. In **Components** panel (left side):
   - If no visible mesh: Click **Add** → **Static Mesh**
   - Set **Static Mesh** to any visible object:
     - Engine Content: `Cube` or `Sphere`
     - Or: Import a simple spaceship mesh
   - Click **Add** → **Camera** (Spring Arm Camera if available)
   - Set Camera **Location**: X=`-500`, Y=`0`, Z=`200` (behind and above)
4. In **Components** panel:
   - Click **Add** → Search: `SpaceshipControlsComponent`
   - Add it to the ship
5. Click **Compile** and **Save**

### 4. Open Player Controller Blueprint (3 min)
1. **Content Browser** → `Content/Blueprints/Controllers/`
2. Double-click `BP_PlayerController`
3. Click **Event Graph** tab
4. **Delete existing nodes** if any basic setup exists
5. **Right-click** → Type: `Event BeginPlay`
6. Create this node chain:

```
[Event BeginPlay]
  ↓ (Execution pin)
[Get Player Controller] (drag from Self → Cast To APlayerController)
  ↓ (Execution pin)
[Get Enhanced Input Local Player Subsystem]
  - Player Controller: wire from Cast result
  ↓ (Execution pin)
[Add Mapping Context]
  - Target: wire from Subsystem result
  - Mapping Context: Select `IMC_Spaceship` from dropdown
  - Priority: 0
```

**Simplified version if you get stuck**:
- Just ensure `Event BeginPlay` exists
- Add mapping context manually in C++ (we'll do this if Blueprint is confusing)

7. Click **Compile** and **Save**

### 5. Test PIE (1 min)
1. Open `Content/Maps/TestLevel`
2. Press **Alt + P** or click **Play ▶** button
3. **Try**: WASD to move, Mouse to look, Mouse wheel to adjust speed

---

## ⚠️ If It Still Doesn't Work

### Check #1: Do you see anything at all?
**NO** → Problem: No camera or spawning failed
**Solution**: 
- Add **Player Start** actor to TestLevel
- Verify BP_PlayerShip has a Camera component

### Check #2: Can you see ship but not move?
**YES** → Problem: Input not connected
**Solution**: 
- Open **Window** → **Developer Tools** → **Output Log**
- Look for errors mentioning "Input" or "SpaceshipControls"
- Verify `Content/Input/IMC_Spaceship` exists and has key mappings

### Check #3: Ship rotates wildly or doesn't respond correctly?
**YES** → Problem: Physics or input sensitivity
**Solution**:
- Open BP_PlayerShip
- **Class Defaults** → **Spaceship Controls Component**:
  - **Look Sensitivity**: 0.5 (reduce if too sensitive)
  - **Movement Speed**: 1.0 (default)

---

## 🔧 Nuclear Option: C++ Direct Fix

If Blueprints are completely broken, edit C++ directly:

### Edit `AdastreaPlayerController.cpp`

Add to `BeginPlay()`:

```cpp
void AAdastreaPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    // Add Enhanced Input mapping context
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        // Load the mapping context
        UInputMappingContext* SpaceshipContext = LoadObject<UInputMappingContext>(
            nullptr,
            TEXT("/Game/Input/IMC_Spaceship.IMC_Spaceship")
        );
        
        if (SpaceshipContext)
        {
            Subsystem->AddMappingContext(SpaceshipContext, 0);
            UE_LOG(LogAdastrea, Log, TEXT("Added spaceship input mapping context"));
        }
    }
    
    // Create basic HUD
    if (UWorld* World = GetWorld())
    {
        // Simple debug HUD (you can replace with proper widget later)
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                TEXT("Controls: WASD=Move, Mouse=Look, Scroll=Speed"));
        }
    }
}
```

**Include headers** (add to top of `.cpp` file):
```cpp
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Engine/Engine.h"
```

**Recompile**:
1. Close Unreal Editor
2. Build solution in Visual Studio (Ctrl+Shift+B)
3. Reopen Unreal Editor
4. Test PIE again

---

## 📋 Minimal Required Setup

To fly a spaceship in PIE, you MUST have:

1. ✅ **Game Mode** with Default Pawn set → `BP_PlayerShip`
2. ✅ **Player Ship** with:
   - Visible mesh (cube is fine)
   - Camera component
   - SpaceshipControlsComponent
3. ✅ **Player Controller** that:
   - Adds Input Mapping Context (`IMC_Spaceship`)
4. ✅ **Input Mapping Context** (`IMC_Spaceship`) with:
   - WASD → IA_Move action
   - Mouse XY → IA_Look action
   - Mouse Wheel → IA_Speed action (optional)
5. ✅ **Map** with:
   - Directional Light (so you can see)
   - Player Start actor (spawn point)

---

## 🎯 Success Criteria

After the fix, when you press PIE:

✅ You spawn in a visible object (ship)  
✅ WASD moves the ship in different directions  
✅ Mouse moves your view/rotates ship  
✅ You can see the environment (lights work)  
✅ No errors in Output Log

---

## 📞 If You're Still Stuck

### Debug Checklist:

1. **Open Output Log**: Window → Developer Tools → Output Log
2. **Press PIE**
3. **Look for errors** with keywords:
   - "Input"
   - "Spaceship"
   - "Spawn"
   - "Controller"
   - "Pawn"

4. **Take screenshot of error** and check against these common issues:

**Error: "No default pawn class"**
→ Fix: Set Default Pawn Class in BP_TestGameMode

**Error: "Failed to add mapping context"**
→ Fix: Verify IMC_Spaceship exists in Content/Input/

**Error: "InputComponent is not an EnhancedInputComponent"**
→ Fix: Already configured correctly in DefaultEngine.ini (line 13-14)

**Error: "SpaceshipControlsComponent: Owner is not a Pawn"**
→ Fix: Ensure BP_PlayerShip parent class is `Spaceship` or `Pawn`

---

## 🏁 Next Steps After It Works

Once you can fly:

1. **Add better mesh**: Import or buy a spaceship model
2. **Add real HUD**: Configure WBP_SpaceShipHUD widget
3. **Add environment**: Space stations, planets, asteroids
4. **Add interaction**: Docking, trading, combat
5. **Polish controls**: Sensitivity, boost, speed limits

---

**Generated**: 2025-11-29  
**For**: Quick troubleshooting PIE issues  
**Time**: ~10 minutes if following exactly  
**Difficulty**: Beginner-friendly
