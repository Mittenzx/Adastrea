#!/usr/bin/env python3
"""
Adastrea - Google Sheets Integration

This script provides integration with Google Sheets for importing and exporting game data.
It uses the Google Sheets API v4 to read and write data.

Features:
- Export Data Assets to Google Sheets
- Import/Update Data Assets from Google Sheets
- Two-way sync with validation
- Schema-based column mapping
- Batch operations support

Setup:
1. Enable Google Sheets API in Google Cloud Console
2. Create OAuth 2.0 credentials or Service Account
3. Download credentials.json and place in project root
4. Run authentication flow on first use

Usage:
    # Export existing data to Google Sheets
    python GoogleSheetsIntegration.py export --type spaceship --sheet-id YOUR_SHEET_ID
    
    # Import data from Google Sheets
    python GoogleSheetsIntegration.py import --type spaceship --sheet-id YOUR_SHEET_ID
    
    # Sync (two-way update)
    python GoogleSheetsIntegration.py sync --type spaceship --sheet-id YOUR_SHEET_ID
"""

import os
import sys
import json
import yaml
import argparse
import pickle
from pathlib import Path
from typing import Dict, Any, List, Optional, Tuple
from datetime import datetime

# Try to import Google Sheets API libraries
try:
    from google.auth.transport.requests import Request
    from google.oauth2.credentials import Credentials
    from google_auth_oauthlib.flow import InstalledAppFlow
    from googleapiclient.discovery import build
    from googleapiclient.errors import HttpError
    GOOGLE_AVAILABLE = True
except ImportError:
    GOOGLE_AVAILABLE = False
    print("WARNING: Google API libraries not available.")
    print("Install with: pip install google-auth google-auth-oauthlib google-auth-httplib2 google-api-python-client")

# Import our schema validator
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


# Google Sheets API scopes
SCOPES = ['https://www.googleapis.com/auth/spreadsheets']


class GoogleSheetsIntegration:
    """
    Integration with Google Sheets for game data management
    """
    
    def __init__(self, credentials_path: Optional[Path] = None):
        """
        Initialize Google Sheets integration
        
        Args:
            credentials_path: Path to credentials.json file
        """
        if not GOOGLE_AVAILABLE:
            raise RuntimeError("Google API libraries are required. See installation instructions above.")
        
        self.project_dir = Path.cwd()
        
        # Credentials file
        if credentials_path:
            self.credentials_path = Path(credentials_path)
        else:
            self.credentials_path = self.project_dir / "credentials.json"
        
        # Token storage
        self.token_path = self.project_dir / "token.pickle"
        
        # Google Sheets service
        self.service = None
        self.creds = None
        
        # Schema validator
        if VALIDATOR_AVAILABLE:
            self.validator = SchemaValidator()
        else:
            self.validator = None
        
        # Asset type configurations
        if ASSET_CONFIG_AVAILABLE:
            self.asset_configs = get_asset_configs()
        else:
            self.asset_configs = self._get_asset_configs()
    
    def _get_asset_configs(self) -> Dict[str, Dict[str, Any]]:
        """
        Get configuration for each asset type
        
        Returns:
            Dictionary of asset type configurations
        """
        return {
            'spaceship': {
                'schema_name': 'spaceship',
                'template_dir': 'Assets/SpaceshipTemplates',
                'content_path': '/Game/Spaceships/DataAssets',
                'id_field': 'ShipID',
                'name_field': 'ShipName',
                'sheet_name': 'Spaceships',
                'columns': self._get_spaceship_columns()
            },
            'trade_item': {
                'schema_name': 'trade_item',
                'template_dir': 'Assets/TradingTemplates',
                'content_path': '/Game/Trading/DataAssets',
                'id_field': 'ItemID',
                'name_field': 'ItemName',
                'sheet_name': 'TradeItems',
                'columns': self._get_trade_item_columns()
            },
            'faction': {
                'schema_name': 'faction',
                'template_dir': 'Assets/FactionTemplates',
                'content_path': '/Game/Factions/DataAssets',
                'id_field': 'FactionID',
                'name_field': 'FactionName',
                'sheet_name': 'Factions',
                'columns': []  # To be defined
            },
            'personnel': {
                'schema_name': 'personnel',
                'template_dir': 'Assets/PersonnelTemplates',
                'content_path': '/Game/Personnel/DataAssets',
                'id_field': 'PersonnelID',
                'name_field': 'DisplayName',
                'sheet_name': 'Personnel',
                'columns': []  # To be defined
            }
        }
    
    def _get_spaceship_columns(self) -> List[Dict[str, str]]:
        """Get column definitions for spaceship data"""
        return [
            # Basic Info
            {'header': 'Ship ID', 'path': 'BasicInfo.ShipID', 'type': 'string'},
            {'header': 'Ship Name', 'path': 'BasicInfo.ShipName', 'type': 'string'},
            {'header': 'Description', 'path': 'BasicInfo.Description', 'type': 'string'},
            {'header': 'Ship Class', 'path': 'BasicInfo.ShipClass', 'type': 'string'},
            
            # Core Stats
            {'header': 'Hull Strength', 'path': 'CoreStats.HullStrength', 'type': 'number'},
            {'header': 'Cargo Capacity', 'path': 'CoreStats.CargoCapacity', 'type': 'number'},
            {'header': 'Crew Required', 'path': 'CoreStats.CrewRequired', 'type': 'integer'},
            {'header': 'Max Crew', 'path': 'CoreStats.MaxCrew', 'type': 'integer'},
            {'header': 'Modular Points', 'path': 'CoreStats.ModularPoints', 'type': 'integer'},
            
            # Combat Stats
            {'header': 'Armor Rating', 'path': 'CombatStats.ArmorRating', 'type': 'number'},
            {'header': 'Shield Strength', 'path': 'CombatStats.ShieldStrength', 'type': 'number'},
            {'header': 'Shield Recharge Rate', 'path': 'CombatStats.ShieldRechargeRate', 'type': 'number'},
            {'header': 'Weapon Slots', 'path': 'CombatStats.WeaponSlots', 'type': 'integer'},
            {'header': 'Weapon Power Capacity', 'path': 'CombatStats.WeaponPowerCapacity', 'type': 'number'},
            {'header': 'Point Defense Rating', 'path': 'CombatStats.PointDefenseRating', 'type': 'number'},
            
            # Mobility Stats
            {'header': 'Max Speed', 'path': 'MobilityStats.MaxSpeed', 'type': 'number'},
            {'header': 'Acceleration', 'path': 'MobilityStats.Acceleration', 'type': 'number'},
            {'header': 'Maneuverability', 'path': 'MobilityStats.Maneuverability', 'type': 'integer'},
            {'header': 'Jump Range', 'path': 'MobilityStats.JumpRange', 'type': 'number'},
            {'header': 'Fuel Capacity', 'path': 'MobilityStats.FuelCapacity', 'type': 'number'},
            {'header': 'Fuel Consumption Rate', 'path': 'MobilityStats.FuelConsumptionRate', 'type': 'number'},
            
            # Utility Stats
            {'header': 'Sensor Range', 'path': 'UtilityStats.SensorRange', 'type': 'number'},
            {'header': 'Sensor Resolution', 'path': 'UtilityStats.SensorResolution', 'type': 'integer'},
            {'header': 'Stealth Rating', 'path': 'UtilityStats.StealthRating', 'type': 'integer'},
            {'header': 'Repair System Rating', 'path': 'UtilityStats.RepairSystemRating', 'type': 'integer'},
            {'header': 'Science Rating', 'path': 'UtilityStats.ScienceRating', 'type': 'integer'},
            {'header': 'Medical Rating', 'path': 'UtilityStats.MedicalRating', 'type': 'integer'},
            
            # Operational Stats
            {'header': 'Power Capacity', 'path': 'OperationalStats.PowerCapacity', 'type': 'number'},
            {'header': 'Life Support Rating', 'path': 'OperationalStats.LifeSupportRating', 'type': 'integer'},
            {'header': 'Maintenance Level', 'path': 'OperationalStats.MaintenanceLevel', 'type': 'integer'},
            {'header': 'Hangar Capacity', 'path': 'OperationalStats.HangarCapacity', 'type': 'integer'},
            {'header': 'Drone Capacity', 'path': 'OperationalStats.DroneCapacity', 'type': 'integer'},
            {'header': 'AI System Rating', 'path': 'OperationalStats.AISystemRating', 'type': 'integer'},
            
            # Advanced Stats
            {'header': 'Diplomacy Rating', 'path': 'AdvancedStats.DiplomacyRating', 'type': 'integer'},
            {'header': 'Boarding Capability', 'path': 'AdvancedStats.BoardingCapability', 'type': 'integer'},
            {'header': 'Customizable Slots', 'path': 'AdvancedStats.CustomizableSlots', 'type': 'integer'},
            {'header': 'EWar Rating', 'path': 'AdvancedStats.EWarRating', 'type': 'integer'},
            {'header': 'Mining Rating', 'path': 'AdvancedStats.MiningRating', 'type': 'integer'},
            
            # Lore
            {'header': 'Manufacturer', 'path': 'Lore.Manufacturer', 'type': 'string'},
            {'header': 'Year Built', 'path': 'Lore.YearBuilt', 'type': 'integer'},
            {'header': 'Rarity Tier', 'path': 'Lore.RarityTier', 'type': 'string'},
            {'header': 'Lore Notes', 'path': 'Lore.LoreNotes', 'type': 'string'},
        ]
    
    def _get_trade_item_columns(self) -> List[Dict[str, str]]:
        """Get column definitions for trade item data"""
        return [
            {'header': 'Item ID', 'path': 'ItemID', 'type': 'string'},
            {'header': 'Item Name', 'path': 'ItemName', 'type': 'string'},
            {'header': 'Description', 'path': 'Description', 'type': 'string'},
            {'header': 'Category', 'path': 'Category', 'type': 'string'},
            {'header': 'Material Data', 'path': 'MaterialData', 'type': 'string'},
            {'header': 'Base Price', 'path': 'BasePrice', 'type': 'number'},
            {'header': 'Volatility Multiplier', 'path': 'PriceVolatility.VolatilityMultiplier', 'type': 'number'},
            {'header': 'Min Price Deviation', 'path': 'PriceVolatility.MinPriceDeviation', 'type': 'number'},
            {'header': 'Max Price Deviation', 'path': 'PriceVolatility.MaxPriceDeviation', 'type': 'number'},
            {'header': 'Affected By Supply/Demand', 'path': 'bAffectedBySupplyDemand', 'type': 'boolean'},
            {'header': 'Affected By Market Events', 'path': 'bAffectedByMarketEvents', 'type': 'boolean'},
            {'header': 'Legality Status', 'path': 'LegalityStatus', 'type': 'string'},
            {'header': 'Min Reputation Required', 'path': 'TradeRestrictions.MinReputationRequired', 'type': 'integer'},
            {'header': 'Contraband Fine Multiplier', 'path': 'ContrabandFineMultiplier', 'type': 'number'},
            {'header': 'Volume Per Unit', 'path': 'VolumePerUnit', 'type': 'number'},
            {'header': 'Mass Per Unit', 'path': 'MassPerUnit', 'type': 'number'},
            {'header': 'Standard Lot Size', 'path': 'StandardLotSize', 'type': 'integer'},
            {'header': 'Typical Market Stock', 'path': 'TypicalMarketStock', 'type': 'integer'},
            {'header': 'Replenishment Rate', 'path': 'ReplenishmentRate', 'type': 'integer'},
            {'header': 'AI Trade Priority', 'path': 'AITradePriority', 'type': 'integer'},
            {'header': 'AI Hoardable', 'path': 'bAIHoardable', 'type': 'boolean'},
            {'header': 'AI Arbitrage Enabled', 'path': 'bAIArbitrageEnabled', 'type': 'boolean'},
        ]
    
    def authenticate(self) -> bool:
        """
        Authenticate with Google Sheets API
        
        Returns:
            True if authentication successful
        """
        try:
            # Load existing token
            if self.token_path.exists():
                with open(self.token_path, 'rb') as token:
                    self.creds = pickle.load(token)
            
            # Refresh or get new credentials
            if not self.creds or not self.creds.valid:
                if self.creds and self.creds.expired and self.creds.refresh_token:
                    self.creds.refresh(Request())
                else:
                    if not self.credentials_path.exists():
                        print(f"ERROR: Credentials file not found: {self.credentials_path}")
                        print("Please download OAuth 2.0 credentials from Google Cloud Console")
                        print("and save as 'credentials.json' in the project root.")
                        return False
                    
                    flow = InstalledAppFlow.from_client_secrets_file(
                        str(self.credentials_path), SCOPES)
                    self.creds = flow.run_local_server(port=0)
                
                # Save credentials
                with open(self.token_path, 'wb') as token:
                    pickle.dump(self.creds, token)
            
            # Build service
            self.service = build('sheets', 'v4', credentials=self.creds)
            print("✓ Authenticated with Google Sheets API")
            return True
            
        except Exception as e:
            print(f"ERROR: Authentication failed: {e}")
            return False
    
    def export_to_sheets(self, asset_type: str, spreadsheet_id: str, 
                        sheet_name: Optional[str] = None) -> bool:
        """
        Export existing data assets to Google Sheets
        
        Args:
            asset_type: Type of asset (spaceship, trade_item, etc.)
            spreadsheet_id: Google Sheets spreadsheet ID
            sheet_name: Optional sheet name (defaults to asset type config)
            
        Returns:
            True if export successful
        """
        if not self.service:
            print("ERROR: Not authenticated. Call authenticate() first.")
            return False
        
        if asset_type not in self.asset_configs:
            print(f"ERROR: Unknown asset type: {asset_type}")
            return False
        
        config = self.asset_configs[asset_type]
        sheet_name = sheet_name or config['sheet_name']
        
        print(f"Exporting {asset_type} data to Google Sheets...")
        
        # Load existing data files
        template_dir = self.project_dir / config['template_dir']
        if not template_dir.exists():
            print(f"ERROR: Template directory not found: {template_dir}")
            return False
        
        data_files = list(template_dir.glob('*.yaml'))
        print(f"Found {len(data_files)} data file(s)")
        
        # Prepare sheet data
        headers = [col['header'] for col in config['columns']]
        rows = [headers]
        
        # Load and convert each file
        for data_file in data_files:
            try:
                with open(data_file, 'r', encoding='utf-8') as f:
                    data = yaml.safe_load(f)
                
                row = self._data_to_row(data, config['columns'])
                rows.append(row)
                
            except Exception as e:
                print(f"WARNING: Failed to load {data_file.name}: {e}")
                continue
        
        # Write to Google Sheets
        try:
            # Clear existing data
            self.service.spreadsheets().values().clear(
                spreadsheetId=spreadsheet_id,
                range=f"{sheet_name}!A:ZZ"
            ).execute()
            
            # Write new data
            body = {'values': rows}
            result = self.service.spreadsheets().values().update(
                spreadsheetId=spreadsheet_id,
                range=f"{sheet_name}!A1",
                valueInputOption='RAW',
                body=body
            ).execute()
            
            print(f"✓ Exported {len(rows)-1} row(s) to {sheet_name}")
            print(f"   Spreadsheet: https://docs.google.com/spreadsheets/d/{spreadsheet_id}")
            return True
            
        except HttpError as e:
            print(f"ERROR: Failed to write to Google Sheets: {e}")
            return False
    
    def import_from_sheets(self, asset_type: str, spreadsheet_id: str,
                          sheet_name: Optional[str] = None,
                          validate: bool = True) -> bool:
        """
        Import data from Google Sheets and create/update YAML files
        
        Args:
            asset_type: Type of asset (spaceship, trade_item, etc.)
            spreadsheet_id: Google Sheets spreadsheet ID
            sheet_name: Optional sheet name (defaults to asset type config)
            validate: Whether to validate data against schema
            
        Returns:
            True if import successful
        """
        if not self.service:
            print("ERROR: Not authenticated. Call authenticate() first.")
            return False
        
        if asset_type not in self.asset_configs:
            print(f"ERROR: Unknown asset type: {asset_type}")
            return False
        
        config = self.asset_configs[asset_type]
        sheet_name = sheet_name or config['sheet_name']
        
        print(f"Importing {asset_type} data from Google Sheets...")
        
        # Read data from Google Sheets
        try:
            result = self.service.spreadsheets().values().get(
                spreadsheetId=spreadsheet_id,
                range=f"{sheet_name}!A:ZZ"
            ).execute()
            
            rows = result.get('values', [])
            
            if not rows:
                print("WARNING: No data found in sheet")
                return False
            
            if len(rows) < 2:
                print("WARNING: Sheet only contains headers, no data rows")
                return False
            
            headers = rows[0]
            data_rows = rows[1:]
            
            print(f"Found {len(data_rows)} row(s) to import")
            
        except HttpError as e:
            print(f"ERROR: Failed to read from Google Sheets: {e}")
            return False
        
        # Prepare output directory
        template_dir = self.project_dir / config['template_dir']
        template_dir.mkdir(parents=True, exist_ok=True)
        
        # Process each row
        success_count = 0
        error_count = 0
        
        for i, row in enumerate(data_rows, start=2):
            try:
                # Convert row to data structure
                data = self._row_to_data(row, headers, config['columns'])
                
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
    
    def _data_to_row(self, data: Dict[str, Any], columns: List[Dict[str, str]]) -> List[Any]:
        """Convert data structure to spreadsheet row"""
        row = []
        for col in columns:
            path_parts = col['path'].split('.')
            value = self._get_nested_value(data, path_parts)
            row.append(self._format_value(value, col['type']))
        return row
    
    def _row_to_data(self, row: List[Any], headers: List[str], 
                     columns: List[Dict[str, str]]) -> Dict[str, Any]:
        """Convert spreadsheet row to data structure"""
        data = {}
        
        # Create header to column mapping
        header_map = {}
        for col in columns:
            header_map[col['header']] = col
        
        # Process each cell
        for i, header in enumerate(headers):
            if header not in header_map:
                continue
            
            if i >= len(row):
                continue
            
            col = header_map[header]
            value = row[i]
            
            # Parse value based on type
            parsed_value = self._parse_value(value, col['type'])
            
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
        """Format value for spreadsheet"""
        if value is None:
            return ''
        
        if value_type == 'boolean':
            return 'TRUE' if value else 'FALSE'
        
        return value
    
    def _parse_value(self, value: Any, value_type: str) -> Any:
        """Parse value from spreadsheet"""
        if value == '' or value is None:
            return None
        
        try:
            if value_type == 'integer':
                return int(float(value))
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
            return None


def main():
    """Command line interface"""
    parser = argparse.ArgumentParser(
        description="Google Sheets integration for game data management"
    )
    
    subparsers = parser.add_subparsers(dest='command', help='Command to execute')
    
    # Export command
    export_parser = subparsers.add_parser('export', help='Export data to Google Sheets')
    export_parser.add_argument('--type', required=True, 
                              help='Asset type (spaceship, trade_item, etc.)')
    export_parser.add_argument('--sheet-id', required=True,
                              help='Google Sheets spreadsheet ID')
    export_parser.add_argument('--sheet-name',
                              help='Sheet name (optional)')
    
    # Import command
    import_parser = subparsers.add_parser('import', help='Import data from Google Sheets')
    import_parser.add_argument('--type', required=True,
                              help='Asset type (spaceship, trade_item, etc.)')
    import_parser.add_argument('--sheet-id', required=True,
                              help='Google Sheets spreadsheet ID')
    import_parser.add_argument('--sheet-name',
                              help='Sheet name (optional)')
    import_parser.add_argument('--no-validate', action='store_true',
                              help='Skip validation')
    
    # Credentials path
    parser.add_argument('--credentials',
                       help='Path to credentials.json file')
    
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        sys.exit(1)
    
    # Initialize integration
    try:
        integration = GoogleSheetsIntegration(
            credentials_path=Path(args.credentials) if args.credentials else None
        )
    except RuntimeError as e:
        print(f"ERROR: {e}")
        sys.exit(1)
    
    # Authenticate
    if not integration.authenticate():
        sys.exit(1)
    
    # Execute command
    if args.command == 'export':
        success = integration.export_to_sheets(
            args.type,
            args.sheet_id,
            args.sheet_name
        )
        sys.exit(0 if success else 1)
    
    elif args.command == 'import':
        success = integration.import_from_sheets(
            args.type,
            args.sheet_id,
            args.sheet_name,
            validate=not args.no_validate
        )
        sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
