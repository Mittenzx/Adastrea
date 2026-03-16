# Procedural Generators Documentation

## Overview

The `ProceduralGenerators.py` module provides procedural content generation utilities for the Adastrea game universe. This module enables dynamic generation of names, locations, and other content to create a rich, varied game world.

## Features

- **Character Name Generation**: Generate realistic character names with gender options
- **Star System Names**: Create scientifically-inspired star system names
- **Planet Names**: Generate descriptive planet names
- **Ship Names**: Create memorable ship names for space vessels
- **Extensible Design**: Easy to add new generation categories

## Usage

### Basic Import

```python
from ProceduralGenerators import NameGenerator
```

### Generating Character Names

```python
# Generate a random character name
name = NameGenerator.generate_person_name()
print(f"Character: {name}")

# Generate a male character name
male_name = NameGenerator.generate_person_name(gender="Male")
print(f"Male Character: {male_name}")

# Generate a female character name  
female_name = NameGenerator.generate_person_name(gender="Female")
print(f"Female Character: {female_name}")
```

### Generating Star System Names

```python
# Generate a star system name
star_name = NameGenerator.generate_star_name()
print(f"Star System: {star_name}")
```

### Generating Planet Names

```python
# Generate a planet name
planet_name = NameGenerator.generate_planet_name()
print(f"Planet: {planet_name}")
```

### Generating Ship Names

```python
# Generate a ship name
ship_name = NameGenerator.generate_ship_name()
print(f"Ship: {ship_name}")
```

### Bulk Name Generation

```python
# Generate multiple names at once
people = NameGenerator.generate_multiple_names(5, "person")
print(f"5 People: {people}")

stars = NameGenerator.generate_multiple_names(3, "star")
print(f"3 Stars: {stars}")

planets = NameGenerator.generate_multiple_names(name_type="planet")
print(f"10 Planets (default): {planets}")
```

## API Reference

### `NameGenerator` Class

#### Class Attributes

- `FIRST_NAMES_MALE`: List of male first names
- `FIRST_NAMES_FEMALE`: List of female first names  
- `LAST_NAMES`: List of common last names
- `STAR_PREFIXES`: Greek letter prefixes for star names
- `STAR_SUFFIXES`: Constellation-based suffixes for star names
- `PLANET_PREFIXES`: Descriptive prefixes for planet names
- `PLANET_SUFFIXES`: Numbering/classification suffixes for planets

#### Methods

##### `generate_person_name(gender=None)`

Generates a procedural person name.

**Parameters:**
- `gender` (str, optional): Gender specification ("Male" or "Female")

**Returns:**
- `str`: Generated name string

**Example:**
```python
name = NameGenerator.generate_person_name()  # Random gender
male_name = NameGenerator.generate_person_name("Male")
female_name = NameGenerator.generate_person_name("Female")
```

##### `generate_star_name()`

Generates a procedural star system name.

**Returns:**
- `str`: Generated star name string

**Example:**
```python
star = NameGenerator.generate_star_name()  # "Alpha Centauri"
```

##### `generate_planet_name()`

Generates a procedural planet name.

**Returns:**
- `str`: Generated planet name string

**Example:**
```python
planet = NameGenerator.generate_planet_name()  # "New Desert Prime"
```

##### `generate_ship_name()`

Generates a procedural ship name.

**Returns:**
- `str`: Generated ship name string

**Example:**
```python
ship = NameGenerator.generate_ship_name()  # "USS Voyager 42"
```

##### `get_available_name_types()`

Returns a list of available name types for generation.

**Returns:**
- `List[str]`: List of valid name type strings

**Example:**
```python
available_types = NameGenerator.get_available_name_types()
print(f"Available name types: {available_types}")
# Output: ['person', 'star', 'planet', 'ship']
```

##### `generate_multiple_names(count=10, name_type="person")`

Generates multiple names of the specified type.

**Parameters:**
- `count` (int, optional): Number of names to generate (default: 10)
- `name_type` (str, optional): Type of names to generate ("person", "star", "planet", "ship") (default: "person")

**Returns:**
- `List[str]`: List of generated name strings

**Raises:**
- `ValueError`: If `name_type` is not one of the available types

**Example:**
```python
# Get available types first
available_types = NameGenerator.get_available_name_types()
print(f"Available: {available_types}")

# Generate 5 person names
people = NameGenerator.generate_multiple_names(5, "person")

# Generate 3 star names
stars = NameGenerator.generate_multiple_names(3, "star")

# Generate 10 planet names (default count)
planets = NameGenerator.generate_multiple_names(name_type="planet")
```

## Data Sources

The name lists are inspired by:
- **First Names**: Common English names with international variations
- **Last Names**: Most common surnames in English-speaking countries
- **Star Names**: Greek alphabet and constellation names
- **Planet Names**: Descriptive terms and Latin numbering
- **Ship Names**: Historical naval naming conventions

## Extending the Module

### Adding New Name Categories

To add a new category of names:

1. Add new class attribute lists:
```python
NEW_CATEGORY_PREFIXES = ["Prefix1", "Prefix2", ...]
NEW_CATEGORY_SUFFIXES = ["Suffix1", "Suffix2", ...]
```

2. Add a new generation method:
```python
@classmethod
def generate_new_category_name(cls):
    prefix = random.choice(cls.NEW_CATEGORY_PREFIXES)
    suffix = random.choice(cls.NEW_CATEGORY_SUFFIXES)
    return f"{prefix} {suffix}"
```

### Customizing Name Lists

You can customize any of the name lists by modifying the class attributes:

```python
# Add custom names to existing lists
NameGenerator.FIRST_NAMES_MALE.extend(["CustomMale1", "CustomMale2"])
NameGenerator.LAST_NAMES.extend(["CustomLastName1", "CustomLastName2"])
```

## Testing

The module includes comprehensive tests in `tests/test_procedural_generators.py`. Run tests with:

```bash
python -m pytest tests/test_procedural_generators.py -v
```

Test coverage includes:
- Basic functionality of all generation methods
- Gender-specific name generation
- List population verification
- Uniqueness of generated names

## Integration with Unreal Engine

This Python module can be integrated with Unreal Engine via:

1. **Python Scripting**: Direct Python execution within UE Editor
2. **MCP Server**: Expose generators as MCP tools for AI agents
3. **Blueprint Nodes**: Create custom Blueprint nodes that call Python functions

Example UE integration:
```python
# In UE Python console
import sys
sys.path.append(r'C:\Path\To\Adastrea')
from ProceduralGenerators import NameGenerator

# Generate names for UE actors
character_name = NameGenerator.generate_person_name()
star_system_name = NameGenerator.generate_star_name()
```

## Performance Considerations

- **Deterministic Randomness**: Uses Python's `random` module
- **Memory Usage**: All name lists are stored in memory (minimal footprint)
- **Speed**: Generation is O(1) for single names, O(n) for bulk generation

For bulk generation of thousands of names, consider:
```python
import random
from ProceduralGenerators import NameGenerator

# Generate 1000 names efficiently
names = [NameGenerator.generate_person_name() for _ in range(1000)]
```

## License

This module is part of the Adastrea project and is licensed under the same terms as the main project.

## Changelog

### Version 1.2.0 (2026-03-16)
- Added `get_available_name_types()` method for API discoverability
- Enhanced test coverage with 19 comprehensive tests (2 new tests)
- Updated documentation with new method examples
- Improved error messages with available type suggestions

### Version 1.1.0 (2026-03-16)
- Added `generate_multiple_names()` method for bulk name generation
- Enhanced test coverage with 17 comprehensive tests
- Updated documentation with bulk generation examples
- Improved error handling for invalid name types

### Version 1.0.0 (2026-03-16)
- Initial release with character, star, planet, and ship name generation
- Comprehensive test suite
- Full documentation
- Unreal Engine integration examples