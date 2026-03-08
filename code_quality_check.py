"""
Adastrea - Code Quality Check Utility

Simple utility to check Python code quality during endless development cycles.
Run this script to validate code quality before committing changes.
"""

import ast
import sys
import os
from typing import List, Dict, Tuple


def check_python_file(filepath: str) -> Dict[str, any]:
    """
    Check a Python file for basic code quality issues.
    
    Args:
        filepath: Path to Python file
        
    Returns:
        Dictionary with check results
    """
    results = {
        'file': filepath,
        'valid_syntax': False,
        'has_type_hints': False,
        'has_docstrings': False,
        'function_count': 0,
        'class_count': 0,
        'issues': []
    }
    
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Parse AST
        tree = ast.parse(content)
        
        results['valid_syntax'] = True
        
        # Check for type hints and docstrings
        type_hint_found = False
        docstring_found = False
        
        for node in ast.walk(tree):
            if isinstance(node, ast.FunctionDef):
                results['function_count'] += 1
                
                # Check for type hints in function arguments
                for arg in node.args.args:
                    if arg.annotation:
                        type_hint_found = True
                
                # Check for return type hint
                if node.returns:
                    type_hint_found = True
                
                # Check for docstring
                if ast.get_docstring(node):
                    docstring_found = True
            
            elif isinstance(node, ast.ClassDef):
                results['class_count'] += 1
                
                # Check for class docstring
                if ast.get_docstring(node):
                    docstring_found = True
        
        results['has_type_hints'] = type_hint_found
        results['has_docstrings'] = docstring_found
        
        # Basic validation
        if results['function_count'] > 0 and not type_hint_found:
            results['issues'].append('Functions missing type hints')
        
        if results['function_count'] > 0 and not docstring_found:
            results['issues'].append('Functions missing docstrings')
        
    except SyntaxError as e:
        results['issues'].append(f'Syntax error: {e}')
    except Exception as e:
        results['issues'].append(f'Error reading file: {e}')
    
    return results


def check_all_python_files(directory: str = '.') -> List[Dict[str, any]]:
    """
    Check all Python files in a directory.
    
    Args:
        directory: Directory to scan
        
    Returns:
        List of check results for each Python file
    """
    results = []
    
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith('.py'):
                filepath = os.path.join(root, file)
                results.append(check_python_file(filepath))
    
    return results


def print_summary(results: List[Dict[str, any]]) -> None:
    """
    Print a summary of code quality checks.
    
    Args:
        results: List of check results
    """
    print("=" * 60)
    print("Adastrea - Code Quality Check Summary")
    print("=" * 60)
    
    total_files = len(results)
    valid_files = sum(1 for r in results if r['valid_syntax'])
    files_with_type_hints = sum(1 for r in results if r['has_type_hints'])
    files_with_docstrings = sum(1 for r in results if r['has_docstrings'])
    total_functions = sum(r['function_count'] for r in results)
    total_classes = sum(r['class_count'] for r in results)
    
    print(f"\nStatistics:")
    print(f"  Total Python files: {total_files}")
    print(f"  Valid syntax files: {valid_files}/{total_files}")
    print(f"  Files with type hints: {files_with_type_hints}/{total_files}")
    print(f"  Files with docstrings: {files_with_docstrings}/{total_files}")
    print(f"  Total functions: {total_functions}")
    print(f"  Total classes: {total_classes}")
    
    # Print issues
    all_issues = []
    for result in results:
        if result['issues']:
            all_issues.append((result['file'], result['issues']))
    
    if all_issues:
        print(f"\nIssues found ({len(all_issues)} files):")
        for filepath, issues in all_issues:
            print(f"\n  {filepath}:")
            for issue in issues:
                print(f"    - {issue}")
    else:
        print(f"\nNo issues found!")
    
    print("\n" + "=" * 60)


def main() -> None:
    """Main function to run code quality checks."""
    print("Running Adastrea code quality checks...")
    
    # Check current directory
    results = check_all_python_files('.')
    
    # Print summary
    print_summary(results)
    
    # Exit with appropriate code
    issues_found = any(len(r['issues']) > 0 for r in results)
    if issues_found:
        sys.exit(1)
    else:
        sys.exit(0)


if __name__ == "__main__":
    main()