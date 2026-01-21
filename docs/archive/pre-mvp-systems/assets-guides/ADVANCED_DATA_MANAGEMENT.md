# Advanced Data Management System

## Overview

The Advanced Data Management System provides comprehensive tools for importing, exporting, and validating game data using spreadsheets and CSV files. This system makes it easy for designers and teams to manage large amounts of game content without writing code.

## Key Features

✅ **Google Sheets Integration** - Primary data management interface  
✅ **CSV Import/Export** - Excel and spreadsheet tool compatibility  
✅ **Schema Validation** - Automatic data quality checks  
✅ **Batch Operations** - Process multiple assets at once  
✅ **Team Collaboration** - Real-time collaborative editing  
✅ **Error Reporting** - Detailed validation reports  

## Quick Start

### Prerequisites

```bash
# Install required Python packages
pip install google-auth google-auth-oauthlib google-auth-httplib2 google-api-python-client jsonschema pyyaml
```

### Google Sheets Setup (One-Time)

1. **Enable Google Sheets API**:
   - Go to [Google Cloud Console](https://console.cloud.google.com/)
   - Create project and enable Google Sheets API
   - Create OAuth 2.0 credentials (Desktop app)
   - Download `credentials.json` to project root

2. **Authenticate**:
   ```bash
   python Tools/GoogleSheetsIntegration.py export --type spaceship --sheet-id YOUR_SHEET_ID
   # Browser window opens for authentication
   ```

### Basic Workflow

```bash
# 1. Export existing data to Google Sheets
python Tools/GoogleSheetsIntegration.py export --type spaceship --sheet-id YOUR_SHEET_ID

# 2. Edit data in Google Sheets

# 3. Import updated data
python Tools/GoogleSheetsIntegration.py import --type spaceship --sheet-id YOUR_SHEET_ID

# 4. Validate changes
python Tools/SchemaValidator.py --schema spaceship --directory Assets/SpaceshipTemplates
```

## Tools Overview

### Schema Validator

Validates game data against JSON Schema definitions.

**Command Line:**
```bash
# Validate single file
python Tools/SchemaValidator.py --schema spaceship --data Assets/SpaceshipTemplates/Scout_Pathfinder.yaml

# Validate directory
python Tools/SchemaValidator.py --schema spaceship --directory Assets/SpaceshipTemplates

# Generate report
python Tools/SchemaValidator.py --schema spaceship --directory Assets/SpaceshipTemplates --report validation.txt
```

**Features:**
- JSON Schema-based validation
- Custom validation rules
- Batch validation
- Detailed error reporting
- Validation reports

### Google Sheets Integration

Primary interface for team collaboration and bulk editing.

**Command Line:**
```bash
# Export to Google Sheets
python Tools/GoogleSheetsIntegration.py export --type spaceship --sheet-id YOUR_SHEET_ID

# Import from Google Sheets
python Tools/GoogleSheetsIntegration.py import --type spaceship --sheet-id YOUR_SHEET_ID

# Custom sheet name
python Tools/GoogleSheetsIntegration.py export --type spaceship --sheet-id YOUR_SHEET_ID --sheet-name "My Ships"
```

**Features:**
- Two-way sync with Google Sheets
- Real-time collaboration
- Automatic schema validation on import
- Support for multiple asset types
- OAuth 2.0 authentication

### CSV Integration

Alternative for Excel and offline editing.

**Command Line:**
```bash
# Export to CSV
python Tools/CSVIntegration.py export --type spaceship --output spaceships.csv

# Import from CSV
python Tools/CSVIntegration.py import --type spaceship --input spaceships.csv

# Use TSV format
python Tools/CSVIntegration.py export --type spaceship --output ships.tsv --delimiter $'\t'
```

**Features:**
- Export data to CSV format
- Import from CSV/TSV/Excel
- Same column mapping as Google Sheets
- Offline editing capability
- Compatible with Excel, LibreOffice, etc.

## Supported Asset Types

| Asset Type | Schema | Template Directory |
|------------|--------|-------------------|
| Spaceship | `spaceship_schema.json` | `Assets/SpaceshipTemplates` |
| Trade Item | `trade_item_schema.json` | `Assets/TradingTemplates` |
| Faction | Coming soon | `Assets/FactionTemplates` |
| Personnel | Coming soon | `Assets/PersonnelTemplates` |

## File Structure

```
Adastrea/
├── Schemas/                           # JSON Schema definitions
│   ├── spaceship_schema.json          # Spaceship data schema
│   └── trade_item_schema.json         # Trade item data schema
│
├── Assets/                            # Game data (YAML files)
│   ├── SpaceshipTemplates/            # Spaceship configurations
│   └── TradingTemplates/              # Trade item definitions
│
├── SchemaValidator.py                 # Validation tool
├── GoogleSheetsIntegration.py         # Google Sheets sync
├── CSVIntegration.py                  # CSV import/export
├── AssetConfig.py                     # Shared configuration
│
├── GOOGLE_SHEETS_GUIDE.md             # Complete Google Sheets guide
├── SCHEMA_GUIDE.md                    # Schema definition guide
├── DATA_MANAGEMENT_QUICK_REFERENCE.md # Command reference
└── GOOGLE_SHEETS_TEMPLATES.md         # Template examples
```

## Documentation

### Complete Guides

- **[Google Sheets Guide](GOOGLE_SHEETS_GUIDE.md)** - Complete setup and usage guide (13KB)
- **[Schema Guide](SCHEMA_GUIDE.md)** - Creating and using validation schemas
- **[Quick Reference](DATA_MANAGEMENT_QUICK_REFERENCE.md)** - Common commands and workflows
- **[Templates Guide](GOOGLE_SHEETS_TEMPLATES.md)** - Google Sheets template examples

### Quick Reference

See [DATA_MANAGEMENT_QUICK_REFERENCE.md](DATA_MANAGEMENT_QUICK_REFERENCE.md) for:
- Common commands
- Complete workflows
- Troubleshooting tips
- File locations

## Common Workflows

### Workflow 1: Create New Ships

1. Export existing ships to establish template
2. Add new rows in Google Sheets
3. Import updated data
4. Validate and commit

```bash
python Tools/GoogleSheetsIntegration.py export --type spaceship --sheet-id YOUR_SHEET_ID
# Edit in Google Sheets...
python Tools/GoogleSheetsIntegration.py import --type spaceship --sheet-id YOUR_SHEET_ID
python Tools/SchemaValidator.py --schema spaceship --directory Assets/SpaceshipTemplates
git add Assets/SpaceshipTemplates/
git commit -m "Add new ships"
```

### Workflow 2: Team Collaboration

1. Team lead exports current data
2. Share spreadsheet with team (Editor permissions)
3. Team edits collaboratively in real-time
4. Team lead imports and validates
5. Commit changes

```bash
# Team lead
python Tools/GoogleSheetsIntegration.py export --type spaceship --sheet-id YOUR_SHEET_ID
# Share spreadsheet...
python Tools/GoogleSheetsIntegration.py import --type spaceship --sheet-id YOUR_SHEET_ID
python Tools/SchemaValidator.py --schema spaceship --directory Assets/SpaceshipTemplates
git commit -m "Update ships from team review"
```

### Workflow 3: Bulk Update via CSV

1. Export to CSV
2. Edit in Excel/LibreOffice
3. Import changes
4. Validate

```bash
python Tools/CSVIntegration.py export --type spaceship --output ships.csv
# Edit ships.csv...
python Tools/CSVIntegration.py import --type spaceship --input ships.csv
python Tools/SchemaValidator.py --schema spaceship --directory Assets/SpaceshipTemplates
```

## Validation

### Schema Validation

All data is automatically validated against JSON schemas:

- **Type checking** - Ensures correct data types (string, number, boolean)
- **Range validation** - Checks values are within valid ranges
- **Required fields** - Verifies all required fields are present
- **Custom rules** - Additional validation beyond JSON Schema

### Validation Errors

Example validation error:

```
ERROR: Assets/SpaceshipTemplates/MyShip.yaml: CoreStats -> HullStrength: 0.5 is less than the minimum of 1
```

This tells you:
- **File**: `MyShip.yaml`
- **Location**: `CoreStats.HullStrength`
- **Problem**: Value is too small (0.5 < 1)

### Validation Reports

Generate detailed reports:

```bash
python Tools/SchemaValidator.py --schema spaceship --directory Assets/SpaceshipTemplates --report validation.txt
```

Report includes:
- Total files validated
- Number passed/failed
- Detailed error messages for failures
- List of all passed validations

## Integration with Existing Workflow

The data management system integrates seamlessly with the existing workflow:

```
YAML Templates → Schema Validation → Google Sheets/CSV ↔ Team Editing → Import → Data Assets
     ↓                    ↓                                                      ↓
  Version           Quality Control                                        Unreal Engine
  Control              (CI/CD)                                               (via Python)
```

### With YAMLtoDataAsset.py

The new system works alongside the existing YAML to Data Asset workflow:

1. **Create/Edit** data in Google Sheets or CSV
2. **Export** to YAML files (this system)
3. **Validate** with schemas (this system)
4. **Import** to Unreal Engine with YAMLtoDataAsset.py (existing)

## Security and Best Practices

### Credentials

**DO NOT COMMIT** these files to git:
- `credentials.json` - Google API credentials
- `token.pickle` - Saved authentication token

Add to `.gitignore`:
```
credentials.json
token.pickle
```

### Data Validation

- **Always validate** before committing to version control
- **Test with small datasets** before bulk operations
- **Review validation reports** before fixing issues
- **Use branches** for experimental changes

### Team Collaboration

- **Share spreadsheets** with appropriate permissions
- **Use Google Sheets history** to track changes
- **Coordinate imports** to avoid conflicts
- **Communicate** before major data changes

## Troubleshooting

### Authentication Issues

```bash
# Delete token and re-authenticate
rm token.pickle
python Tools/GoogleSheetsIntegration.py export --type spaceship --sheet-id YOUR_SHEET_ID
```

### Validation Errors

```bash
# View detailed errors
python Tools/SchemaValidator.py --schema spaceship --data path/to/file.yaml --verbose
```

### Import Failures

```bash
# Import without validation to see issues
python Tools/CSVIntegration.py import --type spaceship --input ships.csv --no-validate
# Then validate manually
python Tools/SchemaValidator.py --schema spaceship --directory Assets/SpaceshipTemplates
```

## Advanced Usage

### Custom Schemas

Create new schemas for other asset types:

1. Create schema file in `Schemas/` directory
2. Add column definitions to `AssetConfig.py`
3. Test with existing data
4. Document in guides

See [SCHEMA_GUIDE.md](SCHEMA_GUIDE.md) for details.

### Continuous Integration

Add validation to CI/CD pipeline:

```yaml
# .github/workflows/validate-data.yml
- name: Validate spaceships
  run: python Tools/SchemaValidator.py --schema spaceship --directory Assets/SpaceshipTemplates
```

## Support and Contribution

### Getting Help

1. Check documentation:
   - [Google Sheets Guide](GOOGLE_SHEETS_GUIDE.md)
   - [Schema Guide](SCHEMA_GUIDE.md)
   - [Quick Reference](DATA_MANAGEMENT_QUICK_REFERENCE.md)

2. Review validation error messages
3. Check schema files in `Schemas/` directory
4. See example YAML files in `Assets/` directories

### Contributing

To add support for new asset types:

1. Create JSON schema in `Schemas/`
2. Add column definitions to `AssetConfig.py`
3. Test export/import/validation
4. Update documentation
5. Submit pull request

## Future Enhancements

Planned features:

- [ ] Faction data schema and import
- [ ] Personnel data schema and import
- [ ] Excel direct integration (without CSV conversion)
- [ ] Unreal Editor UI for import/export
- [ ] Automated sync with Google Sheets
- [ ] Data diff and merge tools
- [ ] Visual schema editor

## License

Same as main Adastrea project (MIT License).
