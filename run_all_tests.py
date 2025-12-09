#!/usr/bin/env python3
"""
Unified Test Runner for Adastrea

This script runs all tests in the repository including:
- Python pytest tests
- C++ Automation tests (documentation only - requires Unreal Editor)
- Code quality checks
- Build validation
- Asset validation

Usage:
    python run_all_tests.py [--verbose] [--quick]

Options:
    --verbose: Show detailed output
    --quick: Run only fast checks (skip slow validation)
    --python-only: Run only Python tests
    --validation-only: Run only validation scripts
"""

import sys
import os
import subprocess
import argparse
from pathlib import Path
from typing import List, Tuple


class TestRunner:
    """Unified test runner for all Adastrea tests."""

    def __init__(self, verbose: bool = False, quick: bool = False):
        self.verbose = verbose
        self.quick = quick
        self.results = []
        self.project_root = Path(__file__).parent

    def run_command(self, cmd: List[str], description: str) -> Tuple[bool, str]:
        """Run a command and capture output."""
        print(f"\n{'=' * 70}")
        print(f"Running: {description}")
        print(f"{'=' * 70}")
        
        try:
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                cwd=self.project_root,
                timeout=300  # 5 minute timeout
            )
            
            success = result.returncode == 0
            output = result.stdout + result.stderr
            
            if self.verbose or not success:
                print(output)
            
            status = "✓ PASS" if success else "✗ FAIL"
            print(f"\n{status}: {description}")
            
            return success, output
        except subprocess.TimeoutExpired:
            print(f"✗ TIMEOUT: {description}")
            return False, "Command timed out"
        except Exception as e:
            print(f"✗ ERROR: {description} - {e}")
            return False, str(e)

    def run_pytest_tests(self):
        """Run all pytest tests."""
        print("\n" + "=" * 70)
        print("PYTHON TESTS")
        print("=" * 70)
        
        # First run standalone tests that don't require pytest
        standalone_tests = [
            ("tests/test_build_errors.py", "Build Error Detection"),
        ]
        
        for test_file, description in standalone_tests:
            if Path(test_file).exists():
                success, output = self.run_command(
                    [sys.executable, test_file],
                    description
                )
                self.results.append((description, success))
        
        # Then try pytest tests if pytest is available
        try:
            import pytest
            pytest_available = True
        except ImportError:
            pytest_available = False
            print("\n⚠️  pytest not available - skipping pytest-based tests")
            print("   Install with: pip install -r tests/requirements-test.txt")
        
        if pytest_available:
            # Run comprehensive test suite
            success, output = self.run_command(
                [sys.executable, "-m", "pytest", "tests/test_comprehensive.py", "-v"],
                "Comprehensive Test Suite"
            )
            self.results.append(("Comprehensive Tests", success))
            
            # Run other pytest tests
            test_files = [
                "tests/test_procedural_generators.py",
                "tests/test_schema_validator.py",
            ]
            
            for test_file in test_files:
                if Path(test_file).exists():
                    success, output = self.run_command(
                        [sys.executable, "-m", "pytest", test_file, "-v"],
                        f"Test: {Path(test_file).name}"
                    )
                    self.results.append((Path(test_file).name, success))

    def run_validation_scripts(self):
        """Run code quality validation scripts."""
        print("\n" + "=" * 70)
        print("CODE QUALITY VALIDATION")
        print("=" * 70)
        
        validation_scripts = [
            ("Tools/check_uproperty.py", "UPROPERTY Usage Check"),
            ("Tools/validate_naming.py", "Asset Naming Convention Check"),
        ]
        
        if not self.quick:
            validation_scripts.append(
                ("Tools/check_null_safety.py", "Null Safety Check")
            )
        
        for script_path, description in validation_scripts:
            if Path(script_path).exists():
                success, output = self.run_command(
                    [sys.executable, script_path],
                    description
                )
                self.results.append((description, success))

    def run_cpp_automation_tests_info(self):
        """Display information about C++ automation tests."""
        print("\n" + "=" * 70)
        print("C++ AUTOMATION TESTS (Unreal Editor Required)")
        print("=" * 70)
        
        automation_test = Path("tests/AdastreaAutomationTests.cpp")
        if automation_test.exists():
            print(f"✓ C++ automation test file found: {automation_test}")
            print("\nTo run C++ automation tests:")
            print("1. Open Adastrea.uproject in Unreal Editor")
            print("2. Go to Window > Developer Tools > Session Frontend")
            print("3. Navigate to Automation tab")
            print("4. Filter for 'Adastrea' and run the tests")
            print("\nOr use command line:")
            print("UnrealEditor-Cmd.exe Adastrea.uproject -ExecCmds=\"Automation RunTests Adastrea\" -unattended")
            
            self.results.append(("C++ Automation Tests", None))  # None = Info only
        else:
            print("✗ C++ automation test file not found")
            self.results.append(("C++ Automation Tests", False))

    def check_build_configuration(self):
        """Check build configuration files."""
        print("\n" + "=" * 70)
        print("BUILD CONFIGURATION CHECK")
        print("=" * 70)
        
        # Check .uproject file
        uproject = Path("Adastrea.uproject")
        if uproject.exists():
            print(f"✓ Project file found: {uproject}")
            try:
                import json
                with open(uproject) as f:
                    data = json.load(f)
                    print(f"  Engine Version: {data.get('EngineAssociation', 'Unknown')}")
                    print(f"  Modules: {len(data.get('Modules', []))}")
                    print(f"  Plugins: {len(data.get('Plugins', []))}")
                self.results.append(("Project Configuration", True))
            except Exception as e:
                print(f"✗ Error reading .uproject: {e}")
                self.results.append(("Project Configuration", False))
        else:
            print("✗ Project file not found")
            self.results.append(("Project Configuration", False))
        
        # Check Build.cs files
        build_files = list(Path("Source").rglob("*.Build.cs"))
        print(f"\n✓ Found {len(build_files)} Build.cs files:")
        for bf in build_files:
            print(f"  - {bf}")
        
        if len(build_files) > 0:
            self.results.append(("Build.cs Files", True))
        else:
            self.results.append(("Build.cs Files", False))

    def check_documentation(self):
        """Check that documentation exists."""
        print("\n" + "=" * 70)
        print("DOCUMENTATION CHECK")
        print("=" * 70)
        
        required_docs = [
            "README.md",
            "ARCHITECTURE.md",
            "CODE_STYLE.md",
            "CONTRIBUTING.md",
        ]
        
        all_found = True
        for doc in required_docs:
            if Path(doc).exists():
                print(f"✓ {doc}")
            else:
                print(f"✗ {doc} NOT FOUND")
                all_found = False
        
        self.results.append(("Documentation", all_found))

    def print_summary(self):
        """Print test results summary."""
        print("\n" + "=" * 70)
        print("TEST RESULTS SUMMARY")
        print("=" * 70)
        
        passed = sum(1 for _, result in self.results if result is True)
        failed = sum(1 for _, result in self.results if result is False)
        info = sum(1 for _, result in self.results if result is None)
        total = len(self.results)
        
        print(f"\nTotal Test Suites: {total}")
        print(f"  ✓ Passed: {passed}")
        print(f"  ✗ Failed: {failed}")
        print(f"  ℹ Info Only: {info}")
        
        print("\nDetailed Results:")
        for name, result in self.results:
            if result is True:
                status = "✓ PASS"
            elif result is False:
                status = "✗ FAIL"
            else:
                status = "ℹ INFO"
            print(f"  {status}: {name}")
        
        print("\n" + "=" * 70)
        
        # Return exit code
        return 0 if failed == 0 else 1

    def run_all(self, python_only: bool = False, validation_only: bool = False):
        """Run all tests."""
        print("=" * 70)
        print("ADASTREA COMPREHENSIVE TEST SUITE")
        print("=" * 70)
        print(f"Project Root: {self.project_root}")
        print(f"Python Version: {sys.version}")
        print(f"Verbose: {self.verbose}")
        print(f"Quick Mode: {self.quick}")
        
        if validation_only:
            self.run_validation_scripts()
        elif python_only:
            self.run_pytest_tests()
        else:
            # Run all test categories
            self.check_build_configuration()
            self.check_documentation()
            self.run_pytest_tests()
            self.run_validation_scripts()
            self.run_cpp_automation_tests_info()
        
        return self.print_summary()


def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description="Run all tests for Adastrea project"
    )
    parser.add_argument(
        "--verbose", "-v",
        action="store_true",
        help="Show detailed output"
    )
    parser.add_argument(
        "--quick", "-q",
        action="store_true",
        help="Run only fast checks"
    )
    parser.add_argument(
        "--python-only",
        action="store_true",
        help="Run only Python tests"
    )
    parser.add_argument(
        "--validation-only",
        action="store_true",
        help="Run only validation scripts"
    )
    
    args = parser.parse_args()
    
    runner = TestRunner(verbose=args.verbose, quick=args.quick)
    exit_code = runner.run_all(
        python_only=args.python_only,
        validation_only=args.validation_only
    )
    
    sys.exit(exit_code)


if __name__ == "__main__":
    main()
