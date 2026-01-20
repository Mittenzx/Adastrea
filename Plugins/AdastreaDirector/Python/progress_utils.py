#!/usr/bin/env python3
"""
Progress File Utilities

Simple utility module with no external dependencies for writing progress files.
This module is designed to be imported by both rag_ingestion.py and ipc_server.py
to avoid code duplication and ensure consistency in progress file handling.

Dependencies: Only standard library (json, pathlib, time)
"""

import json
import time
from pathlib import Path


def write_progress_file(
    progress_file_path: str,
    percent: float,
    label: str = "",
    details: str = "",
    status: str = "processing"
) -> None:
    """
    Write a progress update to a JSON file.
    
    Creates parent directories if they don't exist. This function uses only
    standard library modules to ensure it can be imported even when other
    dependencies are missing.
    
    Args:
        progress_file_path: Path to the progress file
        percent: Progress percentage (0-100)
        label: Main progress label
        details: Detailed progress information
        status: Status string (processing, complete, error)
        
    Raises:
        Exception: If file cannot be written (caller should handle)
    """
    # Ensure parent directory exists
    progress_path = Path(progress_file_path)
    progress_path.parent.mkdir(parents=True, exist_ok=True)
    
    # Build progress data
    progress_data = {
        'percent': min(100, max(0, percent)),
        'label': label,
        'details': details,
        'status': status,
        'timestamp': time.time()
    }
    
    # Write to file
    with open(progress_file_path, 'w') as f:
        json.dump(progress_data, f)
