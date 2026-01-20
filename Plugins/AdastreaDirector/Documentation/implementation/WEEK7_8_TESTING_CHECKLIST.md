# Week 7-8 Features Testing Checklist

**Date Created:** November 17, 2025  
**Purpose:** Manual testing checklist for Week 7-8 features in Unreal Engine Editor  
**Status:** Ready for testing

---

## Prerequisites

Before starting testing, ensure:

- [ ] Unreal Engine 5.1+ is installed
- [ ] Project with AdastreaDirector plugin is set up
- [ ] Plugin compiles successfully
- [ ] Python backend is available and configured
- [ ] You have valid API keys for testing (Gemini and/or OpenAI)

---

## Test 1: Settings Dialog - Basic Functionality

### Opening Settings Dialog

**Test 1.1: Open via Button**
- [ ] Open Adastrea Director panel (Window > Developer Tools > Adastrea Director)
- [ ] Verify Settings button (⚙️ Settings) is visible in the header
- [ ] Click the Settings button
- [ ] Settings dialog should open as a modal window

**Test 1.2: Open via Keyboard Shortcut**
- [ ] With Adastrea Director panel focused
- [ ] Press Ctrl+, (Ctrl + Comma)
- [ ] Settings dialog should open

**Expected Results:**
- Settings dialog opens centered on screen
- Dialog is modal (blocks interaction with main window)
- Dialog size is approximately 550x600 pixels
- Title shows "Settings"

---

## Test 2: Settings Dialog - API Keys Section

### LLM Provider Selection

**Test 2.1: Gemini Provider**
- [ ] Open Settings dialog
- [ ] Verify Gemini radio button exists with label "Gemini (Recommended)"
- [ ] Click Gemini radio button
- [ ] Verify it becomes selected (filled circle)
- [ ] Verify OpenAI radio button becomes unselected

**Test 2.2: OpenAI Provider**
- [ ] In Settings dialog
- [ ] Click OpenAI radio button
- [ ] Verify it becomes selected
- [ ] Verify Gemini radio button becomes unselected

### API Key Input

**Test 2.3: Gemini API Key Input**
- [ ] Open Settings dialog
- [ ] Locate "Gemini API Key:" text field
- [ ] Click in the field
- [ ] Type a test API key (e.g., "AIzaSyTestKey123")
- [ ] Verify characters are masked with • (bullet) characters
- [ ] Verify you cannot see the actual key text

**Test 2.4: OpenAI API Key Input**
- [ ] In Settings dialog
- [ ] Locate "OpenAI API Key:" text field
- [ ] Click in the field
- [ ] Type a test API key (e.g., "sk-test123")
- [ ] Verify characters are masked with • characters

### Embedding Provider Selection

**Test 2.5: Embedding Provider Toggle**
- [ ] In Settings dialog
- [ ] Locate "Embedding Provider:" section
- [ ] Verify HuggingFace radio button exists with label "HuggingFace (Free)"
- [ ] Verify OpenAI radio button exists
- [ ] Click HuggingFace, verify it selects
- [ ] Click OpenAI, verify HuggingFace deselects

**Expected Results:**
- All API key inputs are masked (password-style)
- Radio buttons work correctly (only one selected at a time)
- All labels are clearly visible
- Section is well-organized

---

## Test 3: Settings Dialog - Display Settings

### Font Size

**Test 3.1: Font Size Adjustment**
- [ ] Open Settings dialog
- [ ] Locate "Default Font Size:" spinbox
- [ ] Verify current value is displayed (default: 10)
- [ ] Click up arrow, verify value increases
- [ ] Click down arrow, verify value decreases
- [ ] Try setting value to 8 (minimum), verify it accepts
- [ ] Try setting value to 20 (maximum), verify it accepts
- [ ] Try setting value below 8 or above 20, verify it clamps to valid range
- [ ] Verify "pt" unit label is shown next to spinbox

### Checkboxes

**Test 3.2: Auto-save Settings Checkbox**
- [ ] In Settings dialog
- [ ] Locate "Auto-save settings" checkbox
- [ ] Verify it has a checkmark by default
- [ ] Click checkbox, verify checkmark disappears
- [ ] Click again, verify checkmark reappears

**Test 3.3: Show Timestamps Checkbox**
- [ ] In Settings dialog
- [ ] Locate "Show timestamps in conversation" checkbox
- [ ] Verify it has a checkmark by default
- [ ] Click checkbox, verify checkmark disappears
- [ ] Click again, verify checkmark reappears

**Expected Results:**
- Font size spinbox has clear min/max bounds
- Spinbox increments by 1
- Checkboxes toggle on/off correctly
- All controls respond to mouse input

---

## Test 4: Settings Dialog - Save and Cancel

### Save Functionality

**Test 4.1: Save Settings**
- [ ] Open Settings dialog
- [ ] Select Gemini as LLM provider
- [ ] Enter "TestGeminiKey123" in Gemini API Key field
- [ ] Select HuggingFace as Embedding Provider
- [ ] Set Font Size to 12
- [ ] Check "Auto-save settings"
- [ ] Check "Show timestamps in conversation"
- [ ] Click "Save" button
- [ ] Verify dialog closes
- [ ] Check for config file: `<Project>/Saved/AdastreaDirector/config.ini`
- [ ] Verify file exists

**Test 4.2: Verify Persistence**
- [ ] Open Settings dialog again
- [ ] Verify Gemini is still selected
- [ ] Verify Gemini API Key field shows masked characters (not empty)
- [ ] Verify HuggingFace is still selected
- [ ] Verify Font Size is 12
- [ ] Verify checkboxes are still checked

### Cancel Functionality

**Test 4.3: Cancel Without Saving**
- [ ] Open Settings dialog
- [ ] Change LLM provider to OpenAI
- [ ] Change Font Size to 8
- [ ] Uncheck "Auto-save settings"
- [ ] Click "Cancel" button
- [ ] Verify dialog closes
- [ ] Open Settings dialog again
- [ ] Verify previous settings are still active (not the cancelled changes)

**Expected Results:**
- Save button persists settings to disk
- Cancel button discards changes
- Settings persist across dialog open/close
- Config file is created in correct location

---

## Test 5: Keyboard Shortcuts

### Settings Shortcut

**Test 5.1: Ctrl+, Shortcut**
- [ ] With Adastrea Director panel focused
- [ ] Press Ctrl+, (Ctrl + Comma)
- [ ] Verify Settings dialog opens
- [ ] Close dialog
- [ ] Test shortcut again to verify it's repeatable

**Test 5.2: Tooltip Verification**
- [ ] Hover over Settings button
- [ ] Verify tooltip appears
- [ ] Verify tooltip mentions "Ctrl+," shortcut

**Expected Results:**
- Keyboard shortcut works consistently
- Tooltip provides helpful information
- Shortcut is intuitive and documented

---

## Test 6: Conversation History

### Clear History with Confirmation

**Test 6.1: Clear History Dialog**
- [ ] In Adastrea Director panel, send a test query
- [ ] Wait for response
- [ ] Click "Clear History" button
- [ ] Verify confirmation dialog appears
- [ ] Verify dialog title: "Clear Conversation History"
- [ ] Verify dialog message mentions "cannot be undone"
- [ ] Verify dialog has "Yes" and "No" buttons

**Test 6.2: Confirm Clear**
- [ ] Send another test query
- [ ] Click "Clear History"
- [ ] Click "Yes" in confirmation dialog
- [ ] Verify results area shows success message: "✓ Conversation history cleared successfully."

**Test 6.3: Cancel Clear**
- [ ] Send a test query
- [ ] Click "Clear History"
- [ ] Click "No" in confirmation dialog
- [ ] Verify conversation is NOT cleared
- [ ] Verify previous messages are still visible

**Expected Results:**
- Confirmation dialog prevents accidental clearing
- "Yes" clears history and shows success message
- "No" preserves conversation
- Dialog is modal and clearly worded

---

## Test 7: Error Handling

### Empty Query

**Test 7.1: Empty Query Error**
- [ ] Leave query input box empty
- [ ] Click "Send Query" button
- [ ] Verify error message: "Error: Query cannot be empty."
- [ ] Verify no request is sent to backend

### Backend Connection Errors

**Test 7.2: Backend Not Ready**
- [ ] Stop Python backend (if running)
- [ ] Restart Unreal Engine (to ensure backend doesn't auto-start)
- [ ] Open Adastrea Director panel
- [ ] Send a query
- [ ] Verify error message mentions "Python backend is not ready"
- [ ] Verify error message provides troubleshooting guidance

### Settings Errors

**Test 7.3: Missing Config File**
- [ ] Delete config file: `<Project>/Saved/AdastreaDirector/config.ini`
- [ ] Open Settings dialog
- [ ] Verify dialog opens without crashing
- [ ] Verify default values are shown
- [ ] Enter settings and click Save
- [ ] Verify config file is recreated

**Expected Results:**
- All error messages are user-friendly
- No technical jargon or stack traces
- Errors provide actionable guidance
- System handles missing files gracefully

---

## Test 8: Integration Testing

### End-to-End Workflow

**Test 8.1: Complete Workflow**
- [ ] Open Unreal Engine Editor
- [ ] Open Adastrea Director panel
- [ ] Press Ctrl+, to open Settings
- [ ] Configure API keys
- [ ] Select providers
- [ ] Save settings
- [ ] Send a query about Unreal Engine
- [ ] Verify response is received
- [ ] Send follow-up query
- [ ] Verify conversation context is maintained
- [ ] Clear history with confirmation
- [ ] Send new query
- [ ] Verify fresh conversation starts

**Expected Results:**
- Complete workflow works smoothly
- No crashes or hangs
- Settings persist correctly
- Conversation flows naturally

---

## Test 9: Visual and UX

### Layout and Styling

**Test 9.1: Settings Dialog Layout**
- [ ] Open Settings dialog
- [ ] Verify all sections are clearly separated
- [ ] Verify text is readable (good contrast)
- [ ] Verify buttons are properly aligned
- [ ] Verify no text is cut off or overlapping
- [ ] Verify dialog is scrollable if content is too large

**Test 9.2: Panel Layout**
- [ ] Open Adastrea Director panel
- [ ] Verify Settings button doesn't crowd the title
- [ ] Verify all buttons are accessible
- [ ] Verify layout adjusts when panel is resized
- [ ] Verify panel is dockable

**Expected Results:**
- Professional appearance consistent with UE style
- Clear visual hierarchy
- Responsive layout
- No UI glitches

---

## Test 10: Edge Cases and Stress Testing

### Long Input Strings

**Test 10.1: Long API Key**
- [ ] Open Settings
- [ ] Paste a very long string (500+ characters) into API key field
- [ ] Verify field handles it gracefully
- [ ] Save and reload to verify persistence

**Test 10.2: Special Characters**
- [ ] Enter API key with special characters: `!@#$%^&*()_+-={}[]|:;"'<>,.?/~`
- [ ] Save settings
- [ ] Verify settings are saved correctly
- [ ] Reload and verify special characters are preserved

### Rapid Clicking

**Test 10.3: Button Spam**
- [ ] Rapidly click Settings button multiple times
- [ ] Verify only one dialog opens
- [ ] Rapidly click Save button
- [ ] Verify settings are only saved once

**Expected Results:**
- System handles edge cases gracefully
- No crashes with unusual input
- No duplicate dialogs or actions

---

## Test Results Summary

**Date Tested:** _______________  
**Tested By:** _______________  
**UE Version:** _______________  
**Plugin Version:** Week 7-8

### Pass/Fail Summary

| Test Section | Tests Passed | Tests Failed | Notes |
|-------------|--------------|--------------|-------|
| Settings Dialog Basic | __/4 | __/4 | |
| API Keys Section | __/5 | __/5 | |
| Display Settings | __/3 | __/3 | |
| Save and Cancel | __/3 | __/3 | |
| Keyboard Shortcuts | __/2 | __/2 | |
| Conversation History | __/3 | __/3 | |
| Error Handling | __/3 | __/3 | |
| Integration | __/1 | __/1 | |
| Visual and UX | __/2 | __/2 | |
| Edge Cases | __/3 | __/3 | |
| **TOTAL** | __/29 | __/29 | |

### Overall Status

- [ ] **PASS** - All tests passed, ready for production
- [ ] **PASS WITH ISSUES** - Minor issues found, documented below
- [ ] **FAIL** - Critical issues found, needs fixes

### Issues Found

1. _______________________________________________________________
2. _______________________________________________________________
3. _______________________________________________________________
4. _______________________________________________________________
5. _______________________________________________________________

### Recommendations

_______________________________________________________________
_______________________________________________________________
_______________________________________________________________

---

**Testing Complete:** _______________  
**Approved By:** _______________  
**Date:** _______________
