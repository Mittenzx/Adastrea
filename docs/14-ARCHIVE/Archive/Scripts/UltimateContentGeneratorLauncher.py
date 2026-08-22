#!/usr/bin/env python3
"""
Adastrea Ultimate Content Generator Launcher

This script launches the complete content generation pipeline for the Adastrea project.
Execute this in Unreal Engine's Python console to generate massive amounts of content.

USAGE IN UNREAL ENGINE PYTHON CONSOLE:
1. Open Unreal Editor with Adastrea project
2. Window → Developer Tools → Python Console
3. Copy and paste this entire script
4. Press Enter to execute

This will generate:
- 1000+ Data Assets (ships, factions, weapons, commodities, quests)
- 100+ Blueprints (stations, modules, AI, UI)
- Enhanced Input configurations
- Test maps and scenes
- Complete validation and testing

Author: Adastrea Development Team
"""

import sys
import os
from pathlib import Path

def setup_paths():
    """Set up Python paths for content generation."""
    print("🔧 Setting up Python paths...")

    # Get current script location and project structure
    script_dir = Path(__file__).parent  # Content/Python
    content_dir = script_dir.parent     # Content
    project_dir = content_dir.parent    # Project root

    # Add necessary paths
    paths_to_add = [
        str(script_dir),      # Content/Python
        str(content_dir),     # Content
        str(project_dir),     # Project root
        str(project_dir / "Plugins" / "AdastreaDirector" / "Python"),  # Plugin Python
    ]

    for path in paths_to_add:
        if path not in sys.path:
            sys.path.insert(0, path)
            print(f"  ✅ Added: {Path(path).name}")

    print("🔧 Python paths configured!\n")

def launch_content_generation():
    """Launch the master content generator."""
    print("🚀 Launching Adastrea Content Generation...")
    print("=" * 60)

    try:
        # Import the master generator
        print("📦 Importing MasterContentGenerator...")
        import MasterContentGenerator as mcg
        print("✅ MasterContentGenerator imported successfully!\n")

        # Show available functions
        print("🎯 Available generation functions:")
        functions = [
            ("generate_all_content()", "Generate EVERYTHING (recommended)"),
            ("generate_blueprints()", "Blueprints only"),
            ("generate_data_assets()", "Data Assets only"),
            ("generate_input_system()", "Input system only"),
            ("generate_maps()", "Maps only"),
            ("generate_ui_widgets()", "UI widgets only"),
        ]

        for func, desc in functions:
            print(f"  • {func:<25} - {desc}")

        print("\n" + "=" * 60)
        print("🎯 EXECUTING: generate_all_content()")
        print("This will take several minutes and generate 1000+ assets...")
        print("=" * 60 + "\n")

        # Execute the full generation
        start_time = __import__('time').time()

        result = mcg.generate_all_content()

        end_time = __import__('time').time()
        duration = end_time - start_time

        print("\n" + "=" * 60)
        if result:
            print("🎉 CONTENT GENERATION COMPLETED SUCCESSFULLY!")
            print(".1f"        else:
            print("❌ Content generation failed!")
            print("Check the Unreal Editor Output Log for details.")

        print("=" * 60)

        return result

    except ImportError as e:
        print(f"❌ Failed to import MasterContentGenerator: {e}")
        print("Make sure you're running this in Unreal Editor's Python environment.")
        return False
    except Exception as e:
        print(f"❌ Content generation failed with error: {e}")
        import traceback
        traceback.print_exc()
        return False

def main():
    """Main execution function."""
    print("🌟 ADASTREA ULTIMATE CONTENT GENERATOR 🌟")
    print("==========================================")
    print()

    # Setup
    setup_paths()

    # Verify Unreal availability
    try:
        import unreal
        version = unreal.SystemLibrary.get_engine_version()
        print(f"🎮 Unreal Engine {version} detected")
    except ImportError:
        print("❌ ERROR: Not running in Unreal Engine environment!")
        print("This script must be executed in Unreal Editor's Python console.")
        print("\nSteps:")
        print("1. Open Adastrea.uproject in Unreal Editor")
        print("2. Go to: Window → Developer Tools → Python Console")
        print("3. Copy and paste this script")
        print("4. Press Enter")
        return False

    print()

    # Launch generation
    success = launch_content_generation()

    if success:
        print("\n🎊 All content generated successfully!")
        print("Check the Content Browser and World Outliner for new assets.")
        print("\nNext steps:")
        print("• Test generated Blueprints in PIE mode")
        print("• Validate Data Assets in editor")
        print("• Run automated tests")
    else:
        print("\n❌ Content generation encountered issues.")
        print("Check the Output Log for detailed error messages.")

    return success

# Auto-execute when run
if __name__ == "__main__":
    main()