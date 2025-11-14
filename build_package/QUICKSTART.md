# Quick Start Guide

Need the build files from PR #120? Here's the fastest way to get them.

## TL;DR - One Command

```bash
cd /path/to/Adastrea && bash build_package/extract_files.sh && tar -czf adastrea_build_files.tar.gz build_package/ && echo "✓ Done! Archive created: adastrea_build_files.tar.gz"
```

That's it! You now have `adastrea_build_files.tar.gz` ready to transfer to your build repository.

## What Just Happened?

1. **Extracted 6 files** from git history (commit before PR #120)
2. **Created archive** with all build scripts and documentation
3. **Ready to use** for migration to separate build repository

## What's in the Archive?

```
build_package/
├── build_windows.bat              # Windows build script
├── build_unix.sh                  # Unix/Linux/Mac build script
├── BUILD_ERROR_DEBUGGING.md       # Build error debugging guide
├── BUILD_TROUBLESHOOTING.md       # Build troubleshooting guide
├── UNREAL_BUILD_GUIDE.md         # Complete build documentation
├── .github/workflows/
│   └── build-test.yml            # GitHub Actions workflow
└── [documentation files]
```

## Next Steps

1. **Extract the archive** in your new build repository:
   ```bash
   tar -xzf adastrea_build_files.tar.gz
   ```

2. **Review the files**:
   ```bash
   ls -lh build_package/
   cat build_package/README.md
   ```

3. **Test the scripts** (optional):
   ```bash
   # Windows
   build_windows.bat

   # Unix/Linux/Mac
   ./build_unix.sh
   ```

4. **Integrate** following the checklist in `build_package/README.md`

## Troubleshooting

### "Commit not found"
```bash
# Fetch full history
git fetch --unshallow
# Try again
bash build_package/extract_files.sh
```

### "Script fails"
See `EXTRACTION_INSTRUCTIONS.md` for manual extraction methods.

### "Need more help"
- Read `IMPORTANT_NOTE.md` for detailed explanation
- Check `README.md` for complete documentation
- Open an issue in the repository

## What If I Already Have the Files?

If you previously extracted the files, you can just create the archive:

```bash
tar -czf adastrea_build_files.tar.gz build_package/
```

## File Sizes

- Uncompressed: ~85-90 KB
- Compressed (.tar.gz): ~17 KB
- Files: 6 build files + 4 documentation files

## Why This Process?

The build files were removed in PR #120 and only exist in git history. This extraction process:
- ✅ Retrieves files from the commit before deletion
- ✅ Preserves all content exactly as it was
- ✅ Packages everything for easy migration
- ✅ Includes documentation for the new repository

## Support

For issues or questions:
1. Check `EXTRACTION_INSTRUCTIONS.md` for detailed guidance
2. Read `IMPORTANT_NOTE.md` for background information
3. Review `README.md` for comprehensive documentation
4. Open an issue in the Adastrea repository

---

**Quick Reference Commands**

```bash
# Extract files only
bash build_package/extract_files.sh

# Create archive only
tar -czf adastrea_build_files.tar.gz build_package/

# Extract from archive
tar -xzf adastrea_build_files.tar.gz

# List archive contents
tar -tzf adastrea_build_files.tar.gz

# Verify extraction
ls -lh build_package/*.{bat,sh,md} build_package/.github/workflows/*.yml
```

---

**Last Updated**: 2025-11-14  
**Purpose**: Quick access to PR #120 build files
