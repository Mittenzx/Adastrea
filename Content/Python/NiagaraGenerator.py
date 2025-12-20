#!/usr/bin/env python3
"""
Niagara Generator for Adastrea

Creates Niagara particle systems for visual effects in the space flight game.
Generates engine trails, explosions, weapon effects, and environmental particles.

Usage:
    from NiagaraGenerator import NiagaraGenerator
    generator = NiagaraGenerator()
    generator.generate_all_systems()
"""

import unreal
from typing import Dict, List, Optional, Any
import logging

logger = logging.getLogger(__name__)


class NiagaraGenerator:
    """
    Generates Niagara particle systems for Adastrea
    """

    def __init__(self):
        """Initialize the Niagara generator"""
        self.asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        self.editor_asset_lib = unreal.EditorAssetLibrary
        self.generated_count = 0

    def generate_all_systems(self) -> int:
        """
        Generate all Niagara particle systems

        Returns:
            Number of systems created
        """
        print("[NiagaraGen] Generating Niagara particle systems...")

        count = 0

        # Ship effects
        count += self._create_engine_trails()
        count += self._create_ship_exhaust()

        # Weapon effects
        count += self._create_weapon_muzzle_flash()
        count += self._create_projectile_trails()
        count += self._create_explosion_effects()

        # Environmental effects
        count += self._create_space_dust()
        count += self._create_nebula_effects()

        print(f"[NiagaraGen] Created {count} Niagara systems")
        return count

    def _create_engine_trails(self) -> int:
        """Create engine trail effects"""
        try:
            system_name = "NS_Engine_Trail"
            system_path = f"/Game/Particles/Ships/{system_name}"

            if self.editor_asset_lib.does_asset_exist(system_path):
                print(f"[NiagaraGen] {system_name} already exists")
                return 0

            # Create Niagara system
            niagara_system = self.asset_tools.create_asset(
                asset_name=system_name,
                package_path="/Game/Particles/Ships",
                asset_class=unreal.NiagaraSystem,
                factory=unreal.NiagaraSystemFactoryNew()
            )

            if niagara_system:
                print(f"[NiagaraGen] ✓ Created {system_name}")
                return 1
            return 0

        except Exception as e:
            print(f"[NiagaraGen] Error creating engine trails: {e}")
            return 0

    def _create_ship_exhaust(self) -> int:
        """Create ship exhaust effects"""
        try:
            system_name = "NS_Ship_Exhaust"
            system_path = f"/Game/Particles/Ships/{system_name}"

            if self.editor_asset_lib.does_asset_exist(system_path):
                print(f"[NiagaraGen] {system_name} already exists")
                return 0

            niagara_system = self.asset_tools.create_asset(
                asset_name=system_name,
                package_path="/Game/Particles/Ships",
                asset_class=unreal.NiagaraSystem,
                factory=unreal.NiagaraSystemFactoryNew()
            )

            if niagara_system:
                print(f"[NiagaraGen] ✓ Created {system_name}")
                return 1
            return 0

        except Exception as e:
            print(f"[NiagaraGen] Error creating ship exhaust: {e}")
            return 0

    def _create_weapon_muzzle_flash(self) -> int:
        """Create weapon muzzle flash effects"""
        try:
            system_name = "NS_Weapon_Muzzle_Flash"
            system_path = f"/Game/Particles/Weapons/{system_name}"

            if self.editor_asset_lib.does_asset_exist(system_path):
                print(f"[NiagaraGen] {system_name} already exists")
                return 0

            niagara_system = self.asset_tools.create_asset(
                asset_name=system_name,
                package_path="/Game/Particles/Weapons",
                asset_class=unreal.NiagaraSystem,
                factory=unreal.NiagaraSystemFactoryNew()
            )

            if niagara_system:
                print(f"[NiagaraGen] ✓ Created {system_name}")
                return 1
            return 0

        except Exception as e:
            print(f"[NiagaraGen] Error creating muzzle flash: {e}")
            return 0

    def _create_projectile_trails(self) -> int:
        """Create projectile trail effects"""
        projectile_types = [
            "NS_Laser_Beam",
            "NS_Plasma_Bolt",
            "NS_Missile_Trail",
            "NS_Railgun_Slug",
        ]

        count = 0
        for system_name in projectile_types:
            try:
                system_path = f"/Game/Particles/Weapons/{system_name}"

                if self.editor_asset_lib.does_asset_exist(system_path):
                    continue

                niagara_system = self.asset_tools.create_asset(
                    asset_name=system_name,
                    package_path="/Game/Particles/Weapons",
                    asset_class=unreal.NiagaraSystem,
                    factory=unreal.NiagaraSystemFactoryNew()
                )

                if niagara_system:
                    print(f"[NiagaraGen] ✓ Created {system_name}")
                    count += 1

            except Exception as e:
                print(f"[NiagaraGen] Error creating {system_name}: {e}")

        return count

    def _create_explosion_effects(self) -> int:
        """Create explosion effects"""
        explosion_types = [
            "NS_Ship_Explosion",
            "NS_Weapon_Impact",
            "NS_Station_Destruction",
            "NS_Asteroid_Impact",
        ]

        count = 0
        for system_name in explosion_types:
            try:
                system_path = f"/Game/Particles/Effects/{system_name}"

                if self.editor_asset_lib.does_asset_exist(system_path):
                    continue

                niagara_system = self.asset_tools.create_asset(
                    asset_name=system_name,
                    package_path="/Game/Particles/Effects",
                    asset_class=unreal.NiagaraSystem,
                    factory=unreal.NiagaraSystemFactoryNew()
                )

                if niagara_system:
                    print(f"[NiagaraGen] ✓ Created {system_name}")
                    count += 1

            except Exception as e:
                print(f"[NiagaraGen] Error creating {system_name}: {e}")

        return count

    def _create_space_dust(self) -> int:
        """Create space dust effects"""
        try:
            system_name = "NS_Space_Dust"
            system_path = f"/Game/Particles/Environment/{system_name}"

            if self.editor_asset_lib.does_asset_exist(system_path):
                print(f"[NiagaraGen] {system_name} already exists")
                return 0

            niagara_system = self.asset_tools.create_asset(
                asset_name=system_name,
                package_path="/Game/Particles/Environment",
                asset_class=unreal.NiagaraSystem,
                factory=unreal.NiagaraSystemFactoryNew()
            )

            if niagara_system:
                print(f"[NiagaraGen] ✓ Created {system_name}")
                return 1
            return 0

        except Exception as e:
            print(f"[NiagaraGen] Error creating space dust: {e}")
            return 0

    def _create_nebula_effects(self) -> int:
        """Create nebula effects"""
        nebula_types = [
            "NS_Nebula_Gas",
            "NS_Nebula_Particles",
            "NS_Nebula_Lightning",
        ]

        count = 0
        for system_name in nebula_types:
            try:
                system_path = f"/Game/Particles/Environment/{system_name}"

                if self.editor_asset_lib.does_asset_exist(system_path):
                    continue

                niagara_system = self.asset_tools.create_asset(
                    asset_name=system_name,
                    package_path="/Game/Particles/Environment",
                    asset_class=unreal.NiagaraSystem,
                    factory=unreal.NiagaraSystemFactoryNew()
                )

                if niagara_system:
                    print(f"[NiagaraGen] ✓ Created {system_name}")
                    count += 1

            except Exception as e:
                print(f"[NiagaraGen] Error creating {system_name}: {e}")

        return count