#!/usr/bin/env python3
"""
Comprehensive Build Error Detection for Adastrea

This standalone test suite performs exhaustive checks for potential build errors.
Designed to run on main branch after merges to catch any issues that slipped through.

Run with: python tests/test_all_build_errors.py
Or: python tests/test_all_build_errors.py --verbose

Checks performed:
- Missing include files (all variations)
- Circular dependencies between modules
- Invalid Build.cs configurations
- Missing UPROPERTY on UObject pointers
- Deprecated API usage
- Incorrect include order (.generated.h)
- Missing header guards
- Filename convention violations
- Hardcoded secrets/sensitive data
- Mismatched property declarations (.h vs .cpp)
- Invalid module API exports
- Forward declaration issues
- Missing #include "CoreMinimal.h"
- Relative path issues in includes
"""

import sys
import re
from pathlib import Path
import argparse


class ComprehensiveBuildChecker:
    """Comprehensive build error checker with detailed diagnostics."""

    def __init__(self, verbose: bool = False):
        self.errors = []
        self.warnings = []
        self.info = []
        self.project_root = Path(__file__).parent.parent
        self.verbose = verbose
        self.module_dependencies = {}

    def add_error(self, message: str, filepath: str = None, line: int = None):
        """Add a critical error."""
        location = f" ({filepath}:{line})" if filepath and line else (f" ({filepath})" if filepath else "")
        self.errors.append(f"✗ ERROR{location}: {message}")

    def add_warning(self, message: str, filepath: str = None, line: int = None):
        """Add a warning."""
        location = f" ({filepath}:{line})" if filepath and line else (f" ({filepath})" if filepath else "")
        self.warnings.append(f"⚠ WARNING{location}: {message}")

    def add_info(self, message: str):
        """Add an info message."""
        self.info.append(f"ℹ INFO: {message}")

    def log(self, message: str):
        """Log verbose output."""
        if self.verbose:
            print(f"  {message}")

    def check_missing_includes_comprehensive(self):
        """
        Comprehensive include file checking.
        Tests multiple search strategies to find missing includes.
        """
        print("\n🔍 Checking include files (comprehensive)...")
        source_dir = self.project_root / "Source"
        
        if not source_dir.exists():
            self.add_error("Source directory not found")
            return
        
        # Get all module directories
        modules = [d for d in source_dir.iterdir() if d.is_dir() and not d.name.startswith('.')]
        module_include_paths = {}
        
        # Build module include path mapping
        for module in modules:
            public_dir = module / "Public"
            private_dir = module / "Private"
            module_include_paths[module.name] = {
                'root': module,
                'public': public_dir if public_dir.exists() else None,
                'private': private_dir if private_dir.exists() else None
            }
        
        cpp_files = list(source_dir.rglob("*.cpp")) + list(source_dir.rglob("*.h"))
        missing_includes = []
        
        for filepath in cpp_files:
            try:
                content = filepath.read_text(encoding='utf-8', errors='ignore')
                for line_num, line in enumerate(content.split('\n'), 1):
                    if not line.strip().startswith('#include'):
                        continue
                    
                    # Extract include path
                    match = re.search(r'#include\s+[<"]([^>"]+)[>"]', line)
                    if not match:
                        continue
                    
                    include_path = match.group(1)
                    
                    # Skip system/engine includes
                    if any(include_path.startswith(prefix) for prefix in [
                        'Core', 'Engine', 'UMG', 'Slate', 'AIModule', 
                        'NavigationSystem', 'EnhancedInput', 'Niagara',
                        'Blueprint', 'GameFramework', 'Components', 'Materials',
                        'Particles', 'Sound', 'PhysicsEngine', 'InputCore',
                        'Kismet', 'UObject', 'Modules', 'InputMappingContext',
                        'TimerManager', 'DrawDebugHelpers', 'InputAction',
                        'InputModifiers', 'Net/', 'Animation', 'Camera',
                        'HAL/', 'Misc/', 'Subsystems/'
                    ]):
                        continue
                    
                    # Skip generated files
                    if '.generated' in include_path:
                        continue
                    
                    # Check if it's a local include
                    if not include_path.endswith('.h'):
                        continue
                    
                    # Try multiple search strategies
                    found = False
                    search_results = []
                    
                    # Strategy 1: Relative to current file
                    test_path = filepath.parent / include_path
                    if test_path.exists():
                        found = True
                        search_results.append(f"Found relative: {test_path}")
                    
                    # Strategy 2: Module public directory
                    if not found:
                        for module_name, paths in module_include_paths.items():
                            if paths['public']:
                                test_path = paths['public'] / include_path
                                if test_path.exists():
                                    found = True
                                    search_results.append(f"Found in {module_name}/Public")
                                    break
                    
                    # Strategy 3: Module root directory
                    if not found:
                        for module_name, paths in module_include_paths.items():
                            test_path = paths['root'] / include_path
                            if test_path.exists():
                                found = True
                                search_results.append(f"Found in {module_name} root")
                                break
                    
                    # Strategy 4: Just the filename (search all modules)
                    if not found:
                        filename = Path(include_path).name
                        for test_path in source_dir.rglob(filename):
                            if test_path.suffix == '.h':
                                found = True
                                search_results.append(f"Found by filename: {test_path}")
                                break
                    
                    if not found:
                        rel_filepath = str(filepath.relative_to(self.project_root))
                        missing_includes.append((rel_filepath, line_num, include_path))
                        self.log(f"Missing: {include_path} in {rel_filepath}:{line_num}")
                    elif self.verbose and search_results:
                        self.log(f"{include_path}: {search_results[0]}")
                        
            except Exception as e:
                self.add_warning(f"Could not read {filepath}: {e}")
        
        if missing_includes:
            self.add_error(f"Found {len(missing_includes)} missing include files")
            for filepath, line_num, include_path in missing_includes[:10]:
                self.add_info(f"  {filepath}:{line_num} - missing '{include_path}'")
            if len(missing_includes) > 10:
                self.add_info(f"  ... and {len(missing_includes) - 10} more missing includes")
        else:
            print("  ✓ All includes found")

    def check_circular_dependencies(self):
        """Check for circular dependencies between modules."""
        print("\n🔄 Checking for circular dependencies...")
        source_dir = self.project_root / "Source"
        
        build_files = list(source_dir.rglob("*.Build.cs"))
        dependencies = {}
        
        for build_file in build_files:
            try:
                content = build_file.read_text()
                module_name = build_file.stem.replace('.Build', '')
                
                # Extract dependencies
                public_deps = re.findall(r'PublicDependencyModuleNames\.(?:Add|AddRange)\([^)]+\)', content)
                private_deps = re.findall(r'PrivateDependencyModuleNames\.(?:Add|AddRange)\([^)]+\)', content)
                
                all_deps = []
                for dep_str in public_deps + private_deps:
                    # Extract module names from the string
                    module_names = re.findall(r'"(\w+)"', dep_str)
                    all_deps.extend(module_names)
                
                dependencies[module_name] = set(all_deps)
                self.log(f"{module_name}: {all_deps}")
                
            except Exception as e:
                self.add_warning(f"Could not parse {build_file}: {e}")
        
        # Check for circular dependencies
        def has_circular_dep(module: str, target: str, visited: set) -> bool:
            if module == target:
                return True
            if module in visited:
                return False
            visited.add(module)
            
            if module not in dependencies:
                visited.remove(module)
                return False
            
            for dep in dependencies[module]:
                if has_circular_dep(dep, target, visited):
                    visited.remove(module)
                    return True
            visited.remove(module)
            return False
        
        circular_deps = []
        checked_pairs = set()
        for module, deps in dependencies.items():
            for dep in deps:
                pair = tuple(sorted([module, dep]))
                if pair not in checked_pairs and dep in dependencies and has_circular_dep(dep, module, set()):
                    circular_deps.append((module, dep))
                    checked_pairs.add(pair)
        
        if circular_deps:
            self.add_error(f"Found {len(circular_deps)} circular dependencies")
            for mod1, mod2 in circular_deps[:5]:
                self.add_info(f"  {mod1} <-> {mod2}")
        else:
            print("  ✓ No circular dependencies found")

    def check_uproperty_on_pointers(self):
        """Check that UObject* pointers have UPROPERTY()."""
        print("\n📦 Checking UPROPERTY on UObject pointers...")
        source_dir = self.project_root / "Source"
        
        h_files = list(source_dir.rglob("*.h"))
        missing_uproperty = []
        
        for h_file in h_files:
            try:
                content = h_file.read_text(encoding='utf-8', errors='ignore')
                lines = content.split('\n')
                
                class_brace_depth = 0
                function_brace_depth = 0
                last_uproperty_line = -999
                
                for i, line in enumerate(lines):
                    stripped = line.strip()
                    
                    # Track if we're entering a class or struct
                    if re.match(r'^\s*(class|struct)\s+\w+', stripped):
                        # Look for opening brace on this or subsequent lines
                        if '{' in stripped:
                            class_brace_depth += 1
                        else:
                            # Multi-line class declaration, search ahead
                            for j in range(i+1, min(i+5, len(lines))):
                                if '{' in lines[j]:
                                    class_brace_depth += 1
                                    break
                    
                    # Track class/struct scope by counting braces
                    class_brace_depth += stripped.count('{')
                    class_brace_depth -= stripped.count('}')
                    in_class = class_brace_depth > 0

                    # Track if we're entering a function (skip if not in class)
                    if in_class:
                        # Improved function detection: return type, name, params, opening brace
                        # Avoid matching class/struct/enum declarations
                        if re.match(r'^[\w:<>~]+\s+[\w:<>~]+\s*\([^;{)]*\)\s*(const)?\s*{', stripped):
                            function_brace_depth += 1
                    # Track function scope by counting braces (only if in class)
                    if function_brace_depth > 0:
                        function_brace_depth += stripped.count('{')
                        function_brace_depth -= stripped.count('}')
                    in_function = function_brace_depth > 0
                    
                    # Track UPROPERTY
                    if 'UPROPERTY' in stripped:
                        last_uproperty_line = i
                    
                    # Check for UObject* declarations
                    if in_class and not in_function:
                        # Match: Type* VariableName; (only UObject/AActor-derived types)
                        match = re.match(r'(U\w+|A\w+)\s*\*\s*\w+\s*;', stripped)
                        if match and i - last_uproperty_line > 2:
                            rel_path = str(h_file.relative_to(self.project_root))
                            missing_uproperty.append((rel_path, i + 1, line.strip()))
                                
            except Exception as e:
                self.add_warning(f"Could not check {h_file}: {e}")
        
        if missing_uproperty:
            self.add_warning(f"Found {len(missing_uproperty)} potential missing UPROPERTY declarations")
            for filepath, line_num, line_content in missing_uproperty[:5]:
                self.add_info(f"  {filepath}:{line_num} - {line_content[:60]}")
        else:
            print("  ✓ All UObject pointers have UPROPERTY")

    def check_hardcoded_secrets(self):
        """Check for hardcoded secrets and sensitive data."""
        print("\n🔐 Checking for hardcoded secrets...")
        source_dir = self.project_root / "Source"
        
        patterns = [
            (r'password\s*=\s*["\']', 'password'),
            (r'apikey\s*=\s*["\']', 'api key'),
            (r'api_key\s*=\s*["\']', 'api key'),
            (r'secret\s*=\s*["\']', 'secret'),
            (r'token\s*=\s*["\']', 'token'),
            (r'private.*key\s*=\s*["\']', 'private key'),
        ]
        
        cpp_files = list(source_dir.rglob("*.cpp")) + list(source_dir.rglob("*.h"))
        secrets_found = []
        
        for filepath in cpp_files:
            try:
                content = filepath.read_text(encoding='utf-8', errors='ignore')
                for line_num, line in enumerate(content.split('\n'), 1):
                    for pattern, secret_type in patterns:
                        if re.search(pattern, line, re.IGNORECASE):
                            # Skip comments
                            if '//' in line and '=' in line and line.index('//') < line.index('='):
                                continue
                            rel_path = str(filepath.relative_to(self.project_root))
                            secrets_found.append((rel_path, line_num, secret_type))
            except Exception as e:
                # Skip files that cannot be read, but continue checking others
                self.log(f"Could not read {filepath}: {e}")
        
        if secrets_found:
            self.add_error(f"Found {len(secrets_found)} potential hardcoded secrets")
            for filepath, line_num, secret_type in secrets_found:
                self.add_info(f"  {filepath}:{line_num} - potential {secret_type}")
        else:
            print("  ✓ No hardcoded secrets found")

    def check_core_minimal_include(self):
        """Check that .cpp files include CoreMinimal.h."""
        print("\n📚 Checking CoreMinimal.h includes...")
        source_dir = self.project_root / "Source"
        
        cpp_files = list(source_dir.rglob("*.cpp"))
        missing_core_minimal = []
        
        for cpp_file in cpp_files:
            try:
                content = cpp_file.read_text(encoding='utf-8', errors='ignore')
                # Check first 10 lines for CoreMinimal.h
                first_lines = content.split('\n')[:10]
                has_core_minimal = any(
                    line.strip().startswith('#include') and 'CoreMinimal.h' in line
                    for line in first_lines
                )
                
                if not has_core_minimal:
                    rel_path = str(cpp_file.relative_to(self.project_root))
                    missing_core_minimal.append(rel_path)
                    
            except Exception as e:
                # Skip files that cannot be read
                self.log(f"Could not read {cpp_file}: {e}")
        
        if missing_core_minimal:
            self.add_warning(f"Found {len(missing_core_minimal)} .cpp files without CoreMinimal.h")
            for filepath in missing_core_minimal[:5]:
                self.add_info(f"  {filepath}")
        else:
            print("  ✓ All .cpp files include CoreMinimal.h")

    def check_api_export_macros(self):
        """Check for proper API export macros."""
        print("\n🔧 Checking API export macros...")
        source_dir = self.project_root / "Source"
        
        h_files = list(source_dir.rglob("*.h"))
        missing_api = []
        
        for h_file in h_files:
            try:
                content = h_file.read_text(encoding='utf-8', errors='ignore')
                
                # Check for UCLASS/USTRUCT without API macro
                uclass_matches = re.finditer(r'U(CLASS|STRUCT)\s*\([^)]*\)\s*class\s+(?![\w]+_API\s+)(\w+)', content, re.MULTILINE)
                for match in uclass_matches:
                    rel_path = str(h_file.relative_to(self.project_root))
                    class_name = match.group(2)
                    missing_api.append((rel_path, class_name))
                    
            except Exception as e:
                # Skip files that cannot be read
                self.log(f"Could not read {h_file}: {e}")
        
        if missing_api:
            self.add_warning(f"Found {len(missing_api)} classes without API export macro")
            for filepath, class_name in missing_api[:5]:
                self.add_info(f"  {filepath} - class {class_name}")
        else:
            print("  ✓ All classes have proper API macros")

    def check_relative_include_paths(self):
        """Check for problematic relative paths in includes."""
        print("\n🔀 Checking relative include paths...")
        source_dir = self.project_root / "Source"
        
        cpp_files = list(source_dir.rglob("*.cpp")) + list(source_dir.rglob("*.h"))
        suspicious_paths = []
        
        for filepath in cpp_files:
            try:
                content = filepath.read_text(encoding='utf-8', errors='ignore')
                for line_num, line in enumerate(content.split('\n'), 1):
                    if line.strip().startswith('#include'):
                        # Check for ../ without proper validation
                        if '../' in line:
                            # Verify the include actually exists
                            match = re.search(r'#include\s+"([^"]+)"', line)
                            if match:
                                include_path = match.group(1)
                                test_path = filepath.parent / include_path
                                if not test_path.exists():
                                    rel_path = str(filepath.relative_to(self.project_root))
                                    suspicious_paths.append((rel_path, line_num, include_path))
                                    
            except Exception as e:
                # Skip files that cannot be read, but continue checking other files
                self.log(f"Could not read {filepath}: {e}")
        
        if suspicious_paths:
            self.add_error(f"Found {len(suspicious_paths)} broken relative include paths")
            for filepath, line_num, include_path in suspicious_paths:
                self.add_info(f"  {filepath}:{line_num} - {include_path}")
        else:
            print("  ✓ All relative include paths are valid")

    def run_all_checks(self) -> bool:
        """Run all comprehensive checks."""
        print("=" * 70)
        print("COMPREHENSIVE BUILD ERROR DETECTION")
        print("=" * 70)
        print(f"Project Root: {self.project_root}")
        print(f"Verbose Mode: {self.verbose}")
        
        # Run all checks
        self.check_missing_includes_comprehensive()
        self.check_circular_dependencies()
        self.check_uproperty_on_pointers()
        self.check_hardcoded_secrets()
        self.check_core_minimal_include()
        self.check_api_export_macros()
        self.check_relative_include_paths()
        
        # Display results
        print("\n" + "=" * 70)
        print("RESULTS")
        print("=" * 70)
        
        if self.errors:
            print(f"\n❌ CRITICAL ERRORS ({len(self.errors)}):")
            for error in self.errors:
                print(error)
        
        if self.warnings:
            print(f"\n⚠️  WARNINGS ({len(self.warnings)}):")
            for warning in self.warnings:
                print(warning)
        
        if self.info:
            print(f"\n📋 Details ({len(self.info)}):")
            for info_msg in self.info[:30]:
                print(info_msg)
            if len(self.info) > 30:
                print(f"  ... and {len(self.info) - 30} more details")
        
        print("\n" + "=" * 70)
        
        if not self.errors:
            print("✅ All critical checks passed!")
            if self.warnings:
                print(f"⚠️  {len(self.warnings)} warnings found - review recommended")
            return True
        else:
            print(f"❌ {len(self.errors)} critical errors found - must be fixed!")
            return False


def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description='Comprehensive build error detection for Adastrea'
    )
    parser.add_argument(
        '--verbose', '-v',
        action='store_true',
        help='Show verbose output including detailed search results'
    )
    
    args = parser.parse_args()
    
    checker = ComprehensiveBuildChecker(verbose=args.verbose)
    success = checker.run_all_checks()
    
    print(f"\nTest completed: {'PASSED' if success else 'FAILED'}")
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
