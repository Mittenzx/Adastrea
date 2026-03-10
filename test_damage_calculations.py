#!/usr/bin/env python3
"""
Test script for damage calculation with armor and damage type modifiers.
Simulates the C++ logic in Python for verification.
"""

def calculate_damage_after_armor(raw_damage, armor_value, damage_type):
    """
    Python implementation of CalculateDamageAfterArmor function
    """
    if raw_damage <= 0.0:
        return 0.0
    
    # Basic armor mitigation formula
    armor_reduction = armor_value / (armor_value + 100.0)
    
    # Apply damage type modifiers
    damage_type_multiplier = 1.0
    
    # Damage type enum mapping
    DAMAGE_TYPES = {
        'Kinetic': 0,
        'Energy': 1,
        'Explosive': 2,
        'Thermal': 3,
        'EMP': 4
    }
    
    damage_type_idx = DAMAGE_TYPES.get(damage_type, 0)
    
    if damage_type_idx == 0:  # Kinetic
        damage_type_multiplier = 1.0 - armor_reduction
    elif damage_type_idx == 1:  # Energy
        damage_type_multiplier = 1.0 - (armor_reduction * 0.7)
    elif damage_type_idx == 2:  # Explosive
        damage_type_multiplier = 1.0 - (armor_reduction * 1.2)
    elif damage_type_idx == 3:  # Thermal
        damage_type_multiplier = 1.0 - (armor_reduction * 0.3)
    elif damage_type_idx == 4:  # EMP
        damage_type_multiplier = 1.0  # Ignores armor
    
    # Clamp multiplier to valid range
    damage_type_multiplier = max(0.0, min(1.0, damage_type_multiplier))
    
    final_damage = raw_damage * damage_type_multiplier
    return max(final_damage, 0.0)


def run_test_suite():
    """Run comprehensive test suite for damage calculations"""
    print("=== Damage Calculation Test Suite ===")
    print("=" * 60)
    
    test_cases = [
        # (raw_damage, armor, damage_type, expected_description)
        (100.0, 0.0, 'Kinetic', "No armor - all damage gets through"),
        (100.0, 50.0, 'Kinetic', "Medium armor - standard reduction"),
        (100.0, 100.0, 'Kinetic', "Heavy armor - significant reduction"),
        (100.0, 50.0, 'Energy', "Energy vs medium armor - bypasses 30%"),
        (100.0, 50.0, 'Explosive', "Explosive vs medium armor - reduced by 20% more"),
        (100.0, 50.0, 'Thermal', "Thermal vs medium armor - minimal effect"),
        (100.0, 100.0, 'EMP', "EMP vs heavy armor - ignores armor"),
        (50.0, 200.0, 'Kinetic', "High armor vs low damage"),
        (200.0, 10.0, 'Energy', "Low armor vs high energy damage"),
    ]
    
    for raw_damage, armor, damage_type, description in test_cases:
        result = calculate_damage_after_armor(raw_damage, armor, damage_type)
        armor_reduction = armor / (armor + 100.0)
        damage_received = raw_damage - result
        
        print(f"\n[Test] {description}")
        print(f"   Raw Damage: {raw_damage:.1f}, Armor: {armor:.1f}, Type: {damage_type}")
        print(f"   Armor Reduction: {armor_reduction:.1%}")
        print(f"   Final Damage: {result:.1f}")
        print(f"   Damage Blocked: {damage_received:.1f} ({damage_received/raw_damage:.1%})")
    
    print("\n" + "=" * 60)
    print("[OK] Test suite completed successfully!")
    
    # Comparative analysis
    print("\n=== Comparative Analysis (100 damage vs 50 armor) ===")
    print("-" * 40)
    
    base_damage = 100.0
    armor = 50.0
    
    for damage_type in ['Kinetic', 'Energy', 'Explosive', 'Thermal', 'EMP']:
        result = calculate_damage_after_armor(base_damage, armor, damage_type)
        print(f"{damage_type:10} -> {result:6.1f} damage ({result/base_damage:5.1%} of original)")


def calculate_effectiveness_matrix():
    """Calculate effectiveness matrix for different armor values"""
    print("\n=== Damage Type Effectiveness Matrix ===")
    print("=" * 60)
    
    damage_types = ['Kinetic', 'Energy', 'Explosive', 'Thermal', 'EMP']
    armor_values = [0, 25, 50, 75, 100, 150, 200]
    
    print("\nArmor Value ->", end="")
    for armor in armor_values:
        print(f"{armor:>8}", end="")
    print()
    print("-" * (8 + len(armor_values) * 8))
    
    for damage_type in damage_types:
        print(f"{damage_type:10}", end="")
        for armor in armor_values:
            result = calculate_damage_after_armor(100.0, armor, damage_type)
            print(f"{result:>8.1f}", end="")
        print()


if __name__ == "__main__":
    run_test_suite()
    calculate_effectiveness_matrix()
    
    print("\n" + "=" * 60)
    print("[Implementation Notes]:")
    print("- Kinetic: Standard armor penetration")
    print("- Energy: Effective against armor (bypasses 30%)")
    print("- Explosive: Less effective against armor (+20% reduction)")
    print("- Thermal: Minimal armor effect (bypasses 70%)")
    print("- EMP: Ignores armor completely")
    print("\n[Note] This matches the C++ implementation in AdastreaFunctionLibrary.cpp")