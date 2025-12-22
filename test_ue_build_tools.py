#!/usr/bin/env python3
"""
Test script to verify UE build tools setup and build scripts
"""

import os
import subprocess
import sys

def test_scripts_exist():
    """Test that all necessary scripts exist"""
    scripts = [
        'setup_ue_build_tools.sh',
        'build_with_ue_tools.sh'
    ]
    
    print("\n" + "="*60)
    print("Checking Script Files")
    print("="*60)
    
    all_exist = True
    for script in scripts:
        if os.path.exists(script):
            print(f"✓ {script} exists")
            # Check if executable
            if os.access(script, os.X_OK):
                print(f"  └─ Executable: Yes")
            else:
                print(f"  └─ Executable: No (needs chmod +x)")
                all_exist = False
        else:
            print(f"✗ {script} NOT FOUND")
            all_exist = False
    
    return all_exist

def test_script_syntax():
    """Test that scripts have valid bash syntax"""
    scripts = [
        'setup_ue_build_tools.sh',
        'build_with_ue_tools.sh'
    ]
    
    print("\n" + "="*60)
    print("Validating Script Syntax")
    print("="*60)
    
    all_valid = True
    for script in scripts:
        if os.path.exists(script):
            # Note: shell=True is used here for bash -n syntax validation.
            # The script filename is hardcoded, not from user input, so this is safe.
            result = subprocess.run(
                f"bash -n {script}",
                shell=True,
                capture_output=True,
                text=True
            )
            
            if result.returncode == 0:
                print(f"✓ {script} - Valid syntax")
            else:
                print(f"✗ {script} - Syntax error:")
                print(result.stderr)
                all_valid = False
        else:
            print(f"✗ {script} - Not found, skipping syntax check")
            all_valid = False
    
    return all_valid

def test_documentation_exists():
    """Test that documentation file exists"""
    doc_file = 'BUILD_WITH_UE_TOOLS.md'
    
    print("\n" + "="*60)
    print("Checking Documentation")
    print("="*60)
    
    if os.path.exists(doc_file):
        print(f"✓ {doc_file} exists")
        # Check file size
        size = os.path.getsize(doc_file)
        print(f"  └─ Size: {size} bytes")
        
        # Check for key sections
        with open(doc_file, 'r') as f:
            content = f.read()
            sections = [
                'Prerequisites',
                'Quick Start',
                'Troubleshooting',
                'GitHub Actions'
            ]
            
            for section in sections:
                if section in content:
                    print(f"  └─ Section '{section}': Found")
                else:
                    print(f"  └─ Section '{section}': Missing")
        
        return True
    else:
        print(f"✗ {doc_file} NOT FOUND")
        return False

def test_gitignore():
    """Test that .gitignore includes UnrealBuildTools"""
    print("\n" + "="*60)
    print("Checking .gitignore")
    print("="*60)
    
    if os.path.exists('.gitignore'):
        with open('.gitignore', 'r') as f:
            content = f.read()
            
        if 'UnrealBuildTools' in content:
            print("✓ .gitignore includes UnrealBuildTools")
            return True
        else:
            print("✗ .gitignore does NOT include UnrealBuildTools")
            return False
    else:
        print("✗ .gitignore NOT FOUND")
        return False

def test_workflow_exists():
    """Test that GitHub Actions workflow exists"""
    workflow_file = '.github/workflows/test-build-with-ue-tools.yml'
    
    print("\n" + "="*60)
    print("Checking GitHub Actions Workflow")
    print("="*60)
    
    if os.path.exists(workflow_file):
        print(f"✓ {workflow_file} exists")
        
        # Check for key workflow elements
        with open(workflow_file, 'r') as f:
            content = f.read()
            elements = [
                'workflow_dispatch',
                'setup_ue_build_tools.sh',
                'build_with_ue_tools.sh',
                'UnrealBuildTool'
            ]
            
            for element in elements:
                if element in content:
                    print(f"  └─ Element '{element}': Found")
                else:
                    print(f"  └─ Element '{element}': Missing")
        
        return True
    else:
        print(f"✗ {workflow_file} NOT FOUND")
        return False

def main():
    """Run all tests"""
    print("\n" + "="*60)
    print("UE Build Tools - Test Suite")
    print("="*60)
    
    tests = [
        ("Script Files Exist", test_scripts_exist),
        ("Script Syntax Valid", test_script_syntax),
        ("Documentation Exists", test_documentation_exists),
        (".gitignore Updated", test_gitignore),
        ("GitHub Workflow Exists", test_workflow_exists),
    ]
    
    results = []
    for name, test_func in tests:
        try:
            passed = test_func()
            results.append((name, passed))
        except Exception as e:
            print(f"\n✗ {name} - ERROR: {e}")
            results.append((name, False))
    
    # Summary
    print("\n" + "="*60)
    print("Test Summary")
    print("="*60)
    
    passed_count = sum(1 for _, passed in results if passed)
    total_count = len(results)
    
    for name, passed in results:
        status = "✓ PASS" if passed else "✗ FAIL"
        print(f"{status}: {name}")
    
    print("\n" + "-"*60)
    print(f"Total: {passed_count}/{total_count} tests passed")
    print("="*60)
    
    if passed_count == total_count:
        print("\n✓ All tests passed!")
        return 0
    else:
        print(f"\n✗ {total_count - passed_count} test(s) failed")
        return 1

if __name__ == '__main__':
    sys.exit(main())
