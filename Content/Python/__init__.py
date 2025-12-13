"""
Adastrea Content Python Package

This package contains Python scripts for content generation in the Adastrea project.
All scripts are designed to run within Unreal Engine's Python environment.

Main modules:
- MasterContentGenerator: Main content generation orchestrator
- MasterContentGeneratorLauncher: Easy launcher with path setup
"""

# Auto-setup Python path when package is imported
import sys
import os
from pathlib import Path

# Add current directory and project root to path
_current_dir = Path(__file__).parent
_project_root = _current_dir.parent.parent

for path in [_current_dir, _project_root]:
    if str(path) not in sys.path:
        sys.path.insert(0, str(path))

__version__ = "1.0.0"
__author__ = "Adastrea Development Team"