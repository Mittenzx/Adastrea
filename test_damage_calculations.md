# Damage Calculation Test Suite

## Overview
This Python test script simulates the C++ damage calculation logic from `AdastreaFunctionLibrary.cpp` for verification and analysis purposes.

## Purpose
- **Verification**: Ensure Python implementation matches C++ logic
- **Analysis**: Test damage type effectiveness against different armor values
- **Documentation**: Provide clear examples of damage calculation behavior
- **Development**: Rapid iteration without Unreal Engine compilation

## Damage Types
The system implements 5 damage types with different armor penetration characteristics:

1. **Kinetic** - Standard armor penetration
   - Formula: `damage * (1 - armor_reduction)`
   - Standard effectiveness against armor

2. **Energy** - Effective against armor (bypasses 30%)
   - Formula: `damage * (1 - (armor_reduction * 0.7))`
   - 30% more effective against armor than kinetic

3. **Explosive** - Less effective against armor (+20% reduction)
   - Formula: `damage * (1 - (armor_reduction * 1.2))`
   - 20% less effective against armor than kinetic

4. **Thermal** - Minimal armor effect (bypasses 70%)
   - Formula: `damage * (1 - (armor_reduction * 0.3))`
   - 70% more effective against armor than kinetic

5. **EMP** - Ignores armor completely
   - Formula: `damage * 1.0`
   - Full damage regardless of armor

## Armor Reduction Formula
```
armor_reduction = armor / (armor + 100.0)
```

This creates a diminishing returns curve where:
- 0 armor = 0% reduction
- 50 armor = 33.3% reduction
- 100 armor = 50% reduction
- 200 armor = 66.7% reduction
- 500 armor = 83.3% reduction

## Test Suite Features

### 1. Basic Test Cases
- No armor scenarios
- Medium armor (50) against all damage types
- Heavy armor (100) scenarios
- Edge cases (high armor vs low damage)

### 2. Comparative Analysis
Shows damage effectiveness for 100 damage vs 50 armor:
- Kinetic: 66.7 damage (66.7% of original)
- Energy: 76.7 damage (76.7% of original)
- Explosive: 60.0 damage (60.0% of original)
- Thermal: 90.0 damage (90.0% of original)
- EMP: 100.0 damage (100.0% of original)

### 3. Effectiveness Matrix
Generates a matrix showing damage received for each damage type against armor values from 0 to 200.

## Usage
```bash
# Run the test suite
python test_damage_calculations.py

# Expected output includes:
# - Test case results with damage blocked percentages
# - Comparative analysis table
# - Effectiveness matrix
```

## Integration with C++ Code
This Python implementation matches the C++ logic in:
- `AdastreaFunctionLibrary.cpp::CalculateDamageAfterArmor()`
- Damage type enum mapping (Kinetic=0, Energy=1, Explosive=2, Thermal=3, EMP=4)

## Development Notes
- Created as part of endless development cycle
- Useful for balancing combat system post-MVP
- Can be extended for more complex damage models
- Provides quick feedback without UE compilation

## Future Enhancements
1. Add critical hit calculations
2. Implement shield penetration mechanics
3. Add damage over time (DoT) effects
4. Create visual damage effectiveness charts
5. Integrate with automated testing framework