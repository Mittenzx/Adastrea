@echo off
REM Re-ingest documentation with updated configuration

echo.
echo ============================================================
echo   AdastreaDirector - Documentation Re-ingestion
echo ============================================================
echo.
echo This will re-ingest all documentation with the new
echo focused configuration (188 files instead of 352)
echo.
echo Benefits:
echo   - Excludes obsolete documents
echo   - Removes build artifacts
echo   - Better organized by category
echo   - Faster queries and more accurate results
echo.
pause

cd "Plugins\AdastreaDirector\Python"

echo.
echo Preview of what will be ingested:
echo.
python preview_ingestion.py

echo.
echo.
echo Press any key to start re-ingestion, or Ctrl+C to cancel...
pause > nul

echo.
echo Starting re-ingestion...
python setup_rag.py --reingest

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ============================================================
    echo   Re-ingestion completed successfully!
    echo ============================================================
    echo.
    echo The RAG database has been updated with curated documentation.
    echo You can now query your project with improved accuracy.
    echo.
) else (
    echo.
    echo ============================================================
    echo   Re-ingestion encountered errors
    echo ============================================================
    echo.
    echo Please check the error messages above
    echo.
)

pause
