#!/usr/bin/env python3
"""
Adastrea - Scene and Asset Population Script

This script provides automated level population for Unreal Editor, spawning actors
based on data asset records and configuration files. Supports batch placement,
randomization, and customization via YAML/CSV input.

Features:
- Populate levels with spaceships, stations, NPCs, and planets
- Support multiple placement patterns (grid, random, cluster, orbit, spiral)
- Load actor configurations from YAML/CSV files
- Batch placement with customizable parameters
- Integration with existing Data Assets
- Command-line and Editor Utility Widget support

Usage in Unreal Editor Python Console:
    import ScenePopulator
    ScenePopulator.populate_from_config("population_config.yaml")
    
    # Or use specific functions
    ScenePopulator.spawn_actors_grid("DA_Ship_Fighter", count=10, spacing=5000)
    ScenePopulator.spawn_station_with_modules("DA_Station_Hub", module_count=8)

Command Line Usage:
    # Populate from config file
    python ScenePopulator.py --config scene_config.yaml
    
    # Quick spawn with pattern
    python ScenePopulator.py --pattern grid --asset-path /Game/Ships/DA_Ship_Fighter --count 10
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

# Try to import Unreal Engine modules (will be available in Editor)
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


class PlacementPattern:
    """Defines different patterns for placing actors in a level"""
    
    @staticmethod
    def grid(count: int, spacing: float, center: Tuple[float, float, float] = (0, 0, 0)) -> List[Tuple[float, float, float]]:
        """
        Generate positions in a grid pattern
        
        Args:
            count: Number of positions to generate
            spacing: Distance between positions
            center: Center point of the grid
            
        Returns:
            List of (x, y, z) positions
        """
        positions = []
        grid_size = math.ceil(math.sqrt(count))
        
        for i in range(count):
            row = i // grid_size
            col = i % grid_size
            
            x = center[0] + (col - grid_size / 2) * spacing
            y = center[1] + (row - grid_size / 2) * spacing
            z = center[2]
            
            positions.append((x, y, z))
        
        return positions
    
    @staticmethod
    def random_sphere(count: int, radius: float, center: Tuple[float, float, float] = (0, 0, 0)) -> List[Tuple[float, float, float]]:
        """
        Generate random positions within a sphere
        
        Args:
            count: Number of positions to generate
            radius: Radius of the sphere
            center: Center point of the sphere
            
        Returns:
            List of (x, y, z) positions
        """
        positions = []
        
        for _ in range(count):
            # Generate random point in sphere using rejection sampling
            while True:
                x = random.uniform(-1, 1) * radius
                y = random.uniform(-1, 1) * radius
                z = random.uniform(-1, 1) * radius
                
                if x*x + y*y + z*z <= radius*radius:
                    positions.append((
                        center[0] + x,
                        center[1] + y,
                        center[2] + z
                    ))
                    break
        
        return positions
    
    @staticmethod
    def cluster(count: int, cluster_count: int, cluster_radius: float, 
                spread: float, center: Tuple[float, float, float] = (0, 0, 0)) -> List[Tuple[float, float, float]]:
        """
        Generate positions in multiple clusters
        
        Args:
            count: Total number of positions
            cluster_count: Number of clusters
            cluster_radius: Radius of each cluster
            spread: Distance between cluster centers
            center: Overall center point
            
        Returns:
            List of (x, y, z) positions
        """
        positions = []
        items_per_cluster = count // cluster_count
        remainder = count % cluster_count
        
        # Generate cluster centers
        cluster_centers = PlacementPattern.random_sphere(cluster_count, spread, center)
        
        # Populate each cluster
        for i, cluster_center in enumerate(cluster_centers):
            cluster_size = items_per_cluster + (1 if i < remainder else 0)
            cluster_positions = PlacementPattern.random_sphere(cluster_size, cluster_radius, cluster_center)
            positions.extend(cluster_positions)
        
        return positions
    
    @staticmethod
    def orbit(count: int, radius: float, center: Tuple[float, float, float] = (0, 0, 0),
              z_variation: float = 0) -> List[Tuple[float, float, float]]:
        """
        Generate positions in circular orbit pattern
        
        Args:
            count: Number of positions
            radius: Orbital radius
            center: Center point
            z_variation: Random variation in Z axis
            
        Returns:
            List of (x, y, z) positions
        """
        positions = []
        angle_step = 360 / count if count > 0 else 0
        
        for i in range(count):
            angle = math.radians(i * angle_step)
            x = center[0] + radius * math.cos(angle)
            y = center[1] + radius * math.sin(angle)
            z = center[2] + random.uniform(-z_variation, z_variation)
            
            positions.append((x, y, z))
        
        return positions
    
    @staticmethod
    def spiral(count: int, radius: float, height: float, 
               center: Tuple[float, float, float] = (0, 0, 0), turns: float = 3) -> List[Tuple[float, float, float]]:
        """
        Generate positions in spiral pattern
        
        Args:
            count: Number of positions
            radius: Maximum radius
            height: Total height of spiral
            center: Center point
            turns: Number of complete turns
            
        Returns:
            List of (x, y, z) positions
        """
        positions = []
        
        for i in range(count):
            t = i / max(count - 1, 1)  # Parameter from 0 to 1
            angle = t * turns * 2 * math.pi
            r = t * radius
            
            x = center[0] + r * math.cos(angle)
            y = center[1] + r * math.sin(angle)
            z = center[2] + t * height
            
            positions.append((x, y, z))
        
        return positions
    
    @staticmethod
    def line(count: int, spacing: float, start: Tuple[float, float, float] = (0, 0, 0),
             direction: Tuple[float, float, float] = (1, 0, 0)) -> List[Tuple[float, float, float]]:
        """
        Generate positions in a line
        
        Args:
            count: Number of positions
            spacing: Distance between positions
            start: Starting point
            direction: Direction vector (will be normalized)
            
        Returns:
            List of (x, y, z) positions
        """
        # Normalize direction
        length = math.sqrt(direction[0]**2 + direction[1]**2 + direction[2]**2)
        if length > 0:
            dir_norm = (direction[0]/length, direction[1]/length, direction[2]/length)
        else:
            dir_norm = (1, 0, 0)
        
        positions = []
        for i in range(count):
            x = start[0] + i * spacing * dir_norm[0]
            y = start[1] + i * spacing * dir_norm[1]
            z = start[2] + i * spacing * dir_norm[2]
            positions.append((x, y, z))
        
        return positions


class ScenePopulator:
    """Main class for populating Unreal levels with actors"""
    
    def __init__(self):
        """Initialize the scene populator"""
        if not UNREAL_AVAILABLE:
            print("Warning: Unreal Engine Python API not available. Running in limited mode.")
            self.editor_actor_subsystem = None
            self.editor_level_library = None
            self.level = None
        else:
            # Get editor subsystems
            self.editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
            self.editor_level_library = unreal.EditorLevelLibrary()
            self.level = self.editor_level_library.get_editor_world()
        
        # Configuration
        self.project_root = Path.cwd() if not UNREAL_AVAILABLE else Path(unreal.SystemLibrary.get_project_directory())
        self.spawned_actors = []
    
    def log(self, message: str, level: str = "info"):
        """
        Log message to console or Unreal Editor
        
        Args:
            message: Message to log
            level: Log level (info, warning, error)
        """
        if UNREAL_AVAILABLE:
            if level == "error":
                unreal.log_error(f"[ScenePopulator] {message}")
            elif level == "warning":
                unreal.log_warning(f"[ScenePopulator] {message}")
            else:
                unreal.log(f"[ScenePopulator] {message}")
        else:
            prefix = {"error": "ERROR", "warning": "WARNING", "info": "INFO"}
            print(f"[{prefix.get(level, 'INFO')}] {message}")
    
    def spawn_actor(self, asset_path: str, location: Tuple[float, float, float],
                   rotation: Tuple[float, float, float] = (0, 0, 0),
                   scale: Tuple[float, float, float] = (1, 1, 1)) -> Optional[Any]:
        """
        Spawn a single actor in the level
        
        Args:
            asset_path: Path to the asset (e.g., "/Game/Blueprints/BP_Ship")
            location: (x, y, z) world location
            rotation: (pitch, yaw, roll) in degrees
            scale: (x, y, z) scale
            
        Returns:
            Spawned actor or None if failed
        """
        if not UNREAL_AVAILABLE:
            self.log(f"Would spawn {asset_path} at {location}", "info")
            return None
        
        try:
            # Load the asset
            asset = unreal.load_asset(asset_path)
            if not asset:
                self.log(f"Failed to load asset: {asset_path}", "error")
                return None
            
            # Create transform
            transform = unreal.Transform(
                location=unreal.Vector(*location),
                rotation=unreal.Rotator(*rotation),
                scale=unreal.Vector(*scale)
            )
            
            # Spawn actor
            actor = self.editor_actor_subsystem.spawn_actor_from_object(
                asset,
                transform.translation,
                transform.rotation
            )
            
            if actor:
                # Set scale
                actor.set_actor_scale3d(transform.scale)
                self.spawned_actors.append(actor)
                self.log(f"Spawned actor: {asset_path} at {location}")
                return actor
            else:
                self.log(f"Failed to spawn actor: {asset_path}", "error")
                return None
                
        except Exception as e:
            self.log(f"Error spawning actor {asset_path}: {e}", "error")
            return None
    
    def spawn_actors_pattern(self, asset_path: str, pattern: str, count: int,
                            pattern_params: Dict[str, Any] = None,
                            rotation_randomness: float = 0,
                            scale_randomness: float = 0) -> List[Any]:
        """
        Spawn multiple actors using a placement pattern
        
        Args:
            asset_path: Path to the asset
            pattern: Pattern name (grid, random, cluster, orbit, spiral, line)
            count: Number of actors to spawn
            pattern_params: Parameters for the pattern
            rotation_randomness: Random rotation variation in degrees
            scale_randomness: Random scale variation (0-1)
            
        Returns:
            List of spawned actors
        """
        if pattern_params is None:
            pattern_params = {}
        
        # Generate positions based on pattern
        pattern_funcs = {
            'grid': PlacementPattern.grid,
            'random': PlacementPattern.random_sphere,
            'cluster': PlacementPattern.cluster,
            'orbit': PlacementPattern.orbit,
            'spiral': PlacementPattern.spiral,
            'line': PlacementPattern.line
        }
        
        if pattern not in pattern_funcs:
            self.log(f"Unknown pattern: {pattern}", "error")
            return []
        
        # Set default parameters based on pattern
        if pattern == 'grid':
            pattern_params.setdefault('spacing', 5000)
            pattern_params.setdefault('center', (0, 0, 0))
        elif pattern == 'random':
            pattern_params.setdefault('radius', 10000)
            pattern_params.setdefault('center', (0, 0, 0))
        elif pattern == 'cluster':
            pattern_params.setdefault('cluster_count', 3)
            pattern_params.setdefault('cluster_radius', 2000)
            pattern_params.setdefault('spread', 10000)
            pattern_params.setdefault('center', (0, 0, 0))
        elif pattern == 'orbit':
            pattern_params.setdefault('radius', 10000)
            pattern_params.setdefault('center', (0, 0, 0))
            pattern_params.setdefault('z_variation', 0)
        elif pattern == 'spiral':
            pattern_params.setdefault('radius', 10000)
            pattern_params.setdefault('height', 5000)
            pattern_params.setdefault('center', (0, 0, 0))
            pattern_params.setdefault('turns', 3)
        elif pattern == 'line':
            pattern_params.setdefault('spacing', 5000)
            pattern_params.setdefault('start', (0, 0, 0))
            pattern_params.setdefault('direction', (1, 0, 0))
        
        # Generate positions
        positions = pattern_funcs[pattern](count, **pattern_params)
        
        # Spawn actors at each position
        spawned = []
        for i, pos in enumerate(positions):
            # Add rotation randomness
            rotation = (
                random.uniform(-rotation_randomness, rotation_randomness),
                random.uniform(-rotation_randomness, rotation_randomness),
                random.uniform(-rotation_randomness, rotation_randomness)
            )
            
            # Add scale randomness
            scale_factor = 1 + random.uniform(-scale_randomness, scale_randomness)
            scale = (scale_factor, scale_factor, scale_factor)
            
            actor = self.spawn_actor(asset_path, pos, rotation, scale)
            if actor:
                spawned.append(actor)
        
        self.log(f"Spawned {len(spawned)}/{count} actors using {pattern} pattern")
        return spawned
    
    def load_config_yaml(self, config_path: str) -> Optional[Dict[str, Any]]:
        """
        Load population configuration from YAML file
        
        Args:
            config_path: Path to YAML configuration file
            
        Returns:
            Configuration dictionary or None if failed
        """
        if not YAML_AVAILABLE:
            self.log("PyYAML not available. Cannot load YAML config.", "error")
            return None
        
        try:
            config_file = Path(config_path)
            if not config_file.exists():
                self.log(f"Config file not found: {config_path}", "error")
                return None
            
            with open(config_file, 'r', encoding='utf-8') as f:
                config = yaml.safe_load(f)
            
            self.log(f"Loaded configuration from {config_path}")
            return config
        except Exception as e:
            self.log(f"Error loading YAML config: {e}", "error")
            return None
    
    def load_config_csv(self, csv_path: str) -> Optional[List[Dict[str, Any]]]:
        """
        Load population configuration from CSV file
        
        Args:
            csv_path: Path to CSV configuration file
            
        Returns:
            List of spawn configurations or None if failed
        """
        try:
            csv_file = Path(csv_path)
            if not csv_file.exists():
                self.log(f"CSV file not found: {csv_path}", "error")
                return None
            
            configs = []
            with open(csv_file, 'r', encoding='utf-8') as f:
                reader = csv.DictReader(f)
                for row in reader:
                    # Convert string values to appropriate types
                    config = {}
                    for key, value in row.items():
                        # Try to convert to number
                        try:
                            if '.' in value:
                                config[key] = float(value)
                            else:
                                config[key] = int(value)
                        except ValueError:
                            # Keep as string
                            config[key] = value
                    configs.append(config)
            
            self.log(f"Loaded {len(configs)} configurations from {csv_path}")
            return configs
        except Exception as e:
            self.log(f"Error loading CSV config: {e}", "error")
            return None
    
    def populate_from_config(self, config_path: str) -> bool:
        """
        Populate level from configuration file (YAML or CSV)
        
        Args:
            config_path: Path to configuration file
            
        Returns:
            True if successful, False otherwise
        """
        config_file = Path(config_path)
        
        if config_file.suffix.lower() == '.yaml' or config_file.suffix.lower() == '.yml':
            config = self.load_config_yaml(config_path)
            if not config:
                return False
            
            # Process YAML configuration
            return self._process_yaml_config(config)
        
        elif config_file.suffix.lower() == '.csv':
            configs = self.load_config_csv(config_path)
            if not configs:
                return False
            
            # Process CSV configuration
            return self._process_csv_config(configs)
        
        else:
            self.log(f"Unsupported config file format: {config_file.suffix}", "error")
            return False
    
    def _process_yaml_config(self, config: Dict[str, Any]) -> bool:
        """Process YAML configuration and spawn actors"""
        try:
            spawn_groups = config.get('spawn_groups', [])
            
            for group in spawn_groups:
                asset_path = group.get('asset_path')
                pattern = group.get('pattern', 'grid')
                count = group.get('count', 1)
                pattern_params = group.get('pattern_params', {})
                rotation_randomness = group.get('rotation_randomness', 0)
                scale_randomness = group.get('scale_randomness', 0)
                
                if not asset_path:
                    self.log("Spawn group missing asset_path", "warning")
                    continue
                
                self.spawn_actors_pattern(
                    asset_path,
                    pattern,
                    count,
                    pattern_params,
                    rotation_randomness,
                    scale_randomness
                )
            
            self.log(f"Completed spawning {len(spawn_groups)} groups")
            return True
            
        except Exception as e:
            self.log(f"Error processing YAML config: {e}", "error")
            return False
    
    def _process_csv_config(self, configs: List[Dict[str, Any]]) -> bool:
        """Process CSV configuration and spawn actors"""
        try:
            for config in configs:
                asset_path = config.get('asset_path')
                x = config.get('x', 0)
                y = config.get('y', 0)
                z = config.get('z', 0)
                pitch = config.get('pitch', 0)
                yaw = config.get('yaw', 0)
                roll = config.get('roll', 0)
                scale_x = config.get('scale_x', 1)
                scale_y = config.get('scale_y', 1)
                scale_z = config.get('scale_z', 1)
                
                if not asset_path:
                    self.log("CSV row missing asset_path", "warning")
                    continue
                
                self.spawn_actor(
                    asset_path,
                    (x, y, z),
                    (pitch, yaw, roll),
                    (scale_x, scale_y, scale_z)
                )
            
            self.log(f"Completed spawning {len(configs)} actors from CSV")
            return True
            
        except Exception as e:
            self.log(f"Error processing CSV config: {e}", "error")
            return False
    
    def clear_spawned_actors(self):
        """Remove all actors spawned by this populator"""
        if not UNREAL_AVAILABLE:
            self.log("Cannot clear actors - not in Unreal Editor", "warning")
            return
        
        count = len(self.spawned_actors)
        for actor in self.spawned_actors:
            try:
                self.editor_actor_subsystem.destroy_actor(actor)
            except Exception as e:
                self.log(f"Error destroying actor: {e}", "warning")
        
        self.spawned_actors.clear()
        self.log(f"Cleared {count} spawned actors")
    
    def export_spawned_to_csv(self, output_path: str) -> bool:
        """
        Export spawned actors to CSV file
        
        Args:
            output_path: Path to output CSV file
            
        Returns:
            True if successful, False otherwise
        """
        if not UNREAL_AVAILABLE:
            self.log("Cannot export - not in Unreal Editor", "warning")
            return False
        
        try:
            with open(output_path, 'w', newline='', encoding='utf-8') as f:
                writer = csv.writer(f)
                writer.writerow(['asset_path', 'x', 'y', 'z', 'pitch', 'yaw', 'roll', 
                               'scale_x', 'scale_y', 'scale_z'])
                
                for actor in self.spawned_actors:
                    location = actor.get_actor_location()
                    rotation = actor.get_actor_rotation()
                    scale = actor.get_actor_scale3d()
                    asset_path = actor.get_class().get_path_name()
                    
                    writer.writerow([
                        asset_path,
                        location.x, location.y, location.z,
                        rotation.pitch, rotation.yaw, rotation.roll,
                        scale.x, scale.y, scale.z
                    ])
            
            self.log(f"Exported {len(self.spawned_actors)} actors to {output_path}")
            return True
            
        except Exception as e:
            self.log(f"Error exporting to CSV: {e}", "error")
            return False


# Module-level convenience functions
def populate_from_config(config_path: str) -> bool:
    """Convenience function to populate from config"""
    populator = ScenePopulator()
    return populator.populate_from_config(config_path)


def spawn_actors_grid(asset_path: str, count: int = 10, spacing: float = 5000,
                     center: Tuple[float, float, float] = (0, 0, 0)) -> List[Any]:
    """Convenience function to spawn actors in grid"""
    populator = ScenePopulator()
    return populator.spawn_actors_pattern(
        asset_path, 'grid', count,
        {'spacing': spacing, 'center': center}
    )


def spawn_actors_random(asset_path: str, count: int = 10, radius: float = 10000,
                       center: Tuple[float, float, float] = (0, 0, 0)) -> List[Any]:
    """Convenience function to spawn actors randomly"""
    populator = ScenePopulator()
    return populator.spawn_actors_pattern(
        asset_path, 'random', count,
        {'radius': radius, 'center': center}
    )


def spawn_actors_orbit(asset_path: str, count: int = 8, radius: float = 10000,
                      center: Tuple[float, float, float] = (0, 0, 0)) -> List[Any]:
    """Convenience function to spawn actors in orbit"""
    populator = ScenePopulator()
    return populator.spawn_actors_pattern(
        asset_path, 'orbit', count,
        {'radius': radius, 'center': center}
    )


def main():
    """Main function for command-line usage"""
    parser = argparse.ArgumentParser(
        description='Adastrea Scene Populator - Automated level population',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Populate from config file
  python ScenePopulator.py --config scene_config.yaml
  
  # Quick spawn with grid pattern
  python ScenePopulator.py --pattern grid --asset /Game/Ships/BP_Fighter --count 10
  
  # Spawn in orbit pattern
  python ScenePopulator.py --pattern orbit --asset /Game/Stations/BP_Station --count 6 --radius 15000
        """
    )
    
    parser.add_argument('--config', type=str, help='Path to YAML/CSV configuration file')
    parser.add_argument('--pattern', type=str, choices=['grid', 'random', 'orbit', 'spiral', 'cluster', 'line'],
                       help='Placement pattern')
    parser.add_argument('--asset', type=str, help='Asset path (e.g., /Game/Ships/BP_Fighter)')
    parser.add_argument('--count', type=int, default=10, help='Number of actors to spawn')
    parser.add_argument('--spacing', type=float, default=5000, help='Spacing for grid/line patterns')
    parser.add_argument('--radius', type=float, default=10000, help='Radius for circular patterns')
    
    args = parser.parse_args()
    
    populator = ScenePopulator()
    
    if args.config:
        # Populate from config file
        success = populator.populate_from_config(args.config)
        sys.exit(0 if success else 1)
    
    elif args.pattern and args.asset:
        # Quick spawn with pattern
        pattern_params = {}
        if args.pattern == 'grid':
            pattern_params = {'spacing': args.spacing}
        elif args.pattern in ['random', 'orbit']:
            pattern_params = {'radius': args.radius}
        
        actors = populator.spawn_actors_pattern(args.asset, args.pattern, args.count, pattern_params)
        print(f"Spawned {len(actors)} actors")
        sys.exit(0)
    
    else:
        parser.print_help()
        sys.exit(1)


def spawn_station_with_modules(station_asset: str, module_count: int = 8,
                              module_spacing: float = 1000, center: Tuple[float, float, float] = (0, 0, 0)) -> List[Any]:
    """
    Spawn a station with attached modules in a circular pattern

    Args:
        station_asset: Path to station blueprint/asset
        module_count: Number of modules to spawn
        module_spacing: Distance from center for modules
        center: Center position for the station

    Returns:
        List of spawned actors (station + modules)
    """
    if not UNREAL_AVAILABLE:
        print(f"Would spawn station with {module_count} modules at {center}")
        return []

    populator = ScenePopulator()

    # Spawn central station
    station = populator.spawn_actor(station_asset, center, (0, 0, 0))
    if not station:
        return []

    actors = [station]

    # Spawn modules in circle around station
    for i in range(module_count):
        angle = (2 * math.pi * i) / module_count
        x = center[0] + math.cos(angle) * module_spacing
        y = center[1] + math.sin(angle) * module_spacing
        z = center[2] + random.uniform(-100, 100)  # Slight vertical variation

        # Rotate to face center
        rotation = (0, math.degrees(angle) + 180, 0)

        # Use different module types (would need actual module assets)
        module_asset = f"/Game/Adastrea/Blueprints/Stations/Modules/StationModule_{random.randint(1, 5)}"

        module = populator.spawn_actor(module_asset, (x, y, z), rotation)
        if module:
            actors.append(module)

    print(f"Spawned station with {len(actors)-1} modules")
    return actors


def create_trading_route(start_station: Any, end_station: Any, waypoint_count: int = 5) -> List[Any]:
    """
    Create a trading route between two stations with waypoints

    Args:
        start_station: Starting station actor
        end_station: Ending station actor
        waypoint_count: Number of waypoints along the route

    Returns:
        List of waypoint actors
    """
    if not UNREAL_AVAILABLE:
        print(f"Would create trading route with {waypoint_count} waypoints")
        return []

    waypoints = []

    if not start_station or not end_station:
        return waypoints

    # Get positions
    start_pos = start_station.get_actor_location()
    end_pos = end_station.get_actor_location()

    # Create waypoints along the line
    for i in range(waypoint_count):
        t = (i + 1) / (waypoint_count + 1)  # Position between 0 and 1

        x = start_pos.x + t * (end_pos.x - start_pos.x)
        y = start_pos.y + t * (end_pos.y - start_pos.y)
        z = start_pos.z + t * (end_pos.z - start_pos.z)

        # Add some randomness to make routes less straight
        x += random.uniform(-500, 500)
        y += random.uniform(-500, 500)
        z += random.uniform(-200, 200)

        # Spawn waypoint marker (would need actual waypoint asset)
        waypoint_asset = "/Game/Adastrea/Blueprints/Navigation/WaypointMarker"
        waypoint = ScenePopulator().spawn_actor(waypoint_asset, (x, y, z), (0, 0, 0))

        if waypoint:
            waypoints.append(waypoint)

    print(f"Created trading route with {len(waypoints)} waypoints")
    return waypoints


def populate_faction_territory(faction_asset: str, center: Tuple[float, float, float],
                              radius: float, station_count: int = 3, ship_count: int = 8) -> Dict[str, List[Any]]:
    """
    Populate a faction's territory with stations and ships

    Args:
        faction_asset: Path to faction data asset
        center: Center of territory
        radius: Territory radius
        station_count: Number of stations to spawn
        ship_count: Number of ships to patrol

    Returns:
        Dictionary with spawned actors by type
    """
    if not UNREAL_AVAILABLE:
        print(f"Would populate faction territory: {station_count} stations, {ship_count} ships")
        return {"stations": [], "ships": [], "waypoints": []}

    result = {"stations": [], "ships": [], "waypoints": []}

    # Spawn stations in territory
    station_positions = PlacementPattern.cluster(station_count, station_count, 2000, radius * 0.7, center)
    for pos in station_positions:
        station_asset_path = f"/Game/Adastrea/Blueprints/Stations/{faction_asset.split('_')[-1]}_Station"
        station = ScenePopulator().spawn_actor(station_asset_path, pos, (0, 0, 0))
        if station:
            result["stations"].append(station)

    # Spawn patrol ships
    ship_positions = PlacementPattern.orbit(ship_count, radius * 0.8, center)
    for pos in ship_positions:
        ship_asset_path = f"/Game/Adastrea/Blueprints/Ships/{faction_asset.split('_')[-1]}_Fighter"
        ship = ScenePopulator().spawn_actor(ship_asset_path, pos, (0, 0, 0))
        if ship:
            result["ships"].append(ship)

    # Create trading routes between stations
    if len(result["stations"]) > 1:
        for i in range(len(result["stations"]) - 1):
            waypoints = create_trading_route(result["stations"][i], result["stations"][i + 1])
            result["waypoints"].extend(waypoints)

    print(f"Populated faction territory with {len(result['stations'])} stations, {len(result['ships'])} ships, {len(result['waypoints'])} waypoints")
    return result


def create_asteroid_belt(center: Tuple[float, float, float], inner_radius: float,
                        outer_radius: float, asteroid_count: int = 200) -> List[Any]:
    """
    Create an asteroid belt between inner and outer radii

    Args:
        center: Center of the asteroid belt
        inner_radius: Inner radius of belt
        outer_radius: Outer radius of belt
        asteroid_count: Number of asteroids to spawn

    Returns:
        List of spawned asteroid actors
    """
    if not UNREAL_AVAILABLE:
        print(f"Would create asteroid belt with {asteroid_count} asteroids")
        return []

    asteroids = []

    for _ in range(asteroid_count):
        # Random radius between inner and outer
        radius = random.uniform(inner_radius, outer_radius)

        # Random angle
        angle = random.uniform(0, 2 * math.pi)

        # Position on circle
        x = center[0] + math.cos(angle) * radius
        y = center[1] + math.sin(angle) * radius
        z = center[2] + random.uniform(-500, 500)  # Some vertical variation

        # Random rotation
        rotation = (random.uniform(-180, 180), random.uniform(-180, 180), random.uniform(-180, 180))

        # Random scale
        scale = random.uniform(0.2, 2.0)

        # Spawn asteroid (using basic shapes as placeholders)
        asteroid_types = ["/Engine/BasicShapes/Sphere", "/Engine/BasicShapes/Cube", "/Engine/BasicShapes/Cylinder"]
        asteroid_asset = random.choice(asteroid_types)

        asteroid = ScenePopulator().spawn_actor(asteroid_asset, (x, y, z), rotation)
        if asteroid:
            # Set random scale
            asteroid.set_actor_scale3d((scale, scale, scale))
            asteroids.append(asteroid)

    print(f"Created asteroid belt with {len(asteroids)} asteroids")
    return asteroids


def create_jump_gate_network(gate_positions: List[Tuple[float, float, float]]) -> List[Any]:
    """
    Create a network of jump gates connected by routes

    Args:
        gate_positions: List of positions for jump gates

    Returns:
        List of spawned jump gate actors
    """
    if not UNREAL_AVAILABLE:
        print(f"Would create jump gate network with {len(gate_positions)} gates")
        return []

    gates = []

    for pos in gate_positions:
        # Spawn jump gate (using torus as placeholder)
        gate = ScenePopulator().spawn_actor("/Engine/BasicShapes/Torus", pos, (0, 0, 0))
        if gate:
            # Make it large and visible
            gate.set_actor_scale3d((20.0, 20.0, 5.0))
            gates.append(gate)

    # Create navigation routes between gates
    waypoints = []
    for i in range(len(gates) - 1):
        route_waypoints = create_trading_route(gates[i], gates[i + 1], 3)
        waypoints.extend(route_waypoints)

    print(f"Created jump gate network with {len(gates)} gates and {len(waypoints)} route waypoints")
    return gates + waypoints


def populate_combat_zone(center: Tuple[float, float, float], radius: float,
                        faction_a_ships: int = 5, faction_b_ships: int = 5) -> Dict[str, List[Any]]:
    """
    Populate a combat zone with ships from two opposing factions

    Args:
        center: Center of combat zone
        radius: Combat zone radius
        faction_a_ships: Number of ships for faction A
        faction_b_ships: Number of ships for faction B

    Returns:
        Dictionary with spawned actors by faction
    """
    if not UNREAL_AVAILABLE:
        print(f"Would create combat zone with {faction_a_ships + faction_b_ships} ships")
        return {"faction_a": [], "faction_b": []}

    result = {"faction_a": [], "faction_b": []}

    # Spawn faction A ships (one side of zone)
    for i in range(faction_a_ships):
        angle = random.uniform(-math.pi/4, math.pi/4)  # Front arc
        distance = random.uniform(radius * 0.3, radius * 0.8)
        x = center[0] + math.cos(angle) * distance
        y = center[1] + math.sin(angle) * distance
        z = center[2] + random.uniform(-200, 200)

        ship = ScenePopulator().spawn_actor("/Game/Adastrea/Blueprints/Ships/Fighter_A", (x, y, z), (0, 0, 0))
        if ship:
            result["faction_a"].append(ship)

    # Spawn faction B ships (opposite side)
    for i in range(faction_b_ships):
        angle = random.uniform(math.pi * 3/4, math.pi * 5/4)  # Back arc
        distance = random.uniform(radius * 0.3, radius * 0.8)
        x = center[0] + math.cos(angle) * distance
        y = center[1] + math.sin(angle) * distance
        z = center[2] + random.uniform(-200, 200)

        ship = ScenePopulator().spawn_actor("/Game/Adastrea/Blueprints/Ships/Fighter_B", (x, y, z), (0, 0, 0))
        if ship:
            result["faction_b"].append(ship)

    print(f"Created combat zone with {len(result['faction_a'])} vs {len(result['faction_b'])} ships")
    return result


def create_environment_effects(center: Tuple[float, float, float], effect_type: str = "nebula",
                              radius: float = 5000, count: int = 20) -> List[Any]:
    """
    Create environmental effects like nebulae, dust clouds, etc.

    Args:
        center: Center of effect area
        effect_type: Type of effect ("nebula", "dust", "energy")
        radius: Effect radius
        count: Number of effect actors

    Returns:
        List of spawned effect actors
    """
    if not UNREAL_AVAILABLE:
        print(f"Would create {effect_type} effects: {count} actors")
        return []

    effects = []

    for _ in range(count):
        # Random position within radius
        angle = random.uniform(0, 2 * math.pi)
        distance = random.uniform(0, radius)
        x = center[0] + math.cos(angle) * distance
        y = center[1] + math.sin(angle) * distance
        z = center[2] + random.uniform(-1000, 1000)

        # Different effects based on type
        if effect_type == "nebula":
            effect_asset = "/Engine/BasicShapes/Sphere"  # Placeholder for nebula volume
            scale = random.uniform(5.0, 15.0)
        elif effect_type == "dust":
            effect_asset = "/Engine/BasicShapes/Cube"  # Placeholder for dust cloud
            scale = random.uniform(2.0, 8.0)
        else:  # energy
            effect_asset = "/Engine/BasicShapes/Cylinder"  # Placeholder for energy field
            scale = random.uniform(1.0, 5.0)

        effect = ScenePopulator().spawn_actor(effect_asset, (x, y, z),
                                            (random.uniform(-180, 180), random.uniform(-180, 180), random.uniform(-180, 180)))
        if effect:
            effect.set_actor_scale3d((scale, scale, scale))
            effects.append(effect)

    print(f"Created {effect_type} effects: {len(effects)} actors")
    return effects


def populate_complete_sector() -> Dict[str, Any]:
    """
    Populate a complete sector with all content types

    Returns:
        Dictionary with all spawned content organized by type
    """
    if not UNREAL_AVAILABLE:
        print("Would populate complete sector with all content types")
        return {}

    sector_content = {
        "stations": [],
        "ships": [],
        "asteroids": [],
        "jump_gates": [],
        "effects": [],
        "waypoints": []
    }

    # Create central hub station
    hub_station = spawn_station_with_modules("/Game/Adastrea/Blueprints/Stations/TradingHub", 12, 1500, (0, 0, 0))
    sector_content["stations"].extend(hub_station)

    # Create faction territories
    faction_centers = [(15000, 0, 0), (-15000, 0, 0), (0, 15000, 0), (0, -15000, 0)]
    for i, center in enumerate(faction_centers):
        faction_territory = populate_faction_territory(f"DA_Faction_{i+1}", center, 8000, 3, 6)
        sector_content["stations"].extend(faction_territory["stations"])
        sector_content["ships"].extend(faction_territory["ships"])
        sector_content["waypoints"].extend(faction_territory["waypoints"])

    # Create asteroid belts
    asteroid_belts = [
        ((5000, 5000, 0), 3000, 6000, 150),
        ((-5000, -5000, 0), 4000, 7000, 120),
        ((0, 8000, 0), 2000, 5000, 100)
    ]
    for center, inner, outer, count in asteroid_belts:
        asteroids = create_asteroid_belt(center, inner, outer, count)
        sector_content["asteroids"].extend(asteroids)

    # Create jump gate network
    gate_positions = [(20000, 0, 0), (-20000, 0, 0), (0, 20000, 0), (0, -20000, 0), (14142, 14142, 0), (-14142, -14142, 0)]
    jump_network = create_jump_gate_network(gate_positions)
    sector_content["jump_gates"].extend(jump_network)

    # Create environmental effects
    nebulae = create_environment_effects((10000, 10000, 0), "nebula", 8000, 15)
    dust_clouds = create_environment_effects((-10000, -10000, 0), "dust", 6000, 25)
    sector_content["effects"].extend(nebulae + dust_clouds)

    # Create some combat zones
    combat_zones = [
        ((8000, 8000, 0), 3000, 4, 4),
        ((-8000, -8000, 0), 2500, 3, 3)
    ]
    for center, radius, ships_a, ships_b in combat_zones:
        combat = populate_combat_zone(center, radius, ships_a, ships_b)
        sector_content["ships"].extend(combat["faction_a"] + combat["faction_b"])

    total_actors = sum(len(actors) for actors in sector_content.values())
    print(f"Populated complete sector with {total_actors} total actors:")
    for content_type, actors in sector_content.items():
        print(f"  {content_type}: {len(actors)}")

    return sector_content


if __name__ == "__main__":
    main()
