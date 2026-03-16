#!/usr/bin/env python3
"""
Check for Unicode characters that might cause issues on Windows.
"""

import re

def check_file_for_unicode(filename):
    """Check a file for non-ASCII characters."""
    try:
        with open(filename, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Find all non-ASCII characters
        matches = re.findall(r'[^\x00-\x7F]', content)
        
        if matches:
            print(f"Found {len(matches)} non-ASCII characters in {filename}:")
            unique_chars = set(matches)
            for char in unique_chars:
                print(f"  - '{char}' (U+{ord(char):04X}) appears {matches.count(char)} times")
            return False
        else:
            print(f"No non-ASCII characters found in {filename}")
            return True
    except Exception as e:
        print(f"Error checking {filename}: {e}")
        return False

if __name__ == "__main__":
    files_to_check = [
        "repository_health_check_enhanced.py",
        "docs/CHANGELOG.md",
        "TODO_TRACKING.md",
        "README.md"
    ]
    
    all_good = True
    for filename in files_to_check:
        print(f"\n{'='*60}")
        if check_file_for_unicode(filename):
            print(f"[OK] {filename} is ASCII-safe")
        else:
            print(f"[WARN] {filename} has Unicode characters that may cause issues")
            all_good = False
    
    print(f"\n{'='*60}")
    if all_good:
        print("[SUCCESS] All files are ASCII-safe for Windows compatibility!")
    else:
        print("[WARNING] Some files have Unicode characters that may cause issues on Windows.")