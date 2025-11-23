# Diagnostic Tools Summary

This document provides an overview of all diagnostic tools available for troubleshooting Epic Games Unreal Engine container access issues.

## Quick Navigation

- **Error Code:** "denied" → [Quick Reference](../DOCKER_TROUBLESHOOTING_QUICK_REFERENCE.md)
- **Full Guide:** [CONTAINER_DIAGNOSTICS.md](../CONTAINER_DIAGNOSTICS.md)
- **Setup Guide:** [GITHUB_TOKEN_DOCKER_SETUP.md](../GITHUB_TOKEN_DOCKER_SETUP.md)

## Available Tools

### 1. Local Diagnostic Script

**File:** `test_epic_connection.sh`

**Best for:** Comprehensive local testing

**What it does:**
- 10 automated tests covering all connection points
- Tests Docker, GHCR, Epic services
- Validates authentication and permissions
- Provides specific recommendations

**How to use:**
```bash
# Option 1: Interactive mode (will prompt for credentials)
./test_epic_connection.sh

# Option 2: With environment variables
export GITHUB_TOKEN="your_token"
export GITHUB_USERNAME="your_username"
./test_epic_connection.sh

# Option 3: Non-interactive
echo "n" | ./test_epic_connection.sh
```

**Output:** Color-coded test results with recommendations

---

### 2. Docker Setup Checker

**File:** `check_docker_setup.sh`

**Best for:** Quick validation before running workflows

**What it does:**
- 6 essential checks
- Focuses on core setup requirements
- Lighter weight than test_epic_connection.sh
- Error-specific diagnostics

**How to use:**
```bash
export GITHUB_TOKEN="your_token"
export GITHUB_USERNAME="your_username"
./check_docker_setup.sh
```

**Output:** Step-by-step validation with actionable errors

---

### 3. GitHub Actions Diagnostic Workflow

**File:** `.github/workflows/diagnose-container-access.yml`

**Best for:** Testing in GitHub Actions environment

**What it does:**
- Tests GHCR connectivity
- Validates GITHUB_TOKEN
- Tests Docker login
- Checks Epic services
- Inspects container manifest
- Optional: Full container pull

**How to use:**
1. Go to repository Actions tab
2. Select "Diagnose Container Access"
3. Click "Run workflow"
4. Select branch
5. Review results

**Output:** Detailed workflow logs with diagnostics

---

### 4. Comprehensive Documentation

**File:** `CONTAINER_DIAGNOSTICS.md`

**Best for:** Understanding and troubleshooting issues

**Contents:**
- All error messages explained
- Step-by-step setup instructions
- Diagnostic tool documentation
- Reference commands
- FAQ section
- Getting help resources

---

### 5. Quick Reference Card

**File:** `DOCKER_TROUBLESHOOTING_QUICK_REFERENCE.md`

**Best for:** Quick lookup of common errors

**Contents:**
- Common error messages and solutions
- Quick test commands
- Setup checklist
- Links to full documentation

---

## Choosing the Right Tool

### For First-Time Setup
1. Read [GITHUB_TOKEN_DOCKER_SETUP.md](../GITHUB_TOKEN_DOCKER_SETUP.md)
2. Complete Epic Games setup steps
3. Run `./test_epic_connection.sh` to verify

### For Troubleshooting a Failed Workflow
1. Check workflow logs for error message
2. Trigger "Diagnose Container Access" workflow
3. Follow recommendations from diagnostic output
4. Refer to [CONTAINER_DIAGNOSTICS.md](../CONTAINER_DIAGNOSTICS.md) for details

### For Quick Checks
1. Run `./check_docker_setup.sh` before pushing
2. Or use [DOCKER_TROUBLESHOOTING_QUICK_REFERENCE.md](../DOCKER_TROUBLESHOOTING_QUICK_REFERENCE.md)

### For Understanding Errors
1. Look up error in [DOCKER_TROUBLESHOOTING_QUICK_REFERENCE.md](../DOCKER_TROUBLESHOOTING_QUICK_REFERENCE.md)
2. Read detailed explanation in [CONTAINER_DIAGNOSTICS.md](../CONTAINER_DIAGNOSTICS.md)

## Integration with Workflows

Both main build workflows now reference these diagnostic tools:

### ue-build-example.yml
- Enhanced error messages
- Links to diagnostic workflow
- Points to CONTAINER_DIAGNOSTICS.md
- Provides setup checklist

### Ue_build.yml
- References diagnostic workflow
- Links to documentation
- Simplified error messages

## Common Scenarios

### Scenario 1: "denied" Error

**What it means:** GitHub auth works, Epic permissions not granted

**What to do:**
1. Check [Quick Reference "denied" section](../DOCKER_TROUBLESHOOTING_QUICK_REFERENCE.md#-error-denied-when-pulling-container)
2. Complete Epic Games setup steps
3. Wait 2-24 hours for permissions
4. Re-run diagnostics

### Scenario 2: "unauthorized" Error

**What it means:** Docker login failed

**What to do:**
1. Check token has `read:packages` scope
2. Verify token hasn't expired
3. Re-authenticate with Docker
4. Run `./test_epic_connection.sh` to verify

### Scenario 3: Network Issues

**What it means:** Cannot reach GHCR or Epic services

**What to do:**
1. Check internet connection
2. Verify firewall settings
3. Check corporate proxy settings
4. Run `./test_epic_connection.sh` for detailed diagnosis

## Maintenance

### Updating Diagnostic Tools

When making changes to diagnostic tools:
1. Update all related documentation
2. Test in both local and CI environments
3. Verify error messages are accurate
4. Update version numbers where applicable

### Adding New Diagnostics

To add new diagnostic tests:
1. Add to `test_epic_connection.sh` for local testing
2. Add to `diagnose-container-access.yml` for CI testing
3. Document in `CONTAINER_DIAGNOSTICS.md`
4. Add quick reference to `DOCKER_TROUBLESHOOTING_QUICK_REFERENCE.md`

## Support Resources

### Internal Documentation
- [GITHUB_TOKEN_DOCKER_SETUP.md](../GITHUB_TOKEN_DOCKER_SETUP.md)
- [CONTAINER_DIAGNOSTICS.md](../CONTAINER_DIAGNOSTICS.md)
- [CLOUD_BUILD_QUICK_START.md](../CLOUD_BUILD_QUICK_START.md)
- [CLOUD_BUILD_SERVICES.md](../CLOUD_BUILD_SERVICES.md)

### External Resources
- [Epic Games Account](https://www.epicgames.com/account/connections)
- [Epic Games Organization](https://github.com/EpicGames)
- [Container Images Docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-container-images)
- [Epic Games Support](https://www.epicgames.com/help/)

## Version History

- **v1.0** (2025-11-23): Initial release of diagnostic tools
  - Created test_epic_connection.sh
  - Enhanced check_docker_setup.sh
  - Added diagnose-container-access.yml workflow
  - Created comprehensive documentation

---

**Last Updated:** 2025-11-23  
**Maintained By:** Adastrea Development Team
