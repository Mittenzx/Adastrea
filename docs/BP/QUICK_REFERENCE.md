# Blueprint Quick Reference

> **Quick lookup guide for essential Adastrea blueprints**

## 🚀 Start Here

New to the project? Follow this order:

1. [README.md](README.md) - Overview and navigation
2. [BP_SpaceGameMode.md](BP_SpaceGameMode.md) - Game initialization
3. [BP_PlayerShip.md](BP_PlayerShip.md) - Player controls
4. [WBP_TradingUI.md](WBP_TradingUI.md) - Trading interface (MVP core)

---

## 📑 All Blueprints

| Blueprint | Priority | MVP Phase | Difficulty |
|-----------|----------|-----------|------------|
| [BP_SpaceGameMode](BP_SpaceGameMode.md) | ⚠️ CRITICAL | Phase 1 | ⭐⭐ Intermediate |
| [BP_PlayerShip](BP_PlayerShip.md) | ⚠️ CRITICAL | Phase 1 | ⭐⭐⭐ Advanced |
| [WBP_TradingUI](WBP_TradingUI.md) | ⚠️ CRITICAL | Phase 1 | ⭐⭐⭐⭐ Advanced |
| [BP_SpaceStation_Main](BP_SpaceStation_Main.md) | 🔶 HIGH | Phase 1 | ⭐⭐⭐ Advanced |
| [BP_Ship_NPC_Trader](BP_Ship_NPC_Trader.md) | 🔷 MEDIUM | Phase 2 | ⭐⭐⭐⭐ Advanced |

---

## 🎯 By Feature

### Game Initialization
- [BP_SpaceGameMode](BP_SpaceGameMode.md) - Main game mode

### Player Systems
- [BP_PlayerShip](BP_PlayerShip.md) - Player spaceship

### Trading System (MVP Core)
- [WBP_TradingUI](WBP_TradingUI.md) - Trading interface ⭐
- [BP_SpaceStation_Main](BP_SpaceStation_Main.md) - Docking and markets

### AI and Economy
- [BP_Ship_NPC_Trader](BP_Ship_NPC_Trader.md) - AI trader ships

---

## 🖼️ SVG Diagrams

All diagrams are in `../reference/images/blueprints/` folder:

| Diagram | Shows | Used In |
|---------|-------|---------|
| `gamemode_setup.svg` | Game initialization flow | BP_SpaceGameMode |
| `player_ship_beginplay.svg` | Ship setup | BP_PlayerShip |
| `trading_ui_open.svg` | Open market flow | WBP_TradingUI |
| `buy_validation_flow.svg` | Purchase validation | WBP_TradingUI |
| `station_module_attachment.svg` | Module attachment | BP_SpaceStation_Main |
| `npc_ai_decision.svg` | AI state machine | BP_Ship_NPC_Trader |

---

## 🎮 Common Tasks

### "I need to make player ship move"
→ [BP_PlayerShip.md](BP_PlayerShip.md) - Input Actions section

### "I need to setup trading"
→ [WBP_TradingUI.md](WBP_TradingUI.md) - Complete MVP guide

### "I need to add a station"
→ [BP_SpaceStation_Main.md](BP_SpaceStation_Main.md) - Station setup

### "I need to spawn AI traders"
→ [BP_Ship_NPC_Trader.md](BP_Ship_NPC_Trader.md) - AI configuration

### "Game won't start"
→ [BP_SpaceGameMode.md](BP_SpaceGameMode.md) - Troubleshooting section

---

## 🔗 Related Documentation

- **Blueprint Standards**: [blueprint-documentation.instructions.md](../../.github/instructions/blueprint-documentation.instructions.md)
- **Blueprint Image Guide**: [BLUEPRINT_IMAGE_GUIDE.md](../reference/BLUEPRINT_IMAGE_GUIDE.md)
- **Requirements List**: [BLUEPRINT_REQUIREMENTS_LIST.md](../reference/BLUEPRINT_REQUIREMENTS_LIST.md)
- **MVP Guide**: [TRADING_MVP_GUIDE.md](../mvp/TRADING_MVP_GUIDE.md)

---

## 🛠️ Tools

Generate new diagrams:
```bash
python3 Tools/generate_bp_guides.py
```

Generate basic node examples:
```bash
python3 Tools/generate_blueprint_images.py
```

---

**Last Updated**: December 30, 2025
