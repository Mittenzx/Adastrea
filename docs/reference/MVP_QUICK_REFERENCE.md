# MVP Developer Quick Reference

**Purpose**: Fast answers for common MVP development questions  
**Target**: Developers working on Trade Simulator MVP  
**Last Updated**: 2025-12-24

---

## 🚨 MOST IMPORTANT QUESTIONS

### "Can I add [feature X]?"

**Ask these 3 questions**:
1. Is this required for buy/sell/upgrade loop? → If NO, defer
2. Can players have fun without this? → If YES, defer
3. Would removing this break the core loop? → If NO, defer

**Standard Response**: "Great idea for v2.0! Let's validate trading is fun first."

---

### "Which systems should I use?"

**MVP-CRITICAL (Use Full)**:
- ✅ Trading System (core)
- ✅ Spaceship System (flight + cargo)
- ✅ Space Station System (docking)
- ✅ HUD/UI System (minimal)
- ✅ Save System (basic)

**SIMPLIFIED (Limited Use)**:
- ⚠️ Faction System (prices only)
- ⚠️ Navigation (waypoints only)
- ⚠️ Inventory (cargo only)

**DEFERRED (Don't Touch)**:
- 🚫 Combat, Exploration, Quests, Personnel, AI, etc.

See: [System Classification](../mvp/SYSTEM_CLASSIFICATION.md)

---

### "How do I build the project?"

**ONE Recommended Path**:
1. Install UE 5.6 + VS 2022
2. Generate project files (right-click .uproject)
3. Build in "Development Editor" configuration
4. Launch editor

See: [Simplified Build Guide](SIMPLIFIED_BUILD_GUIDE.md)

---

### "What should I build this week?"

**Week 1-4**: Hardcoded prototype (no Data Assets)  
**Week 5-8**: Convert to Data Assets, add variety  
**Week 9-12**: Polish and demo package

See: [MVP README](../mvp/README.md)

---

### "Where do I put documentation?"

**DO**:
- Update CHANGELOG.md for changes
- Put docs in `docs/mvp/`, `docs/development/`, `docs/reference/`
- Create docs AFTER implementation

**DON'T**:
- Create new summary files in root
- Document features before they exist
- Create multiple guides for same task

See: Anti-Pattern #3 (Documentation Obsession)

---

## 📁 File Organization

### Where to Find Things

```
Adastrea/
├── CHANGELOG.md              ← Update this for changes
├── README.md                 ← Project overview
├── CRITICAL_REVIEW_*.md      ← Why we're pivoting
├── .github/
│   └── instructions/         ← Agent guidelines
│       ├── trade-simulator-mvp.instructions.md
│       └── anti-patterns.instructions.md
├── docs/
│   ├── mvp/                  ← MVP-specific docs
│   │   ├── README.md         ← MVP overview
│   │   └── SYSTEM_CLASSIFICATION.md
│   ├── development/          ← Dev guides
│   │   ├── SIMPLIFIED_BUILD_GUIDE.md
│   │   └── PERFORMANCE_MEASUREMENT.md
│   └── reference/            ← Quick references
│       └── MVP_QUICK_REFERENCE.md ← This file
├── Source/Adastrea/          ← C++ code
│   ├── Trading/              ← MVP-critical
│   ├── Ships/                ← MVP-critical
│   ├── Stations/             ← MVP-critical
│   └── [Other systems]       ← Mostly deferred
└── Content/                  ← Unreal assets
```

---

## 🎯 Success Metrics

### Week 4 (Prototype Gate)
- [ ] 10-minute playable experience
- [ ] 60%+ "had fun" rating
- [ ] 0 critical bugs
- [ ] Core loop is clear

**If not met**: Pivot or kill

### Week 12 (Demo Gate)
- [ ] 30-minute polished demo
- [ ] 75%+ "had fun" rating
- [ ] 50%+ "would buy" interest
- [ ] <5 known bugs

**If not met**: Extend polish phase

---

## 💻 Common Tasks

### Add a New Trade Good

**Week 1-4** (Hardcoded):
```cpp
// Just hardcode it
float WaterPrice_Station1 = 10.0f;
float WaterPrice_Station2 = 15.0f;
```

**Week 5-8** (Data Asset):
1. Create Blueprint based on `UTradeItemDataAsset`
2. Set properties in editor
3. Add to market configuration

### Add a New Station

**Week 1-4** (Hardcoded):
```cpp
// Place actor in level
// Hardcode prices in level blueprint
```

**Week 5-8** (Data Asset):
1. Create Blueprint based on `ASpaceStation`
2. Create `UMarketDataAsset` for station
3. Set price multipliers
4. Place in level

### Check Performance

```cpp
// In console (~ key):
stat fps          // Show FPS
stat unit         // Show frame time
stat memory       // Show memory usage
```

See: [Performance Measurement](PERFORMANCE_MEASUREMENT.md)

---

## 🛠️ Code Patterns

### Hardcoded Prototype Pattern (Week 1-4)

```cpp
// It's OK to hardcode for prototype
class APrototypeStation : public AActor
{
    // Hardcoded prices
    float WaterPrice = 10.0f;
    float FuelPrice = 20.0f;
    
    // Hardcoded inventory
    int32 WaterStock = 100;
    int32 FuelStock = 50;
    
    // Simple buy function
    bool BuyItem(FString ItemName, int32 Quantity);
};
```

**This is TEMPORARY and that's OK**

### Data Asset Pattern (Week 5-8)

```cpp
// Convert to structured approach
class ATradingStation : public AActor
{
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UMarketDataAsset* MarketData;  // Reference to data
    
    UFUNCTION(BlueprintCallable)
    float GetItemPrice(FName ItemID) const;
};
```

---

## 🚫 Anti-Patterns to Avoid

### 1. Architecture Before Gameplay
❌ Don't build perfect systems before validating fun  
✅ Build ugly but playable, validate, then structure

### 2. Over-Engineering
❌ Don't expose all functions to Blueprints  
✅ Expose 20% (what designers actually need)

### 3. Documentation Obsession
❌ Don't write docs for features that don't exist  
✅ Code first, document after validation

### 4. Feature Creep
❌ Don't add "just one more system"  
✅ Stick to buy/sell/upgrade loop only

### 5. Working in Isolation
❌ Don't develop for 3 months without playtesters  
✅ Get feedback at Week 4 or pivot

See: [Anti-Patterns Guide](../../.github/instructions/anti-patterns.instructions.md)

---

## 📊 Performance Targets

| Metric | Target | Minimum |
|--------|--------|---------|
| **FPS** | 60 | 30 |
| **Memory** | <2 GB | <4 GB |
| **Load Time** | <30 sec | <60 sec |

**Measure every build, every playtest**

---

## ✅ Daily Checklist

- [ ] Code compiles (0 errors)
- [ ] Can launch and play 5 minutes
- [ ] One feature/fix completed
- [ ] Performance checked
- [ ] Progress noted

---

## 📞 Getting Help

### "Build is broken"
1. Check [Simplified Build Guide](SIMPLIFIED_BUILD_GUIDE.md)
2. Try regenerating project files
3. Try clean build
4. If >1 hour stuck, ask team

### "Performance is bad"
1. Check [Performance Measurement](PERFORMANCE_MEASUREMENT.md)
2. Run profiling tools
3. Identify bottleneck
4. If can't fix in 1 hour, ask team

### "Not sure if feature is MVP-critical"
1. Check [System Classification](../mvp/SYSTEM_CLASSIFICATION.md)
2. Ask 3 questions (top of this doc)
3. When in doubt, defer

---

## 🎓 Key Documents to Read

**Must Read** (before starting):
1. [Critical Review Summary](../../CRITICAL_REVIEW_SUMMARY.md) - Why we're here
2. [MVP Instructions](../../.github/instructions/trade-simulator-mvp.instructions.md) - What to build
3. [Anti-Patterns](../../.github/instructions/anti-patterns.instructions.md) - What to avoid

**Reference** (as needed):
4. [System Classification](../mvp/SYSTEM_CLASSIFICATION.md) - What systems to use
5. [Build Guide](SIMPLIFIED_BUILD_GUIDE.md) - How to build
6. [Performance Guide](PERFORMANCE_MEASUREMENT.md) - How to measure

---

## 💡 Key Principles

### The Golden Rule
**One polished gameplay loop beats ten half-finished systems**

### The Three Questions
1. Is this MVP-critical?
2. Can players have fun without this?
3. Would removing this break core loop?

### The Validation Gates
- Week 4: 60% fun or pivot
- Week 12: 75% fun, 50% buy or extend

### The Build Philosophy
- Hardcode → Validate → Structure → Polish
- Not: Perfect → Perfect → Perfect → Ship

---

## 🚨 Scope Creep Responses

When someone suggests:
- ❌ "Let's add combat" → "After MVP validation"
- ❌ "Let's add crew management" → "After MVP validation"
- ❌ "Let's add exploration" → "After MVP validation"
- ❌ "Let's add quests" → "Trading IS the game"
- ❌ "Let's add multiplayer" → "After MVP validation"

**Standard response**: "Great idea for v2.0! Let's validate the trading loop is fun first."

---

## 📈 Progress Tracking

Track in CHANGELOG.md:
- Major milestones
- System completions
- Playtest results
- Scope changes
- Performance metrics

**Don't create new files for status updates**

---

**Remember**: You're building a **fun game**, not **perfect code**.

---

**Last Updated**: 2025-12-24  
**Maintained By**: Adastrea MVP Team
