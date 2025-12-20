#!/usr/bin/env python3
"""
Map Generator for Adastrea

Creates test maps and levels for the space flight game.
Generates basic scenes with lighting, atmosphere, and spawn points.

Usage:
    from MapGenerator import MapGenerator
    generator = MapGenerator()
    generator.generate_test_maps()
"""

import unreal
from typing import Dict, List, Optional, Any
import logging

logger = logging.getLogger(__name__)


class MapGenerator:
    """
    Generates test maps and levels for Adastrea
    """

    def __init__(self):
        """Initialize the map generator"""
        self.asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        self.editor_asset_lib = unreal.EditorAssetLibrary
        self.generated_count = 0

    def generate_test_maps(self) -> int:
        """
        Generate test maps for development and testing

        Returns:
            Number of maps created
        """
        print("[MapGen] Generating test maps...")

        count = 0

        # Basic test maps
        count += self._create_main_menu_map()
        count += self._create_gameplay_test_map()
        count += self._create_trading_station_map()
        count += self._create_combat_test_map()

        print(f"[MapGen] Created {count} test maps")
        return count

    def _create_main_menu_map(self) -> int:
        """Create main menu map"""
        try:
            map_name = "MainMenu"
            map_path = f"/Game/Maps/{map_name}"

            if self.editor_asset_lib.does_asset_exist(map_path):
                print(f"[MapGen] {map_name} already exists")
                return 0

            # Create new level
            world = unreal.UnrealEdEngine.new_level(unreal.UnrealEdEngine, map_path)

            if world:
                # Add basic lighting
                self._add_basic_lighting(world)
                self._add_menu_camera(world)

                # Save the level
                unreal.EditorLevelLibrary.save_current_level()

                print(f"[MapGen] ✓ Created {map_name}")
                return 1
            return 0

        except Exception as e:
            print(f"[MapGen] Error creating main menu map: {e}")
            return 0

    def _create_gameplay_test_map(self) -> int:
        """Create gameplay test map"""
        try:
            map_name = "GameplayTest"
            map_path = f"/Game/Maps/{map_name}"

            if self.editor_asset_lib.does_asset_exist(map_path):
                print(f"[MapGen] {map_name} already exists")
                return 0

            # Create new level
            world = unreal.UnrealEdEngine.new_level(unreal.UnrealEdEngine, map_path)

            if world:
                # Add basic setup
                self._add_basic_lighting(world)
                self._add_player_start(world)
                self._add_test_geometry(world)

                # Save the level
                unreal.EditorLevelLibrary.save_current_level()

                print(f"[MapGen] ✓ Created {map_name}")
                return 1
            return 0

        except Exception as e:
            print(f"[MapGen] Error creating gameplay test map: {e}")
            return 0

    def _create_trading_station_map(self) -> int:
        """Create trading station test map"""
        try:
            map_name = "TradingStation"
            map_path = f"/Game/Maps/{map_name}"

            if self.editor_asset_lib.does_asset_exist(map_path):
                print(f"[MapGen] {map_name} already exists")
                return 0

            # Create new level
            world = unreal.UnrealEdEngine.new_level(unreal.UnrealEdEngine, map_path)

            if world:
                # Add station setup
                self._add_basic_lighting(world)
                self._add_player_start(world)
                self._add_trading_station(world)

                # Save the level
                unreal.EditorLevelLibrary.save_current_level()

                print(f"[MapGen] ✓ Created {map_name}")
                return 1
            return 0

        except Exception as e:
            print(f"[MapGen] Error creating trading station map: {e}")
            return 0

    def _create_combat_test_map(self) -> int:
        """Create combat test map"""
        try:
            map_name = "CombatTest"
            map_path = f"/Game/Maps/{map_name}"

            if self.editor_asset_lib.does_asset_exist(map_path):
                print(f"[MapGen] {map_name} already exists")
                return 0

            # Create new level
            world = unreal.UnrealEdEngine.new_level(unreal.UnrealEdEngine, map_path)

            if world:
                # Add combat setup
                self._add_basic_lighting(world)
                self._add_player_start(world)
                self._add_combat_targets(world)

                # Save the level
                unreal.EditorLevelLibrary.save_current_level()

                print(f"[MapGen] ✓ Created {map_name}")
                return 1
            return 0

        except Exception as e:
            print(f"[MapGen] Error creating combat test map: {e}")
            return 0

    def _add_basic_lighting(self, world: Any):
        """Add basic lighting to the level"""
        try:
            # Add directional light
            light_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
                unreal.DirectionalLight,
                unreal.Vector(0, 0, 1000),
                unreal.Rotator(0, 0, 0)
            )

            if light_actor:
                # Set light properties
                light_component = light_actor.light_component
                if light_component:
                    light_component.set_intensity(3.0)
                    light_component.set_light_color(unreal.LinearColor(1.0, 1.0, 0.9, 1.0))

            # Add sky light
            sky_light = unreal.EditorLevelLibrary.spawn_actor_from_class(
                unreal.SkyLight,
                unreal.Vector(0, 0, 0),
                unreal.Rotator(0, 0, 0)
            )

        except Exception as e:
            print(f"[MapGen] Error adding basic lighting: {e}")

    def _add_player_start(self, world: Any):
        """Add player start actor"""
        try:
            player_start = unreal.EditorLevelLibrary.spawn_actor_from_class(
                unreal.PlayerStart,
                unreal.Vector(0, 0, 100),
                unreal.Rotator(0, 0, 0)
            )
        except Exception as e:
            print(f"[MapGen] Error adding player start: {e}")

    def _add_menu_camera(self, world: Any):
        """Add menu camera"""
        try:
            camera_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
                unreal.CineCameraActor,
                unreal.Vector(0, -500, 200),
                unreal.Rotator(-15, 0, 0)
            )
        except Exception as e:
            print(f"[MapGen] Error adding menu camera: {e}")

    def _add_test_geometry(self, world: Any):
        """Add test geometry for gameplay testing"""
        try:
            # Add some basic cubes and spheres for testing
            geometries = [
                (unreal.CubeBuilder, unreal.Vector(500, 0, 0)),
                (unreal.SphereBuilder, unreal.Vector(-500, 0, 0)),
                (unreal.CylinderBuilder, unreal.Vector(0, 500, 0)),
            ]

            for builder_class, location in geometries:
                actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
                    unreal.Actor,
                    location,
                    unreal.Rotator(0, 0, 0)
                )

                if actor:
                    # Add static mesh component
                    mesh_component = actor.add_component(unreal.StaticMeshComponent)
                    if mesh_component:
                        # Create basic geometry
                        builder = builder_class()
                        mesh = builder.build_mesh()
                        if mesh:
                            mesh_component.set_static_mesh(mesh)

        except Exception as e:
            print(f"[MapGen] Error adding test geometry: {e}")

    def _add_trading_station(self, world: Any):
        """Add trading station setup"""
        try:
            # Add large platform for station
            platform = unreal.EditorLevelLibrary.spawn_actor_from_class(
                unreal.Actor,
                unreal.Vector(0, 0, 0),
                unreal.Rotator(0, 0, 0)
            )

            if platform:
                mesh_component = platform.add_component(unreal.StaticMeshComponent)
                if mesh_component:
                    # Create large cube for platform
                    builder = unreal.CubeBuilder()
                    builder.x = 2000
                    builder.y = 2000
                    builder.z = 50
                    mesh = builder.build_mesh()
                    if mesh:
                        mesh_component.set_static_mesh(mesh)

            # Add some smaller structures
            for i in range(5):
                angle = (i / 5.0) * 360
                x = 800 * unreal.MathLibrary.sin(angle)
                y = 800 * unreal.MathLibrary.cos(angle)

                structure = unreal.EditorLevelLibrary.spawn_actor_from_class(
                    unreal.Actor,
                    unreal.Vector(x, y, 100),
                    unreal.Rotator(0, angle, 0)
                )

                if structure:
                    mesh_component = structure.add_component(unreal.StaticMeshComponent)
                    if mesh_component:
                        builder = unreal.CylinderBuilder()
                        builder.outer_radius = 50
                        builder.inner_radius = 0
                        builder.outer_height = 200
                        mesh = builder.build_mesh()
                        if mesh:
                            mesh_component.set_static_mesh(mesh)

        except Exception as e:
            print(f"[MapGen] Error adding trading station: {e}")

    def _add_combat_targets(self, world: Any):
        """Add combat test targets"""
        try:
            # Add target drones in a circle
            for i in range(8):
                angle = (i / 8.0) * 360
                x = 1000 * unreal.MathLibrary.sin(angle)
                y = 1000 * unreal.MathLibrary.cos(angle)

                target = unreal.EditorLevelLibrary.spawn_actor_from_class(
                    unreal.Actor,
                    unreal.Vector(x, y, 100),
                    unreal.Rotator(0, angle, 0)
                )

                if target:
                    mesh_component = target.add_component(unreal.StaticMeshComponent)
                    if mesh_component:
                        builder = unreal.SphereBuilder()
                        builder.radius = 25
                        mesh = builder.build_mesh()
                        if mesh:
                            mesh_component.set_static_mesh(mesh)

        except Exception as e:
            print(f"[MapGen] Error adding combat targets: {e}")