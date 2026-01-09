# MVP Telemetry System - Quick Reference

**Purpose**: Quick lookup for implementing gameplay telemetry for Trade Simulator MVP  
**Target**: Developers adding telemetry to Blueprints  
**Created**: January 8, 2026

---

## 📊 Critical Metrics to Track

### Week 4 Validation Gate (Prototype)
```
✅ Fun rating (1-10)
✅ Time to first trade (seconds)
✅ Trades completed (#)
✅ Session duration (minutes)
✅ Quit events (location + reason)
```

### Week 8 Validation Gate (Structured)
```
✅ All Week 4 metrics
✅ Trade completion rate (%)
✅ Average profit per trade (credits)
✅ Station visit frequency
✅ Ship upgrade timing
✅ Confusion points (text)
```

### Week 12 Validation Gate (Demo)
```
✅ All Week 4/8 metrics
✅ FPS (frames per second)
✅ Load times (seconds)
✅ Crashes (#)
✅ 30-minute completion rate (%)
```

---

## 🎯 Telemetry Component (Blueprint)

### Add to GameMode/GameInstance

```
1. Open BP_GameMode or BP_GameInstance
2. Add Component → UMVPTelemetryComponent
3. Set to "Auto Activate"
```

### Log Trading Events

**When Player Buys Item**:
```
Event: OnBuyButtonClicked
  ↓
Get MVP Telemetry Component
  ↓
Call: LogTradeStarted
  - Station ID: "AgriculturalHub"
  - Item ID: "Water"
  - Quantity: 100
  - Price: 8.0
```

**When Player Sells Item**:
```
Event: OnSellButtonClicked
  ↓
Get MVP Telemetry Component
  ↓
Call: LogTradeCompleted
  - Station ID: "IndustrialStation"
  - Item ID: "Water"
  - Quantity: 100
  - Price: 12.0
  - Profit: 400 credits
```

**When Player Visits Station**:
```
Event: OnDockingComplete
  ↓
Get MVP Telemetry Component
  ↓
Call: LogStationVisit
  - Station ID: "TradeHub"
  - Time: Current Game Time
```

**When Player Quits**:
```
Event: OnApplicationQuit
  ↓
Get MVP Telemetry Component
  ↓
Call: LogQuitEvent
  - Reason: "User quit" (or crash info)
```

**Export Session Data**:
```
Event: EndPlay
  ↓
Get MVP Telemetry Component
  ↓
Call: ExportSessionData
  - Exports to: Saved/Telemetry/session_YYYY-MM-DD_HH-MM-SS.json
```

---

## 📝 Survey Widget (Blueprint)

### Show at End of Session

```
Event: OnSessionEnd
  ↓
Create Widget: WBP_PlaytestSurvey
  ↓
Add to Viewport
  ↓
Set Input Mode: UI Only
  ↓
Show Mouse Cursor
```

### Survey Questions

1. **Fun Rating** (1-10 slider):
   - "How fun was this experience?"

2. **Would Play Again** (Yes/No):
   - "Would you play this again?"

3. **Would Buy** (Yes/No):
   - "Would you pay money for this game?"

4. **Confusion Points** (Text input):
   - "What confused you? (be specific)"

5. **Best Moment** (Text input):
   - "What was the most fun part?"

### Export Survey Data

```
Event: OnSubmitClicked
  ↓
Collect Responses
  ↓
Call: ExportSurveyData
  - Exports to: Saved/Surveys/survey_YYYY-MM-DD_HH-MM-SS.json
```

---

## 📁 Data Export Format

### Telemetry JSON Structure
```json
{
  "session_id": "UUID",
  "start_time": "2026-01-08T10:00:00Z",
  "end_time": "2026-01-08T10:15:23Z",
  "duration_seconds": 923,
  "events": [
    {
      "type": "trade_started",
      "timestamp": "2026-01-08T10:02:15Z",
      "station_id": "AgriculturalHub",
      "item_id": "Water",
      "quantity": 100,
      "price": 8.0
    },
    {
      "type": "trade_completed",
      "timestamp": "2026-01-08T10:07:42Z",
      "station_id": "IndustrialStation",
      "item_id": "Water",
      "quantity": 100,
      "price": 12.0,
      "profit": 400
    }
  ],
  "stats": {
    "trades_completed": 5,
    "total_profit": 2350,
    "stations_visited": 3,
    "quit_reason": "session_complete"
  }
}
```

### Survey JSON Structure
```json
{
  "survey_id": "UUID",
  "session_id": "UUID (link to telemetry)",
  "timestamp": "2026-01-08T10:15:30Z",
  "responses": {
    "fun_rating": 8,
    "would_play_again": true,
    "would_buy": true,
    "confusion_points": "Didn't understand how to dock at first",
    "best_moment": "Making my first big profit!"
  }
}
```

---

## 🔍 Analysis Scripts

### Analyze Single Session
```bash
python analyze_telemetry.py --session Saved/Telemetry/session_2026-01-08_10-00-00.json
```

**Output**:
```
Session Summary:
  Duration: 15 minutes, 23 seconds
  Trades Completed: 5
  Average Profit: 470 credits
  Time per Trade: 3 min, 5 sec ✅ (target: 3-5 min)
  Stations Visited: 3
  Fun Rating: 8/10
  Would Buy: Yes
```

### Analyze Playtest Batch
```bash
python analyze_playtest_batch.py --folder Saved/Telemetry/ --output report.html
```

**Output**: HTML report with:
- Success rate metrics
- Fun rating distribution
- Common confusion points
- Trade route heatmap
- Quit point analysis

---

## ✅ Validation Gates Checklist

### Week 4 Gate (GO/NO-GO Decision)
```
Required Metrics:
  ☐ 60%+ playtesters say "fun" (rating ≥7)
  ☐ Core loop clear (no major confusion)
  ☐ Can complete trade in 3-5 minutes
  ☐ 0 critical bugs

If NOT met: STOP and pivot
If met: Continue to Phase 2
```

### Week 8 Gate (Structured Playtest)
```
Required Metrics:
  ☐ 80%+ complete at least 3 trades
  ☐ 70%+ say "had fun" (rating ≥7)
  ☐ <3 major confusion points
  ☐ 50%+ "would buy" interest

If NOT met: Extend polish phase
If met: Continue to Week 12
```

### Week 12 Gate (Demo Validation)
```
Required Metrics:
  ☐ 90%+ complete full 30 minutes
  ☐ 75%+ say "fun" (rating ≥7)
  ☐ 50%+ "would buy"
  ☐ 60%+ "would play again"
  ☐ 0 crashes
  ☐ <5 known bugs
  ☐ 60 FPS on mid-range PC
  ☐ <30 second load time

If ALL met: Ready for publisher pitch
If NOT met: Additional polish needed
```

---

## 🚀 Quick Start (5 Minutes)

### Step 1: Add Telemetry Component (2 min)
1. Open `BP_GameMode`
2. Add Component → `UMVPTelemetryComponent`
3. Compile and save

### Step 2: Log First Event (2 min)
1. Open trading Blueprint
2. Find `OnBuyClicked` event
3. Add node: `Log Trade Started`
4. Connect station ID, item ID, quantity, price

### Step 3: Export Data (1 min)
1. Open `BP_GameMode` → Event Graph
2. Find `EndPlay` event
3. Add node: `Export Session Data`

**Done!** Now run a test session and check `Saved/Telemetry/` for JSON output.

---

## 📞 Getting Help

**Issues?**
- Check `docs/development/INFO_COLLECTOR_ANALYSIS.md` for full documentation
- Verify `UMVPTelemetryComponent` is in project
- Ensure output directory exists: `Saved/Telemetry/`

**Questions?**
- Review Trade Simulator MVP instructions: `.github/instructions/trade-simulator-mvp.instructions.md`
- Check anti-patterns guide: `.github/instructions/anti-patterns.instructions.md`

---

**Last Updated**: January 8, 2026  
**Next Review**: After Week 4 playtest  
**Status**: Planning/specification phase (implementation needed)
