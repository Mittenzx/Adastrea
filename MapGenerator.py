#!/usr/bin/env python3
"""
Adastrea - Map/Level Generator

This script creates basic test maps/levels for the Adastrea project with proper setup.

Features:
- Create TestMode.umap with basic environment
- Create sector test maps
- Add essential actors (PlayerStart, Lighting, PostProcess)
- Configure World Settings with Game Mode
- Add SpaceshipSpawnPoint markers

Usage:
    # In Unreal Editor Python Console
    import MapGenerator
    MapGenerator.generate_test_maps()
    MapGenerator.create_test_mode_map()
"""

import sys
from typing import Optional

try:
    import unreal
    UNREAL_AVAILABLE = True
except ImportError:
    UNREAL_AVAILABLE = False
    print("ERROR: This script must be run inside Unreal Editor!")
    sys.exit(1)


class MapGenerator:
    """Generator for creating test maps and levels"""
    
    # Maps to create
    TEST_MAPS = [
        ("TestMode", "Basic test environment for spaceship testing"),
        ("SectorTest_Alpha", "Alpha sector test area"),
        ("SectorTest_Beta", "Beta sector test area"),
        ("CombatArena", "Combat testing arena"),
    ]
    
    def __init__(self):
        """Initialize the map generator"""
        self.editor_level_lib = unreal.EditorLevelLibrary()
        self.editor_asset_lib = unreal.EditorAssetLibrary()
        
        self.created_count = 0
        self.skipped_count = 0
        self.errors = []
    
    def log(self, message: str, level: str = "info"):
        """Log message to Unreal Editor"""
        if level == "error":
            unreal.log_error(f"[MapGen] {message}")
        elif level == "warning":
            unreal.log_warning(f"[MapGen] {message}")
        else:
            unreal.log(f"[MapGen] {message}")
    
    def create_map(self, map_name: str, description: str = "") -> Optional[str]:
        """
        Create a new map/level
        
        Args:
            map_name: Name of the map
            description: Description of the map
            
        Returns:
            Path to created map, or None if failed
        """
        map_path = f"/Game/Maps/{map_name}"
        
        # Check if map already exists
        if self.editor_asset_lib.does_asset_exist(map_path):
            self.log(f"Map already exists: {map_name}", "info")
            self.skipped_count += 1
            return map_path
        
        try:
            # Create new level
            new_level = self.editor_level_lib.new_level(f"/Game/Maps/{map_name}")
            
            if not new_level:
                self.log(f"Failed to create map: {map_name}", "error")
                return None
            
            # Add basic actors
            self._setup_basic_environment(map_name)
            
            # Save the level
            success = self.editor_level_lib.save_current_level()
            
            if success:
                self.log(f"✓ Created map: {map_name}")
                self.created_count += 1
                return map_path
            else:
                self.log(f"Failed to save map: {map_name}", "error")
                return None
                
        except Exception as e:
            self.log(f"Error creating map {map_name}: {str(e)}", "error")
            self.errors.append(f"{map_name}: {str(e)}")
            return None
    
    def _setup_basic_environment(self, map_name: str):
        """Setup basic environment for a map"""
        try:
            # Add Player Start
            player_start = self.editor_level_lib.spawn_actor_from_class(
                unreal.PlayerStart,
                unreal.Vector(0, 0, 100)
            )
            if player_start:
                player_start.set_actor_label("PlayerStart")
                self.log(f"  → Added PlayerStart to {map_name}")
            
            # Add Directional Light
            light = self.editor_level_lib.spawn_actor_from_class(
                unreal.DirectionalLight,
                unreal.Vector(0, 0, 500),
                unreal.Rotator(-45, 0, 0)
            )
            if light:
                light.set_actor_label("DirectionalLight_Sun")
                self.log(f"  → Added DirectionalLight to {map_name}")
            
            # Add Sky Light
            sky_light = self.editor_level_lib.spawn_actor_from_class(
                unreal.SkyLight,
                unreal.Vector(0, 0, 400)
            )
            if sky_light:
                sky_light.set_actor_label("SkyLight")
                self.log(f"  → Added SkyLight to {map_name}")
            
            # Add Post Process Volume
            post_process = self.editor_level_lib.spawn_actor_from_class(
                unreal.PostProcessVolume,
                unreal.Vector(0, 0, 0)
            )
            if post_process:
                post_process.set_actor_label("PostProcessVolume")
                # Make it infinite extent
                post_process.set_editor_property("bUnbound", True)
                self.log(f"  → Added PostProcessVolume to {map_name}")
            
            # Add a basic floor plane for reference
            floor = self.editor_level_lib.spawn_actor_from_class(
                unreal.StaticMeshActor,
                unreal.Vector(0, 0, 0)
            )
            if floor:
                floor.set_actor_label("Floor")
                # Try to set a basic cube mesh
                static_mesh_comp = floor.get_component_by_class(unreal.StaticMeshComponent)
                if static_mesh_comp:
                    # Load basic cube mesh if available
                    cube_mesh = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube")
                    if cube_mesh:
                        static_mesh_comp.set_static_mesh(cube_mesh)
                        # Scale it to be a floor
                        floor.set_actor_scale3d(unreal.Vector(100, 100, 0.1))
                self.log(f"  → Added Floor plane to {map_name}")
            
            # Add spaceship spawn point marker (for TestMode)
            if "test" in map_name.lower():
                spawn_marker = self.editor_level_lib.spawn_actor_from_class(
                    unreal.TargetPoint,
                    unreal.Vector(0, 0, 150)
                )
                if spawn_marker:
                    spawn_marker.set_actor_label("SpaceshipSpawnPoint")
                    spawn_marker.tags.append("SpaceshipSpawnPoint")
                    self.log(f"  → Added SpaceshipSpawnPoint to {map_name}")
            
        except Exception as e:
            self.log(f"Warning: Could not setup all environment actors: {str(e)}", "warning")
    
    def generate_test_maps(self) -> int:
        """
        Generate all test maps
        
        Returns:
            Number of maps created
        """
        self.log("=" * 80)
        self.log("GENERATING TEST MAPS")
        self.log("=" * 80)
        
        for map_name, description in self.TEST_MAPS:
            self.create_map(map_name, description)
        
        self.log("")
        self.log("=" * 80)
        self.log("MAP GENERATION COMPLETE!")
        self.log(f"Created: {self.created_count}, Skipped: {self.skipped_count}")
        
        if self.errors:
            self.log(f"\nErrors encountered: {len(self.errors)}", "warning")
            for error in self.errors:
                self.log(f"  - {error}", "warning")
        
        self.log("")
        self.log("NEXT STEPS:")
        self.log("1. Open each map in the editor")
        self.log("2. Configure World Settings → Game Mode Override → BP_SpaceGameMode")
        self.log("3. Add additional environment details as needed")
        self.log("4. Test by pressing Play")
        self.log("=" * 80)
        
        return self.created_count


# Convenience functions
def generate_test_maps() -> int:
    """Generate test maps - convenience function"""
    generator = MapGenerator()
    return generator.generate_test_maps()


def create_test_mode_map() -> Optional[str]:
    """Create TestMode map - convenience function"""
    generator = MapGenerator()
    return generator.create_map("TestMode", "Basic test environment for spaceship testing")


# Command line interface
def main():
    """Command line interface"""
    import argparse
    
    parser = argparse.ArgumentParser(
        description="Adastrea Map Generator - Create test maps/levels"
    )
    
    parser.add_argument("--all", action="store_true",
                       help="Generate all test maps")
    parser.add_argument("--name", type=str,
                       help="Generate specific map by name")
    
    args = parser.parse_args()
    
    generator = MapGenerator()
    
    if args.all:
        generator.generate_test_maps()
    elif args.name:
        generator.create_map(args.name)
    else:
        parser.print_help()
        print("\nTIP: Use --all to generate all test maps")


if __name__ == "__main__":
    main()
