#!/usr/bin/env python3
"""
Enhanced Repository Health Check Script
Comprehensive health checking for Adastrea repository during endless development cycles.
"""

import os
import subprocess
import sys
from datetime import datetime
import json

def check_git_status():
    """Check git repository status with more detail."""
    print("[CHECK] Checking Git repository status...")
    
    try:
        # Check if we're in a git repository
        result = subprocess.run(['git', 'status'], 
                              capture_output=True, text=True, cwd=os.getcwd())
        
        if "not a git repository" in result.stderr:
            print("[ERROR] Not a git repository")
            return False, "Not a git repository"
        
        # Check for uncommitted changes
        if "nothing to commit" in result.stdout:
            print("[OK] Repository is clean")
            return True, "Clean"
        else:
            # Count uncommitted changes
            changes_result = subprocess.run(['git', 'status', '--porcelain'],
                                          capture_output=True, text=True, cwd=os.getcwd())
            changes = [line for line in changes_result.stdout.strip().split('\n') if line]
            print(f"[WARN] Repository has {len(changes)} uncommitted changes")
            return False, f"{len(changes)} uncommitted changes"
            
    except Exception as e:
        print(f"[ERROR] Error checking git status: {e}")
        return False, f"Error: {str(e)}"

def check_git_history():
    """Check recent git commit history."""
    print("[CHECK] Checking Git commit history...")
    
    try:
        # Get last 5 commits
        result = subprocess.run(['git', 'log', '--oneline', '-5'],
                              capture_output=True, text=True, cwd=os.getcwd())
        
        if result.returncode != 0:
            print("[ERROR] Failed to get git history")
            return False, "Failed to get history"
        
        commits = result.stdout.strip().split('\n')
        print(f"[OK] Found {len(commits)} recent commits")
        
        # Check for endless development commits
        endless_commits = [c for c in commits if 'endless' in c.lower() or 'cycle' in c.lower()]
        if endless_commits:
            print(f"[OK] Found {len(endless_commits)} endless development commits")
        
        return True, f"{len(commits)} recent commits, {len(endless_commits)} endless"
        
    except Exception as e:
        print(f"[ERROR] Error checking git history: {e}")
        return False, f"Error: {str(e)}"

def check_python_tests():
    """Run Python test scripts with detailed reporting."""
    print("\n[TEST] Running Python tests...")
    
    test_files = [
        'test_todo_verification.py',
        'test_station_systems.py', 
        'test_damage_calculations.py'
    ]
    
    results = []
    all_passed = True
    
    for test_file in test_files:
        if os.path.exists(test_file):
            print(f"  Running {test_file}...")
            try:
                result = subprocess.run(['python', test_file], 
                                      capture_output=True, text=True, cwd=os.getcwd())
                
                if result.returncode == 0:
                    print(f"  [OK] {test_file} passed")
                    results.append((test_file, True, "Passed"))
                else:
                    print(f"  [FAIL] {test_file} failed")
                    print(f"    Error: {result.stderr[:200]}")
                    results.append((test_file, False, result.stderr[:200]))
                    all_passed = False
            except Exception as e:
                print(f"  [ERROR] {test_file} error: {e}")
                results.append((test_file, False, str(e)))
                all_passed = False
        else:
            print(f"  [MISSING] {test_file} not found")
            results.append((test_file, False, "File not found"))
            all_passed = False
    
    return all_passed, results

def check_documentation_files():
    """Check documentation files for freshness and completeness."""
    print("\n[DOCS] Checking documentation files...")
    
    doc_files = [
        'README.md',
        'ROADMAP.md',
        'TODO_TRACKING.md',
        'docs/GETTING_STARTED.md',
        'docs/CHANGELOG.md'
    ]
    
    results = []
    all_ok = True
    
    for doc_file in doc_files:
        if os.path.exists(doc_file):
            # Check modification time
            mtime = os.path.getmtime(doc_file)
            age_days = (datetime.now().timestamp() - mtime) / (60 * 60 * 24)
            
            if age_days < 7:  # Updated in last week
                status = "Fresh"
                print(f"  [OK] {doc_file} (updated {age_days:.1f} days ago)")
                results.append((doc_file, True, f"Fresh ({age_days:.1f} days)"))
            else:
                status = "Stale"
                print(f"  [WARN] {doc_file} (updated {age_days:.1f} days ago - may be stale)")
                results.append((doc_file, False, f"Stale ({age_days:.1f} days)"))
                all_ok = False
        else:
            print(f"  [MISSING] {doc_file} not found")
            results.append((doc_file, False, "Missing"))
            all_ok = False
    
    return all_ok, results

def check_code_files():
    """Check key code files exist and are accessible."""
    print("\n[CODE] Checking code files...")
    
    key_cpp_files = [
        'Source/Adastrea/Private/Stations/SpaceStation.cpp',
        'Source/Adastrea/Public/Stations/SpaceStation.h',
        'Source/Adastrea/Private/Stations/SpaceStationModule.cpp',
        'Source/Adastrea/Public/Stations/SpaceStationModule.h'
    ]
    
    results = []
    all_exist = True
    
    for cpp_file in key_cpp_files:
        if os.path.exists(cpp_file):
            # Check file size
            size_kb = os.path.getsize(cpp_file) / 1024
            print(f"  [OK] {cpp_file} ({size_kb:.1f} KB)")
            results.append((cpp_file, True, f"Exists ({size_kb:.1f} KB)"))
        else:
            print(f"  [ERROR] {cpp_file} not found")
            results.append((cpp_file, False, "Missing"))
            all_exist = False
    
    return all_exist, results

def check_todo_tracking():
    """Check TODO tracking file is up to date."""
    print("\n[TODO] Checking TODO tracking...")
    
    todo_file = 'TODO_TRACKING.md'
    
    if not os.path.exists(todo_file):
        print(f"  [ERROR] {todo_file} not found")
        return False, "Missing"
    
    try:
        with open(todo_file, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Check for recent updates
        if 'March 16, 2026' in content or '2026-03-16' in content:
            print(f"  [OK] TODO tracking updated today")
            return True, "Updated today"
        elif 'March 2026' in content:
            print(f"  [OK] TODO tracking updated this month")
            return True, "Updated this month"
        else:
            print(f"  [WARN] TODO tracking may be outdated")
            return False, "May be outdated"
            
    except Exception as e:
        print(f"  [ERROR] Error reading TODO file: {e}")
        return False, f"Error: {str(e)}"

def check_file_sizes():
    """Check for unusually large or small files."""
    print("\n[SIZE] Checking file sizes...")
    
    large_files = []
    
    for root, dirs, files in os.walk('.'):
        # Skip some directories
        if any(skip in root for skip in ['.git', '__pycache__', '.vs', 'Binaries', 'Intermediate']):
            continue
            
        for file in files:
            filepath = os.path.join(root, file)
            try:
                size_mb = os.path.getsize(filepath) / (1024 * 1024)
                
                # Check for very large files (>10MB)
                if size_mb > 10:
                    large_files.append((filepath, size_mb))
            except:
                pass
    
    if large_files:
        print(f"  [WARN] Found {len(large_files)} large files (>10MB):")
        for filepath, size_mb in large_files[:5]:  # Show first 5
            print(f"    {filepath}: {size_mb:.1f} MB")
        return False, f"{len(large_files)} large files"
    else:
        print(f"  [OK] No unusually large files found")
        return True, "No large files"

def main():
    """Run all enhanced health checks."""
    print("=" * 60)
    print("Adastrea Enhanced Repository Health Check")
    print(f"Date: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print("=" * 60)
    
    # Store detailed results
    results = []
    
    # Run enhanced checks
    print("\n[PHASE 1] Basic Checks")
    print("-" * 40)
    
    git_status_ok, git_status_detail = check_git_status()
    results.append(("Git Status", git_status_ok, git_status_detail))
    
    git_history_ok, git_history_detail = check_git_history()
    results.append(("Git History", git_history_ok, git_history_detail))
    
    print("\n[PHASE 2] Testing & Documentation")
    print("-" * 40)
    
    tests_ok, tests_detail = check_python_tests()
    results.append(("Python Tests", tests_ok, f"{len(tests_detail)} tests"))
    
    docs_ok, docs_detail = check_documentation_files()
    results.append(("Documentation", docs_ok, f"{len(docs_detail)} files"))
    
    print("\n[PHASE 3] Code & Quality")
    print("-" * 40)
    
    code_ok, code_detail = check_code_files()
    results.append(("Code Files", code_ok, f"{len(code_detail)} files"))
    
    todo_ok, todo_detail = check_todo_tracking()
    results.append(("TODO Tracking", todo_ok, todo_detail))
    
    size_ok, size_detail = check_file_sizes()
    results.append(("File Sizes", size_ok, size_detail))
    
    # Summary
    print("\n" + "=" * 60)
    print("ENHANCED HEALTH CHECK SUMMARY")
    print("=" * 60)
    
    passed = sum(1 for _, ok, _ in results if ok)
    total = len(results)
    score = (passed / total) * 100 if total > 0 else 0
    
    for check_name, status, detail in results:
        if status:
            print(f"[PASS] {check_name}: {detail}")
        else:
            print(f"[FAIL] {check_name}: {detail}")
    
    print(f"\n[SCORE] {passed}/{total} checks passed ({score:.1f}%)")
    
    # Health assessment
    if score >= 90:
        print("[EXCELLENT] Repository is in excellent health! 🎉")
    elif score >= 75:
        print("[GOOD] Repository is in good health 👍")
    elif score >= 60:
        print("[FAIR] Repository needs some attention ⚠️")
    else:
        print("[POOR] Repository needs significant attention 🚨")
    
    # Save detailed report
    report = {
        "timestamp": datetime.now().isoformat(),
        "score": score,
        "passed": passed,
        "total": total,
        "checks": [
            {
                "name": name,
                "passed": passed_status,
                "detail": detail
            }
            for name, passed_status, detail in results
        ]
    }
    
    report_file = f"health_report_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json"
    with open(report_file, 'w') as f:
        json.dump(report, f, indent=2)
    
    print(f"\n[REPORT] Detailed report saved to {report_file}")
    
    return score >= 75  # Return True if at least "good" health

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)