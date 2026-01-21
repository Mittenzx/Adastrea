# Tutorial System Guide

**Version**: 1.0.0  
**Last Updated**: November 15, 2025  
**System Status**: Alpha - Foundation Complete

---

## Table of Contents

- [Overview](#overview)
- [System Components](#system-components)
- [Quick Start](#quick-start)
- [Tutorial Steps](#tutorial-steps)
- [Tutorial Manager](#tutorial-manager)
- [Blueprint Integration](#blueprint-integration)
- [Best Practices](#best-practices)
- [Examples](#examples)

---

## Overview

The Adastrea Tutorial System provides a flexible framework for creating interactive tutorials that guide players through game mechanics. It features:

- **Data-Driven Design**: All tutorials defined through Data Assets
- **6 Step Types**: Message, UI Highlight, Wait for Action, etc.
- **Progress Tracking**: Automatic saving of tutorial completion
- **Blueprint-Friendly**: Complete designer control

### Key Features

1. **Tutorial Step Types**
   - **Message**: Display text instructions
   - **Highlight UI**: Focus attention on UI elements
   - **Wait for Action**: Require player to perform action
   - **Wait for Condition**: Wait for game state change
   - **Play Animation**: Show visual demonstrations
   - **Spawn Actor**: Create objects for practice

2. **Tutorial Management**
   - Progress tracking and saving
   - Step-by-step sequencing
   - Skip functionality
   - Time limits per step
   - Voiceover support

---

## System Components

### Data Assets

#### TutorialStepDataAsset
Defines individual tutorial steps:
- **Step Title**: Short name for step
- **Step Description**: Detailed instructions
- **Step Type**: Message, UI Highlight, Wait for Action, etc.
- **Time Limit**: Optional timeout
- **Skip Options**: Whether step can be skipped
- **Audio/Visual**: Optional voiceover and images

### Subsystems

#### TutorialManagerSubsystem
Manages tutorial execution:
- **Active Tutorial**: Current tutorial ID and steps
- **Progress Tracking**: Current step and time
- **Completion History**: Completed tutorials
- **State Management**: Start, advance, skip tutorials

---

## Quick Start

### 1. Create Tutorial Steps

Create 3 Data Assets for a basic movement tutorial:

**Step 1**: `DA_Tutorial_Movement_Step1`
```
Step Title: "Welcome!"
Step Description: "Welcome to Adastrea! Let's learn the basic controls."
Step Type: Message
Time Limit: 0 (no limit)
Can Skip: true
Pause Game: false
```

**Step 2**: `DA_Tutorial_Movement_Step2`
```
Step Title: "Move Forward"
Step Description: "Press W or Up Arrow to move forward"
Step Type: Wait For Action
Action To Wait For: "MoveForward"
Time Limit: 30
Can Skip: true
```

**Step 3**: `DA_Tutorial_Movement_Step3`
```
Step Title: "Great Job!"
Step Description: "You've learned how to move! Continue exploring."
Step Type: Message
Time Limit: 5
Can Skip: true
```

### 2. Start Tutorial in Blueprint

**Player Controller or Game Mode**:
```blueprint
Event: BeginPlay
├─ Get Subsystem → TutorialManagerSubsystem
├─ Make Array: Tutorial Steps
│  ├─ DA_Tutorial_Movement_Step1
│  ├─ DA_Tutorial_Movement_Step2
│  └─ DA_Tutorial_Movement_Step3
└─ StartTutorial
   ├─ Tutorial ID: "BasicMovement"
   └─ Steps: [Array from above]
```

### 3. Create Tutorial UI Widget

**WBP_TutorialDisplay**:
```blueprint
Event: On Tutorial Started
└─ Show Widget

Event: On Step Changed
├─ Get Current Step
├─ Set Text: Title → Step Title
└─ Set Text: Description → Step Description

Event: On Tutorial Completed
└─ Hide Widget
```

---

## Tutorial Steps

### Step Type 1: Message

Display text to the player.

**Configuration**:
```
Step Type: Message
Step Title: "Engine Controls"
Step Description: "Use Shift to boost your engines for faster travel"
Time Limit: 10 (auto-advance after 10 seconds)
Can Skip: true
```

**Use Case**: Explaining mechanics, tips, story context

### Step Type 2: Highlight UI

Draw attention to specific UI elements.

**Configuration**:
```
Step Type: Highlight UI
Step Title: "Health Display"
Step Description: "Your health is shown in the top-left corner"
UI Element To Highlight: "HealthBar_Widget"
Time Limit: 0
Can Skip: true
```

**Implementation**:
```blueprint
Event: On Step Changed
├─ Branch: Step Type == Highlight UI?
└─ True
   ├─ Get Widget by Name: UI Element To Highlight
   └─ Play Animation: "PulseHighlight"
```

**Use Case**: Teaching UI layout, important indicators

### Step Type 3: Wait for Action

Require player to perform a specific action.

**Configuration**:
```
Step Type: Wait For Action
Step Title: "Fire Your Weapon"
Step Description: "Press Left Mouse Button to fire"
Action To Wait For: "FireWeapon"
Time Limit: 30
Can Skip: false
```

**Implementation**:
```blueprint
Event: On Fire Weapon (in player controller)
├─ Get Subsystem → TutorialManagerSubsystem
├─ Get Current Step
├─ Branch: Waiting for "FireWeapon"?
└─ True
   └─ TutorialManager → CompleteCurrentStep
```

**Use Case**: Teaching controls, ensuring player understands

### Step Type 4: Wait for Condition

Wait for a game state condition.

**Configuration**:
```
Step Type: Wait For Condition
Step Title: "Reach the Waypoint"
Step Description: "Navigate to the marked location"
Time Limit: 60
Can Skip: true
```

**Implementation**:
```blueprint
Event: Player Entered Trigger Volume
├─ Get Subsystem → TutorialManagerSubsystem
├─ Get Current Step
├─ Branch: Step Type == Wait For Condition?
└─ True
   └─ TutorialManager → CompleteCurrentStep
```

**Use Case**: Objective-based learning, navigation

### Step Type 5: Play Animation

Show a visual demonstration.

**Configuration**:
```
Step Type: Play Animation
Step Title: "Docking Procedure"
Step Description: "Watch how to dock at a station"
Time Limit: 15
Can Skip: true
```

**Implementation**:
```blueprint
Event: On Step Changed
├─ Branch: Step Type == Play Animation?
└─ True
   ├─ Spawn Demonstration Actor
   └─ Play Animation Sequence
```

**Use Case**: Complex procedures, visual learning

### Step Type 6: Spawn Actor

Create practice objects.

**Configuration**:
```
Step Type: Spawn Actor
Step Title: "Target Practice"
Step Description: "Destroy the practice targets"
Actor To Spawn: BP_PracticeTarget
Time Limit: 0
Can Skip: false
```

**Implementation**:
```blueprint
Event: On Step Changed
├─ Branch: Step Type == Spawn Actor?
└─ True
   ├─ Get Actor To Spawn
   └─ Spawn Actor at Location
```

**Use Case**: Combat training, skill practice

---

## Tutorial Manager

### Starting Tutorials

```blueprint
Function: Start Basic Tutorial
├─ Get Subsystem → TutorialManagerSubsystem
├─ Check: Is Tutorial Completed("BasicControls")?
│  └─ Return: false → Continue
└─ StartTutorial
   ├─ Tutorial ID: "BasicControls"
   └─ Steps: [Array of tutorial steps]
```

### Checking Completion

```blueprint
Function: Should Show Tutorial
├─ Get Subsystem → TutorialManagerSubsystem
└─ IsTutorialCompleted
   └─ Tutorial ID: "BasicControls"
```

### Manual Step Control

```blueprint
// Advance to next step
Event: Skip Button Pressed
└─ TutorialManager → AdvanceStep

// Go back a step
Event: Previous Button Pressed
└─ TutorialManager → PreviousStep

// Skip entire tutorial
Event: Skip Tutorial Button
└─ TutorialManager → SkipTutorial
```

### Progress Tracking

```blueprint
Function: Get Tutorial Progress
├─ TutorialManager → GetTutorialProgress
└─ Returns: 0-100 percentage

Function: Get Current Step
├─ TutorialManager → GetCurrentStep
└─ Returns: TutorialStepDataAsset
```

---

## Blueprint Integration

### TutorialManagerSubsystem Functions

```cpp
// Start a tutorial
bool StartTutorial(TutorialID, Steps)

// Step navigation
bool AdvanceStep()
bool PreviousStep()
void CompleteCurrentStep()

// Tutorial control
void SkipTutorial()
bool IsTutorialCompleted(TutorialID)

// State queries
UTutorialStepDataAsset* GetCurrentStep()
float GetTutorialProgress() // Returns 0-100
bool HasActiveTutorial()

// Reset
void ResetAllProgress()
```

### Events

Implement these in your UI widget or game mode:

```cpp
Event OnTutorialStarted(TutorialID)
// Fired when tutorial begins

Event OnStepChanged(StepIndex, Step)
// Fired when moving to new step

Event OnTutorialCompleted(TutorialID)
// Fired when tutorial finishes
```

---

## Best Practices

### Tutorial Design

1. **Start Simple**
   - Begin with basic controls
   - Gradually increase complexity
   - One concept per step

2. **Provide Context**
   - Explain *why* mechanics matter
   - Use real gameplay situations
   - Show consequences of actions

3. **Allow Practice**
   - Give time to experiment
   - Use forgiving time limits
   - Provide feedback on actions

4. **Make Skippable**
   - Let experienced players skip
   - Save completion state
   - Don't force repeated tutorials

### Step Configuration

1. **Time Limits**
   - Message steps: 5-10 seconds
   - Action steps: 30-60 seconds
   - Complex tasks: No limit or generous time

2. **Step Count**
   - Basic tutorial: 3-5 steps
   - Advanced tutorial: 5-10 steps
   - Total time: 2-5 minutes max

3. **Text Length**
   - Title: 2-5 words
   - Description: 1-2 sentences
   - Keep it concise and clear

### Implementation

1. **Tutorial Sequence**
   - Basic Controls → Movement → Combat → Advanced
   - Check prerequisites before starting
   - Skip if already completed

2. **UI Integration**
   - Clear, non-intrusive display
   - Highlight important info
   - Pause option for reading

3. **Audio/Visual**
   - Add voiceovers when possible
   - Use animations for demonstrations
   - Provide visual cues

---

## Examples

### Example 1: Basic Flight Tutorial

**Create Steps**:

```
Step 1: DA_Tutorial_Flight_Welcome
- Title: "Flight Training"
- Description: "Learn how to pilot your spacecraft"
- Type: Message
- Time Limit: 5

Step 2: DA_Tutorial_Flight_Forward
- Title: "Forward Movement"
- Description: "Press W to move forward"
- Type: Wait For Action
- Action: "MoveForward"
- Time Limit: 30

Step 3: DA_Tutorial_Flight_Turn
- Title: "Turning"
- Description: "Move your mouse to turn the ship"
- Type: Wait For Action
- Action: "Turn"
- Time Limit: 30

Step 4: DA_Tutorial_Flight_Boost
- Title: "Boost"
- Description: "Hold Shift to boost your engines"
- Type: Wait For Action
- Action: "Boost"
- Time Limit: 30

Step 5: DA_Tutorial_Flight_Complete
- Title: "Flight Training Complete!"
- Description: "You're ready to explore space"
- Type: Message
- Time Limit: 5
```

**Start Tutorial**:
```blueprint
Event: Player Entered Ship (First Time)
├─ Get Subsystem → TutorialManagerSubsystem
├─ Branch: Is Tutorial Completed("FlightBasics")?
└─ False
   ├─ Make Array: [All 5 tutorial steps]
   └─ StartTutorial
      ├─ ID: "FlightBasics"
      └─ Steps: [Array]
```

### Example 2: Combat Tutorial

**Create Spawn Step**:
```
Step 1: DA_Tutorial_Combat_Targets
- Title: "Target Practice"
- Description: "Destroy the practice targets"
- Type: Spawn Actor
- Actor To Spawn: BP_PracticeTarget
- Time Limit: 0
- Can Skip: false
```

**Implementation**:
```blueprint
Event: On Step Changed
├─ Branch: Step Type == Spawn Actor?
└─ True
   ├─ For Loop: 3 times
   │  └─ Spawn Actor: BP_PracticeTarget
   │     └─ Location: Random around player
   └─ Set Variable: Targets To Destroy = 3

Event: Practice Target Destroyed
├─ Decrement: Targets To Destroy
└─ Branch: Targets To Destroy == 0?
   └─ True
      └─ TutorialManager → CompleteCurrentStep
```

### Example 3: UI Tutorial with Highlights

**Create UI Highlight Steps**:
```
Step 1: DA_Tutorial_UI_HUD
- Title: "HUD Overview"
- Description: "This is your Heads-Up Display"
- Type: Message

Step 2: DA_Tutorial_UI_Health
- Title: "Health Bar"
- Description: "Your hull integrity is shown here"
- Type: Highlight UI
- UI Element: "HealthBar_Widget"

Step 3: DA_Tutorial_UI_Shields
- Title: "Shield Status"
- Description: "Your shields protect you from damage"
- Type: Highlight UI
- UI Element: "ShieldBar_Widget"
```

**Widget Implementation**:
```blueprint
Event: On Step Changed (in HUD Widget)
├─ Get Current Step
├─ Branch: Step Type == Highlight UI?
└─ True
   ├─ Get Widget by Name: UI Element To Highlight
   └─ For Each: All Widgets
      └─ Branch: Is Highlighted Widget?
         ├─ True
         │  ├─ Play Animation: "PulseGlow"
         │  └─ Set Opacity: 1.0
         └─ False
            └─ Set Opacity: 0.3 (dim others)
```

---

## Tutorial Sequences

### Recommended Tutorial Order

1. **Basic Movement** (Required)
   - Forward/backward
   - Turning
   - Speed control

2. **Advanced Flight** (Optional)
   - Strafing
   - Rolling
   - Precision control

3. **Combat Basics** (Required before combat)
   - Targeting
   - Firing weapons
   - Shield management

4. **Trading** (Optional)
   - Finding trade station
   - Buying/selling
   - Price checking

5. **Station Management** (Optional)
   - Docking
   - Station UI
   - Module management

### Auto-Start Logic

```blueprint
Event: Game Started (First Time)
├─ TutorialManager → Start Tutorial("BasicMovement")
└─ On Tutorial Completed
   └─ TutorialManager → Start Tutorial("CombatBasics")
```

---

## Troubleshooting

### Tutorial Not Starting

1. Check TutorialManagerSubsystem is initialized
2. Verify tutorial steps are valid Data Assets
3. Check tutorial isn't already completed
4. Ensure bTutorialsEnabled is true

### Steps Not Advancing

1. Verify CompleteCurrentStep is being called
2. Check Action names match exactly
3. Ensure time limits are reasonable
4. Test manual AdvanceStep

### UI Not Updating

1. Implement OnStepChanged event
2. Check Get Current Step returns valid data
3. Verify widget is visible
4. Test with simple Print statements

---

## Future Enhancements

1. **Advanced Interactions**
   - Multi-step actions
   - Branching tutorials
   - Conditional paths

2. **Enhanced UI**
   - 3D arrows pointing to objects
   - Animated tutorials
   - Video playback

3. **Localization**
   - Multi-language support
   - Text-to-speech
   - Subtitle system

4. **Analytics**
   - Track completion rates
   - Identify problem steps
   - Optimize difficulty

---

## See Also

- [EnhancedInputImplementation.md](EnhancedInputImplementation.md) - For input action names
- [HUDSystemGuide.md](HUDSystemGuide.md) - For UI integration
- [AudioSystemGuide.md](AudioSystemGuide.md) - For voiceover integration

---

**Note**: This is an alpha system. Tutorial content and flow will evolve based on playtesting feedback.
