#!/usr/bin/env python3
"""
Repository Health Check Script
Checks overall health of Adastrea repository during endless development cycles.
"""

import os
import subprocess
import sys
from datetime import datetime

def check_git_status():
    """Check git repository status."""
    print("[CHECK] Checking Git repository status...")
    
    try:
        # Check if we're in a git repository
        result = subprocess.run(['git', 'status'], 
                              capture_output=True, text=True, cwd=os.getcwd())
        
        if "not a git repository" in result.stderr:
            print("[ERROR] Not a git repository")
            return False
        
        if "nothing to commit" in result.stdout:
            print("[OK] Repository is clean")
            return True
        else:
            print("[WARN] Repository has uncommitted changes")
            return False
            
    except Exception as e:
        print(f"[ERROR] Error checking git status: {e}")
        return False

def check_python_tests():
    """Run Python test scripts."""
    print("\n[TEST] Running Python tests...")
    
    test_files = [
        'test_todo_verification.py',
        'test_station_systems.py', 
        'test_damage_calculations.py'
    ]
    
    all_passed = True
    for test_file in test_files:
        if os.path.exists(test_file):
            print(f"  Running {test_file}...")
            try:
                result = subprocess.run([sys.executable, test_file], 
                                      capture_output=True, text=True, cwd=os.getcwd())
                
                if result.returncode == 0:
                    print(f"  [OK] {test_file} passed")
                else:
                    print(f"  [ERROR] {test_file} failed")
                    print(f"    Error: {result.stderr[:200]}")
                    all_passed = False
            except Exception as e:
                print(f"  [ERROR] Error running {test_file}: {e}")
                all_passed = False
        else:
            print(f"  [WARN] {test_file} not found")
    
    return all_passed

def check_documentation_files():
    """Check key documentation files exist and are recent."""
    print("\n[DOCS] Checking documentation files...")
    
    key_files = [
        'README.md',
        'ROADMAP.md',
        'TODO_TRACKING.md',
        'docs/GETTING_STARTED.md',
        'docs/CHANGELOG.md'
    ]
    
    all_exist = True
    for doc_file in key_files:
        if os.path.exists(doc_file):
            # Check modification time
            mtime = os.path.getmtime(doc_file)
            mod_date = datetime.fromtimestamp(mtime)
            days_old = (datetime.now() - mod_date).days
            
            if days_old <= 7:
                print(f"  [OK] {doc_file} (updated {days_old} days ago)")
            else:
                print(f"  [WARN] {doc_file} (updated {days_old} days ago - consider updating)")
        else:
            print(f"  [ERROR] {doc_file} not found")
            all_exist = False
    
    return all_exist

def check_code_files():
    """Check key C++ source files exist."""
    print("\n[CODE] Checking code files...")
    
    key_cpp_files = [
        'Source/Adastrea/Private/Stations/SpaceStation.cpp',
        'Source/Adastrea/Public/Stations/SpaceStation.h',
        'Source/Adastrea/Private/Stations/SpaceStationModule.cpp',
        'Source/Adastrea/Public/Stations/SpaceStationModule.h'
    ]
    
    all_exist = True
    for cpp_file in key_cpp_files:
        if os.path.exists(cpp_file):
            print(f"  [OK] {cpp_file}")
        else:
            print(f"  [ERROR] {cpp_file} not found")
            all_exist = False
    
    return all_exist

def main():
    """Run all health checks."""
    print("=" * 60)
    print("Adastrea Repository Health Check")
    print(f"Date: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print("=" * 60)
    
    # Store results
    results = []
    
    # Run checks
    results.append(("Git Status", check_git_status()))
    results.append(("Python Tests", check_python_tests()))
    results.append(("Documentation", check_documentation_files()))
    results.append(("Code Files", check_code_files()))
    
    # Summary
    print("\n" + "=" * 60)
    print("HEALTH CHECK SUMMARY")
    print("=" * 60)
    
    passed = 0
    total = len(results)
    
    for check_name, status in results:
        if status:
            print(f"[PASS] {check_name}")
            passed += 1
        else:
            print(f"[FAIL] {check_name}")
    
    print(f"\n[SCORE] {passed}/{total} checks passed ({passed/total*100:.1f}%)")
    
    if passed == total:
        print("[EXCELLENT] Repository is in excellent health!")
        return 0
    elif passed >= total * 0.75:
        print("[WARNING] Repository has minor issues but is generally healthy")
        return 1
    else:
        print("[ERROR] Repository needs attention")
        return 2

if __name__ == "__main__":
    sys.exit(main())