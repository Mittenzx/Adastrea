#!/usr/bin/env python3
"""
TestSettingsWidget Validation Test

This script validates the TestSettingsWidget implementation by checking:
- Header file structure and syntax
- Implementation file completeness
- Documentation quality
- Adherence to Adastrea coding standards

Usage:
    python TestSettingsWidget_ValidationTest.py

Requirements:
    - Python 3.6+
    - No external dependencies
"""

import os
import re
import sys
from pathlib import Path
from typing import List, Tuple, Dict

class TestSettingsWidgetValidator:
    """Validator for TestSettingsWidget implementation"""
    
    def __init__(self, repo_root: str):
        self.repo_root = Path(repo_root)
        self.header_path = self.repo_root / "Source/Adastrea/UI/TestSettingsWidget.h"
        self.impl_path = self.repo_root / "Source/Adastrea/UI/TestSettingsWidget.cpp"
        self.usage_doc_path = self.repo_root / "Source/Adastrea/UI/TestSettingsWidget_Usage.md"
        self.blueprint_doc_path = self.repo_root / "Assets/TestSettingsWidget_Blueprint_Example.md"
        self.content_readme_path = self.repo_root / "Content/UI/Menus/TestSettings_README.md"
        
        self.errors: List[str] = []
        self.warnings: List[str] = []
        self.info: List[str] = []
    
    def run_all_tests(self) -> bool:
        """Run all validation tests"""
        print("=" * 70)
        print("TestSettingsWidget Validation Test")
        print("=" * 70)
        print()
        
        # File existence
        print("1. Checking file existence...")
        if not self.check_files_exist():
            return False
        print("   ✅ All files exist")
        print()
        
        # Header file validation
        print("2. Validating header file...")
        self.validate_header_file()
        print("   ✅ Header file validated")
        print()
        
        # Implementation file validation
        print("3. Validating implementation file...")
        self.validate_implementation_file()
        print("   ✅ Implementation file validated")
        print()
        
        # Documentation validation
        print("4. Validating documentation...")
        self.validate_documentation()
        print("   ✅ Documentation validated")
        print()
        
        # Coding standards validation
        print("5. Checking coding standards...")
        self.validate_coding_standards()
        print("   ✅ Coding standards checked")
        print()
        
        # Integration validation
        print("6. Checking integration points...")
        self.validate_integration()
        print("   ✅ Integration points checked")
        print()
        
        # Print summary
        self.print_summary()
        
        return len(self.errors) == 0
    
    def check_files_exist(self) -> bool:
        """Check that all required files exist"""
        files = [
            self.header_path,
            self.impl_path,
            self.usage_doc_path,
            self.blueprint_doc_path,
            self.content_readme_path
        ]
        
        missing = []
        for file_path in files:
            if not file_path.exists():
                missing.append(str(file_path))
        
        if missing:
            for path in missing:
                self.errors.append(f"Missing file: {path}")
            return False
        
        return True
    
    def validate_header_file(self):
        """Validate header file structure"""
        content = self.header_path.read_text()
        
        # Check for required includes
        required_includes = [
            '#pragma once',
            '#include "CoreMinimal.h"',
            '#include "Blueprint/UserWidget.h"',
            '.generated.h"'
        ]
        
        for include in required_includes:
            if include not in content:
                self.errors.append(f"Header missing required include: {include}")
        
        # Check class declaration
        if 'class ADASTREA_API UTestSettingsWidget : public UUserWidget' not in content:
            self.errors.append("Header missing correct class declaration")
        
        # Check UCLASS macro
        if 'UCLASS(BlueprintType, Blueprintable)' not in content:
            self.errors.append("Header missing UCLASS macro with correct specifiers")
        
        # Check for BlueprintNativeEvent functions
        native_event_functions = [
            'InitializeTestSettings',
            'OnContinueClicked',
            'OnResetClicked',
            'OnShipTypeChanged',
            'OnDifficultyChanged',
            'OnDebugModeChanged',
            'TransitionToMainMenu',
            'ApplySettings',
            'LoadDefaultSettings'
        ]
        
        for func in native_event_functions:
            pattern = f'UFUNCTION\\(BlueprintNativeEvent.*\\)\\s*void {func}'
            if not re.search(pattern, content):
                self.warnings.append(f"Function {func} may not be properly declared as BlueprintNativeEvent")
        
        # Check for getter functions
        getters = [
            'GetSelectedShipType',
            'GetSelectedDifficulty',
            'GetSelectedDebugMode',
            'GetAutoSpawnEnabled'
        ]
        
        for getter in getters:
            if getter not in content:
                self.warnings.append(f"Getter function {getter} not found in header")
        
        # Check for UPROPERTY declarations
        if content.count('UPROPERTY(') < 8:
            self.warnings.append("Expected at least 8 UPROPERTY declarations")
        
        # Check documentation
        if '/**' not in content:
            self.errors.append("Header missing Doxygen-style documentation comments")
    
    def validate_implementation_file(self):
        """Validate implementation file"""
        content = self.impl_path.read_text()
        
        # Check includes
        required_includes = [
            '#include "UI/TestSettingsWidget.h"',
            '#include "AdastreaLog.h"'
        ]
        
        for include in required_includes:
            if include not in content:
                self.errors.append(f"Implementation missing required include: {include}")
        
        # Check constructor
        if 'UTestSettingsWidget::UTestSettingsWidget(const FObjectInitializer& ObjectInitializer)' not in content:
            self.errors.append("Constructor not properly defined")
        
        # Check initializer list
        if ': Super(ObjectInitializer)' not in content:
            self.errors.append("Constructor missing Super call in initializer list")
        
        # Check _Implementation functions
        implementation_functions = [
            'InitializeTestSettings_Implementation',
            'OnContinueClicked_Implementation',
            'OnResetClicked_Implementation',
            'OnShipTypeChanged_Implementation',
            'OnDifficultyChanged_Implementation',
            'OnDebugModeChanged_Implementation',
            'TransitionToMainMenu_Implementation',
            'ApplySettings_Implementation',
            'LoadDefaultSettings_Implementation'
        ]
        
        for func in implementation_functions:
            pattern = f'void UTestSettingsWidget::{func}\\('
            if not re.search(pattern, content):
                self.errors.append(f"Missing implementation for {func}")
        
        # Check NativeConstruct override
        if 'void UTestSettingsWidget::NativeConstruct()' not in content:
            self.warnings.append("NativeConstruct not overridden")
        
        # Check NativeDestruct override
        if 'void UTestSettingsWidget::NativeDestruct()' not in content:
            self.warnings.append("NativeDestruct not overridden")
        
        # Check logging
        if content.count('UE_LOG(LogAdastrea') < 10:
            self.warnings.append("Expected more logging statements (found < 10)")
        
        # Check default values in constructor
        default_values = [
            'TEXT("Default Fighter")',
            'TEXT("Normal")',
            'TEXT("None")',
            'bAutoSpawnShip(true)'
        ]
        
        for value in default_values:
            if value not in content:
                self.warnings.append(f"Default value not found in constructor: {value}")
    
    def validate_documentation(self):
        """Validate documentation files"""
        # Usage documentation
        usage_content = self.usage_doc_path.read_text()
        
        required_sections = [
            '## Overview',
            '## Quick Start',
            '## Core Features',
            '## Properties',
            '## Functions',
            '## Troubleshooting'
        ]
        
        for section in required_sections:
            if section not in usage_content:
                self.warnings.append(f"Usage doc missing section: {section}")
        
        if len(usage_content) < 5000:
            self.warnings.append("Usage documentation seems short (< 5000 chars)")
        
        # Blueprint example documentation
        blueprint_content = self.blueprint_doc_path.read_text()
        
        blueprint_sections = [
            '## Widget Blueprint:',
            '## Event Graph',
            '## Designer Layout'
        ]
        
        for section in blueprint_sections:
            if section not in blueprint_content:
                self.warnings.append(f"Blueprint doc missing section: {section}")
        
        # Content README
        readme_content = self.content_readme_path.read_text()
        
        if '## Overview' not in readme_content:
            self.warnings.append("Content README missing overview section")
    
    def validate_coding_standards(self):
        """Check adherence to Adastrea coding standards"""
        header_content = self.header_path.read_text()
        impl_content = self.impl_path.read_text()
        
        # Check naming conventions
        
        # Boolean variables should start with 'b'
        bool_pattern = r'bool\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*[;=]'
        for match in re.finditer(bool_pattern, header_content):
            var_name = match.group(1)
            if not var_name.startswith('b') and var_name != 'bool':
                self.warnings.append(f"Boolean variable '{var_name}' should start with 'b'")
        
        # Function names should be PascalCase
        func_pattern = r'void\s+([a-z][a-zA-Z0-9_]*)\s*\('
        for match in re.finditer(func_pattern, impl_content):
            func_name = match.group(1)
            if func_name[0].islower() and func_name not in ['operator']:
                self.info.append(f"Function '{func_name}' starts with lowercase (expected PascalCase)")
        
        # Check for nullptr initialization
        if 'nullptr' not in impl_content and '*' in impl_content:
            self.info.append("Consider using nullptr for pointer initialization")
        
        # Check Category usage
        if header_content.count('Category=') < 8:
            self.warnings.append("Expected more Category specifications in UPROPERTY/UFUNCTION")
    
    def validate_integration(self):
        """Check integration with other systems"""
        impl_content = self.impl_path.read_text()
        
        # Check for proper input mode setup
        if 'FInputModeUIOnly' not in impl_content:
            self.warnings.append("Missing FInputModeUIOnly for proper UI input handling")
        
        # Check for GetOwningPlayer usage
        if 'GetOwningPlayer()' not in impl_content:
            self.warnings.append("Consider using GetOwningPlayer() for player controller access")
        
        # Check for RemoveFromParent usage
        if 'RemoveFromParent()' not in impl_content:
            self.warnings.append("Missing RemoveFromParent() for widget cleanup")
    
    def print_summary(self):
        """Print validation summary"""
        print("=" * 70)
        print("VALIDATION SUMMARY")
        print("=" * 70)
        print()
        
        if self.errors:
            print(f"❌ ERRORS ({len(self.errors)}):")
            for error in self.errors:
                print(f"   • {error}")
            print()
        
        if self.warnings:
            print(f"⚠️  WARNINGS ({len(self.warnings)}):")
            for warning in self.warnings:
                print(f"   • {warning}")
            print()
        
        if self.info:
            print(f"ℹ️  INFO ({len(self.info)}):")
            for info_msg in self.info:
                print(f"   • {info_msg}")
            print()
        
        if not self.errors and not self.warnings:
            print("✅ ALL CHECKS PASSED!")
            print()
            print("The TestSettingsWidget implementation is EXCELLENT and ready for production.")
            print()
        elif not self.errors:
            print("✅ VALIDATION PASSED (with warnings)")
            print()
            print("The implementation is solid. Review warnings for potential improvements.")
            print()
        else:
            print("❌ VALIDATION FAILED")
            print()
            print("Please fix the errors listed above before proceeding.")
            print()
        
        print("=" * 70)

def main():
    """Main entry point"""
    # Detect repository root
    script_dir = Path(__file__).parent
    repo_root = script_dir.parent
    
    # Check if we're in the right place
    if not (repo_root / "Adastrea.uproject").exists():
        print("Error: Could not find Adastrea.uproject")
        print(f"Looking in: {repo_root}")
        sys.exit(1)
    
    # Run validation
    validator = TestSettingsWidgetValidator(str(repo_root))
    success = validator.run_all_tests()
    
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
