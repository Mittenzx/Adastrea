# Docker Setup Quick Reference Card

**Quick answer for: "How do I set up GITHUB_TOKEN to work with Docker for Unreal build?"**

## TL;DR

**GITHUB_TOKEN is already configured!** The issue is Epic Games container access.

## 4-Step Setup

### Step 1: Link Accounts (5 minutes)

Visit: <https://www.epicgames.com/account/connections>

✅ Connect your GitHub account to Epic Games

### Step 2: Accept Invitation (5 minutes)

Visit: <https://github.com/EpicGames>

✅ Accept the Epic Games organization invitation

### Step 3: Review Container Documentation (5 minutes)

Visit: <https://dev.epicgames.com/documentation/en-us/unreal-engine/quick-start-guide-for-using-container-images-in-unreal-engine>

✅ Review the container quick start guide and understand the Unreal Engine EULA covers container usage (no separate acceptance needed)

### Step 4: Wait

⏳ Permissions may take a few hours to propagate

## Test Your Setup

### Automated Validation

```bash
# Run the validation script
./check_docker_setup.sh
```

### Manual Testing

```bash
# For local testing only (create token at https://github.com/settings/tokens)
export GITHUB_TOKEN="your_token_here"
echo $GITHUB_TOKEN | docker login ghcr.io -u YOUR_USERNAME --password-stdin
docker pull ghcr.io/epicgames/unreal-engine:dev-slim-5.6
```

## Common Issues

### ❌ "Login Succeeded" but "denied" when pulling

**Cause:** Missing Epic account setup

**Fix:** Complete steps 1-3 above and wait

### ❌ "unauthorized: authentication required"

**Cause:** Token issue

**Fix (GitHub Actions):** Check workflow has `packages: read` permission

**Fix (Local):** Verify PAT has `read:packages` scope

## Important Notes

- ✅ `GITHUB_TOKEN` is **automatic** in GitHub Actions - no setup needed
- ✅ Workflow files are **already configured** correctly
- ⚠️ Epic access requires **all 3 steps** above
- ⏳ May take **up to 48 hours** for permissions

## Need More Help?

📖 **Complete Guide:** [GITHUB_TOKEN_DOCKER_SETUP.md](./GITHUB_TOKEN_DOCKER_SETUP.md)

🚀 **Cloud Build Guide:** [CLOUD_BUILD_QUICK_START.md](./CLOUD_BUILD_QUICK_START.md)

🔧 **Workflow Docs:** [.github/workflows/README.md](./.github/workflows/README.md)

## Summary

| What | Status | Action Needed |
|------|--------|---------------|
| GITHUB_TOKEN | ✅ Configured | None - automatic |
| Docker login | ✅ Works | None - automatic |
| Epic container access | ⚠️ **Requires setup** | **Link Epic account** |

---

**Last Updated:** 2024-11-23
**Version:** 1.0
