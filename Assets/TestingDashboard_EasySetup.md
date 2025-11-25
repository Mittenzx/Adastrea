# Testing Dashboard - Easy Setup Guide

**⏱️ ~10 minutes** | Press **F12** to toggle dashboard during gameplay

## What You're Building

```
┌─────────────────────────────────────────┐
│  🎮 TESTING DASHBOARD        [X] Close │
├─────────────────────────────────────────┤
│  Progress: ████████░░░░  63%           │
├─────────────────────────────────────────┤
│  ☑ Ship Movement                        │
│  ☑ Ship Collision                       │
│  ☐ Weapon Firing      ← click to check │
│  ☐ Targeting System                     │
└─────────────────────────────────────────┘
```

---

## Step 1: Create Widget Blueprint

1. **Content Browser** → `Content/` folder (or `Content/UI/` if you have it)
2. Right-click → **User Interface** → **Widget Blueprint**
3. Name it: `WBP_TestingDashboard`

## Step 2: Set Parent Class

1. Double-click `WBP_TestingDashboard` to open
2. **File** → **Reparent Blueprint**
3. Search `TestingDashboardWidget` → Click **Reparent**

## Step 3: Design Layout

In **Designer** tab, build this:

```
Canvas Panel
└── Vertical Box
    ├── Text Block      → "Testing Dashboard"
    ├── Text Block      → "Progress: 0%" (name: ProgressText)
    ├── Scroll Box      → (name: FeatureScrollBox)
    └── Button          → "Close"
```

1. Drag **Vertical Box** onto Canvas
2. Add **Text Block** for title
3. Add **Text Block** named `ProgressText`
4. Add **Scroll Box** named `FeatureScrollBox`
5. Add **Button** → text "Close"

## Step 4: Close Button Logic

1. Click **Graph** tab
2. Select Close button in Hierarchy
3. **Details** panel → **On Clicked** → Click **+** to add event
4. From the red exec pin, drag and search: `Toggle Dashboard`
5. Set **Visible** input pin to `false` (unchecked)

```
[On Clicked] ──→ [Toggle Dashboard]
                       └── Visible: ☐ (unchecked = false)
```

## Step 5: F12 Key Binding

**In Project Settings:**
1. **Input** → **Action Mappings** → Add `ToggleDashboard` → Key: **F12**

**In Player Controller Blueprint:**

```
[Begin Play] ──→ [Create Widget: WBP_TestingDashboard]
                              ↓
                 [Add to Viewport] ──→ Promote Return Value to variable: DashboardWidget
```

For toggle (show/hide):
```
[Input Action: ToggleDashboard]
         ↓
[Get DashboardWidget variable]
         ↓
[Is Valid?] ──→ [Get bIsDashboardVisible] ──→ [NOT Boolean]
                                                     ↓
                                              [Toggle Dashboard]
```
*This reads the `bIsDashboardVisible` property, flips it, and passes to Toggle Dashboard.*

## Step 6: Test!

1. Click **Play**
2. Press **F12** → Dashboard appears
3. Press **F12** → Dashboard hides ✓

---

## Show Feature Checkboxes

In **Event Graph** of your widget:

```
[Event Construct] ──→ [Initialize Dashboard]
                              ↓
                      [Get Testable Features] ──→ For Each ──→ Add to ScrollBox
```

Each feature has: `FeatureName`, `bIsTested`, `Category`
Use **Set Feature Tested(index, bool)** to check items off.

## Update Progress Text

```
[Get Testing Progress] ──→ Format "Progress: {0}%" ──→ Set ProgressText
```

---

## Quick Troubleshooting

| Problem | Solution |
|---------|----------|
| F12 does nothing | Add Input Action in Project Settings → Input |
| Widget not showing | Check Add to Viewport is called |
| Class not found | Build project first (Ctrl+Shift+B) |
| Progress = 0% | Call Initialize Dashboard on Event Construct |

---

## Key Functions

| Function | Purpose |
|----------|---------|
| `Toggle Dashboard(bool)` | Show/hide |
| `Initialize Dashboard()` | Setup (call once) |
| `Get Testable Features()` | Returns array of default features |
| `Set Feature Tested(index, bool)` | Check/uncheck item |
| `Get Testing Progress()` | Returns 0-100% |
| `bIsDashboardVisible` | Property: current visibility state |

---

**Full docs:** `Source/Adastrea/UI/TestingDashboardWidget_Usage.md` | `Assets/TestingDashboard_QuickStart.md`
