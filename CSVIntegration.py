#!/usr/bin/env python3
"""
Adastrea - CSV Import/Export

This script provides CSV file import and export for game data.
Works as a fallback or alternative to Google Sheets integration.

Features:
- Export Data Assets to CSV files
- Import/Update Data Assets from CSV files
- Schema-based validation
- Batch operations support

Usage:
    # Export to CSV
    python CSVIntegration.py export --type spaceship --output ships.csv
    
    # Import from CSV
    python CSVIntegration.py import --type spaceship --input ships.csv
"""

import csv
import yaml
import sys
import argparse
from pathlib import Path
from typing import Dict, Any, List, Optional
from datetime import datetime

try:
    from SchemaValidator import SchemaValidator
    VALIDATOR_AVAILABLE = True
except ImportError:
    VALIDATOR_AVAILABLE = False
    print("WARNING: SchemaValidator not available. Validation will be skipped.")

# Import shared asset configurations
try:
    from AssetConfig import get_asset_configs
    ASSET_CONFIG_AVAILABLE = True
except ImportError:
    ASSET_CONFIG_AVAILABLE = False
    print("WARNING: AssetConfig not available. Using basic configuration.")


class CSVIntegration:
    """
    CSV file integration for game data management
    """
    
    def __init__(self):
        """Initialize CSV integration"""
        self.project_dir = Path.cwd()
        
        # Schema validator
        if VALIDATOR_AVAILABLE:
            self.validator = SchemaValidator()
        else:
            self.validator = None
        
        # Get asset configurations
        if ASSET_CONFIG_AVAILABLE:
            self.asset_configs = get_asset_configs()
        else:
            # Fallback basic config
            self.asset_configs = self._get_basic_asset_configs()
    
    def _get_basic_asset_configs(self) -> Dict[str, Dict[str, Any]]:
        """Basic asset configurations if Google Sheets integration unavailable"""
        return {
            'spaceship': {
                'schema_name': 'spaceship',
                'template_dir': 'Assets/SpaceshipTemplates',
                'id_field': 'BasicInfo.ShipID',
                'columns': []  # Will use all fields from YAML
            },
            'trade_item': {
                'schema_name': 'trade_item',
                'template_dir': 'Assets/TradingTemplates',
                'id_field': 'ItemID',
                'columns': []  # Will use all fields from YAML
            }
        }
    
    def export_to_csv(self, asset_type: str, output_file: Path,
                     delimiter: str = ',') -> bool:
        """
        Export existing data assets to CSV file
        
        Args:
            asset_type: Type of asset (spaceship, trade_item, etc.)
            output_file: Output CSV file path
            delimiter: CSV delimiter (default: comma)
            
        Returns:
            True if export successful
        """
        if asset_type not in self.asset_configs:
            print(f"ERROR: Unknown asset type: {asset_type}")
            return False
        
        config = self.asset_configs[asset_type]
        
        print(f"Exporting {asset_type} data to CSV...")
        
        # Load existing data files
        template_dir = self.project_dir / config['template_dir']
        if not template_dir.exists():
            print(f"ERROR: Template directory not found: {template_dir}")
            return False
        
        data_files = list(template_dir.glob('*.yaml'))
        print(f"Found {len(data_files)} data file(s)")
        
        if not data_files:
            print("WARNING: No data files to export")
            return False
        
        # Determine columns from data if not configured
        columns = config.get('columns', [])
        if not columns:
            # Use first file to determine structure
            with open(data_files[0], 'r', encoding='utf-8') as f:
                sample_data = yaml.safe_load(f)
            columns = self._infer_columns(sample_data)
        
        # Prepare CSV data
        headers = [col['header'] for col in columns]
        
        try:
            with open(output_file, 'w', newline='', encoding='utf-8') as csvfile:
                writer = csv.writer(csvfile, delimiter=delimiter)
                writer.writerow(headers)
                
                # Load and convert each file
                for data_file in data_files:
                    try:
                        with open(data_file, 'r', encoding='utf-8') as f:
                            data = yaml.safe_load(f)
                        
                        row = self._data_to_row(data, columns)
                        writer.writerow(row)
                        
                    except Exception as e:
                        print(f"WARNING: Failed to process {data_file.name}: {e}")
                        continue
            
            print(f"✓ Exported to {output_file}")
            print(f"  {len(data_files)} row(s) written")
            return True
            
        except Exception as e:
            print(f"ERROR: Failed to write CSV file: {e}")
            return False
    
    def import_from_csv(self, asset_type: str, input_file: Path,
                       delimiter: str = ',', validate: bool = True) -> bool:
        """
        Import data from CSV file and create/update YAML files
        
        Args:
            asset_type: Type of asset (spaceship, trade_item, etc.)
            input_file: Input CSV file path
            delimiter: CSV delimiter (default: comma)
            validate: Whether to validate data against schema
            
        Returns:
            True if import successful
        """
        if asset_type not in self.asset_configs:
            print(f"ERROR: Unknown asset type: {asset_type}")
            return False
        
        if not input_file.exists():
            print(f"ERROR: Input file not found: {input_file}")
            return False
        
        config = self.asset_configs[asset_type]
        
        print(f"Importing {asset_type} data from CSV...")
        
        # Read CSV file
        try:
            with open(input_file, 'r', newline='', encoding='utf-8') as csvfile:
                reader = csv.reader(csvfile, delimiter=delimiter)
                headers = next(reader)
                data_rows = list(reader)
            
            print(f"Found {len(data_rows)} row(s) to import")
            
        except Exception as e:
            print(f"ERROR: Failed to read CSV file: {e}")
            return False
        
        # Prepare output directory
        template_dir = self.project_dir / config['template_dir']
        template_dir.mkdir(parents=True, exist_ok=True)
        
        # Determine column mapping
        columns = config.get('columns', [])
        if not columns:
            # Create basic column mapping from headers
            columns = [{'header': h, 'path': h, 'type': 'string'} for h in headers]
        
        # Process each row
        success_count = 0
        error_count = 0
        
        for i, row in enumerate(data_rows, start=2):
            try:
                # Convert row to data structure
                data = self._row_to_data(row, headers, columns)
                
                # Get ID for filename
                id_value = self._get_nested_value(data, config['id_field'].split('.'))
                if not id_value:
                    print(f"WARNING: Row {i} missing {config['id_field']}, skipping")
                    error_count += 1
                    continue
                
                # Validate if requested
                if validate and self.validator:
                    is_valid, errors = self.validator.validate_data(
                        config['schema_name'], data, f"Row {i}"
                    )
                    if not is_valid:
                        print(f"WARNING: Row {i} validation failed, skipping")
                        for error in errors[:3]:  # Show first 3 errors
                            print(f"  {error}")
                        error_count += 1
                        continue
                
                # Write to YAML file
                output_file = template_dir / f"{id_value}.yaml"
                with open(output_file, 'w', encoding='utf-8') as f:
                    yaml.dump(data, f, default_flow_style=False, sort_keys=False)
                
                print(f"✓ Imported: {output_file.name}")
                success_count += 1
                
            except Exception as e:
                print(f"ERROR: Failed to process row {i}: {e}")
                error_count += 1
                continue
        
        print(f"\nImport complete: {success_count} successful, {error_count} errors")
        return error_count == 0
    
    def _infer_columns(self, data: Dict[str, Any], prefix: str = '') -> List[Dict[str, str]]:
        """Infer column structure from data"""
        columns = []
        
        for key, value in data.items():
            path = f"{prefix}.{key}" if prefix else key
            
            if isinstance(value, dict):
                # Recurse into nested dictionaries
                columns.extend(self._infer_columns(value, path))
            elif isinstance(value, list):
                # Skip complex list types for CSV
                continue
            else:
                # Determine type
                if isinstance(value, bool):
                    value_type = 'boolean'
                elif isinstance(value, int):
                    value_type = 'integer'
                elif isinstance(value, float):
                    value_type = 'number'
                else:
                    value_type = 'string'
                
                columns.append({
                    'header': key,
                    'path': path,
                    'type': value_type
                })
        
        return columns
    
    def _data_to_row(self, data: Dict[str, Any], columns: List[Dict[str, str]]) -> List[Any]:
        """Convert data structure to CSV row"""
        row = []
        for col in columns:
            path_parts = col['path'].split('.')
            value = self._get_nested_value(data, path_parts)
            row.append(self._format_value(value, col.get('type', 'string')))
        return row
    
    def _row_to_data(self, row: List[Any], headers: List[str],
                     columns: List[Dict[str, str]]) -> Dict[str, Any]:
        """Convert CSV row to data structure"""
        data = {}
        
        # Create header to column mapping
        header_map = {}
        for col in columns:
            header_map[col['header']] = col
        
        # Process each cell
        for i, header in enumerate(headers):
            if header not in header_map:
                # Use basic mapping for unmapped columns
                header_map[header] = {
                    'header': header,
                    'path': header,
                    'type': 'string'
                }
            
            if i >= len(row):
                continue
            
            col = header_map[header]
            value = row[i]
            
            # Parse value based on type
            parsed_value = self._parse_value(value, col.get('type', 'string'))
            
            # Set nested value
            path_parts = col['path'].split('.')
            self._set_nested_value(data, path_parts, parsed_value)
        
        return data
    
    def _get_nested_value(self, data: Dict[str, Any], path: List[str]) -> Any:
        """Get value from nested dictionary using path"""
        current = data
        for key in path:
            if isinstance(current, dict) and key in current:
                current = current[key]
            else:
                return None
        return current
    
    def _set_nested_value(self, data: Dict[str, Any], path: List[str], value: Any):
        """Set value in nested dictionary using path"""
        current = data
        for key in path[:-1]:
            if key not in current:
                current[key] = {}
            current = current[key]
        
        if value is not None and value != '':
            current[path[-1]] = value
    
    def _format_value(self, value: Any, value_type: str) -> Any:
        """Format value for CSV"""
        if value is None:
            return ''
        
        if value_type == 'boolean':
            return 'TRUE' if value else 'FALSE'
        
        return value
    
    def _parse_value(self, value: Any, value_type: str) -> Any:
        """Parse value from CSV"""
        if value == '' or value is None:
            return None
        
        try:
            if value_type == 'integer':
                # Convert directly to int, handling potential float strings
                return int(float(str(value)))
            elif value_type == 'number':
                return float(value)
            elif value_type == 'boolean':
                if isinstance(value, bool):
                    return value
                value_str = str(value).strip().upper()
                return value_str in ['TRUE', 'YES', '1', 'Y']
            else:  # string
                return str(value)
        except (ValueError, TypeError):
            return value


def main():
    """Command line interface"""
    parser = argparse.ArgumentParser(
        description="CSV integration for game data management"
    )
    
    subparsers = parser.add_subparsers(dest='command', help='Command to execute')
    
    # Export command
    export_parser = subparsers.add_parser('export', help='Export data to CSV')
    export_parser.add_argument('--type', required=True,
                              help='Asset type (spaceship, trade_item, etc.)')
    export_parser.add_argument('--output', required=True,
                              help='Output CSV file path')
    export_parser.add_argument('--delimiter', default=',',
                              help='CSV delimiter (default: comma)')
    
    # Import command
    import_parser = subparsers.add_parser('import', help='Import data from CSV')
    import_parser.add_argument('--type', required=True,
                              help='Asset type (spaceship, trade_item, etc.)')
    import_parser.add_argument('--input', required=True,
                              help='Input CSV file path')
    import_parser.add_argument('--delimiter', default=',',
                              help='CSV delimiter (default: comma)')
    import_parser.add_argument('--no-validate', action='store_true',
                              help='Skip validation')
    
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        sys.exit(1)
    
    # Initialize integration
    integration = CSVIntegration()
    
    # Execute command
    if args.command == 'export':
        success = integration.export_to_csv(
            args.type,
            Path(args.output),
            args.delimiter
        )
        sys.exit(0 if success else 1)
    
    elif args.command == 'import':
        success = integration.import_from_csv(
            args.type,
            Path(args.input),
            args.delimiter,
            validate=not args.no_validate
        )
        sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
