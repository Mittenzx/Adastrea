#!/usr/bin/env python3
"""
Null Safety Validation Script for Adastrea

This script scans C++ implementation files to find pointer dereferences that may
be missing null checks, which can lead to crashes.

Usage:
    python Tools/check_null_safety.py [--verbose] [--export FILENAME]

Options:
    --verbose: Show detailed analysis for each file
    --export: Export results to file
"""

import re
import sys
from pathlib import Path
from typing import List, Dict

# Patterns for pointer operations that need null checks
DEREFERENCE_PATTERNS = [
    r'(\w+)->',  # Arrow operator (pointer member access)
    r'(\w+)\s*\.\s*\w+\s*\(',  # Call through reference (less critical but check)
]

# Patterns that indicate null checking
NULL_CHECK_PATTERNS = [
    r'if\s*\(\s*(\w+)\s*\)',
    r'if\s*\(\s*!(\w+)\s*\)',
    r'if\s*\(\s*(\w+)\s*==\s*nullptr\s*\)',
    r'if\s*\(\s*(\w+)\s*!=\s*nullptr\s*\)',
    r'if\s*\(\s*nullptr\s*==\s*(\w+)\s*\)',
    r'if\s*\(\s*nullptr\s*!=\s*(\w+)\s*\)',
    r'IsValid\s*\(\s*(\w+)\s*\)',
    r'check\s*\(\s*(\w+)\s*\)',
    r'ensure\s*\(\s*(\w+)\s*\)',
]

# Keywords to skip (not actual pointers)
SKIP_KEYWORDS = [
    'this', 'nullptr', 'NULL', 'true', 'false',
    'if', 'while', 'for', 'switch', 'return',
    'class', 'struct', 'enum', 'namespace',
]


class NullSafetyChecker:
    def __init__(self, source_dir: str, verbose: bool = False):
        self.source_dir = Path(source_dir)
        self.verbose = verbose
        self.issues = []
        self.total_files = 0
        self.files_with_issues = 0
        
    def is_null_checked(self, lines: List[str], line_idx: int, var_name: str, context_size: int = 5) -> bool:
        """Check if a variable has been null-checked in nearby context."""
        # Check previous lines for null checks
        start_idx = max(0, line_idx - context_size)
        
        for i in range(start_idx, line_idx):
            line = lines[i]
            for pattern in NULL_CHECK_PATTERNS:
                match = re.search(pattern, line)
                if match and var_name in match.groups():
                    return True
                # Also check for general pattern matching the variable
                if var_name in line and any(check in line for check in ['IsValid', 'nullptr', 'check', 'ensure']):
                    return True
        
        return False
    
    def scan_file(self, filepath: Path) -> List[Dict]:
        """Scan a single implementation file for potential null safety issues."""
        issues = []
        
        # Only scan .cpp files
        if filepath.suffix != '.cpp':
            return issues
        
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                lines = f.readlines()
        except Exception as e:
            if self.verbose:
                print(f"Error reading {filepath}: {e}")
            return issues
        
        in_comment_block = False
        
        for i, line in enumerate(lines, 1):
            # Track comment blocks
            if '/*' in line:
                in_comment_block = True
            if '*/' in line:
                in_comment_block = False
                continue
            
            # Skip comments and empty lines
            if in_comment_block or line.strip().startswith('//') or not line.strip():
                continue
            
            # Check for pointer dereferences
            for pattern in DEREFERENCE_PATTERNS:
                matches = re.finditer(pattern, line)
                for match in matches:
                    var_name = match.group(1) if match.lastindex >= 1 else None
                    
                    if not var_name or var_name in SKIP_KEYWORDS:
                        continue
                    
                    # Skip if it looks like a type cast or declaration
                    if 'Cast<' in line or 'new ' in line or '::' in line:
                        continue
                    
                    # Check if variable has been null-checked nearby
                    if not self.is_null_checked(lines, i-1, var_name):
                        issues.append({
                            'file': str(filepath),
                            'line': i,
                            'content': line.strip(),
                            'variable': var_name,
                            'severity': 'WARNING'
                        })
        
        return issues
    
    def scan_directory(self) -> Dict:
        """Scan all implementation files in the source directory."""
        print(f"Scanning {self.source_dir} for null safety issues...")
        
        cpp_files = list(self.source_dir.rglob('*.cpp'))
        self.total_files = len(cpp_files)
        
        all_issues = []
        files_with_issues = set()
        
        for filepath in cpp_files:
            file_issues = self.scan_file(filepath)
            if file_issues:
                all_issues.extend(file_issues)
                files_with_issues.add(str(filepath))
        
        self.issues = all_issues
        self.files_with_issues = len(files_with_issues)
        
        return {
            'total_files': self.total_files,
            'files_with_issues': self.files_with_issues,
            'total_issues': len(all_issues),
            'issues': all_issues
        }
    
    def print_report(self, results: Dict, max_issues: int = 50):
        """Print a formatted report of findings."""
        print("\n" + "="*80)
        print("Null Safety Validation Report")
        print("="*80)
        print(f"Total files scanned: {results['total_files']}")
        print(f"Files with potential issues: {results['files_with_issues']}")
        print(f"Total potential issues found: {results['total_issues']}")
        print("="*80)
        
        if results['total_issues'] == 0:
            print("\n✅ No obvious null safety issues found!")
            print("Note: This is a heuristic check. Manual code review is still recommended.")
            return
        
        print("\n⚠️  Potential Null Safety Issues Found:\n")
        print("Note: This is an automated heuristic scan with high false positive rate.")
        print("Many findings may be safe due to guarantees not visible to static analysis.")
        print("Use this as a guide for manual review, not as definitive issues.\n")
        
        # Limit output to avoid overwhelming
        display_issues = results['issues'][:max_issues]
        
        current_file = None
        for issue in display_issues:
            if issue['file'] != current_file:
                current_file = issue['file']
                print(f"\n📁 {current_file}")
            
            print(f"  Line {issue['line']}: Variable '{issue['variable']}'")
            if self.verbose:
                print(f"    {issue['content']}")
        
        if results['total_issues'] > max_issues:
            print(f"\n... and {results['total_issues'] - max_issues} more issues")
            print("Use --export option to see all findings")
        
        print("\n" + "="*80)
        print("Recommendations:")
        print("1. For Actor pointers, use: if (IsValid(ActorPtr)) { ActorPtr->Method(); }")
        print("2. For UObject pointers, use: if (ObjectPtr) { ObjectPtr->Method(); }")
        print("3. Consider helper macros for common patterns")
        print("4. Focus on high-risk areas: combat, spawning, AI decision making")
        print("="*80 + "\n")
    
    def export_results(self, output_file: str):
        """Export results to a file for review."""
        with open(output_file, 'w') as f:
            f.write("Null Safety Validation Results\n")
            f.write("="*80 + "\n\n")
            f.write(f"Total files scanned: {self.total_files}\n")
            f.write(f"Files with potential issues: {self.files_with_issues}\n")
            f.write(f"Total potential issues: {len(self.issues)}\n\n")
            f.write("Note: High false positive rate - manual review required\n\n")
            
            for issue in self.issues:
                f.write(f"{issue['file']}:{issue['line']} - Variable '{issue['variable']}'\n")
                f.write(f"  {issue['content']}\n\n")
        
        print(f"Results exported to {output_file}")


def main():
    import argparse
    
    parser = argparse.ArgumentParser(description='Check for null safety issues')
    parser.add_argument('--source-dir', default='Source/Adastrea',
                       help='Source directory to scan (default: Source/Adastrea)')
    parser.add_argument('--verbose', action='store_true',
                       help='Show detailed analysis')
    parser.add_argument('--export', help='Export results to file')
    parser.add_argument('--max-display', type=int, default=50,
                       help='Maximum issues to display (default: 50)')
    
    args = parser.parse_args()
    
    # Get the repository root
    script_dir = Path(__file__).parent
    repo_root = script_dir.parent
    source_dir = repo_root / args.source_dir
    
    if not source_dir.exists():
        print(f"Error: Source directory not found: {source_dir}")
        sys.exit(1)
    
    checker = NullSafetyChecker(source_dir, verbose=args.verbose)
    results = checker.scan_directory()
    checker.print_report(results, max_issues=args.max_display)
    
    if args.export:
        checker.export_results(args.export)
    
    # Don't exit with error for null checks (too many false positives)
    # This is informational only
    sys.exit(0)


if __name__ == '__main__':
    main()
