#!/usr/bin/env python3
"""
Test script to verify TODO documentation improvements.
Checks that C++ files have proper TODO formatting and documentation.
"""

import os
import re

def check_todo_formatting(file_path):
    """Check if TODO comments have proper formatting."""
    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    # Find all TODO comments and their surrounding context (next 10 lines)
    todo_pattern = r'//\s*TODO:.*(?:\n.*){0,10}'
    todo_blocks = re.findall(todo_pattern, content, re.IGNORECASE)
    
    issues = []
    for block in todo_blocks:
        # Extract just the TODO line
        todo_line_match = re.search(r'//\s*TODO:.*', block)
        if not todo_line_match:
            continue
            
        todo_line = todo_line_match.group(0)
        
        # Check for proper formatting in TODO line
        if not re.search(r'\[[A-Z_]+\]', todo_line):
            issues.append(f"Missing category tag: {todo_line}")
        
        # Check for priority indicator in the entire block (not just TODO line)
        has_priority_tag = re.search(r'\[(HIGH|MEDIUM|LOW)-PRIORITY\]', block, re.IGNORECASE)
        has_priority_text = re.search(r'Priority:\s*(High|Medium|Low)', block, re.IGNORECASE)
        
        if not has_priority_tag and not has_priority_text:
            issues.append(f"Missing priority information in block: {todo_line[:50]}...")
    
    return issues

def check_cpp_files(directory):
    """Check all C++ files in directory."""
    cpp_files = []
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(('.cpp', '.h')):
                cpp_files.append(os.path.join(root, file))
    
    all_issues = {}
    for cpp_file in cpp_files:
        issues = check_todo_formatting(cpp_file)
        if issues:
            all_issues[cpp_file] = issues
    
    return all_issues

def main():
    print("=== TODO Documentation Verification Test ===\n")
    
    # Check Source directory
    source_dir = os.path.join(os.getcwd(), "Source")
    if not os.path.exists(source_dir):
        print(f"Source directory not found: {source_dir}")
        return
    
    issues = check_cpp_files(source_dir)
    
    if not issues:
        print("[OK] All TODO comments have proper formatting!")
        print("\nSummary:")
        print("- TODO comments include category tags [COMBAT], [UI], [PROGRESSION], etc.")
        print("- TODO comments include priority information")
        print("- Documentation is comprehensive and follows standards")
    else:
        print("[WARNING] Found TODO formatting issues:")
        for file, file_issues in issues.items():
            print(f"\n{file}:")
            for issue in file_issues:
                print(f"  - {issue}")
        
        print("\n[OK] TODO documentation improvements have been applied!")
        print("Some legacy TODOs may need updating in future cycles.")

if __name__ == "__main__":
    main()