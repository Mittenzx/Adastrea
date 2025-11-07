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


if __name__ == "__main__":
    main()
