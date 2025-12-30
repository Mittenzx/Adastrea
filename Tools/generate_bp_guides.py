#!/usr/bin/env python3
"""
Generate comprehensive SVG Blueprint documentation for Adastrea project
Creates visual guides for all essential blueprints needed for MVP
"""

import os
import sys
import xml.etree.ElementTree as ET

# Add parent directory to path to import from generate_blueprint_images
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from generate_blueprint_images import (
    COLORS, create_svg_base, add_node_box, add_pin, 
    add_connection_wire, save_svg
)

def generate_gamemode_setup():
    """Generate BP_SpaceGameMode setup diagram"""
    svg = create_svg_base(900, 500)
    
    # Title
    title = ET.SubElement(svg, 'text', {
        'x': '450',
        'y': '30',
        'fill': COLORS['text'],
        'font-family': 'Arial, sans-serif',
        'font-size': '18',
        'font-weight': 'bold',
        'text-anchor': 'middle'
    })
    title.text = 'BP_SpaceGameMode - BeginPlay Setup'
    
    # Event BeginPlay
    body_y1 = add_node_box(svg, 50, 60, 280, 80, COLORS['event'], 'EVENT: BeginPlay')
    add_pin(svg, 330, body_y1 + 25, 'exec', '', is_input=False, is_exec=True)
    
    # Initialize Economy System
    body_y2 = add_node_box(svg, 50, 180, 300, 120, COLORS['function'], 'Initialize Economy')
    add_pin(svg, 50, body_y2 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 350, body_y2 + 20, 'exec', '', is_input=False, is_exec=True)
    
    desc_text = ET.SubElement(svg, 'text', {
        'x': '65',
        'y': str(body_y2 + 70),
        'fill': '#AAAAAA',
        'font-family': 'Arial, sans-serif',
        'font-size': '11',
        'font-style': 'italic'
    })
    desc_text.text = 'Sets up economy manager and market prices'
    
    # Initialize Faction System
    body_y3 = add_node_box(svg, 50, 340, 300, 120, COLORS['function'], 'Initialize Factions')
    add_pin(svg, 50, body_y3 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 350, body_y3 + 20, 'exec', '', is_input=False, is_exec=True)
    
    desc_text2 = ET.SubElement(svg, 'text', {
        'x': '65',
        'y': str(body_y3 + 70),
        'fill': '#AAAAAA',
        'font-family': 'Arial, sans-serif',
        'font-size': '11',
        'font-style': 'italic'
    })
    desc_text2.text = 'Loads faction data and relationships'
    
    # Spawn Player Ship
    body_y4 = add_node_box(svg, 450, 180, 400, 140, COLORS['function'], 'Spawn Player Ship')
    add_pin(svg, 450, body_y4 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 450, body_y4 + 40, 'object', 'Ship Data Asset', is_input=True)
    add_pin(svg, 450, body_y4 + 60, 'vector', 'Spawn Location', is_input=True)
    add_pin(svg, 850, body_y4 + 20, 'exec', '', is_input=False, is_exec=True)
    add_pin(svg, 850, body_y4 + 40, 'object', 'Spawned Ship', is_input=False)
    
    # Connection wires
    add_connection_wire(svg, 330, body_y1 + 25, 50, body_y2 + 20)
    add_connection_wire(svg, 350, body_y2 + 20, 50, body_y3 + 20)
    add_connection_wire(svg, 350, body_y3 + 20, 450, body_y4 + 20)
    
    return svg

def generate_player_ship_beginplay():
    """Generate BP_PlayerShip BeginPlay setup"""
    svg = create_svg_base(900, 550)
    
    # Title
    title = ET.SubElement(svg, 'text', {
        'x': '450',
        'y': '30',
        'fill': COLORS['text'],
        'font-family': 'Arial, sans-serif',
        'font-size': '18',
        'font-weight': 'bold',
        'text-anchor': 'middle'
    })
    title.text = 'BP_PlayerShip - BeginPlay Initialization'
    
    # Event BeginPlay
    body_y1 = add_node_box(svg, 50, 60, 280, 80, COLORS['event'], 'EVENT: BeginPlay')
    add_pin(svg, 330, body_y1 + 25, 'exec', '', is_input=False, is_exec=True)
    
    # Load Ship Data
    body_y2 = add_node_box(svg, 50, 180, 300, 120, COLORS['function'], 'Load Ship Data')
    add_pin(svg, 50, body_y2 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 50, body_y2 + 40, 'object', 'Ship Data Asset', is_input=True)
    add_pin(svg, 350, body_y2 + 20, 'exec', '', is_input=False, is_exec=True)
    
    # Initialize Combat
    body_y3 = add_node_box(svg, 50, 340, 300, 100, COLORS['function'], 'Initialize Combat')
    add_pin(svg, 50, body_y3 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 350, body_y3 + 20, 'exec', '', is_input=False, is_exec=True)
    
    # Setup Input
    body_y4 = add_node_box(svg, 450, 180, 400, 140, COLORS['function'], 'Setup Enhanced Input')
    add_pin(svg, 450, body_y4 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 450, body_y4 + 40, 'object', 'Input Mapping Context', is_input=True)
    add_pin(svg, 850, body_y4 + 20, 'exec', '', is_input=False, is_exec=True)
    
    desc_text = ET.SubElement(svg, 'text', {
        'x': '465',
        'y': str(body_y4 + 100),
        'fill': '#AAAAAA',
        'font-family': 'Arial, sans-serif',
        'font-size': '11',
        'font-style': 'italic'
    })
    desc_text.text = 'Binds player input actions to ship controls'
    
    # Create HUD
    body_y5 = add_node_box(svg, 450, 360, 400, 140, COLORS['function'], 'Create HUD Widget')
    add_pin(svg, 450, body_y5 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 450, body_y5 + 40, 'object', 'Widget Class', is_input=True)
    add_pin(svg, 850, body_y5 + 20, 'exec', '', is_input=False, is_exec=True)
    add_pin(svg, 850, body_y5 + 40, 'object', 'Created Widget', is_input=False)
    
    # Connection wires
    add_connection_wire(svg, 330, body_y1 + 25, 50, body_y2 + 20)
    add_connection_wire(svg, 350, body_y2 + 20, 50, body_y3 + 20)
    add_connection_wire(svg, 350, body_y3 + 20, 450, body_y4 + 20)
    add_connection_wire(svg, 850, body_y4 + 20, 450, body_y5 + 20)
    
    return svg

def generate_trading_ui_open():
    """Generate WBP_TradingUI - Open Market flow"""
    svg = create_svg_base(900, 650)
    
    # Title
    title = ET.SubElement(svg, 'text', {
        'x': '450',
        'y': '30',
        'fill': COLORS['text'],
        'font-family': 'Arial, sans-serif',
        'font-size': '18',
        'font-weight': 'bold',
        'text-anchor': 'middle'
    })
    title.text = 'WBP_TradingUI - Open Market Flow'
    
    # Event Open Market
    body_y1 = add_node_box(svg, 50, 60, 320, 100, COLORS['event'], 'EVENT: On Market Opened')
    add_pin(svg, 370, body_y1 + 25, 'exec', '', is_input=False, is_exec=True)
    add_pin(svg, 370, body_y1 + 45, 'object', 'Market Data', is_input=False)
    
    # Get Available Items
    body_y2 = add_node_box(svg, 50, 200, 350, 120, COLORS['function'], 'Get Available Items')
    add_pin(svg, 50, body_y2 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 50, body_y2 + 40, 'object', 'Market Data', is_input=True)
    add_pin(svg, 400, body_y2 + 20, 'exec', '', is_input=False, is_exec=True)
    add_pin(svg, 400, body_y2 + 40, 'object', 'Item Array', is_input=False)
    
    # ForEach Loop
    body_y3 = add_node_box(svg, 50, 360, 350, 180, COLORS['flow_control'], 'ForEach Loop')
    add_pin(svg, 50, body_y3 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 50, body_y3 + 40, 'object', 'Array', is_input=True)
    add_pin(svg, 400, body_y3 + 20, 'exec', 'Loop Body', is_input=False, is_exec=True)
    add_pin(svg, 400, body_y3 + 40, 'exec', 'Completed', is_input=False, is_exec=True)
    add_pin(svg, 400, body_y3 + 60, 'object', 'Array Element', is_input=False)
    add_pin(svg, 400, body_y3 + 80, 'int', 'Array Index', is_input=False)
    
    # Create Item Widget
    body_y4 = add_node_box(svg, 480, 200, 370, 140, COLORS['function'], 'Create Trade Item Widget')
    add_pin(svg, 480, body_y4 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 480, body_y4 + 40, 'object', 'Widget Class', is_input=True)
    add_pin(svg, 480, body_y4 + 60, 'object', 'Item Data', is_input=True)
    add_pin(svg, 850, body_y4 + 20, 'exec', '', is_input=False, is_exec=True)
    add_pin(svg, 850, body_y4 + 40, 'object', 'Widget', is_input=False)
    
    # Add to Scroll Box
    body_y5 = add_node_box(svg, 480, 380, 370, 120, COLORS['function'], 'Add Child to ScrollBox')
    add_pin(svg, 480, body_y5 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 480, body_y5 + 40, 'object', 'Parent ScrollBox', is_input=True)
    add_pin(svg, 480, body_y5 + 60, 'object', 'Child Widget', is_input=True)
    add_pin(svg, 850, body_y5 + 20, 'exec', '', is_input=False, is_exec=True)
    
    # Connection wires
    add_connection_wire(svg, 370, body_y1 + 25, 50, body_y2 + 20)
    add_connection_wire(svg, 400, body_y2 + 20, 50, body_y3 + 20)
    add_connection_wire(svg, 400, body_y3 + 20, 480, body_y4 + 20)
    add_connection_wire(svg, 850, body_y4 + 20, 480, body_y5 + 20)
    # Loop back wire
    add_connection_wire(svg, 850, body_y5 + 20, 400, body_y3 + 20)
    
    return svg

def generate_buy_validation_flow():
    """Generate buy item validation complete flow"""
    svg = create_svg_base(900, 800)
    
    # Title
    title = ET.SubElement(svg, 'text', {
        'x': '450',
        'y': '30',
        'fill': COLORS['text'],
        'font-family': 'Arial, sans-serif',
        'font-size': '18',
        'font-weight': 'bold',
        'text-anchor': 'middle'
    })
    title.text = 'WBP_TradingUI - Buy Item Validation Flow'
    
    # Event OnBuyButtonClicked
    body_y1 = add_node_box(svg, 200, 60, 500, 100, COLORS['event'], 'EVENT: On Buy Button Clicked')
    add_pin(svg, 700, body_y1 + 25, 'exec', '', is_input=False, is_exec=True)
    add_pin(svg, 700, body_y1 + 45, 'object', 'Item', is_input=False)
    add_pin(svg, 700, body_y1 + 65, 'int', 'Quantity', is_input=False)
    
    # Get Item Price
    body_y2 = add_node_box(svg, 150, 200, 600, 140, COLORS['function'], 'Calculate Total Cost')
    add_pin(svg, 150, body_y2 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 150, body_y2 + 40, 'object', 'Item', is_input=True)
    add_pin(svg, 150, body_y2 + 60, 'int', 'Quantity', is_input=True)
    add_pin(svg, 750, body_y2 + 20, 'exec', '', is_input=False, is_exec=True)
    add_pin(svg, 750, body_y2 + 40, 'int', 'Total Price', is_input=False)
    
    # Branch: Can Afford?
    body_y3 = add_node_box(svg, 200, 380, 500, 140, COLORS['branch'], 'BRANCH: Can Afford?')
    add_pin(svg, 200, body_y3 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 200, body_y3 + 40, 'bool', 'Credits >= Total', is_input=True)
    add_pin(svg, 700, body_y3 + 20, 'exec', 'True', is_input=False, is_exec=True)
    add_pin(svg, 700, body_y3 + 60, 'exec', 'False', is_input=False, is_exec=True)
    
    # Branch: Has Cargo Space?
    body_y4 = add_node_box(svg, 200, 560, 500, 140, COLORS['branch'], 'BRANCH: Has Cargo Space?')
    add_pin(svg, 200, body_y4 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 200, body_y4 + 40, 'bool', 'Space Available', is_input=True)
    add_pin(svg, 700, body_y4 + 20, 'exec', 'True', is_input=False, is_exec=True)
    add_pin(svg, 700, body_y4 + 60, 'exec', 'False', is_input=False, is_exec=True)
    
    # Execute Purchase (Success)
    body_y5 = add_node_box(svg, 50, 740, 300, 80, COLORS['function'], 'Execute Purchase')
    add_pin(svg, 50, body_y5 + 25, 'exec', 'Exec', is_input=True, is_exec=True)
    
    # Show Error (Failure)
    body_y6 = add_node_box(svg, 550, 740, 300, 80, COLORS['function'], 'Show Error Message')
    add_pin(svg, 550, body_y6 + 25, 'exec', 'Exec', is_input=True, is_exec=True)
    
    # Connection wires
    add_connection_wire(svg, 700, body_y1 + 25, 150, body_y2 + 20)
    add_connection_wire(svg, 750, body_y2 + 20, 200, body_y3 + 20)
    add_connection_wire(svg, 700, body_y3 + 20, 200, body_y4 + 20)
    add_connection_wire(svg, 700, body_y3 + 60, 550, body_y6 + 25)  # Can't afford
    add_connection_wire(svg, 700, body_y4 + 20, 200, body_y5 + 25)  # Success
    add_connection_wire(svg, 700, body_y4 + 60, 700, body_y6 + 25)  # No cargo space
    
    return svg

def generate_station_module_attachment():
    """Generate space station module attachment flow"""
    svg = create_svg_base(900, 550)
    
    # Title
    title = ET.SubElement(svg, 'text', {
        'x': '450',
        'y': '30',
        'fill': COLORS['text'],
        'font-family': 'Arial, sans-serif',
        'font-size': '18',
        'font-weight': 'bold',
        'text-anchor': 'middle'
    })
    title.text = 'BP_SpaceStation - Attach Module Flow'
    
    # Event Attach Module
    body_y1 = add_node_box(svg, 50, 60, 350, 120, COLORS['event'], 'EVENT: On Module Attached')
    add_pin(svg, 400, body_y1 + 25, 'exec', '', is_input=False, is_exec=True)
    add_pin(svg, 400, body_y1 + 45, 'object', 'Module Actor', is_input=False)
    add_pin(svg, 400, body_y1 + 65, 'int', 'Attachment Point', is_input=False)
    
    # Validate Attachment
    body_y2 = add_node_box(svg, 50, 220, 380, 140, COLORS['function'], 'Validate Module Attachment')
    add_pin(svg, 50, body_y2 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 50, body_y2 + 40, 'object', 'Module', is_input=True)
    add_pin(svg, 50, body_y2 + 60, 'int', 'Point Index', is_input=True)
    add_pin(svg, 430, body_y2 + 20, 'exec', '', is_input=False, is_exec=True)
    add_pin(svg, 430, body_y2 + 40, 'bool', 'Is Valid', is_input=False)
    
    # Branch: Is Valid?
    body_y3 = add_node_box(svg, 80, 400, 320, 120, COLORS['branch'], 'BRANCH: Is Valid?')
    add_pin(svg, 80, body_y3 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 80, body_y3 + 40, 'bool', 'Condition', is_input=True)
    add_pin(svg, 400, body_y3 + 20, 'exec', 'True', is_input=False, is_exec=True)
    add_pin(svg, 400, body_y3 + 60, 'exec', 'False', is_input=False, is_exec=True)
    
    # Attach to Station (Success)
    body_y4 = add_node_box(svg, 480, 220, 370, 140, COLORS['function'], 'Attach Module to Station')
    add_pin(svg, 480, body_y4 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 480, body_y4 + 40, 'object', 'Module', is_input=True)
    add_pin(svg, 480, body_y4 + 60, 'object', 'Attachment Point', is_input=True)
    add_pin(svg, 850, body_y4 + 20, 'exec', '', is_input=False, is_exec=True)
    
    # Update Station Stats
    body_y5 = add_node_box(svg, 480, 400, 370, 100, COLORS['function'], 'Update Station Stats')
    add_pin(svg, 480, body_y5 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 850, body_y5 + 20, 'exec', '', is_input=False, is_exec=True)
    
    # Connection wires
    add_connection_wire(svg, 400, body_y1 + 25, 50, body_y2 + 20)
    add_connection_wire(svg, 430, body_y2 + 20, 80, body_y3 + 20)
    add_connection_wire(svg, 400, body_y3 + 20, 480, body_y4 + 20)
    add_connection_wire(svg, 850, body_y4 + 20, 480, body_y5 + 20)
    
    return svg

def generate_npc_ai_decision():
    """Generate NPC AI decision flow"""
    svg = create_svg_base(900, 700)
    
    # Title
    title = ET.SubElement(svg, 'text', {
        'x': '450',
        'y': '30',
        'fill': COLORS['text'],
        'font-family': 'Arial, sans-serif',
        'font-size': '18',
        'font-weight': 'bold',
        'text-anchor': 'middle'
    })
    title.text = 'BP_Ship_NPC_Trader - AI Decision Flow'
    
    # Event AI Tick
    body_y1 = add_node_box(svg, 50, 60, 300, 80, COLORS['event'], 'EVENT: AI Update Tick')
    add_pin(svg, 350, body_y1 + 25, 'exec', '', is_input=False, is_exec=True)
    
    # Get Current State
    body_y2 = add_node_box(svg, 50, 180, 320, 120, COLORS['function'], 'Get AI State')
    add_pin(svg, 50, body_y2 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 370, body_y2 + 20, 'exec', '', is_input=False, is_exec=True)
    add_pin(svg, 370, body_y2 + 40, 'object', 'Current State', is_input=False)
    
    # Switch on State
    body_y3 = add_node_box(svg, 50, 340, 320, 180, COLORS['flow_control'], 'SWITCH: State')
    add_pin(svg, 50, body_y3 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 50, body_y3 + 40, 'object', 'Selection', is_input=True)
    add_pin(svg, 370, body_y3 + 20, 'exec', 'Trading', is_input=False, is_exec=True)
    add_pin(svg, 370, body_y3 + 50, 'exec', 'Traveling', is_input=False, is_exec=True)
    add_pin(svg, 370, body_y3 + 80, 'exec', 'Idle', is_input=False, is_exec=True)
    add_pin(svg, 370, body_y3 + 110, 'exec', 'Default', is_input=False, is_exec=True)
    
    # Execute Trade Behavior
    body_y4 = add_node_box(svg, 460, 180, 380, 100, COLORS['function'], 'Execute Trade Behavior')
    add_pin(svg, 460, body_y4 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 840, body_y4 + 20, 'exec', '', is_input=False, is_exec=True)
    
    # Execute Travel Behavior
    body_y5 = add_node_box(svg, 460, 320, 380, 100, COLORS['function'], 'Execute Travel Behavior')
    add_pin(svg, 460, body_y5 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 840, body_y5 + 20, 'exec', '', is_input=False, is_exec=True)
    
    # Execute Idle Behavior
    body_y6 = add_node_box(svg, 460, 460, 380, 100, COLORS['function'], 'Execute Idle Behavior')
    add_pin(svg, 460, body_y6 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 840, body_y6 + 20, 'exec', '', is_input=False, is_exec=True)
    
    # Connection wires
    add_connection_wire(svg, 350, body_y1 + 25, 50, body_y2 + 20)
    add_connection_wire(svg, 370, body_y2 + 20, 50, body_y3 + 20)
    add_connection_wire(svg, 370, body_y3 + 20, 460, body_y4 + 20)
    add_connection_wire(svg, 370, body_y3 + 50, 460, body_y5 + 20)
    add_connection_wire(svg, 370, body_y3 + 80, 460, body_y6 + 20)
    
    return svg

def main():
    """Generate all Blueprint guide diagrams"""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    # Use official blueprint documentation standard location
    output_dir = os.path.join(project_root, 'docs', 'reference', 'images', 'blueprints')
    os.makedirs(output_dir, exist_ok=True)
    
    print("Generating comprehensive Blueprint guides for Adastrea MVP...")
    print(f"Output directory: {output_dir}\n")
    
    # Generate all blueprint diagrams
    diagrams = [
        ('gamemode_setup.svg', generate_gamemode_setup, 'BP_SpaceGameMode Setup'),
        ('player_ship_beginplay.svg', generate_player_ship_beginplay, 'BP_PlayerShip BeginPlay'),
        ('trading_ui_open.svg', generate_trading_ui_open, 'WBP_TradingUI Open Market'),
        ('buy_validation_flow.svg', generate_buy_validation_flow, 'Buy Item Validation'),
        ('station_module_attachment.svg', generate_station_module_attachment, 'Station Module Attachment'),
        ('npc_ai_decision.svg', generate_npc_ai_decision, 'NPC AI Decision Flow'),
    ]
    
    for filename, generator, description in diagrams:
        filepath = os.path.join(output_dir, filename)
        save_svg(generator(), filepath)
        print(f"  ✓ {description}")
    
    print(f"\n✅ Generated {len(diagrams)} Blueprint guide diagrams!")
    print(f"📁 Location: {output_dir}/")
    print("\nNext steps:")
    print("1. Review generated SVG files")
    print("2. Create markdown documentation referencing these images")
    print("3. Add to docs/BP/ folder")

if __name__ == '__main__':
    main()
