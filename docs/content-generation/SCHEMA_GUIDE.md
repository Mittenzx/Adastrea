# Schema Definition Guide

Complete guide for creating and using JSON schemas to validate game data in Adastrea.

## Overview

Schemas define the structure, types, and validation rules for game data assets. They ensure data quality and catch errors early in the content creation process.

## Schema Locations

All schema files are stored in the `Schemas/` directory:

- `spaceship_schema.json` - Validates spaceship data
- `trade_item_schema.json` - Validates trade item data
- Additional schemas can be added for other asset types

## Schema Structure

Schemas use [JSON Schema Draft 7](https://json-schema.org/draft-07/schema) format.

### Basic Schema Elements

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "Asset Name Schema",
  "description": "Description of what this schema validates",
  "type": "object",
  "properties": {
    // Define properties here
  },
  "required": ["field1", "field2"]
}
```

### Property Types

#### String Fields

```json
"PropertyName": {
  "type": "string",
  "minLength": 1,
  "maxLength": 100,
  "description": "Description of this property"
}
```

**With Enum (Fixed Values):**

```json
"PropertyName": {
  "type": "string",
  "enum": ["Value1", "Value2", "Value3"],
  "description": "Must be one of the listed values"
}
```

**With Pattern (Regex):**

```json
"PropertyName": {
  "type": "string",
  "pattern": "^[A-Za-z][A-Za-z0-9_]*$",
  "description": "Must start with letter, alphanumeric + underscore"
}
```

#### Number Fields

```json
"PropertyName": {
  "type": "number",
  "minimum": 0,
  "maximum": 100,
  "description": "Floating point number"
}
```

**Integer (Whole Numbers):**

```json
"PropertyName": {
  "type": "integer",
  "minimum": 1,
  "maximum": 10,
  "description": "Whole number only"
}
```

#### Boolean Fields

```json
"PropertyName": {
  "type": "boolean",
  "description": "True or false value"
}
```

#### Array Fields

```json
"PropertyName": {
  "type": "array",
  "items": {
    "type": "string"
  },
  "minItems": 0,
  "maxItems": 10,
  "description": "List of strings"
}
```

#### Nested Objects

```json
"PropertyName": {
  "type": "object",
  "properties": {
    "SubProperty1": {"type": "string"},
    "SubProperty2": {"type": "number"}
  },
  "required": ["SubProperty1"]
}
```

### Validation Rules

#### Required Fields

Fields listed in `required` array must be present:

```json
{
  "type": "object",
  "properties": {
    "Name": {"type": "string"},
    "ID": {"type": "string"},
    "Description": {"type": "string"}
  },
  "required": ["Name", "ID"]
}
```

In this example, `Description` is optional but `Name` and `ID` must be provided.

#### Value Constraints

**Numeric Ranges:**

```json
{
  "type": "number",
  "minimum": 0,           // Inclusive minimum
  "maximum": 100,         // Inclusive maximum
  "exclusiveMinimum": 0,  // Exclusive minimum (> instead of >=)
  "exclusiveMaximum": 100 // Exclusive maximum (< instead of <=)
}
```

**String Lengths:**

```json
{
  "type": "string",
  "minLength": 1,
  "maxLength": 100
}
```

**Array Sizes:**

```json
{
  "type": "array",
  "minItems": 1,
  "maxItems": 10
}
```

## Example: Spaceship Schema

### Basic Info Section

```json
{
  "BasicInfo": {
    "type": "object",
    "required": ["ShipName", "ShipClass", "ShipID"],
    "properties": {
      "ShipName": {
        "type": "string",
        "minLength": 1,
        "maxLength": 100,
        "description": "Display name of the spaceship"
      },
      "ShipClass": {
        "type": "string",
        "minLength": 1,
        "maxLength": 100,
        "description": "Ship class/type"
      },
      "ShipID": {
        "type": "string",
        "pattern": "^[A-Za-z][A-Za-z0-9_]*$",
        "description": "Unique identifier"
      }
    }
  }
}
```

### Stats Section

```json
{
  "CoreStats": {
    "type": "object",
    "required": ["HullStrength", "CargoCapacity"],
    "properties": {
      "HullStrength": {
        "type": "number",
        "minimum": 1,
        "maximum": 100000,
        "description": "Maximum hull strength (health points)"
      },
      "CargoCapacity": {
        "type": "number",
        "minimum": 0,
        "maximum": 50000,
        "description": "Cargo capacity in tons"
      }
    }
  }
}
```

## Custom Validation Rules

Beyond JSON Schema, you can add custom validation in `SchemaValidator.py`.

### Adding Custom Validation

Edit the `apply_custom_validation()` method:

```python
def apply_custom_validation(self, schema_name: str, data: Dict[str, Any], 
                           source_name: str) -> List[str]:
    """Apply custom validation rules beyond JSON Schema"""
    errors = []
    
    if schema_name == "spaceship":
        errors.extend(self._validate_spaceship_custom(data, source_name))
    
    return errors

def _validate_spaceship_custom(self, data: Dict[str, Any], 
                               source_name: str) -> List[str]:
    """Custom validation rules for spaceship data"""
    errors = []
    
    # Example: Validate crew requirements
    if 'CoreStats' in data:
        core = data['CoreStats']
        crew_min = core.get('CrewRequired', 0)
        crew_max = core.get('MaxCrew', 0)
        
        if crew_min > crew_max:
            errors.append(
                f"{source_name}: CrewRequired ({crew_min}) "
                f"cannot be greater than MaxCrew ({crew_max})"
            )
    
    return errors
```

### Common Custom Validations

1. **Cross-field validation** - One field depends on another
2. **Complex range checks** - Ranges that depend on other values
3. **Logical consistency** - Business rules that JSON Schema can't express
4. **Data integrity** - References to other assets exist

## Creating New Schemas

### Step 1: Define Structure

Create a new JSON file in `Schemas/` directory:

```bash
touch Schemas/faction_schema.json
```

### Step 2: Write Schema

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "Faction Data Asset Schema",
  "description": "Schema for faction data in Adastrea",
  "type": "object",
  "properties": {
    "FactionID": {
      "type": "string",
      "pattern": "^[A-Za-z][A-Za-z0-9_]*$",
      "description": "Unique faction identifier"
    },
    "FactionName": {
      "type": "string",
      "minLength": 1,
      "maxLength": 100,
      "description": "Display name"
    },
    "TechLevel": {
      "type": "integer",
      "minimum": 1,
      "maximum": 10,
      "description": "Technology level (1-10)"
    }
  },
  "required": ["FactionID", "FactionName", "TechLevel"]
}
```

### Step 3: Add to AssetConfig

Edit `AssetConfig.py` to add column definitions:

```python
def get_faction_columns() -> List[Dict[str, str]]:
    """Get column definitions for faction data"""
    return [
        {'header': 'Faction ID', 'path': 'FactionID', 'type': 'string'},
        {'header': 'Faction Name', 'path': 'FactionName', 'type': 'string'},
        {'header': 'Tech Level', 'path': 'TechLevel', 'type': 'integer'},
        # Add more columns...
    ]
```

And add to `get_asset_configs()`:

```python
'faction': {
    'schema_name': 'faction',
    'template_dir': 'Assets/FactionTemplates',
    'content_path': '/Game/Factions/DataAssets',
    'id_field': 'FactionID',
    'name_field': 'FactionName',
    'sheet_name': 'Factions',
    'columns': get_faction_columns()
}
```

### Step 4: Test Schema

```bash
# Validate a single file
python SchemaValidator.py --schema faction --data Assets/FactionTemplates/MyFaction.yaml

# Validate directory
python SchemaValidator.py --schema faction --directory Assets/FactionTemplates
```

## Best Practices

### Schema Design

1. **Start Simple** - Begin with required fields, add optional fields later
2. **Be Specific** - Use narrow ranges and specific enums when possible
3. **Document Everything** - Add clear descriptions to all fields
4. **Test Early** - Validate against real data during schema development

### Field Naming

- Use **PascalCase** for property names (e.g., `ShipName`, `TechLevel`)
- Be **descriptive** but **concise**
- Match **C++ property names** from Data Asset classes

### Value Ranges

- Set **realistic limits** based on game balance
- Use **minimum** and **maximum** to prevent data entry errors
- Consider **exclusiveMinimum/Maximum** for ranges like `(0, 1)` instead of `[0, 1]`

### Required vs Optional

- Mark **critical fields** as required (IDs, names, essential stats)
- Make **nice-to-have fields** optional (lore, descriptions)
- Consider **default values** in code for optional fields

## Validation Workflow

### During Development

1. **Create/Edit Data** in Google Sheets or CSV
2. **Export** to YAML files
3. **Validate** against schema:
   ```bash
   python SchemaValidator.py --schema spaceship --directory Assets/SpaceshipTemplates --report validation.txt
   ```
4. **Fix Errors** in spreadsheet
5. **Re-import** and validate again

### Before Committing

Always validate before committing to version control:

```bash
# Validate all asset types
python SchemaValidator.py --schema spaceship --directory Assets/SpaceshipTemplates
python SchemaValidator.py --schema trade_item --directory Assets/TradingTemplates
```

### Continuous Integration

Add validation to CI/CD pipeline:

```yaml
# .github/workflows/validate-data.yml
name: Validate Game Data
on: [push, pull_request]
jobs:
  validate:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install dependencies
        run: pip install jsonschema pyyaml
      - name: Validate spaceships
        run: python SchemaValidator.py --schema spaceship --directory Assets/SpaceshipTemplates
      - name: Validate trade items
        run: python SchemaValidator.py --schema trade_item --directory Assets/TradingTemplates
```

## Troubleshooting

### Common Errors

**Error:** `'Value' is not one of [...]`

**Solution:** Check enum values in schema match exactly (case-sensitive)

**Error:** `X is greater than the maximum of Y`

**Solution:** Either fix the data value or increase the schema maximum

**Error:** `'Field' is a required property`

**Solution:** Add the required field to your data or make it optional in schema

### Debugging

Use verbose validation to see detailed information:

```bash
python SchemaValidator.py --schema spaceship --data file.yaml --verbose
```

View the generated validation report:

```bash
cat validation_report.txt
```

## Advanced Topics

### Conditional Schemas

Use `if/then/else` for conditional validation:

```json
{
  "if": {
    "properties": {
      "ShipClass": {"const": "Fighter"}
    }
  },
  "then": {
    "properties": {
      "WeaponSlots": {"minimum": 2}
    }
  }
}
```

### Schema References

Reuse schema definitions with `$ref`:

```json
{
  "definitions": {
    "Color": {
      "type": "array",
      "items": {"type": "number", "minimum": 0, "maximum": 1},
      "minItems": 4,
      "maxItems": 4
    }
  },
  "properties": {
    "PrimaryColor": {"$ref": "#/definitions/Color"},
    "SecondaryColor": {"$ref": "#/definitions/Color"}
  }
}
```

### Multiple Data Types

Allow multiple types with arrays:

```json
{
  "PropertyName": {
    "type": ["string", "number"],
    "description": "Can be either string or number"
  }
}
```

## Resources

- [JSON Schema Documentation](https://json-schema.org/)
- [Understanding JSON Schema](https://json-schema.org/understanding-json-schema/)
- [JSON Schema Validator](https://www.jsonschemavalidator.net/) - Online testing
- [Schema Store](https://www.schemastore.org/) - Example schemas

## See Also

- [Google Sheets Integration Guide](GOOGLE_SHEETS_GUIDE.md)
- [CSV Import Guide](#) (coming soon)
- [Data Asset Workflow](YAML_IMPORT_GUIDE.md)
