#!/bin/bash
# Script to build Adastrea using Unreal Build Tools (UBT) only
# This uses the minimal build tools instead of full engine source

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
UE_TOOLS_DIR="${SCRIPT_DIR}/UnrealBuildTools"
PROJECT_FILE="${SCRIPT_DIR}/Adastrea.uproject"
BUILD_CONFIG="${1:-Development}"
PLATFORM="${2:-Linux}"

echo "========================================"
echo "Adastrea - Build with UE Build Tools"
echo "========================================"
echo ""

# Validate UnrealBuildTools directory exists
if [ ! -d "${UE_TOOLS_DIR}" ]; then
    echo "ERROR: UnrealBuildTools directory not found at: ${UE_TOOLS_DIR}"
    echo "Please run setup_ue_build_tools.sh first to download the build tools."
    exit 1
fi

# Validate project file exists
if [ ! -f "${PROJECT_FILE}" ]; then
    echo "ERROR: Project file not found at: ${PROJECT_FILE}"
    exit 1
fi

# Function to find UnrealBuildTool
find_ubt() {
    local UBT_PATHS=(
        "${UE_TOOLS_DIR}/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.dll"
        "${UE_TOOLS_DIR}/Engine/Binaries/DotNET/UnrealBuildTool.dll"
        "${UE_TOOLS_DIR}/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool"
        "${UE_TOOLS_DIR}/Engine/Binaries/DotNET/UnrealBuildTool"
    )
    
    for path in "${UBT_PATHS[@]}"; do
        if [ -f "${path}" ]; then
            echo "${path}"
            return 0
        fi
    done
    
    return 1
}

# Function to check .NET SDK
check_dotnet() {
    echo "Checking for .NET SDK..."
    
    if ! command -v dotnet &> /dev/null; then
        echo "ERROR: .NET SDK not found"
        echo ""
        echo "UnrealBuildTool requires .NET SDK to run."
        echo "Install it with:"
        echo "  Ubuntu/Debian: sudo apt-get install dotnet-sdk-6.0"
        echo "  Mac: brew install dotnet-sdk"
        echo ""
        exit 1
    fi
    
    local dotnet_version=$(dotnet --version)
    echo "✓ .NET SDK found: ${dotnet_version}"
}

# Function to build UnrealBuildTool if needed
build_ubt() {
    echo ""
    echo "Checking if UnrealBuildTool needs to be built..."
    
    local UBT_SOURCE="${UE_TOOLS_DIR}/Engine/Source/Programs/UnrealBuildTool"
    local UBT_CSPROJ="${UBT_SOURCE}/UnrealBuildTool.csproj"
    
    if [ ! -f "${UBT_CSPROJ}" ]; then
        echo "ERROR: UnrealBuildTool.csproj not found at: ${UBT_CSPROJ}"
        echo "Build tools may not have been downloaded correctly."
        exit 1
    fi
    
    # Check if UBT binary already exists
    local UBT_PATH=$(find_ubt) || true
    
    if [ -n "${UBT_PATH}" ] && [ -f "${UBT_PATH}" ]; then
        echo "✓ UnrealBuildTool already built: ${UBT_PATH}"
        return 0
    fi
    
    echo "Building UnrealBuildTool..."
    cd "${UBT_SOURCE}"
    
    # Build UBT
    dotnet build UnrealBuildTool.csproj -c Development
    
    if [ $? -ne 0 ]; then
        echo "ERROR: Failed to build UnrealBuildTool"
        exit 1
    fi
    
    cd "${SCRIPT_DIR}"
    echo "✓ UnrealBuildTool built successfully"
}

# Function to generate project files
generate_project_files() {
    echo ""
    echo "Generating project files..."
    
    local UBT_PATH=$(find_ubt)
    
    if [ -z "${UBT_PATH}" ]; then
        echo "ERROR: UnrealBuildTool not found"
        exit 1
    fi
    
    echo "Using UnrealBuildTool at: ${UBT_PATH}"
    
    dotnet "${UBT_PATH}" -projectfiles -project="${PROJECT_FILE}" -game -rocket -progress
    
    if [ $? -eq 0 ]; then
        echo "✓ Project files generated"
    else
        echo "WARNING: Project file generation had issues"
    fi
}

# Function to build Adastrea project
build_project() {
    echo ""
    echo "Building Adastrea project..."
    echo "Configuration: ${BUILD_CONFIG}Editor"
    echo "Platform: ${PLATFORM}"
    echo ""
    
    local UBT_PATH=$(find_ubt)
    
    if [ -z "${UBT_PATH}" ]; then
        echo "ERROR: UnrealBuildTool not found"
        exit 1
    fi
    
    echo "Using UnrealBuildTool at: ${UBT_PATH}"
    echo "Project: ${PROJECT_FILE}"
    echo ""
    
    # Build the project
    dotnet "${UBT_PATH}" AdastreaEditor ${PLATFORM} ${BUILD_CONFIG} -Project="${PROJECT_FILE}" -Progress -NoHotReloadFromIDE
    
    if [ $? -eq 0 ]; then
        echo ""
        echo "✓ Adastrea built successfully!"
    else
        echo ""
        echo "✗ Build failed"
        exit 1
    fi
}

# Function to validate build output
validate_build() {
    echo ""
    echo "Validating build output..."
    
    local success=true
    
    # Check if binaries were created
    if [ -d "${SCRIPT_DIR}/Binaries" ]; then
        echo "✓ Binaries directory created"
        echo "  Contents:"
        ls -lh "${SCRIPT_DIR}/Binaries/" | head -10
    else
        echo "✗ WARNING: Binaries directory not found"
        success=false
    fi
    
    # Check if intermediate files were created
    if [ -d "${SCRIPT_DIR}/Intermediate" ]; then
        echo "✓ Intermediate directory created"
    else
        echo "✗ WARNING: Intermediate directory not found"
        success=false
    fi
    
    # Check for specific module binaries
    if [ -f "${SCRIPT_DIR}/Binaries/${PLATFORM}/libUnrealEditor-Adastrea.so" ] || \
       [ -f "${SCRIPT_DIR}/Binaries/${PLATFORM}/UnrealEditor-Adastrea.dylib" ]; then
        echo "✓ Adastrea module binary found"
    else
        echo "⚠ Adastrea module binary not found (may be in subdirectory)"
    fi
    
    echo ""
    if [ "$success" = true ]; then
        echo "Build validation: SUCCESS"
        return 0
    else
        echo "Build validation: PARTIAL (some warnings)"
        return 1
    fi
}

# Main execution
main() {
    echo "Build configuration: ${BUILD_CONFIG}"
    echo "Platform: ${PLATFORM}"
    echo "Build tools path: ${UE_TOOLS_DIR}"
    echo "Project file: ${PROJECT_FILE}"
    echo ""
    
    # Check prerequisites
    check_dotnet
    
    # Build UnrealBuildTool if needed
    build_ubt
    
    # Generate project files
    generate_project_files
    
    # Build Adastrea project
    build_project
    
    # Validate build
    validate_build
    
    echo ""
    echo "========================================"
    echo "Build Complete!"
    echo "========================================"
    echo ""
    echo "Build output: ${SCRIPT_DIR}/Binaries/${PLATFORM}/"
    echo ""
    echo "Note: To run the editor, you still need a full Unreal Engine 5.6 installation."
    echo "These build tools are for compilation only."
}

# Run main function
main "$@"
