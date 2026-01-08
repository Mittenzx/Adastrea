# Player System Quick Reference

**For**: Designers, developers, and anyone asking "Where is the player?"  
**Status**: MVP Phase (Ship-Only)  
**Last Updated**: 2026-01-08

---

## Quick Answer

### "Where is the player?"

**MVP (Now)**: Player = Spaceship pawn  
**Post-MVP (Later)**: Player = Spaceship OR Character pawn

---

## MVP Phase (Current)

### What Exists

✅ **Player Controller** (`AAdastreaPlayerController`)
- Manages input
- Possesses spaceship
- Opens/closes UI
- Handles docking/undocking

✅ **Spaceship Pawn** (`ASpaceship`)
- Player-controlled flying pawn
- Has cargo space
- Can dock at stations
- Upgradeable

### What Doesn't Exist

❌ **Walking Character** - Not in MVP
❌ **Interior Exploration** - Not in MVP
❌ **Station Walking** - Not in MVP
❌ **First-Person View** - Not in MVP

### MVP Gameplay

```
1. Start game → Player spawns in spaceship
2. Fly spaceship to Station A
3. Dock at Station A
4. Trading UI opens (automatic)
5. Buy/sell cargo
6. Undock
7. Fly to Station B
8. Repeat
```

**Player never leaves ship.**

---

## Post-MVP Phase (Future)

### What Will Be Added

✅ **Character Class** (`AAdastreaCharacter`)
- Walking character pawn
- Third-person movement
- Interaction system
- Station exploration

✅ **Possession Switching**
- Exit ship → Spawn character
- Board ship → Destroy character
- Seamless transitions

✅ **Interior Exploration**
- Walk inside ships
- Use control consoles
- Explore stations
- Talk to NPCs

### Post-MVP Gameplay

```
1. Start game → Player spawns in spaceship
2. Fly to Station A
3. Dock at Station A
4. Exit ship → Character spawns
5. Walk through station
6. Interact with trader NPC
7. Trading UI opens
8. Complete trade
9. Walk back to ship
10. Board ship → Character destroyed
11. Undock and fly away
```

**Player switches between ship and character.**

---

## Technical Details

### Current Classes (MVP)

| Class | Purpose | Status |
|-------|---------|--------|
| `AAdastreaPlayerController` | Manages player | ✅ Active |
| `ASpaceship` | Player's flying pawn | ✅ Active |
| `ASpaceshipInterior` | Interior data (unused) | ⏸️ Dormant |
| `AShipControlConsole` | Console data (unused) | ⏸️ Dormant |

### Future Classes (Post-MVP)

| Class | Purpose | Status |
|-------|---------|--------|
| `AAdastreaCharacter` | Walking character | ❌ Not implemented |
| Character Movement | WASD controls | ❌ Not implemented |
| Interaction System | E key interactions | ❌ Not implemented |
| Airlock Triggers | Ship entry/exit | ❌ Not implemented |

---

## For Designers

### Creating MVP Content

**Ship-Based Trading:**
```
1. Create trading station Blueprint
2. Add docking trigger volume
3. Configure trading UI to open on dock
4. Set market prices in station data
5. Test: Dock → UI opens → Buy/Sell → Undock
```

**No character setup needed!**

### Planning Post-MVP Content

**Interior Spaces:**
```
1. Model ship interior mesh
2. Add collision for walkable floors
3. Place control console
4. Set entry/exit points
5. Add props and details
```

**Station Walkable Areas:**
```
1. Create station interior level
2. Add NavMesh for pathfinding
3. Place NPC traders
4. Add interaction triggers
5. Configure trading UI on NPC interaction
```

---

## Common Questions

### Q: Can players walk around in MVP?
**A**: No. MVP is ship-only. Walking added post-MVP.

### Q: How do players trade in MVP?
**A**: Dock at station → UI opens automatically → Buy/sell → Undock.

### Q: When will character be added?
**A**: After MVP validation (Week 12+). Only if MVP hits success metrics.

### Q: Is interior exploration important?
**A**: Not for MVP. Trading loop validation is priority. Interior exploration is enhancement.

### Q: Can we skip character and stay ship-only?
**A**: Possibly! If MVP players love ship-only trading, we may not need character at all.

### Q: What about multiplayer?
**A**: Way post-MVP. Single-player first.

---

## Decision Tree

### Should I Work on Character?

```
Is MVP validated? (75% fun rating achieved)
  ├─ NO → Focus on MVP ship/trading gameplay
  └─ YES → Is there funding/runway for 2-3 months?
            ├─ NO → Keep iterating MVP
            └─ YES → Start character implementation
```

### Should I Plan for Interior?

```
Is this interior critical for MVP trading?
  ├─ NO → Defer to post-MVP
  └─ YES → Rethink - MVP is ship-only trading
```

---

## File References

### MVP Documentation
- `docs/development/PLAYER_CHARACTER_ARCHITECTURE.md` - Full architecture doc
- `.github/instructions/trade-simulator-mvp.instructions.md` - MVP scope
- `ARCHITECTURE.md` - Overall architecture with player section

### Post-MVP Documentation
- `Assets/ShipControlGuide.md` - Interior control console
- `Assets/SpaceshipInteriorGuide.md` - Interior implementation
- `Docs/PLAYER_CONTROLS_DIAGRAMS.md` - Control flow diagrams

### Code Files
- `Source/Adastrea/Public/Player/AdastreaPlayerController.h` - Player controller
- `Source/Adastrea/Public/Ships/Spaceship.h` - Ship pawn
- `Source/Adastrea/Public/Ships/SpaceshipInterior.h` - Interior class (future)
- `Source/Adastrea/Public/Ships/ShipControlConsole.h` - Console class (future)

---

## Version History

- **v1.0** (2026-01-08) - Initial quick reference for MVP phase

---

**Remember**: MVP = Ship-only. Character = Post-MVP. Keep it simple!
