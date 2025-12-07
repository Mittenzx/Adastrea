@echo off
REM AdastreaDirector Quick Setup Launcher
REM This script launches the interactive setup assistant

echo.
echo ============================================================
echo   AdastreaDirector - Quick Setup Launcher
echo ============================================================
echo.
echo Starting interactive setup assistant...
echo.

cd "Plugins\AdastreaDirector\Python"
python setup_interactive.py

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ============================================================
    echo   Setup completed successfully!
    echo ============================================================
    echo.
    echo Next: Open Unreal Engine and test the plugin
    echo   Window ^> Developer Tools ^> Adastrea Director
    echo.
) else (
    echo.
    echo ============================================================
    echo   Setup encountered errors
    echo ============================================================
    echo.
    echo Please check the error messages above
    echo Or see: RAG_SETUP_SUMMARY.md for manual steps
    echo.
)

pause
