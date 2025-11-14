# 🚀 Start Here - Get Your Build Files in 30 Seconds

You're looking for the build scripts and documentation from PR #120. Here's how to get them.

## ⚡ The Fastest Way

Copy and paste this into your terminal:

```bash
cd /path/to/Adastrea && bash build_package/extract_files.sh && tar -czf adastrea_build_files.tar.gz build_package/
```

✅ **Done!** You now have `adastrea_build_files.tar.gz` containing all 6 build files.

## 📦 What You'll Get

| File | Size | Purpose |
|------|------|---------|
| build_windows.bat | ~9 KB | Windows build script |
| build_unix.sh | ~10 KB | Unix/Linux/Mac build script |
| BUILD_ERROR_DEBUGGING.md | ~15 KB | Quick debugging reference |
| BUILD_TROUBLESHOOTING.md | ~9 KB | Troubleshooting guide |
| UNREAL_BUILD_GUIDE.md | ~35 KB | Complete build guide |
| .github/workflows/build-test.yml | ~8 KB | CI/CD workflow |
| **Total** | **~85 KB** | **Compresses to ~17 KB** |

## 📚 Documentation Files

Choose what you need:

1. **QUICKSTART.md** ← Start here for the one-command solution
2. **IMPORTANT_NOTE.md** - Why the files need extraction
3. **EXTRACTION_INSTRUCTIONS.md** - Detailed methods if quick start fails
4. **README.md** - Complete package documentation
5. **extract_files.sh** - The automated extraction script

## 🎯 Your Options

### Option 1: Quickest (Recommended)
```bash
bash build_package/extract_files.sh && tar -czf adastrea_build_files.tar.gz build_package/
```

### Option 2: Step by Step
```bash
# Step 1: Extract files from git history
bash build_package/extract_files.sh

# Step 2: Create archive
tar -czf adastrea_build_files.tar.gz build_package/

# Step 3: Verify
ls -lh adastrea_build_files.tar.gz
```

### Option 3: Manual
See `EXTRACTION_INSTRUCTIONS.md` for manual git commands.

## ❓ Common Questions

**Q: Why aren't the files already here?**  
A: They were removed in PR #120 and only exist in git history. The CI environment can't access git history, but your local clone can.

**Q: Will the script work on my machine?**  
A: Yes! It works on Windows (Git Bash/WSL), Mac, and Linux. It automatically handles:
- Shallow clones (fetches full history if needed)
- Missing commits (provides clear instructions)
- File permissions (makes scripts executable)

**Q: What if it fails?**  
A: See `EXTRACTION_INSTRUCTIONS.md` for 2 alternative manual methods.

**Q: How long does this take?**  
A: Usually 10-30 seconds depending on internet speed (if fetching history).

## 🔍 What's Next?

After you have `adastrea_build_files.tar.gz`:

1. **Transfer** it to your new build repository
2. **Extract**: `tar -xzf adastrea_build_files.tar.gz`
3. **Review**: Check `build_package/README.md` for integration steps
4. **Use**: Run the build scripts in your new repo

## 🆘 Need Help?

1. Read `QUICKSTART.md` for more details
2. Check `IMPORTANT_NOTE.md` for troubleshooting
3. See `EXTRACTION_INSTRUCTIONS.md` for manual methods
4. Open an issue in the Adastrea repository

## ✨ Quick Commands Reference

```bash
# Extract files only
bash build_package/extract_files.sh

# Create archive only  
tar -czf adastrea_build_files.tar.gz build_package/

# Extract archive in new repo
tar -xzf adastrea_build_files.tar.gz

# List archive contents
tar -tzf adastrea_build_files.tar.gz

# Verify extracted files
ls -lh build_package/*.{bat,sh,md} build_package/.github/workflows/*.yml
```

## 📋 Checklist

- [ ] Navigate to your local Adastrea repository
- [ ] Run: `bash build_package/extract_files.sh`
- [ ] Create archive: `tar -czf adastrea_build_files.tar.gz build_package/`
- [ ] Verify: `ls -lh adastrea_build_files.tar.gz`
- [ ] Transfer to build repository
- [ ] Done! ✅

---

**Need more details?** → Read `QUICKSTART.md`  
**Having issues?** → Read `IMPORTANT_NOTE.md`  
**Want manual control?** → Read `EXTRACTION_INSTRUCTIONS.md`

**Created**: 2025-11-14 | **Purpose**: Quick access to PR #120 build files
