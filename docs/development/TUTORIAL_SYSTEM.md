# Tutorial System for Trade Simulator MVP

**Purpose**: Onboard new players with minimal friction, teaching core trading loop in 5 minutes.

**Design Goal**: "Show, don't tell" - Learn by doing, not reading.

---

## Tutorial Philosophy

### Core Principles

**Minimize Text**:
- No walls of text
- Short, actionable instructions
- Visual indicators over explanations

**Learn by Doing**:
- First trade within 2 minutes
- Hands-on experience
- Immediate feedback

**Non-Intrusive**:
- Can skip tutorial
- Doesn't block gameplay
- Contextual hints, not lectures

**Progressive Disclosure**:
- Teach one thing at a time
- Build on previous knowledge
- Unlock features as needed

---

## Tutorial Flow (5-7 Minutes Total)

### Step 1: Welcome & Goal (30 seconds)

**Screen: Simple Welcome**:
```
┌───────────────────────────────────────┐
│                                       │
│      Welcome to Trade Simulator       │
│                                       │
│  Your Goal: Buy Low, Sell High        │
│                                       │
│  You have 1,000 credits               │
│  Can you turn it into 10,000?         │
│                                       │
│  [Start Tutorial] [Skip]              │
│                                       │
└───────────────────────────────────────┘
```

**Blueprint**: `WBP_TutorialWelcome`

---

### Step 2: Flight Basics (60 seconds)

**Objective**: Fly to Agricultural Station

**On-Screen Prompt**:
```
╔══════════════════════════════════════╗
║ FLIGHT CONTROLS                      ║
╠══════════════════════════════════════╣
║ W/A/S/D    : Move                    ║
║ Mouse      : Look Around             ║
║ Shift      : Boost                   ║
╚══════════════════════════════════════╝

→ Fly to the green marker ahead
```

**Visual Indicator**:
- Large green arrow pointing to Agricultural Station
- Distance meter: "750m to station"
- Pulsing green circle around station

**Implementation**:
```cpp
// In BP_TutorialManager
void ShowFlightTutorial()
{
    // Spawn arrow indicator
    AActor* Arrow = SpawnActor(BP_TutorialArrow, PlayerLocation + Forward * 500);
    Arrow->SetTarget(AgriculturalStation);
    
    // Show UI prompt
    WBP_TutorialPrompt->SetText("Fly to the green marker");
    WBP_TutorialPrompt->Show();
}

void OnPlayerNearStation(ASpaceStation* Station)
{
    if (CurrentTutorialStep == EFlightBasics)
    {
        CompleteTutorialStep(EFlightBasics);
        StartTutorialStep(EDocking);
    }
}
```

---

### Step 3: Docking (30 seconds)

**Objective**: Dock at Agricultural Station

**On-Screen Prompt**:
```
══════════════════════════════════════
You're near the station!

Get close (within 200m)
Press [E] to dock

══════════════════════════════════════
```

**Visual Indicator**:
- Docking range circle (200m radius)
- "Press E to Dock" when in range
- Station highlighted

**Auto-Trigger**: When player presses E and docks successfully

---

### Step 4: Market Overview (45 seconds)

**Objective**: Understand market UI

**Trading UI Opens** with tutorial overlay:
```
╔══════════════════════════════════════════════════════╗
║  FARMING COLLECTIVE MARKET           Credits: 1000cr ║
╠══════════════════════════════════════════════════════╣
║                                                      ║
║  ┌─────────────────────────────────────────────┐   ║
║  │ Item: WATER               Stock: 10000 units│◄──┐║
║  │ Buy Price: 3cr/unit      [Buy] [Amount: 10]│   │║
║  └─────────────────────────────────────────────┘   │║
║                                     ▲               │║
║  Water is CHEAP here!               │               │║
║                                     │               │║
║  You'll sell it elsewhere      These are prices    │║
║  for a profit!                                     │║
║                                                This  ║
║                                               is an  ║
║                                               item   ║
╚══════════════════════════════════════════════════════╝

[Next]
```

**Annotations**:
1. "This is an item" → Points to item row
2. "These are prices" → Points to price display
3. "Your credits are here" → Points to credits display

---

### Step 5: First Purchase (60 seconds)

**Objective**: Buy 10 water units

**Guided Instructions**:
```
══════════════════════════════════════
YOUR FIRST TRADE

1. Select "Water" (already selected)
2. Set Amount: 10 units
3. Click [Buy]

Total Cost: 30 credits
══════════════════════════════════════

[Waiting for you to buy...]
```

**On Purchase Success**:
```
✅ Great! You bought 10 water units!

Spent: 30 credits
Remaining: 970 credits
Cargo: 10/10 units (FULL!)

Next: Sell at Industrial Station
for a HUGE profit!

[Continue]
```

**Auto-Advance**: After successful purchase

---

### Step 6: Close Market & Undock (20 seconds)

**Prompt**:
```
══════════════════════════════════════
Close the market (X button)
Then press [E] again to undock

Time to fly to Industrial Station!
══════════════════════════════════════
```

**Auto-Trigger**: When player undocks

---

### Step 7: Navigate to Industrial Station (90 seconds)

**Objective**: Fly to Industrial Station

**On-Screen Prompt**:
```
══════════════════════════════════════
FLY TO INDUSTRIAL STATION

→ Follow the green arrow
   Distance: 4000m

Water Price There: ~87cr/unit
Your Profit: ~570 credits!
══════════════════════════════════════
```

**Visual Indicator**:
- Green arrow pointing to Industrial Station
- Distance meter updating
- Profit potential displayed

---

### Step 8: Dock & Sell (60 seconds)

**At Industrial Station**:

**Prompt 1 (Docking)**:
```
══════════════════════════════════════
Press [E] to dock at Industrial Station
══════════════════════════════════════
```

**Market Opens** with tutorial:
```
══════════════════════════════════════
INDUSTRIAL EXCHANGE

Notice: Water is EXPENSIVE here!
Buy: 87cr (vs 3cr at Agricultural!)

Your Move:
1. Select "Water" in your cargo
2. Set Amount: 10 units (all of it)
3. Click [Sell]

Expected Profit: ~570 credits!
══════════════════════════════════════
```

**On Sell Success**:
```
🎉 CONGRATULATIONS!

You just completed your first trade!

Profit: 570 credits
New Balance: 1570 credits
Return on Investment: 57%!

This is the core of trading:
Buy Low → Sell High → Repeat!

[Finish Tutorial]
```

---

### Step 9: Tutorial Complete (30 seconds)

**Final Screen**:
```
╔══════════════════════════════════════╗
║    TUTORIAL COMPLETE! 🎉              ║
╠══════════════════════════════════════╣
║                                      ║
║  You've learned:                     ║
║  ✅ How to fly and dock              ║
║  ✅ How to buy goods                 ║
║  ✅ How to sell for profit           ║
║                                      ║
║  Next Steps:                         ║
║  • Try other trade routes            ║
║  • Buy higher-value goods            ║
║  • Save up for a better ship!        ║
║                                      ║
║  Ship Upgrades Unlock at:            ║
║  • 50,000cr - Merchant Hauler        ║
║  • 250,000cr - Trade Liner           ║
║                                      ║
║  [Start Trading!]                    ║
║                                      ║
╚══════════════════════════════════════╝
```

**Auto-Save**: Mark tutorial as completed

---

## Tutorial Skip Option

### Skip Tutorial Flow

**On "Skip Tutorial" Clicked**:
```
═══════════════════════════════════════
Skip Tutorial?

You'll start at Agricultural Station
with 1,000 credits and an empty ship.

Market UI and controls will be
available without guidance.

[Skip Tutorial] [Go Back]
═══════════════════════════════════════
```

**If Skipped**:
- Player spawns at Agricultural Station (docked)
- Full 1,000 credits
- No tutorial prompts
- Market UI available immediately
- Optional: Show quick reference card

---

## Contextual Hints (Post-Tutorial)

### Hint System

**Purpose**: Provide help without tutorial

**Triggers**:

**Hint 1: First Ship Upgrade Available**
```
💡 TIP: Ship Upgrade Available!

You've earned 50,000 credits!
You can now upgrade to Merchant Hauler.

Visit any station's shipyard menu.

[Got It]
```

**Hint 2: Cargo Full**
```
💡 TIP: Cargo Full

Your cargo hold is at maximum capacity.

Sell some items or upgrade to a
larger ship for more space.

[Understood]
```

**Hint 3: First Market Event**
```
💡 MARKET ALERT

Market events create profit opportunities!

Droughts, strikes, and other events
cause price spikes.

Watch for notifications!

[Thanks]
```

**Implementation**:
```cpp
// In PlayerTraderComponent
void CheckForHints()
{
    // Check if should show hint
    if (TotalLifetimeEarnings >= 50000 && !HasShownHint("FirstUpgrade"))
    {
        ShowHint("FirstUpgrade", TEXT("Ship Upgrade Available!"), ...);
        MarkHintShown("FirstUpgrade");
    }
}
```

---

## UI Components

### WBP_TutorialPrompt Widget

**Layout**:
```
┌────────────────────────────────┐
│ ┌──────────────────────────┐  │
│ │ Tutorial Step Text       │  │
│ │                          │  │
│ │ Instructions here        │  │
│ │                          │  │
│ └──────────────────────────┘  │
│                                │
│  [Next] [Skip Tutorial]        │
└────────────────────────────────┘
```

**Position**: Top-center of screen  
**Style**: Semi-transparent background, large readable text  
**Animation**: Fade in/out

### WBP_TutorialArrow Actor

**Visual**: Large 3D arrow pointing to target  
**Behavior**: Follows target, pulses, rotates toward player  
**Cleanup**: Auto-destroy when tutorial step completes

---

## Implementation

### BP_TutorialManager (Actor)

**Spawned**: On game start if tutorial not completed

**Properties**:
```cpp
UPROPERTY(EditAnywhere)
bool bTutorialEnabled;

UPROPERTY(EditAnywhere)
ETutorialStep CurrentStep;

UPROPERTY(EditAnywhere)
TArray<FTutorialStepData> TutorialSteps;
```

**Functions**:
```cpp
UFUNCTION(BlueprintCallable)
void StartTutorial();

UFUNCTION(BlueprintCallable)
void SkipTutorial();

UFUNCTION(BlueprintCallable)
void AdvanceToNextStep();

UFUNCTION(BlueprintCallable)
void CompleteTutorial();
```

### Save Integration

**In UAdastreaTradeSimSaveGame**:
```cpp
UPROPERTY(VisibleAnywhere, Category="Tutorial")
bool bHasCompletedTutorial;

UPROPERTY(VisibleAnywhere, Category="Tutorial")
TMap<FName, bool> TutorialStepsCompleted;

UPROPERTY(VisibleAnywhere, Category="Tutorial")
TArray<FName> HintsShown;
```

---

## Testing

### Tutorial Playtest Checklist

**Test 1: Complete Tutorial** (7 min):
- [ ] Start new game → Tutorial starts
- [ ] Follow all 9 steps
- [ ] Complete first trade successfully
- [ ] Tutorial marks as complete
- [ ] Can continue playing freely

**Test 2: Skip Tutorial** (2 min):
- [ ] Start new game → Click "Skip"
- [ ] Confirm skip dialog
- [ ] Spawn at correct location
- [ ] No tutorial prompts appear
- [ ] All features accessible

**Test 3: Tutorial Interruption** (5 min):
- [ ] Start tutorial
- [ ] Complete 3 steps
- [ ] Quit game
- [ ] Load game
- [ ] Tutorial resumes OR offers to restart

**Test 4: Contextual Hints** (10 min):
- [ ] Complete tutorial
- [ ] Play until 50,000cr
- [ ] Ship upgrade hint appears
- [ ] Fill cargo → Cargo full hint
- [ ] First market event → Event hint

---

## Localization (Post-MVP)

**Tutorial Text Keys**:
```
Tutorial.Welcome.Title
Tutorial.Welcome.Goal
Tutorial.Flight.Instructions
Tutorial.Docking.Instructions
Tutorial.Market.Overview
Tutorial.Purchase.Instructions
Tutorial.Sell.Instructions
Tutorial.Complete.Congratulations
```

**Hint Text Keys**:
```
Hint.ShipUpgrade.Title
Hint.ShipUpgrade.Message
Hint.CargoFull.Title
Hint.MarketEvent.Title
```

---

## Accessibility

**Features**:
- Colorblind mode (different arrow colors)
- Large text option (150% scale)
- Audio cues for tutorial steps
- Subtitles for narration (if added post-MVP)
- Skip any step option

---

## Summary

**Tutorial Duration**: 5-7 minutes  
**Steps**: 9 total  
**Goal**: Complete first profitable trade  
**Method**: Learn by doing  
**Result**: Player understands core loop

**Contextual Hints**:
- Ship upgrade available
- Cargo full warning
- Market event explanation
- (Expandable post-MVP)

**Implementation**:
- BP_TutorialManager actor
- WBP_TutorialPrompt widget
- WBP_TutorialArrow actor
- Save integration
- Skip option

---

**Design Goal**: "Anyone can learn to trade in 5 minutes"

---

**Last Updated**: 2026-01-17  
**Version**: 1.0  
**Part of**: Trade Simulator MVP (Week 1-4 Prototype Phase)
