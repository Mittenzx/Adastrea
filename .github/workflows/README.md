# GitHub Actions Workflows

This directory contains GitHub Actions workflow configurations for CI/CD automation.

## Current Workflows

### ci.yml
Active workflow that runs on every push and pull request:
- **Lint Documentation**: Validates markdown files with markdownlint
- **Asset Validation**: Template for Unreal asset validation (requires self-hosted runner)

### ue-build-example.yml.disabled
Example Unreal Engine build workflow (disabled by default):
- Complete UE5.6 build configuration template
- Docker-based Linux builds
- Windows build support
- Project validation
- Artifact upload

**To enable:** Rename from `.yml.disabled` to `.yml` after completing prerequisites.

## Setting Up UE5 Build Workflow

### Prerequisites

1. **Link Epic Games Account to GitHub**
   - Visit: [https://www.epicgames.com/account/connections](https://www.epicgames.com/account/connections)
   - Connect your GitHub account
   - This grants access to Epic's container images

2. **Accept Epic EULA for Container Images**
   - Visit Epic Games Developer portal
   - Accept End User License Agreement for container usage
   - Required to pull UE container images

3. **Verify Docker Access**
   ```bash
   # Test authentication
   echo $GITHUB_TOKEN | docker login ghcr.io -u YOUR_USERNAME --password-stdin
   
   # Try pulling UE container
   docker pull ghcr.io/epicgames/unreal-engine:dev-slim-5.6
   ```

4. **Configure GitHub Secrets** (if needed)
   - Go to Repository Settings → Secrets and variables → Actions
   - Add any required secrets (e.g., `UE_LICENSE_KEY`)

### Quick Start

#### Option 1: Enable Example Workflow

```bash
# Rename the example file to activate it
cd .github/workflows
mv ue-build-example.yml.disabled ue-build-example.yml

# Commit and push
git add ue-build-example.yml
git commit -m "Enable UE5 build workflow"
git push
```

#### Option 2: Test Validation Only First

The example workflow includes a standalone validation job that runs without UE:

1. Edit `ue-build-example.yml.disabled`
2. Change the validation job's `if: false` to `if: true`
3. Leave build jobs disabled (`if: false`)
4. Rename file to `.yml`
5. Test the validation step first

#### Option 3: Create Custom Workflow

Create your own workflow based on the example:

```yaml
name: My Custom Build

on:
  workflow_dispatch:

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Validate Project
        run: |
          test -f Adastrea.uproject || exit 1
          echo "✓ Project valid"
```

## Workflow Features

### Project Validation
- Checks for required files and directories
- Verifies UE version compatibility
- Runs SetupCheck.py if available
- Fast execution (~1-2 minutes)

### Docker-Based Builds
- Uses official Epic UE containers
- Consistent build environment
- Supports Linux builds
- Can run on GitHub-hosted runners

### Windows Builds
- Requires self-hosted runner with UE5 installed
- Native Windows compilation
- MSBuild integration

### Artifact Management
- Uploads build outputs
- 7-day retention by default
- Downloadable from Actions tab

## Choosing a Build Strategy

### Strategy 1: GitHub Actions + Docker (Recommended)
**Best for:** Public repositories, budget-conscious teams

**Pros:**
- Free for public repos
- No infrastructure to maintain
- Good documentation

**Cons:**
- Slower builds than self-hosted
- Requires Epic container access
- Limited to GitHub ecosystem

### Strategy 2: Self-Hosted Runners
**Best for:** Private repos with high build frequency

**Pros:**
- Faster builds
- Unlimited minutes
- Full control

**Cons:**
- Infrastructure costs
- Maintenance required
- Need to install UE5

### Strategy 3: External Build Services
**Best for:** Enterprises or teams wanting managed services

**Options:**
- Unreal Cloud Build
- TensorWorks Admiral
- Epic Horde on AWS

See [CLOUD_BUILD_SERVICES.md](../../CLOUD_BUILD_SERVICES.md) for comparisons.

## Monitoring Builds

### View Build Status
- Go to repository Actions tab
- Click on workflow run
- View logs for each job

### Build Badges
Add to README.md:
```markdown
[![Build Status](https://github.com/Mittenzx/Adastrea/actions/workflows/ue-build-example.yml/badge.svg)](https://github.com/Mittenzx/Adastrea/actions)
```

### Notifications
Configure in repository settings:
- Email notifications
- Slack integration
- Discord webhooks

## Troubleshooting

### Container Access Denied
```
Error: denied: permission_denied: The token provided does not match expected scopes.
```

**Solution:**
1. Verify Epic account linked to GitHub
2. Accept EULA at Epic developer portal
3. Re-authenticate Docker

### Build Timeout
```
Error: The job running on runner has exceeded the maximum execution time of 360 minutes.
```

**Solutions:**
- Optimize build settings
- Use build caching
- Enable distributed builds with UBA
- Consider self-hosted runner

### Disk Space Issues
```
Error: No space left on device
```

**Solution:**
- Add disk cleanup step (included in example)
- Use more efficient container images
- Clean artifacts more aggressively

### UE Container Not Found
```
Error: manifest for ghcr.io/epicgames/unreal-engine:dev-slim-5.6 not found
```

**Solutions:**
- Check UE version tag (5.6 vs 5.6.0)
- Verify container exists in GitHub Container Registry
- Use alternative tag (dev, dev-slim, etc.)

## Advanced Configuration

### Build Caching
Add caching to speed up builds:

```yaml
- name: Cache Build Files
  uses: actions/cache@v4
  with:
    path: |
      Build/
      Intermediate/
    key: ${{ runner.os }}-build-${{ hashFiles('**/*.uproject') }}
```

### Matrix Builds
Build for multiple platforms:

```yaml
strategy:
  matrix:
    platform: [Linux, Windows, Mac]
    config: [Development, Shipping]
```

### Conditional Builds
Only build on certain conditions:

```yaml
jobs:
  build:
    if: contains(github.event.head_commit.message, '[build]')
```

## Best Practices

1. **Start Small**
   - Begin with validation only
   - Add builds incrementally
   - Test on feature branches first

2. **Optimize Build Times**
   - Use incremental builds
   - Cache intermediate files
   - Limit builds to necessary paths

3. **Monitor Costs**
   - Track GitHub Actions minutes
   - Use workflow_dispatch for manual control
   - Consider self-hosted for high frequency

4. **Security**
   - Don't commit secrets
   - Use GitHub Secrets for credentials
   - Limit workflow permissions

5. **Documentation**
   - Document custom workflows
   - Keep README updated
   - Share knowledge with team

## Resources

### Documentation
- [CLOUD_BUILD_SERVICES.md](../../CLOUD_BUILD_SERVICES.md) - Complete guide
- [CLOUD_BUILD_QUICK_START.md](../../CLOUD_BUILD_QUICK_START.md) - Quick reference
- [GitHub Actions Docs](https://docs.github.com/en/actions)

### Actions Marketplace
- [UE5-Build-Project](https://github.com/marketplace/actions/ue5-build-project)
- [Upload Artifact](https://github.com/marketplace/actions/upload-a-build-artifact)
- [Cache Action](https://github.com/marketplace/actions/cache)

### Unreal Engine
- [UE Container Images](https://dev.epicgames.com/documentation/en-us/unreal-engine/quick-start-guide-for-using-container-images-in-unreal-engine)
- [Build Automation](https://docs.unrealengine.com/5.0/en-US/build-operations-cooking-packaging-deploying-and-running-projects-in-unreal-engine/)

## Support

For issues with workflows:
1. Check workflow logs in Actions tab
2. Review [CLOUD_BUILD_SERVICES.md](../../CLOUD_BUILD_SERVICES.md)
3. Search GitHub Discussions
4. Open issue in repository

For Epic container issues:
- Epic Games Developer Forums
- Unreal Engine documentation
- Epic Games support

---

**Last Updated:** 2025-11-21  
**Maintained By:** Adastrea Development Team
