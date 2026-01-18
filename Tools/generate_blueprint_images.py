#!/usr/bin/env python3
"""
Generate SVG images for Blueprint node documentation
Creates visual representations of Blueprint nodes matching Unreal Engine's appearance
"""

import os
import xml.etree.ElementTree as ET

# Blueprint node color scheme matching Unreal Engine
COLORS = {
    'event': '#C80000',
    'function': '#1B4F72',
    'pure': '#006400',
    'branch': '#FFFFFF',
    'flow_control': '#FF8C00',
    'variable': '#5DADE2',
    'macro': '#8E44AD',
    'delegate': '#8B0000',
    'exec_pin': '#FFFFFF',
    'bool_pin': '#DC143C',
    'int_pin': '#00FF00',
    'float_pin': '#90EE90',
    'string_pin': '#FF00FF',
    'text_pin': '#FFB6C1',
    'vector_pin': '#FFD700',
    'object_pin': '#00FFFF',
    'struct_pin': '#4682B4',
    'background': '#2D2D2D',
    'text': '#FFFFFF',
    'pin_border': '#000000'
}

def calculate_relative_luminance(hex_color):
    """
    Calculate relative luminance according to WCAG 2.0 standards.
    Returns a value between 0 (darkest) and 1 (lightest).
    
    Formula: L = 0.2126 * R + 0.7152 * G + 0.0722 * B
    where R, G, B are linearized color values
    """
    # Remove '#' if present
    hex_color = hex_color.lstrip('#')
    
    # Convert hex to RGB
    r, g, b = int(hex_color[0:2], 16), int(hex_color[2:4], 16), int(hex_color[4:6], 16)
    
    # Normalize to 0-1 range
    r, g, b = r / 255.0, g / 255.0, b / 255.0
    
    # Linearize RGB values (sRGB to linear RGB)
    def linearize(c):
        if c <= 0.03928:
            return c / 12.92
        return ((c + 0.055) / 1.055) ** 2.4
    
    r_linear = linearize(r)
    g_linear = linearize(g)
    b_linear = linearize(b)
    
    # Calculate relative luminance
    return 0.2126 * r_linear + 0.7152 * g_linear + 0.0722 * b_linear

def should_use_black_text(background_color):
    """
    Determine if black text should be used on the given background color.
    Uses WCAG contrast ratio calculation for accessibility.
    
    Returns True if black text provides better contrast, False otherwise.
    """
    # Calculate luminance of background
    bg_luminance = calculate_relative_luminance(background_color)
    
    # Luminance of white (#FFFFFF) is 1.0
    # Luminance of black (#000000) is 0.0
    white_luminance = 1.0
    black_luminance = 0.0
    
    # Calculate contrast ratios (WCAG formula)
    # Contrast = (lighter + 0.05) / (darker + 0.05)
    contrast_with_white = (max(bg_luminance, white_luminance) + 0.05) / (min(bg_luminance, white_luminance) + 0.05)
    contrast_with_black = (max(bg_luminance, black_luminance) + 0.05) / (min(bg_luminance, black_luminance) + 0.05)
    
    # Return True if black text has better contrast
    return contrast_with_black > contrast_with_white

def create_svg_base(width, height):
    """Create base SVG element"""
    svg = ET.Element('svg', {
        'xmlns': 'http://www.w3.org/2000/svg',
        'width': str(width),
        'height': str(height),
        'viewBox': f'0 0 {width} {height}'
    })
    # Add background
    ET.SubElement(svg, 'rect', {
        'width': str(width),
        'height': str(height),
        'fill': COLORS['background']
    })
    return svg

def add_node_box(svg, x, y, width, height, color, title, border_radius='8'):
    """Add a Blueprint node box"""
    # Main node container with shadow
    ET.SubElement(svg, 'rect', {
        'x': str(x + 3),
        'y': str(y + 3),
        'width': str(width),
        'height': str(height),
        'rx': border_radius,
        'fill': '#000000',
        'opacity': '0.3'
    })
    
    # Node border
    ET.SubElement(svg, 'rect', {
        'x': str(x),
        'y': str(y),
        'width': str(width),
        'height': str(height),
        'rx': border_radius,
        'fill': 'none',
        'stroke': '#000000',
        'stroke-width': '2'
    })
    
    # Node header
    header_height = 32
    ET.SubElement(svg, 'rect', {
        'x': str(x),
        'y': str(y),
        'width': str(width),
        'height': str(header_height),
        'rx': border_radius,
        'fill': color
    })
    
    # Determine text color based on background color using WCAG contrast calculation
    text_color = '#000000' if should_use_black_text(color) else COLORS['text']
    
    # Header text
    text = ET.SubElement(svg, 'text', {
        'x': str(x + 10),
        'y': str(y + 22),
        'fill': text_color,
        'font-family': 'Arial, sans-serif',
        'font-size': '14',
        'font-weight': 'bold'
    })
    text.text = title
    
    # Node body
    ET.SubElement(svg, 'rect', {
        'x': str(x),
        'y': str(y + header_height),
        'width': str(width),
        'height': str(height - header_height),
        'fill': '#1E1E1E'
    })
    
    return y + header_height

def add_pin(svg, x, y, pin_type, label, is_input=True, is_exec=False):
    """Add an input or output pin"""
    pin_size = 12
    
    # Determine pin color
    pin_color = COLORS.get(f'{pin_type}_pin', COLORS['object_pin'])
    if is_exec:
        pin_color = COLORS['exec_pin']
    
    # Pin circle
    circle_x = x + 5 if is_input else x - 5
    ET.SubElement(svg, 'circle', {
        'cx': str(circle_x),
        'cy': str(y),
        'r': str(pin_size // 2),
        'fill': pin_color,
        'stroke': COLORS['pin_border'],
        'stroke-width': '1.5'
    })
    
    # Pin label
    text_x = x + 20 if is_input else x - 20
    anchor = 'start' if is_input else 'end'
    text = ET.SubElement(svg, 'text', {
        'x': str(text_x),
        'y': str(y + 4),
        'fill': COLORS['text'],
        'font-family': 'Arial, sans-serif',
        'font-size': '12',
        'text-anchor': anchor
    })
    text.text = label
    
    return y + 20

def add_connection_wire(svg, x1, y1, x2, y2, color=None):
    """Add a connection wire between pins"""
    if color is None:
        color = COLORS['exec_pin']
    
    # Bezier curve for wire
    mid_x = (x1 + x2) / 2
    path_d = f'M {x1} {y1} C {mid_x} {y1}, {mid_x} {y2}, {x2} {y2}'
    
    ET.SubElement(svg, 'path', {
        'd': path_d,
        'stroke': color,
        'stroke-width': '3',
        'fill': 'none',
        'opacity': '0.8'
    })

def generate_event_node():
    """Generate an Event node example"""
    svg = create_svg_base(400, 120)
    
    body_y = add_node_box(svg, 20, 20, 360, 80, COLORS['event'], 'EVENT: OnMarketInventoryUpdated')
    
    # Output exec pin
    add_pin(svg, 360, body_y + 25, 'exec', 'Exec', is_input=False, is_exec=True)
    
    return svg

def generate_function_node():
    """Generate a Function node example"""
    svg = create_svg_base(400, 200)
    
    body_y = add_node_box(svg, 20, 20, 360, 160, COLORS['function'], 'FUNCTION: Get Available Items')
    
    # Input pins
    pin_y = body_y + 20
    pin_y = add_pin(svg, 20, pin_y, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 20, pin_y, 'object', 'Target: TradingInterface', is_input=True)
    
    # Output pins
    pin_y = body_y + 20
    pin_y = add_pin(svg, 380, pin_y, 'exec', 'Exec', is_input=False, is_exec=True)
    add_pin(svg, 380, pin_y, 'object', 'Return: Array<Entry>', is_input=False)
    
    # Description box
    desc_y = body_y + 80
    ET.SubElement(svg, 'rect', {
        'x': '35',
        'y': str(desc_y),
        'width': '330',
        'height': '30',
        'fill': '#2D2D2D',
        'stroke': '#555555',
        'stroke-width': '1',
        'rx': '3'
    })
    desc_text = ET.SubElement(svg, 'text', {
        'x': '45',
        'y': str(desc_y + 20),
        'fill': '#AAAAAA',
        'font-family': 'Arial, sans-serif',
        'font-size': '11',
        'font-style': 'italic'
    })
    desc_text.text = 'C++ Function - Returns current market inventory'
    
    return svg

def generate_foreach_loop():
    """Generate a ForEach Loop node"""
    svg = create_svg_base(400, 220)
    
    body_y = add_node_box(svg, 20, 20, 360, 180, COLORS['flow_control'], 'FLOW CONTROL: ForEach Loop')
    
    # Input pins
    pin_y = body_y + 20
    pin_y = add_pin(svg, 20, pin_y, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 20, pin_y, 'object', 'Array: Available Items', is_input=True)
    
    # Output pins
    pin_y = body_y + 20
    pin_y = add_pin(svg, 380, pin_y, 'exec', 'Loop Body', is_input=False, is_exec=True)
    pin_y = add_pin(svg, 380, pin_y, 'exec', 'Completed', is_input=False, is_exec=True)
    pin_y = add_pin(svg, 380, pin_y, 'object', 'Array Element', is_input=False)
    add_pin(svg, 380, pin_y, 'int', 'Array Index', is_input=False)
    
    return svg

def generate_branch_node():
    """Generate a Branch (if/else) node"""
    svg = create_svg_base(400, 160)
    
    body_y = add_node_box(svg, 20, 20, 360, 120, COLORS['branch'], 'BRANCH: Is Valid?', border_radius='4')
    
    # Input pins
    pin_y = body_y + 20
    pin_y = add_pin(svg, 20, pin_y, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 20, pin_y, 'bool', 'Condition', is_input=True)
    
    # Output pins
    pin_y = body_y + 20
    pin_y = add_pin(svg, 380, pin_y, 'exec', 'True', is_input=False, is_exec=True)
    add_pin(svg, 380, pin_y, 'exec', 'False', is_input=False, is_exec=True)
    
    # Add text in body
    text = ET.SubElement(svg, 'text', {
        'x': '200',
        'y': str(body_y + 80),
        'fill': '#000000',
        'font-family': 'Arial, sans-serif',
        'font-size': '12',
        'text-anchor': 'middle'
    })
    text.text = 'Credits >= TotalPrice'
    
    return svg

def generate_complete_flow():
    """Generate a complete flow showing multiple connected nodes"""
    svg = create_svg_base(800, 600)
    
    # Event node
    body_y1 = add_node_box(svg, 250, 20, 300, 80, COLORS['event'], 'EVENT: OnBuyButtonClicked')
    add_pin(svg, 530, body_y1 + 25, 'exec', '', is_input=False, is_exec=True)
    
    # Function node 1
    body_y2 = add_node_box(svg, 200, 140, 400, 140, COLORS['function'], 'Get Item Price')
    add_pin(svg, 200, body_y2 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 200, body_y2 + 40, 'object', 'Item', is_input=True)
    add_pin(svg, 200, body_y2 + 60, 'int', 'Quantity', is_input=True)
    add_pin(svg, 600, body_y2 + 20, 'exec', '', is_input=False, is_exec=True)
    add_pin(svg, 600, body_y2 + 40, 'int', 'Total Price', is_input=False)
    
    # Branch node
    body_y3 = add_node_box(svg, 230, 320, 340, 120, COLORS['branch'], 'BRANCH: Can Afford?')
    add_pin(svg, 230, body_y3 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 230, body_y3 + 40, 'bool', 'Condition', is_input=True)
    add_pin(svg, 570, body_y3 + 20, 'exec', 'True', is_input=False, is_exec=True)
    add_pin(svg, 570, body_y3 + 40, 'exec', 'False', is_input=False, is_exec=True)
    
    # Success node
    body_y4 = add_node_box(svg, 50, 480, 280, 80, COLORS['function'], 'Execute Purchase')
    add_pin(svg, 50, body_y4 + 25, 'exec', 'Exec', is_input=True, is_exec=True)
    
    # Error node
    body_y5 = add_node_box(svg, 470, 480, 280, 80, COLORS['function'], 'Show Error')
    add_pin(svg, 470, body_y5 + 25, 'exec', 'Exec', is_input=True, is_exec=True)
    
    # Connection wires
    add_connection_wire(svg, 530, body_y1 + 25, 200, body_y2 + 20)
    add_connection_wire(svg, 600, body_y2 + 20, 230, body_y3 + 20)
    add_connection_wire(svg, 570, body_y3 + 20, 190, body_y4 + 25)
    add_connection_wire(svg, 570, body_y3 + 40, 470, body_y5 + 25)
    
    return svg

def generate_buy_sell_toggle():
    """Generate Buy/Sell Toggle button flow"""
    svg = create_svg_base(800, 400)
    
    # Event: Buy Button Clicked
    body_y1 = add_node_box(svg, 50, 20, 300, 80, COLORS['event'], 'EVENT: Buy Button Clicked')
    add_pin(svg, 330, body_y1 + 25, 'exec', '', is_input=False, is_exec=True)
    
    # Function: ToggleBuySellView
    body_y2 = add_node_box(svg, 450, 20, 300, 120, COLORS['function'], 'ToggleBuySellView')
    add_pin(svg, 450, body_y2 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 450, body_y2 + 40, 'object', 'Target', is_input=True)
    add_pin(svg, 750, body_y2 + 20, 'exec', '', is_input=False, is_exec=True)
    
    # Connection wire
    add_connection_wire(svg, 330, body_y1 + 25, 450, body_y2 + 20)
    
    # Event: Sell Button Clicked
    body_y3 = add_node_box(svg, 50, 170, 300, 80, COLORS['event'], 'EVENT: Sell Button Clicked')
    add_pin(svg, 330, body_y3 + 25, 'exec', '', is_input=False, is_exec=True)
    
    # Function: ToggleBuySellView (second instance)
    body_y4 = add_node_box(svg, 450, 170, 300, 120, COLORS['function'], 'ToggleBuySellView')
    add_pin(svg, 450, body_y4 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 450, body_y4 + 40, 'object', 'Target', is_input=True)
    add_pin(svg, 750, body_y4 + 20, 'exec', '', is_input=False, is_exec=True)
    
    # Connection wire
    add_connection_wire(svg, 330, body_y3 + 25, 450, body_y4 + 20)
    
    # Description text
    desc_text = ET.SubElement(svg, 'text', {
        'x': '400',
        'y': '350',
        'fill': COLORS['text'],
        'font-family': 'Arial, sans-serif',
        'font-size': '14',
        'text-anchor': 'middle',
        'font-style': 'italic'
    })
    desc_text.text = 'Note: Same function toggles between Buy and Sell modes'
    
    return svg

def generate_add_to_cart_flow():
    """Generate Add to Cart validation flow"""
    svg = create_svg_base(900, 700)
    
    # Event: Add to Cart Clicked
    body_y1 = add_node_box(svg, 300, 20, 300, 100, COLORS['event'], 'EVENT: Add to Cart')
    add_pin(svg, 580, body_y1 + 25, 'exec', '', is_input=False, is_exec=True)
    add_pin(svg, 580, body_y1 + 45, 'object', 'Item', is_input=False)
    add_pin(svg, 580, body_y1 + 65, 'int', 'Quantity', is_input=False)
    
    # Branch: In Stock?
    body_y2 = add_node_box(svg, 250, 160, 400, 120, COLORS['branch'], 'BRANCH: Is Item In Stock?')
    add_pin(svg, 250, body_y2 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 250, body_y2 + 40, 'bool', 'Condition', is_input=True)
    add_pin(svg, 650, body_y2 + 20, 'exec', 'True', is_input=False, is_exec=True)
    add_pin(svg, 650, body_y2 + 40, 'exec', 'False', is_input=False, is_exec=True)
    
    # Branch: Can Afford?
    body_y3 = add_node_box(svg, 250, 320, 400, 120, COLORS['branch'], 'BRANCH: Can Afford?')
    add_pin(svg, 250, body_y3 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 250, body_y3 + 40, 'bool', 'Condition', is_input=True)
    add_pin(svg, 650, body_y3 + 20, 'exec', 'True', is_input=False, is_exec=True)
    add_pin(svg, 650, body_y3 + 40, 'exec', 'False', is_input=False, is_exec=True)
    
    # Branch: Has Space?
    body_y4 = add_node_box(svg, 250, 480, 400, 120, COLORS['branch'], 'BRANCH: Has Cargo Space?')
    add_pin(svg, 250, body_y4 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 250, body_y4 + 40, 'bool', 'Condition', is_input=True)
    add_pin(svg, 650, body_y4 + 20, 'exec', 'True', is_input=False, is_exec=True)
    add_pin(svg, 650, body_y4 + 40, 'exec', 'False', is_input=False, is_exec=True)
    
    # Success: Add to Cart
    body_y5 = add_node_box(svg, 50, 630, 250, 60, COLORS['function'], 'Add to Cart')
    add_pin(svg, 50, body_y5 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    
    # Error nodes
    body_y6 = add_node_box(svg, 320, 630, 250, 60, COLORS['function'], 'Show Error')
    add_pin(svg, 320, body_y6 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    
    body_y7 = add_node_box(svg, 590, 630, 250, 60, COLORS['function'], 'Show Error')
    add_pin(svg, 590, body_y7 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    
    # Connection wires
    add_connection_wire(svg, 580, body_y1 + 25, 250, body_y2 + 20)
    add_connection_wire(svg, 650, body_y2 + 20, 250, body_y3 + 20)
    add_connection_wire(svg, 650, body_y2 + 40, 445, body_y6 + 20)
    add_connection_wire(svg, 650, body_y3 + 20, 250, body_y4 + 20)
    add_connection_wire(svg, 650, body_y3 + 40, 445, body_y6 + 20)
    add_connection_wire(svg, 650, body_y4 + 20, 175, body_y5 + 20)
    add_connection_wire(svg, 650, body_y4 + 40, 715, body_y7 + 20)
    
    return svg

def generate_execute_trade_flow():
    """Generate Execute Trade complete flow"""
    svg = create_svg_base(900, 600)
    
    # Event: Complete Trade Button
    body_y1 = add_node_box(svg, 300, 20, 300, 80, COLORS['event'], 'EVENT: Complete Trade Clicked')
    add_pin(svg, 580, body_y1 + 25, 'exec', '', is_input=False, is_exec=True)
    
    # Function: Validate Transaction
    body_y2 = add_node_box(svg, 200, 140, 500, 140, COLORS['function'], 'Validate Transaction')
    add_pin(svg, 200, body_y2 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 700, body_y2 + 20, 'exec', '', is_input=False, is_exec=True)
    add_pin(svg, 700, body_y2 + 40, 'bool', 'Is Valid', is_input=False)
    add_pin(svg, 700, body_y2 + 60, 'text', 'Error Message', is_input=False)
    
    # Branch: Is Valid?
    body_y3 = add_node_box(svg, 250, 320, 400, 120, COLORS['branch'], 'BRANCH: Transaction Valid?')
    add_pin(svg, 250, body_y3 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 250, body_y3 + 40, 'bool', 'Condition', is_input=True)
    add_pin(svg, 650, body_y3 + 20, 'exec', 'True', is_input=False, is_exec=True)
    add_pin(svg, 650, body_y3 + 40, 'exec', 'False', is_input=False, is_exec=True)
    
    # Success: Execute Trade
    body_y4 = add_node_box(svg, 50, 480, 350, 100, COLORS['function'], 'Execute Trade')
    add_pin(svg, 50, body_y4 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 400, body_y4 + 20, 'exec', '', is_input=False, is_exec=True)
    add_pin(svg, 400, body_y4 + 40, 'bool', 'Success', is_input=False)
    
    # Error: Show Error
    body_y5 = add_node_box(svg, 500, 480, 350, 80, COLORS['function'], 'On Trade Completed (Error)')
    add_pin(svg, 500, body_y5 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 500, body_y5 + 40, 'text', 'Error Message', is_input=True)
    
    # Connection wires
    add_connection_wire(svg, 580, body_y1 + 25, 200, body_y2 + 20)
    add_connection_wire(svg, 700, body_y2 + 20, 250, body_y3 + 20)
    add_connection_wire(svg, 650, body_y3 + 20, 50, body_y4 + 20)
    add_connection_wire(svg, 650, body_y3 + 40, 500, body_y5 + 20)
    
    return svg

def generate_market_item_display():
    """Generate Market Item Display population flow"""
    svg = create_svg_base(800, 600)
    
    # Event: Open Market
    body_y1 = add_node_box(svg, 250, 20, 300, 100, COLORS['event'], 'EVENT: Open Market')
    add_pin(svg, 530, body_y1 + 25, 'exec', '', is_input=False, is_exec=True)
    add_pin(svg, 530, body_y1 + 45, 'object', 'Market Data', is_input=False)
    
    # Function: Get Available Items
    body_y2 = add_node_box(svg, 200, 160, 400, 120, COLORS['function'], 'Get Available Items')
    add_pin(svg, 200, body_y2 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 200, body_y2 + 40, 'object', 'Market', is_input=True)
    add_pin(svg, 600, body_y2 + 20, 'exec', '', is_input=False, is_exec=True)
    add_pin(svg, 600, body_y2 + 40, 'object', 'Items Array', is_input=False)
    
    # ForEach Loop
    body_y3 = add_node_box(svg, 180, 320, 440, 180, COLORS['flow_control'], 'FLOW: ForEach Loop')
    add_pin(svg, 180, body_y3 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 180, body_y3 + 40, 'object', 'Array', is_input=True)
    add_pin(svg, 620, body_y3 + 20, 'exec', 'Loop Body', is_input=False, is_exec=True)
    add_pin(svg, 620, body_y3 + 40, 'exec', 'Completed', is_input=False, is_exec=True)
    add_pin(svg, 620, body_y3 + 60, 'object', 'Array Element', is_input=False)
    add_pin(svg, 620, body_y3 + 80, 'int', 'Array Index', is_input=False)
    
    # Function: Create Widget
    body_y4 = add_node_box(svg, 150, 530, 250, 60, COLORS['function'], 'Create Widget')
    add_pin(svg, 150, body_y4 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    
    # Function: Add to List
    body_y5 = add_node_box(svg, 450, 530, 250, 60, COLORS['function'], 'Add to ScrollBox')
    add_pin(svg, 450, body_y5 + 20, 'exec', 'Exec', is_input=True, is_exec=True)
    
    # Connection wires
    add_connection_wire(svg, 530, body_y1 + 25, 200, body_y2 + 20)
    add_connection_wire(svg, 600, body_y2 + 20, 180, body_y3 + 20)
    add_connection_wire(svg, 620, body_y3 + 20, 150, body_y4 + 20)
    add_connection_wire(svg, 400, body_y4 + 20, 450, body_y5 + 20)
    
    return svg

def generate_trade_item_row_initialize():
    """Generate WBP_TradeItemRow Initialize flow"""
    svg = create_svg_base(800, 350)
    
    # Event: Construct
    body_y1 = add_node_box(svg, 20, 20, 180, 60, COLORS['event'], 'EVENT: Construct')
    add_pin(svg, 200, body_y1 + 20, 'exec', '', is_input=False, is_exec=True)
    
    # Function: Set Item Data
    body_y2 = add_node_box(svg, 230, 20, 200, 80, COLORS['function'], 'Set Item Data')
    pin_y = body_y2 + 15
    pin_y = add_pin(svg, 230, pin_y, 'exec', '', is_input=True, is_exec=True)
    add_pin(svg, 230, pin_y, 'struct', 'Item Data', is_input=True)
    add_pin(svg, 430, body_y2 + 15, 'exec', '', is_input=False, is_exec=True)
    
    # Function: Update Display
    body_y3 = add_node_box(svg, 460, 20, 180, 60, COLORS['function'], 'Update Display')
    add_pin(svg, 460, body_y3 + 20, 'exec', '', is_input=True, is_exec=True)
    add_pin(svg, 640, body_y3 + 20, 'exec', '', is_input=False, is_exec=True)
    
    # Function: Set Quantity
    body_y4 = add_node_box(svg, 230, 140, 200, 80, COLORS['function'], 'Set Quantity')
    pin_y = body_y4 + 15
    pin_y = add_pin(svg, 230, pin_y, 'exec', '', is_input=True, is_exec=True)
    add_pin(svg, 230, pin_y, 'int', 'Value: 1', is_input=True)
    add_pin(svg, 430, body_y4 + 15, 'exec', '', is_input=False, is_exec=True)
    
    # Function: Bind Events
    body_y5 = add_node_box(svg, 460, 140, 180, 60, COLORS['function'], 'Bind Events')
    add_pin(svg, 460, body_y5 + 20, 'exec', '', is_input=True, is_exec=True)
    add_pin(svg, 640, body_y5 + 20, 'exec', '', is_input=False, is_exec=True)
    
    # Connection wires
    add_connection_wire(svg, 200, body_y1 + 20, 230, body_y2 + 15)
    add_connection_wire(svg, 430, body_y2 + 15, 460, body_y3 + 20)
    add_connection_wire(svg, 640, body_y3 + 20, 230, body_y4 + 15)
    add_connection_wire(svg, 430, body_y4 + 15, 460, body_y5 + 20)
    
    # Add labels
    labels = [
        (110, 60, "1. Widget created"),
        (340, 60, "2. Receive item data"),
        (550, 60, "3. Populate UI"),
        (340, 180, "4. Initialize qty"),
        (550, 180, "5. Connect handlers")
    ]
    for x, y, label in labels:
        text = ET.SubElement(svg, 'text', {
            'x': str(x), 'y': str(y),
            'fill': '#CCCCCC', 'font-family': 'Arial, sans-serif',
            'font-size': '11', 'text-anchor': 'middle'
        })
        text.text = label
    
    return svg

def generate_trade_item_row_add_to_cart():
    """Generate WBP_TradeItemRow Add to Cart flow"""
    svg = create_svg_base(750, 480)
    
    # Event: OnAddToCartClicked
    body_y1 = add_node_box(svg, 20, 20, 220, 60, COLORS['event'], 'EVENT: OnAddToCartClicked')
    add_pin(svg, 240, body_y1 + 20, 'exec', '', is_input=False, is_exec=True)
    
    # Function: Get Quantity
    body_y2 = add_node_box(svg, 270, 20, 180, 60, COLORS['function'], 'Get Quantity')
    add_pin(svg, 270, body_y2 + 20, 'exec', '', is_input=True, is_exec=True)
    add_pin(svg, 450, body_y2 + 20, 'int', 'Quantity', is_input=False)
    
    # Function: Validate Quantity
    body_y3 = add_node_box(svg, 20, 120, 200, 80, COLORS['pure'], 'Validate Quantity')
    pin_y = body_y3 + 15
    pin_y = add_pin(svg, 20, pin_y, 'int', 'Quantity', is_input=True)
    add_pin(svg, 20, pin_y, 'int', 'Stock', is_input=True)
    add_pin(svg, 220, body_y3 + 35, 'bool', 'Is Valid', is_input=False)
    
    # Branch: Is Valid?
    body_y4 = add_node_box(svg, 250, 120, 220, 100, COLORS['branch'], 'BRANCH: Is Valid?')
    pin_y = body_y4 + 15
    pin_y = add_pin(svg, 250, pin_y, 'exec', '', is_input=True, is_exec=True)
    add_pin(svg, 250, pin_y, 'bool', 'Condition', is_input=True)
    pin_y = body_y4 + 15
    pin_y = add_pin(svg, 470, pin_y, 'exec', 'True', is_input=False, is_exec=True)
    add_pin(svg, 470, pin_y, 'exec', 'False', is_input=False, is_exec=True)
    
    # Function: Add to Cart (True path)
    body_y5 = add_node_box(svg, 500, 120, 220, 80, COLORS['function'], 'Add Item To Cart')
    pin_y = body_y5 + 15
    pin_y = add_pin(svg, 500, pin_y, 'exec', '', is_input=True, is_exec=True)
    add_pin(svg, 500, pin_y, 'int', 'Quantity', is_input=True)
    add_pin(svg, 720, body_y5 + 15, 'exec', '', is_input=False, is_exec=True)
    
    # Function: Show Error (False path)
    body_y6 = add_node_box(svg, 500, 240, 180, 60, COLORS['function'], 'Show Error')
    add_pin(svg, 500, body_y6 + 20, 'exec', '', is_input=True, is_exec=True)
    
    # Function: Update Visual State
    body_y7 = add_node_box(svg, 270, 280, 200, 60, COLORS['function'], 'Update Visual State')
    add_pin(svg, 270, body_y7 + 20, 'exec', '', is_input=True, is_exec=True)
    add_pin(svg, 470, body_y7 + 20, 'exec', '', is_input=False, is_exec=True)
    
    # Function: Reset Quantity
    body_y8 = add_node_box(svg, 20, 360, 180, 60, COLORS['function'], 'Reset Quantity')
    add_pin(svg, 20, body_y8 + 20, 'exec', '', is_input=True, is_exec=True)
    
    # Connection wires
    add_connection_wire(svg, 240, body_y1 + 20, 270, body_y2 + 20)
    add_connection_wire(svg, 220, body_y3 + 35, 250, body_y4 + 35)
    add_connection_wire(svg, 470, body_y4 + 15, 500, body_y5 + 15)  # True path
    add_connection_wire(svg, 470, body_y4 + 35, 500, body_y6 + 20)  # False path
    add_connection_wire(svg, 720, body_y5 + 15, 270, body_y7 + 20)
    add_connection_wire(svg, 470, body_y7 + 20, 20, body_y8 + 20)
    
    return svg

def generate_trade_item_row_quantity_increment():
    """Generate WBP_TradeItemRow Quantity Increment flow"""
    svg = create_svg_base(700, 380)
    
    # Event: OnIncrementClicked
    body_y1 = add_node_box(svg, 20, 20, 220, 60, COLORS['event'], 'EVENT: OnIncrementClicked')
    add_pin(svg, 240, body_y1 + 20, 'exec', '', is_input=False, is_exec=True)
    
    # Function: Get Current Quantity
    body_y2 = add_node_box(svg, 270, 20, 220, 60, COLORS['function'], 'Get Current Quantity')
    add_pin(svg, 270, body_y2 + 20, 'exec', '', is_input=True, is_exec=True)
    add_pin(svg, 490, body_y2 + 20, 'int', 'Quantity', is_input=False)
    
    # Function: Increment (Math)
    body_y3 = add_node_box(svg, 40, 120, 180, 80, COLORS['pure'], 'Add (+)')
    pin_y = body_y3 + 15
    pin_y = add_pin(svg, 40, pin_y, 'int', 'A', is_input=True)
    add_pin(svg, 40, pin_y, 'int', 'B: 1', is_input=True)
    add_pin(svg, 220, body_y3 + 35, 'int', 'Result', is_input=False)
    
    # Branch: Check Max Stock
    body_y4 = add_node_box(svg, 250, 120, 240, 100, COLORS['branch'], 'BRANCH: Check Max Stock')
    pin_y = body_y4 + 15
    pin_y = add_pin(svg, 250, pin_y, 'exec', '', is_input=True, is_exec=True)
    add_pin(svg, 250, pin_y, 'bool', 'Qty <= Stock', is_input=True)
    pin_y = body_y4 + 15
    pin_y = add_pin(svg, 490, pin_y, 'exec', 'True', is_input=False, is_exec=True)
    add_pin(svg, 490, pin_y, 'exec', 'False', is_input=False, is_exec=True)
    
    # Function: Update Display (True path)
    body_y5 = add_node_box(svg, 520, 100, 160, 60, COLORS['function'], 'Update Display')
    add_pin(svg, 520, body_y5 + 20, 'exec', '', is_input=True, is_exec=True)
    
    # Function: Show Message (False path)
    body_y6 = add_node_box(svg, 520, 180, 160, 60, COLORS['function'], 'Show Message')
    add_pin(svg, 520, body_y6 + 20, 'exec', '', is_input=True, is_exec=True)
    
    # Connection wires
    add_connection_wire(svg, 240, body_y1 + 20, 270, body_y2 + 20)
    add_connection_wire(svg, 220, body_y3 + 35, 250, body_y4 + 35)
    add_connection_wire(svg, 490, body_y4 + 15, 520, body_y5 + 20)  # True
    add_connection_wire(svg, 490, body_y4 + 35, 520, body_y6 + 20)  # False
    
    # Add labels
    labels = [
        (370, 60, "1. Get current"),
        (130, 160, "2. Add 1"),
        (370, 160, "3. Check limit"),
        (600, 140, "4. Update UI"),
        (600, 220, "Max reached")
    ]
    for x, y, label in labels:
        text = ET.SubElement(svg, 'text', {
            'x': str(x), 'y': str(y),
            'fill': '#CCCCCC', 'font-family': 'Arial, sans-serif',
            'font-size': '11', 'text-anchor': 'middle'
        })
        text.text = label
    
    return svg

def generate_trade_item_row_quantity_decrement():
    """Generate WBP_TradeItemRow Quantity Decrement flow"""
    svg = create_svg_base(700, 380)
    
    # Event: OnDecrementClicked
    body_y1 = add_node_box(svg, 20, 20, 220, 60, COLORS['event'], 'EVENT: OnDecrementClicked')
    add_pin(svg, 240, body_y1 + 20, 'exec', '', is_input=False, is_exec=True)
    
    # Function: Get Current Quantity
    body_y2 = add_node_box(svg, 270, 20, 220, 60, COLORS['function'], 'Get Current Quantity')
    add_pin(svg, 270, body_y2 + 20, 'exec', '', is_input=True, is_exec=True)
    add_pin(svg, 490, body_y2 + 20, 'int', 'Quantity', is_input=False)
    
    # Function: Decrement (Math)
    body_y3 = add_node_box(svg, 40, 120, 180, 80, COLORS['pure'], 'Subtract (-)')
    pin_y = body_y3 + 15
    pin_y = add_pin(svg, 40, pin_y, 'int', 'A', is_input=True)
    add_pin(svg, 40, pin_y, 'int', 'B: 1', is_input=True)
    add_pin(svg, 220, body_y3 + 35, 'int', 'Result', is_input=False)
    
    # Branch: Check Minimum
    body_y4 = add_node_box(svg, 250, 120, 240, 100, COLORS['branch'], 'BRANCH: Check Minimum')
    pin_y = body_y4 + 15
    pin_y = add_pin(svg, 250, pin_y, 'exec', '', is_input=True, is_exec=True)
    add_pin(svg, 250, pin_y, 'bool', 'Qty >= 1', is_input=True)
    pin_y = body_y4 + 15
    pin_y = add_pin(svg, 490, pin_y, 'exec', 'True', is_input=False, is_exec=True)
    add_pin(svg, 490, pin_y, 'exec', 'False', is_input=False, is_exec=True)
    
    # Function: Update Display (True path)
    body_y5 = add_node_box(svg, 520, 100, 160, 60, COLORS['function'], 'Update Display')
    add_pin(svg, 520, body_y5 + 20, 'exec', '', is_input=True, is_exec=True)
    
    # Function: Keep at 1 (False path)
    body_y6 = add_node_box(svg, 520, 180, 160, 60, COLORS['function'], 'Keep at 1')
    add_pin(svg, 520, body_y6 + 20, 'exec', '', is_input=True, is_exec=True)
    
    # Connection wires
    add_connection_wire(svg, 240, body_y1 + 20, 270, body_y2 + 20)
    add_connection_wire(svg, 220, body_y3 + 35, 250, body_y4 + 35)
    add_connection_wire(svg, 490, body_y4 + 15, 520, body_y5 + 20)  # True
    add_connection_wire(svg, 490, body_y4 + 35, 520, body_y6 + 20)  # False
    
    # Add labels
    labels = [
        (370, 60, "1. Get current"),
        (130, 160, "2. Subtract 1"),
        (370, 160, "3. Check min"),
        (600, 140, "4. Update UI"),
        (600, 220, "Min is 1")
    ]
    for x, y, label in labels:
        text = ET.SubElement(svg, 'text', {
            'x': str(x), 'y': str(y),
            'fill': '#CCCCCC', 'font-family': 'Arial, sans-serif',
            'font-size': '11', 'text-anchor': 'middle'
        })
        text.text = label
    
    return svg

def generate_docking_range_overlap():
    """Generate Docking Range Overlap detection flow"""
    svg = create_svg_base(950, 600)
    
    # Title
    title = ET.SubElement(svg, 'text', {
        'x': '475', 'y': '30',
        'fill': COLORS['text'], 'font-family': 'Arial, sans-serif',
        'font-size': '18', 'font-weight': 'bold', 'text-anchor': 'middle'
    })
    title.text = 'BP_SpaceStation - Docking Range Overlap Detection'
    
    # Event: OnComponentBeginOverlap
    body_y1 = add_node_box(svg, 50, 60, 380, 120, COLORS['event'], 'EVENT: OnComponentBeginOverlap')
    pin_y = body_y1 + 15
    pin_y = add_pin(svg, 430, pin_y, 'exec', '', is_input=False, is_exec=True)
    pin_y = add_pin(svg, 430, pin_y, 'object', 'Overlapped Component', is_input=False)
    add_pin(svg, 430, pin_y, 'object', 'Other Actor', is_input=False)
    
    # Function: Cast to ASpaceship
    body_y2 = add_node_box(svg, 480, 60, 400, 120, COLORS['function'], 'Cast to ASpaceship')
    pin_y = body_y2 + 15
    pin_y = add_pin(svg, 480, pin_y, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 480, pin_y, 'object', 'Object', is_input=True)
    pin_y = body_y2 + 15
    pin_y = add_pin(svg, 880, pin_y, 'exec', '', is_input=False, is_exec=True)
    add_pin(svg, 880, pin_y, 'object', 'As Spaceship', is_input=False)
    
    # Branch: Is Valid Cast?
    body_y3 = add_node_box(svg, 150, 230, 280, 100, COLORS['branch'], 'BRANCH: Is Valid Ship?')
    pin_y = body_y3 + 15
    pin_y = add_pin(svg, 150, pin_y, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 150, pin_y, 'bool', 'Is Valid', is_input=True)
    pin_y = body_y3 + 15
    pin_y = add_pin(svg, 430, pin_y, 'exec', 'True', is_input=False, is_exec=True)
    add_pin(svg, 430, pin_y, 'exec', 'False', is_input=False, is_exec=True)
    
    # Function: Is Player Controlled?
    body_y4 = add_node_box(svg, 480, 230, 400, 100, COLORS['function'], 'Is Player Controlled?')
    pin_y = body_y4 + 15
    pin_y = add_pin(svg, 480, pin_y, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 480, pin_y, 'object', 'Target: Spaceship', is_input=True)
    pin_y = body_y4 + 15
    pin_y = add_pin(svg, 880, pin_y, 'exec', '', is_input=False, is_exec=True)
    add_pin(svg, 880, pin_y, 'bool', 'Is Player', is_input=False)
    
    # Branch: Is Player?
    body_y5 = add_node_box(svg, 150, 380, 280, 100, COLORS['branch'], 'BRANCH: Is Player?')
    pin_y = body_y5 + 15
    pin_y = add_pin(svg, 150, pin_y, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 150, pin_y, 'bool', 'Condition', is_input=True)
    pin_y = body_y5 + 15
    pin_y = add_pin(svg, 430, pin_y, 'exec', 'True', is_input=False, is_exec=True)
    add_pin(svg, 430, pin_y, 'exec', 'False (Ignore)', is_input=False, is_exec=True)
    
    # Function: Show Docking Prompt
    body_y6 = add_node_box(svg, 480, 380, 400, 100, COLORS['function'], 'Show Docking UI Prompt')
    pin_y = body_y6 + 15
    pin_y = add_pin(svg, 480, pin_y, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 480, pin_y, 'text', 'Message: "Press F to Dock"', is_input=True)
    
    # Connection wires
    add_connection_wire(svg, 430, body_y1 + 15, 480, body_y2 + 15)
    add_connection_wire(svg, 880, body_y2 + 15, 150, body_y3 + 15)
    add_connection_wire(svg, 430, body_y3 + 15, 480, body_y4 + 15)
    add_connection_wire(svg, 880, body_y4 + 15, 150, body_y5 + 15)
    add_connection_wire(svg, 430, body_y5 + 15, 480, body_y6 + 15)
    
    # Add labels
    labels = [
        (455, 145, "1. Detect overlap"),
        (680, 145, "2. Cast to ship"),
        (290, 315, "3. Check valid"),
        (680, 315, "4. Check player"),
        (290, 465, "5. If player"),
        (680, 465, "6. Show UI")
    ]
    for x, y, label in labels:
        text = ET.SubElement(svg, 'text', {
            'x': str(x), 'y': str(y),
            'fill': '#CCCCCC', 'font-family': 'Arial, sans-serif',
            'font-size': '11', 'text-anchor': 'middle'
        })
        text.text = label
    
    return svg

def generate_dock_ship_flow():
    """Generate Dock Ship function flow"""
    svg = create_svg_base(950, 700)
    
    # Title
    title = ET.SubElement(svg, 'text', {
        'x': '475', 'y': '30',
        'fill': COLORS['text'], 'font-family': 'Arial, sans-serif',
        'font-size': '18', 'font-weight': 'bold', 'text-anchor': 'middle'
    })
    title.text = 'BP_SpaceStation - Dock Ship Function'
    
    # Function header: Dock Ship
    body_y1 = add_node_box(svg, 50, 60, 380, 120, COLORS['function'], 'FUNCTION: Dock Ship')
    pin_y = body_y1 + 15
    pin_y = add_pin(svg, 50, pin_y, 'exec', 'Exec', is_input=True, is_exec=True)
    pin_y = add_pin(svg, 50, pin_y, 'object', 'Ship: ASpaceship', is_input=True)
    add_pin(svg, 50, pin_y, 'int', 'Docking Point Index', is_input=True)
    
    # Function: Validate Docking
    body_y2 = add_node_box(svg, 480, 60, 400, 140, COLORS['function'], 'Validate Docking')
    pin_y = body_y2 + 15
    pin_y = add_pin(svg, 480, pin_y, 'exec', 'Exec', is_input=True, is_exec=True)
    pin_y = add_pin(svg, 480, pin_y, 'object', 'Ship', is_input=True)
    add_pin(svg, 480, pin_y, 'int', 'Point Index', is_input=True)
    pin_y = body_y2 + 15
    pin_y = add_pin(svg, 880, pin_y, 'exec', '', is_input=False, is_exec=True)
    add_pin(svg, 880, pin_y, 'bool', 'Is Valid', is_input=False)
    
    # Branch: Is Valid?
    body_y3 = add_node_box(svg, 150, 250, 280, 100, COLORS['branch'], 'BRANCH: Is Valid?')
    pin_y = body_y3 + 15
    pin_y = add_pin(svg, 150, pin_y, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 150, pin_y, 'bool', 'Condition', is_input=True)
    pin_y = body_y3 + 15
    pin_y = add_pin(svg, 430, pin_y, 'exec', 'True', is_input=False, is_exec=True)
    add_pin(svg, 430, pin_y, 'exec', 'False', is_input=False, is_exec=True)
    
    # Function: Get Docking Point Transform
    body_y4 = add_node_box(svg, 480, 210, 400, 100, COLORS['pure'], 'Get Docking Point Transform')
    pin_y = body_y4 + 15
    add_pin(svg, 480, pin_y, 'int', 'Index', is_input=True)
    add_pin(svg, 880, body_y4 + 45, 'struct', 'Transform', is_input=False)
    
    # Function: Move Ship To Position
    body_y5 = add_node_box(svg, 480, 330, 400, 120, COLORS['function'], 'Move Ship To Position')
    pin_y = body_y5 + 15
    pin_y = add_pin(svg, 480, pin_y, 'exec', 'Exec', is_input=True, is_exec=True)
    pin_y = add_pin(svg, 480, pin_y, 'object', 'Ship', is_input=True)
    add_pin(svg, 480, pin_y, 'struct', 'Target Transform', is_input=True)
    pin_y = body_y5 + 15
    add_pin(svg, 880, pin_y, 'exec', '', is_input=False, is_exec=True)
    
    # Function: Disable Ship Controls
    body_y6 = add_node_box(svg, 50, 490, 380, 100, COLORS['function'], 'Disable Ship Input')
    pin_y = body_y6 + 15
    pin_y = add_pin(svg, 50, pin_y, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 50, pin_y, 'object', 'Ship', is_input=True)
    pin_y = body_y6 + 15
    add_pin(svg, 430, pin_y, 'exec', '', is_input=False, is_exec=True)
    
    # Function: Mark Docking Point Occupied
    body_y7 = add_node_box(svg, 480, 490, 400, 100, COLORS['function'], 'Mark Point Occupied')
    pin_y = body_y7 + 15
    pin_y = add_pin(svg, 480, pin_y, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 480, pin_y, 'int', 'Point Index', is_input=True)
    pin_y = body_y7 + 15
    add_pin(svg, 880, pin_y, 'exec', '', is_input=False, is_exec=True)
    
    # Function: Open Trading UI
    body_y8 = add_node_box(svg, 250, 620, 400, 60, COLORS['function'], 'Open Trading UI')
    pin_y = body_y8 + 15
    add_pin(svg, 250, pin_y, 'exec', 'Exec', is_input=True, is_exec=True)
    
    # Connection wires
    add_connection_wire(svg, 430, body_y1 + 15, 480, body_y2 + 15)
    add_connection_wire(svg, 880, body_y2 + 15, 150, body_y3 + 15)
    add_connection_wire(svg, 430, body_y3 + 15, 480, body_y5 + 15)
    add_connection_wire(svg, 880, body_y5 + 15, 50, body_y6 + 15)
    add_connection_wire(svg, 430, body_y6 + 15, 480, body_y7 + 15)
    add_connection_wire(svg, 880, body_y7 + 15, 250, body_y8 + 15)
    
    # Add labels
    labels = [
        (680, 145, "1. Validate docking"),
        (290, 335, "2. Check valid"),
        (680, 380, "3. Move ship"),
        (240, 575, "4. Disable input"),
        (680, 575, "5. Mark occupied"),
        (450, 670, "6. Open trading UI")
    ]
    for x, y, label in labels:
        text = ET.SubElement(svg, 'text', {
            'x': str(x), 'y': str(y),
            'fill': '#CCCCCC', 'font-family': 'Arial, sans-serif',
            'font-size': '11', 'text-anchor': 'middle'
        })
        text.text = label
    
    return svg

def generate_docking_point_component_setup():
    """Generate visual guide for adding Scene Components as docking points"""
    svg = create_svg_base(1200, 800)
    
    # Title
    title = ET.SubElement(svg, 'text', {
        'x': '600', 'y': '30',
        'fill': COLORS['text'], 'font-family': 'Arial, sans-serif',
        'font-size': '20', 'font-weight': 'bold', 'text-anchor': 'middle'
    })
    title.text = 'Docking Bay Module - Scene Component Setup'
    
    # Subtitle
    subtitle = ET.SubElement(svg, 'text', {
        'x': '600', 'y': '55',
        'fill': '#AAAAAA', 'font-family': 'Arial, sans-serif',
        'font-size': '14', 'text-anchor': 'middle'
    })
    subtitle.text = 'Add Scene Components for docking points in the Components panel'
    
    # Component Hierarchy Box
    ET.SubElement(svg, 'rect', {
        'x': '50', 'y': '80',
        'width': '500', 'height': '650',
        'fill': '#1A1A1A',
        'stroke': '#555555',
        'stroke-width': '2',
        'rx': '5'
    })
    
    # Header: Components Panel
    ET.SubElement(svg, 'rect', {
        'x': '50', 'y': '80',
        'width': '500', 'height': '40',
        'fill': '#2D2D2D',
        'rx': '5'
    })
    
    panel_title = ET.SubElement(svg, 'text', {
        'x': '60', 'y': '106',
        'fill': COLORS['text'], 'font-family': 'Arial, sans-serif',
        'font-size': '16', 'font-weight': 'bold'
    })
    panel_title.text = '📋 Components Panel'
    
    # Component tree structure
    components = [
        ('BP_DockingBay (Self)', 140, '#5DADE2', 0),
        ('├─ StaticMesh_DockingBay', 180, '#8E44AD', 20),
        ('├─ InteractionTrigger (Sphere)', 220, '#FF8C00', 20),
        ('│   └─ Radius: 1500.0', 260, '#AAAAAA', 40),
        ('├─ DockingPoint_1 (Scene)', 310, '#00FF00', 20),
        ('│   ├─ Location: (200, 0, 100)', 350, '#AAAAAA', 40),
        ('│   └─ Rotation: (0, 0, 0)', 390, '#AAAAAA', 40),
        ('├─ DockingPoint_2 (Scene)', 440, '#00FF00', 20),
        ('│   ├─ Location: (200, 200, 100)', 480, '#AAAAAA', 40),
        ('│   └─ Rotation: (0, 0, 0)', 520, '#AAAAAA', 40),
        ('├─ DockingPoint_3 (Scene)', 570, '#00FF00', 20),
        ('│   ├─ Location: (200, -200, 100)', 610, '#AAAAAA', 40),
        ('│   └─ Rotation: (0, 0, 0)', 650, '#AAAAAA', 40),
        ('└─ DockingPoint_4 (Scene)', 700, '#00FF00', 20)
    ]
    
    for comp_name, y_pos, color, indent in components:
        comp_text = ET.SubElement(svg, 'text', {
            'x': str(70 + indent),
            'y': str(y_pos),
            'fill': color,
            'font-family': 'Courier New, monospace',
            'font-size': '13'
        })
        comp_text.text = comp_name
    
    # Properties panel for Scene Component
    ET.SubElement(svg, 'rect', {
        'x': '600', 'y': '80',
        'width': '550', 'height': '650',
        'fill': '#1A1A1A',
        'stroke': '#555555',
        'stroke-width': '2',
        'rx': '5'
    })
    
    # Header: Details Panel
    ET.SubElement(svg, 'rect', {
        'x': '600', 'y': '80',
        'width': '550', 'height': '40',
        'fill': '#2D2D2D',
        'rx': '5'
    })
    
    details_title = ET.SubElement(svg, 'text', {
        'x': '610', 'y': '106',
        'fill': COLORS['text'], 'font-family': 'Arial, sans-serif',
        'font-size': '16', 'font-weight': 'bold'
    })
    details_title.text = '⚙️ Details: DockingPoint_1 (SceneComponent)'
    
    # Property sections
    sections = [
        ('Transform', 150, [
            ('Location X: 200.0', 180),
            ('Location Y: 0.0', 210),
            ('Location Z: 100.0', 240),
            ('Rotation X: 0.0', 280),
            ('Rotation Y: 0.0', 310),
            ('Rotation Z: 0.0', 340)
        ]),
        ('Sockets', 390, [
            ('Parent Socket: None', 420)
        ]),
        ('Tags', 470, [
            ('Component Tags: [DockingPoint]', 500)
        ]),
        ('Activation', 550, [
            ('Auto Activate: True', 580),
            ('Is Active: True', 610)
        ])
    ]
    
    for section_name, header_y, properties in sections:
        # Section header
        section_header = ET.SubElement(svg, 'text', {
            'x': '620',
            'y': str(header_y),
            'fill': '#FFD700',
            'font-family': 'Arial, sans-serif',
            'font-size': '14',
            'font-weight': 'bold'
        })
        section_header.text = f'▼ {section_name}'
        
        # Properties
        for prop_text, prop_y in properties:
            prop_elem = ET.SubElement(svg, 'text', {
                'x': '640',
                'y': str(prop_y),
                'fill': COLORS['text'],
                'font-family': 'Arial, sans-serif',
                'font-size': '12'
            })
            prop_elem.text = prop_text
    
    # Instructions box at bottom
    ET.SubElement(svg, 'rect', {
        'x': '50', 'y': '750',
        'width': '1100', 'height': '35',
        'fill': '#2D4A2D',
        'stroke': '#4CAF50',
        'stroke-width': '2',
        'rx': '3'
    })
    
    instr_text = ET.SubElement(svg, 'text', {
        'x': '70',
        'y': '773',
        'fill': '#90EE90',
        'font-family': 'Arial, sans-serif',
        'font-size': '13',
        'font-weight': 'bold'
    })
    instr_text.text = '💡 TIP: Add 4-6 Scene Components and space them 200-400 units apart for optimal docking'
    
    return svg

def generate_docking_bay_array_setup():
    """Generate visual guide for configuring DockingPoints array"""
    svg = create_svg_base(1000, 650)
    
    # Title
    title = ET.SubElement(svg, 'text', {
        'x': '500', 'y': '30',
        'fill': COLORS['text'], 'font-family': 'Arial, sans-serif',
        'font-size': '20', 'font-weight': 'bold', 'text-anchor': 'middle'
    })
    title.text = 'Docking Bay - DockingPoints Array Configuration'
    
    # Step 1: Open Class Defaults
    step1_y = 80
    ET.SubElement(svg, 'rect', {
        'x': '50', 'y': str(step1_y),
        'width': '900', 'height': '120',
        'fill': '#1A1A1A',
        'stroke': '#1B4F72',
        'stroke-width': '3',
        'rx': '5'
    })
    
    step1_title = ET.SubElement(svg, 'text', {
        'x': '70', 'y': str(step1_y + 30),
        'fill': '#5DADE2',
        'font-family': 'Arial, sans-serif',
        'font-size': '16',
        'font-weight': 'bold'
    })
    step1_title.text = 'STEP 1: Open Class Defaults'
    
    step1_desc = ET.SubElement(svg, 'text', {
        'x': '70', 'y': str(step1_y + 60),
        'fill': COLORS['text'],
        'font-family': 'Arial, sans-serif',
        'font-size': '13'
    })
    step1_desc.text = 'In BP_DockingBay Blueprint editor, click "Class Defaults" button in toolbar'
    
    ET.SubElement(svg, 'rect', {
        'x': '70', 'y': str(step1_y + 75),
        'width': '200', 'height': '30',
        'fill': '#2D2D2D',
        'stroke': '#555555',
        'stroke-width': '1',
        'rx': '3'
    })
    
    step1_button = ET.SubElement(svg, 'text', {
        'x': '170', 'y': str(step1_y + 95),
        'fill': COLORS['text'],
        'font-family': 'Arial, sans-serif',
        'font-size': '12',
        'text-anchor': 'middle'
    })
    step1_button.text = '📋 Class Defaults'
    
    # Step 2: Find DockingPoints Array
    step2_y = 230
    ET.SubElement(svg, 'rect', {
        'x': '50', 'y': str(step2_y),
        'width': '900', 'height': '370',
        'fill': '#1A1A1A',
        'stroke': '#1B4F72',
        'stroke-width': '3',
        'rx': '5'
    })
    
    step2_title = ET.SubElement(svg, 'text', {
        'x': '70', 'y': str(step2_y + 30),
        'fill': '#5DADE2',
        'font-family': 'Arial, sans-serif',
        'font-size': '16',
        'font-weight': 'bold'
    })
    step2_title.text = 'STEP 2: Configure DockingPoints Array'
    
    step2_desc = ET.SubElement(svg, 'text', {
        'x': '70', 'y': str(step2_y + 60),
        'fill': COLORS['text'],
        'font-family': 'Arial, sans-serif',
        'font-size': '13'
    })
    step2_desc.text = 'Find "Docking Points" array in Details panel and add references to Scene Components'
    
    # Array visualization
    array_y = step2_y + 90
    
    # Array header
    ET.SubElement(svg, 'rect', {
        'x': '100', 'y': str(array_y),
        'width': '800', 'height': '35',
        'fill': '#2D2D2D',
        'stroke': '#FFD700',
        'stroke-width': '2',
        'rx': '3'
    })
    
    array_title_text = ET.SubElement(svg, 'text', {
        'x': '120', 'y': str(array_y + 23),
        'fill': '#FFD700',
        'font-family': 'Arial, sans-serif',
        'font-size': '14',
        'font-weight': 'bold'
    })
    array_title_text.text = '▼ Docking Points [4]'
    
    # Array elements
    array_elements = [
        ('[0] DockingPoint_1', array_y + 50, '#00FF00'),
        ('[1] DockingPoint_2', array_y + 90, '#00FF00'),
        ('[2] DockingPoint_3', array_y + 130, '#00FF00'),
        ('[3] DockingPoint_4', array_y + 170, '#00FF00')
    ]
    
    for elem_text, elem_y, elem_color in array_elements:
        ET.SubElement(svg, 'rect', {
            'x': '150', 'y': str(elem_y),
            'width': '700', 'height': '30',
            'fill': '#1E1E1E',
            'stroke': '#555555',
            'stroke-width': '1',
            'rx': '3'
        })
        
        elem_label = ET.SubElement(svg, 'text', {
            'x': '170', 'y': str(elem_y + 20),
            'fill': elem_color,
            'font-family': 'Courier New, monospace',
            'font-size': '12'
        })
        elem_label.text = elem_text
    
    # Add Element button
    add_btn_y = array_y + 220
    ET.SubElement(svg, 'rect', {
        'x': '150', 'y': str(add_btn_y),
        'width': '150', 'height': '30',
        'fill': '#2D4A2D',
        'stroke': '#4CAF50',
        'stroke-width': '2',
        'rx': '3'
    })
    
    add_btn_text = ET.SubElement(svg, 'text', {
        'x': '225', 'y': str(add_btn_y + 20),
        'fill': '#90EE90',
        'font-family': 'Arial, sans-serif',
        'font-size': '12',
        'text-anchor': 'middle',
        'font-weight': 'bold'
    })
    add_btn_text.text = '+ Add Element'
    
    return svg

def generate_station_docking_module_config():
    """Generate visual guide for adding DockingBay to station"""
    svg = create_svg_base(1100, 750)
    
    # Title
    title = ET.SubElement(svg, 'text', {
        'x': '550', 'y': '30',
        'fill': COLORS['text'], 'font-family': 'Arial, sans-serif',
        'font-size': '20', 'font-weight': 'bold', 'text-anchor': 'middle'
    })
    title.text = 'Space Station - Add Docking Bay Module'
    
    # Description
    desc = ET.SubElement(svg, 'text', {
        'x': '550', 'y': '55',
        'fill': '#AAAAAA',
        'font-family': 'Arial, sans-serif',
        'font-size': '14',
        'text-anchor': 'middle'
    })
    desc.text = 'Configure DefaultModuleClasses to automatically spawn docking capability'
    
    # Main panel
    ET.SubElement(svg, 'rect', {
        'x': '50', 'y': '80',
        'width': '1000', 'height': '620',
        'fill': '#1A1A1A',
        'stroke': '#555555',
        'stroke-width': '2',
        'rx': '5'
    })
    
    # Panel header
    ET.SubElement(svg, 'rect', {
        'x': '50', 'y': '80',
        'width': '1000', 'height': '40',
        'fill': '#2D2D2D',
        'rx': '5'
    })
    
    panel_title = ET.SubElement(svg, 'text', {
        'x': '70', 'y': '106',
        'fill': COLORS['text'],
        'font-family': 'Arial, sans-serif',
        'font-size': '16',
        'font-weight': 'bold'
    })
    panel_title.text = '⚙️ BP_SpaceStation - Class Defaults'
    
    # Section 1: Station Configuration
    section1_y = 150
    section1_header = ET.SubElement(svg, 'text', {
        'x': '80',
        'y': str(section1_y),
        'fill': '#FFD700',
        'font-family': 'Arial, sans-serif',
        'font-size': '15',
        'font-weight': 'bold'
    })
    section1_header.text = '▼ Station → Configuration'
    
    # Station properties
    station_props = [
        ('Station Name: "Trading Hub Alpha"', section1_y + 40),
        ('Max Structural Integrity: 10000.0', section1_y + 70),
        ('Current Structural Integrity: 10000.0', section1_y + 100)
    ]
    
    for prop_text, prop_y in station_props:
        prop_elem = ET.SubElement(svg, 'text', {
            'x': '120',
            'y': str(prop_y),
            'fill': COLORS['text'],
            'font-family': 'Arial, sans-serif',
            'font-size': '13'
        })
        prop_elem.text = prop_text
    
    # Section 2: DefaultModuleClasses Array
    section2_y = 300
    ET.SubElement(svg, 'rect', {
        'x': '80',
        'y': str(section2_y),
        'width': '900',
        'height': '35',
        'fill': '#2D2D2D',
        'stroke': '#FFD700',
        'stroke-width': '2',
        'rx': '3'
    })
    
    section2_title = ET.SubElement(svg, 'text', {
        'x': '100',
        'y': str(section2_y + 23),
        'fill': '#FFD700',
        'font-family': 'Arial, sans-serif',
        'font-size': '15',
        'font-weight': 'bold'
    })
    section2_title.text = '▼ Default Module Classes [3]'
    
    # Array elements
    modules = [
        ('[0] BP_SpaceStationModule_DockingBay', section2_y + 60, '#00FF00', '✓ Required for docking'),
        ('[1] BP_SpaceStationModule_Marketplace', section2_y + 100, '#00FF00', '✓ Required for trading'),
        ('[2] BP_CargoBayModule', section2_y + 140, '#5DADE2', '○ Optional storage')
    ]
    
    for module_text, module_y, module_color, note_text in modules:
        # Module box
        ET.SubElement(svg, 'rect', {
            'x': '120',
            'y': str(module_y),
            'width': '500',
            'height': '30',
            'fill': '#1E1E1E',
            'stroke': '#555555',
            'stroke-width': '1',
            'rx': '3'
        })
        
        # Module text
        module_label = ET.SubElement(svg, 'text', {
            'x': '140',
            'y': str(module_y + 20),
            'fill': module_color,
            'font-family': 'Courier New, monospace',
            'font-size': '12'
        })
        module_label.text = module_text
        
        # Note text
        note_label = ET.SubElement(svg, 'text', {
            'x': '650',
            'y': str(module_y + 20),
            'fill': '#AAAAAA',
            'font-family': 'Arial, sans-serif',
            'font-size': '11',
            'font-style': 'italic'
        })
        note_label.text = note_text
    
    # Add Element button
    add_btn_y = section2_y + 190
    ET.SubElement(svg, 'rect', {
        'x': '120',
        'y': str(add_btn_y),
        'width': '150',
        'height': '30',
        'fill': '#2D4A2D',
        'stroke': '#4CAF50',
        'stroke-width': '2',
        'rx': '3'
    })
    
    add_btn_text = ET.SubElement(svg, 'text', {
        'x': '195',
        'y': str(add_btn_y + 20),
        'fill': '#90EE90',
        'font-family': 'Arial, sans-serif',
        'font-size': '12',
        'text-anchor': 'middle',
        'font-weight': 'bold'
    })
    add_btn_text.text = '+ Add Element'
    
    # Information box
    info_y = 550
    ET.SubElement(svg, 'rect', {
        'x': '80',
        'y': str(info_y),
        'width': '900',
        'height': '120',
        'fill': '#1A2A4A',
        'stroke': '#5DADE2',
        'stroke-width': '2',
        'rx': '5'
    })
    
    info_icon = ET.SubElement(svg, 'text', {
        'x': '100',
        'y': str(info_y + 35),
        'fill': '#5DADE2',
        'font-family': 'Arial, sans-serif',
        'font-size': '24'
    })
    info_icon.text = 'ℹ️'
    
    info_lines = [
        'How it works:',
        '  • Station\'s BeginPlay() spawns all modules from this array',
        '  • Modules automatically attach to station as child actors',
        '  • Each module type provides specific functionality (docking, trading, storage)'
    ]
    
    for i, line in enumerate(info_lines):
        info_text = ET.SubElement(svg, 'text', {
            'x': '140',
            'y': str(info_y + 30 + (i * 25)),
            'fill': COLORS['text'],
            'font-family': 'Arial, sans-serif',
            'font-size': '12'
        })
        info_text.text = line
    
    return svg

def generate_undock_ship_flow():
    """Generate Undock Ship function flow"""
    svg = create_svg_base(950, 550)
    
    # Title
    title = ET.SubElement(svg, 'text', {
        'x': '475', 'y': '30',
        'fill': COLORS['text'], 'font-family': 'Arial, sans-serif',
        'font-size': '18', 'font-weight': 'bold', 'text-anchor': 'middle'
    })
    title.text = 'BP_SpaceStation - Undock Ship Function'
    
    # Function header: Undock Ship
    body_y1 = add_node_box(svg, 50, 60, 380, 100, COLORS['function'], 'FUNCTION: Undock Ship')
    pin_y = body_y1 + 15
    pin_y = add_pin(svg, 50, pin_y, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 50, pin_y, 'object', 'Ship: ASpaceship', is_input=True)
    
    # Function: Find Docking Point
    body_y2 = add_node_box(svg, 480, 60, 400, 100, COLORS['function'], 'Find Ship Docking Point')
    pin_y = body_y2 + 15
    pin_y = add_pin(svg, 480, pin_y, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 480, pin_y, 'object', 'Ship', is_input=True)
    pin_y = body_y2 + 15
    pin_y = add_pin(svg, 880, pin_y, 'exec', '', is_input=False, is_exec=True)
    add_pin(svg, 880, pin_y, 'int', 'Point Index', is_input=False)
    
    # Function: Clear Docking Point
    body_y3 = add_node_box(svg, 50, 210, 380, 100, COLORS['function'], 'Clear Docking Point')
    pin_y = body_y3 + 15
    pin_y = add_pin(svg, 50, pin_y, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 50, pin_y, 'int', 'Point Index', is_input=True)
    pin_y = body_y3 + 15
    add_pin(svg, 430, pin_y, 'exec', '', is_input=False, is_exec=True)
    
    # Function: Move Ship Away
    body_y4 = add_node_box(svg, 480, 210, 400, 100, COLORS['function'], 'Move Ship Away From Station')
    pin_y = body_y4 + 15
    pin_y = add_pin(svg, 480, pin_y, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 480, pin_y, 'object', 'Ship', is_input=True)
    pin_y = body_y4 + 15
    add_pin(svg, 880, pin_y, 'exec', '', is_input=False, is_exec=True)
    
    # Function: Re-enable Ship Controls
    body_y5 = add_node_box(svg, 50, 360, 380, 100, COLORS['function'], 'Enable Ship Input')
    pin_y = body_y5 + 15
    pin_y = add_pin(svg, 50, pin_y, 'exec', 'Exec', is_input=True, is_exec=True)
    add_pin(svg, 50, pin_y, 'object', 'Ship', is_input=True)
    pin_y = body_y5 + 15
    add_pin(svg, 430, pin_y, 'exec', '', is_input=False, is_exec=True)
    
    # Function: Close Trading UI
    body_y6 = add_node_box(svg, 480, 360, 400, 80, COLORS['function'], 'Close Trading UI')
    pin_y = body_y6 + 15
    add_pin(svg, 480, pin_y, 'exec', 'Exec', is_input=True, is_exec=True)
    pin_y = body_y6 + 15
    add_pin(svg, 880, pin_y, 'exec', '', is_input=False, is_exec=True)
    
    # Connection wires
    add_connection_wire(svg, 430, body_y1 + 15, 480, body_y2 + 15)
    add_connection_wire(svg, 880, body_y2 + 15, 50, body_y3 + 15)
    add_connection_wire(svg, 430, body_y3 + 15, 480, body_y4 + 15)
    add_connection_wire(svg, 880, body_y4 + 15, 50, body_y5 + 15)
    add_connection_wire(svg, 430, body_y5 + 15, 480, body_y6 + 15)
    
    # Add labels
    labels = [
        (680, 145, "1. Find docking point"),
        (240, 295, "2. Clear point"),
        (680, 295, "3. Move ship away"),
        (240, 445, "4. Enable controls"),
        (680, 445, "5. Close UI")
    ]
    for x, y, label in labels:
        text = ET.SubElement(svg, 'text', {
            'x': str(x), 'y': str(y),
            'fill': '#CCCCCC', 'font-family': 'Arial, sans-serif',
            'font-size': '11', 'text-anchor': 'middle'
        })
        text.text = label
    
    return svg

def save_svg(svg, filename):
    """Save SVG to file"""
    tree = ET.ElementTree(svg)
    ET.indent(tree, space="  ")
    tree.write(filename, encoding='unicode', xml_declaration=True)
    print(f"Generated: {filename}")

def main():
    """Generate all Blueprint node images"""
    # Determine output directory relative to script location
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    output_dir = os.path.join(project_root, 'docs', 'reference', 'images', 'blueprints')
    os.makedirs(output_dir, exist_ok=True)
    
    # Generate individual node examples
    save_svg(generate_event_node(), f'{output_dir}/event_node.svg')
    save_svg(generate_function_node(), f'{output_dir}/function_node.svg')
    save_svg(generate_foreach_loop(), f'{output_dir}/foreach_loop.svg')
    save_svg(generate_branch_node(), f'{output_dir}/branch_node.svg')
    save_svg(generate_complete_flow(), f'{output_dir}/complete_flow.svg')
    
    # Generate trading system flows
    save_svg(generate_buy_sell_toggle(), f'{output_dir}/buy_sell_toggle.svg')
    save_svg(generate_add_to_cart_flow(), f'{output_dir}/add_to_cart_flow.svg')
    save_svg(generate_execute_trade_flow(), f'{output_dir}/execute_trade_flow.svg')
    save_svg(generate_market_item_display(), f'{output_dir}/market_item_display.svg')
    
    # Generate WBP_TradeItemRow flows (PR #409)
    save_svg(generate_trade_item_row_initialize(), f'{output_dir}/trade_item_row_initialize.svg')
    save_svg(generate_trade_item_row_add_to_cart(), f'{output_dir}/trade_item_row_add_to_cart.svg')
    save_svg(generate_trade_item_row_quantity_increment(), f'{output_dir}/trade_item_row_quantity_increment.svg')
    save_svg(generate_trade_item_row_quantity_decrement(), f'{output_dir}/trade_item_row_quantity_decrement.svg')
    
    # Generate BP_SpaceStation docking flows
    save_svg(generate_docking_range_overlap(), f'{output_dir}/docking_range_overlap.svg')
    save_svg(generate_dock_ship_flow(), f'{output_dir}/dock_ship_flow.svg')
    save_svg(generate_undock_ship_flow(), f'{output_dir}/undock_ship_flow.svg')
    
    # Generate docking setup visual guides (NEW)
    save_svg(generate_docking_point_component_setup(), f'{output_dir}/docking_point_component_setup.svg')
    save_svg(generate_docking_bay_array_setup(), f'{output_dir}/docking_bay_array_setup.svg')
    save_svg(generate_station_docking_module_config(), f'{output_dir}/station_docking_module_config.svg')
    
    print(f"\nGenerated 19 Blueprint node diagrams in {output_dir}/")
    print("\nTrading UI diagrams:")
    print("  - buy_sell_toggle.svg: Buy/Sell mode toggle flow")
    print("  - add_to_cart_flow.svg: Add item to cart validation")
    print("  - execute_trade_flow.svg: Complete trade transaction")
    print("  - market_item_display.svg: Market inventory display population")
    print("\nWBP_TradeItemRow diagrams (PR #409):")
    print("  - trade_item_row_initialize.svg: Row initialization flow")
    print("  - trade_item_row_add_to_cart.svg: Add to cart flow")
    print("  - trade_item_row_quantity_increment.svg: Quantity increment flow")
    print("  - trade_item_row_quantity_decrement.svg: Quantity decrement flow")
    print("\nBP_SpaceStation docking diagrams:")
    print("  - docking_range_overlap.svg: Docking range overlap detection flow")
    print("  - dock_ship_flow.svg: Dock ship function flow")
    print("  - undock_ship_flow.svg: Undock ship function flow")
    print("\nDocking setup visual guides (NEW):")
    print("  - docking_point_component_setup.svg: Scene Component hierarchy and configuration")
    print("  - docking_bay_array_setup.svg: DockingPoints array setup in Class Defaults")
    print("  - station_docking_module_config.svg: Adding DockingBay to station modules")

if __name__ == '__main__':
    main()
