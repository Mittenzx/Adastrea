# BP_ImportAI Quick Start Guide

> **AI-controlled ship that flies autonomously to random coordinates**

## What You'll Get

An AI ship (BP_ImportAI) that:
- ✅ Spawns in your level
- ✅ Automatically starts flying
- ✅ Picks random coordinates and flies there
- ✅ Repeats forever - no player input needed!

## Quick Setup (3 Steps)

### Step 1: Build C++ Code

**In Visual Studio:**
1. Open `Adastrea.sln`
2. Set configuration to **Development Editor**
3. Build Solution (Ctrl+Shift+B)
4. Wait for "Build succeeded" message

**OR in Unreal Editor:**
1. Open Unreal Editor
2. Click **Tools** → **Refresh Visual Studio Project**
3. Go to **Tools** → **Compile**
4. Wait for compilation to finish

### Step 2: Create BP_ImportAI

**In Unreal Editor:**
1. Open **Window** → **Developer Tools** → **Output Log**
2. At the bottom Python console, type:
   ```python
   import sys
   sys.path.append('.')
   import CreateAIShip
   CreateAIShip.create_ai_ship()
   ```
3. Press Enter
4. Watch for "✅ Blueprint base created successfully!" message

**Alternative (Quick but less safe):**
```python
exec(open('CreateAIShip.py').read())
```

### Step 3: Add AI Component

1. In **Content Browser**, go to `Content/Blueprints/Ships/`
2. Double-click **BP_ImportAI**
3. Click **+ Add Component** button
4. Search for: `SimpleAIMovement`
5. Click **SimpleAIMovementComponent** to add it
6. In **Details** panel, set:
   - ✅ **Show Debug**: TRUE (for testing)
   - Leave other values at defaults
7. Click **Compile** button (top toolbar)
8. Click **Save** button (top toolbar)

## Test It!

1. Open any level (or create new one)
2. From **Content Browser** → `Content/Blueprints/Ships/`
3. **Drag BP_ImportAI** into the viewport
4. Press **Alt+P** (or click **Play**)
5. **Watch it fly!** 🚀

You should see:
- Ship automatically starts moving
- Green line showing its target
- Yellow sphere at destination
- Ship flies there, then picks new target

## Done!

That's it! Your AI ship is now flying around autonomously.

## Troubleshooting

**Ship doesn't move?**
- Check: Did you add SimpleAIMovementComponent?
- Check: Did you compile and save BP_ImportAI?
- Check: Is C++ code compiled?

**Can't find SimpleAIMovementComponent?**
- C++ code needs to be compiled first
- Restart Unreal Editor after compiling

**Script fails?**
- Make sure BP_Import exists in `Content/Blueprints/Ships/`
- Check Output Log for specific error

## Next Steps

- 🎯 Adjust movement speed in component properties
- 🎯 Add multiple AI ships to your level
- 🎯 Create variants with different behaviors
- 📖 See [BP_IMPORTAI_IMPLEMENTATION_GUIDE.md](BP_IMPORTAI_IMPLEMENTATION_GUIDE.md) for advanced usage

---

**Questions?** See the full implementation guide for detailed documentation.
