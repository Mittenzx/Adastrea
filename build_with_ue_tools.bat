@echo off
REM Script to build Adastrea using Unreal Build Tools (UBT) only
REM This uses the minimal build tools instead of full engine source
REM Windows version of build_with_ue_tools.sh

setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set UE_TOOLS_DIR=%SCRIPT_DIR%UnrealBuildTools
set PROJECT_FILE=%SCRIPT_DIR%Adastrea.uproject
set BUILD_CONFIG=%1
set PLATFORM=%2

REM Set defaults if not specified
if "%BUILD_CONFIG%"=="" set BUILD_CONFIG=Development
if "%PLATFORM%"=="" set PLATFORM=Win64

echo ========================================
echo Adastrea - Build with UE Build Tools
echo ========================================
echo.

REM Validate UnrealBuildTools directory exists
if not exist "%UE_TOOLS_DIR%" (
    echo ERROR: UnrealBuildTools directory not found at: %UE_TOOLS_DIR%
    echo Please run setup_ue_build_tools.bat first to download the build tools.
    exit /b 1
)

REM Validate project file exists
if not exist "%PROJECT_FILE%" (
    echo ERROR: Project file not found at: %PROJECT_FILE%
    exit /b 1
)

REM Check for .NET SDK
echo Checking for .NET SDK...
where dotnet >nul 2>&1
if errorlevel 1 (
    echo ERROR: .NET SDK not found
    echo.
    echo UnrealBuildTool requires .NET SDK to run.
    echo Install it from: https://dotnet.microsoft.com/download/dotnet/6.0
    echo.
    exit /b 1
)

for /f "tokens=*" %%i in ('dotnet --version') do set DOTNET_VERSION=%%i
echo ✓ .NET SDK found: %DOTNET_VERSION%

REM Function to find UnrealBuildTool
set UBT_PATH=
if exist "%UE_TOOLS_DIR%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" (
    set UBT_PATH=%UE_TOOLS_DIR%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll
) else if exist "%UE_TOOLS_DIR%\Engine\Binaries\DotNET\UnrealBuildTool.dll" (
    set UBT_PATH=%UE_TOOLS_DIR%\Engine\Binaries\DotNET\UnrealBuildTool.dll
)

if "%UBT_PATH%"=="" (
    echo.
    echo UnrealBuildTool not found, building from source...
    call :build_ubt
    if errorlevel 1 exit /b 1
) else (
    echo ✓ UnrealBuildTool already built: %UBT_PATH%
)

REM Generate project files
call :generate_project_files
if errorlevel 1 echo WARNING: Project file generation had issues

REM Build Adastrea project
call :build_project
if errorlevel 1 exit /b 1

REM Validate build
call :validate_build

echo.
echo ========================================
echo Build Complete!
echo ========================================
echo.
echo Build output: %SCRIPT_DIR%Binaries\%PLATFORM%\
echo.
echo Note: To run the editor, you still need a full Unreal Engine 5.6 installation.
echo These build tools are for compilation only.

endlocal
exit /b 0

REM ============================================
REM Function: Build UnrealBuildTool from source
REM ============================================
:build_ubt
echo.
echo Checking if UnrealBuildTool needs to be built...

set UBT_SOURCE=%UE_TOOLS_DIR%\Engine\Source\Programs\UnrealBuildTool
set UBT_CSPROJ=%UBT_SOURCE%\UnrealBuildTool.csproj

if not exist "%UBT_CSPROJ%" (
    echo ERROR: UnrealBuildTool.csproj not found at: %UBT_CSPROJ%
    echo Build tools may not have been downloaded correctly.
    exit /b 1
)

echo Building UnrealBuildTool...
pushd "%UBT_SOURCE%"

dotnet build UnrealBuildTool.csproj -c Development

if errorlevel 1 (
    echo ERROR: Failed to build UnrealBuildTool
    popd
    exit /b 1
)

popd
echo ✓ UnrealBuildTool built successfully

REM Find the built UBT
if exist "%UE_TOOLS_DIR%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" (
    set UBT_PATH=%UE_TOOLS_DIR%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll
) else if exist "%UE_TOOLS_DIR%\Engine\Binaries\DotNET\UnrealBuildTool.dll" (
    set UBT_PATH=%UE_TOOLS_DIR%\Engine\Binaries\DotNET\UnrealBuildTool.dll
)

if "%UBT_PATH%"=="" (
    echo ERROR: UnrealBuildTool not found after building
    exit /b 1
)

exit /b 0

REM ============================================
REM Function: Generate project files
REM ============================================
:generate_project_files
echo.
echo Generating project files...

if "%UBT_PATH%"=="" (
    echo ERROR: UnrealBuildTool not found
    exit /b 1
)

echo Using UnrealBuildTool at: %UBT_PATH%

dotnet "%UBT_PATH%" -projectfiles -project="%PROJECT_FILE%" -game -rocket -progress

if not errorlevel 1 (
    echo ✓ Project files generated
) else (
    echo WARNING: Project file generation had issues
)

exit /b 0

REM ============================================
REM Function: Build Adastrea project
REM ============================================
:build_project
echo.
echo Building Adastrea project...
echo Configuration: %BUILD_CONFIG%Editor
echo Platform: %PLATFORM%
echo.

if "%UBT_PATH%"=="" (
    echo ERROR: UnrealBuildTool not found
    exit /b 1
)

echo Using UnrealBuildTool at: %UBT_PATH%
echo Project: %PROJECT_FILE%
echo.

REM Build the project
dotnet "%UBT_PATH%" Adastrea %PLATFORM% %BUILD_CONFIG%Editor -Project="%PROJECT_FILE%" -Progress -NoHotReloadFromIDE

if errorlevel 1 (
    echo.
    echo ✗ Build failed
    exit /b 1
)

echo.
echo ✓ Adastrea built successfully!
exit /b 0

REM ============================================
REM Function: Validate build output
REM ============================================
:validate_build
echo.
echo Validating build output...

set SUCCESS=true

REM Check if binaries were created
if exist "%SCRIPT_DIR%Binaries" (
    echo ✓ Binaries directory created
    echo   Contents:
    dir /b "%SCRIPT_DIR%Binaries\%PLATFORM%" 2>nul | findstr "." >nul
    if errorlevel 1 (
        echo   (No platform-specific binaries yet - may be in subdirectory)
    )
) else (
    echo ✗ WARNING: Binaries directory not found
    set SUCCESS=false
)

REM Check if intermediate files were created
if exist "%SCRIPT_DIR%Intermediate" (
    echo ✓ Intermediate directory created
) else (
    echo ✗ WARNING: Intermediate directory not found
    set SUCCESS=false
)

REM Check for specific module binaries
if exist "%SCRIPT_DIR%Binaries\%PLATFORM%\UnrealEditor-Adastrea.dll" (
    echo ✓ Adastrea module binary found
) else (
    echo ⚠ Adastrea module binary not found (may be in subdirectory)
)

echo.
if "%SUCCESS%"=="true" (
    echo Build validation: SUCCESS
    exit /b 0
) else (
    echo Build validation: PARTIAL (some warnings)
    exit /b 1
)
