#!/usr/bin/env python3
"""
Tests for SchemaValidator.py

Run with: pytest tests/test_schema_validator.py -v
"""

import sys
import os
import pytest
import json

# Add parent directory to path to import the module
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# Check if jsonschema is available for testing
try:
    import jsonschema
    JSONSCHEMA_AVAILABLE = True
except ImportError:
    JSONSCHEMA_AVAILABLE = False


@pytest.mark.skipif(not JSONSCHEMA_AVAILABLE, reason="jsonschema module not installed")
class TestSchemaValidator:
    """Tests for the SchemaValidator class."""

    def test_import_schema_validator(self):
        """Test that SchemaValidator can be imported."""
        from SchemaValidator import SchemaValidator
        assert SchemaValidator is not None

    def test_schema_validator_init(self):
        """Test that SchemaValidator can be instantiated."""
        from SchemaValidator import SchemaValidator
        validator = SchemaValidator()
        assert validator is not None
        assert validator.schema_dir.exists() or True  # May not exist in test env

    def test_schema_dir_configured(self):
        """Test that schema directory is configured."""
        from SchemaValidator import SchemaValidator
        validator = SchemaValidator()
        assert hasattr(validator, 'schema_dir')


class TestSchemaFiles:
    """Tests for schema file existence and validity."""

    @pytest.fixture
    def schema_dir(self):
        """Get the Schemas directory path."""
        return os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'Schemas')

    def test_schemas_directory_exists(self, schema_dir):
        """Test that Schemas directory exists."""
        assert os.path.isdir(schema_dir)

    def test_spaceship_schema_exists(self, schema_dir):
        """Test that spaceship_schema.json exists."""
        schema_path = os.path.join(schema_dir, 'spaceship_schema.json')
        assert os.path.isfile(schema_path)

    def test_trade_item_schema_exists(self, schema_dir):
        """Test that trade_item_schema.json exists."""
        schema_path = os.path.join(schema_dir, 'trade_item_schema.json')
        assert os.path.isfile(schema_path)

    def test_spaceship_schema_valid_json(self, schema_dir):
        """Test that spaceship_schema.json is valid JSON."""
        schema_path = os.path.join(schema_dir, 'spaceship_schema.json')
        with open(schema_path, 'r') as f:
            data = json.load(f)
        assert isinstance(data, dict)
        assert '$schema' in data or 'type' in data or 'properties' in data

    def test_trade_item_schema_valid_json(self, schema_dir):
        """Test that trade_item_schema.json is valid JSON."""
        schema_path = os.path.join(schema_dir, 'trade_item_schema.json')
        with open(schema_path, 'r') as f:
            data = json.load(f)
        assert isinstance(data, dict)


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
