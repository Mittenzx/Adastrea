# Important Note About This Package

## Current Status

This `build_package/` directory currently contains:
- ✅ `README.md` - Complete documentation about the package
- ✅ `EXTRACTION_INSTRUCTIONS.md` - Step-by-step extraction guide  
- ✅ `extract_files.sh` - Automated extraction script
- ⚠️ **Build files NOT YET EXTRACTED** (see below)

## Why Are the Files Not Included?

The actual build scripts and documentation were removed from the repository in PR #120 and are only available in the git history. This CI/CD environment has a shallow/grafted clone that doesn't include the full history needed to extract them.

However, **YOU can extract them locally** because your local clone has (or can fetch) the full git history!

## How to Get the Build Files

### Option 1: Automated (Recommended)

Run the provided extraction script from your local repository:

```bash
# From your local Adastrea repository root
cd /path/to/your/local/Adastrea
bash build_package/extract_files.sh
```

This will automatically:
1. Check if you have the required commit in your git history
2. Fetch full history if needed
3. Extract all 6 files from the commit before deletion
4. Set proper file permissions
5. Report success

### Option 2: Manual Extraction

If the script doesn't work, follow the detailed manual instructions in `EXTRACTION_INSTRUCTIONS.md`.

## What Happens After Extraction?

Once you run the extraction (locally on your machine), your `build_package/` directory will contain:

```
build_package/
├── .github/
│   └── workflows/
│       └── build-test.yml           # ~8 KB - GitHub Actions workflow
├── BUILD_ERROR_DEBUGGING.md         # ~15 KB - Debugging guide
├── BUILD_TROUBLESHOOTING.md         # ~9 KB - Troubleshooting guide
├── EXTRACTION_INSTRUCTIONS.md       # This guide
├── IMPORTANT_NOTE.md                # This file
├── README.md                        # Package documentation
├── UNREAL_BUILD_GUIDE.md           # ~35 KB - Complete build guide
├── build_unix.sh                   # ~10 KB - Unix/Linux/Mac build script
├── build_windows.bat               # ~9 KB - Windows build script
└── extract_files.sh                # Automated extraction script
```

Then you can create the archive:

```bash
# From repository root
tar -czf adastrea_build_files.tar.gz build_package/
```

This creates `adastrea_build_files.tar.gz` (~17 KB) that you can transfer to your new build repository.

## Why This Approach?

This approach is necessary because:

1. **Git History Required**: The files only exist in git history (commit `7ffc4368a`)
2. **CI Environment Limitations**: GitHub Actions runners use shallow clones
3. **Security**: We can't include authentication tokens to fetch from private repos
4. **Local Access**: Your local git repository has (or can fetch) the full history

## Verification Steps

After running the extraction script locally:

1. **Check file count**:
   ```bash
   ls -1 build_package/*.{bat,sh,md} build_package/.github/workflows/*.yml 2>/dev/null | wc -l
   # Should show: 9 files
   ```

2. **Check file sizes**:
   ```bash
   du -sh build_package/
   # Should show approximately 85-90 KB total
   ```

3. **Test the scripts** (optional):
   ```bash
   # Make sure build_unix.sh is executable
   chmod +x build_package/build_unix.sh
   
   # Check scripts have proper content
   head -5 build_package/build_windows.bat
   head -5 build_package/build_unix.sh
   ```

## Still Having Issues?

If you're unable to extract the files:

1. **Check your git version**: `git --version` (recommended: 2.25+)
2. **Verify you have the commit**:
   ```bash
   git fetch --unshallow  # If you have a shallow clone
   git show 7ffc4368a4b45088c187608da9a66029fbbcde57 --stat
   ```
3. **Use GitHub Web Interface**: Navigate to the commit on GitHub and download files manually
4. **Ask for help**: Open an issue in the repository

## Quick Start for Impatient Users

```bash
# One-liner to extract everything (run from repo root)
cd /path/to/Adastrea && bash build_package/extract_files.sh && tar -czf adastrea_build_files.tar.gz build_package/ && ls -lh adastrea_build_files.tar.gz
```

That's it! You'll have `adastrea_build_files.tar.gz` ready to go.

---

**Note**: This file will remain in the package as documentation even after files are extracted. It explains why the package was structured this way.

**Last Updated**: 2025-11-14  
**Related**: PR #120, BUILD_FILES_MOVED.md
