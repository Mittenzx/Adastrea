# Google Sheets Integration Guide

Complete guide for using Google Sheets to manage Adastrea game data.

## Table of Contents

- [Overview](#overview)
- [Quick Start](#quick-start)
- [Setup Instructions](#setup-instructions)
- [Using Google Sheets](#using-google-sheets)
- [Command Reference](#command-reference)
- [Workflow Examples](#workflow-examples)
- [Troubleshooting](#troubleshooting)

## Overview

The Google Sheets integration allows you to:

✅ **Export** existing game data to Google Sheets for easy editing  
✅ **Import** data from Google Sheets back into the project  
✅ **Validate** all data against schema definitions  
✅ **Collaborate** with team members in real-time  
✅ **Version control** your spreadsheets using Google Sheets history  

### Supported Asset Types

- **Spaceships** - Ship configurations and stats
- **Trade Items** - Tradeable goods and resources
- **Factions** - Faction definitions (coming soon)
- **Personnel** - Crew and NPC data (coming soon)

## Quick Start

### 1. Setup (One-Time)

```bash
# Install required Python packages
pip install google-auth google-auth-oauthlib google-auth-httplib2 google-api-python-client jsonschema pyyaml

# Enable Google Sheets API and download credentials
# See "Setup Instructions" section below
```

### 2. Export Existing Data

```bash
# Export all spaceships to Google Sheets
python GoogleSheetsIntegration.py export --type spaceship --sheet-id YOUR_SHEET_ID

# Export trade items
python GoogleSheetsIntegration.py export --type trade_item --sheet-id YOUR_SHEET_ID
```

### 3. Edit in Google Sheets

1. Open the spreadsheet at: `https://docs.google.com/spreadsheets/d/YOUR_SHEET_ID`
2. Edit data in the spreadsheet
3. Data is automatically saved by Google Sheets

### 4. Import Updated Data

```bash
# Import updated data from Google Sheets
python GoogleSheetsIntegration.py import --type spaceship --sheet-id YOUR_SHEET_ID

# Data is validated and converted to YAML files
```

## Setup Instructions

### Step 1: Enable Google Sheets API

1. Go to [Google Cloud Console](https://console.cloud.google.com/)
2. Create a new project or select an existing one
3. Enable the **Google Sheets API**:
   - Go to "APIs & Services" → "Library"
   - Search for "Google Sheets API"
   - Click "Enable"

### Step 2: Create OAuth 2.0 Credentials

1. Go to "APIs & Services" → "Credentials"
2. Click "Create Credentials" → "OAuth client ID"
3. Configure OAuth consent screen if prompted:
   - User Type: External (for testing)
   - Add your email as a test user
4. Application type: **Desktop app**
5. Download the credentials JSON file
6. Save as `credentials.json` in your Adastrea project root

### Step 3: First-Time Authentication

```bash
# Run any command to trigger authentication
python GoogleSheetsIntegration.py export --type spaceship --sheet-id YOUR_SHEET_ID

# A browser window will open for authentication
# Sign in with your Google account
# Grant permissions to access Google Sheets

# Credentials will be saved in token.pickle for future use
```

### Step 4: Create a Google Sheet

1. Go to [Google Sheets](https://sheets.google.com/)
2. Create a new spreadsheet
3. Name it "Adastrea Game Data" (or whatever you prefer)
4. Copy the spreadsheet ID from the URL:
   ```
   https://docs.google.com/spreadsheets/d/SPREADSHEET_ID_HERE/edit
   ```

## Using Google Sheets

### Spreadsheet Structure

Each asset type has its own sheet within the spreadsheet:

- **Spaceships** - All spaceship data
- **TradeItems** - All trade item data
- **Factions** - Faction data (coming soon)
- **Personnel** - Personnel data (coming soon)

### Column Headers

**DO NOT** modify the column headers! The system uses them to map data correctly.

Headers are generated automatically during export and match the schema definitions.

### Data Entry Guidelines

#### Spaceship Data

| Column | Type | Valid Values | Notes |
|--------|------|--------------|-------|
| Ship ID | Text | Alphanumeric + underscore | Unique identifier, no spaces |
| Ship Name | Text | Any text | Display name |
| Ship Class | Text | Scout, Fighter, Freighter, etc. | See schema for full list |
| Hull Strength | Number | 1 - 100,000 | Health points |
| Cargo Capacity | Number | 0 - 50,000 | Tons |
| Crew Required | Integer | 1 - 10,000 | Minimum crew |
| Max Crew | Integer | 1 - 10,000 | Must be ≥ Crew Required |

#### Trade Item Data

| Column | Type | Valid Values | Notes |
|--------|------|--------------|-------|
| Item ID | Text | Alphanumeric + underscore | Unique identifier |
| Item Name | Text | Any text | Display name |
| Category | Text | RawMaterials, Technology, etc. | See schema |
| Base Price | Number | ≥ 0 | Price in credits |
| Legality Status | Text | Legal, Restricted, Contraband, etc. | Legal status |
| AI Trade Priority | Integer | 1 - 10 | Higher = more desirable |

#### Boolean Fields

Use one of these values:
- `TRUE` / `FALSE`
- `true` / `false`
- `YES` / `NO`
- `1` / `0`

#### Empty Values

Leave cells **empty** (not "0" or "N/A") for optional fields you don't want to set.

### Validation

Data is automatically validated during import:

✅ Required fields present  
✅ Data types correct  
✅ Values within valid ranges  
✅ IDs are unique  
✅ Cross-field consistency (e.g., CrewRequired ≤ MaxCrew)  

Validation errors are reported with specific row numbers and issues.

## Command Reference

### Export Data

Export existing YAML data to Google Sheets:

```bash
python GoogleSheetsIntegration.py export \
  --type ASSET_TYPE \
  --sheet-id SPREADSHEET_ID \
  [--sheet-name SHEET_NAME] \
  [--credentials PATH_TO_CREDENTIALS]
```

**Arguments:**
- `--type`: Asset type (spaceship, trade_item)
- `--sheet-id`: Google Sheets spreadsheet ID
- `--sheet-name`: Optional custom sheet name (defaults to asset type)
- `--credentials`: Optional path to credentials.json

**Examples:**

```bash
# Export spaceships to default "Spaceships" sheet
python GoogleSheetsIntegration.py export --type spaceship --sheet-id 1ABC...XYZ

# Export to custom sheet name
python GoogleSheetsIntegration.py export --type trade_item --sheet-id 1ABC...XYZ --sheet-name "Trade Items v2"
```

### Import Data

Import data from Google Sheets to YAML files:

```bash
python GoogleSheetsIntegration.py import \
  --type ASSET_TYPE \
  --sheet-id SPREADSHEET_ID \
  [--sheet-name SHEET_NAME] \
  [--no-validate] \
  [--credentials PATH_TO_CREDENTIALS]
```

**Arguments:**
- `--type`: Asset type (spaceship, trade_item)
- `--sheet-id`: Google Sheets spreadsheet ID
- `--sheet-name`: Optional custom sheet name
- `--no-validate`: Skip validation (not recommended)
- `--credentials`: Optional path to credentials.json

**Examples:**

```bash
# Import spaceships with validation
python GoogleSheetsIntegration.py import --type spaceship --sheet-id 1ABC...XYZ

# Import without validation (not recommended)
python GoogleSheetsIntegration.py import --type trade_item --sheet-id 1ABC...XYZ --no-validate
```

### Validate Data

Validate YAML files against schemas (without importing):

```bash
# Validate a single file
python SchemaValidator.py --schema spaceship --data Assets/SpaceshipTemplates/Scout_Pathfinder.yaml

# Validate entire directory
python SchemaValidator.py --schema spaceship --directory Assets/SpaceshipTemplates

# Generate validation report
python SchemaValidator.py --schema spaceship --directory Assets/SpaceshipTemplates --report validation_report.txt
```

## Workflow Examples

### Workflow 1: Create New Ships in Google Sheets

1. **Export existing ships** to establish template:
   ```bash
   python GoogleSheetsIntegration.py export --type spaceship --sheet-id YOUR_SHEET_ID
   ```

2. **Open spreadsheet** in browser

3. **Add new rows** for new ships:
   - Copy an existing row as template
   - Modify values for new ship
   - Ensure Ship ID is unique

4. **Import updated data**:
   ```bash
   python GoogleSheetsIntegration.py import --type spaceship --sheet-id YOUR_SHEET_ID
   ```

5. **Check output** in `Assets/SpaceshipTemplates/`
   - New YAML files created
   - Validation errors reported if any

### Workflow 2: Bulk Update Ship Stats

1. **Export to Google Sheets**:
   ```bash
   python GoogleSheetsIntegration.py export --type spaceship --sheet-id YOUR_SHEET_ID
   ```

2. **Edit multiple ships** in spreadsheet:
   - Use spreadsheet formulas for calculations
   - Apply bulk changes (e.g., increase all Hull Strength by 10%)
   - Use find/replace for consistent changes

3. **Import changes**:
   ```bash
   python GoogleSheetsIntegration.py import --type spaceship --sheet-id YOUR_SHEET_ID
   ```

4. **Review changes** using git:
   ```bash
   git diff Assets/SpaceshipTemplates/
   ```

### Workflow 3: Team Collaboration

1. **Team lead exports** current data:
   ```bash
   python GoogleSheetsIntegration.py export --type spaceship --sheet-id YOUR_SHEET_ID
   ```

2. **Share spreadsheet** with team members:
   - File → Share
   - Add team member emails
   - Grant "Editor" permissions

3. **Team members edit** simultaneously:
   - Google Sheets tracks who made what changes
   - Changes are real-time and auto-saved

4. **Team lead imports** when ready:
   ```bash
   python GoogleSheetsIntegration.py import --type spaceship --sheet-id YOUR_SHEET_ID
   ```

5. **Commit to git**:
   ```bash
   git add Assets/SpaceshipTemplates/
   git commit -m "Update ship stats from team review"
   ```

### Workflow 4: Data Review and Quality Control

1. **Export all data**:
   ```bash
   python GoogleSheetsIntegration.py export --type spaceship --sheet-id YOUR_SHEET_ID
   python GoogleSheetsIntegration.py export --type trade_item --sheet-id YOUR_SHEET_ID
   ```

2. **Use spreadsheet features** for QA:
   - Sort by stats to find outliers
   - Use conditional formatting to highlight issues
   - Create charts to visualize balance
   - Use filters to review specific ship classes

3. **Make corrections** in spreadsheet

4. **Validate before importing**:
   ```bash
   # This happens automatically during import
   python GoogleSheetsIntegration.py import --type spaceship --sheet-id YOUR_SHEET_ID
   ```

5. **Review validation report** if errors occur

## Troubleshooting

### Authentication Issues

**Problem:** `credentials.json not found`

**Solution:**
1. Download credentials from Google Cloud Console
2. Save as `credentials.json` in project root
3. Ensure filename is exactly `credentials.json`

**Problem:** `Authentication failed`

**Solution:**
1. Delete `token.pickle` file
2. Run command again to re-authenticate
3. Sign in with correct Google account
4. Grant all requested permissions

### Permission Errors

**Problem:** `The caller does not have permission`

**Solution:**
1. Ensure you're signed in with correct Google account
2. Check spreadsheet sharing settings
3. You must be Owner or Editor of the spreadsheet

### Import Errors

**Problem:** `Validation failed for Row X`

**Solution:**
1. Check error message for specific issue
2. Open spreadsheet and go to Row X
3. Fix the reported issue
4. Import again

**Problem:** `No data found in sheet`

**Solution:**
1. Check sheet name matches (case-sensitive)
2. Ensure sheet has both headers and data rows
3. Use `--sheet-name` if using custom name

### Data Type Errors

**Problem:** `Expected integer, got string`

**Solution:**
1. Remove any text from number columns
2. Remove commas from numbers (use 1000, not 1,000)
3. Use TRUE/FALSE for boolean columns

**Problem:** `Value out of range`

**Solution:**
1. Check schema for valid ranges
2. Adjust value to be within range
3. See schema files in `Schemas/` directory

### Missing Data

**Problem:** `Required field missing`

**Solution:**
1. Check which fields are required in schema
2. Add values for all required fields
3. Leave optional fields empty (not "N/A")

## Advanced Tips

### Using Sheet Formulas

You can use Google Sheets formulas for calculations:

```
# Calculate total power usage
=B2+C2+D2

# Calculate armor efficiency
=E2/F2

# Conditional stat adjustments
=IF(G2>1000, G2*1.1, G2)
```

**Important:** Formulas are evaluated before import. Only the resulting values are imported.

### Template Sheets

Create a "Template" sheet with:
- Sample data
- Data validation rules
- Dropdown lists for enum fields
- Conditional formatting for valid ranges

Copy rows from template when creating new entries.

### Version Control Integration

Best practices:

1. **Before export**: Commit current YAML files
2. **After import**: Review changes with `git diff`
3. **Commit with message**: Include what was changed
4. **Use branches**: For experimental changes

### Automation

You can automate workflows with scripts:

```bash
#!/bin/bash
# auto_sync.sh - Sync data daily

# Export to sheets
python GoogleSheetsIntegration.py export --type spaceship --sheet-id YOUR_SHEET_ID

# Wait for manual edits (run this later)
# python GoogleSheetsIntegration.py import --type spaceship --sheet-id YOUR_SHEET_ID
```

## Next Steps

- [Schema Definition Guide](SCHEMA_GUIDE.md) - Understanding schemas
- [CSV Import Guide](CSV_IMPORT_GUIDE.md) - Using CSV files
- [Data Asset Workflow](YAML_IMPORT_GUIDE.md) - YAML to Data Assets

## Support

For issues or questions:
1. Check schema files in `Schemas/` directory
2. Review validation error messages
3. See example YAML files in `Assets/` directories
4. Consult project documentation
