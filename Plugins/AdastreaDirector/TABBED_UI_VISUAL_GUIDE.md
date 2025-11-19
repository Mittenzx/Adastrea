# Tabbed UI Visual Guide

**Visual representation of the new tabbed interface**

---

## UI Layout Overview

```
┌────────────────────────────────────────────────────────────────┐
│  Adastrea Director - AI Assistant              [⚙️ Settings]  │
├────────────────────────────────────────────────────────────────┤
│  ═══════════════════════════════════════════════════════════  │
├────────────────────────────────────────────────────────────────┤
│  ◉ Query        ○ Ingestion                                    │
├────────────────────────────────────────────────────────────────┤
│  ──────────────────────────────────────────────────────────── │
│                                                                 │
│  [Active Tab Content Displayed Here]                           │
│                                                                 │
│                                                                 │
│                                                                 │
│                                                                 │
└────────────────────────────────────────────────────────────────┘
```

---

## Query Tab View (Default)

```
┌────────────────────────────────────────────────────────────────┐
│  Adastrea Director - AI Assistant              [⚙️ Settings]  │
├────────────────────────────────────────────────────────────────┤
│  ═══════════════════════════════════════════════════════════  │
├────────────────────────────────────────────────────────────────┤
│  ◉ Query        ○ Ingestion                                    │
├────────────────────────────────────────────────────────────────┤
│  ──────────────────────────────────────────────────────────── │
│                                                                 │
│  Query:                                                         │
│  ┌──────────────────────────────────────┐                      │
│  │ Enter your query here...             │  [Send Query]        │
│  └──────────────────────────────────────┘  [Clear History]    │
│                                                                 │
│  ─────────────────────────────────────────────────────────────│
│                                                                 │
│  Results:                                                       │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │ Welcome to Adastrea Director!                           │  │
│  │                                                         │  │
│  │ Enter a query above and click 'Send Query' or press    │  │
│  │ Enter to get started.                                   │  │
│  │                                                         │  │
│  │ Example: "What is Unreal Engine?"                       │  │
│  │                                                         │  │
│  │                                                         │  │
│  │                                                         │  │
│  │                                                         │  │
│  └─────────────────────────────────────────────────────────┘  │
│                                                                 │
└────────────────────────────────────────────────────────────────┘
```

### Query Tab Features

**Query Input Section:**
- Single-line text input for entering questions
- "Send Query" button to submit
- "Clear History" button to reset conversation
- Enter key support for quick submission

**Results Display Section:**
- Multi-line scrollable text area
- Read-only display of AI responses
- Auto-wrapping for long text
- Text selection support for copying

---

## Ingestion Tab View

```
┌────────────────────────────────────────────────────────────────┐
│  Adastrea Director - AI Assistant              [⚙️ Settings]  │
├────────────────────────────────────────────────────────────────┤
│  ═══════════════════════════════════════════════════════════  │
├────────────────────────────────────────────────────────────────┤
│  ○ Query        ◉ Ingestion                                    │
├────────────────────────────────────────────────────────────────┤
│  ──────────────────────────────────────────────────────────── │
│                                                                 │
│  Documentation Folder:                                          │
│  ┌────────────────────────────────────────┐                    │
│  │ C:/Projects/MyGame/Docs                │  [Browse...]       │
│  └────────────────────────────────────────┘                    │
│                                                                 │
│  Database Path:                                                 │
│  ┌────────────────────────────────────────┐                    │
│  │ C:/Projects/MyGame/chroma_db           │  [Browse...]       │
│  └────────────────────────────────────────┘                    │
│                                                                 │
│  [Start Ingestion]  [Stop]                                      │
│                                                                 │
│  ─────────────────────────────────────────────────────────────│
│                                                                 │
│  Ingestion in progress...                                       │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │ ████████████████████░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  42%  │  │
│  └─────────────────────────────────────────────────────────┘  │
│                                                                 │
│  Processing: /Docs/Level_Design/lighting_guide.md              │
│  Files processed: 84 of 200                                     │
│                                                                 │
└────────────────────────────────────────────────────────────────┘
```

### Ingestion Tab Features

**Path Configuration:**
- Documentation folder path input with browse button
- Database path input with browse button
- Default paths pre-filled
- Path validation before ingestion

**Control Section:**
- "Start Ingestion" button (enabled when paths valid)
- "Stop" button (enabled during ingestion)
- Clear action buttons

**Progress Monitoring:**
- Real-time progress bar (0-100%)
- Status message (current operation)
- Detail message (current file being processed)
- File count statistics

---

## Tab Interaction Flow

### Switching from Query to Ingestion

```
Before Click:                         After Click:
┌──────────────────────┐             ┌──────────────────────┐
│ ◉ Query  ○ Ingestion │  ──────>    │ ○ Query  ◉ Ingestion │
└──────────────────────┘             └──────────────────────┘
       │                                      │
       v                                      v
   Query Tab                            Ingestion Tab
   Content                              Content
```

### Visual States

**Active Tab (Checked Radio Button):**
```
◉ Query          ← Filled circle, bold text
```

**Inactive Tab (Unchecked Radio Button):**
```
○ Ingestion      ← Empty circle, normal text
```

---

## Typical Workflows

### Workflow 1: Ask a Question

```
1. User opens plugin
   └─> Query tab active by default

2. User types question
   └─> "What is the Actor component system?"

3. User clicks Send Query or presses Enter
   └─> Query sent to Python backend

4. Results display updates
   └─> AI response appears with relevant context
```

### Workflow 2: Ingest Documentation

```
1. User clicks Ingestion tab
   └─> Tab switches to Ingestion view

2. User selects documentation folder
   └─> Click Browse → Select folder

3. User configures database path
   └─> Use default or browse to custom location

4. User clicks Start Ingestion
   └─> Processing begins

5. Progress updates in real-time
   ├─> Progress bar fills
   ├─> Status messages update
   └─> File names displayed

6. Ingestion completes
   └─> Final status: "Ingestion complete!"
```

### Workflow 3: Switch Between Tabs

```
1. User working in Query tab
   └─> Asking questions and getting answers

2. User needs to add more documentation
   └─> Click Ingestion tab

3. Ingestion tab opens
   └─> Previous query tab state preserved

4. User starts ingestion
   └─> Can still switch back to Query tab

5. User switches back to Query
   └─> Query history still intact
```

---

## Design Rationale

### Radio Button Tabs

**Why Radio Buttons?**
- Standard UE pattern for mutually exclusive options
- Clear visual indication of active state
- Familiar to UE developers
- Accessible (keyboard navigation support)

**Advantages:**
- Only one tab active at a time
- No ambiguity about which view is displayed
- Consistent with Unreal Engine's design language

### Two-Tab Layout

**Why Two Tabs?**
- Clean, uncluttered interface
- Clear separation of concerns:
  - Query: Retrieve information
  - Ingestion: Populate knowledge base
- Easy to understand for new users
- Foundation for future tabs

**Future Expansion:**
```
Current:
┌──────────────────────────────────┐
│ ◉ Query  ○ Ingestion             │
└──────────────────────────────────┘

Future Possibilities:
┌────────────────────────────────────────────────────┐
│ ◉ Query  ○ Ingestion  ○ Planning  ○ Agents        │
└────────────────────────────────────────────────────┘
```

---

## Visual Comparison

### Before Enhancement

```
┌────────────────────────────────────┐
│  Adastrea Director                 │
├────────────────────────────────────┤
│  Query:                            │
│  [________________] [Send Query]   │
│                                    │
│  Results:                          │
│  ┌──────────────────────────────┐ │
│  │                              │ │
│  │  (Results displayed here)    │ │
│  │                              │ │
│  └──────────────────────────────┘ │
└────────────────────────────────────┘

❌ No access to ingestion features
❌ Hidden functionality
❌ Limited user experience
```

### After Enhancement

```
┌────────────────────────────────────┐
│  Adastrea Director    [Settings]   │
├────────────────────────────────────┤
│  ◉ Query  ○ Ingestion              │
├────────────────────────────────────┤
│  [Active Tab Content]              │
│                                    │
│  Query Tab:                        │
│  - Ask questions                   │
│  - View responses                  │
│                                    │
│  Ingestion Tab:                    │
│  - Select docs folder              │
│  - Configure database              │
│  - Monitor progress                │
└────────────────────────────────────┘

✅ Full feature access
✅ Professional UI
✅ Intuitive navigation
```

---

## Implementation Details

### Widget Hierarchy

```
SAdastreaDirectorPanel (Root)
│
├─ SVerticalBox (Main Container)
│  │
│  ├─ Header Row
│  │  ├─ Title TextBlock
│  │  └─ Settings Button
│  │
│  ├─ Separator
│  │
│  ├─ Tab Buttons Row
│  │  ├─ Query Radio Button (CheckBox with RadioButton style)
│  │  └─ Ingestion Radio Button (CheckBox with RadioButton style)
│  │
│  ├─ Separator
│  │
│  └─ SWidgetSwitcher (Content Area)
│     ├─ Slot 0: CreateQueryTab()
│     │  └─ [Query UI widgets]
│     │
│     └─ Slot 1: CreateIngestionTab()
│        └─ [Ingestion UI widgets]
```

### State Management

```cpp
// Current tab index (0 = Query, 1 = Ingestion)
int32 CurrentTabIndex;

// Widget switcher reference
TSharedPtr<SWidgetSwitcher> TabContentSwitcher;

// Tab switching logic
FReply OnTabButtonClicked(int32 TabIndex)
{
    CurrentTabIndex = TabIndex;
    // SWidgetSwitcher automatically updates based on WidgetIndex_Lambda
    return FReply::Handled();
}
```

---

## User Feedback Expected

After implementing this UI, users should experience:

✅ **Improved Discoverability**
- Clear that the plugin has multiple features
- Tab names indicate functionality

✅ **Better Organization**
- Query and ingestion workflows separated
- Less cluttered interface

✅ **Enhanced Usability**
- Easy to switch between tasks
- Visual feedback for active tab

✅ **Professional Appearance**
- Matches UE editor style
- Familiar interaction patterns

---

## Testing the UI

When testing in Unreal Engine Editor, verify:

### Visual Tests
- [ ] Tab buttons render correctly
- [ ] Active tab has filled radio button (◉)
- [ ] Inactive tab has empty radio button (○)
- [ ] Separator lines display properly
- [ ] Content area takes full available height

### Interaction Tests
- [ ] Clicking Query tab switches to Query view
- [ ] Clicking Ingestion tab switches to Ingestion view
- [ ] Only one tab can be active at a time
- [ ] Tab switching is instant (no lag)
- [ ] No visual glitches during switch

### Functional Tests
- [ ] Query tab features work (send query, clear history)
- [ ] Ingestion tab features work (browse, start, stop)
- [ ] State preserved when switching tabs
- [ ] Progress bar updates in Ingestion tab
- [ ] Results display updates in Query tab

---

## Conclusion

The tabbed interface provides a clean, professional UI that exposes all plugin features while maintaining excellent usability. The design follows Unreal Engine conventions and provides a solid foundation for future enhancements.

**Ready for testing!** 🚀

