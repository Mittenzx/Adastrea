# Docker Container Access - Quick Reference Card

## 🚨 Error: "denied" when pulling container

```bash
Error response from daemon: denied
```

**Diagnosis:** GitHub authentication ✓ works, Epic permissions ✗ not granted

**Fix:**
1. Link Epic account to GitHub: [epicgames.com/account/connections](https://www.epicgames.com/account/connections)
2. Join EpicGames org: [github.com/EpicGames](https://github.com/EpicGames)
3. Accept EULA: [Container Images Docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-container-images)
4. Wait 2-24 hours for permissions

---

## 🚨 Error: "unauthorized: authentication required"

```bash
Error: unauthorized: authentication required
```

**Diagnosis:** Docker login failed or token invalid

**Fix:**
```bash
# Create token at: https://github.com/settings/tokens
# Required scope: read:packages

export GITHUB_TOKEN="ghp_xxxxx"
echo $GITHUB_TOKEN | docker login ghcr.io -u YOUR_USERNAME --password-stdin
```

---

## 🚨 Error: "manifest unknown"

```bash
Error: manifest unknown: manifest unknown
```

**Diagnosis:** Container doesn't exist or you don't have access

**Fix:**
- Verify tag: `dev-slim-5.6`
- Check available tags: [github.com/orgs/EpicGames/packages](https://github.com/orgs/EpicGames/packages)
- Ensure Epic Games access granted

---

## 🔍 Diagnostic Tools

### Local Testing
```bash
# Comprehensive 10-test diagnostic
./test_epic_connection.sh

# Quick Docker setup check
./check_docker_setup.sh
```

### GitHub Actions
1. Go to **Actions** tab
2. Select **"Diagnose Container Access"**
3. Click **"Run workflow"**
4. Review output for specific issues

---

## ⚡ Quick Commands

```bash
# Test Docker
docker version
docker info

# Test GHCR connectivity
curl -I https://ghcr.io

# Test Docker login (requires token)
echo $GITHUB_TOKEN | docker login ghcr.io -u $USERNAME --password-stdin

# Inspect manifest (doesn't download)
docker manifest inspect ghcr.io/epicgames/unreal-engine:dev-slim-5.6

# Pull container (several GB)
docker pull ghcr.io/epicgames/unreal-engine:dev-slim-5.6
```

---

## 📖 Full Documentation

- **Complete Troubleshooting:** [CONTAINER_DIAGNOSTICS.md](CONTAINER_DIAGNOSTICS.md)
- **Setup Guide:** [GITHUB_TOKEN_DOCKER_SETUP.md](GITHUB_TOKEN_DOCKER_SETUP.md)
- **Cloud Build:** [CLOUD_BUILD_QUICK_START.md](CLOUD_BUILD_QUICK_START.md)

---

## ✅ Setup Checklist

- [ ] Epic account created
- [ ] Epic account linked to GitHub
- [ ] EpicGames organization invitation accepted
- [ ] Container EULA accepted
- [ ] Permissions propagated (2-24 hours)
- [ ] Docker installed and running
- [ ] GitHub token created with `read:packages`
- [ ] Docker login succeeded
- [ ] Container pull successful

---

## 🆘 Getting Help

1. Wait 24-48 hours after completing setup
2. Run diagnostics: `./test_epic_connection.sh`
3. Check [CONTAINER_DIAGNOSTICS.md](CONTAINER_DIAGNOSTICS.md)
4. Epic support: [epicgames.com/help](https://www.epicgames.com/help/)
5. Open GitHub issue with diagnostic output

---

**Version:** 1.0 | **Last Updated:** 2025-11-23
