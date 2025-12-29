@echo off
REM Script to build Adastrea using installed Unreal Engine 5.6
REM Detects UE 5.6 installation and uses its build tools

setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set PROJECT_FILE=%SCRIPT_DIR%Adastrea.uproject
set BUILD_CONFIG=%1
set PLATFORM=%2

REM Set defaults if not specified
if "%BUILD_CONFIG%"=="" set BUILD_CONFIG=Development
if "%PLATFORM%"=="" set PLATFORM=Win64

echo ========================================
echo Adastrea - Build with Unreal Engine 5.6
echo ========================================
echo.

REM Validate project file exists
if not exist "%PROJECT_FILE%" (
    echo ERROR: Project file not found at: %PROJECT_FILE%
    exit /b 1
)

REM Detect Unreal Engine 5.6 installation
call :detect_ue_install
if errorlevel 1 exit /b 1

echo ✓ Using Unreal Engine at: %UE_ROOT%
echo.

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
if exist "%UE_ROOT%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" (
    set UBT_PATH=%UE_ROOT%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll
) else if exist "%UE_ROOT%\Engine\Binaries\DotNET\UnrealBuildTool.dll" (
    set UBT_PATH=%UE_ROOT%\Engine\Binaries\DotNET\UnrealBuildTool.dll
)

if "%UBT_PATH%"=="" (
    echo ERROR: UnrealBuildTool not found in UE installation
    echo Expected at: %UE_ROOT%\Engine\Binaries\DotNET\
    exit /b 1
)

echo ✓ UnrealBuildTool found: %UBT_PATH%

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

endlocal
exit /b 0

REM ============================================
REM Function: Detect Unreal Engine 5.6 installation
REM ============================================
:detect_ue_install
echo Detecting Unreal Engine 5.6 installation...

REM Check environment variable first
if not "%UE5_ROOT%"=="" (
    if exist "%UE5_ROOT%\Engine\Binaries\DotNET" (
        set UE_ROOT=%UE5_ROOT%
        echo ✓ Found UE via UE5_ROOT environment variable
        exit /b 0
    ) else (
        echo WARNING: UE5_ROOT is set to "%UE5_ROOT%" but does not contain a valid UE installation
        echo Expected to find: %UE5_ROOT%\Engine\Binaries\DotNET
        echo Continuing search in other locations...
        echo.
    )
)

if not "%UE_ROOT%"=="" (
    if exist "%UE_ROOT%\Engine\Binaries\DotNET" (
        echo ✓ Found UE via UE_ROOT environment variable
        exit /b 0
    ) else (
        echo WARNING: UE_ROOT is set to "%UE_ROOT%" but does not contain a valid UE installation
        echo Expected to find: %UE_ROOT%\Engine\Binaries\DotNET
        echo Continuing search in other locations...
        echo.
    )
)

REM Check standard Epic Games Launcher installation paths
set EPIC_GAMES_DIR=C:\Program Files\Epic Games

REM Try exact 5.6 version first
if exist "%EPIC_GAMES_DIR%\UE_5.6\Engine\Binaries\DotNET" (
    set UE_ROOT=%EPIC_GAMES_DIR%\UE_5.6
    echo ✓ Found UE 5.6 at: %UE_ROOT%
    exit /b 0
)

REM Try without underscore
if exist "%EPIC_GAMES_DIR%\UE5.6\Engine\Binaries\DotNET" (
    set UE_ROOT=%EPIC_GAMES_DIR%\UE5.6
    echo ✓ Found UE 5.6 at: %UE_ROOT%
    exit /b 0
)

REM Try 5.6.0
if exist "%EPIC_GAMES_DIR%\UE_5.6.0\Engine\Binaries\DotNET" (
    set UE_ROOT=%EPIC_GAMES_DIR%\UE_5.6.0
    echo ✓ Found UE 5.6.0 at: %UE_ROOT%
    exit /b 0
)

REM Check registry for custom installation path
for /f "tokens=2*" %%a in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\EpicGames\Unreal Engine\5.6" /v "InstalledDirectory" 2^>nul') do (
    set UE_ROOT=%%b
    if exist "!UE_ROOT!\Engine\Binaries\DotNET" (
        echo ✓ Found UE 5.6 via registry at: !UE_ROOT!
        exit /b 0
    )
)

REM Also check HKEY_CURRENT_USER
for /f "tokens=2*" %%a in ('reg query "HKEY_CURRENT_USER\SOFTWARE\EpicGames\Unreal Engine\5.6" /v "InstalledDirectory" 2^>nul') do (
    set UE_ROOT=%%b
    if exist "!UE_ROOT!\Engine\Binaries\DotNET" (
        echo ✓ Found UE 5.6 via registry at: !UE_ROOT!
        exit /b 0
    )
)

echo.
echo ERROR: Unreal Engine 5.6 installation not found
echo.
echo Searched locations:
echo   - Environment variables: UE5_ROOT, UE_ROOT
echo   - %EPIC_GAMES_DIR%\UE_5.6
echo   - %EPIC_GAMES_DIR%\UE5.6
echo   - %EPIC_GAMES_DIR%\UE_5.6.0
echo   - Windows Registry (HKLM and HKCU)
echo.
echo Please install Unreal Engine 5.6 or set UE5_ROOT environment variable.
echo.
exit /b 1

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
dotnet "%UBT_PATH%" AdastreaEditor %PLATFORM% %BUILD_CONFIG% -Project="%PROJECT_FILE%" -Progress -NoHotReloadFromIDE
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
