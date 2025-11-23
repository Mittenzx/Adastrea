# GitHub Token and Docker Setup for Unreal Engine Builds

Complete guide for setting up GitHub Actions with Docker to build Unreal Engine projects in the Adastrea repository.

## Table of Contents

- [Quick Answer](#quick-answer)
- [Understanding the Setup](#understanding-the-setup)
- [GitHub Token Configuration](#github-token-configuration)
- [Epic Games Container Access](#epic-games-container-access)
- [Complete Setup Checklist](#complete-setup-checklist)
- [Testing Your Setup](#testing-your-setup)
- [Troubleshooting](#troubleshooting)
- [Advanced Configuration](#advanced-configuration)

## Quick Answer

**TL;DR:** The `GITHUB_TOKEN` is automatically provided by GitHub Actions and requires NO manual configuration. The actual setup you need is **Epic Games container access**:

1. Link your Epic Games account to GitHub: <https://www.epicgames.com/account/connections>
2. Accept Epic's organization invitation on GitHub
3. Accept Epic's container EULA
4. Wait for permissions to propagate (may take a few hours)

## Understanding the Setup

### What is GITHUB_TOKEN?

`GITHUB_TOKEN` is an **automatically generated secret** by GitHub Actions. You don't need to create or configure it manually.

**Key facts:**
- ✅ Automatically available in all workflows
- ✅ Has permissions to read repo and packages
- ✅ Expires after the workflow run
- ✅ Used to authenticate with GitHub Container Registry (ghcr.io)

**You DO NOT need to:**
- ❌ Create a Personal Access Token (PAT)
- ❌ Add it to repository secrets
- ❌ Configure it in workflow files

### What is the Epic Games Container?

Epic Games provides official Docker container images for Unreal Engine that:
- Include pre-installed Unreal Engine
- Enable building UE projects in CI/CD
- Are hosted on GitHub Container Registry (ghcr.io)
- **Require special Epic Games authorization to access**

### The Two-Step Authentication Process

```
Step 1: Authenticate with GitHub Container Registry
  → Uses GITHUB_TOKEN (automatic)
  → Allows basic access to ghcr.io
  → ✅ This works out of the box

Step 2: Pull Epic Games Containers
  → Requires Epic Games organization membership
  → Needs Epic EULA acceptance
  → ⚠️ This is what usually fails
```

## GitHub Token Configuration

### For GitHub Actions (Automated Workflows)

**No configuration needed!** The workflow already has the correct setup:

```yaml
# This is already in your workflow files
- name: Login to GitHub Container Registry
  env:
    GH_TOKEN: ${{ secrets.GITHUB_TOKEN }}
  run: |
    echo "$GH_TOKEN" | docker login ghcr.io -u ${{ github.actor }} --password-stdin
```

**What's happening here:**
- `${{ secrets.GITHUB_TOKEN }}` - Automatically provided by GitHub
- `${{ github.actor }}` - Your GitHub username
- `ghcr.io` - GitHub Container Registry

### For Local Development (Manual Testing)

If you want to test Docker builds locally, you need a Personal Access Token:

#### Create a Personal Access Token

1. Go to: <https://github.com/settings/tokens>
2. Click "Generate new token" → "Generate new token (classic)"
3. Give it a name (e.g., "Docker GHCR Access")
4. Select scopes:
   - ✅ `read:packages` - Required to pull containers
   - ✅ `repo` - If you need private repo access
5. Click "Generate token"
6. **Copy the token immediately** (you won't see it again)

#### Use the Token Locally

```bash
# Export your token
export GITHUB_TOKEN="ghp_yourTokenHere"

# Login to GitHub Container Registry
echo $GITHUB_TOKEN | docker login ghcr.io -u YOUR_GITHUB_USERNAME --password-stdin

# Test pulling a public container
docker pull ghcr.io/epicgames/unreal-engine:dev-slim-5.6
```

**Security tip:** Never commit tokens to your repository!

## Epic Games Container Access

This is the critical part that most users are missing.

### Prerequisites

1. **GitHub Account** (you already have this)
2. **Epic Games Account** (create at <https://www.epicgames.com/>)
3. **Linked Accounts** (connect them together)

### Step-by-Step Setup

#### Step 1: Link Epic Games Account to GitHub (5 minutes)

1. Go to: <https://www.epicgames.com/account/connections>
2. Find "GitHub" in the connections list
3. Click "Connect" or "Link Account"
4. Authorize the connection
5. ✅ Verify the connection shows as "Connected"

#### Step 2: Accept Epic Games Organization Invitation (Critical!)

After linking, you should receive an invitation to join the Epic Games GitHub organization.

**Check for invitation:**
1. Go to: <https://github.com/EpicGames>
2. Look for a banner saying "You've been invited to join EpicGames"
3. Click "Join EpicGames"

**Alternative check:**
- Check your GitHub notifications
- Check your email for "Invitation to join EpicGames"

**Verify membership:**
- Visit: <https://github.com/orgs/EpicGames/people>
- Search for your username (if organization membership is public)

⚠️ **Important:** Without organization membership, you cannot access Epic's containers!

#### Step 3: Accept Container Images EULA (5 minutes)

1. Go to: <https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-container-images>
2. Read the End User License Agreement
3. Accept the terms
4. ✅ Confirm acceptance

#### Step 4: Wait for Permissions to Propagate (Variable Time)

- **Typical wait:** A few minutes to a few hours
- **Maximum wait:** 24-48 hours
- **If longer:** Contact Epic Games support

**During this time:**
- Your Docker login will succeed
- Container pulls will fail with "denied" error
- This is normal and expected

## Complete Setup Checklist

Use this checklist to ensure you've completed all steps:

### One-Time Setup (For Repository Owner/Maintainer)

- [ ] Epic Games account created
- [ ] Epic account linked to GitHub at <https://www.epicgames.com/account/connections>
- [ ] Epic Games organization invitation accepted on GitHub
- [ ] Verified membership in Epic Games organization
- [ ] Container EULA accepted
- [ ] Waited for permissions to propagate (test with local Docker)
- [ ] Workflow file enabled (removed `.disabled` extension if present)

### GitHub Actions Configuration (Already Done)

- [x] Workflow uses `${{ secrets.GITHUB_TOKEN }}` ✅ Already configured
- [x] Workflow has `packages: read` permission ✅ Already configured
- [x] Docker login step included ✅ Already configured
- [x] Error handling and helpful messages ✅ Already configured

### Per-Developer Setup (For Team Members)

Each developer who wants to build locally needs to:

- [ ] Link their Epic Games account to their GitHub account
- [ ] Accept Epic Games organization invitation
- [ ] Accept Epic container EULA
- [ ] Create a Personal Access Token for local Docker use
- [ ] Test local Docker access

## Testing Your Setup

### Quick Validation Script

Run the automated validation script to check your setup:

```bash
# Make script executable (first time only)
chmod +x check_docker_setup.sh

# Run the validation
./check_docker_setup.sh
```

The script will check:
- ✅ Docker installation and daemon
- ✅ GitHub Container Registry authentication
- ✅ Epic Games container access
- ✅ Provide specific guidance for any issues

### Manual Testing Steps

### Test 1: Check Epic Account Connection

```bash
# Visit this URL in your browser
https://www.epicgames.com/account/connections

# Verify GitHub shows as "Connected"
```

### Test 2: Check GitHub Organization Membership

```bash
# Visit this URL in your browser
https://github.com/orgs/EpicGames/people

# Search for your username (if visible)
```

### Test 3: Test Local Docker Access

```bash
# Create a token at: https://github.com/settings/tokens
export GITHUB_TOKEN="your_token_here"

# Login to GHCR
echo $GITHUB_TOKEN | docker login ghcr.io -u YOUR_USERNAME --password-stdin

# Expected output: "Login Succeeded"

# Try pulling Epic container
docker pull ghcr.io/epicgames/unreal-engine:dev-slim-5.6

# Expected output: Successfully pulled image
# If denied: Wait longer or check Epic account setup
```

### Test 4: Trigger GitHub Actions Workflow

```bash
# Go to your repository Actions tab
https://github.com/Mittenzx/Adastrea/actions

# Click on "Unreal Engine 5.6 Build" workflow
# Click "Run workflow" button
# Select branch and options
# Click "Run workflow"

# Watch the logs to see if container pull succeeds
```

## Troubleshooting

### Issue: "Login Succeeded" but "denied" when pulling container

**Symptoms:**
```
Login Succeeded
Error response from daemon: denied
```

**Diagnosis:** GITHUB_TOKEN works, but you don't have Epic container access.

**Solutions:**
1. Verify Epic account is linked: <https://www.epicgames.com/account/connections>
2. Check for Epic organization invitation on GitHub
3. Accept Epic container EULA
4. Wait a few hours for permissions to propagate
5. Contact Epic support if still failing after 48 hours

### Issue: "unauthorized: authentication required"

**Symptoms:**
```
Error response from daemon: unauthorized: authentication required
```

**Diagnosis:** Docker login failed or token expired.

**Solutions:**
1. For GitHub Actions:
   - Check workflow has `packages: read` permission
   - Verify `GITHUB_TOKEN` is used correctly
   - Check workflow syntax

2. For local development:
   - Verify your PAT has `read:packages` scope
   - Check token hasn't expired
   - Re-run docker login command

### Issue: "GITHUB_TOKEN not found" or empty

**Symptoms:**
```
GHCR_TOKEN not found, using GITHUB_TOKEN instead
# But then authentication fails
```

**Diagnosis:** Environment variable not properly set in workflow.

**Solution:**
```yaml
# Use this pattern in workflow:
env:
  GH_TOKEN: ${{ secrets.GITHUB_TOKEN }}
run: |
  echo "$GH_TOKEN" | docker login ghcr.io -u ${{ github.actor }} --password-stdin
```

### Issue: Container version not found

**Symptoms:**
```
Error: manifest for ghcr.io/epicgames/unreal-engine:dev-slim-5.6 not found
```

**Solutions:**
1. Check available tags: <https://github.com/EpicGames/unreal-engine/pkgs/container/unreal-engine>
2. Try alternative tags:
   - `dev-5.6`
   - `dev-slim-5.6.0`
   - `runtime-5.6`
3. Verify UE version matches your project

### Issue: Build timeout

**Symptoms:**
```
Error: The job has exceeded the maximum execution time of 360 minutes.
```

**Solutions:**
1. Enable build caching
2. Use incremental builds
3. Consider self-hosted runner
4. Optimize build configuration

### Issue: Out of disk space

**Symptoms:**
```
Error: No space left on device
```

**Solutions:**
1. Use the disk cleanup step (already in workflow)
2. Clean up artifacts more aggressively
3. Use smaller container images
4. Use self-hosted runner with more storage

## Advanced Configuration

### Using a Custom Token (Optional)

If you want to use a custom token instead of `GITHUB_TOKEN`:

1. Create a PAT with `read:packages` scope
2. Add it to repository secrets as `GHCR_TOKEN`
3. Update workflow:

```yaml
- name: Login to GHCR
  env:
    TOKEN: ${{ secrets.GHCR_TOKEN }}  # or secrets.GITHUB_TOKEN
  run: |
    echo "$TOKEN" | docker login ghcr.io -u ${{ github.actor }} --password-stdin
```

### Using GitHub Packages Authentication Token

For organizations or enterprise accounts:

```yaml
- name: Login to GHCR
  uses: docker/login-action@v3
  with:
    registry: ghcr.io
    username: ${{ github.actor }}
    password: ${{ secrets.GITHUB_TOKEN }}
```

### Setting Up Self-Hosted Runner

If you want unlimited build minutes and faster builds:

1. Set up a machine with Docker installed
2. Install GitHub Actions runner
3. Configure runner:

```bash
# Download latest runner (check version at: https://github.com/actions/runner/releases)
curl -o actions-runner-linux.tar.gz -L \
  https://github.com/actions/runner/releases/latest/download/actions-runner-linux-x64.tar.gz

# Extract
tar xzf actions-runner-linux.tar.gz

# Configure
./config.sh --url https://github.com/Mittenzx/Adastrea --token YOUR_RUNNER_TOKEN

# Run
./run.sh
```

4. Update workflow to use self-hosted runner:

```yaml
jobs:
  build:
    runs-on: self-hosted  # Instead of ubuntu-latest
```

### Configuring Workflow Permissions

Explicitly set permissions in workflow (already done in `ue-build-example.yml`):

```yaml
permissions:
  contents: read      # Read repository
  packages: read      # Pull containers
  actions: read       # Read workflow status
```

## Security Best Practices

1. **Never commit tokens to repository**
   - Use GitHub Secrets for sensitive data
   - Add tokens to `.gitignore`
   - Use environment variables

2. **Use minimal permissions**
   - Only grant `read:packages` for container access
   - Avoid `write` permissions unless necessary

3. **Rotate tokens regularly**
   - Set expiration dates on PATs
   - Rotate every 90 days or less

4. **Use scoped tokens**
   - Create separate tokens for different purposes
   - Revoke unused tokens

5. **Monitor token usage**
   - Check GitHub audit log
   - Review active tokens regularly

## Summary

### What You Need to Understand

1. **GITHUB_TOKEN is automatic** - No setup required for GitHub Actions
2. **Epic access is manual** - You must link accounts and accept EULA
3. **Local testing needs PAT** - Create a Personal Access Token for local Docker
4. **Permissions take time** - Wait up to 48 hours after Epic account linking

### What's Already Configured

- ✅ Workflow files have correct GITHUB_TOKEN usage
- ✅ Docker login commands are properly configured
- ✅ Error messages guide users to Epic setup
- ✅ Workflow permissions are correctly set

### What You Need to Do

- 🔲 Link Epic Games account to GitHub
- 🔲 Accept Epic organization invitation
- 🔲 Accept Epic container EULA
- 🔲 Wait for permissions (test periodically)
- 🔲 (Optional) Create PAT for local testing

## Additional Resources

### Documentation

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [GitHub Packages Documentation](https://docs.github.com/en/packages)
- [Docker Login Action](https://github.com/docker/login-action)
- [Epic UE Container Images](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-container-images)

### Related Files

- [CLOUD_BUILD_QUICK_START.md](./CLOUD_BUILD_QUICK_START.md) - Quick setup guide
- [CLOUD_BUILD_SERVICES.md](./CLOUD_BUILD_SERVICES.md) - Comprehensive build services comparison
- [.github/workflows/README.md](./.github/workflows/README.md) - Workflow documentation
- [.github/workflows/ue-build-example.yml](./.github/workflows/ue-build-example.yml) - Example workflow

### Support

**For GITHUB_TOKEN issues:**
- GitHub Actions documentation
- GitHub support

**For Epic container issues:**
- Epic Games Developer Forums: <https://forums.unrealengine.com/>
- Epic Games Support: <https://www.epicgames.com/help/>
- Unreal Engine Documentation: <https://dev.epicgames.com/>

**For workflow issues:**
- Check workflow logs in Actions tab
- Review this documentation
- Open issue in Adastrea repository

---

**Last Updated:** 2024-11-23  
**Version:** 1.0  
**Maintained By:** Adastrea Development Team
