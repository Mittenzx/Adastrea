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
    
    # Determine text color based on header background color
    # Use black text for light backgrounds (white, light orange, light blue, etc.)
    light_colors = ['#FFFFFF', '#FFB6C1', '#90EE90', '#FFD700', '#00FFFF', '#5DADE2']
    text_color = '#000000' if color.upper() in [c.upper() for c in light_colors] else COLORS['text']
    
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
    
    print(f"\nGenerated 13 Blueprint node diagrams in {output_dir}/")
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

if __name__ == '__main__':
    main()
