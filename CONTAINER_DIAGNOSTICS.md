# Epic Games Container Access Diagnostics

This guide provides tools and procedures to diagnose why Unreal Engine container pulls fail in GitHub Actions or locally.

## Quick Diagnosis

### Symptom: "Error response from daemon: denied"

**What it means:** GitHub authentication works, but you don't have Epic Games organization access.

**Solution:** Complete the Epic Games setup steps below.

### Symptom: "unauthorized: authentication required"

**What it means:** Docker login failed or your token is invalid.

**Solution:** Check your `GITHUB_TOKEN` and verify Docker login succeeded.

### Symptom: "manifest unknown: manifest unknown"

**What it means:** The container doesn't exist or you can't access it.

**Solution:** Verify the container tag and your Epic Games access.

## Diagnostic Tools

### 1. Test Epic Connection Script (Local)

**Best for:** Local development testing

```bash
# Run the comprehensive connection test
./test_epic_connection.sh
```

This script tests:
- Docker installation and daemon
- GitHub Container Registry connectivity
- Epic Games services connectivity
- GitHub authentication
- Docker login to GHCR
- Epic container registry access
- Container pull capabilities
- Docker credentials storage
- Network diagnostics

**Usage:**

```bash
# Set environment variables (required for full testing)
export GITHUB_TOKEN="your_github_token_here"
export GITHUB_USERNAME="your_github_username"

# Run the test
./test_epic_connection.sh
```

**Output:** The script provides color-coded results and specific recommendations for each failure.

### 2. Docker Setup Check Script (Local)

**Best for:** Quick validation before running workflows

```bash
# Run the existing Docker setup validator
./check_docker_setup.sh
```

This is a lighter-weight version that focuses on the essential setup steps.

**Usage:**

```bash
export GITHUB_TOKEN="your_token"
export GITHUB_USERNAME="your_username"
./check_docker_setup.sh
```

### 3. Diagnose Container Access Workflow (GitHub Actions)

**Best for:** Testing in the GitHub Actions environment

**How to run:**

1. Go to your repository on GitHub
2. Click "Actions" tab
3. Select "Diagnose Container Access" workflow
4. Click "Run workflow"
5. Select branch (usually `main`)
6. Click "Run workflow"

**What it tests:**

- GitHub Container Registry connectivity
- GITHUB_TOKEN authentication
- Docker login to GHCR
- Epic Games services connectivity
- Epic container registry access
- Container manifest inspection
- Full container pull (if manifest access succeeds)

**Interpreting results:**

- ✓ Green checks = Test passed
- ✗ Red X = Test failed (see output for details)
- ⚠ Yellow warning = Non-critical issue

## Epic Games Setup Requirements

All three of these steps are **required** to pull Epic Games containers:

### Step 1: Link Epic Games Account to GitHub

1. Visit: [https://www.epicgames.com/account/connections](https://www.epicgames.com/account/connections)
2. Find "GitHub" in the connections list
3. Click "Connect" or "Link Account"
4. Authorize the connection
5. Verify connection shows as "Connected"

**Verification:** You should see "GitHub: Connected" on your Epic account page.

### Step 2: Join Epic Games GitHub Organization

After linking your accounts, you should receive an invitation to join the EpicGames organization.

**Find your invitation:**

1. Check GitHub notifications: [https://github.com/notifications](https://github.com/notifications)
2. Check your email for "Invitation to join EpicGames"
3. Visit: [https://github.com/EpicGames](https://github.com/EpicGames)
4. Look for invitation banner

**Accept invitation:**

1. Click "Join EpicGames" button
2. Confirm membership

**Verification:** Visit [https://github.com/orgs/EpicGames/people](https://github.com/orgs/EpicGames/people) and search for your username (if membership is public).

### Step 3: Accept Epic Container EULA

1. Visit: [https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-container-images](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-container-images)
2. Read the End User License Agreement
3. Accept the terms

### Step 4: Wait for Permissions

**Important:** After completing steps 1-3, permissions can take time to propagate.

- **Typical wait:** 30 minutes to 4 hours
- **Maximum wait:** 24-48 hours
- **If longer:** Contact Epic Games support

**During the wait:**

- Docker login will succeed
- Container pull will fail with "denied" error
- This is normal and expected

## Troubleshooting by Error Message

### "Login Succeeded" but "Error response from daemon: denied"

**Diagnosis:** GitHub authentication works, Epic permissions not granted.

**Solutions:**

1. Verify Epic account linked (Step 1)
2. Check for organization invitation (Step 2)
3. Accept EULA (Step 3)
4. Wait for permissions to propagate (up to 48 hours)

**Test command:**

```bash
# This should succeed
docker login ghcr.io -u YOUR_USERNAME

# This will fail with "denied" if Epic access not granted
docker pull ghcr.io/epicgames/unreal-engine:dev-slim-5.6
```

### "unauthorized: authentication required"

**Diagnosis:** Docker login failed or token is invalid.

**Solutions:**

1. Verify GITHUB_TOKEN has `read:packages` scope
2. Check token hasn't expired
3. Verify GitHub username is correct
4. Try logging in again

**Test command:**

```bash
# Create token at: https://github.com/settings/tokens
# Required scope: read:packages

export GITHUB_TOKEN="ghp_xxxxxxxxxxxx"
echo $GITHUB_TOKEN | docker login ghcr.io -u YOUR_USERNAME --password-stdin
```

### "manifest unknown: manifest unknown"

**Diagnosis:** Container doesn't exist or you can't access it.

**Solutions:**

1. Verify container tag is correct
2. Check available tags: [https://github.com/orgs/EpicGames/packages](https://github.com/orgs/EpicGames/packages)
3. Ensure Epic Games access is granted

**Common tags:**

- `dev-slim-5.6` - UE 5.6 development build
- `dev-5.6` - Full UE 5.6 development build
- `runtime-5.6` - UE 5.6 runtime only

### Network/Timeout Errors

**Diagnosis:** Network connectivity issues.

**Solutions:**

1. Check internet connection
2. Verify firewall allows Docker registry access
3. If behind corporate proxy, configure Docker proxy settings
4. Try again later (Epic services might be down)

**Test commands:**

```bash
# Test DNS resolution
nslookup ghcr.io

# Test HTTP connectivity
curl -I https://ghcr.io

# Test Epic Games site
curl -I https://www.epicgames.com
```

## Common Scenarios

### Scenario 1: First Time Setup

**Goal:** Set up Epic Games container access for the first time.

**Steps:**

1. Complete Steps 1-3 above
2. Wait 2-4 hours
3. Run `./test_epic_connection.sh` locally
4. Or trigger "Diagnose Container Access" workflow
5. If tests pass, you're ready to build

### Scenario 2: Workflow Failing

**Goal:** Diagnose why GitHub Actions workflow can't pull containers.

**Steps:**

1. Check workflow logs for specific error message
2. Trigger "Diagnose Container Access" workflow
3. Review diagnostic output
4. Follow recommendations from failed tests
5. If Epic setup is complete, wait for permissions

### Scenario 3: Local Development

**Goal:** Pull Unreal Engine container on your development machine.

**Steps:**

1. Create Personal Access Token: [https://github.com/settings/tokens](https://github.com/settings/tokens)
   - Required scope: `read:packages`
2. Set environment variables:
   ```bash
   export GITHUB_TOKEN="your_token"
   export GITHUB_USERNAME="your_username"
   ```
3. Run `./test_epic_connection.sh`
4. Follow any recommendations
5. Once tests pass, use Docker normally

## Reference Commands

### Quick Test Commands

```bash
# Test 1: Check Docker is working
docker version
docker info

# Test 2: Check GitHub Container Registry connectivity
curl -I https://ghcr.io

# Test 3: Test Docker login
echo $GITHUB_TOKEN | docker login ghcr.io -u $GITHUB_USERNAME --password-stdin

# Test 4: Inspect container manifest (doesn't download)
docker manifest inspect ghcr.io/epicgames/unreal-engine:dev-slim-5.6

# Test 5: Pull container (downloads several GB)
docker pull ghcr.io/epicgames/unreal-engine:dev-slim-5.6
```

### Cleanup Commands

```bash
# Remove pulled container
docker rmi ghcr.io/epicgames/unreal-engine:dev-slim-5.6

# Clear Docker credentials
rm ~/.docker/config.json

# Logout from registry
docker logout ghcr.io
```

## Getting Help

If you've completed all setup steps and diagnostics still fail:

1. **Wait 24-48 hours** for permissions to propagate
2. **Check Epic Games status page** for service outages
3. **Review Epic Games forums:** [https://forums.unrealengine.com/](https://forums.unrealengine.com/)
4. **Contact Epic Games support:** [https://www.epicgames.com/help/](https://www.epicgames.com/help/)
5. **Open GitHub issue** in this repository with diagnostic output

## Additional Resources

- [GITHUB_TOKEN_DOCKER_SETUP.md](./GITHUB_TOKEN_DOCKER_SETUP.md) - Comprehensive setup guide
- [CLOUD_BUILD_QUICK_START.md](./CLOUD_BUILD_QUICK_START.md) - Quick start for cloud builds
- [.github/workflows/README.md](./.github/workflows/README.md) - Workflow documentation
- [Epic UE Container Images](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-container-images) - Official documentation

## FAQ

### Q: Do I need Epic Games organization access for public containers?

**A:** Yes, even though the containers are in a public registry, access is restricted to Epic Games organization members only.

### Q: Can I use this without linking my Epic account?

**A:** No, Epic Games organization membership is required to access Unreal Engine containers.

### Q: How long do I need to wait after linking accounts?

**A:** Usually 2-4 hours, but can take up to 48 hours in some cases.

### Q: Will this work with a free GitHub account?

**A:** Yes, GitHub Actions minutes are free for public repositories, and GITHUB_TOKEN is automatically provided.

### Q: Can I test this without downloading the full container?

**A:** Yes, use `docker manifest inspect` to test access without downloading. The diagnostic scripts do this automatically.

### Q: What if I'm behind a corporate firewall?

**A:** You may need to configure Docker to use your corporate proxy. See Docker documentation for proxy configuration.

### Q: Can I use a different version of Unreal Engine?

**A:** Yes, change the tag in your workflow. Available tags: [https://github.com/orgs/EpicGames/packages/container/package/unreal-engine](https://github.com/orgs/EpicGames/packages/container/package/unreal-engine)

---

**Last Updated:** 2025-11-23  
**Version:** 1.0  
**Maintained By:** Adastrea Development Team
