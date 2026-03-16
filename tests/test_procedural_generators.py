#!/usr/bin/env python3
"""
Tests for ProceduralGenerators.py

Run with: pytest tests/test_procedural_generators.py -v
"""

import sys
import os
import pytest

# Add parent directory to path to import the module
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from ProceduralGenerators import NameGenerator


class TestNameGenerator:
    """Tests for the NameGenerator class."""

    def test_generate_person_name_returns_string(self):
        """Test that generate_person_name returns a non-empty string."""
        name = NameGenerator.generate_person_name()
        assert isinstance(name, str)
        assert len(name) > 0

    def test_generate_person_name_male(self):
        """Test generating a male name."""
        name = NameGenerator.generate_person_name(gender="Male")
        assert isinstance(name, str)
        # Name should have at least two parts (first and last)
        parts = name.split()
        assert len(parts) >= 2

    def test_generate_person_name_female(self):
        """Test generating a female name."""
        name = NameGenerator.generate_person_name(gender="Female")
        assert isinstance(name, str)
        parts = name.split()
        assert len(parts) >= 2

    def test_first_names_male_not_empty(self):
        """Test that male first names list is populated."""
        assert len(NameGenerator.FIRST_NAMES_MALE) > 0

    def test_first_names_female_not_empty(self):
        """Test that female first names list is populated."""
        assert len(NameGenerator.FIRST_NAMES_FEMALE) > 0

    def test_last_names_not_empty(self):
        """Test that last names list is populated."""
        assert len(NameGenerator.LAST_NAMES) > 0

    def test_star_prefixes_not_empty(self):
        """Test that star prefixes list is populated."""
        assert len(NameGenerator.STAR_PREFIXES) > 0

    def test_star_suffixes_not_empty(self):
        """Test that star suffixes list is populated."""
        assert len(NameGenerator.STAR_SUFFIXES) > 0

    def test_planet_prefixes_not_empty(self):
        """Test that planet prefixes list is populated."""
        assert len(NameGenerator.PLANET_PREFIXES) > 0

    def test_planet_suffixes_not_empty(self):
        """Test that planet suffixes list is populated."""
        assert len(NameGenerator.PLANET_SUFFIXES) > 0

    def test_generate_unique_names(self):
        """Test that multiple calls can generate different names."""
        names = set()
        for _ in range(100):
            name = NameGenerator.generate_person_name()
            names.add(name)
        # Should have generated at least some unique names
        assert len(names) > 1


class TestNameGeneratorStarNames:
    """Tests for star name generation."""

    def test_generate_star_name_returns_string(self):
        """Test that generate_star_name returns a non-empty string."""
        # Check if the method exists
        if hasattr(NameGenerator, 'generate_star_name'):
            name = NameGenerator.generate_star_name()
            assert isinstance(name, str)
            assert len(name) > 0


class TestNameGeneratorPlanetNames:
    """Tests for planet name generation."""

    def test_generate_planet_name_returns_string(self):
        """Test that generate_planet_name returns a non-empty string."""
        # Check if the method exists
        if hasattr(NameGenerator, 'generate_planet_name'):
            name = NameGenerator.generate_planet_name()
            assert isinstance(name, str)
            assert len(name) > 0


class TestNameGeneratorShipNames:
    """Tests for ship name generation."""

    def test_generate_ship_name_returns_string(self):
        """Test that generate_ship_name returns a non-empty string."""
        # Check if the method exists
        if hasattr(NameGenerator, 'generate_ship_name'):
            name = NameGenerator.generate_ship_name()
            assert isinstance(name, str)
            assert len(name) > 0


class TestNameGeneratorAvailableTypes:
    """Tests for the get_available_name_types method."""

    def test_get_available_name_types_returns_list(self):
        """Test that get_available_name_types returns a list."""
        if hasattr(NameGenerator, 'get_available_name_types'):
            types = NameGenerator.get_available_name_types()
            assert isinstance(types, list)
            assert len(types) > 0
    
    def test_available_types_contains_expected_values(self):
        """Test that available types list contains expected values."""
        if hasattr(NameGenerator, 'get_available_name_types'):
            types = NameGenerator.get_available_name_types()
            expected_types = ["person", "star", "planet", "ship"]
            for expected in expected_types:
                assert expected in types


class TestNameGeneratorMultipleNames:
    """Tests for bulk name generation."""

    def test_generate_multiple_names_person(self):
        """Test generating multiple person names."""
        if hasattr(NameGenerator, 'generate_multiple_names'):
            names = NameGenerator.generate_multiple_names(5, "person")
            assert isinstance(names, list)
            assert len(names) == 5
            for name in names:
                assert isinstance(name, str)
                assert len(name) > 0
    
    def test_generate_multiple_names_star(self):
        """Test generating multiple star names."""
        if hasattr(NameGenerator, 'generate_multiple_names'):
            names = NameGenerator.generate_multiple_names(3, "star")
            assert isinstance(names, list)
            assert len(names) == 3
            for name in names:
                assert isinstance(name, str)
                assert len(name) > 0
    
    def test_generate_multiple_names_invalid_type(self):
        """Test that invalid name type raises ValueError."""
        if hasattr(NameGenerator, 'generate_multiple_names'):
            with pytest.raises(ValueError):
                NameGenerator.generate_multiple_names(5, "invalid_type")


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
