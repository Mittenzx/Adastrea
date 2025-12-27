@echo off
REM Script to download and setup only Unreal Engine build tools (not the full engine)
REM This includes UnrealBuildTool, UnrealHeaderTool, and necessary build scripts
REM Windows version of setup_ue_build_tools.sh

setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set UE_TOOLS_DIR=%SCRIPT_DIR%UnrealBuildTools
set UE_VERSION=5.6
set UE_REPO=https://github.com/EpicGames/UnrealEngine.git

echo ========================================
echo Adastrea - Unreal Build Tools Setup
echo ========================================
echo.
echo This script downloads ONLY the build tools from Unreal Engine
echo Not the full engine source (~500MB vs ~50GB)
echo.

REM Check if git is available
where git >nul 2>&1
if errorlevel 1 (
    echo ERROR: git is not installed or not in PATH
    echo Please install Git for Windows from: https://git-scm.com/download/win
    exit /b 1
)

REM Function to clone sparse checkout of UnrealEngine (build tools only)
if exist "%UE_TOOLS_DIR%" (
    echo UnrealBuildTools directory already exists at: %UE_TOOLS_DIR%
    
    REM Non-interactive mode check
    if defined CI (
        echo Non-interactive mode detected; validating existing UnrealBuildTools directory before removal...
        if exist "%UE_TOOLS_DIR%\Engine\Build\BatchFiles" (
            echo Verified Unreal Engine build tools directory structure. Removing existing directory...
            rmdir /s /q "%UE_TOOLS_DIR%"
            goto :clone_tools
        ) else (
            echo ERROR: Existing directory does not appear to be a valid Unreal Build Tools checkout.
            echo ERROR: Skipping automatic deletion in CI mode for safety.
            exit /b 1
        )
    )
    
    set /p "REPLY=Remove and re-download? (y/N): "
    if /i "!REPLY!"=="y" (
        echo Removing existing directory...
        rmdir /s /q "%UE_TOOLS_DIR%"
        goto :clone_tools
    ) else (
        echo Using existing UnrealBuildTools directory.
        goto :setup_tools
    )
)

:clone_tools
echo Creating sparse checkout directory...
mkdir "%UE_TOOLS_DIR%"
cd /d "%UE_TOOLS_DIR%"

REM Initialize git repository with sparse checkout
git init
git config core.sparseCheckout true

REM Add remote
echo Adding UnrealEngine remote...
git remote add origin "%UE_REPO%"

REM Configure sparse checkout to get only build tools
echo Configuring sparse checkout (build tools only)...
echo # Build tools and scripts> .git\info\sparse-checkout
echo Engine/Build/>> .git\info\sparse-checkout
echo Engine/Binaries/DotNET/>> .git\info\sparse-checkout
echo Engine/Binaries/ThirdParty/DotNet/>> .git\info\sparse-checkout
echo Engine/Source/Programs/UnrealBuildTool/>> .git\info\sparse-checkout
echo Engine/Source/Programs/Shared/>> .git\info\sparse-checkout
echo(>> .git\info\sparse-checkout
echo # Minimal config files needed for build>> .git\info\sparse-checkout
echo Engine/Config/BaseEngine.ini>> .git\info\sparse-checkout
echo Engine/Config/BasePlatforms.ini>> .git\info\sparse-checkout

REM Fetch only the specified paths
echo.
echo Downloading build tools (this may take a few minutes)...
echo Size: ~500MB instead of ~50GB
echo.

git fetch --depth 1 origin %UE_VERSION%

if errorlevel 1 (
    echo.
    echo ERROR: Failed to fetch from UnrealEngine repository.
    echo.
    echo This typically happens because:
    echo 1. You haven't linked your GitHub account to your Epic Games account
    echo 2. You haven't accepted the Epic Games organization invitation
    echo.
    echo Please follow these steps:
    echo 1. Go to https://www.epicgames.com/account/connections
    echo 2. Link your GitHub account
    echo 3. Accept the invitation to join the Epic Games GitHub organization
    echo 4. Verify access at https://github.com/EpicGames
    echo.
    cd /d "%SCRIPT_DIR%"
    rmdir /s /q "%UE_TOOLS_DIR%"
    exit /b 1
)

git checkout FETCH_HEAD

cd /d "%SCRIPT_DIR%"

echo.
echo ✓ Build tools downloaded successfully!
echo.

:setup_tools
echo.
echo Setting up build tools...
cd /d "%UE_TOOLS_DIR%"

REM Check for Setup script
if exist "Engine\Build\BatchFiles\Setup.bat" (
    echo Found Setup.bat for build tools
    REM Note: We might not need to run this for just build tools
)

cd /d "%SCRIPT_DIR%"

:verify_tools
echo.
echo Verifying build tools installation...

set ERRORS=0

REM Check for UnrealBuildTool
if exist "%UE_TOOLS_DIR%\Engine\Source\Programs\UnrealBuildTool" (
    echo ✓ UnrealBuildTool source found
) else (
    echo ✗ UnrealBuildTool source not found
    set /a ERRORS+=1
)

REM Check for DotNET binaries
if exist "%UE_TOOLS_DIR%\Engine\Binaries\DotNET" (
    echo ✓ DotNET binaries directory found
) else (
    echo ✗ DotNET binaries directory not found
    set /a ERRORS+=1
)

REM Check for build batch files
if exist "%UE_TOOLS_DIR%\Engine\Build\BatchFiles" (
    echo ✓ Build batch files found
) else (
    echo ✗ Build batch files not found
    set /a ERRORS+=1
)

if !ERRORS! gtr 0 (
    echo.
    echo WARNING: Some build tools are missing. Build may fail.
    exit /b 1
)

echo.
echo ✓ All essential build tools verified!

echo.
echo ========================================
echo Setup Complete!
echo ========================================
echo.
echo Build tools installed at: %UE_TOOLS_DIR%
echo.
echo Next steps:
echo 1. Run 'build_with_ue_tools.bat' to build Adastrea
echo 2. Or use the build tools directly with UnrealBuildTool
echo.
echo Note: This contains only build tools, not the full engine.
echo For development/editing, install Unreal Engine 5.6 normally.

endlocal
exit /b 0
