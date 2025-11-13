#!/bin/bash
# Adastrea - Unix Build Script (Mac/Linux)
# This script builds the Adastrea project using UnrealBuildTool
# Usage: ./build_unix.sh [clean|editor|game|shipping] [verbose]
# Examples:
#   ./build_unix.sh              - Build editor in Development
#   ./build_unix.sh clean        - Clean build artifacts and rebuild
#   ./build_unix.sh editor       - Build editor (same as default)
#   ./build_unix.sh game         - Build standalone game
#   ./build_unix.sh shipping     - Build shipping configuration
#   ./build_unix.sh editor verbose - Build with verbose output

set -e  # Exit on error

# ============================================================================
# Configuration - Modify these if your setup differs
# ============================================================================

# Auto-detect platform
case "$(uname -s)" in
    Darwin*)
        PLATFORM="Mac"
        DEFAULT_UE_ROOT="/Users/Shared/Epic Games/UE_5.6"
        ;;
    Linux*)
        PLATFORM="Linux"
        DEFAULT_UE_ROOT="$HOME/UnrealEngine"
        ;;
    *)
        echo "ERROR: Unsupported platform: $(uname -s)"
        echo "This script supports Mac and Linux only."
        exit 1
        ;;
esac

# Unreal Engine installation path (use environment variable or default)
UE_ROOT="${UE_ROOT:-$DEFAULT_UE_ROOT}"

# Project settings (automatically determined)
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
PROJECT_FILE="$PROJECT_ROOT/Adastrea.uproject"

# Default build settings
BUILD_CONFIG="Development"
TARGET="AdastreaEditor"
CLEAN_BUILD=0
VERBOSE_OUTPUT=0

# ============================================================================
# Parse Command Line Arguments
# ============================================================================

show_help() {
    cat << EOF
Adastrea Build Script - $PLATFORM

Usage: $0 [options]

Options:
  clean          - Clean build artifacts before building
  editor         - Build editor (default)
  game           - Build standalone game
  shipping       - Build in Shipping configuration (optimized, no debug)
  development    - Build in Development configuration (default)
  debug          - Build in Debug configuration (full debug symbols)
  verbose        - Show detailed build output
  help, -h       - Show this help message

Examples:
  $0
    Builds the editor in Development configuration

  $0 clean
    Deletes build artifacts and rebuilds the editor

  $0 game
    Builds the standalone game in Development configuration

  $0 shipping
    Builds the standalone game in Shipping configuration (release)

  $0 editor verbose
    Builds the editor with detailed output for debugging

  $0 clean shipping verbose
    Clean build of shipping game with verbose output

Configuration:
  Platform: $PLATFORM
  Unreal Engine Path: $UE_ROOT
  Project File: $PROJECT_FILE

Environment Variables:
  UE_ROOT        - Override Unreal Engine installation path
                   Example: UE_ROOT="/path/to/UE_5.6" $0

For more information, see UNREAL_BUILD_GUIDE.md
EOF
    exit 0
}

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        clean)
            CLEAN_BUILD=1
            shift
            ;;
        editor)
            TARGET="AdastreaEditor"
            shift
            ;;
        game)
            TARGET="Adastrea"
            shift
            ;;
        shipping)
            BUILD_CONFIG="Shipping"
            TARGET="Adastrea"
            shift
            ;;
        development)
            BUILD_CONFIG="Development"
            shift
            ;;
        debug)
            BUILD_CONFIG="Debug"
            shift
            ;;
        verbose)
            VERBOSE_OUTPUT=1
            shift
            ;;
        help|-h|--help)
            show_help
            ;;
        *)
            echo "Warning: Unknown argument '$1' - ignoring"
            shift
            ;;
    esac
done

# ============================================================================
# Validate Setup
# ============================================================================

echo "============================================================================"
echo "Adastrea Build Script - $PLATFORM"
echo "============================================================================"
echo

# Check if Unreal Engine exists
if [ ! -d "$UE_ROOT" ]; then
    echo "ERROR: Unreal Engine not found at: $UE_ROOT"
    echo
    echo "Please set the UE_ROOT environment variable or edit this script."
    echo "Common locations:"
    if [ "$PLATFORM" = "Mac" ]; then
        echo "  - /Users/Shared/Epic Games/UE_5.6"
        echo "  - /Applications/Epic Games/UE_5.6"
    else
        echo "  - $HOME/UnrealEngine"
        echo "  - /opt/UnrealEngine"
    fi
    echo
    echo "Example: UE_ROOT=\"/path/to/UE_5.6\" $0"
    echo
    exit 1
fi

# Check if project file exists
if [ ! -f "$PROJECT_FILE" ]; then
    echo "ERROR: Project file not found: $PROJECT_FILE"
    echo
    echo "Please run this script from the Adastrea project root directory."
    echo
    exit 1
fi

# Check if Build.sh exists
BUILD_TOOL="$UE_ROOT/Engine/Build/BatchFiles/$PLATFORM/Build.sh"
if [ ! -f "$BUILD_TOOL" ]; then
    echo "ERROR: Build.sh not found at: $BUILD_TOOL"
    echo
    echo "Your Unreal Engine installation may be corrupted."
    if [ "$PLATFORM" == "Mac" ]; then
        echo "Try verifying your installation in Epic Games Launcher."
    else
        echo "Try reinstalling Unreal Engine."
    fi
    echo
    exit 1
fi

# Make sure Build.sh is executable
chmod +x "$BUILD_TOOL"

echo "Configuration:"
echo "  Platform: $PLATFORM"
echo "  Unreal Engine: $UE_ROOT"
echo "  Project: $PROJECT_FILE"
echo "  Target: $TARGET"
echo "  Configuration: $BUILD_CONFIG"
echo "  Clean Build: $CLEAN_BUILD"
echo "  Verbose: $VERBOSE_OUTPUT"
echo

# ============================================================================
# Clean Build Artifacts (if requested)
# ============================================================================

if [ "$CLEAN_BUILD" -eq 1 ]; then
    echo "Cleaning build artifacts..."
    echo
    
    if [ -d "$PROJECT_ROOT/Binaries" ]; then
        echo "  Deleting Binaries..."
        rm -rf "$PROJECT_ROOT/Binaries"
    fi
    
    if [ -d "$PROJECT_ROOT/Intermediate" ]; then
        echo "  Deleting Intermediate..."
        rm -rf "$PROJECT_ROOT/Intermediate"
    fi
    
    if [ -d "$PROJECT_ROOT/Saved/Logs" ]; then
        echo "  Deleting logs..."
        rm -rf "$PROJECT_ROOT/Saved/Logs"
    fi
    
    if [ -d "$PROJECT_ROOT/.vs" ]; then
        echo "  Deleting Visual Studio cache..."
        rm -rf "$PROJECT_ROOT/.vs"
    fi
    
    echo "  Clean completed."
    echo
fi

# ============================================================================
# Build Project
# ============================================================================

echo "============================================================================"
echo "Building $TARGET for $PLATFORM in $BUILD_CONFIG configuration..."
echo "============================================================================"
echo

# Construct build command
BUILD_CMD=("$BUILD_TOOL" "$TARGET" "$PLATFORM" "$BUILD_CONFIG" "-Project=$PROJECT_FILE")

# Add optional flags
if [ "$VERBOSE_OUTPUT" -eq 1 ]; then
    BUILD_CMD+=("-Verbose")
fi
BUILD_CMD+=("-Progress")

# Display command
echo "Command: ${BUILD_CMD[*]}"
echo

# Capture start time
START_TIME=$(date +%s)

# Execute build
set +e  # Don't exit on error, we want to handle it
"${BUILD_CMD[@]}"
BUILD_RESULT=$?
set -e  # Re-enable exit on error

# Capture end time
END_TIME=$(date +%s)
BUILD_DURATION=$((END_TIME - START_TIME))

echo
echo "============================================================================"

# ============================================================================
# Report Results
# ============================================================================

if [ $BUILD_RESULT -ne 0 ]; then
    echo "BUILD FAILED with error code $BUILD_RESULT"
    echo "============================================================================"
    echo
    echo "Troubleshooting:"
    echo "  1. Check the error messages above for specific issues"
    echo "  2. Try running: $0 clean"
    echo "  3. Check BUILD_TROUBLESHOOTING.md for common solutions"
    echo "  4. Run with verbose: $0 verbose"
    echo
    echo "Build logs are in: $PROJECT_ROOT/Saved/Logs/"
    echo
    exit $BUILD_RESULT
else
    echo "BUILD SUCCEEDED"
    echo "============================================================================"
    echo
    echo "Build Time: ${BUILD_DURATION} seconds"
    echo
    if [ "$TARGET" = "AdastreaEditor" ]; then
        echo "You can now open the project in Unreal Editor:"
        echo "  $PROJECT_FILE"
    else
        echo "Standalone game executable built in:"
        echo "  $PROJECT_ROOT/Binaries/$PLATFORM/"
    fi
    echo
    echo "Build logs are in: $PROJECT_ROOT/Saved/Logs/"
    echo
    exit 0
fi
