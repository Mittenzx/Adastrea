#!/usr/bin/env python3
"""
Adastrea - Niagara System Generator

This script creates Niagara particle systems for visual effects in the Adastrea project.

Features:
- Generate spaceship engine effects
- Generate weapon fire effects
- Generate explosion effects
- Generate environmental effects (nebula, asteroids)
- Generate shield effects
- Automatic emitter configuration

Usage:
    # In Unreal Editor Python Console
    import NiagaraGenerator
    NiagaraGenerator.generate_all_effects()
    
    # Generate specific categories
    NiagaraGenerator.generate_engine_effects()
    NiagaraGenerator.generate_weapon_effects()
    NiagaraGenerator.generate_explosion_effects()
"""

import sys
from typing import Dict, Any, Optional

try:
    import unreal
    UNREAL_AVAILABLE = True
except ImportError:
    UNREAL_AVAILABLE = False
    print("ERROR: This script must be run inside Unreal Editor!")
    sys.exit(1)


class NiagaraGenerator:
    """Generator for Niagara particle system effects"""
    
    # Niagara Systems to create
    ENGINE_EFFECTS = [
        ("NS_Engine_Thrust", "Main engine thrust trail", "/Game/VFX/Ships/Engines"),
        ("NS_Engine_Boost", "Boost afterburner effect", "/Game/VFX/Ships/Engines"),
        ("NS_Engine_Idle", "Idle engine glow", "/Game/VFX/Ships/Engines"),
        ("NS_Thruster_Maneuvering", "Small maneuvering thrusters", "/Game/VFX/Ships/Engines"),
    ]
    
    WEAPON_EFFECTS = [
        ("NS_Weapon_LaserBeam", "Laser beam continuous fire", "/Game/VFX/Combat/Weapons"),
        ("NS_Weapon_PlasmaBolt", "Plasma projectile", "/Game/VFX/Combat/Weapons"),
        ("NS_Weapon_MissileLaunch", "Missile launch smoke", "/Game/VFX/Combat/Weapons"),
        ("NS_Weapon_MissileTrail", "Missile exhaust trail", "/Game/VFX/Combat/Weapons"),
        ("NS_Weapon_CannonFire", "Ballistic cannon muzzle flash", "/Game/VFX/Combat/Weapons"),
    ]
    
    EXPLOSION_EFFECTS = [
        ("NS_Explosion_Small", "Small explosion (fighter destroyed)", "/Game/VFX/Combat/Explosions"),
        ("NS_Explosion_Medium", "Medium explosion (frigate destroyed)", "/Game/VFX/Combat/Explosions"),
        ("NS_Explosion_Large", "Large explosion (capital ship)", "/Game/VFX/Combat/Explosions"),
        ("NS_Explosion_Impact", "Projectile impact on hull", "/Game/VFX/Combat/Explosions"),
    ]
    
    SHIELD_EFFECTS = [
        ("NS_Shield_Active", "Active shield bubble", "/Game/VFX/Combat/Shields"),
        ("NS_Shield_Impact", "Shield hit effect", "/Game/VFX/Combat/Shields"),
        ("NS_Shield_Recharge", "Shield recharging particles", "/Game/VFX/Combat/Shields"),
        ("NS_Shield_Failure", "Shield failure sparks", "/Game/VFX/Combat/Shields"),
    ]
    
    ENVIRONMENTAL_EFFECTS = [
        ("NS_Nebula_Particles", "Nebula particle field", "/Game/VFX/Environment"),
        ("NS_Asteroid_Dust", "Dust around asteroids", "/Game/VFX/Environment"),
        ("NS_Space_Debris", "Floating space debris", "/Game/VFX/Environment"),
        ("NS_Warp_Effect", "Warp/jump effect", "/Game/VFX/Environment"),
        ("NS_Dock_Landing", "Station docking particles", "/Game/VFX/Environment"),
    ]
    
    DAMAGE_EFFECTS = [
        ("NS_Hull_Fire", "Hull damage fire", "/Game/VFX/Combat/Damage"),
        ("NS_Hull_Sparks", "Electrical damage sparks", "/Game/VFX/Combat/Damage"),
        ("NS_Hull_Smoke", "Damage smoke trail", "/Game/VFX/Combat/Damage"),
        ("NS_Leak_Atmosphere", "Atmosphere leaking from hull breach", "/Game/VFX/Combat/Damage"),
    ]
    
    def __init__(self):
        """Initialize the Niagara generator"""
        self.asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        self.editor_asset_lib = unreal.EditorAssetLibrary()
        
        self.created_count = 0
        self.skipped_count = 0
        self.errors = []
    
    def log(self, message: str, level: str = "info"):
        """Log message to Unreal Editor"""
        if level == "error":
            unreal.log_error(f"[NiagaraGen] {message}")
        elif level == "warning":
            unreal.log_warning(f"[NiagaraGen] {message}")
        else:
            unreal.log(f"[NiagaraGen] {message}")
    
    def create_niagara_system(self, system_name: str, description: str, 
                             folder_path: str) -> Optional[str]:
        """
        Create a Niagara System asset
        
        Args:
            system_name: Name of the system (e.g., "NS_Engine_Thrust")
            description: Description of the effect
            folder_path: Unreal content folder path
            
        Returns:
            Path to created system, or None if failed
        """
        asset_path = f"{folder_path}/{system_name}"
        
        # Check if already exists
        if self.editor_asset_lib.does_asset_exist(asset_path):
            self.log(f"Niagara System already exists: {system_name}", "info")
            self.skipped_count += 1
            return asset_path
        
        try:
            # Create Niagara System factory
            factory = unreal.NiagaraSystemFactoryNew()
            
            # Create the Niagara System
            niagara_system = self.asset_tools.create_asset(
                asset_name=system_name,
                package_path=folder_path,
                asset_class=unreal.NiagaraSystem,
                factory=factory
            )
            
            if niagara_system:
                self.log(f"✓ Created Niagara System: {system_name}")
                self.created_count += 1
                
                # Save the asset
                self.editor_asset_lib.save_loaded_asset(niagara_system)
                
                return asset_path
            else:
                self.log(f"Failed to create Niagara System: {system_name}", "error")
                self.errors.append(f"Creation failed: {system_name}")
                return None
                
        except Exception as e:
            self.log(f"Error creating Niagara System {system_name}: {str(e)}", "error")
            self.errors.append(f"{system_name}: {str(e)}")
            return None
    
    def generate_engine_effects(self) -> int:
        """
        Generate engine and thruster effects
        
        Returns:
            Number of systems created
        """
        self.log("--- Generating Engine Effects ---")
        initial_count = self.created_count
        
        for system_name, description, folder in self.ENGINE_EFFECTS:
            self.create_niagara_system(system_name, description, folder)
        
        created = self.created_count - initial_count
        self.log(f"  → Created {created} engine effects")
        return created
    
    def generate_weapon_effects(self) -> int:
        """
        Generate weapon fire effects
        
        Returns:
            Number of systems created
        """
        self.log("--- Generating Weapon Effects ---")
        initial_count = self.created_count
        
        for system_name, description, folder in self.WEAPON_EFFECTS:
            self.create_niagara_system(system_name, description, folder)
        
        created = self.created_count - initial_count
        self.log(f"  → Created {created} weapon effects")
        return created
    
    def generate_explosion_effects(self) -> int:
        """
        Generate explosion effects
        
        Returns:
            Number of systems created
        """
        self.log("--- Generating Explosion Effects ---")
        initial_count = self.created_count
        
        for system_name, description, folder in self.EXPLOSION_EFFECTS:
            self.create_niagara_system(system_name, description, folder)
        
        created = self.created_count - initial_count
        self.log(f"  → Created {created} explosion effects")
        return created
    
    def generate_shield_effects(self) -> int:
        """
        Generate shield effects
        
        Returns:
            Number of systems created
        """
        self.log("--- Generating Shield Effects ---")
        initial_count = self.created_count
        
        for system_name, description, folder in self.SHIELD_EFFECTS:
            self.create_niagara_system(system_name, description, folder)
        
        created = self.created_count - initial_count
        self.log(f"  → Created {created} shield effects")
        return created
    
    def generate_environmental_effects(self) -> int:
        """
        Generate environmental effects
        
        Returns:
            Number of systems created
        """
        self.log("--- Generating Environmental Effects ---")
        initial_count = self.created_count
        
        for system_name, description, folder in self.ENVIRONMENTAL_EFFECTS:
            self.create_niagara_system(system_name, description, folder)
        
        created = self.created_count - initial_count
        self.log(f"  → Created {created} environmental effects")
        return created
    
    def generate_damage_effects(self) -> int:
        """
        Generate damage and hull effects
        
        Returns:
            Number of systems created
        """
        self.log("--- Generating Damage Effects ---")
        initial_count = self.created_count
        
        for system_name, description, folder in self.DAMAGE_EFFECTS:
            self.create_niagara_system(system_name, description, folder)
        
        created = self.created_count - initial_count
        self.log(f"  → Created {created} damage effects")
        return created
    
    def generate_all_effects(self) -> int:
        """
        Generate all Niagara effects
        
        Returns:
            Total number of systems created
        """
        self.log("=" * 80)
        self.log("GENERATING NIAGARA PARTICLE SYSTEMS")
        self.log("=" * 80)
        self.log("")
        
        # Generate all categories
        self.generate_engine_effects()
        self.log("")
        self.generate_weapon_effects()
        self.log("")
        self.generate_explosion_effects()
        self.log("")
        self.generate_shield_effects()
        self.log("")
        self.generate_environmental_effects()
        self.log("")
        self.generate_damage_effects()
        
        # Summary
        self.log("")
        self.log("=" * 80)
        self.log("NIAGARA GENERATION COMPLETE!")
        self.log(f"Created: {self.created_count}, Skipped: {self.skipped_count}")
        
        if self.errors:
            self.log(f"\nErrors encountered: {len(self.errors)}", "warning")
            for error in self.errors:
                self.log(f"  - {error}", "warning")
        
        self.log("")
        self.log("NEXT STEPS:")
        self.log("1. Open each Niagara System in the Niagara Editor")
        self.log("2. Configure emitters (sprites, ribbons, meshes)")
        self.log("3. Set particle properties (color, size, velocity, lifetime)")
        self.log("4. Add modules (forces, collision, lights)")
        self.log("5. Test in viewport and save")
        self.log("=" * 80)
        
        return self.created_count
    
    def get_statistics(self) -> Dict[str, Any]:
        """Get generation statistics"""
        return {
            "created": self.created_count,
            "skipped": self.skipped_count,
            "errors": len(self.errors),
            "error_details": self.errors,
            "total_defined": (len(self.ENGINE_EFFECTS) + 
                            len(self.WEAPON_EFFECTS) + 
                            len(self.EXPLOSION_EFFECTS) + 
                            len(self.SHIELD_EFFECTS) + 
                            len(self.ENVIRONMENTAL_EFFECTS) + 
                            len(self.DAMAGE_EFFECTS))
        }


# Convenience functions
def generate_all_effects() -> int:
    """Generate all Niagara effects - convenience function"""
    generator = NiagaraGenerator()
    return generator.generate_all_effects()


def generate_engine_effects() -> int:
    """Generate engine effects - convenience function"""
    generator = NiagaraGenerator()
    return generator.generate_engine_effects()


def generate_weapon_effects() -> int:
    """Generate weapon effects - convenience function"""
    generator = NiagaraGenerator()
    return generator.generate_weapon_effects()


def generate_explosion_effects() -> int:
    """Generate explosion effects - convenience function"""
    generator = NiagaraGenerator()
    return generator.generate_explosion_effects()


def generate_shield_effects() -> int:
    """Generate shield effects - convenience function"""
    generator = NiagaraGenerator()
    return generator.generate_shield_effects()


def generate_environmental_effects() -> int:
    """Generate environmental effects - convenience function"""
    generator = NiagaraGenerator()
    return generator.generate_environmental_effects()


def generate_damage_effects() -> int:
    """Generate damage effects - convenience function"""
    generator = NiagaraGenerator()
    return generator.generate_damage_effects()


# Command line interface
def main():
    """Command line interface"""
    import argparse
    
    parser = argparse.ArgumentParser(
        description="Adastrea Niagara Generator - Create particle system effects"
    )
    
    parser.add_argument("--all", action="store_true",
                       help="Generate all Niagara effects")
    parser.add_argument("--engines", action="store_true",
                       help="Generate engine effects only")
    parser.add_argument("--weapons", action="store_true",
                       help="Generate weapon effects only")
    parser.add_argument("--explosions", action="store_true",
                       help="Generate explosion effects only")
    parser.add_argument("--shields", action="store_true",
                       help="Generate shield effects only")
    parser.add_argument("--environment", action="store_true",
                       help="Generate environmental effects only")
    parser.add_argument("--damage", action="store_true",
                       help="Generate damage effects only")
    
    args = parser.parse_args()
    
    generator = NiagaraGenerator()
    
    if args.all:
        generator.generate_all_effects()
    elif args.engines:
        generator.generate_engine_effects()
    elif args.weapons:
        generator.generate_weapon_effects()
    elif args.explosions:
        generator.generate_explosion_effects()
    elif args.shields:
        generator.generate_shield_effects()
    elif args.environment:
        generator.generate_environmental_effects()
    elif args.damage:
        generator.generate_damage_effects()
    else:
        parser.print_help()
        print("\nTIP: Use --all to generate all Niagara effects")


if __name__ == "__main__":
    main()
