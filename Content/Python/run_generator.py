#!/usr/bin/env python3
"""
Adastrea - Simple Content Generator Launcher

Execute this in Unreal Engine Python console:
exec(open('Content/Python/run_generator.py').read())
"""

import sys
import os
from pathlib import Path

# Force add the Content/Python directory to path
script_dir = Path(__file__).parent  # Content/Python
content_dir = script_dir.parent     # Content
project_dir = content_dir.parent    # Project root

# Add both directories to ensure imports work
paths_to_add = [str(script_dir), str(project_dir)]
for path in paths_to_add:
    if path not in sys.path:
        sys.path.insert(0, path)
        print(f"Added to path: {path}")

print(f"Python path now includes: {sys.path[0]}")

# Now try to import
try:
    import MasterContentGenerator as mcg
    print("✅ MasterContentGenerator loaded successfully!")

    # Make functions available globally
    generate_all = mcg.generate_all_content
    generate_blueprints = mcg.generate_blueprints
    generate_data_assets = mcg.generate_data_assets
    generate_input = mcg.generate_input_system
    generate_maps = mcg.generate_maps
    generate_ui = mcg.generate_ui_widgets

    print("\n🚀 Ready to generate content!")
    print("Available commands:")
    print("  generate_all()         - Generate everything")
    print("  generate_blueprints()  - Blueprints only")
    print("  generate_data_assets() - Data Assets only")
    print("  generate_input()       - Input system only")
    print("  generate_maps()        - Maps only")
    print("  generate_ui()          - UI widgets only")

    print("\n💡 Try: generate_all()")

except ImportError as e:
    print(f"❌ Import failed: {e}")
    print("Debug info:")
    print(f"Script dir: {script_dir}")
    print(f"Content dir: {content_dir}")
    print(f"Project dir: {project_dir}")
    print(f"Python path: {sys.path[:3]}")

    # Try to find the file
    mcg_path = script_dir / "MasterContentGenerator.py"
    if mcg_path.exists():
        print(f"✅ MasterContentGenerator.py exists at: {mcg_path}")
    else:
        print(f"❌ MasterContentGenerator.py NOT found at: {mcg_path}")

except Exception as e:
    print(f"❌ Unexpected error: {e}")
    import traceback
    traceback.print_exc()