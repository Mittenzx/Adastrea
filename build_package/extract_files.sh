#!/bin/bash
# Automated extraction script for Adastrea build files from PR #120
# This script extracts deleted build scripts and documentation from git history

set -e  # Exit on error

echo "============================================================================"
echo "Adastrea Build Files Extraction"
echo "============================================================================"
echo ""

# Configuration
COMMIT_SHA="7ffc4368a4b45088c187608da9a66029fbbcde57"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "Repository root: $REPO_ROOT"
echo "Build package directory: $SCRIPT_DIR"
echo "Source commit: $COMMIT_SHA"
echo ""

# Check if we're in a git repository
if [ ! -d "$REPO_ROOT/.git" ]; then
    echo "ERROR: Not in a git repository"
    echo "Please run this script from within the Adastrea repository"
    exit 1
fi

# Change to repository root
cd "$REPO_ROOT"

# Check if commit exists
echo "Checking if commit exists in repository..."
if ! git cat-file -e "$COMMIT_SHA" 2>/dev/null; then
    echo "WARNING: Commit $COMMIT_SHA not found in local repository"
    echo ""
    echo "This might be a shallow clone. Attempting to fetch full history..."
    
    # Try to fetch full history
    if git fetch --unshallow 2>/dev/null; then
        echo "✓ Successfully fetched full history"
    else
        echo "✗ Could not fetch full history"
        echo ""
        echo "Please manually fetch the commit:"
        echo "  git fetch origin $COMMIT_SHA"
        echo ""
        exit 1
    fi
fi

# Verify commit is now accessible
if ! git cat-file -e "$COMMIT_SHA" 2>/dev/null; then
    echo "ERROR: Commit $COMMIT_SHA still not accessible"
    echo ""
    echo "This might be because:"
    echo "  1. The repository history has been rewritten"
    echo "  2. Access permissions prevent fetching the commit"
    echo ""
    echo "Please refer to EXTRACTION_INSTRUCTIONS.md for manual extraction"
    exit 1
fi

echo "✓ Commit found"
echo ""

# Create directory structure
echo "Creating directory structure..."
mkdir -p "$SCRIPT_DIR/.github/workflows"
echo "✓ Directories created"
echo ""

# Array of files to extract with their paths
declare -a FILES=(
    "build_windows.bat"
    "build_unix.sh"
    "BUILD_ERROR_DEBUGGING.md"
    "BUILD_TROUBLESHOOTING.md"
    "UNREAL_BUILD_GUIDE.md"
    ".github/workflows/build-test.yml"
)

# Extract each file
echo "Extracting files from commit $COMMIT_SHA..."
echo ""

EXTRACTED=0
FAILED=0

for FILE in "${FILES[@]}"; do
    echo -n "  Extracting: $FILE ... "
    
    TARGET_PATH="$SCRIPT_DIR/$FILE"
    TARGET_DIR="$(dirname "$TARGET_PATH")"
    
    # Ensure target directory exists
    mkdir -p "$TARGET_DIR"
    
    # Extract file using git show
    if git show "$COMMIT_SHA:$FILE" > "$TARGET_PATH" 2>/dev/null; then
        echo "✓"
        EXTRACTED=$((EXTRACTED + 1))
    else
        echo "✗ FAILED"
        FAILED=$((FAILED + 1))
    fi
done

echo ""

# Make Unix script executable
if [ -f "$SCRIPT_DIR/build_unix.sh" ]; then
    echo "Making build_unix.sh executable..."
    chmod +x "$SCRIPT_DIR/build_unix.sh"
    echo "✓ Done"
    echo ""
fi

# Print summary
echo "============================================================================"
echo "Extraction Summary"
echo "============================================================================"
echo "  Successfully extracted: $EXTRACTED files"
echo "  Failed: $FAILED files"
echo ""

if [ $FAILED -eq 0 ]; then
    echo "✓ All files extracted successfully!"
    echo ""
    echo "Build package contents:"
    ls -lh "$SCRIPT_DIR" | grep -v "^total" | grep -v "^d"
    echo ""
    echo "Next steps:"
    echo "  1. Review the README.md in build_package/"
    echo "  2. Create archive: tar -czf adastrea_build_files.tar.gz build_package/"
    echo "  3. Transfer to your build tools repository"
    echo ""
else
    echo "⚠ Some files failed to extract"
    echo ""
    echo "Please refer to EXTRACTION_INSTRUCTIONS.md for manual extraction methods"
    echo ""
    exit 1
fi

echo "============================================================================"
echo "Extraction Complete"
echo "============================================================================"
