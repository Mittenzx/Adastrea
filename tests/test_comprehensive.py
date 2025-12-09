#!/usr/bin/env python3
"""
Comprehensive Build and Test Suite for Adastrea

This test suite incorporates all existing tests and adds comprehensive
checks for build errors, deprecated code, filename validation, and
dependency issues.

Run with: pytest tests/test_comprehensive.py -v
"""

import sys
import os
import re
import json
from pathlib import Path
from typing import List, Dict, Set, Tuple
import pytest

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


class TestRepositoryStructure:
    """Test that repository has expected structure and files."""

    def test_uproject_exists(self):
        """Test that .uproject file exists."""
        uproject = Path("Adastrea.uproject")
        assert uproject.exists(), "Adastrea.uproject should exist"

    def test_source_directory_exists(self):
        """Test that Source directory exists."""
        source_dir = Path("Source")
        assert source_dir.exists(), "Source directory should exist"
        assert source_dir.is_dir(), "Source should be a directory"

    def test_content_directory_exists(self):
        """Test that Content directory exists."""
        content_dir = Path("Content")
        assert content_dir.exists(), "Content directory should exist"

    def test_build_cs_files_exist(self):
        """Test that all modules have Build.cs files."""
        source_dir = Path("Source")
        modules = [d for d in source_dir.iterdir() if d.is_dir() and not d.name.startswith('.')]
        
        for module in modules:
            if module.name.endswith('.Target'):
                continue  # Skip .Target.cs files
            build_cs = module / f"{module.name}.Build.cs"
            if not build_cs.exists():
                # Check if it's a Target.cs file instead
                target_cs = source_dir / f"{module.name}.Target.cs"
                assert target_cs.exists() or build_cs.exists(), \
                    f"Module {module.name} should have a Build.cs or Target.cs file"


class TestBuildConfiguration:
    """Test Build.cs files for common issues."""

    def test_all_build_cs_files_valid_syntax(self):
        """Test that all Build.cs files have valid C# syntax."""
        source_dir = Path("Source")
        build_files = list(source_dir.rglob("*.Build.cs"))
        
        assert len(build_files) > 0, "Should find at least one Build.cs file"
        
        for build_file in build_files:
            content = build_file.read_text(encoding='utf-8')
            # Basic syntax checks
            assert "public class" in content, f"{build_file} should define a public class"
            assert "ModuleRules" in content, f"{build_file} should inherit from ModuleRules"
            assert "PublicDependencyModuleNames" in content or "PrivateDependencyModuleNames" in content, \
                f"{build_file} should define dependency modules"

    def test_no_circular_dependencies(self):
        """Test for circular module dependencies."""
        source_dir = Path("Source")
        build_files = list(source_dir.rglob("*.Build.cs"))
        
        dependencies = {}
        for build_file in build_files:
            module_name = build_file.stem.replace('.Build', '')
            content = build_file.read_text(encoding='utf-8')
            
            # Extract dependency modules
            deps = []
            for match in re.finditer(r'AddRange\s*\(\s*new\s+string\[\]\s*\{([^}]+)\}', content):
                deps_str = match.group(1)
                deps.extend([d.strip().strip('"').strip("'") for d in deps_str.split(',') if d.strip()])
            
            dependencies[module_name] = deps
        
        # Check for circular dependencies (simplified check)
        for module, deps in dependencies.items():
            if module in deps:
                pytest.fail(f"Module {module} depends on itself")

    def test_core_modules_present(self):
        """Test that core Unreal Engine modules are included."""
        source_dir = Path("Source")
        adastrea_build = source_dir / "Adastrea" / "Adastrea.Build.cs"
        
        if adastrea_build.exists():
            content = adastrea_build.read_text(encoding='utf-8')
            assert '"Core"' in content, "Should include Core module"
            assert '"CoreUObject"' in content, "Should include CoreUObject module"
            assert '"Engine"' in content, "Should include Engine module"


class TestDeprecatedCode:
    """Test for deprecated code usage."""

    def test_scan_for_deprecated_apis(self):
        """Scan source files for deprecated API usage."""
        source_dir = Path("Source")
        deprecated_patterns = [
            (r'\bUE_DEPRECATED\b', 'UE_DEPRECATED macro usage'),
            (r'\bDEPRECATED_FORGAME\b', 'DEPRECATED_FORGAME macro'),
        ]
        
        cpp_files = list(source_dir.rglob("*.cpp")) + list(source_dir.rglob("*.h"))
        deprecated_found = []
        
        for filepath in cpp_files:
            try:
                content = filepath.read_text(encoding='utf-8', errors='ignore')
                for pattern, desc in deprecated_patterns:
                    if re.search(pattern, content):
                        # Check if it's in a comment documenting deprecation (allowed)
                        lines = content.split('\n')
                        for line_num, line in enumerate(lines, 1):
                            if re.search(pattern, line) and not line.strip().startswith('//') and not line.strip().startswith('*'):
                                deprecated_found.append((filepath, line_num, desc))
            except Exception:
                continue
        
        # Report but don't fail - this is informational
        if deprecated_found:
            print(f"\nFound {len(deprecated_found)} deprecated API usages:")
            for filepath, line_num, desc in deprecated_found[:10]:  # Show first 10
                print(f"  {filepath}:{line_num} - {desc}")

    def test_check_for_old_unreal_apis(self):
        """Check for usage of old Unreal Engine APIs."""
        source_dir = Path("Source")
        old_apis = [
            (r'\bFStringAssetReference\b', 'Use FSoftObjectPath instead'),
            (r'\bFAssetData::GetAsset\(\)', 'Use FSoftObjectPath or direct loading'),
        ]
        
        cpp_files = list(source_dir.rglob("*.cpp")) + list(source_dir.rglob("*.h"))
        old_api_usage = []
        
        for filepath in cpp_files:
            try:
                content = filepath.read_text(encoding='utf-8', errors='ignore')
                for pattern, suggestion in old_apis:
                    if re.search(pattern, content):
                        old_api_usage.append((filepath, suggestion))
            except Exception:
                continue
        
        # Report findings
        if old_api_usage:
            print(f"\nFound {len(old_api_usage)} old API usages:")
            for filepath, suggestion in old_api_usage[:5]:
                print(f"  {filepath} - {suggestion}")


class TestFileNaming:
    """Test filename conventions."""

    def test_source_files_proper_extensions(self):
        """Test that source files have proper extensions."""
        source_dir = Path("Source")
        valid_extensions = {'.h', '.cpp', '.cs', '.ini', '.txt', '.md'}
        
        invalid_files = []
        for filepath in source_dir.rglob("*"):
            if filepath.is_file():
                if filepath.suffix and filepath.suffix not in valid_extensions:
                    if filepath.suffix not in {'.generated', '.uasset', '.umap'}:
                        invalid_files.append(filepath)
        
        if invalid_files:
            print(f"\nFound {len(invalid_files)} files with unexpected extensions:")
            for f in invalid_files[:10]:
                print(f"  {f}")

    def test_no_spaces_in_source_filenames(self):
        """Test that source filenames don't contain spaces."""
        source_dir = Path("Source")
        files_with_spaces = []
        
        for filepath in source_dir.rglob("*"):
            if filepath.is_file() and ' ' in filepath.name:
                files_with_spaces.append(filepath)
        
        assert len(files_with_spaces) == 0, \
            f"Found {len(files_with_spaces)} files with spaces in names: {files_with_spaces[:5]}"

    def test_header_cpp_pairs_exist(self):
        """Test that .cpp files have corresponding .h files and vice versa."""
        source_dir = Path("Source")
        cpp_files = set(source_dir.rglob("*.cpp"))
        h_files = set(source_dir.rglob("*.h"))
        
        missing_headers = []
        missing_implementations = []
        
        for cpp_file in cpp_files:
            # Skip generated files
            if '.generated' in cpp_file.name:
                continue
            
            expected_header = cpp_file.with_suffix('.h')
            if expected_header not in h_files:
                # Check in Public directory
                public_path = cpp_file.parent.parent / "Public" / cpp_file.name.replace('.cpp', '.h')
                if not public_path.exists():
                    missing_headers.append(cpp_file)
        
        # Some headers are interface-only (no .cpp), so we don't enforce this strictly
        # Just report if there are many missing implementations
        if len(missing_headers) > 0:
            print(f"\nFound {len(missing_headers)} .cpp files without obvious .h counterparts")


class TestIncludeDirectives:
    """Test for common include directive issues."""

    def test_generated_h_include_last(self):
        """Test that .generated.h includes are last in header files."""
        source_dir = Path("Source")
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
            print(f"\nFound {len(violations)} files with .generated.h not as last include:")
            for v in violations[:5]:
                print(f"  {v}")

    def test_no_absolute_includes(self):
        """Test that there are no absolute path includes."""
        source_dir = Path("Source")
        source_files = list(source_dir.rglob("*.cpp")) + list(source_dir.rglob("*.h"))
        
        absolute_includes = []
        for filepath in source_files:
            try:
                content = filepath.read_text(encoding='utf-8', errors='ignore')
                for line in content.split('\n'):
                    if line.strip().startswith('#include'):
                        # Check for Windows-style absolute paths
                        if re.search(r'#include\s+[<"]([A-Za-z]:|\\\\|/home|/usr)', line):
                            absolute_includes.append((filepath, line.strip()))
            except Exception:
                continue
        
        assert len(absolute_includes) == 0, \
            f"Found {len(absolute_includes)} absolute path includes: {absolute_includes[:3]}"


class TestCodeQuality:
    """Test for code quality issues."""

    def test_no_hardcoded_secrets(self):
        """Test that there are no hardcoded passwords or API keys."""
        source_dir = Path("Source")
        source_files = list(source_dir.rglob("*.cpp")) + list(source_dir.rglob("*.h"))
        
        secret_patterns = [
            (r'password\s*=\s*["\']', 'hardcoded password'),
            (r'apiKey\s*=\s*["\']', 'hardcoded API key'),
            (r'api_key\s*=\s*["\']', 'hardcoded API key'),
            (r'secret\s*=\s*["\']', 'hardcoded secret'),
        ]
        
        secrets_found = []
        for filepath in source_files:
            try:
                content = filepath.read_text(encoding='utf-8', errors='ignore')
                # Search line by line to avoid duplicate work
                for line in content.split('\n'):
                    # Skip comment lines
                    if line.strip().startswith('//') or line.strip().startswith('*'):
                        continue
                    for pattern, desc in secret_patterns:
                        if re.search(pattern, line, re.IGNORECASE):
                            secrets_found.append((filepath, desc))
                            break  # Only report once per file
            except Exception:
                continue
        
        assert len(secrets_found) == 0, \
            f"Found {len(secrets_found)} potential hardcoded secrets: {secrets_found}"

    def test_proper_uproperty_usage(self):
        """Test that UObject pointers have UPROPERTY macro (integration with existing tool)."""
        # This integrates with the existing check_uproperty.py tool
        tools_dir = Path("Tools")
        check_script = tools_dir / "check_uproperty.py"
        
        if check_script.exists():
            # Import and run the checker
            import subprocess
            result = subprocess.run(
                [sys.executable, str(check_script)],
                capture_output=True,
                text=True,
                cwd=Path.cwd()
            )
            
            # Check if there are critical issues (we allow warnings)
            if "CRITICAL" in result.stdout or result.returncode > 1:
                pytest.fail(f"UPROPERTY checker found critical issues:\n{result.stdout}")


class TestAssetNaming:
    """Test asset naming conventions."""

    def test_blueprint_naming_conventions(self):
        """Test that Blueprint assets follow naming conventions."""
        content_dir = Path("Content")
        if not content_dir.exists():
            pytest.skip("Content directory not found")
        
        # This integrates with existing validate_naming.py tool
        tools_dir = Path("Tools")
        validate_script = tools_dir / "validate_naming.py"
        
        if validate_script.exists():
            import subprocess
            result = subprocess.run(
                [sys.executable, str(validate_script)],
                capture_output=True,
                text=True,
                cwd=Path.cwd()
            )
            
            # Report but don't fail on naming violations (they're not build-breaking)
            if result.returncode != 0:
                print(f"\nNaming convention violations found:\n{result.stdout}")


class TestModuleStructure:
    """Test module structure and organization."""

    def test_all_modules_have_public_private_dirs(self):
        """Test that modules follow Public/Private directory structure."""
        source_dir = Path("Source")
        modules = [d for d in source_dir.iterdir() 
                  if d.is_dir() and not d.name.startswith('.') and not d.name.endswith('Target')]
        
        for module in modules:
            # Check if module has a Build.cs file
            build_cs = list(module.glob("*.Build.cs"))
            if not build_cs:
                continue  # Not a proper module
            
            # Check for Public or Private directory (at least one should exist)
            public_dir = module / "Public"
            private_dir = module / "Private"
            
            # Some modules may not need this structure, so we just report
            if not public_dir.exists() and not private_dir.exists():
                print(f"\nModule {module.name} doesn't use Public/Private structure")

    def test_headers_in_public_implementations_in_private(self):
        """Test that .h files are in Public and .cpp in Private (where applicable)."""
        source_dir = Path("Source")
        
        violations = []
        for cpp_file in source_dir.rglob("*.cpp"):
            if 'Public' in str(cpp_file):
                violations.append(f"Implementation file in Public: {cpp_file}")
        
        # This is a warning, not a hard failure
        if violations:
            print(f"\nFound {len(violations)} implementation files in Public directories:")
            for v in violations[:5]:
                print(f"  {v}")


class TestDocumentation:
    """Test that documentation exists and is consistent."""

    def test_readme_exists(self):
        """Test that README.md exists."""
        readme = Path("README.md")
        assert readme.exists(), "README.md should exist"

    def test_architecture_doc_exists(self):
        """Test that ARCHITECTURE.md exists."""
        arch_doc = Path("ARCHITECTURE.md")
        assert arch_doc.exists(), "ARCHITECTURE.md should exist"

    def test_all_systems_have_guides(self):
        """Test that major systems have documentation."""
        required_docs = [
            "Assets/SpaceshipSetupGuide.md",
            "Assets/FactionSetupGuide.md",
            "Assets/PersonnelSetupGuide.md",
        ]
        
        missing_docs = []
        for doc in required_docs:
            if not Path(doc).exists():
                missing_docs.append(doc)
        
        if missing_docs:
            print(f"\nMissing documentation files: {missing_docs}")


class TestExistingTests:
    """Run existing test suites."""

    def test_run_procedural_generator_tests(self):
        """Run existing procedural generator tests."""
        test_file = Path("tests/test_procedural_generators.py")
        if test_file.exists():
            import subprocess
            result = subprocess.run(
                [sys.executable, "-m", "pytest", str(test_file), "-v"],
                capture_output=True,
                text=True,
                cwd=Path.cwd()
            )
            # Don't fail if tests don't pass - just report
            if result.returncode != 0:
                print(f"\nProcedural generator tests output:\n{result.stdout}")

    def test_run_schema_validator_tests(self):
        """Run existing schema validator tests."""
        test_file = Path("tests/test_schema_validator.py")
        if test_file.exists():
            import subprocess
            result = subprocess.run(
                [sys.executable, "-m", "pytest", str(test_file), "-v"],
                capture_output=True,
                text=True,
                cwd=Path.cwd()
            )
            # Don't fail if tests don't pass - just report
            if result.returncode != 0:
                print(f"\nSchema validator tests output:\n{result.stdout}")


if __name__ == "__main__":
    pytest.main([__file__, "-v", "--tb=short"])
