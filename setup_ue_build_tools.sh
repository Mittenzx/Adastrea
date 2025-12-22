#!/bin/bash
# Script to download and setup only Unreal Engine build tools (not the full engine)
# This includes UnrealBuildTool, UnrealHeaderTool, and necessary build scripts

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
UE_TOOLS_DIR="${SCRIPT_DIR}/UnrealBuildTools"
UE_VERSION="5.6"
UE_REPO="https://github.com/EpicGames/UnrealEngine.git"

echo "========================================"
echo "Adastrea - Unreal Build Tools Setup"
echo "========================================"
echo ""
echo "This script downloads ONLY the build tools from Unreal Engine"
echo "Not the full engine source (~500MB vs ~50GB)"
echo ""

# Check if git is available
if ! command -v git &> /dev/null; then
    echo "ERROR: git is not installed or not in PATH"
    exit 1
fi

# Function to clone sparse checkout of UnrealEngine (build tools only)
clone_build_tools() {
    echo "Setting up sparse checkout for build tools only..."
    
    if [ -d "${UE_TOOLS_DIR}" ]; then
        echo "UnrealBuildTools directory already exists at: ${UE_TOOLS_DIR}"
        read -p "Remove and re-download? (y/N): " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            echo "Removing existing directory..."
            rm -rf "${UE_TOOLS_DIR}"
        else
            echo "Using existing UnrealBuildTools directory."
            return 0
        fi
    fi
    
    echo "Creating sparse checkout directory..."
    mkdir -p "${UE_TOOLS_DIR}"
    cd "${UE_TOOLS_DIR}"
    
    # Initialize git repository with sparse checkout
    git init
    git config core.sparseCheckout true
    
    # Add remote
    echo "Adding UnrealEngine remote..."
    git remote add origin "${UE_REPO}"
    
    # Configure sparse checkout to get only build tools
    echo "Configuring sparse checkout (build tools only)..."
    cat > .git/info/sparse-checkout << EOF
# Build tools and scripts
Engine/Build/
Engine/Binaries/DotNET/
Engine/Binaries/ThirdParty/DotNet/
Engine/Source/Programs/UnrealBuildTool/
Engine/Source/Programs/Shared/

# Build batch files
Engine/Build/BatchFiles/

# Minimal config files needed for build
Engine/Config/BaseEngine.ini
Engine/Config/BasePlatforms.ini

# Build version info
Engine/Build/Build.version

# Documentation for build process
Engine/Build/README.md
EOF
    
    # Fetch only the specified paths
    echo ""
    echo "Downloading build tools (this may take a few minutes)..."
    echo "Size: ~500MB instead of ~50GB"
    echo ""
    
    git fetch --depth 1 origin "${UE_VERSION}"
    
    if [ $? -ne 0 ]; then
        echo ""
        echo "ERROR: Failed to fetch from UnrealEngine repository."
        echo ""
        echo "This typically happens because:"
        echo "1. You haven't linked your GitHub account to your Epic Games account"
        echo "2. You haven't accepted the Epic Games organization invitation"
        echo ""
        echo "Please follow these steps:"
        echo "1. Go to https://www.epicgames.com/account/connections"
        echo "2. Link your GitHub account"
        echo "3. Accept the invitation to join the Epic Games GitHub organization"
        echo "4. Verify access at https://github.com/EpicGames"
        echo ""
        cd "${SCRIPT_DIR}"
        rm -rf "${UE_TOOLS_DIR}"
        exit 1
    fi
    
    git checkout FETCH_HEAD
    
    cd "${SCRIPT_DIR}"
    
    echo ""
    echo "✓ Build tools downloaded successfully!"
    echo ""
    echo "Downloaded size:"
    du -sh "${UE_TOOLS_DIR}"
}

# Function to setup build tools
setup_build_tools() {
    echo ""
    echo "Setting up build tools..."
    
    cd "${UE_TOOLS_DIR}"
    
    # Check for Setup script
    if [ -f "Engine/Build/BatchFiles/Linux/Setup.sh" ]; then
        echo "Found Setup.sh for build tools"
        chmod +x Engine/Build/BatchFiles/Linux/Setup.sh
        # Note: We might not need to run this for just build tools
    fi
    
    # Make build scripts executable
    if [ -d "Engine/Build/BatchFiles/Linux" ]; then
        echo "Making Linux build scripts executable..."
        chmod +x Engine/Build/BatchFiles/Linux/*.sh 2>/dev/null || true
    fi
    
    if [ -d "Engine/Build/BatchFiles/Mac" ]; then
        echo "Making Mac build scripts executable..."
        chmod +x Engine/Build/BatchFiles/Mac/*.sh 2>/dev/null || true
    fi
    
    cd "${SCRIPT_DIR}"
}

# Function to verify build tools
verify_build_tools() {
    echo ""
    echo "Verifying build tools installation..."
    
    local errors=0
    
    # Check for UnrealBuildTool
    if [ -d "${UE_TOOLS_DIR}/Engine/Source/Programs/UnrealBuildTool" ]; then
        echo "✓ UnrealBuildTool source found"
    else
        echo "✗ UnrealBuildTool source not found"
        errors=$((errors + 1))
    fi
    
    # Check for DotNET binaries
    if [ -d "${UE_TOOLS_DIR}/Engine/Binaries/DotNET" ]; then
        echo "✓ DotNET binaries directory found"
    else
        echo "✗ DotNET binaries directory not found"
        errors=$((errors + 1))
    fi
    
    # Check for build batch files
    if [ -d "${UE_TOOLS_DIR}/Engine/Build/BatchFiles" ]; then
        echo "✓ Build batch files found"
    else
        echo "✗ Build batch files not found"
        errors=$((errors + 1))
    fi
    
    if [ $errors -gt 0 ]; then
        echo ""
        echo "WARNING: Some build tools are missing. Build may fail."
        return 1
    fi
    
    echo ""
    echo "✓ All essential build tools verified!"
    return 0
}

# Main execution
main() {
    echo "Starting build tools setup..."
    echo ""
    
    # Clone build tools with sparse checkout
    clone_build_tools
    
    # Setup build tools
    setup_build_tools
    
    # Verify installation
    verify_build_tools
    
    echo ""
    echo "========================================"
    echo "Setup Complete!"
    echo "========================================"
    echo ""
    echo "Build tools installed at: ${UE_TOOLS_DIR}"
    echo ""
    echo "Next steps:"
    echo "1. Run './build_with_ue_tools.sh' to build Adastrea"
    echo "2. Or use the build tools directly with UnrealBuildTool"
    echo ""
    echo "Note: This contains only build tools, not the full engine."
    echo "For development/editing, install Unreal Engine 5.6 normally."
}

# Run main function
main "$@"
