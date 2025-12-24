# Data Management Quick Reference

Fast reference for common data management operations in Adastrea.

## Prerequisites

```bash
pip install jsonschema pyyaml google-auth google-auth-oauthlib google-auth-httplib2 google-api-python-client
```

## Google Sheets Operations

### Export to Google Sheets

```bash
# Export spaceships
python GoogleSheetsIntegration.py export --type spaceship --sheet-id YOUR_SHEET_ID

# Export trade items
python GoogleSheetsIntegration.py export --type trade_item --sheet-id YOUR_SHEET_ID
```

### Import from Google Sheets

```bash
# Import spaceships
python GoogleSheetsIntegration.py import --type spaceship --sheet-id YOUR_SHEET_ID

# Import trade items
python GoogleSheetsIntegration.py import --type trade_item --sheet-id YOUR_SHEET_ID
```

### Custom Sheet Names

```bash
# Export to custom sheet
python GoogleSheetsIntegration.py export --type spaceship --sheet-id YOUR_SHEET_ID --sheet-name "My Ships"

# Import from custom sheet
python GoogleSheetsIntegration.py import --type spaceship --sheet-id YOUR_SHEET_ID --sheet-name "My Ships"
```

## CSV Operations

### Export to CSV

```bash
# Export spaceships
python CSVIntegration.py export --type spaceship --output spaceships.csv

# Export trade items
python CSVIntegration.py export --type trade_item --output trade_items.csv

# Use different delimiter (TSV)
python CSVIntegration.py export --type spaceship --output ships.tsv --delimiter $'\t'
```

### Import from CSV

```bash
# Import spaceships
python CSVIntegration.py import --type spaceship --input spaceships.csv

# Import without validation (faster, but risky)
python CSVIntegration.py import --type spaceship --input spaceships.csv --no-validate

# Import TSV file
python CSVIntegration.py import --type trade_item --input items.tsv --delimiter $'\t'
```

## Validation Operations

### Validate Single File

```bash
# Validate spaceship
python Tools/SchemaValidator.py --schema spaceship --data Assets/SpaceshipTemplates/Scout_Pathfinder.yaml

# Validate trade item
python Tools/SchemaValidator.py --schema trade_item --data Assets/TradingTemplates/TradeItem_Platinum.yaml
```

### Batch Validation

```bash
# Validate all spaceships
python Tools/SchemaValidator.py --schema spaceship --directory Assets/SpaceshipTemplates

# Validate all trade items
python Tools/SchemaValidator.py --schema trade_item --directory Assets/TradingTemplates

# Validate with custom file pattern
python Tools/SchemaValidator.py --schema spaceship --directory Assets/SpaceshipTemplates --pattern "*.yaml"
```

### Generate Validation Report

```bash
# Save report to file
python Tools/SchemaValidator.py --schema spaceship --directory Assets/SpaceshipTemplates --report validation_report.txt

# View report
cat validation_report.txt
```

## Complete Workflows

### Workflow 1: Create New Ships in Google Sheets

```bash
# 1. Export existing ships to Google Sheets
python GoogleSheetsIntegration.py export --type spaceship --sheet-id YOUR_SHEET_ID

# 2. Edit in Google Sheets (add new rows)
# 3. Import updated data

python GoogleSheetsIntegration.py import --type spaceship --sheet-id YOUR_SHEET_ID

# 4. Verify files created
ls -la Assets/SpaceshipTemplates/
```

### Workflow 2: Bulk Update via CSV

```bash
# 1. Export to CSV
python CSVIntegration.py export --type spaceship --output ships.csv

# 2. Edit ships.csv in Excel/LibreOffice
# 3. Import changes

python CSVIntegration.py import --type spaceship --input ships.csv

# 4. Validate changes
python Tools/SchemaValidator.py --schema spaceship --directory Assets/SpaceshipTemplates
```

### Workflow 3: Quality Assurance

```bash
# 1. Validate all asset types
python Tools/SchemaValidator.py --schema spaceship --directory Assets/SpaceshipTemplates --report spaceship_validation.txt
python Tools/SchemaValidator.py --schema trade_item --directory Assets/TradingTemplates --report trade_item_validation.txt

# 2. Review reports
cat spaceship_validation.txt
cat trade_item_validation.txt

# 3. Fix errors in Google Sheets or YAML files
# 4. Re-validate

python Tools/SchemaValidator.py --schema spaceship --directory Assets/SpaceshipTemplates
```

### Workflow 4: Team Collaboration

```bash
# Team Lead: Export current data
python GoogleSheetsIntegration.py export --type spaceship --sheet-id YOUR_SHEET_ID
python GoogleSheetsIntegration.py export --type trade_item --sheet-id YOUR_SHEET_ID

# Share spreadsheet with team (Editor permissions)

# Team: Edit in Google Sheets collaboratively

# Team Lead: Import updated data
python GoogleSheetsIntegration.py import --type spaceship --sheet-id YOUR_SHEET_ID
python GoogleSheetsIntegration.py import --type trade_item --sheet-id YOUR_SHEET_ID

# Validate before committing
python Tools/SchemaValidator.py --schema spaceship --directory Assets/SpaceshipTemplates
python Tools/SchemaValidator.py --schema trade_item --directory Assets/TradingTemplates

# Commit to git
git add Assets/
git commit -m "Update ship and item data from team review"
git push
```

## Troubleshooting

### Authentication Issues

```bash
# Delete saved token and re-authenticate
rm token.pickle
python GoogleSheetsIntegration.py export --type spaceship --sheet-id YOUR_SHEET_ID
```

### Validation Errors

```bash
# View detailed validation errors
python Tools/SchemaValidator.py --schema spaceship --data path/to/file.yaml --verbose

# Check schema definitions
cat Schemas/spaceship_schema.json | python -m json.tool
```

### Import Failures

```bash
# Import without validation to see data issues
python CSVIntegration.py import --type spaceship --input ships.csv --no-validate

# Then validate manually
python Tools/SchemaValidator.py --schema spaceship --directory Assets/SpaceshipTemplates
```

## File Locations

```
Adastrea/
├── Schemas/                           # Schema definitions
│   ├── spaceship_schema.json
│   └── trade_item_schema.json
├── Assets/
│   ├── SpaceshipTemplates/            # Ship YAML files
│   └── TradingTemplates/              # Trade item YAML files
├── SchemaValidator.py                 # Validation tool
├── GoogleSheetsIntegration.py         # Google Sheets integration
├── CSVIntegration.py                  # CSV import/export
├── AssetConfig.py                     # Shared configuration
├── credentials.json                   # Google API credentials (DO NOT COMMIT)
└── token.pickle                       # Saved auth token (DO NOT COMMIT)
```

## Git Ignore

Add to `.gitignore`:

```
# Google Sheets credentials
credentials.json
token.pickle

# Validation reports (optional)
*_validation_report.txt
validation_report.txt
```

## Asset Types Reference

| Type | Schema | Template Dir | ID Field | Sheet Name |
|------|--------|--------------|----------|------------|
| spaceship | spaceship_schema.json | Assets/SpaceshipTemplates | BasicInfo.ShipID | Spaceships |
| trade_item | trade_item_schema.json | Assets/TradingTemplates | ItemID | TradeItems |

## Common Column Mappings

### Spaceship Columns

- Ship ID → BasicInfo.ShipID
- Ship Name → BasicInfo.ShipName
- Description → BasicInfo.Description
- Ship Class → BasicInfo.ShipClass
- Hull Strength → CoreStats.HullStrength
- Cargo Capacity → CoreStats.CargoCapacity
- etc.

### Trade Item Columns

- Item ID → ItemID
- Item Name → ItemName
- Category → Category
- Base Price → BasePrice
- Legality Status → LegalityStatus
- etc.

## Tips

1. **Always validate** before committing changes
2. **Use Google Sheets** for team collaboration
3. **Use CSV** for bulk operations and Excel compatibility
4. **Export before import** to have a backup
5. **Test with small datasets** first
6. **Review validation reports** before fixing issues
7. **Keep spreadsheets organized** with clear sheet names
8. **Document custom validations** in schema guide

## Getting Help

- Schema Guide: `SCHEMA_GUIDE.md`
- Google Sheets Guide: `GOOGLE_SHEETS_GUIDE.md`
- Data Asset Workflow: `YAML_IMPORT_GUIDE.md`
- Project README: `README.md`
