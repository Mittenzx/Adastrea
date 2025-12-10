#!/usr/bin/env python3
"""
Adastrea - Generator Environment Checker

Quick diagnostic script to check if the environment is ready to run
the content generation scripts.

Usage:
    # In Unreal Editor Python Console
    import CheckGeneratorEnvironment
    CheckGeneratorEnvironment.check_all()
"""

import sys
from pathlib import Path

def check_all():
    """Run all environment checks"""
    print("=" * 80)
    print("ADASTREA CONTENT GENERATOR ENVIRONMENT CHECK")
    print("=" * 80)
    print()
    
    all_passed = True
    
    # Check 1: Unreal Engine
    print("1. Checking Unreal Engine availability...")
    try:
        import unreal
        print("   ✓ Unreal Engine Python API available")
        project_dir = unreal.SystemLibrary.get_project_directory()
        print(f"   ✓ Project directory: {project_dir}")
    except ImportError:
        print("   ✗ ERROR: Not running inside Unreal Editor!")
        print("   → Open Adastrea.uproject in Unreal Engine")
        print("   → Use Tools → Python → Python Console")
        all_passed = False
        return False
    
    # Check 2: PyYAML
    print()
    print("2. Checking PyYAML availability...")
    try:
        import yaml
        print("   ✓ PyYAML is installed")
    except ImportError:
        print("   ✗ WARNING: PyYAML not found")
        print("   → Install with: import subprocess, sys")
        print("   → subprocess.check_call([sys.executable, '-m', 'pip', 'install', 'pyyaml'])")
        all_passed = False
    
    # Check 3: Python version
    print()
    print("3. Checking Python version...")
    python_version = sys.version_info
    print(f"   Python {python_version.major}.{python_version.minor}.{python_version.micro}")
    if python_version.major >= 3 and python_version.minor >= 9:
        print("   ✓ Python version is sufficient (3.9+)")
    else:
        print("   ✗ WARNING: Python 3.9+ recommended")
        all_passed = False
    
    # Check 4: Generator scripts
    print()
    print("4. Checking generator scripts...")
    project_dir = Path(unreal.SystemLibrary.get_project_directory())
    
    required_scripts = [
        "MasterContentGenerator.py",
        "BlueprintGenerator.py",
        "DataAssetBatchCreator.py",
        "InputSystemGenerator.py",
        "MapGenerator.py",
        "UIWidgetGenerator.py",
        "ContentValidator.py",
    ]
    
    missing = []
    for script in required_scripts:
        script_path = project_dir / script
        if script_path.exists():
            print(f"   ✓ {script}")
        else:
            print(f"   ✗ {script} - NOT FOUND")
            missing.append(script)
            all_passed = False
    
    if missing:
        print(f"   Missing {len(missing)} scripts!")
    
    # Check 5: YAML templates
    print()
    print("5. Checking YAML templates...")
    assets_dir = project_dir / "Assets"
    
    if not assets_dir.exists():
        print("   ✗ Assets directory not found!")
        all_passed = False
    else:
        print("   ✓ Assets directory exists")
        
        # Count YAML files
        yaml_dirs = [
            "SpaceshipTemplates",
            "PersonnelTemplates",
            "TradingTemplates",
            "StationModuleTemplates",
            "SectorTemplates",
        ]
        
        total_yaml = 0
        for dir_name in yaml_dirs:
            dir_path = assets_dir / dir_name
            if dir_path.exists():
                yaml_count = len(list(dir_path.glob("*.yaml")))
                total_yaml += yaml_count
                print(f"   ✓ {dir_name}: {yaml_count} files")
            else:
                print(f"   - {dir_name}: Not found")
        
        print(f"   Total YAML templates: {total_yaml}")
        
        if total_yaml == 0:
            print("   ⚠ WARNING: No YAML templates found")
    
    # Check 6: Content directory
    print()
    print("6. Checking Content directory...")
    content_dir = project_dir / "Content"
    
    if not content_dir.exists():
        print("   ✗ Content directory not found!")
        all_passed = False
    else:
        print("   ✓ Content directory exists")
        
        # Check key folders
        key_folders = ["Blueprints", "DataAssets", "Input", "Maps", "UI"]
        for folder in key_folders:
            folder_path = content_dir / folder
            if folder_path.exists():
                print(f"   ✓ {folder}/")
            else:
                print(f"   - {folder}/ (will be created)")
    
    # Check 7: C++ classes
    print()
    print("7. Checking C++ Data Asset classes...")
    try:
        # Try to load some key classes
        key_classes = [
            "SpaceshipDataAsset",
            "PersonnelDataAsset",
            "FactionDataAsset",
            "TradeItemDataAsset",
            "InputConfigDataAsset",
        ]
        
        found_classes = []
        missing_classes = []
        
        for class_name in key_classes:
            try:
                cls = unreal.load_class(None, f"/Script/Adastrea.{class_name}")
                if cls:
                    found_classes.append(class_name)
                    print(f"   ✓ {class_name}")
                else:
                    missing_classes.append(class_name)
                    print(f"   ✗ {class_name} - Not found")
            except Exception:
                missing_classes.append(class_name)
                print(f"   ✗ {class_name} - Not found")
        
        if missing_classes:
            print(f"   ⚠ WARNING: {len(missing_classes)} classes not found")
            print("   → Ensure project is compiled in Development Editor")
            all_passed = False
        else:
            print(f"   ✓ All {len(found_classes)} key classes found")
    except Exception as e:
        print(f"   ✗ Error checking classes: {str(e)}")
        all_passed = False
    
    # Check 8: Documentation
    print()
    print("8. Checking documentation...")
    docs = [
        "CONTENT_GENERATION_GUIDE.md",
        "INPUT_CONTROLS_REFERENCE.md",
        "PYTHON_GENERATORS_README.md",
        "BLUEPRINT_REQUIREMENTS_LIST.md",
    ]
    
    for doc in docs:
        doc_path = project_dir / doc
        if doc_path.exists():
            print(f"   ✓ {doc}")
        else:
            print(f"   - {doc} (optional)")
    
    # Final summary
    print()
    print("=" * 80)
    if all_passed:
        print("✓ ENVIRONMENT CHECK PASSED!")
        print()
        print("Ready to generate content! Run:")
        print("  import MasterContentGenerator")
        print("  MasterContentGenerator.generate_all_content()")
    else:
        print("✗ ENVIRONMENT CHECK FAILED")
        print()
        print("Please fix the issues above before running generators.")
    print("=" * 80)
    
    return all_passed


def quick_check():
    """Quick check - just the essentials"""
    try:
        import unreal
        print("✓ Unreal Engine available")
    except ImportError:
        print("✗ Not in Unreal Editor")
        return False
    
    try:
        import yaml
        print("✓ PyYAML available")
    except ImportError:
        print("✗ PyYAML not installed")
        return False
    
    print("✓ Ready to generate!")
    return True


# Auto-run when imported
if __name__ == "__main__":
    check_all()
else:
    # When imported, run quick check
    print("Adastrea Generator Environment Checker")
    print("Run: CheckGeneratorEnvironment.check_all() for detailed check")
    print()
