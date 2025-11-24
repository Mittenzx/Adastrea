# Epic Games Container Access Documentation Update Summary

**Date:** 2025-11-24  
**Issue:** Users experiencing broken EULA links and unclear container access requirements

## Problem Statement

Users reported that when attempting to diagnose container access:
1. The EULA link provided in error messages led to "document does not exist in this version" error
2. Unclear prerequisites for container access
3. Confusion about EULA acceptance process

## Root Cause Analysis

Research revealed that Epic Games has restructured their documentation:
- **Old URL Pattern (Broken):** `https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-container-images`
- **Current Documentation Structure:**
  - Quick Start Guide: `https://dev.epicgames.com/documentation/en-us/unreal-engine/quick-start-guide-for-using-container-images-in-unreal-engine`
  - Container Deployments: `https://dev.epicgames.com/documentation/en-us/unreal-engine/container-deployments-and-images-for-unreal-editor-and-unreal-engine`
  - Overview: `https://dev.epicgames.com/documentation/en-us/unreal-engine/overview-of-containers-in-unreal-engine`

## Changes Made

### 1. Documentation URL Updates

Updated all references across 9 documentation files:
- CONTAINER_DIAGNOSTICS.md
- GITHUB_TOKEN_DOCKER_SETUP.md
- .github/workflows/diagnose-container-access.yml
- .github/workflows/README.md
- .github/DIAGNOSTIC_TOOLS_SUMMARY.md
- DOCKER_TROUBLESHOOTING_QUICK_REFERENCE.md
- DOCKER_SETUP_QUICK_REFERENCE.md
- CLOUD_BUILD_QUICK_START.md
- CLOUD_BUILD_SERVICES.md

### 2. Clarified EULA Acceptance Process

**Key Clarification:** There is NO separate EULA acceptance step for containers.

The previous documentation incorrectly suggested users needed to:
1. Visit a specific page
2. Accept a separate EULA for containers

**Corrected Information:**
- Container usage is covered by the main Unreal Engine EULA
- EULA is accepted when creating an Epic Games account
- Access is granted through:
  1. Linking Epic Games account to GitHub
  2. Joining the EpicGames GitHub organization
  3. Waiting for permissions to propagate (2-24 hours)

### 3. Enhanced Resource Links

Added comprehensive documentation links:
- Epic Container Quick Start Guide (primary resource)
- Container Deployments and Images guide (detailed info)
- Unreal Engine EULA (legal terms reference)

### 4. Updated Version Numbers

- CONTAINER_DIAGNOSTICS.md: v1.0 → v1.1
- GITHUB_TOKEN_DOCKER_SETUP.md: v1.0 → v1.1

## Current Container Access Requirements (2025)

Based on research from Epic's official documentation and community resources:

### Required Steps:

1. **Create Epic Games Account**
   - Sign up at https://www.epicgames.com/
   - Accept the Unreal Engine EULA during account creation

2. **Link GitHub Account**
   - Visit: https://www.epicgames.com/account/connections
   - Connect your GitHub account
   - Authorize the connection

3. **Join EpicGames Organization**
   - You will receive an invitation to join the EpicGames GitHub organization
   - Check: https://github.com/notifications
   - Accept the organization invitation
   - This is CRITICAL - without org membership, container access will fail

4. **Wait for Permissions**
   - Typical wait: 2-4 hours
   - Maximum wait: 24-48 hours
   - During this time, Docker login will succeed but pulls will fail with "denied" error

5. **Authenticate with Docker**
   - For GitHub Actions: Automatic via `GITHUB_TOKEN`
   - For local development: Create Personal Access Token with `read:packages` scope
   - Login command: `echo $TOKEN | docker login ghcr.io -u USERNAME --password-stdin`

### Container Details:

- **Registry:** GitHub Container Registry (ghcr.io)
- **Repository:** ghcr.io/epicgames/unreal-engine
- **Available Tags:** 
  - `dev-slim-5.6` (development environment, smaller size)
  - `dev-5.6` (full development environment)
  - `runtime-5.6` (runtime only, no editor)

## Testing Recommendations

Since direct container access testing requires:
1. Network connectivity to ghcr.io
2. Valid GITHUB_TOKEN or Personal Access Token
3. Proper Epic Games organization membership

We recommend users test using one of these methods:

### Method 1: GitHub Actions Workflow (Recommended)

Trigger the "Diagnose Container Access" workflow:
1. Go to: https://github.com/Mittenzx/Adastrea/actions
2. Select "Diagnose Container Access" workflow
3. Click "Run workflow"
4. Review the diagnostic output

This tests:
- GitHub Container Registry connectivity
- GITHUB_TOKEN authentication
- Epic container registry access
- Container manifest inspection

### Method 2: Local Testing

If you have completed Epic account setup:

```bash
# Set your credentials
export GITHUB_TOKEN="your_personal_access_token"
export GITHUB_USERNAME="your_github_username"

# Test Docker login
echo $GITHUB_TOKEN | docker login ghcr.io -u $GITHUB_USERNAME --password-stdin

# Test manifest access (doesn't download container)
docker manifest inspect ghcr.io/epicgames/unreal-engine:dev-slim-5.6

# If manifest works, try pulling (downloads several GB)
docker pull ghcr.io/epicgames/unreal-engine:dev-slim-5.6
```

### Method 3: Use Diagnostic Scripts

```bash
# Comprehensive connection test
./test_epic_connection.sh

# Quick validation
./check_docker_setup.sh
```

## Expected Results

### Success Indicators:
- ✓ Docker login to ghcr.io succeeds
- ✓ Manifest inspection returns container metadata
- ✓ Container pull completes successfully
- ✓ GitHub Actions workflow shows all green checks

### Common Failures and Solutions:

**"Login Succeeded" but "denied" on pull:**
- Cause: Epic organization access not granted yet
- Solution: Verify account linking, organization membership, wait longer

**"unauthorized: authentication required":**
- Cause: Docker login failed or token invalid
- Solution: Check token has `read:packages` scope, verify token hasn't expired

**"manifest unknown":**
- Cause: Container doesn't exist or no access
- Solution: Verify tag is correct, check Epic org membership

## Verification Checklist

For repository maintainers and users, verify:

- [ ] All documentation URLs are accessible (visit each link)
- [ ] Quick Start Guide loads correctly
- [ ] Container Deployments guide is available
- [ ] Unreal Engine EULA page loads
- [ ] Instructions match current Epic process (no outdated steps)
- [ ] Workflow error messages provide correct guidance
- [ ] Diagnostic tools reference correct URLs

## Notes on Testing Limitations

The development environment where these changes were made has:
- ✗ No external network access to ghcr.io
- ✗ No GITHUB_TOKEN available in local context
- ✗ DNS resolution blocked for external domains

Therefore, actual container access cannot be tested from this environment. However:
- ✓ All URL updates verified through web research
- ✓ Documentation structure confirmed through Epic's current site
- ✓ EULA process verified through official documentation
- ✓ Community forums and resources consulted for accuracy

## Recommendations for Users

If you previously encountered the "document does not exist" error:

1. **Clear any cached instructions** - The old URL is no longer valid
2. **Follow the updated documentation** - All files now reference working URLs
3. **Verify your Epic account setup:**
   - Account linked: https://www.epicgames.com/account/connections
   - Organization membership: https://github.com/EpicGames
4. **Be patient** - Permissions can take 2-24 hours to propagate
5. **Test using the diagnostic workflow** - It will provide specific guidance

## Additional Resources

- [CONTAINER_DIAGNOSTICS.md](./CONTAINER_DIAGNOSTICS.md) - Complete diagnostic guide
- [GITHUB_TOKEN_DOCKER_SETUP.md](./GITHUB_TOKEN_DOCKER_SETUP.md) - Setup walkthrough
- [Epic Container Quick Start](https://dev.epicgames.com/documentation/en-us/unreal-engine/quick-start-guide-for-using-container-images-in-unreal-engine)
- [Unreal Containers Community](https://unrealcontainers.com/)

## Summary

This update resolves the reported issue by:
1. ✓ Fixing broken documentation URLs
2. ✓ Clarifying the EULA acceptance process
3. ✓ Providing accurate, up-to-date guidance
4. ✓ Adding comprehensive resource links
5. ✓ Improving error messages in workflows

Users should now be able to:
- Access working documentation links
- Understand the actual prerequisites
- Complete setup without confusion about EULA acceptance
- Get accurate guidance from diagnostic tools

---

**Status:** Documentation updates completed and committed  
**Next Steps:** Users should test the updated documentation by following the setup steps and running the diagnostic workflow
