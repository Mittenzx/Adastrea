# Anti-Patterns & Lessons Learned

**Trade Simulator MVP | 18 Mistakes to Avoid | Last Updated: August 9, 2026**

---

## 🎯 Origin

Documented during **December 2025 Critical Review** after 3 months of development produced:
- ✅ 33K+ lines of C++ across 22 systems
- ✅ Excellent architecture, clean code
- ❌ **Zero playable gameplay**
- ❌ **Zero player feedback**

> **Lesson**: "Perfect infrastructure without gameplay = zero value"

---

## 🚫 The 18 Anti-Patterns

### 1. Architecture-First Development
**Mistake**: Build all systems before validating any gameplay.
**Fix**: **Gameplay first**. Hardcode a playable loop in Week 1. Structure later.

### 2. Infrastructure Before Validation
**Mistake**: 22 complete systems, 0 minutes of fun gameplay.
**Fix**: **MVP scope only**. If it doesn't serve the core loop, defer it.

### 3. Premature DataAsset Architecture
**Mistake**: Built full DataAsset system before knowing what items/markets needed.
**Fix**: **Hardcode first**. Use literal values in Blueprints. Convert to DataAssets after loop is fun.

### 4. Over-Engineering for Future Features
**Mistake**: Combat, quests, factions, multiplayer all "ready" but disabled.
**Fix**: **Delete or disable**. If not in MVP, it's waste. Code rots if unused.

### 5. Perfect Code, Zero Content
**Mistake**: 33K lines C++, 0 DataAssets created for months.
**Fix**: **Content drives code**. Create 1 DataAsset, wire 1 Blueprint, test in PIE. Repeat.

### 6. Documentation Before Implementation
**Mistake**: 74K words of docs for systems nobody has used.
**Fix**: **Document after**. Code first, docs second. Docs for unused systems are fiction.

### 7. No External Playtesting Until Month 3
**Mistake**: Only internal "testing" — developers playing their own game.
**Fix**: **Week 4 playtest**. 5-10 strangers. No instructions. Watch silently.

### 8. Scope Creep via "Just One More System"
**Mistake**: "Let's add crew management, it's almost done!"
**Fix**: **Strict scope enforcement**. Post-MVP list is a graveyard, not a backlog.

### 9. Building for Scale Before Traction
**Mistake**: Multiplayer foundation, cloud saves, modding API — zero players.
**Fix**: **Single-player first**. Scale when you have retention data.

### 10. Automated Testing Instead of Human Testing
**Mistake**: 4 Python test suites passing, but game not fun.
**Fix**: **Human fun > automated passes**. Tests prevent regressions, not bad design.

### 11. Endless Development Cycles Without Shipping
**Mistake**: Hourly commits, daily docs, weekly summaries — no playable build.
**Fix**: **Ship something playable every week**. Even if ugly.

### 12. Data-Driven Everything (Including Things That Shouldn't Be)
**Mistake**: DataAssets for UI colors, tutorial text, particle parameters.
**Fix**: **Data-drive content, not polish**. Hardcode tweakables; expose only what designers need.

### 13. Modular Everything (Including Things That Never Vary)
**Mistake**: Station modules for systems that only ever have one configuration.
**Fix**: **Modularize variation points only**. If every station has a DockingBay, make it a component, not a module.

### 14. Blueprint-Only Logic for Complex Systems
**Mistake**: Economy simulation attempted in Blueprint graphs.
**Fix**: **C++ for math, Blueprint for wiring**. Supply/demand formulas in C++, UI in Blueprint.

### 15. Ignoring the "Boring" Work
**Mistake**: Skipped DataAsset creation, Blueprint wiring, level building — the "grind."
**Fix**: **The grind IS the game**. Schedule it. Automate it (MCP). Do it first.

### 16. No Clear Definition of Done
**Mistake**: "System complete" meant "C++ compiles" not "works in PIE."
**Fix**: **Definition of Done = works in PIE**. Compiles = not done.

### 17. Delaying the "Fun" Question
**Mistake**: Assumed trading would be fun because spreadsheets said so.
**Fix**: **Ask players Week 1**. "Is this fun?" If no, pivot immediately.

### 18. Treating MVP as "Less Features" Instead of "One Complete Loop"
**Mistake**: MVP = 22 systems at 50% each.
**Fix**: **MVP = 1 system at 100%**. Trading loop: buy → fly → sell → upgrade → repeat.

---

## ✅ New Principles (Post-Pivot)

| Old Way | New Way |
|---------|---------|
| Architecture first | Gameplay first |
| Document everything | Document after |
| Build for scale | Build for validation |
| Perfect code | Working PIE |
| Internal testing | External playtesters |
| All systems | One loop |
| Deferred decisions | Decide now |
| Complex modularity | Simple hardcoded |

---

## 🎯 Daily Reminders

> **Before starting any task, ask:**
> 1. Does this make the trading loop more fun?
> 2. Can I validate this in PIE today?
> 3. Is this in MVP scope? (If no → graveyard)
> 4. Am I documenting or building? (Build first)

> **If stuck for 30 min**: Stop. Ask for help. Hardcode it. Move on.

---

## 📋 Graveyard (Explicitly Deferred)

These systems exist in C++ but are **not worked on until MVP validates**:

| System | LOC | Status |
|--------|-----|--------|
| Combat | 5,700+ | Complete, deleted from build |
| Navigation/Autopilot | 865 | Complete, deleted |
| Quest System | 1,327 | Complete, deleted |
| Faction Diplomacy | 2,000+ | Complete, deleted |
| Personnel/Crew | 3,000+ | Complete, deleted |
| Advanced AI | 4,000+ | Complete, deleted |
| Exploration/Scanning | 3,500+ | Partial, deleted |
| Way Network | 2,800+ | Complete, deleted |
| Rivals/Antagonists | 2,200+ | Complete, deleted |
| Battle Pets | 1,800+ | Complete, deleted |
| Building/Crafting | 3,000+ | Complete, deleted |
| Tutorial System | 1,200+ | Complete, deleted |
| Audio System | 2,000+ | In Archive, deleted |
| Multiplayer | 0 | Planned only |

**Total Deferred**: ~35K lines (50% of codebase)

---

## 📖 Reference

- **Critical Review**: `docs/14-ARCHIVE/archive_old/critical-review-2025/`
- **MVP Spec**: `.github/instructions/trade-simulator-mvp.instructions.md`
- **Action Plan**: `docs/14-ARCHIVE/archive_old/critical-review-2025/CRITICAL_REVIEW_ACTION_PLAN.md`

---

*Back to [INDEX.md](INDEX.md)*