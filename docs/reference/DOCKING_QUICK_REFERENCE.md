# Docking Quick Reference Card

**Last Updated**: January 8, 2026  
**System**: Simplified Instant Docking (MVP)

---

## ⚡ Quick Start (30 Seconds)

1. **Create input action**: `IA_Dock` (Digital/Boolean)
2. **Map F key**: In `IMC_Spaceship` → Map F to `IA_Dock`
3. **Configure asset**: `DA_InputConfig` → Set `DockAction = IA_Dock`
4. **Station setup**: Add `DockingBayModule` + Scene Components as docking points
5. **Test**: Fly near station (within 2000 units), press F

---

## 🎮 Player Controls

| Action | Key | Function |
|--------|-----|----------|
| Dock | **F** | Dock at nearby station (when in range) |
| Undock | **F** | Leave station and resume flight |
| Trade | **Mouse** | Use trading UI while docked |

---

## 📍 Key Values

| Property | Default | Description |
|----------|---------|-------------|
| `DockingRange` | 2000.0 | Max distance to dock (units) |
| `MaxDockedShips` | 4 | Docking capacity per station |

---

## ⚠️ Common Issues

| Problem | Solution |
|---------|----------|
| F key does nothing | Check input binding in `DA_InputConfig` |
| "No station in range" | Fly closer (within 2000 units) |
| "No docking slots" | Station full - try another bay |
| Ship teleports weird | Fix docking point rotation in Blueprint |
| Trading UI doesn't open | Assign `TradingInterfaceClass` in ship |

---

## 🔍 Debug Console Commands

```
// Show docking range
DisplayAll ASpaceship DockingRange

// Show station locations
DisplayAll DockingBayModule Location

// Show available slots
DisplayAll DockingBayModule CurrentDockedShips
```

---

## 📂 Key Files

| File | Purpose |
|------|---------|
| `Spaceship.h` (lines 338-396) | Docking system declarations |
| `Spaceship.cpp` (lines 997-1214) | Docking implementation |
| `DockingBayModule.h` | Station docking component |
| `DA_InputConfig` | Input action configuration |

---

## 🚀 Code Snippet: Custom Docking Range

```cpp
// In Blueprint or C++:
Ship->DockingRange = 5000.0f; // Increase range

// Or in Blueprint:
// Select ship → Details → Docking → Docking Range = 5000
```

---

## 📊 Function Call Order

```
F key pressed
  ↓
RequestDocking()     // Validate and check distance
  ↓
NavigateToDockingPoint() // Instant teleport
  ↓
CompleteDocking()    // Open trade UI
```

---

## ✅ Validation Checks

Before docking succeeds, system checks:
1. ✓ Station nearby?
2. ✓ Is DockingBayModule?
3. ✓ Has available slots?
4. ✓ Within docking range?
5. ✓ Valid docking point?

All must pass! ❌ Any fails = warning logged.

---

## 🎨 Widget Classes Needed

| Property | Type | Purpose |
|----------|------|---------|
| `DockingPromptWidgetClass` | UUserWidget | "Press F to Dock" prompt |
| `TradingInterfaceClass` | UUserWidget | Trading/cargo UI |

---

## 🔧 Blueprint Node Reference

**Check if docked:**
```
Ship → Is Docked → bool
```

**Check if in docking sequence:**
```
Ship → Is Docking → bool
```

**Manual dock (advanced):**
```
Ship → Request Docking
```

**Manual undock:**
```
Ship → Undock
```

---

## 🎯 MVP Philosophy

**Current Focus**: Simple, instant docking for gameplay testing  
**Not Current Focus**: Cinematic animations, manual flight  
**After MVP**: Polish, effects, realistic physics

---

## 📚 Full Documentation

- **Setup Guide**: `docs/reference/SIMPLE_DOCKING_SETUP.md`
- **Visual Reference**: `docs/reference/DOCKING_VISUAL_REFERENCE.md`
- **Trade Simulator MVP**: `.github/instructions/trade-simulator-mvp.instructions.md`

---

## 💡 Pro Tips

1. **Test with 2 stations** minimum to verify trade loop
2. **Set different prices** at each station for trading
3. **Start with large docking range** (5000) while learning
4. **Use Debug Camera** (F8) to see docking points in editor
5. **Check Output Log** for helpful warnings

---

**Need help?** Check the full guides or open a GitHub issue! 🚀
