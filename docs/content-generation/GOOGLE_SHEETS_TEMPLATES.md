# Google Sheets Template Examples

This file contains information about template Google Sheets that can be used as starting points.

## Template Spreadsheets

### Option 1: Create from Scratch

Create a new Google Sheet with these sheets (tabs):

1. **Spaceships** - For spaceship data
2. **TradeItems** - For trade item data
3. **Factions** - For faction data (coming soon)
4. **Personnel** - For personnel data (coming soon)

### Option 2: Export Existing Data

The best way to create a template is to export your existing data:

```bash
# Create new spreadsheet in Google Sheets
# Copy the spreadsheet ID from URL
# Run export command

python GoogleSheetsIntegration.py export --type spaceship --sheet-id YOUR_SHEET_ID
python GoogleSheetsIntegration.py export --type trade_item --sheet-id YOUR_SHEET_ID
```

This creates properly formatted sheets with all columns.

## Spaceship Sheet Structure

### Column Headers (Row 1)

Copy these headers exactly into Row 1 of your "Spaceships" sheet:

```
Ship ID | Ship Name | Description | Ship Class | Hull Strength | Cargo Capacity | Crew Required | Max Crew | Modular Points | Armor Rating | Shield Strength | Shield Recharge Rate | Weapon Slots | Weapon Power Capacity | Point Defense Rating | Max Speed | Acceleration | Maneuverability | Jump Range | Fuel Capacity | Fuel Consumption Rate | Sensor Range | Sensor Resolution | Stealth Rating | Repair System Rating | Science Rating | Medical Rating | Power Capacity | Life Support Rating | Maintenance Level | Hangar Capacity | Drone Capacity | AI System Rating | Diplomacy Rating | Boarding Capability | Customizable Slots | EWar Rating | Mining Rating | Manufacturer | Year Built | Rarity Tier | Lore Notes
```

### Example Data (Row 2)

```
PathfinderScout | Pathfinder Scout | A lightweight reconnaissance vessel designed for deep space exploration and intelligence gathering. The Pathfinder trades armor and firepower for speed, sensors, and fuel efficiency. | Scout | 800 | 50 | 2 | 4 | 6 | 20 | 400 | 15 | 1 | 500 | 7 | 900 | 120 | 9 | 25 | 1500 | 50 | 25000 | 9 | 8 | 4 | 6 | 2 | 3000 | 6 | 3 | 0 | 10 | 7 | 2 | 1 | 4 | 6 | 0 | Vanguard Aerospace | 2498 | Uncommon | The Pathfinder-class has been the gold standard for scout vessels for over a decade.
```

### Column Details

| Column | Type | Range | Required | Notes |
|--------|------|-------|----------|-------|
| Ship ID | Text | 1-50 chars | Yes | Alphanumeric + underscore, no spaces |
| Ship Name | Text | 1-100 chars | Yes | Display name |
| Description | Text | 1-1000 chars | No | Multi-line OK |
| Ship Class | Text | 1-100 chars | Yes | Scout, Fighter, Gunship, etc. |
| Hull Strength | Number | 1-100,000 | Yes | Health points |
| Cargo Capacity | Number | 0-50,000 | Yes | Tons |
| Crew Required | Integer | 1-10,000 | Yes | Minimum crew |
| Max Crew | Integer | 1-10,000 | Yes | Maximum crew (≥ Crew Required) |
| ... | ... | ... | ... | See full schema for all fields |

## Trade Item Sheet Structure

### Column Headers (Row 1)

```
Item ID | Item Name | Description | Category | Material Data | Base Price | Volatility Multiplier | Min Price Deviation | Max Price Deviation | Affected By Supply/Demand | Affected By Market Events | Legality Status | Min Reputation Required | Contraband Fine Multiplier | Volume Per Unit | Mass Per Unit | Standard Lot Size | Typical Market Stock | Replenishment Rate | AI Trade Priority | AI Hoardable | AI Arbitrage Enabled
```

### Example Data (Row 2)

```
TradeItem_Platinum | Platinum | A precious metal used in advanced electronics and ship components. | RawMaterials | DA_Material_Platinum | 100 | 1.0 | 0.7 | 1.5 | TRUE | TRUE | Legal | -100 | 0 | 1.0 | 1.0 | 100 | 1000 | 100 | 5 | FALSE | TRUE
```

### Column Details

| Column | Type | Range | Required | Notes |
|--------|------|-------|----------|-------|
| Item ID | Text | 1-50 chars | Yes | Unique identifier |
| Item Name | Text | 1-100 chars | Yes | Display name |
| Category | Text | Enum | Yes | RawMaterials, Technology, Food, etc. |
| Base Price | Number | ≥ 0 | Yes | Price in credits |
| Legality Status | Text | Enum | No | Legal, Restricted, Contraband, etc. |
| AI Trade Priority | Integer | 1-10 | No | Higher = more desirable |

## Data Entry Tips

### Text Fields

- Leave empty if optional (don't use "N/A" or "-")
- Use quotes if text contains commas: "Ship, the"
- Multi-line descriptions are OK

### Numbers

- Don't use commas: `1000` not `1,000`
- Decimals are OK: `123.45`
- Leave empty if optional, don't use 0

### Booleans

Use any of these:
- TRUE / FALSE
- true / false  
- YES / NO
- 1 / 0

### IDs

- Must start with a letter
- Can contain letters, numbers, underscore
- No spaces or special characters
- Examples: `PathfinderScout`, `TradeItem_Platinum`

## Spreadsheet Setup Tips

### Formatting

1. **Freeze Header Row**: View → Freeze → 1 row
2. **Data Validation**: Add dropdowns for enum fields
3. **Conditional Formatting**: Highlight errors or out-of-range values
4. **Number Formatting**: Format number columns as numbers

### Example Data Validation

For "Category" column in Trade Items sheet:

1. Select Category column (column D)
2. Data → Data validation
3. Criteria: List of items
4. Items: `RawMaterials,RefinedGoods,Components,Technology,Food,Luxury,Contraband,Military,Medical,Data,Other`
5. Show dropdown list: Yes
6. Reject input: Yes

### Example Conditional Formatting

Highlight invalid Ship IDs:

1. Select Ship ID column (column A)
2. Format → Conditional formatting
3. Format rules: Custom formula is
4. Formula: `=NOT(REGEXMATCH(A2,"^[A-Za-z][A-Za-z0-9_]*$"))`
5. Formatting style: Light red background
6. Done

## Sharing and Permissions

### Share with Team

1. Click "Share" button
2. Add email addresses
3. Choose permission level:
   - **Viewer**: Read-only
   - **Commenter**: Can add comments
   - **Editor**: Can edit data
4. Send notification email

### Link Sharing

1. Click "Share" button
2. Click "Change to anyone with the link"
3. Choose permission level
4. Copy link and share

## Version History

Google Sheets automatically tracks changes:

1. File → Version history → See version history
2. View who made what changes and when
3. Restore previous versions if needed
4. Name versions for important milestones

## Collaboration Tips

1. **Use comments** for questions: Right-click cell → Comment
2. **Assign tasks**: Use @ mentions in comments
3. **Protect ranges**: Prevent accidental edits to headers
4. **Use filters**: View → Create filter
5. **Sort data**: Data → Sort range

## Template Checklist

Before sharing a template:

- [ ] Column headers match exactly (case-sensitive)
- [ ] At least one example row of data
- [ ] Data validation rules for enum fields
- [ ] Conditional formatting for common errors
- [ ] Header row is frozen
- [ ] Number columns formatted as numbers
- [ ] Instructions sheet included (optional)

## Resources

- [Google Sheets Function List](https://support.google.com/docs/table/25273)
- [Data Validation Guide](https://support.google.com/docs/answer/139705)
- [Conditional Formatting Guide](https://support.google.com/docs/answer/78413)

## Next Steps

1. Create your Google Sheet
2. Set up column headers
3. Add data validation rules
4. Add example data
5. Test export/import cycle
6. Share with team

See [GOOGLE_SHEETS_GUIDE.md](GOOGLE_SHEETS_GUIDE.md) for complete workflow instructions.
