# Status Indicators Quick Reference

## Quick Status Check

Open the Dashboard tab and look at the status indicator grid. For a healthy system, you should see:

```
● Python Process          ● IPC Connection
● Python Bridge Ready     ● Backend Health
● Query Processing        ● Document Ingestion
```

**All green = Good to go!**

## Color Meanings

| Color | Meaning |
|-------|---------|
| 🟢 Green | Working correctly |
| 🟡 Yellow | Warning or active operation |
| 🔴 Red | Error or not functional |
| ⚪ Gray | Unknown or checking |

## Common Problems

### Problem: All lights are RED or GRAY
**Cause:** Python backend not running  
**Fix:** 
1. Open Settings (Ctrl+,)
2. Check Python executable path
3. Check backend script path
4. Click Apply
5. Restart Unreal Engine

### Problem: Python Process GREEN but others RED
**Cause:** IPC connection failed  
**Fix:**
1. Click "Reconnect" button
2. If that fails, check port 5555 is not in use
3. Check firewall allows localhost connections

### Problem: Document Ingestion stuck at YELLOW
**Cause:** Ingestion in progress or hung  
**Fix:**
1. Switch to Ingestion tab
2. Click "Stop" button
3. Wait for status to change to GREEN (Ready)

### Problem: Query Processing RED but Backend Health GREEN
**Cause:** Unlikely - indicates inconsistent state  
**Fix:**
1. Click "Refresh Status"
2. Try sending a test query
3. Check System Logs for errors

## Development Notes

### Adding New Status Indicators

To add a new status indicator:

1. Add widget declaration in `SAdastreaDirectorPanel.h`:
```cpp
TSharedPtr<SStatusIndicator> MyNewStatusLight;
```

2. Create widget in `CreateDashboardTab()`:
```cpp
+ SGridPanel::Slot(0, 3)  // Row 3, Column 0
.Padding(5.0f)
[
    SAssignNew(MyNewStatusLight, SStatusIndicator)
    .StatusText(LOCTEXT("MyNewStatus", "My New Feature"))
    .InitialStatus(SStatusIndicator::EStatus::Unknown)
]
```

3. Update status in `UpdateStatusLights()`:
```cpp
if (MyNewStatusLight.IsValid())
{
    if (MyCondition)
        MyNewStatusLight->SetStatus(SStatusIndicator::EStatus::Good, 
            LOCTEXT("MyNewGood", "My New Feature: Operational"));
    else
        MyNewStatusLight->SetStatus(SStatusIndicator::EStatus::Error,
            LOCTEXT("MyNewError", "My New Feature: Error"));
}
```

### Status Update Frequency

- Status lights: 0.5 seconds (defined by `StatusLightsUpdateInterval`)
- Dashboard logs: 2.0 seconds (defined by `DashboardRefreshInterval`)
- Connection status: 0.5 seconds (defined by `ConnectionStatusUpdateInterval`)

To change update frequency, modify the constants in `SAdastreaDirectorPanel.h`.

### Testing Status Indicators

Run the Python integration test to simulate status checks:

```bash
# Terminal 1: Start IPC server
cd Plugins/AdastreaDirector/Python
python3 ipc_server.py

# Terminal 2: Run test
python3 test_ui_integration.py
```

The test will show what status each indicator should display.

## Troubleshooting Workflow

```
1. Open Dashboard tab
   │
   ├─→ All GREEN? ──→ System healthy ✓
   │
   ├─→ Some RED/GRAY? ──→ Continue below
   │
2. Check Python Process
   │
   ├─→ RED? ──→ Check Settings → Python path
   │
   └─→ GREEN? ──→ Continue below
   │
3. Check IPC Connection
   │
   ├─→ RED? ──→ Click Reconnect
   │
   └─→ GREEN? ──→ Continue below
   │
4. Check Bridge Ready
   │
   ├─→ RED? ──→ Check System Logs
   │
   └─→ GREEN? ──→ Backend is ready
```

## API Reference

### SStatusIndicator

**Status Types:**
- `EStatus::Good` - Component working correctly (green)
- `EStatus::Warning` - Has warnings but functional (yellow)
- `EStatus::Error` - Has errors or not functional (red)
- `EStatus::Unknown` - Status unknown or checking (gray)

**Methods:**
```cpp
// Set status only
void SetStatus(EStatus NewStatus);

// Set status and update text
void SetStatus(EStatus NewStatus, const FText& NewText);

// Get current status
EStatus GetStatus() const;
```

**Example Usage:**
```cpp
// Update just the status
MyStatusLight->SetStatus(SStatusIndicator::EStatus::Good);

// Update status and text
MyStatusLight->SetStatus(
    SStatusIndicator::EStatus::Warning,
    LOCTEXT("Processing", "Processing: 42% complete")
);
```

## Files Modified/Created

- `SStatusIndicator.h/cpp` - New status indicator widget
- `SAdastreaDirectorPanel.h/cpp` - Enhanced dashboard with status lights
- `STATUS_INDICATORS.md` - Complete documentation
- `STATUS_INDICATORS_MOCKUP.txt` - Visual mockup
- `STATUS_INDICATORS_QUICKREF.md` - This file
- `test_ui_integration.py` - Enhanced with status testing

## Performance Notes

Status indicators are lightweight:
- No network requests to Python backend for status checks
- Uses existing `PythonBridge` state (already cached)
- Updates throttled to 0.5s intervals
- No impact on query processing performance
