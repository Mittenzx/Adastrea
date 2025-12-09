#!/usr/bin/env python3
"""
UPROPERTY Validation Script for Adastrea

This script scans C++ header files to find UObject* pointers that may be missing
the UPROPERTY() macro, which is critical for garbage collection tracking.

Usage:
    python Tools/check_uproperty.py [--verbose]

Options:
    --verbose: Show detailed analysis for each file
"""

import re
import sys
from pathlib import Path
from typing import List, Dict

# Patterns to match UObject-derived pointers
UOBJECT_PATTERNS = [
    r'\bU\w+\*',  # UClass* pointers
    r'\bA\w+\*',  # AActor* pointers
    r'\bF\w+\*',  # Some struct pointers (though not all need UPROPERTY)
]

# Patterns that indicate UPROPERTY is present
UPROPERTY_PATTERN = r'UPROPERTY\s*\([^)]*\)'

# Patterns to exclude (forward declarations, function parameters, etc.)
EXCLUDE_PATTERNS = [
    r'^\s*class\s+',  # Forward declarations
    r'^\s*struct\s+',  # Struct forward declarations
    r'^\s*//',  # Comments
    r'^\s*/\*',  # Block comments
    r'\bconst\s+\w+\*\s+\w+\s*[,)]',  # Function parameters
    r'\w+\*\s+\w+\s*[,)]',  # Function parameters without const
]


class UPropertyChecker:
    def __init__(self, source_dir: str, verbose: bool = False):
        self.source_dir = Path(source_dir)
        self.verbose = verbose
        self.issues = []
        self.total_files = 0
        self.files_with_issues = 0
        
    def scan_file(self, filepath: Path) -> List[Dict]:
        """Scan a single header file for potential UPROPERTY issues."""
        issues = []
        
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                lines = f.readlines()
        except Exception as e:
            print(f"Error reading {filepath}: {e}")
            return issues
        
        in_class = False
        in_comment_block = False
        in_function_signature = False
        
        for i, line in enumerate(lines, 1):
            # Track comment blocks
            if '/*' in line:
                in_comment_block = True
            if '*/' in line:
                in_comment_block = False
                continue
            
            # Skip if in comment block
            if in_comment_block:
                continue
            
            # Track if we're inside a class/struct
            if re.search(r'\b(?:UCLASS|USTRUCT|UINTERFACE)\b', line):
                in_class = True
            
            # Track function signatures (UFUNCTION or regular functions)
            if re.search(r'\b(?:UFUNCTION|virtual|static|inline)\b', line) or \
               (i >= 2 and re.search(r'\b(?:UFUNCTION|virtual|static|inline)\b', lines[i-2])):
                in_function_signature = True
            
            # Reset function signature tracking when we see a semicolon or opening brace
            # Note: This is a simple heuristic and may have edge cases with complex syntax
            if ';' in line or '{' in line:
                in_function_signature = False
            
            # Check for UObject pointers only inside classes and not in function signatures
            if in_class and not in_function_signature:
                # Check each UObject pattern
                for pattern in UOBJECT_PATTERNS:
                    if re.search(pattern, line):
                        # Skip if it matches exclude patterns
                        is_excluded = False
                        for exclude in EXCLUDE_PATTERNS:
                            if re.search(exclude, line):
                                is_excluded = True
                                break
                        
                        if is_excluded:
                            continue
                        
                        # Skip if line looks like a function signature, function pointer, or function call
                        # This regex matches lines with a word (possibly with return type), optional whitespace, then '('
                        # Covers function declarations, function pointers, and calls
                        if re.search(r'\b\w+\s*\([^;{)]*\)', line):
                            continue
                        
                        # Check if UPROPERTY is on the previous line(s)
                        # Look back up to 5 lines to handle multiline UPROPERTY declarations
                        # Note: i is 1-indexed, but lines array is 0-indexed
                        has_uproperty = False
                        
                        # Regex to match a C++ variable declaration (not a comment or string literal)
                        var_decl_pattern = re.compile(
                            r'^\s*'                                 # Optional leading whitespace
                            r'(?:[\w:<>]+[\s*&]+)+'                 # Type (with possible template, pointer, ref)
                            r'\w+'                                  # Variable name
                            r'\s*(?:=\s*[^;]+)?'                    # Optional initializer
                            r';\s*(?://.*)?$'                       # Semicolon, optional comment
                        )
                        
                        # Search backwards for UPROPERTY
                        for lookback in range(1, 6):
                            # Convert to 0-indexed for array access
                            check_idx = i - 1 - lookback
                            if check_idx >= 0:
                                check_content = lines[check_idx]
                                
                                # Found UPROPERTY, now verify it applies to current line
                                if 'UPROPERTY' in check_content:
                                    # Check if there's a variable declaration between the UPROPERTY and current line
                                    has_intervening_declaration = False
                                    for between_idx in range(check_idx + 1, i - 1):
                                        between_line = lines[between_idx].strip()
                                        # Skip if line is a comment
                                        if between_line.startswith('//') or between_line.startswith('/*'):
                                            continue
                                        # Skip if line is empty
                                        if not between_line:
                                            continue
                                        # If we find a line that matches a variable declaration, it's intervening
                                        if var_decl_pattern.match(between_line):
                                            has_intervening_declaration = True
                                            break
                                    
                                    # If no intervening declaration, this UPROPERTY applies to current line
                                    if not has_intervening_declaration:
                                        has_uproperty = True
                                        break
                                
                                # If we hit a variable declaration before finding UPROPERTY,
                                # the search should stop. Only stop if it's a real variable declaration.
                                if var_decl_pattern.match(lines[check_idx].strip()):
                                    break
                        
                        # If no UPROPERTY found, this might be an issue
                        if not has_uproperty:
                            # Additional heuristics to reduce false positives
                            # Skip if it looks like a typedef or using statement
                            if re.search(r'\b(?:typedef|using)\b', line):
                                continue
                            # Skip if preceded by UFUNCTION within last few lines
                            skip_ufunction = False
                            for check_line in range(max(0, i-1-5), i-1):
                                if 'UFUNCTION' in lines[check_line]:
                                    skip_ufunction = True
                                    break
                            
                            if skip_ufunction:
                                continue
                            
                            issues.append({
                                'file': str(filepath),
                                'line': i,
                                'content': line.strip(),
                                'severity': 'WARNING'
                            })
        
        return issues
    
    def scan_directory(self) -> Dict:
        """Scan all header files in the source directory."""
        print(f"Scanning {self.source_dir} for UPROPERTY issues...")
        
        header_files = list(self.source_dir.rglob('*.h'))
        self.total_files = len(header_files)
        
        all_issues = []
        files_with_issues = set()
        
        for filepath in header_files:
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
    
    def print_report(self, results: Dict):
        """Print a formatted report of findings."""
        print("\n" + "="*80)
        print("UPROPERTY Validation Report")
        print("="*80)
        print(f"Total files scanned: {results['total_files']}")
        print(f"Files with potential issues: {results['files_with_issues']}")
        print(f"Total potential issues found: {results['total_issues']}")
        print("="*80)
        
        if results['total_issues'] == 0:
            print("\n✅ No issues found! All UObject pointers appear to have UPROPERTY.")
            return
        
        print("\n⚠️  Potential Issues Found:\n")
        print("Note: This is an automated scan and may contain false positives.")
        print("Please manually review each finding.\n")
        
        current_file = None
        for issue in results['issues']:
            if issue['file'] != current_file:
                current_file = issue['file']
                print(f"\n📁 {current_file}")
            
            print(f"  Line {issue['line']}: {issue['content']}")
            
            if self.verbose:
                print(f"    Severity: {issue['severity']}")
        
        print("\n" + "="*80)
        print("Recommendations:")
        print("1. Manually review each finding to confirm if UPROPERTY is needed")
        print("2. Add UPROPERTY() macro for member variables (even private ones)")
        print("3. Example: UPROPERTY() UFactionDataAsset* CachedFaction;")
        print("4. For private pointers, consider: UPROPERTY(Transient)")
        print("="*80 + "\n")
    
    def export_results(self, output_file: str):
        """Export results to a file for review."""
        with open(output_file, 'w') as f:
            f.write("UPROPERTY Validation Results\n")
            f.write("="*80 + "\n\n")
            f.write(f"Total files scanned: {self.total_files}\n")
            f.write(f"Files with potential issues: {self.files_with_issues}\n")
            f.write(f"Total potential issues: {len(self.issues)}\n\n")
            
            for issue in self.issues:
                f.write(f"{issue['file']}:{issue['line']} - {issue['content']}\n")
        
        print(f"Results exported to {output_file}")


def main():
    import argparse
    
    parser = argparse.ArgumentParser(description='Check for missing UPROPERTY macros')
    parser.add_argument('--source-dir', default='Source/Adastrea',
                       help='Source directory to scan (default: Source/Adastrea)')
    parser.add_argument('--verbose', action='store_true',
                       help='Show detailed analysis')
    parser.add_argument('--export', help='Export results to file')
    
    args = parser.parse_args()
    
    # Get the repository root
    script_dir = Path(__file__).parent
    repo_root = script_dir.parent
    source_dir = repo_root / args.source_dir
    
    if not source_dir.exists():
        print(f"Error: Source directory not found: {source_dir}")
        sys.exit(1)
    
    checker = UPropertyChecker(source_dir, verbose=args.verbose)
    results = checker.scan_directory()
    checker.print_report(results)
    
    if args.export:
        checker.export_results(args.export)
    
    # Exit with error code if issues found (for CI/CD)
    if results['total_issues'] > 0:
        sys.exit(1)
    else:
        sys.exit(0)


if __name__ == '__main__':
    main()
