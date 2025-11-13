@echo off
REM Adastrea - Windows Build Script
REM This script builds the Adastrea project using UnrealBuildTool
REM Usage: build_windows.bat [clean|editor|game|shipping] [verbose]
REM Examples:
REM   build_windows.bat              - Build editor in Development
REM   build_windows.bat clean        - Clean build artifacts and rebuild
REM   build_windows.bat editor       - Build editor (same as default)
REM   build_windows.bat game         - Build standalone game
REM   build_windows.bat shipping     - Build shipping configuration
REM   build_windows.bat editor verbose - Build with verbose output

setlocal EnableDelayedExpansion

REM ============================================================================
REM Configuration - Modify these if your setup differs
REM ============================================================================

REM Unreal Engine installation path
set "UE_ROOT=C:\Program Files\Epic Games\UE_5.6"

REM Project settings (automatically determined)
set "PROJECT_ROOT=%~dp0"
set "PROJECT_FILE=%PROJECT_ROOT%Adastrea.uproject"

REM Default build settings
set "PLATFORM=Win64"
set "BUILD_CONFIG=Development"
set "TARGET=AdastreaEditor"
set "CLEAN_BUILD=0"
set "VERBOSE_OUTPUT=0"

REM ============================================================================
REM Parse Command Line Arguments
REM ============================================================================

:parse_args
if "%~1"=="" goto validate_setup
if /i "%~1"=="clean" (
    set "CLEAN_BUILD=1"
    shift
    goto parse_args
)
if /i "%~1"=="editor" (
    set "TARGET=AdastreaEditor"
    shift
    goto parse_args
)
if /i "%~1"=="game" (
    set "TARGET=Adastrea"
    shift
    goto parse_args
)
if /i "%~1"=="shipping" (
    set "BUILD_CONFIG=Shipping"
    set "TARGET=Adastrea"
    shift
    goto parse_args
)
if /i "%~1"=="development" (
    set "BUILD_CONFIG=Development"
    shift
    goto parse_args
)
if /i "%~1"=="debug" (
    set "BUILD_CONFIG=Debug"
    shift
    goto parse_args
)
if /i "%~1"=="verbose" (
    set "VERBOSE_OUTPUT=1"
    shift
    goto parse_args
)
if /i "%~1"=="help" goto show_help
if /i "%~1"=="-h" goto show_help
if /i "%~1"=="--help" goto show_help
if /i "%~1"=="/?" goto show_help

echo Warning: Unknown argument "%~1" - ignoring
shift
goto parse_args

REM ============================================================================
REM Validate Setup
REM ============================================================================

:validate_setup
echo ============================================================================
echo Adastrea Build Script - Windows
echo ============================================================================
echo.

REM Check if Unreal Engine exists
if not exist "%UE_ROOT%" (
    echo ERROR: Unreal Engine not found at: %UE_ROOT%
    echo.
    echo Please edit this script and set UE_ROOT to your Unreal Engine installation path.
    echo Common locations:
    echo   - C:\Program Files\Epic Games\UE_5.6
    echo   - C:\Program Files\Epic Games\UE_5.5
    echo   - D:\UnrealEngine\UE_5.6
    echo.
    exit /b 1
)

REM Check if project file exists
if not exist "%PROJECT_FILE%" (
    echo ERROR: Project file not found: %PROJECT_FILE%
    echo.
    echo Please run this script from the Adastrea project root directory.
    echo.
    exit /b 1
)

REM Check if Build.bat exists
set "BUILD_TOOL=%UE_ROOT%\Engine\Build\BatchFiles\Build.bat"
if not exist "%BUILD_TOOL%" (
    echo ERROR: Build.bat not found at: %BUILD_TOOL%
    echo.
    echo Your Unreal Engine installation may be corrupted.
    echo Try verifying your installation in Epic Games Launcher.
    echo.
    exit /b 1
)

echo Configuration:
echo   Unreal Engine: %UE_ROOT%
echo   Project: %PROJECT_FILE%
echo   Target: %TARGET%
echo   Platform: %PLATFORM%
echo   Configuration: %BUILD_CONFIG%
echo   Clean Build: %CLEAN_BUILD%
echo   Verbose: %VERBOSE_OUTPUT%
echo.

REM ============================================================================
REM Clean Build Artifacts (if requested)
REM ============================================================================

if "%CLEAN_BUILD%"=="1" (
    echo Cleaning build artifacts...
    echo.
    
    if exist "%PROJECT_ROOT%Binaries" (
        echo   Deleting Binaries...
        rmdir /s /q "%PROJECT_ROOT%Binaries" 2>nul
    )
    
    if exist "%PROJECT_ROOT%Intermediate" (
        echo   Deleting Intermediate...
        rmdir /s /q "%PROJECT_ROOT%Intermediate" 2>nul
    )
    
    if exist "%PROJECT_ROOT%Saved\Logs" (
        echo   Deleting logs...
        rmdir /s /q "%PROJECT_ROOT%Saved\Logs" 2>nul
    )
    
    if exist "%PROJECT_ROOT%.vs" (
        echo   Deleting Visual Studio cache...
        rmdir /s /q "%PROJECT_ROOT%.vs" 2>nul
    )
    
    echo   Clean completed.
    echo.
)

REM ============================================================================
REM Build Project
REM ============================================================================

echo ============================================================================
echo Building %TARGET% for %PLATFORM% in %BUILD_CONFIG% configuration...
echo ============================================================================
echo.

REM Construct build command
set "BUILD_CMD=%BUILD_TOOL% %TARGET% %PLATFORM% %BUILD_CONFIG% -Project=%PROJECT_FILE%"

REM Add optional flags
if "%VERBOSE_OUTPUT%"=="1" (
    set "BUILD_CMD=!BUILD_CMD! -Verbose"
)
set "BUILD_CMD=!BUILD_CMD! -Progress"

REM Display command
echo Command: !BUILD_CMD!
echo.

REM Capture start time
set START_TIME=%TIME%

REM Execute build
call !BUILD_CMD!
set BUILD_RESULT=%ERRORLEVEL%

REM Capture end time
set END_TIME=%TIME%

echo.
echo ============================================================================

REM ============================================================================
REM Report Results
REM ============================================================================

if %BUILD_RESULT% NEQ 0 (
    echo BUILD FAILED with error code %BUILD_RESULT%
    echo ============================================================================
    echo.
    echo Troubleshooting:
    echo   1. Check the error messages above for specific issues
    echo   2. Try running: build_windows.bat clean
    echo   3. Check BUILD_TROUBLESHOOTING.md for common solutions
    echo   4. Run with verbose: build_windows.bat %1 verbose
    echo.
    echo Build logs are in: %PROJECT_ROOT%Saved\Logs\
    echo.
    exit /b %BUILD_RESULT%
) else (
    echo BUILD SUCCEEDED
    echo ============================================================================
    echo.
    echo Build Time: Started at %START_TIME%, ended at %END_TIME%
    echo.
    if "%TARGET%"=="AdastreaEditor" (
        echo You can now open the project in Unreal Editor:
        echo   "%PROJECT_FILE%"
    ) else (
        echo Standalone game executable built in:
        echo   "%PROJECT_ROOT%Binaries\%PLATFORM%\"
    )
    echo.
    echo Build logs are in: %PROJECT_ROOT%Saved\Logs\
    echo.
    exit /b 0
)

REM ============================================================================
REM Help Display
REM ============================================================================

:show_help
echo Adastrea Build Script - Windows
echo.
echo Usage: build_windows.bat [options]
echo.
echo Options:
echo   clean          - Clean build artifacts before building
echo   editor         - Build editor (default)
echo   game           - Build standalone game
echo   shipping       - Build in Shipping configuration (optimized, no debug)
echo   development    - Build in Development configuration (default)
echo   debug          - Build in Debug configuration (full debug symbols)
echo   verbose        - Show detailed build output
echo   help, -h, /?   - Show this help message
echo.
echo Examples:
echo   build_windows.bat
echo     Builds the editor in Development configuration
echo.
echo   build_windows.bat clean
echo     Deletes build artifacts and rebuilds the editor
echo.
echo   build_windows.bat game
echo     Builds the standalone game in Development configuration
echo.
echo   build_windows.bat shipping
echo     Builds the standalone game in Shipping configuration (release)
echo.
echo   build_windows.bat editor verbose
echo     Builds the editor with detailed output for debugging
echo.
echo   build_windows.bat clean shipping verbose
echo     Clean build of shipping game with verbose output
echo.
echo Configuration:
echo   Unreal Engine Path: %UE_ROOT%
echo   Project File: %PROJECT_FILE%
echo.
echo For more information, see UNREAL_BUILD_GUIDE.md
echo.
exit /b 0
