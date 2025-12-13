#!/usr/bin/env python3
"""
Adastrea Content Generator - Direct Import Version

Copy and paste this entire script into your Unreal Engine Python console:
"""

import sys
sys.path.insert(0, r'C:\Unreal Projects\Adastrea\Content\Python')
sys.path.insert(0, r'C:\Unreal Projects\Adastrea')

import MasterContentGenerator as mcg

# Make functions globally available
generate_all = mcg.generate_all_content
generate_blueprints = mcg.generate_blueprints
generate_data_assets = mcg.generate_data_assets
generate_input = mcg.generate_input_system
generate_maps = mcg.generate_maps
generate_ui = mcg.generate_ui_widgets

print("✅ Adastrea Content Generator Ready!")
print("Available commands:")
print("  generate_all()         - Generate everything")
print("  generate_blueprints()  - Blueprints only")
print("  generate_data_assets() - Data Assets only")
print("  generate_input()       - Input system only")
print("  generate_maps()        - Maps only")
print("  generate_ui()          - UI widgets only")