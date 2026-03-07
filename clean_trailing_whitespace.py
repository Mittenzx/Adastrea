#!/usr/bin/env python3
"""
Clean trailing whitespace from Adastrea files
"""

import os
import re
from pathlib import Path

def clean_file(filepath):
    """Clean trailing whitespace from a single file"""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()

        # Remove trailing whitespace from each line
        cleaned = re.sub(r'[ \t]+$', '', content, flags=re.MULTILINE)

        if content != cleaned:
            with open(filepath, 'w', encoding='utf-8', newline='\n') as f:
                f.write(cleaned)
            return True
        return False
    except Exception as e:
        print(f"  Error processing {filepath}: {e}")
        return False

def main():
    print("Cleaning trailing whitespace from Adastrea files")
    print("=" * 50)

    project_root = Path.cwd()

    # File extensions to clean
    extensions = {'.cpp', '.h', '.py', '.md', '.txt', '.json', '.yml', '.yaml', '.ini', '.cfg', '.bat', '.sh'}

    cleaned_count = 0
    total_files = 0

    for ext in extensions:
        for filepath in project_root.rglob(f'*{ext}'):
            total_files += 1
            if clean_file(filepath):
                cleaned_count += 1
                print(f"  Cleaned: {filepath.relative_to(project_root)}")

    print(f"\nCleaned {cleaned_count} files out of {total_files} scanned")

    # Special handling for README.md - ensure it's properly formatted
    readme_path = project_root / 'README.md'
    if readme_path.exists():
        print(f"\nEnsuring README.md is properly formatted...")
        clean_file(readme_path)
        print("  README.md cleaned")

if __name__ == '__main__':
    main()