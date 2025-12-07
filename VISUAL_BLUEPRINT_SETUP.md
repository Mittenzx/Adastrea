# Visual Blueprint Setup Guide - PIE Quick Fix

## 🎯 Goal
Get from "nothing works" to "flying spaceships" in 15 minutes.

---

## Step 1: Game Mode Setup (3 min)

### Open BP_TestGameMode
**Path**: `Content/Blueprints/BP_TestGameMode`

### Class Defaults Panel
```
┌─────────────────────────────────────────┐
│ Class Defaults                           │
├─────────────────────────────────────────┤
│ ▼ Classes                                │
│                                          │
│   Default Pawn Class:                    │
│   ┌──────────────────────────────────┐  │
│   │ BP_PlayerShip              ▼    │  │ ← CLICK HERE and select BP_PlayerShip
│   └──────────────────────────────────┘  │
│                                          │
│   Player Controller Class:               │
│   ┌──────────────────────────────────┐  │
│   │ BP_PlayerController        ▼    │  │ ← CLICK HERE and select BP_PlayerController
│   └──────────────────────────────────┘  │
│                                          │
│   HUD Class: None                        │
│                                          │
└─────────────────────────────────────────┘
```

**Action**: 
1. Click dropdown for "Default Pawn Class"
2. Type "BP_PlayerShip" → Select it
3. Click dropdown for "Player Controller Class"  
4. Type "BP_PlayerController" → Select it
5. Click **Compile** button (top toolbar)
6. Click **Save** button

---

## Step 2: Player Ship Setup (7 min)

### Open BP_PlayerShip
**Path**: `Content/Blueprints/Ships/BP_PlayerShip`

### Components Panel (Left Side)
```
┌─────────────────────────────────────┐
│ Components                           │
├─────────────────────────────────────┤
│ ▼ BP_PlayerShip (self)               │
│   ├─ 🔷 Root (DefaultSceneRoot)      │
│   ├─ 📦 StaticMeshComponent ←─────┐  │ ADD THIS if missing
│   ├─ 📹 Camera                    │  │ ADD THIS if missing
│   └─ ⚙️ SpaceshipControlsComponent│  │ ADD THIS if missing
└─────────────────────────────────────┘
         ↑ Click "+ Add" button to add these
```

### Add Components:

#### A. Add Static Mesh (For Visibility)
1. Click **+ Add** button
2. Search: "Static Mesh"
3. Select **Static Mesh Component**
4. In **Details** panel → **Static Mesh**:
   - Click dropdown → Search "cube" or "sphere"
   - OR: Use any spaceship mesh you have
5. Set **Scale**: X=2, Y=2, Z=1 (makes it easier to see)

#### B. Add Camera
1. Click **+ Add** button
2. Search: "Camera"
3. Select **Camera Component**
4. In **Details** panel → **Transform** → **Location**:
   - X: `-500` (behind ship)
   - Y: `0` (centered)
   - Z: `200` (above ship)
5. In **Details** panel → **Transform** → **Rotation**:
   - Pitch: `-10` (look slightly down)

#### C. Add Spaceship Controls Component
1. Click **+ Add** button
2. Search: "SpaceshipControlsComponent"
3. Select **Spaceship Controls Component** (from C++ class)
4. Keep default settings

### Class Defaults Panel
```
┌──────────────────────────────────────┐
│ Class Defaults                        │
├──────────────────────────────────────┤
│ ▼ Pawn                                │
│                                       │
│   Auto Possess Player:                │
│   ┌────────────────────────────────┐ │
│   │ Player 0               ▼       │ │ ← Set to "Player 0"
│   └────────────────────────────────┘ │
│                                       │
│   Auto Possess AI: Disabled           │
│                                       │
└──────────────────────────────────────┘
```

**Action**:
1. Select root component in hierarchy
2. Find "Auto Possess Player" in Class Defaults
3. Set to **Player 0**
4. Click **Compile** and **Save**

---

## Step 3: Player Controller Setup (3 min)

### Open BP_PlayerController
**Path**: `Content/Blueprints/Controllers/BP_PlayerController`

### Class Defaults Panel
```
┌────────────────────────────────────────┐
│ Class Defaults                          │
├────────────────────────────────────────┤
│ ▼ HUD                                   │
│                                         │
│   HUD Widget Class:                     │
│   ┌──────────────────────────────────┐ │
│   │ WBP_SpaceShipHUD         ▼      │ │ ← CLICK and select WBP_SpaceShipHUD
│   └──────────────────────────────────┘ │
│                                         │
│   HUD Widget: None (Auto-created)       │
│                                         │
├────────────────────────────────────────┤
│ ▼ Mouse Interface                       │
│                                         │
│   ☐ Show Mouse Cursor               │ │ ← UNCHECK this
│   ☐ Enable Click Events             │ │
│   ☐ Enable Mouse Over Events        │ │
│                                         │
└────────────────────────────────────────┘
```

**Action**:
1. Find "HUD Widget Class"
2. Click dropdown → Search "WBP_SpaceShipHUD" → Select it
3. Uncheck "Show Mouse Cursor"
4. Click **Compile** and **Save**

**Note**: The C++ code now handles input setup automatically! No Event Graph needed.

---

## Step 4: Basic HUD Setup (2 min)

### Open WBP_SpaceShipHUD
**Path**: `Content/UI/HUD/WBP_SpaceShipHUD`

### Designer View
```
┌──────────────────────────────────────────────────────┐
│  Canvas Panel                                         │
│                                                       │
│  ┌────────────────┐                  ┌─────────────┐│
│  │ Text Block      │                  │ Text Block  ││
│  │ "Speed: 0"      │                  │ "Health: 100"││
│  └────────────────┘                  └─────────────┘│
│   ↑ Top-Left                           Top-Right ↑  │
│                                                       │
│                                                       │
│              ┌──────────────────────────┐            │
│              │ Text Block               │            │
│              │ "Controls: WASD=Move"    │            │
│              │ "Mouse=Look, Scroll=Speed"│            │
│              └──────────────────────────┘            │
│                     ↑ Center                          │
│                                                       │
└──────────────────────────────────────────────────────┘
```

**Add These Elements**:

#### A. Speed Display (Top-Left)
1. **Palette** → Search "Text"
2. Drag **Text Block** onto Canvas
3. **Details** panel:
   - **Anchors**: Top-Left
   - **Position**: X=20, Y=20
   - **Text**: "Speed: 0"
   - **Font Size**: 18
   - **Color**: White or Cyan

#### B. Health Display (Top-Right)
1. Drag another **Text Block** onto Canvas
2. **Details** panel:
   - **Anchors**: Top-Right
   - **Position**: X=-150, Y=20
   - **Text**: "Health: 100"
   - **Font Size**: 18
   - **Color**: Green

#### C. Controls Help (Center)
1. Drag another **Text Block** onto Canvas
2. **Details** panel:
   - **Anchors**: Center
   - **Position**: X=0, Y=200
   - **Text**: 
     ```
     Controls: WASD=Move, Mouse=Look, Scroll=Speed
     ```
   - **Font Size**: 14
   - **Color**: Light Gray
   - **Justification**: Center

**Action**: Click **Compile** and **Save**

---

## Step 5: Verify Test Level (Optional - 2 min)

### Open TestLevel
**Path**: `Content/Maps/TestLevel`

### World Outliner - Check These Exist:
```
┌─────────────────────────────┐
│ World Outliner               │
├─────────────────────────────┤
│ 🔆 DirectionalLight          │ ← MUST HAVE (or you see nothing)
│ 🌍 PlayerStart               │ ← MUST HAVE (spawn location)
│ 💨 AtmosphericFog           │ ← Nice to have
│ ☁️ SkyAtmosphere            │ ← Nice to have
└─────────────────────────────┘
```

### Add If Missing:

#### Add Directional Light
1. **Place Actors** panel
2. Search: "Directional Light"
3. Drag into level
4. Rotate to point downward

#### Add Player Start
1. **Place Actors** panel
2. Search: "Player Start"
3. Drag into level at origin (0,0,0)

### World Settings
```
┌─────────────────────────────────────┐
│ World Settings                       │
├─────────────────────────────────────┤
│ ▼ Game Mode                          │
│                                      │
│   Game Mode Override:                │
│   ┌────────────────────────────────┐│
│   │ BP_TestGameMode        ▼      ││ ← Verify this is set
│   └────────────────────────────────┘│
│                                      │
└─────────────────────────────────────┘
```

**Action**: Save map

---

## 🎮 Testing Time!

### Press PIE (Alt+P or Play Button)

### Expected Results:
```
✅ You spawn in a visible object (cube/ship)
✅ WASD keys move the ship in different directions
✅ Mouse moves your camera view
✅ Mouse wheel up/down changes speed
✅ HUD shows at least basic text
✅ No errors in Output Log
```

### Keyboard Controls:
```
W - Move forward (up in space)
S - Move backward (down in space)
A - Strafe left
D - Strafe right
Mouse - Rotate camera/ship
Mouse Wheel Up - Increase speed
Mouse Wheel Down - Decrease speed
Shift - Boost (may not work yet)
Esc - Exit PIE
```

---

## 🔧 Troubleshooting

### "I don't see anything"
**Check**:
1. BP_PlayerShip has a visible mesh
2. Camera component exists
3. Directional Light in level
4. Output Log for spawn errors

### "I spawn but camera is weird"
**Fix**: Adjust Camera location in BP_PlayerShip
- More negative X = farther behind
- Higher Z = higher above

### "Can't move at all"
**Check Output Log for**:
- "Added spaceship input mapping context" (should see this)
- If missing: IMC_Spaceship may not exist or have mappings

### "Ship rotates but doesn't move"
**Fix**: Ensure SpaceshipControlsComponent is added to BP_PlayerShip

### "No HUD"
**Check**:
- BP_PlayerController has HUD Widget Class set
- WBP_SpaceShipHUD has text blocks
- Output Log for widget creation

---

## 📸 Quick Reference - What Success Looks Like

### When PIE is Working:
1. **Viewport**: Shows your ship (cube/mesh) from behind
2. **HUD**: Text overlays visible on screen
3. **Movement**: Ship responds to WASD keys immediately
4. **Camera**: Mouse smoothly rotates view
5. **Output Log**: Shows "Added spaceship input mapping context"

### Output Log Success Messages:
```
LogAdastrea: AdastreaPlayerController: Added spaceship input mapping context
LogAdastrea: AdastreaPlayerController: Created and initialized HUD widget
LogAdastreaInput: SpaceshipControlsComponent: Initialized on BP_PlayerShip_C_0
```

---

## ✨ You're Done!

If you see the ship, can move with WASD, and rotate with mouse:

**🎉 CONGRATULATIONS! PIE IS WORKING! 🎉**

### Next Steps:
1. Replace cube with proper spaceship mesh
2. Design better HUD with real-time data
3. Add weapons and combat
4. Add space stations and objects to interact with
5. Have fun! 🚀

---

**Time to Complete**: ~15 minutes  
**Difficulty**: Easy (just follow the boxes)  
**Result**: Fully playable spaceship in PIE  
**Status**: C++ code handles input automatically ✅
