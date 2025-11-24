# Cloud Build Quick Start Guide

A quick reference for setting up cloud build services for the Adastrea Unreal Engine project.

## TL;DR - Fastest Setup

For Adastrea (open-source, GitHub-hosted), use **GitHub Actions with Docker**:

```bash
# 1. Link Epic Games account to GitHub
# Visit: https://www.epicgames.com/account/connections

# 2. Accept Epic Container EULA
# Visit Epic Games Developer portal

# 3. Add GitHub Actions workflow to your repo
# Copy .github/workflows/ue-build-example.yml.disabled to .github/workflows/ue-build-example.yml (remove .disabled)

# 4. Push code and watch build run in Actions tab
```

## Quick Service Comparison

### GitHub Actions + Docker ⭐ (RECOMMENDED)
- **Cost:** FREE for public repos
- **Setup Time:** 1-2 hours
- **Difficulty:** Medium
- **Best For:** Adastrea's current setup

### Unreal Cloud Build
- **Cost:** Subscription (~$99+/month estimated)
- **Setup Time:** 30 minutes
- **Difficulty:** Easy
- **Best For:** Small teams wanting managed service

### Horde on AWS
- **Cost:** AWS usage (~$200+/month estimated)
- **Setup Time:** 1-2 days
- **Difficulty:** High
- **Best For:** Large studios, 10+ developers

## GitHub Actions Implementation (30 Minutes)

### Prerequisites Checklist
- [ ] GitHub repository (already have: Mittenzx/Adastrea)
- [ ] Epic Games account linked to GitHub
- [ ] Docker knowledge (basic)
- [ ] GitHub Actions enabled

### Step-by-Step Setup

#### 1. Set Up Epic Container Access (15-30 minutes, may require waiting)

**📖 Complete Setup Guide:** See [GITHUB_TOKEN_DOCKER_SETUP.md](./GITHUB_TOKEN_DOCKER_SETUP.md) for detailed instructions.

**Critical:** Simply linking accounts is not sufficient. You must complete ALL steps:

```
Step 1: Link Epic Games account to GitHub
  → Visit: https://www.epicgames.com/account/connections
  → Connect your GitHub account to Epic Games account
  
Step 2: Accept Epic Games GitHub organization invitation
  → Check GitHub notifications and email for invitation
  → Visit: https://github.com/EpicGames
  → Accept the organization membership invitation
  → Verify you appear in the members list
  → NOTE: This step is REQUIRED to access containers
  
Step 3: Accept Epic's container EULA
  → Visit: https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-container-images
  → Review and accept the license agreement
  
Step 4: Wait for access to propagate (if needed)
  → Access may take a few hours to become active
  → Some accounts require manual Epic approval (24-48 hours)
```

**Note:** `GITHUB_TOKEN` is automatically provided by GitHub Actions - you don't need to configure it!

#### 2. Create Workflow File (10 minutes)

> **Note:** There is an example workflow file in the repository: `.github/workflows/ue-build-example.yml.disabled`.  
> You can either:
> - Copy and rename `.github/workflows/ue-build-example.yml.disabled` to `.github/workflows/ue-build-example.yml` (removing the `.disabled` extension), **or**
> - Create a new file `.github/workflows/ue-build.yml` and paste the following YAML.

> **Version Management:** The workflow automatically detects the Unreal Engine version from your `.uproject` file's `EngineAssociation` field. This means you don't need to hardcode version numbers in your CI/CD scripts - just update your `.uproject` file and the workflows will adapt automatically.

Create `.github/workflows/ue-build.yml`:

```yaml
name: UE5 Build Test

on:
  workflow_dispatch:  # Manual trigger for testing
  push:
    branches: [ main ]
    paths:
      - 'Source/**'
      - '*.uproject'

jobs:
  build-test:
    runs-on: ubuntu-latest
    
    steps:
      - name: Checkout
        uses: actions/checkout@v4
      
      - name: Detect UE Version and Validate
        id: detect-version
        run: |
          echo "Project: Adastrea"
          
          # Extract UE version from .uproject file
          UE_VERSION=$(grep -oP '"EngineAssociation":\s*"\K[^"]+' Adastrea.uproject)
          echo "UE Version: $UE_VERSION"
          echo "version=$UE_VERSION" >> $GITHUB_OUTPUT
          
          ls -la *.uproject
          
      - name: Validate Project Structure
        run: |
          test -f Adastrea.uproject || exit 1
          test -d Source || exit 1
          test -d Content || exit 1
          echo "✓ Project structure valid"
```

#### 3. Test Workflow (5 minutes)
```bash
# If you created a new file:
git add .github/workflows/ue-build.yml
# If you renamed the example:
# git add .github/workflows/ue-build-example.yml

git commit -m "Add UE5 build test workflow"
git push

# Then visit: https://github.com/Mittenzx/Adastrea/actions
```

#### 4. Add Docker Build (10 minutes)

Expand workflow after test succeeds:

```yaml
      - name: Login to GHCR
        run: echo ${{ secrets.GITHUB_TOKEN }} | docker login ghcr.io -u $GITHUB_ACTOR --password-stdin
        
      - name: Pull UE Container
        env:
          UE_VERSION: ${{ steps.detect-version.outputs.version }}
        run: docker pull ghcr.io/epicgames/unreal-engine:dev-slim-${UE_VERSION} || echo "Container access not configured yet"
```

## Alternative: Unreal Cloud Build (15 Minutes)

### For Teams Wanting Managed Service

#### 1. Sign Up (5 minutes)
```
1. Visit https://unreal.build/
2. Create account with GitHub OAuth
3. Select subscription plan
```

#### 2. Connect Repository (5 minutes)
```
1. Authorize Unreal Cloud Build GitHub app
2. Select Mittenzx/Adastrea repository
3. Choose branch for builds (main/develop)
```

#### 3. Configure Build (5 minutes)
```
1. Set UE version: Auto-detected from .uproject file (currently 5.6)
2. Set target platforms: Windows, Linux, Mac
3. Enable automatic builds on push
4. Save configuration
```

## Validation Commands

Test if your environment can build locally:

```bash
# Run project validation
./SetupCheck.sh

# Or with Python
python SetupCheck.py --verbose

# Check for build requirements
ls -la Source/
cat Adastrea.uproject
```

## Quick Troubleshooting

### Problem: Can't access Epic UE containers
**Solution:**
```bash
# 1. Verify Epic account linked at:
# https://www.epicgames.com/account/connections

# 2. Re-authenticate Docker:
echo $GITHUB_TOKEN | docker login ghcr.io -u YOUR_USERNAME --password-stdin
```

### Problem: Build times too long (>30 minutes)
**Solutions:**
- Enable build caching in workflow
- Use self-hosted runner with faster hardware
- Use Unreal Build Accelerator (UBA)
- Build only changed components

### Problem: Out of GitHub Actions minutes
**Solutions:**
- Optimize workflow to run only on specific paths
- Use self-hosted runner (free unlimited minutes)
- Upgrade to GitHub Pro/Team plan
- Use workflow_dispatch for manual builds only

### Problem: Container permission denied
**Solution:**
```yaml
# Add to workflow:
# Use 755 instead of 777 for better security. Prefer fixing user mapping in Docker.
- name: Fix Permissions
  run: sudo chmod -R 755 ${{ github.workspace }}
```

## Resource Links

### Essential
- [CLOUD_BUILD_SERVICES.md](./CLOUD_BUILD_SERVICES.md) - Full documentation
- [UE5 Container Quick Start](https://dev.epicgames.com/documentation/en-us/unreal-engine/quick-start-guide-for-using-container-images-in-unreal-engine)
- [GitHub Actions Docs](https://docs.github.com/en/actions)

### Actions Marketplace
- [UE5-Build-Project Action](https://github.com/marketplace/actions/ue5-build-project)
- [Docker Login Action](https://github.com/marketplace/actions/docker-login)
- [Upload Artifact Action](https://github.com/marketplace/actions/upload-a-build-artifact)

### Community
- [Unreal CI/CD Tutorial](https://dev.epicgames.com/community/learning/tutorials/1wVK/let-s-do-your-first-ci-with-unreal-engine)
- [UnrealContainers.com](https://unrealcontainers.com/)
- [ue4-docker GitHub](https://github.com/adamrehn/ue4-docker)

## Cost Estimates

### GitHub Actions (Public Repo)
- **Free tier:** Unlimited minutes
- **Private repo:** 2,000 minutes/month free, then $0.008/minute
- **Typical build:** 20-60 minutes
- **Monthly cost:** $0 (public) or $16-96 (private, 10 builds/month)

### Self-Hosted Runner
- **GitHub cost:** $0 (free unlimited minutes)
- **Infrastructure:** Your hardware or cloud VM
- **AWS t3.xlarge (example):** ~$120/month
- **Best for:** >1000 minutes/month usage

### Unreal Cloud Build
- **Estimated:** $99-299/month (check current pricing)
- **Includes:** Managed infrastructure, UBA, support
- **Best for:** Teams wanting hands-off solution

### Horde on AWS
- **Compute:** ~$200-500/month (variable)
- **Storage:** ~$50-100/month
- **Best for:** Large teams, >10 developers

## Next Steps After Setup

1. ✅ Choose and implement build service
2. ✅ Test first build
3. Add build status badge to README
4. Configure build notifications (Slack, email)
5. Set up automated testing
6. Add packaging for releases
7. Implement multi-platform builds
8. Configure artifact storage
9. Set up deployment pipelines
10. Document team workflows

## Sample Build Status Badge

Add to your README.md:

```markdown
# Use the filename of your workflow (ue-build.yml or ue-build-example.yml)
[![Build Status](https://github.com/Mittenzx/Adastrea/actions/workflows/ue-build-example.yml/badge.svg)](https://github.com/Mittenzx/Adastrea/actions)
```

## Getting Help

If you encounter issues:

1. **Check workflow logs:** GitHub Actions → Select run → View logs
2. **Verify prerequisites:** Run `./SetupCheck.sh`
3. **Review documentation:** [CLOUD_BUILD_SERVICES.md](./CLOUD_BUILD_SERVICES.md)
4. **Community:** Unreal Engine Forums, GitHub Discussions
5. **Epic Support:** For container access issues

## Version Management Strategy

The workflows automatically detect the Unreal Engine version from your `.uproject` file, eliminating the need to hardcode version numbers in multiple places.

**Benefits:**
- ✅ Single source of truth (`.uproject` file)
- ✅ Easier upgrades - update version once in `.uproject`
- ✅ No version mismatch between project and CI/CD
- ✅ More maintainable codebase

**How it works:**
1. Workflow extracts version: `UE_VERSION=$(grep -oP '"EngineAssociation":\s*"\K[^"]+' Adastrea.uproject)`
2. Uses dynamic version in Docker tags: `ghcr.io/epicgames/unreal-engine:dev-slim-${UE_VERSION}`
3. Version is passed between workflow jobs as needed

**Upgrading Unreal Engine:**
1. Update `EngineAssociation` in `Adastrea.uproject` (e.g., from "5.6" to "5.7")
2. Push to GitHub - workflows automatically use the new version
3. No workflow file changes needed!

## Summary

For **Adastrea**, the recommended path is:

1. **Start with:** GitHub Actions basic workflow (validation only)
2. **Then add:** Docker container integration
3. **Finally:** Full build and packaging
4. **Scale to:** Horde/AWS if team grows significantly

This progressive approach minimizes upfront investment while enabling rapid iteration.

---

**Quick Start Version:** 1.1  
**Last Updated:** 2025-11-24  
**See Also:** [CLOUD_BUILD_SERVICES.md](./CLOUD_BUILD_SERVICES.md) for comprehensive documentation
