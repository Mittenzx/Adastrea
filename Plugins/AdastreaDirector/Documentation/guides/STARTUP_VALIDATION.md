# Startup Validation System

## Overview

The Adastrea Director plugin implements a comprehensive startup validation system to ensure all prerequisites are met before the plugin becomes fully operational. This prevents the plugin from appearing to work while silently failing.

## Validation Sequence

### 1. Module Initialization (StartupModule)

When the plugin loads, the runtime module (`FAdastreaDirectorModule`) performs the following sequence:

```cpp
StartupModule()
  → InitializePythonBridge()
  → ValidateStartup() [NEW]
    → ValidateSettings()
    → ValidateBackend()
    → ValidateAPIKey()
```

### 2. Validation Checks

#### Check 1: Settings Validation

**What it checks:**
- API key is configured for selected provider
- API key format is valid (basic length check)
- Provider is supported (Gemini or OpenAI)

**Failure condition:**
- No API key configured
- API key too short (< 10 characters)
- Unknown provider selected

**Error handling:**
- Plugin marks initialization as failed
- Error message stored in module
- Editor UI shows error screen instead of main panel

#### Check 2: Backend Validation

**What it checks:**
- Python process is running
- IPC connection is established
- Backend responds to ping requests

**Failure condition:**
- Python process failed to start
- Port 5555 blocked or in use
- IPC socket not connected
- Ping timeout or invalid response

**Error handling:**
- Plugin marks initialization as failed
- Provides detailed status in error message
- Suggests troubleshooting steps

#### Check 3: API Key Validation

**What it checks:**
- API key is valid with the provider
- Provider API is accessible
- Key has not been revoked

**Process:**
1. Send validation request to Python backend
2. Backend attempts API call with the key
3. Returns validation result

**Failure condition:**
- API key rejected by provider (401/403)
- Network error preventing validation
- Provider API unavailable

**Error handling:**
- Plugin marks initialization as failed
- Shows specific error from provider
- Distinguishes between invalid key and network issues

## Validation Components

### FAdastreaSettings

Centralized settings manager that:
- Loads configuration from `config.ini`
- Validates settings format
- Provides API key access
- Ensures thread-safe access

**Location:** `Source/AdastreaDirector/Public/AdastreaSettings.h`

### FAdastreaStartupValidator

Performs startup validation checks:
- `ValidateStartup()` - Complete validation
- `ValidateSettings()` - Settings only
- `ValidateBackend()` - Backend connectivity
- `ValidateAPIKey()` - API key validity

**Location:** `Source/AdastreaDirector/Public/AdastreaStartupValidator.h`

### Python Backend Handler

New IPC handler: `validate_api_key`

**Request format:**
```json
{
  "type": "validate_api_key",
  "data": "{\"provider\":\"gemini\",\"api_key\":\"AIza...\"}"
}
```

**Response format (success):**
```json
{
  "status": "success",
  "valid": true,
  "message": "Gemini API key is valid. Found 15 available models.",
  "provider": "gemini"
}
```

**Response format (invalid):**
```json
{
  "status": "success",
  "valid": false,
  "error": "API key is invalid or has been revoked",
  "provider": "gemini"
}
```

## User Experience

### Before Validation (Old Behavior)

1. Plugin loads silently
2. User opens panel
3. Panel shows "Welcome" message
4. User sends query
5. **Query fails with error** (Bad UX)

### After Validation (New Behavior)

1. Plugin loads and validates
2. If validation fails:
   - Error screen shown instead of main panel
   - Clear error message displayed
   - "Open Settings" button provided
   - User cannot proceed until fixed
3. If validation succeeds:
   - Normal panel opens
   - All features guaranteed to work

### Error Screen

When validation fails, the editor module shows:

```
⚠️ Adastrea Director - Initialization Failed

[Specific error message here]

To resolve this issue:
1. Configure your API key in Settings (if not done)
2. Ensure Python backend is properly installed
3. Check the Output Log for detailed error information
4. Restart Unreal Engine after fixing the issue

[Open Settings Button]
```

## Self-Check Integration

The self-check feature has been enhanced to include validation checks:

### New Checks Added

- **Check 1:** Runtime Module + Startup Status
  - Verifies module loaded
  - Checks if fully initialized
  - Shows initialization errors if any

- **Check 2:** Settings Configuration
  - Validates all settings
  - Shows current provider
  - Confirms API key configured

- **Check 7:** API Key Validation
  - Live validation of API key
  - Tests actual API connectivity
  - Reports provider-specific errors

### Running Self-Check

```
Tests Tab → 🔍 Self-Check Button
```

Output includes 8 comprehensive checks:
1. Runtime Module (with startup status)
2. Settings Configuration (NEW)
3. Python Bridge
4. Python Process
5. IPC Connection
6. Backend Health
7. API Key Validation (NEW)
8. Query Processing

## Configuration

### Settings File Location

```
<ProjectDir>/Saved/AdastreaDirector/config.ini
```

### Example Configuration

**config.ini** (plugin settings):
```ini
# Adastrea Director Configuration
# Auto-generated file
# Note: API keys are NOT stored here - they're in .env file

AutoSaveSettings=true
DefaultFontSize=10
EmbeddingProvider=huggingface
LLMProvider=gemini
ShowTimestamps=true
```

**.env file** (API keys - in project root):
```
GEMINI_KEY=AIzaSyXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
# or alternatively:
GOOGLE_API_KEY=AIzaSyXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
# For OpenAI:
OPENAI_API_KEY=sk-XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
```

### Changing Settings

Settings can be modified:
1. Through the UI (Settings dialog)
2. By editing `config.ini` directly
3. Via the Settings API (programmatically)

**Note:** After changing settings, restart Unreal Engine for validation to re-run.

## Validation Results

### Success

```cpp
FStartupValidationResult result = ValidateStartup(pythonBridge);

if (result.bSuccess) {
    // Plugin fully initialized
    // All features available
}
```

### Failure

```cpp
if (!result.bSuccess) {
    // Initialization failed
    FString error = result.ErrorMessage;
    // Show error to user
    // Prevent plugin usage
}
```

### Detailed Status

Every validation result includes:
- `bSuccess` - Overall result
- `ErrorMessage` - Primary error (if failed)
- `Warnings` - Non-critical issues
- `DetailedStatus` - Full check breakdown

## Best Practices

### For Plugin Users

1. **Configure API key before first use**
   - Go to Settings before opening main panel
   - Get API key from provider
   - Test with self-check

2. **Check self-check after setup**
   - Verify all 8 checks pass
   - Address any warnings
   - Use as troubleshooting tool

3. **Monitor Output Log**
   - Enable verbose logging if needed
   - Check for validation messages
   - Look for initialization errors

### For Developers

1. **Always check IsFullyInitialized()**
   ```cpp
   FAdastreaDirectorModule* Module = GetModule();
   if (Module && Module->IsFullyInitialized()) {
       // Safe to use features
   }
   ```

2. **Handle initialization errors gracefully**
   ```cpp
   if (!Module->IsFullyInitialized()) {
       FString error = Module->GetInitializationError();
       // Show error, disable features, etc.
   }
   ```

3. **Add validation for new features**
   - Extend `FAdastreaStartupValidator`
   - Add checks to self-check
   - Document validation requirements

## Debugging

### Enable Verbose Validation Logging

**Security Note:** Verbose logging is safe - API keys are stored in .env files and validated by the Python backend reading environment variables. They are never transmitted through the plugin's IPC layer, so they will not appear in Unreal Engine logs.

In `DefaultEngine.ini`:
```ini
[Core.Log]
LogAdastreaDirector=Verbose
```

### Validation Log Output

```
LogAdastreaDirector: Starting comprehensive startup validation...
LogAdastreaDirector: Settings validated successfully
LogAdastreaDirector: Backend connectivity verified
LogAdastreaDirector: API key validation requested
LogAdastreaDirector: Startup validation completed successfully
```

Or on failure:
```
LogAdastreaDirector: Starting comprehensive startup validation...
LogAdastreaDirector: Settings validated successfully
LogAdastreaDirector: Backend connectivity verified
LogAdastreaDirector: Error: API key validation failed: API key is invalid
LogAdastreaDirector: Startup validation failed: Gemini API key is invalid...
```

### Manual Validation

Test validation independently:
```cpp
#include "AdastreaStartupValidator.h"

FStartupValidationResult result = FAdastreaStartupValidator::ValidateSettings();
UE_LOG(LogTemp, Log, TEXT("Settings valid: %s"), result.bSuccess ? TEXT("Yes") : TEXT("No"));
if (!result.bSuccess) {
    UE_LOG(LogTemp, Error, TEXT("Error: %s"), *result.ErrorMessage);
}
```

## Security Considerations

### API Key Storage

- Stored in plain text in `.env` file (industry standard)
- `.env` file located in project root directory
- **Must** be added to `.gitignore` (not committed to version control)
- Read by Python backend from environment variables
- Never stored in plugin's config.ini file
- Consider using secret management tools for production deployments

### API Key Transmission

- API keys are **never** sent over IPC
- Python backend reads keys directly from environment variables
- Keys are only transmitted to provider APIs for validation
- Not logged in normal or verbose mode
- Never transmitted to third parties or stored in logs

### Validation Requests

- Only sent during startup and self-check
- Minimal API calls to avoid quota usage
- Quick validation (list models)
- Fail fast on errors

## Future Enhancements

### Planned Improvements

1. **Async Validation**
   - Don't block UI thread during validation
   - Show progress indicator
   - Allow cancellation

2. **Retry Logic**
   - Automatic retry on network errors
   - Exponential backoff
   - User-initiated retry

3. **Validation Caching**
   - Cache successful API key validations
   - Reduce API calls
   - Configurable cache duration

4. **Enhanced Diagnostics**
   - Network diagnostics
   - Provider status check
   - Dependency version checks

5. **Validation Profiles**
   - Different validation levels
   - Quick vs. thorough validation
   - Skip certain checks for testing

## Related Documentation

- [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - Common issues and solutions
- [SETUP_GUIDE.md](SETUP_GUIDE.md) - Initial setup and configuration
- [README.md](README.md) - General plugin information

---

**Version:** 1.0.0  
**Last Updated:** December 2025  
**Requires:** UE 5.6+
