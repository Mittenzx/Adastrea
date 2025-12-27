@echo off
REM Script to build Adastrea using Unreal Engine Build Tools
REM Modified to use installed Unreal Engine instead of UnrealBuildTools directory
REM Windows version

setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
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

REM Validate project file exists
if not exist "%PROJECT_FILE%" (
    echo ERROR: Project file not found at: %PROJECT_FILE%
    exit /b 1
)

REM Try to find Unreal Engine installation
echo Searching for Unreal Engine 5.6 installation...
set UE_ROOT=

REM Check common installation locations
if exist "C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" (
    set UE_ROOT=C:\Program Files\Epic Games\UE_5.6
    echo ✓ Found UE 5.6 at: !UE_ROOT!
) else if exist "C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\Build.bat" (
    set UE_ROOT=C:\Program Files\Epic Games\UE_5.5
    echo ✓ Found UE 5.5 at: !UE_ROOT! (using compatible version)
) else if exist "C:\Program Files (x86)\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" (
    set UE_ROOT=C:\Program Files (x86)\Epic Games\UE_5.6
    echo ✓ Found UE 5.6 at: !UE_ROOT!
) else (
    REM Try to read from registry or engine association
    echo WARNING: Could not auto-detect Unreal Engine installation
    echo.
    echo Please specify your Unreal Engine installation path.
    echo Example: C:\Program Files\Epic Games\UE_5.6
    echo.
    set /p UE_ROOT="Enter UE path (or press Ctrl+C to cancel): "
    
    if "!UE_ROOT!"=="" (
        echo ERROR: No Unreal Engine path provided
        exit /b 1
    )
    
    if not exist "!UE_ROOT!\Engine\Build\BatchFiles\Build.bat" (
        echo ERROR: Invalid Unreal Engine path - Build.bat not found
        exit /b 1
    )
)

REM Set paths to UE tools
set UBT_PATH=%UE_ROOT%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe
set BUILD_BAT=%UE_ROOT%\Engine\Build\BatchFiles\Build.bat
set RUNGEN_BAT=%UE_ROOT%\Engine\Build\BatchFiles\RunUBT.bat

REM Check for .NET SDK (needed for some operations)
echo Checking for .NET SDK...
where dotnet >nul 2>&1
if errorlevel 1 (
    echo WARNING: .NET SDK not found (may be required for some operations)
    echo Install from: https://dotnet.microsoft.com/download/dotnet/6.0
    echo.
) else (
    for /f "tokens=*" %%i in ('dotnet --version') do set DOTNET_VERSION=%%i
    echo ✓ .NET SDK found: !DOTNET_VERSION!
)

REM Validate UnrealBuildTool exists
if not exist "%UBT_PATH%" (
    echo ERROR: UnrealBuildTool not found at: %UBT_PATH%
    echo.
    echo Your Unreal Engine installation may be incomplete.
    echo Please verify your installation or reinstall Unreal Engine.
    exit /b 1
)

echo ✓ UnrealBuildTool found: %UBT_PATH%
echo.

REM Generate project files
call :generate_project_files
if errorlevel 1 (
    echo WARNING: Project file generation had issues
)

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
echo To run the editor:
echo   - Open Adastrea.uproject directly, or
echo   - Run: "%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe" "%PROJECT_FILE%"

endlocal
exit /b 0

REM ============================================
REM Function: Generate project files
REM ============================================
:generate_project_files
echo Generating project files...

"%UBT_PATH%" -projectfiles -project="%PROJECT_FILE%" -game -rocket -progress

if not errorlevel 1 (
    echo ✓ Project files generated
) else (
    echo WARNING: Project file generation had issues
    echo This may be okay if project files already exist
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

echo Using Build.bat: %BUILD_BAT%
echo Project: %PROJECT_FILE%
echo.

REM Build using the standard Unreal Build.bat
call "%BUILD_BAT%" AdastreaEditor %PLATFORM% %BUILD_CONFIG% -Project="%PROJECT_FILE%" -WaitMutex -FromMsBuild

if errorlevel 1 (
    echo.
    echo ✗ Build failed
    echo.
    echo Common issues:
    echo   - Ensure Visual Studio 2022 is installed with C++ tools
    echo   - Check that Windows SDK is installed
    echo   - Verify .vsconfig requirements
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
    if exist "%SCRIPT_DIR%Binaries\%PLATFORM%" (
        echo   Contents:
        dir /b "%SCRIPT_DIR%Binaries\%PLATFORM%" 2>nul
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
) else if exist "%SCRIPT_DIR%Binaries\%PLATFORM%\UnrealEditor-Adastrea-Win64-Development.dll" (
    echo ✓ Adastrea module binary found (Development)
) else (
    echo ⚠ Adastrea module binary not found at expected location
)

echo.
if "%SUCCESS%"=="true" (
    echo Build validation: SUCCESS
    exit /b 0
) else (
    echo Build validation: PARTIAL (some warnings)
    exit /b 0
)
