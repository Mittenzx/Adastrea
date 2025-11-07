#!/usr/bin/env python3
"""
Adastrea - Schema Validator

This script validates game data against JSON schema definitions.
It can be run from command line or imported as a module.

Features:
- Validate YAML/JSON data files against schema definitions
- Validate CSV data against schemas
- Generate detailed validation reports
- Support for custom validation rules
- Batch validation of multiple files

Usage:
    # Command line
    python SchemaValidator.py --schema Schemas/spaceship_schema.json --data Assets/SpaceshipTemplates/Scout_Pathfinder.yaml
    
    # As module
    from SchemaValidator import SchemaValidator
    validator = SchemaValidator()
    validator.validate_file("spaceship", "path/to/data.yaml")
"""

import json
import yaml
import sys
import argparse
from pathlib import Path
from typing import Dict, Any, List, Optional, Tuple
from datetime import datetime
import re

try:
    from jsonschema import validate, ValidationError, Draft7Validator
    JSONSCHEMA_AVAILABLE = True
except ImportError:
    JSONSCHEMA_AVAILABLE = False
    print("WARNING: jsonschema module not available. Install with: pip install jsonschema")


class SchemaValidator:
    """
    Validates game data against JSON schema definitions
    """
    
    def __init__(self, schema_dir: Optional[Path] = None):
        """
        Initialize the schema validator
        
        Args:
            schema_dir: Directory containing schema files (defaults to project/Schemas)
        """
        if not JSONSCHEMA_AVAILABLE:
            raise RuntimeError("jsonschema module is required. Install with: pip install jsonschema")
        
        # Determine project root and schema directory
        self.project_dir = Path.cwd()
        if schema_dir:
            self.schema_dir = Path(schema_dir)
        else:
            self.schema_dir = self.project_dir / "Schemas"
        
        # Cache for loaded schemas
        self.schemas = {}
        
        # Validation results
        self.errors = []
        self.warnings = []
        self.info_messages = []
    
    def load_schema(self, schema_name: str) -> Optional[Dict[str, Any]]:
        """
        Load a schema definition from file
        
        Args:
            schema_name: Name of schema (e.g., "spaceship", "trade_item")
            
        Returns:
            Schema dictionary or None if not found
        """
        # Check cache first
        if schema_name in self.schemas:
            return self.schemas[schema_name]
        
        # Try to load from file
        schema_file = self.schema_dir / f"{schema_name}_schema.json"
        
        if not schema_file.exists():
            self.log_error(f"Schema file not found: {schema_file}")
            return None
        
        try:
            with open(schema_file, 'r', encoding='utf-8') as f:
                schema = json.load(f)
            
            self.schemas[schema_name] = schema
            self.log_info(f"Loaded schema: {schema_name}")
            return schema
        except Exception as e:
            self.log_error(f"Failed to load schema {schema_name}: {e}")
            return None
    
    def load_data_file(self, file_path: Path) -> Optional[Dict[str, Any]]:
        """
        Load data from YAML or JSON file
        
        Args:
            file_path: Path to data file
            
        Returns:
            Data dictionary or None if failed
        """
        if not file_path.exists():
            self.log_error(f"Data file not found: {file_path}")
            return None
        
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                if file_path.suffix.lower() in ['.yaml', '.yml']:
                    data = yaml.safe_load(f)
                elif file_path.suffix.lower() == '.json':
                    data = json.load(f)
                else:
                    self.log_error(f"Unsupported file format: {file_path.suffix}")
                    return None
            
            self.log_info(f"Loaded data file: {file_path.name}")
            return data
        except Exception as e:
            self.log_error(f"Failed to load data file {file_path}: {e}")
            return None
    
    def validate_data(self, schema_name: str, data: Dict[str, Any], 
                     source_name: str = "data") -> Tuple[bool, List[str]]:
        """
        Validate data against a schema
        
        Args:
            schema_name: Name of schema to use
            data: Data to validate
            source_name: Name of data source (for error messages)
            
        Returns:
            Tuple of (is_valid, list of error messages)
        """
        schema = self.load_schema(schema_name)
        if not schema:
            return False, [f"Schema '{schema_name}' not found"]
        
        validator = Draft7Validator(schema)
        errors = []
        
        # Validate against schema
        for error in validator.iter_errors(data):
            error_path = " -> ".join(str(p) for p in error.path) if error.path else "root"
            error_msg = f"{source_name}: {error_path}: {error.message}"
            errors.append(error_msg)
            self.log_error(error_msg)
        
        # Custom validation rules
        custom_errors = self.apply_custom_validation(schema_name, data, source_name)
        errors.extend(custom_errors)
        
        is_valid = len(errors) == 0
        
        if is_valid:
            self.log_info(f"✓ Validation passed for {source_name}")
        else:
            self.log_error(f"✗ Validation failed for {source_name} with {len(errors)} error(s)")
        
        return is_valid, errors
    
    def validate_file(self, schema_name: str, file_path: Path) -> Tuple[bool, List[str]]:
        """
        Validate a data file against a schema
        
        Args:
            schema_name: Name of schema to use
            file_path: Path to data file
            
        Returns:
            Tuple of (is_valid, list of error messages)
        """
        data = self.load_data_file(file_path)
        if not data:
            return False, [f"Failed to load data file: {file_path}"]
        
        return self.validate_data(schema_name, data, str(file_path))
    
    def apply_custom_validation(self, schema_name: str, data: Dict[str, Any], 
                               source_name: str) -> List[str]:
        """
        Apply custom validation rules beyond JSON Schema
        
        Args:
            schema_name: Name of schema
            data: Data to validate
            source_name: Name of data source
            
        Returns:
            List of error messages
        """
        errors = []
        
        # Custom validation for spaceship data
        if schema_name == "spaceship":
            errors.extend(self._validate_spaceship_custom(data, source_name))
        
        # Custom validation for trade item data
        elif schema_name == "trade_item":
            errors.extend(self._validate_trade_item_custom(data, source_name))
        
        return errors
    
    def _validate_spaceship_custom(self, data: Dict[str, Any], 
                                   source_name: str) -> List[str]:
        """Custom validation rules for spaceship data"""
        errors = []
        
        # Validate crew requirements
        if 'CoreStats' in data:
            core = data['CoreStats']
            crew_min = core.get('CrewRequired', 0)
            crew_max = core.get('MaxCrew', 0)
            
            if crew_min > crew_max:
                errors.append(f"{source_name}: CrewRequired ({crew_min}) cannot be greater than MaxCrew ({crew_max})")
        
        # Validate color arrays if present
        if 'Lore' in data:
            lore = data['Lore']
            for color_key in ['PrimaryColor', 'SecondaryColor']:
                if color_key in lore:
                    color = lore[color_key]
                    if isinstance(color, list):
                        if len(color) != 4:
                            errors.append(f"{source_name}: {color_key} must have exactly 4 values (RGBA)")
                        elif not all(0 <= v <= 1 for v in color):
                            errors.append(f"{source_name}: {color_key} values must be between 0 and 1")
        
        return errors
    
    def _validate_trade_item_custom(self, data: Dict[str, Any], 
                                    source_name: str) -> List[str]:
        """Custom validation rules for trade item data"""
        errors = []
        
        # Validate price volatility consistency
        if 'PriceVolatility' in data:
            volatility = data['PriceVolatility']
            min_dev = volatility.get('MinPriceDeviation', 0.5)
            max_dev = volatility.get('MaxPriceDeviation', 2.0)
            
            if min_dev >= max_dev:
                errors.append(f"{source_name}: MinPriceDeviation ({min_dev}) must be less than MaxPriceDeviation ({max_dev})")
        
        # Validate contraband settings
        legality = data.get('LegalityStatus', 'Legal')
        fine_multiplier = data.get('ContrabandFineMultiplier', 0.0)
        
        if legality in ['Contraband', 'Illegal'] and fine_multiplier == 0.0:
            warning_msg = f"{source_name}: {legality} item should have a ContrabandFineMultiplier > 0"
            self.log_warning(warning_msg)
        
        return errors
    
    def batch_validate_directory(self, schema_name: str, 
                                 directory: Path,
                                 pattern: str = "*.yaml") -> Dict[str, Tuple[bool, List[str]]]:
        """
        Validate all files in a directory
        
        Args:
            schema_name: Name of schema to use
            directory: Directory containing data files
            pattern: File pattern to match (default: *.yaml)
            
        Returns:
            Dictionary mapping file paths to (is_valid, errors) tuples
        """
        results = {}
        
        if not directory.exists():
            self.log_error(f"Directory not found: {directory}")
            return results
        
        files = list(directory.glob(pattern))
        
        if not files:
            self.log_warning(f"No files matching '{pattern}' found in {directory}")
            return results
        
        self.log_info(f"Validating {len(files)} file(s) in {directory}")
        
        for file_path in files:
            is_valid, errors = self.validate_file(schema_name, file_path)
            results[str(file_path)] = (is_valid, errors)
        
        return results
    
    def generate_report(self, results: Dict[str, Tuple[bool, List[str]]], 
                       output_file: Optional[Path] = None) -> str:
        """
        Generate a validation report
        
        Args:
            results: Dictionary of validation results
            output_file: Optional file to write report to
            
        Returns:
            Report as string
        """
        total = len(results)
        passed = sum(1 for is_valid, _ in results.values() if is_valid)
        failed = total - passed
        
        report_lines = [
            "=" * 80,
            "VALIDATION REPORT",
            "=" * 80,
            f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}",
            f"Total Files: {total}",
            f"Passed: {passed}",
            f"Failed: {failed}",
            "=" * 80,
            ""
        ]
        
        # List failed validations
        if failed > 0:
            report_lines.append("FAILED VALIDATIONS:")
            report_lines.append("-" * 80)
            
            for file_path, (is_valid, errors) in results.items():
                if not is_valid:
                    report_lines.append(f"\n✗ {file_path}")
                    for error in errors:
                        report_lines.append(f"  - {error}")
            
            report_lines.append("")
        
        # List passed validations
        if passed > 0:
            report_lines.append("PASSED VALIDATIONS:")
            report_lines.append("-" * 80)
            
            for file_path, (is_valid, _) in results.items():
                if is_valid:
                    report_lines.append(f"✓ {file_path}")
            
            report_lines.append("")
        
        report_lines.append("=" * 80)
        
        report = "\n".join(report_lines)
        
        # Write to file if specified
        if output_file:
            try:
                with open(output_file, 'w', encoding='utf-8') as f:
                    f.write(report)
                self.log_info(f"Report written to: {output_file}")
            except Exception as e:
                self.log_error(f"Failed to write report to {output_file}: {e}")
        
        return report
    
    def log_error(self, message: str):
        """Log an error message"""
        self.errors.append(message)
        print(f"ERROR: {message}", file=sys.stderr)
    
    def log_warning(self, message: str):
        """Log a warning message"""
        self.warnings.append(message)
        print(f"WARNING: {message}")
    
    def log_info(self, message: str):
        """Log an info message"""
        self.info_messages.append(message)
        print(f"INFO: {message}")
    
    def clear_logs(self):
        """Clear all logged messages"""
        self.errors.clear()
        self.warnings.clear()
        self.info_messages.clear()


def main():
    """Command line interface"""
    parser = argparse.ArgumentParser(
        description="Validate game data files against schema definitions"
    )
    parser.add_argument(
        '--schema',
        required=True,
        help='Schema name (e.g., spaceship, trade_item) or path to schema file'
    )
    parser.add_argument(
        '--data',
        help='Path to data file to validate'
    )
    parser.add_argument(
        '--directory',
        help='Directory containing data files to batch validate'
    )
    parser.add_argument(
        '--pattern',
        default='*.yaml',
        help='File pattern for batch validation (default: *.yaml)'
    )
    parser.add_argument(
        '--schema-dir',
        help='Directory containing schema files (default: project/Schemas)'
    )
    parser.add_argument(
        '--report',
        help='Output file for validation report'
    )
    parser.add_argument(
        '--verbose',
        action='store_true',
        help='Enable verbose output'
    )
    
    args = parser.parse_args()
    
    # Initialize validator
    schema_dir = Path(args.schema_dir) if args.schema_dir else None
    validator = SchemaValidator(schema_dir)
    
    # Determine schema name
    schema_path = Path(args.schema)
    if schema_path.exists():
        # Schema is a file path
        schema_name = schema_path.stem.replace('_schema', '')
    else:
        # Schema is a name
        schema_name = args.schema
    
    # Validate single file or batch
    if args.data:
        # Single file validation
        data_path = Path(args.data)
        is_valid, errors = validator.validate_file(schema_name, data_path)
        
        if is_valid:
            print(f"\n✓ Validation PASSED for {data_path}")
            sys.exit(0)
        else:
            print(f"\n✗ Validation FAILED for {data_path}")
            print(f"\nErrors found: {len(errors)}")
            for error in errors:
                print(f"  - {error}")
            sys.exit(1)
    
    elif args.directory:
        # Batch validation
        directory = Path(args.directory)
        results = validator.batch_validate_directory(schema_name, directory, args.pattern)
        
        # Generate report
        report = validator.generate_report(
            results,
            Path(args.report) if args.report else None
        )
        
        print("\n" + report)
        
        # Exit with error code if any validation failed
        failed = sum(1 for is_valid, _ in results.values() if not is_valid)
        sys.exit(1 if failed > 0 else 0)
    
    else:
        parser.error("Either --data or --directory must be specified")


if __name__ == "__main__":
    main()
