#!/usr/bin/env python3
"""
Build Error Detection Tests for Adastrea

This test suite can run standalone without pytest.
Checks for potential build errors including:
- Missing include files
- Invalid dependency configurations  
- Deprecated code usage
- Filename issues

Run with: python tests/test_build_errors.py
Or with pytest: pytest tests/test_build_errors.py -v
"""

import sys
import os
import re
from pathlib import Path
from typing import List, Dict, Set, Tuple


class BuildErrorChecker:
    """Standalone checker for build errors."""

    def __init__(self):
        self.errors = []
        self.warnings = []
        self.info = []
        self.project_root = Path(__file__).parent.parent

    def add_error(self, message: str):
        """Add an error."""
        self.errors.append(f"✗ ERROR: {message}")

    def add_warning(self, message: str):
        """Add a warning."""
        self.warnings.append(f"⚠ WARNING: {message}")

    def add_info(self, message: str):
        """Add an info message."""
        self.info.append(f"ℹ INFO: {message}")

    def check_include_files(self):
        """Check that included files exist."""
        print("\nChecking include files...")
        source_dir = self.project_root / "Source"
        
        if not source_dir.exists():
            self.add_error("Source directory not found")
            return
        
        cpp_files = list(source_dir.rglob("*.cpp")) + list(source_dir.rglob("*.h"))
        missing_includes = []
        
        for filepath in cpp_files:
            try:
                content = filepath.read_text(encoding='utf-8', errors='ignore')
                for line_num, line in enumerate(content.split('\n'), 1):
                    if line.strip().startswith('#include'):
                        # Extract include path
                        match = re.search(r'#include\s+[<"]([^>"]+)[>"]', line)
                        if match:
                            include_path = match.group(1)
                            
                            # Skip system includes
                            if include_path.startswith('Core') or include_path.startswith('Engine'):
                                continue
                            
                            # Check if it's a local include
                            if include_path.endswith('.h'):
                                # Try to find the file
                                found = False
                                for search_dir in [filepath.parent, source_dir]:
                                    test_path = search_dir / include_path
                                    if test_path.exists():
                                        found = True
                                        break
                                    
                                    # Try without directory prefix
                                    test_path = search_dir / Path(include_path).name
                                    if test_path.exists():
                                        found = True
                                        break
                                
                                if not found and '.generated' not in include_path:
                                    missing_includes.append((filepath, line_num, include_path))
            except Exception as e:
                self.add_warning(f"Could not read {filepath}: {e}")
        
        if missing_includes:
            self.add_warning(f"Found {len(missing_includes)} potentially missing includes (may be false positives)")
            for filepath, line_num, include_path in missing_includes[:5]:
                self.add_info(f"  {filepath.relative_to(self.project_root)}:{line_num} - {include_path}")
        else:
            print("  ✓ No obvious missing includes found")

    def check_deprecated_code(self):
        """Check for deprecated code usage."""
        print("\nChecking for deprecated code...")
        source_dir = self.project_root / "Source"
        
        deprecated_patterns = [
            (r'\bUE_DEPRECATED\b', 'UE_DEPRECATED macro'),
            (r'\bDEPRECATED_FORGAME\b', 'DEPRECATED_FORGAME macro'),
            (r'\bFStringAssetReference\b', 'FStringAssetReference (use FSoftObjectPath)'),
        ]
        
        cpp_files = list(source_dir.rglob("*.cpp")) + list(source_dir.rglob("*.h"))
        deprecated_found = []
        
        for filepath in cpp_files:
            try:
                content = filepath.read_text(encoding='utf-8', errors='ignore')
                for pattern, desc in deprecated_patterns:
                    matches = list(re.finditer(pattern, content))
                    if matches:
                        # Check if it's in a comment (allowed for documentation)
                        lines = content.split('\n')
                        for match in matches:
                            # Find line number
                            line_num = content[:match.start()].count('\n') + 1
                            line = lines[line_num - 1] if line_num <= len(lines) else ""
                            
                            if not line.strip().startswith('//') and not line.strip().startswith('*'):
                                deprecated_found.append((filepath, line_num, desc))
            except Exception:
                continue
        
        if deprecated_found:
            self.add_warning(f"Found {len(deprecated_found)} deprecated API usages")
            for filepath, line_num, desc in deprecated_found[:10]:
                self.add_info(f"  {filepath.relative_to(self.project_root)}:{line_num} - {desc}")
        else:
            print("  ✓ No deprecated code found")

    def check_build_cs_dependencies(self):
        """Check Build.cs files for dependency issues."""
        print("\nChecking Build.cs dependencies...")
        source_dir = self.project_root / "Source"
        
        build_files = list(source_dir.rglob("*.Build.cs"))
        if not build_files:
            self.add_error("No Build.cs files found")
            return
        
        print(f"  Found {len(build_files)} Build.cs files")
        
        dependencies = {}
        for build_file in build_files:
            module_name = build_file.stem.replace('.Build', '')
            content = build_file.read_text(encoding='utf-8')
            
            # Check for basic syntax
            if "public class" not in content:
                self.add_error(f"{build_file.name} missing public class declaration")
            
            if "ModuleRules" not in content:
                self.add_error(f"{build_file.name} doesn't inherit from ModuleRules")
            
            # Extract dependencies (only from PublicDependencyModuleNames and PrivateDependencyModuleNames)
            deps = []
            # Look for PublicDependencyModuleNames and PrivateDependencyModuleNames specifically
            for match in re.finditer(r'(Public|Private)DependencyModuleNames\.AddRange\s*\(\s*new\s+string\[\]\s*\{([^}]+)\}', content):
                deps_str = match.group(2)
                deps.extend([d.strip().strip('"').strip("'") for d in deps_str.split(',') if d.strip()])
            
            dependencies[module_name] = deps
            
            # Check for self-dependency
            if module_name in deps:
                self.add_error(f"Module {module_name} depends on itself")
        
        print(f"  ✓ Analyzed {len(dependencies)} module dependencies")

    def check_filename_conventions(self):
        """Check filename conventions."""
        print("\nChecking filename conventions...")
        source_dir = self.project_root / "Source"
        
        # Check for spaces in filenames
        files_with_spaces = []
        for filepath in source_dir.rglob("*"):
            if filepath.is_file() and ' ' in filepath.name:
                files_with_spaces.append(filepath)
        
        if files_with_spaces:
            self.add_error(f"Found {len(files_with_spaces)} files with spaces in names")
            for f in files_with_spaces[:5]:
                self.add_info(f"  {f.relative_to(self.project_root)}")
        else:
            print("  ✓ No files with spaces in names")
        
        # Check for proper extensions
        valid_extensions = {'.h', '.cpp', '.cs', '.ini', '.txt', '.md', '.json'}
        invalid_files = []
        
        for filepath in source_dir.rglob("*"):
            if filepath.is_file() and filepath.suffix:
                if filepath.suffix not in valid_extensions:
                    if filepath.suffix not in {'.generated', '.uasset', '.umap'}:
                        invalid_files.append(filepath)
        
        if invalid_files:
            self.add_warning(f"Found {len(invalid_files)} files with unexpected extensions")
            for f in invalid_files[:5]:
                self.add_info(f"  {f.relative_to(self.project_root)}")
        else:
            print("  ✓ All files have valid extensions")

    def check_header_guards(self):
        """Check that header files have proper include guards."""
        print("\nChecking header guards...")
        source_dir = self.project_root / "Source"
        
        h_files = list(source_dir.rglob("*.h"))
        missing_guards = []
        
        for h_file in h_files:
            try:
                content = h_file.read_text(encoding='utf-8', errors='ignore')
                
                # Check for #pragma once or traditional guards
                has_pragma_once = '#pragma once' in content
                has_ifndef_guard = re.search(r'#ifndef\s+\w+_H', content)
                
                if not has_pragma_once and not has_ifndef_guard:
                    missing_guards.append(h_file)
            except Exception:
                continue
        
        if missing_guards:
            self.add_warning(f"Found {len(missing_guards)} headers without include guards")
            for h in missing_guards[:5]:
                self.add_info(f"  {h.relative_to(self.project_root)}")
        else:
            print("  ✓ All headers have include guards")

    def check_generated_h_includes(self):
        """Check that .generated.h includes are last."""
        print("\nChecking .generated.h include order...")
        source_dir = self.project_root / "Source"
        
        h_files = list(source_dir.rglob("*.h"))
        violations = []
        
        for h_file in h_files:
            try:
                content = h_file.read_text(encoding='utf-8', errors='ignore')
                lines = content.split('\n')
                
                generated_include_line = -1
                last_include_line = -1
                
                for i, line in enumerate(lines):
                    if line.strip().startswith('#include'):
                        last_include_line = i
                        if '.generated.h' in line:
                            generated_include_line = i
                
                if generated_include_line != -1 and generated_include_line != last_include_line:
                    violations.append(h_file)
            except Exception:
                continue
        
        if violations:
            self.add_warning(f"Found {len(violations)} files with .generated.h not as last include")
            for v in violations[:5]:
                self.add_info(f"  {v.relative_to(self.project_root)}")
        else:
            print("  ✓ All .generated.h includes are properly ordered")

    def run_all_checks(self) -> bool:
        """Run all checks and return True if no errors."""
        print("=" * 70)
        print("BUILD ERROR DETECTION TESTS")
        print("=" * 70)
        print(f"Project Root: {self.project_root}")
        
        self.check_build_cs_dependencies()
        self.check_filename_conventions()
        self.check_header_guards()
        self.check_generated_h_includes()
        self.check_include_files()
        self.check_deprecated_code()
        
        print("\n" + "=" * 70)
        print("RESULTS")
        print("=" * 70)
        
        if self.errors:
            print(f"\n❌ ERRORS ({len(self.errors)}):")
            for error in self.errors:
                print(error)
        
        if self.warnings:
            print(f"\n⚠️  WARNINGS ({len(self.warnings)}):")
            for warning in self.warnings:
                print(warning)
        
        if self.info:
            print(f"\n📋 Details ({len(self.info)}):")
            for info_msg in self.info[:20]:  # Show first 20
                print(info_msg)
            if len(self.info) > 20:
                print(f"  ... and {len(self.info) - 20} more")
        
        print("\n" + "=" * 70)
        
        if not self.errors:
            print("✅ All critical checks passed!")
            return True
        else:
            print("❌ Some checks failed. Please review the errors above.")
            return False


def main():
    """Main entry point."""
    checker = BuildErrorChecker()
    success = checker.run_all_checks()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
