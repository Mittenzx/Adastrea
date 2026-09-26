@echo off
REM ============================================================================
REM  Adastrea - Win64 packaged build (BuildCookRun wrapper)
REM
REM  Usage:   Tools\package_win64.bat [Development^|DebugGame^|Shipping] [extra UAT args...]
REM  Example: Tools\package_win64.bat
REM           Tools\package_win64.bat Shipping
REM           Tools\package_win64.bat Development -iterate
REM
REM  Env overrides:
REM    UE_ROOT      Engine install root (default: C:\Program Files\Epic Games\UE_5.8)
REM    ARCHIVE_DIR  Output folder      (default: <project>\Saved\Packaged)
REM
REM  The cook runs UnrealEditor-Cmd, so close every UnrealEditor instance on this
REM  project first. Output: %ARCHIVE_DIR%\Windows\Adastrea.exe
REM  Log:    <project>\Saved\Logs\Package_Win64_<config>.log (plus UAT's own logs
REM          under %UE_ROOT%\Engine\Programs\AutomationTool\Saved\Logs).
REM  See docs\09-SETUP_GUIDES\PACKAGING.md.
REM ============================================================================
setlocal EnableExtensions

set "PROJECT_DIR=%~dp0.."
for %%I in ("%PROJECT_DIR%") do set "PROJECT_DIR=%%~fI"
set "UPROJECT=%PROJECT_DIR%\Adastrea.uproject"

if "%UE_ROOT%"=="" set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "RUNUAT=%UE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat"

set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Development"
if not "%~1"=="" shift

set "EXTRA="
:collect_args
if "%~1"=="" goto args_done
set "EXTRA=%EXTRA% %1"
shift
goto collect_args
:args_done

if "%ARCHIVE_DIR%"=="" set "ARCHIVE_DIR=%PROJECT_DIR%\Saved\Packaged"
set "LOG_DIR=%PROJECT_DIR%\Saved\Logs"
set "LOG_FILE=%LOG_DIR%\Package_Win64_%CONFIG%.log"

if not exist "%UPROJECT%" (
    echo [package_win64] ERROR: %UPROJECT% not found.
    exit /b 2
)
if not exist "%RUNUAT%" (
    echo [package_win64] ERROR: RunUAT not found at "%RUNUAT%". Set UE_ROOT.
    exit /b 2
)

REM Warn only if an editor has THIS project open (editors on other checkouts are fine).
powershell -NoProfile -Command "if (Get-CimInstance Win32_Process -Filter \"Name like 'UnrealEditor%%'\" | Where-Object { $_.CommandLine -like '*%UPROJECT%*' }) { exit 1 } else { exit 0 }" >nul 2>&1
if errorlevel 1 (
    echo [package_win64] WARNING: an UnrealEditor has %UPROJECT% open. The cook may
    echo [package_win64]          fail or contend for files. Close that editor first.
)

if not exist "%LOG_DIR%" mkdir "%LOG_DIR%"

echo [package_win64] Project : %UPROJECT%
echo [package_win64] Config  : %CONFIG%
echo [package_win64] Archive : %ARCHIVE_DIR%
echo [package_win64] Log     : %LOG_FILE%
echo [package_win64] Started : %DATE% %TIME%

call "%RUNUAT%" BuildCookRun ^
    -project="%UPROJECT%" ^
    -noP4 ^
    -platform=Win64 ^
    -clientconfig=%CONFIG% ^
    -build ^
    -cook ^
    -stage ^
    -pak ^
    -archive ^
    -archivedirectory="%ARCHIVE_DIR%" ^
    -unattended ^
    -utf8output %EXTRA% > "%LOG_FILE%" 2>&1
set "RC=%ERRORLEVEL%"

echo [package_win64] Finished: %DATE% %TIME%  (exit code %RC%)
if not "%RC%"=="0" (
    echo [package_win64] FAILED. Errors from the log:
    findstr /I /C:"error:" /C:"Error:" /C:"BUILD FAILED" /C:"AutomationTool exiting with ExitCode" "%LOG_FILE%"
    exit /b %RC%
)
echo [package_win64] OK: %ARCHIVE_DIR%\Windows\Adastrea.exe
exit /b 0
