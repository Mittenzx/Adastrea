#!/bin/bash

################################################################################
# Adastrea - Pre-Build Setup Check Script
# 
# This script verifies that the development environment is properly configured
# before building the Unreal Engine project. It checks for:
# - Unreal Engine version compatibility
# - C++ compiler availability
# - Required project folders and files
# - Optional: Static analysis tools (cppcheck)
# 
# Usage:
#   ./SetupCheck.sh [options]
#
# Options:
#   --clean     Clean build folders (Intermediate, Saved, Binaries)
#   --analyze   Run static analysis with cppcheck (if available)
#   --help      Display this help message
#
################################################################################

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script options
CLEAN_BUILD=false
RUN_ANALYSIS=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --analyze)
            RUN_ANALYSIS=true
            shift
            ;;
        --help)
            head -n 25 "$0" | grep "^#" | sed 's/^# //g' | sed 's/^#//g'
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Helper functions
print_header() {
    echo -e "\n${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ $1${NC}"
}

# Check if running from project root
if [ ! -f "Adastrea.uproject" ]; then
    print_error "Error: This script must be run from the Adastrea project root directory"
    print_info "Current directory: $(pwd)"
    exit 1
fi

print_header "Adastrea Pre-Build Setup Check"
echo "Checking development environment..."

# Initialize error counter
ERRORS=0
WARNINGS=0

################################################################################
# 1. Check Unreal Engine Version
################################################################################
print_header "1. Checking Unreal Engine Version"

REQUIRED_UE_VERSION="5.6"
if [ -f "Adastrea.uproject" ]; then
    UE_VERSION=$(grep -o '"EngineAssociation": *"[^"]*"' Adastrea.uproject | cut -d'"' -f4)
    if [ "$UE_VERSION" == "$REQUIRED_UE_VERSION" ]; then
        print_success "Required Unreal Engine version: $REQUIRED_UE_VERSION"
    else
        print_warning "Expected Unreal Engine $REQUIRED_UE_VERSION, found: $UE_VERSION"
        ((WARNINGS++))
    fi
else
    print_error "Adastrea.uproject file not found!"
    ((ERRORS++))
fi

################################################################################
# 2. Check C++ Compiler
################################################################################
print_header "2. Checking C++ Compiler"

COMPILER_FOUND=false

# Check for g++
if command -v g++ &> /dev/null; then
    GCC_VERSION=$(g++ --version | head -n1)
    print_success "g++ compiler found: $GCC_VERSION"
    COMPILER_FOUND=true
fi

# Check for clang++
if command -v clang++ &> /dev/null; then
    CLANG_VERSION=$(clang++ --version | head -n1)
    print_success "clang++ compiler found: $CLANG_VERSION"
    COMPILER_FOUND=true
fi

if [ "$COMPILER_FOUND" = false ]; then
    print_error "No C++ compiler found! Install g++ or clang++"
    print_info "  Ubuntu/Debian: sudo apt-get install build-essential"
    print_info "  Mac: xcode-select --install"
    ((ERRORS++))
fi

################################################################################
# 3. Check Required Folders
################################################################################
print_header "3. Checking Required Project Folders"

REQUIRED_FOLDERS=("Source" "Config" "Content")

for folder in "${REQUIRED_FOLDERS[@]}"; do
    if [ -d "$folder" ]; then
        print_success "Found folder: $folder"
    else
        print_error "Missing required folder: $folder"
        ((ERRORS++))
    fi
done

# Check Source subfolders
print_info "Checking Source structure..."
SOURCE_MODULES=("Source/Adastrea" "Source/PlayerMods" "Source/StationEditor")

for module in "${SOURCE_MODULES[@]}"; do
    if [ -d "$module" ]; then
        print_success "Found module: $module"
    else
        print_error "Missing module: $module"
        ((ERRORS++))
    fi
done

################################################################################
# 4. Check Required Files
################################################################################
print_header "4. Checking Required Files"

REQUIRED_FILES=(
    "Adastrea.uproject"
    "Source/Adastrea/Adastrea.h"
    "Source/Adastrea/Adastrea.cpp"
    "Config/DefaultEngine.ini"
    "Config/DefaultGame.ini"
)

for file in "${REQUIRED_FILES[@]}"; do
    if [ -f "$file" ]; then
        print_success "Found file: $file"
    else
        print_error "Missing required file: $file"
        ((ERRORS++))
    fi
done

################################################################################
# 5. Check for DataAssets Documentation
################################################################################
print_header "5. Checking Documentation and Assets"

DOCS_TO_CHECK=(
    "README.md"
    "CONTRIBUTING.md"
    "Assets/PlaytestingChecklist.md"
)

for doc in "${DOCS_TO_CHECK[@]}"; do
    if [ -f "$doc" ]; then
        print_success "Found documentation: $doc"
    else
        print_warning "Missing documentation: $doc"
        ((WARNINGS++))
    fi
done

################################################################################
# 6. Clean Build Folders (Optional)
################################################################################
if [ "$CLEAN_BUILD" = true ]; then
    print_header "6. Cleaning Build Folders"
    
    CLEAN_FOLDERS=("Intermediate" "Saved" "Binaries")
    
    echo -n "This will delete Intermediate, Saved, and Binaries folders. Continue? (y/N): "
    read -r response
    
    if [[ "$response" =~ ^[Yy]$ ]]; then
        for folder in "${CLEAN_FOLDERS[@]}"; do
            if [ -d "$folder" ]; then
                print_info "Removing $folder..."
                rm -rf "$folder"
                print_success "Removed $folder"
            else
                print_info "Folder $folder does not exist (skipped)"
            fi
        done
        print_success "Build cleanup complete!"
    else
        print_info "Cleanup cancelled by user"
    fi
fi

################################################################################
# 7. Static Analysis (Optional)
################################################################################
if [ "$RUN_ANALYSIS" = true ]; then
    print_header "7. Running Static Analysis"
    
    if command -v cppcheck &> /dev/null; then
        print_info "Running cppcheck on Source directory..."
        print_warning "Note: This may take several minutes..."
        
        # Run cppcheck with appropriate flags for Unreal Engine
        cppcheck --enable=warning,style,performance,portability \
                 --suppress=unusedFunction \
                 --suppress=unmatchedSuppression \
                 --quiet \
                 --template='{file}:{line}: {severity}: {message}' \
                 Source/ 2>&1 | tee cppcheck_results.txt
        
        if [ -s cppcheck_results.txt ]; then
            print_warning "Static analysis found issues. See cppcheck_results.txt"
            ((WARNINGS++))
        else
            print_success "No issues found by static analysis"
            rm -f cppcheck_results.txt
        fi
    else
        print_warning "cppcheck not found. Install it for static analysis:"
        print_info "  Ubuntu/Debian: sudo apt-get install cppcheck"
        print_info "  Mac: brew install cppcheck"
        ((WARNINGS++))
    fi
fi

################################################################################
# Summary
################################################################################
print_header "Setup Check Summary"

if [ $ERRORS -eq 0 ] && [ $WARNINGS -eq 0 ]; then
    print_success "All checks passed! Your environment is ready for building."
    echo ""
    print_info "Next steps:"
    echo "  1. Generate project files: Right-click Adastrea.uproject"
    echo "  2. Build the project in Visual Studio/Xcode"
    echo "  3. Launch Unreal Engine and verify all systems"
    echo ""
    exit 0
elif [ $ERRORS -eq 0 ]; then
    print_warning "Setup check completed with $WARNINGS warning(s)"
    print_info "Your environment should work, but please review warnings above"
    echo ""
    exit 0
else
    print_error "Setup check failed with $ERRORS error(s) and $WARNINGS warning(s)"
    print_info "Please fix the errors above before building the project"
    echo ""
    exit 1
fi
