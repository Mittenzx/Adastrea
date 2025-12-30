#!/usr/bin/env python3
"""
Adastrea - Interior Layout and Prop Population Script

This script automates the layout and decoration of interior spaces for space stations,
ships, and other structures. Supports template-based room generation, prop distribution,
and furniture placement with randomization.

Features:
- Generate room layouts (corridors, living quarters, cargo bays, bridge, etc.)
- Distribute props and furniture based on templates
- Support for decoration themes (military, luxury, industrial, civilian)
- Randomized prop placement with collision avoidance
- Navigation mesh compatible layouts
- YAML/CSV template support

Usage in Unreal Editor Python Console:
    import InteriorLayoutPopulator
    InteriorLayoutPopulator.populate_station_interior("DA_Station_Hub", theme="military")
    
    # Or use specific functions
    InteriorLayoutPopulator.create_corridor(start=(0,0,0), end=(1000,0,0))
    InteriorLayoutPopulator.populate_room("cargo_bay", center=(0,0,0), size=(2000,2000,400))

Command Line Usage:
    # Populate from template
    python InteriorLayoutPopulator.py --template station_interior.yaml
    
    # Generate specific room type
    python InteriorLayoutPopulator.py --room bridge --size 3000,3000,400 --theme military
"""

import os
import sys
import csv
import json
import random
import argparse
from pathlib import Path
from typing import Dict, Any, List, Optional, Tuple
from datetime import datetime
import math

# Try to import Unreal Engine modules
try:
    import unreal
    UNREAL_AVAILABLE = True
except ImportError:
    UNREAL_AVAILABLE = False
    print("Note: Running outside Unreal Editor. Some features require Unreal Editor.")

# Try to import PyYAML
try:
    import yaml
    YAML_AVAILABLE = True
except ImportError:
    YAML_AVAILABLE = False
    print("Warning: PyYAML not available. Install with: pip install PyYAML")


class RoomTemplate:
    """Defines templates for different room types"""
    
    ROOM_TYPES = {
        'corridor': {
            'props': ['wall_panel', 'light', 'door', 'vent'],
            'density': 'low',
            'height': 300,
            'description': 'Standard corridor connecting rooms'
        },
        'bridge': {
            'props': ['command_chair', 'console', 'holotable', 'screen', 'terminal'],
            'density': 'high',
            'height': 400,
            'description': 'Ship/station command center'
        },
        'cargo_bay': {
            'props': ['cargo_container', 'crane', 'forklift', 'crate', 'barrel'],
            'density': 'medium',
            'height': 600,
            'description': 'Storage and cargo handling area'
        },
        'living_quarters': {
            'props': ['bed', 'desk', 'chair', 'locker', 'table', 'lamp'],
            'density': 'medium',
            'height': 300,
            'description': 'Crew living space'
        },
        'mess_hall': {
            'props': ['table', 'chair', 'bench', 'counter', 'food_dispenser'],
            'density': 'high',
            'height': 350,
            'description': 'Dining and recreation area'
        },
        'engineering': {
            'props': ['reactor_core', 'control_panel', 'pipe', 'generator', 'tool_bench'],
            'density': 'high',
            'height': 500,
            'description': 'Engineering and maintenance'
        },
        'medbay': {
            'props': ['med_bed', 'medical_scanner', 'cabinet', 'screen', 'table'],
            'density': 'medium',
            'height': 350,
            'description': 'Medical facility'
        },
        'hangar': {
            'props': ['landing_pad', 'fuel_pump', 'tool_rack', 'repair_station'],
            'density': 'low',
            'height': 800,
            'description': 'Vehicle bay and maintenance'
        },
        'armory': {
            'props': ['weapon_rack', 'ammo_crate', 'armor_stand', 'workbench'],
            'density': 'medium',
            'height': 350,
            'description': 'Weapons and equipment storage'
        },
        'laboratory': {
            'props': ['lab_table', 'microscope', 'computer', 'specimen_container', 'storage_unit'],
            'density': 'high',
            'height': 350,
            'description': 'Research and analysis facility'
        }
    }
    
    THEMES = {
        'military': {
            'colors': ['gray', 'dark_green', 'black'],
            'style': 'utilitarian',
            'lighting': 'harsh',
            'props_modifier': ['armor', 'weapon', 'tactical']
        },
        'luxury': {
            'colors': ['gold', 'white', 'cream'],
            'style': 'elegant',
            'lighting': 'warm',
            'props_modifier': ['plush', 'ornate', 'premium']
        },
        'industrial': {
            'colors': ['rust', 'steel', 'orange'],
            'style': 'functional',
            'lighting': 'bright',
            'props_modifier': ['worn', 'heavy_duty', 'rugged']
        },
        'civilian': {
            'colors': ['blue', 'white', 'tan'],
            'style': 'comfortable',
            'lighting': 'soft',
            'props_modifier': ['clean', 'modern', 'efficient']
        },
        'alien': {
            'colors': ['purple', 'green', 'cyan'],
            'style': 'organic',
            'lighting': 'bioluminescent',
            'props_modifier': ['exotic', 'strange', 'advanced']
        }
    }


class PropPlacementRule:
    """Rules for placing props in rooms"""
    
    @staticmethod
    def place_along_walls(room_bounds: Tuple[float, float, float, float],
                         count: int, offset: float = 100) -> List[Tuple[float, float, float]]:
        """
        Generate positions along room walls
        
        Args:
            room_bounds: (min_x, min_y, max_x, max_y)
            count: Number of positions
            offset: Distance from wall
            
        Returns:
            List of (x, y, z) positions
        """
        min_x, min_y, max_x, max_y = room_bounds
        positions = []
        
        # Distribute along perimeter
        perimeter = 2 * ((max_x - min_x) + (max_y - min_y))
        spacing = perimeter / count if count > 0 else 0
        
        current_dist = 0
        for i in range(count):
            # Determine which wall and position
            if current_dist < (max_x - min_x):
                # Bottom wall
                x = min_x + current_dist
                y = min_y + offset
            elif current_dist < (max_x - min_x) + (max_y - min_y):
                # Right wall
                x = max_x - offset
                y = min_y + (current_dist - (max_x - min_x))
            elif current_dist < 2 * (max_x - min_x) + (max_y - min_y):
                # Top wall
                x = max_x - (current_dist - (max_x - min_x) - (max_y - min_y))
                y = max_y - offset
            else:
                # Left wall
                x = min_x + offset
                y = max_y - (current_dist - 2 * (max_x - min_x) - (max_y - min_y))
            
            positions.append((x, y, 0))
            current_dist += spacing
        
        return positions
    
    @staticmethod
    def place_grid_in_room(room_bounds: Tuple[float, float, float, float],
                          count: int, margin: float = 200) -> List[Tuple[float, float, float]]:
        """
        Generate grid positions inside room
        
        Args:
            room_bounds: (min_x, min_y, max_x, max_y)
            count: Number of positions
            margin: Margin from walls
            
        Returns:
            List of (x, y, z) positions
        """
        min_x, min_y, max_x, max_y = room_bounds
        
        # Calculate usable area
        usable_x = max_x - min_x - 2 * margin
        usable_y = max_y - min_y - 2 * margin
        
        # Calculate grid size
        grid_size = math.ceil(math.sqrt(count))
        spacing_x = usable_x / max(grid_size, 1)
        spacing_y = usable_y / max(grid_size, 1)
        
        positions = []
        for i in range(count):
            row = i // grid_size
            col = i % grid_size
            
            x = min_x + margin + col * spacing_x + spacing_x / 2
            y = min_y + margin + row * spacing_y + spacing_y / 2
            
            positions.append((x, y, 0))
        
        return positions
    
    @staticmethod
    def place_random_in_room(room_bounds: Tuple[float, float, float, float],
                            count: int, margin: float = 200,
                            min_spacing: float = 100) -> List[Tuple[float, float, float]]:
        """
        Generate random positions with collision avoidance
        
        Args:
            room_bounds: (min_x, min_y, max_x, max_y)
            count: Number of positions
            margin: Margin from walls
            min_spacing: Minimum distance between props
            
        Returns:
            List of (x, y, z) positions
        """
        min_x, min_y, max_x, max_y = room_bounds
        positions = []
        
        max_attempts = count * 10
        attempts = 0
        
        while len(positions) < count and attempts < max_attempts:
            # Generate random position
            x = random.uniform(min_x + margin, max_x - margin)
            y = random.uniform(min_y + margin, max_y - margin)
            
            # Check spacing from existing positions
            valid = True
            for ex_x, ex_y, ex_z in positions:
                dist = math.sqrt((x - ex_x)**2 + (y - ex_y)**2)
                if dist < min_spacing:
                    valid = False
                    break
            
            if valid:
                positions.append((x, y, 0))
            
            attempts += 1
        
        return positions
    
    @staticmethod
    def place_centered(room_bounds: Tuple[float, float, float, float]) -> Tuple[float, float, float]:
        """
        Get center position of room
        
        Args:
            room_bounds: (min_x, min_y, max_x, max_y)
            
        Returns:
            (x, y, z) center position
        """
        min_x, min_y, max_x, max_y = room_bounds
        return ((min_x + max_x) / 2, (min_y + max_y) / 2, 0)


class InteriorLayoutPopulator:
    """Main class for populating interior layouts"""
    
    def __init__(self):
        """Initialize the interior populator"""
        if not UNREAL_AVAILABLE:
            print("Warning: Unreal Engine Python API not available. Running in limited mode.")
            self.editor_actor_subsystem = None
            self.editor_level_library = None
        else:
            self.editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
            self.editor_level_library = unreal.EditorLevelLibrary()
        
        self.project_root = Path.cwd() if not UNREAL_AVAILABLE else Path(unreal.SystemLibrary.get_project_directory())
        self.spawned_props = []
        
        # Default prop asset paths (can be customized)
        self.prop_library = {
            'wall_panel': '/Game/Props/Architecture/SM_WallPanel',
            'light': '/Game/Props/Lighting/SM_CeilingLight',
            'door': '/Game/Props/Architecture/SM_Door',
            'vent': '/Game/Props/Architecture/SM_Vent',
            'command_chair': '/Game/Props/Furniture/SM_CommandChair',
            'console': '/Game/Props/Tech/SM_Console',
            'holotable': '/Game/Props/Tech/SM_HoloTable',
            'screen': '/Game/Props/Tech/SM_Screen',
            'terminal': '/Game/Props/Tech/SM_Terminal',
            'cargo_container': '/Game/Props/Storage/SM_CargoContainer',
            'crane': '/Game/Props/Industrial/SM_Crane',
            'crate': '/Game/Props/Storage/SM_Crate',
            'barrel': '/Game/Props/Storage/SM_Barrel',
            'bed': '/Game/Props/Furniture/SM_Bed',
            'desk': '/Game/Props/Furniture/SM_Desk',
            'chair': '/Game/Props/Furniture/SM_Chair',
            'locker': '/Game/Props/Storage/SM_Locker',
            'table': '/Game/Props/Furniture/SM_Table',
            'lamp': '/Game/Props/Lighting/SM_Lamp'
        }
    
    def log(self, message: str, level: str = "info"):
        """Log message"""
        if UNREAL_AVAILABLE:
            if level == "error":
                unreal.log_error(f"[InteriorPopulator] {message}")
            elif level == "warning":
                unreal.log_warning(f"[InteriorPopulator] {message}")
            else:
                unreal.log(f"[InteriorPopulator] {message}")
        else:
            prefix = {"error": "ERROR", "warning": "WARNING", "info": "INFO"}
            print(f"[{prefix.get(level, 'INFO')}] {message}")
    
    def create_room(self, room_type: str, center: Tuple[float, float, float],
                   size: Tuple[float, float, float], theme: str = 'military',
                   prop_density: float = 1.0) -> List[Any]:
        """
        Create a complete room with props
        
        Args:
            room_type: Type of room (bridge, cargo_bay, etc.)
            center: (x, y, z) center position
            size: (width, length, height) room dimensions
            theme: Theme name (military, luxury, etc.)
            prop_density: Multiplier for prop count (0.5-2.0)
            
        Returns:
            List of spawned props
        """
        if room_type not in RoomTemplate.ROOM_TYPES:
            self.log(f"Unknown room type: {room_type}", "error")
            return []
        
        self.log(f"Creating {room_type} room at {center} with size {size}")
        
        template = RoomTemplate.ROOM_TYPES[room_type]
        props_list = template['props']
        base_density = {'low': 0.5, 'medium': 1.0, 'high': 1.5}.get(template['density'], 1.0)
        
        # Calculate room bounds
        width, length, height = size
        min_x = center[0] - width / 2
        max_x = center[0] + width / 2
        min_y = center[1] - length / 2
        max_y = center[1] + length / 2
        room_bounds = (min_x, min_y, max_x, max_y)
        
        # Calculate number of props
        room_area = width * length
        base_count = int((room_area / 1000000) * 10 * base_density * prop_density)
        
        spawned = []
        
        # Place props based on room type
        if room_type == 'bridge':
            # Central command chair
            spawned.extend(self._place_props(['command_chair'], [PropPlacementRule.place_centered(room_bounds)], center[2]))
            
            # Consoles around perimeter
            console_positions = PropPlacementRule.place_along_walls(room_bounds, 8, offset=300)
            spawned.extend(self._place_props(['console'] * 8, console_positions, center[2]))
            
        elif room_type == 'cargo_bay':
            # Grid of cargo containers
            container_count = max(5, base_count // 2)
            container_positions = PropPlacementRule.place_grid_in_room(room_bounds, container_count, margin=500)
            spawned.extend(self._place_props(['cargo_container'] * container_count, container_positions, center[2]))
            
        elif room_type == 'living_quarters':
            # Bed against wall
            bed_positions = PropPlacementRule.place_along_walls(room_bounds, 1, offset=200)
            spawned.extend(self._place_props(['bed'], bed_positions, center[2]))
            
            # Furniture scattered
            furniture_count = 3
            furniture_positions = PropPlacementRule.place_random_in_room(room_bounds, furniture_count, min_spacing=300)
            spawned.extend(self._place_props(['desk', 'chair', 'locker'], furniture_positions, center[2]))
            
        else:
            # Generic placement
            prop_count = max(3, base_count)
            positions = PropPlacementRule.place_random_in_room(room_bounds, prop_count, min_spacing=200)
            
            # Select random props from template
            selected_props = [random.choice(props_list) for _ in range(prop_count)]
            spawned.extend(self._place_props(selected_props, positions, center[2]))
        
        self.log(f"Created {room_type} with {len(spawned)} props")
        return spawned
    
    def _place_props(self, prop_types: List[str], positions: List[Tuple[float, float, float]],
                    base_z: float) -> List[Any]:
        """Place props at specified positions"""
        spawned = []
        
        for prop_type, pos in zip(prop_types, positions):
            # Get asset path
            asset_path = self.prop_library.get(prop_type)
            if not asset_path:
                self.log(f"No asset path for prop type: {prop_type}", "warning")
                continue
            
            # Spawn prop
            location = (pos[0], pos[1], base_z + pos[2])
            rotation = (0, random.uniform(0, 360), 0)  # Random yaw
            
            if UNREAL_AVAILABLE:
                try:
                    asset = unreal.load_asset(asset_path)
                    if asset:
                        transform = unreal.Transform(
                            location=unreal.Vector(*location),
                            rotation=unreal.Rotator(*rotation),
                            scale=unreal.Vector(1, 1, 1)
                        )
                        
                        actor = self.editor_actor_subsystem.spawn_actor_from_object(
                            asset, transform.translation, transform.rotation
                        )
                        
                        if actor:
                            spawned.append(actor)
                            self.spawned_props.append(actor)
                except Exception as e:
                    self.log(f"Error spawning prop {prop_type}: {e}", "warning")
            else:
                self.log(f"Would spawn {prop_type} at {location}", "info")
        
        return spawned
    
    def create_corridor(self, start: Tuple[float, float, float],
                       end: Tuple[float, float, float],
                       width: float = 300, height: float = 300,
                       theme: str = 'military') -> List[Any]:
        """
        Create a corridor between two points
        
        Args:
            start: Starting (x, y, z) position
            end: Ending (x, y, z) position
            width: Corridor width
            height: Corridor height
            theme: Theme name
            
        Returns:
            List of spawned props
        """
        self.log(f"Creating corridor from {start} to {end}")
        
        # Calculate corridor length and direction
        dx = end[0] - start[0]
        dy = end[1] - start[1]
        length = math.sqrt(dx**2 + dy**2)
        
        # Place lights and panels along corridor
        segment_length = 500  # Place prop every 500 units
        num_segments = max(1, int(length / segment_length))
        
        spawned = []
        for i in range(num_segments + 1):
            t = i / max(num_segments, 1)
            x = start[0] + t * dx
            y = start[1] + t * dy
            z = start[2]
            
            # Alternate lights and panels
            prop_type = 'light' if i % 2 == 0 else 'wall_panel'
            positions = [(x, y, z)]
            spawned.extend(self._place_props([prop_type], positions, z))
        
        self.log(f"Created corridor with {len(spawned)} props")
        return spawned
    
    def populate_from_template(self, template_path: str) -> bool:
        """
        Populate interior from template file
        
        Args:
            template_path: Path to YAML template file
            
        Returns:
            True if successful
        """
        if not YAML_AVAILABLE:
            self.log("PyYAML not available", "error")
            return False
        
        try:
            template_file = Path(template_path)
            if not template_file.exists():
                self.log(f"Template not found: {template_path}", "error")
                return False
            
            with open(template_file, 'r', encoding='utf-8') as f:
                template = yaml.safe_load(f)
            
            # Process rooms
            rooms = template.get('rooms', [])
            for room in rooms:
                room_type = room.get('type')
                center = tuple(room.get('center', [0, 0, 0]))
                size = tuple(room.get('size', [1000, 1000, 300]))
                theme = room.get('theme', 'military')
                density = room.get('prop_density', 1.0)
                
                self.create_room(room_type, center, size, theme, density)
            
            # Process corridors
            corridors = template.get('corridors', [])
            for corridor in corridors:
                start = tuple(corridor.get('start', [0, 0, 0]))
                end = tuple(corridor.get('end', [1000, 0, 0]))
                width = corridor.get('width', 300)
                height = corridor.get('height', 300)
                theme = corridor.get('theme', 'military')
                
                self.create_corridor(start, end, width, height, theme)
            
            self.log(f"Populated {len(rooms)} rooms and {len(corridors)} corridors")
            return True
            
        except Exception as e:
            self.log(f"Error loading template: {e}", "error")
            return False
    
    def clear_all_props(self):
        """Remove all spawned props"""
        if not UNREAL_AVAILABLE:
            self.log("Cannot clear props - not in Unreal Editor", "warning")
            return
        
        count = len(self.spawned_props)
        for prop in self.spawned_props:
            try:
                self.editor_actor_subsystem.destroy_actor(prop)
            except Exception as e:
                self.log(f"Error destroying prop: {e}", "warning")
        
        self.spawned_props.clear()
        self.log(f"Cleared {count} props")


# Module-level convenience functions
def populate_room(room_type: str, center: Tuple[float, float, float] = (0, 0, 0),
                 size: Tuple[float, float, float] = (2000, 2000, 400),
                 theme: str = 'military') -> List[Any]:
    """Convenience function to populate a room"""
    populator = InteriorLayoutPopulator()
    return populator.create_room(room_type, center, size, theme)


def create_corridor(start: Tuple[float, float, float], end: Tuple[float, float, float],
                   width: float = 300, theme: str = 'military') -> List[Any]:
    """Convenience function to create a corridor"""
    populator = InteriorLayoutPopulator()
    return populator.create_corridor(start, end, width, 300, theme)


def populate_station_interior(station_type: str = "hub", theme: str = "military") -> bool:
    """
    Convenience function to populate a complete station interior
    
    Args:
        station_type: Type of station (hub, outpost, etc.)
        theme: Theme for decoration
        
    Returns:
        True if successful
    """
    populator = InteriorLayoutPopulator()
    
    if station_type == "hub":
        # Create main bridge
        populator.create_room('bridge', (0, 0, 100), (3000, 3000, 400), theme)
        
        # Create corridors to other areas
        populator.create_corridor((1500, 0, 100), (5000, 0, 100), theme=theme)
        populator.create_corridor((-1500, 0, 100), (-5000, 0, 100), theme=theme)
        
        # Create cargo bay
        populator.create_room('cargo_bay', (5000, 0, 100), (4000, 4000, 600), theme)
        
        # Create living quarters
        populator.create_room('living_quarters', (-5000, 0, 100), (2000, 2000, 300), theme)
        
        return True
    
    return False


def main():
    """Main function for command-line usage"""
    parser = argparse.ArgumentParser(
        description='Adastrea Interior Layout Populator',
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    
    parser.add_argument('--template', type=str, help='Path to YAML template file')
    parser.add_argument('--room', type=str, choices=list(RoomTemplate.ROOM_TYPES.keys()),
                       help='Room type to create')
    parser.add_argument('--size', type=str, default='2000,2000,400',
                       help='Room size as width,length,height')
    parser.add_argument('--theme', type=str, default='military',
                       choices=list(RoomTemplate.THEMES.keys()), help='Theme')
    
    args = parser.parse_args()
    
    populator = InteriorLayoutPopulator()
    
    if args.template:
        success = populator.populate_from_template(args.template)
        sys.exit(0 if success else 1)
    
    elif args.room:
        size = tuple(map(float, args.size.split(',')))
        populator.create_room(args.room, (0, 0, 0), size, args.theme)
        sys.exit(0)
    
    else:
        parser.print_help()
        sys.exit(1)


if __name__ == "__main__":
    main()
