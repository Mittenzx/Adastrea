#!/usr/bin/env python3
"""
Adastrea - Pre-Build Setup Check Script (Python)

Cross-platform validation script for Unreal Engine project setup.
This script verifies development environment configuration before building.

Features:
- Checks Unreal Engine version from .uproject file
- Validates existence of key folders and files
- Optional static analysis with cppcheck/clang-tidy
- Clear colored output for designer/programmer use
- Cross-platform support (Windows, Linux, Mac)

Usage:
    python SetupCheck.py [options]

Options:
    --analyze     Run static analysis if tools available
    --verbose     Show detailed output
    --help        Display this help message
"""

import os
import sys
import json
import subprocess
import platform
from pathlib import Path
from typing import List, Tuple


class Colors:
    """ANSI color codes for terminal output"""
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    PURPLE = '\033[0;35m'
    CYAN = '\033[0;36m'
    NC = '\033[0m'  # No Color
    
    @staticmethod
    def disable():
        """Disable colors (for Windows without ANSI support)"""
        Colors.RED = ''
        Colors.GREEN = ''
        Colors.YELLOW = ''
        Colors.BLUE = ''
        Colors.PURPLE = ''
        Colors.CYAN = ''
        Colors.NC = ''


class SetupValidator:
    """Validates Adastrea project setup and development environment"""
    
    def __init__(self, verbose=False, run_analysis=False):
        self.verbose = verbose
        self.run_analysis = run_analysis
        self.errors = 0
        self.warnings = 0
        self.project_root = Path.cwd()
        
        # Enable colors on Windows 10+ or Unix-like systems
        if platform.system() == 'Windows':
            try:
                # Try to enable ANSI colors on Windows
                import ctypes
                kernel32 = ctypes.windll.kernel32
                kernel32.SetConsoleMode(kernel32.GetStdHandle(-11), 7)
            except:
                Colors.disable()
    
    def print_header(self, text: str):
        """Print a section header"""
        print(f"\n{Colors.BLUE}{'=' * 60}{Colors.NC}")
        print(f"{Colors.BLUE}{text}{Colors.NC}")
        print(f"{Colors.BLUE}{'=' * 60}{Colors.NC}")
    
    def print_success(self, text: str):
        """Print a success message"""
        print(f"{Colors.GREEN}✓ {text}{Colors.NC}")
    
    def print_error(self, text: str):
        """Print an error message"""
        print(f"{Colors.RED}✗ {text}{Colors.NC}")
        self.errors += 1
    
    def print_warning(self, text: str):
        """Print a warning message"""
        print(f"{Colors.YELLOW}⚠ {text}{Colors.NC}")
        self.warnings += 1
    
    def print_info(self, text: str):
        """Print an info message"""
        print(f"{Colors.CYAN}ℹ {text}{Colors.NC}")
    
    def check_project_root(self) -> bool:
        """Verify we're running from the project root"""
        uproject_file = self.project_root / "Adastrea.uproject"
        if not uproject_file.exists():
            self.print_error("Error: This script must be run from the Adastrea project root directory")
            self.print_info(f"Current directory: {self.project_root}")
            return False
        return True
    
    def check_unreal_version(self):
        """Check Unreal Engine version from .uproject file"""
        self.print_header("1. Checking Unreal Engine Version")
        
        uproject_file = self.project_root / "Adastrea.uproject"
        required_version = "5.6"
        
        try:
            with open(uproject_file, 'r', encoding='utf-8') as f:
                project_data = json.load(f)
            
            engine_version = project_data.get('EngineAssociation', 'Unknown')
            
            if engine_version == required_version:
                self.print_success(f"Required Unreal Engine version: {required_version}")
            else:
                self.print_warning(f"Expected Unreal Engine {required_version}, found: {engine_version}")
        
        except Exception as e:
            self.print_error(f"Failed to read .uproject file: {e}")
    
    def check_compilers(self):
        """Check for available C++ compilers"""
        self.print_header("2. Checking C++ Compiler")
        
        compilers_found = []
        
        # Check for various compilers
        compilers_to_check = []
        
        if platform.system() == 'Windows':
            compilers_to_check = [
                ('cl.exe', 'MSVC'),
                ('clang++', 'Clang'),
            ]
        else:
            compilers_to_check = [
                ('g++', 'GCC'),
                ('clang++', 'Clang'),
            ]
        
        for compiler_cmd, compiler_name in compilers_to_check:
            if self._check_command(compiler_cmd):
                try:
                    result = subprocess.run(
                        [compiler_cmd, '--version'] if compiler_cmd != 'cl.exe' else [compiler_cmd],
                        capture_output=True,
                        text=True,
                        timeout=5
                    )
                    version_line = result.stdout.split('\n')[0] if result.stdout else 'Version unknown'
                    self.print_success(f"{compiler_name} compiler found: {version_line[:80]}")
                    compilers_found.append(compiler_name)
                except Exception as e:
                    if self.verbose:
                        self.print_info(f"Could not get {compiler_name} version: {e}")
        
        if not compilers_found:
            self.print_error("No C++ compiler found!")
            if platform.system() == 'Windows':
                self.print_info("  Install Visual Studio 2022 with C++ workload")
            elif platform.system() == 'Darwin':  # Mac
                self.print_info("  Run: xcode-select --install")
            else:  # Linux
                self.print_info("  Ubuntu/Debian: sudo apt-get install build-essential")
    
    def check_folders(self):
        """Check for required project folders"""
        self.print_header("3. Checking Required Project Folders")
        
        required_folders = ["Source", "Config", "Content", "Assets"]
        
        for folder in required_folders:
            folder_path = self.project_root / folder
            if folder_path.exists() and folder_path.is_dir():
                self.print_success(f"Found folder: {folder}")
            else:
                self.print_error(f"Missing required folder: {folder}")
        
        # Check Source subfolders
        self.print_info("Checking Source structure...")
        source_modules = [
            "Source/Adastrea",
            "Source/PlayerMods",
            "Source/StationEditor"
        ]
        
        for module in source_modules:
            module_path = self.project_root / module
            if module_path.exists() and module_path.is_dir():
                self.print_success(f"Found module: {module}")
            else:
                self.print_error(f"Missing module: {module}")
    
    def check_files(self):
        """Check for required project files"""
        self.print_header("4. Checking Required Files")
        
        required_files = [
            "Adastrea.uproject",
            "Source/Adastrea/Adastrea.h",
            "Source/Adastrea/Adastrea.cpp",
            "Config/DefaultEngine.ini",
            "Config/DefaultGame.ini",
        ]
        
        for file_path in required_files:
            full_path = self.project_root / file_path
            if full_path.exists() and full_path.is_file():
                self.print_success(f"Found file: {file_path}")
            else:
                self.print_error(f"Missing required file: {file_path}")
    
    def check_documentation(self):
        """Check for important documentation files"""
        self.print_header("5. Checking Documentation")
        
        docs_to_check = [
            "README.md",
            "CONTRIBUTING.md",
            "ARCHITECTURE.md",
            "Assets/PlaytestingChecklist.md",
            "Assets/DesignerOnboarding.md",
        ]
        
        for doc in docs_to_check:
            doc_path = self.project_root / doc
            if doc_path.exists() and doc_path.is_file():
                self.print_success(f"Found documentation: {doc}")
            else:
                self.print_warning(f"Missing documentation: {doc}")
    
    def run_static_analysis(self):
        """Run static analysis if tools are available"""
        self.print_header("6. Running Static Analysis")
        
        analysis_run = False
        
        # Check for cppcheck
        if self._check_command('cppcheck'):
            self.print_info("Running cppcheck on Source directory...")
            self.print_warning("Note: This may take several minutes...")
            
            try:
                result = subprocess.run(
                    [
                        'cppcheck',
                        '--enable=warning,style,performance,portability',
                        '--suppress=unusedFunction',
                        '--suppress=unmatchedSuppression',
                        '--quiet',
                        '--template={file}:{line}: {severity}: {message}',
                        'Source/'
                    ],
                    capture_output=True,
                    text=True,
                    timeout=300  # 5 minute timeout
                )
                
                if result.stderr:
                    output_file = self.project_root / "cppcheck_results.txt"
                    with open(output_file, 'w') as f:
                        f.write(result.stderr)
                    self.print_warning(f"Static analysis found issues. See {output_file}")
                else:
                    self.print_success("No issues found by cppcheck")
                
                analysis_run = True
            
            except subprocess.TimeoutExpired:
                self.print_warning("cppcheck timed out after 5 minutes")
            except Exception as e:
                self.print_warning(f"cppcheck failed: {e}")
        
        # Check for clang-tidy
        if self._check_command('clang-tidy') and not analysis_run:
            self.print_info("clang-tidy is available but not configured for this project")
            self.print_info("Consider setting up .clang-tidy configuration file")
        
        if not analysis_run and not self._check_command('cppcheck'):
            self.print_warning("No static analysis tools found")
            if platform.system() == 'Windows':
                self.print_info("  Install cppcheck from: http://cppcheck.sourceforge.net/")
            elif platform.system() == 'Darwin':
                self.print_info("  Mac: brew install cppcheck")
            else:
                self.print_info("  Ubuntu/Debian: sudo apt-get install cppcheck")
    
    def print_summary(self):
        """Print validation summary"""
        self.print_header("Setup Check Summary")
        
        if self.errors == 0 and self.warnings == 0:
            self.print_success("All checks passed! Your environment is ready for building.")
            print()
            self.print_info("Next steps:")
            print("  1. Generate project files: Right-click Adastrea.uproject")
            print("  2. Build the project in Visual Studio/Xcode/Rider")
            print("  3. Launch Unreal Engine and verify all systems")
            print()
        elif self.errors == 0:
            self.print_warning(f"Setup check completed with {self.warnings} warning(s)")
            self.print_info("Your environment should work, but please review warnings above")
            print()
        else:
            self.print_error(f"Setup check failed with {self.errors} error(s) and {self.warnings} warning(s)")
            self.print_info("Please fix the errors above before building the project")
            print()
        
        return 0 if self.errors == 0 else 1
    
    def _check_command(self, command: str) -> bool:
        """Check if a command is available in PATH"""
        try:
            if platform.system() == 'Windows':
                subprocess.run(['where', command], capture_output=True, check=True)
            else:
                subprocess.run(['which', command], capture_output=True, check=True)
            return True
        except:
            return False
    
    def run(self) -> int:
        """Run all validation checks"""
        print(f"{Colors.PURPLE}{'=' * 60}{Colors.NC}")
        print(f"{Colors.PURPLE}Adastrea Pre-Build Setup Check (Python){Colors.NC}")
        print(f"{Colors.PURPLE}Platform: {platform.system()} {platform.release()}{Colors.NC}")
        print(f"{Colors.PURPLE}Python: {sys.version.split()[0]}{Colors.NC}")
        print(f"{Colors.PURPLE}{'=' * 60}{Colors.NC}")
        
        # Check if running from correct directory
        if not self.check_project_root():
            return 1
        
        # Run all checks
        self.check_unreal_version()
        self.check_compilers()
        self.check_folders()
        self.check_files()
        self.check_documentation()
        
        if self.run_analysis:
            self.run_static_analysis()
        
        # Print summary and return exit code
        return self.print_summary()


def main():
    """Main entry point"""
    import argparse
    
    parser = argparse.ArgumentParser(
        description='Adastrea Pre-Build Setup Validation',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python SetupCheck.py                    # Basic validation
  python SetupCheck.py --verbose          # Detailed output
  python SetupCheck.py --analyze          # Include static analysis
  python SetupCheck.py --analyze --verbose # All checks with details
        """
    )
    
    parser.add_argument('--analyze', action='store_true',
                        help='Run static analysis with cppcheck/clang-tidy')
    parser.add_argument('--verbose', action='store_true',
                        help='Show detailed output')
    
    args = parser.parse_args()
    
    validator = SetupValidator(verbose=args.verbose, run_analysis=args.analyze)
    sys.exit(validator.run())


if __name__ == '__main__':
    main()
