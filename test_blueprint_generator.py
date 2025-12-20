#!/usr/bin/env python3
"""
Test script for BlueprintGenerator fixes
Tests the BlueprintGenerator class without requiring Unreal Engine
"""

import sys
import os
from pathlib import Path

# Add the Content/Python directory to the path
content_python_path = Path(__file__).parent / "Content" / "Python"
sys.path.insert(0, str(content_python_path))

def test_blueprint_generator_import():
    """Test that BlueprintGenerator can be imported without syntax errors"""
    try:
        # Try to import the module - this will catch syntax errors
        import BlueprintGenerator
        print("✓ BlueprintGenerator.py imports successfully")

        # Check if the class exists
        if hasattr(BlueprintGenerator, 'BlueprintGenerator'):
            print("✓ BlueprintGenerator class found")
        else:
            print("✗ BlueprintGenerator class not found")
            return False

        # Check if key methods exist
        methods_to_check = [
            '_create_game_mode_blueprint',
            '_create_player_controller_blueprint',
            '_create_player_ship_blueprint',
            '_create_ai_controller_blueprint',
            '_create_faction_ai_blueprint',
            '_create_space_station_blueprint',
            '_create_station_module_blueprint',
            '_create_advanced_ship_blueprints',
            '_create_weapon_system_blueprints',
            '_create_trading_system_blueprints',
            '_create_quest_system_blueprints',
            '_create_ui_system_blueprints'
        ]

        for method_name in methods_to_check:
            if hasattr(BlueprintGenerator.BlueprintGenerator, method_name):
                print(f"✓ Method {method_name} found")
            else:
                print(f"✗ Method {method_name} not found")
                return False

        print("\n✓ All BlueprintGenerator methods are present and importable")
        return True

    except SyntaxError as e:
        print(f"✗ Syntax error in BlueprintGenerator.py: {e}")
        return False
    except ImportError as e:
        print(f"✗ Import error: {e}")
        return False
    except Exception as e:
        print(f"✗ Unexpected error: {e}")
        return False

def test_method_signatures():
    """Test that methods have correct signatures"""
    try:
        import BlueprintGenerator
        import inspect

        # Check a few key methods for correct signatures
        methods_to_check = [
            ('_create_advanced_ship_blueprints', 1),  # self parameter
            ('_create_weapon_system_blueprints', 1),
            ('_create_trading_system_blueprints', 1),
            ('_create_quest_system_blueprints', 1),
            ('_create_ui_system_blueprints', 1)
        ]

        for method_name, expected_params in methods_to_check:
            method = getattr(BlueprintGenerator.BlueprintGenerator, method_name)
            sig = inspect.signature(method)
            param_count = len(sig.parameters)

            if param_count == expected_params:
                print(f"✓ {method_name} has correct signature ({param_count} parameters)")
            else:
                print(f"✗ {method_name} has {param_count} parameters, expected {expected_params}")
                return False

        return True

    except Exception as e:
        print(f"✗ Error checking method signatures: {e}")
        return False

if __name__ == "__main__":
    print("Testing BlueprintGenerator fixes...")
    print("=" * 50)

    success = True

    print("\n1. Testing import...")
    if not test_blueprint_generator_import():
        success = False

    print("\n2. Testing method signatures...")
    if not test_method_signatures():
        success = False

    print("\n" + "=" * 50)
    if success:
        print("✓ All tests passed! BlueprintGenerator is ready for Unreal Engine.")
        print("\nNext steps:")
        print("- Run the content generation in Unreal Editor")
        print("- Check the output log for successful Blueprint creation")
        print("- Verify assets appear in the Content Browser")
    else:
        print("✗ Some tests failed. Please check the errors above.")

    sys.exit(0 if success else 1)