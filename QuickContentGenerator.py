#!/usr/bin/env python3
"""
Adastrea Content Generator - Python Console Version

COPY AND PASTE THIS ENTIRE SCRIPT into Unreal Engine's Python Console
Window → Developer Tools → Python Console

This will generate 1000+ assets for the Adastrea space flight game.
"""

import sys
import os
from pathlib import Path

# Setup paths
script_dir = Path(__file__).parent if '__file__' in globals() else Path.cwd()
content_dir = script_dir / "Content" / "Python"
project_dir = script_dir

paths_to_add = [str(content_dir), str(project_dir)]
for path in paths_to_add:
    if path not in sys.path:
        sys.path.insert(0, path)

print("🚀 Starting Adastrea Content Generation...")

try:
    import MasterContentGenerator as mcg
    print("✅ MasterContentGenerator loaded")

    print("🎯 Generating ALL content (this may take several minutes)...")
    result = mcg.generate_all_content()

    if result:
        print("🎉 SUCCESS: All content generated!")
        print("Check Content Browser and World Outliner for new assets.")
    else:
        print("❌ Generation failed - check Output Log")

except Exception as e:
    print(f"❌ Error: {e}")
    import traceback
    traceback.print_exc()