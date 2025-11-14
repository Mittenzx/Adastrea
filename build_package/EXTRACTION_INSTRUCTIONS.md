# Build Files Extraction Instructions

The build files and documentation from PR #120 need to be extracted from git history since they were removed from the main branch.

## Automated Extraction (Recommended)

Run the provided script to automatically extract all deleted files:

```bash
# From the repository root
bash build_package/extract_files.sh
```

This will populate the `build_package/` directory with all removed files.

## Manual Extraction

If you prefer to extract files manually or the script doesn't work:

### Method 1: Using git show (Individual Files)

Extract files one by one from the commit before they were deleted:

```bash
# Navigate to repository root
cd /path/to/Adastrea

# Create build_package directory structure
mkdir -p build_package/.github/workflows

# Extract each file from the commit before deletion
# The commit SHA where files still existed: 7ffc4368a4b45088c187608da9a66029fbbcde57

git show 7ffc4368a4b45088c187608da9a66029fbbcde57:build_windows.bat > build_package/build_windows.bat
git show 7ffc4368a4b45088c187608da9a66029fbbcde57:build_unix.sh > build_package/build_unix.sh
git show 7ffc4368a4b45088c187608da9a66029fbbcde57:BUILD_ERROR_DEBUGGING.md > build_package/BUILD_ERROR_DEBUGGING.md
git show 7ffc4368a4b45088c187608da9a66029fbbcde57:BUILD_TROUBLESHOOTING.md > build_package/BUILD_TROUBLESHOOTING.md
git show 7ffc4368a4b45088c187608da9a66029fbbcde57:UNREAL_BUILD_GUIDE.md > build_package/UNREAL_BUILD_GUIDE.md
git show 7ffc4368a4b45088c187608da9a66029fbbcde57:.github/workflows/build-test.yml > build_package/.github/workflows/build-test.yml

# Make Unix script executable
chmod +x build_package/build_unix.sh

echo "Files extracted successfully!"
```

### Method 2: Using git checkout (Temporary)

This method temporarily checks out the old commit to copy files:

```bash
# Navigate to repository root
cd /path/to/Adastrea

# Create a temporary directory
mkdir /tmp/adastrea_build_files_temp

# Checkout the commit where files existed (detached HEAD)
git checkout 7ffc4368a4b45088c187608da9a66029fbbcde57

# Copy the files we need
cp build_windows.bat /tmp/adastrea_build_files_temp/
cp build_unix.sh /tmp/adastrea_build_files_temp/
cp BUILD_ERROR_DEBUGGING.md /tmp/adastrea_build_files_temp/
cp BUILD_TROUBLESHOOTING.md /tmp/adastrea_build_files_temp/
cp UNREAL_BUILD_GUIDE.md /tmp/adastrea_build_files_temp/
mkdir -p /tmp/adastrea_build_files_temp/.github/workflows
cp .github/workflows/build-test.yml /tmp/adastrea_build_files_temp/.github/workflows/

# Return to the current branch
git checkout -

# Copy files to build_package
cp -r /tmp/adastrea_build_files_temp/* build_package/

# Clean up
rm -rf /tmp/adastrea_build_files_temp

# Make Unix script executable
chmod +x build_package/build_unix.sh

echo "Files extracted successfully!"
```

### Method 3: Using GitHub Web Interface

1. Go to the commit in GitHub: https://github.com/Mittenzx/Adastrea/tree/7ffc4368a4b45088c187608da9a66029fbbcde57
2. Navigate to each file
3. Click "Raw" button
4. Save the file contents to `build_package/` directory

## Verification

After extraction, verify all files are present:

```bash
ls -R build_package/

# Expected output:
# build_package/:
# .github  BUILD_ERROR_DEBUGGING.md  BUILD_TROUBLESHOOTING.md  
# EXTRACTION_INSTRUCTIONS.md  README.md  UNREAL_BUILD_GUIDE.md  
# build_unix.sh  build_windows.bat
#
# build_package/.github:
# workflows
#
# build_package/.github/workflows:
# build-test.yml
```

## Creating the Archive

Once files are extracted, create the tar.gz archive:

```bash
# From repository root
cd /path/to/Adastrea

# Create the archive
tar -czf adastrea_build_files.tar.gz build_package/

# Verify archive
tar -tzf adastrea_build_files.tar.gz | head -10

echo "Archive created: adastrea_build_files.tar.gz"
```

## Troubleshooting

### "fatal: Path does not exist in commit"

This error means the commit SHA is not available in your local repository. Fix:

```bash
# Fetch the full history if you have a shallow clone
git fetch --unshallow

# Or fetch the specific commit
git fetch origin 7ffc4368a4b45088c187608da9a66029fbbcde57
```

### "Shallow clone" or "grafted repository"

If you have a shallow clone:

```bash
# Convert to full clone
git fetch --unshallow

# Then try extraction again
```

### Files have wrong line endings

On Windows, ensure files have correct line endings:

```bash
# For Unix script (should have LF)
git show 7ffc4368a4b45088c187608da9a66029fbbcde57:build_unix.sh | dos2unix > build_package/build_unix.sh

# For Windows script (should have CRLF)
git show 7ffc4368a4b45088c187608da9a66029fbbcde57:build_windows.bat > build_package/build_windows.bat
```

## Next Steps

After successful extraction:

1. Review the `README.md` in `build_package/` for usage instructions
2. Create the tar.gz archive as shown above
3. Transfer the archive to your new build tools repository
4. Follow the integration checklist in `build_package/README.md`

## Support

If you encounter issues:
1. Ensure you have the full git history (`git fetch --unshallow`)
2. Verify the commit exists: `git show 7ffc4368a4b45088c187608da9a66029fbbcde57`
3. Check your git version: `git --version` (recommended: 2.25+)

---

**Created**: 2025-11-14  
**For**: Adastrea Build Files Migration  
**Source**: PR #120 (commit 7ffc4368a4b45088c187608da9a66029fbbcde57)
