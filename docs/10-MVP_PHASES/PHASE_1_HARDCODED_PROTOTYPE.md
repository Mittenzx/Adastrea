# Phase 1: Hardcoded Prototype (Weeks 1-4)

**Goal**: Validate core trading gameplay loop is fun | **Status**: ✅ COMPLETE

---

## 📅 Timeline

| Week | Dates | Focus |
|------|-------|-------|
| 1-2 | Dec 29 – Jan 11 | Project setup, basic Blueprint structure, ship movement, station placement |
| 3-4 | Jan 12 – Jan 26 | Complete prototype, bug fixes, package for playtest, GO/NO-GO decision |

---

## ✅ What Was Built

### Hardcoded Systems (C++ Not Required)
- **Ship Movement**: Basic flight controls (WASD + mouse)
- **Stations**: 2 visual distinct stations placed in level
- **Trade Goods**: 3 items with hardcoded prices
- **Trading UI**: Basic buy/sell interface (UMG)
- **Profit Counter**: Credits tracking per session

### Hardcoded Values (Acceptable for Prototype)
```cpp
// In Blueprint or GameMode
float StationA_FoodPrice = 100.0f;
float StationB_FoodPrice = 150.0f;
int32 ShipCargoCapacity = 10;
```

---

## 🎮 Core Loop (Validated)
```
1. Spawn at Station A
2. Fly to Station B (~1 min)
3. Dock at Station B
4. Buy cargo (cheap)
5. Fly to Station A (~1 min)
6. Dock at Station A
7. Sell cargo (profit)
8. Repeat
```

**Time per loop**: 3-5 minutes
**Success Criteria** (All Met):
- ✅ Fly between stations in 1 minute
- ✅ Buy cargo at Station A
- ✅ Sell cargo at Station B
- ✅ See profit increase
- ✅ Run 10+ minutes without crashes
- ✅ 60%+ playtesters said "fun" → GO decision

---

## 📋 Daily Checklist (Reference)

### Week 1-2
- [ ] Project setup & Git init
- [ ] Basic Blueprint hierarchy
- [ ] Ship pawn with movement
- [ ] Station actors placed
- [ ] Docking collision/trigger
- [ ] Basic UI widget

### Week 3-4
- [ ] Hardcoded trade prices
- [ ] Buy/sell logic in UI
- [ ] Credits display
- [ ] Cargo capacity limit
- [ ] Package for playtest
- [ ] External playtest (5-10 people)
- [ ] GO/NO-GO decision

---

## 🔑 Key Lesson

> **Hardcode first, structure later.** The prototype validated the loop is fun without writing a single DataAsset.

---

*Back to [MVP Phases](../10-MVP_PHASES.md) | [INDEX.md](../INDEX.md)*