#!/usr/bin/env python3
"""
Adastrea - Master Content Generator Launcher

This script sets up the Python path and launches the Master Content Generator.
Run this from Unreal Engine Python console.

Usage in Unreal Engine Python Console:
    exec(open('Content/Python/MasterContentGeneratorLauncher.py').read())
    # or
    import MasterContentGeneratorLauncher
"""

import sys
import os
from pathlib import Path

def setup_python_path():
    """Add necessary directories to Python path."""
    # Get the project content directory
    content_dir = Path(__file__).parent.parent  # Go up from Python/ to Content/

    # Add Content/Python to path
    python_dir = content_dir / "Python"
    if str(python_dir) not in sys.path:
        sys.path.insert(0, str(python_dir))
        print(f"Added to Python path: {python_dir}")

    # Add project root for other scripts
    project_root = content_dir.parent
    if str(project_root) not in sys.path:
        sys.path.insert(0, str(project_root))
        print(f"Added to Python path: {project_root}")

def launch_generator():
    """Import MasterContentGenerator and make functions available."""
    try:
        import MasterContentGenerator
        print("✅ MasterContentGenerator imported successfully!")

        # Create a simple interface
        class ContentGenerator:
            @staticmethod
            def generate_all():
                return MasterContentGenerator.generate_all_content()

            @staticmethod
            def generate_blueprints():
                return MasterContentGenerator.generate_blueprints()

            @staticmethod
            def generate_data_assets():
                return MasterContentGenerator.generate_data_assets()

            @staticmethod
            def generate_input():
                return MasterContentGenerator.generate_input_system()

            @staticmethod
            def generate_maps():
                return MasterContentGenerator.generate_maps()

            @staticmethod
            def generate_ui():
                return MasterContentGenerator.generate_ui_widgets()

        # Make it globally available
        import unreal
        unreal.ContentGenerator = ContentGenerator

        print("\n🚀 Content Generator Ready!")
        print("Usage:")
        print("  unreal.ContentGenerator.generate_all()     # Generate everything")
        print("  unreal.ContentGenerator.generate_blueprints()  # Blueprints only")
        print("  unreal.ContentGenerator.generate_data_assets() # Data Assets only")
        print("  unreal.ContentGenerator.generate_input()   # Input system only")
        print("  unreal.ContentGenerator.generate_maps()    # Maps only")
        print("  unreal.ContentGenerator.generate_ui()      # UI widgets only")

        return ContentGenerator

    except ImportError as e:
        print(f"❌ Failed to import MasterContentGenerator: {e}")
        return None

# Auto-run when executed
setup_python_path()
generator = launch_generator()