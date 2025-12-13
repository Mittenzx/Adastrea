#!/usr/bin/env python3
"""
Simple test script for Unreal Engine Python imports
Run this in UE Python console: exec(open('Content/Python/test_import.py').read())
"""

import sys
print(f"Python version: {sys.version}")
print(f"Python executable: {sys.executable}")
print(f"Current working directory: {sys.path[0] if sys.path else 'None'}")

print("\nPython path:")
for i, path in enumerate(sys.path[:10]):  # Show first 10 paths
    print(f"  {i}: {path}")

print("\nTesting basic imports...")
try:
    import os
    print("✅ os import successful")
except ImportError as e:
    print(f"❌ os import failed: {e}")

try:
    import pathlib
    print("✅ pathlib import successful")
except ImportError as e:
    print(f"❌ pathlib import failed: {e}")

print("\nTesting unreal import...")
try:
    import unreal
    print("✅ unreal import successful")
    print(f"Unreal version: {unreal.SystemLibrary.get_engine_version()}")
except ImportError as e:
    print(f"❌ unreal import failed: {e}")

print("\nTesting Content/Python path...")
content_python_path = r"C:\Unreal Projects\Adastrea\Content\Python"
if content_python_path in sys.path:
    print("✅ Content/Python already in path")
else:
    print("❌ Content/Python NOT in path - adding it...")
    sys.path.insert(0, content_python_path)
    print("✅ Added Content/Python to path")

print("\nTesting MasterContentGenerator import...")
try:
    import MasterContentGenerator
    print("✅ MasterContentGenerator import successful!")
    print(f"Available functions: {[f for f in dir(MasterContentGenerator) if not f.startswith('_')]}")
except ImportError as e:
    print(f"❌ MasterContentGenerator import failed: {e}")
    import traceback
    traceback.print_exc()