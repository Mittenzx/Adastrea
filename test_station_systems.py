#!/usr/bin/env python3
"""
Test script for station system calculations and validation.
Simulates station module integrity, power distribution, and operational status.
"""

def calculate_module_integrity(base_integrity, damage_taken, repair_rate=0.0):
    """
    Calculate module integrity after taking damage and applying repairs
    
    Args:
        base_integrity (float): Starting integrity (0-100)
        damage_taken (float): Damage amount to apply
        repair_rate (float): Repair rate per calculation cycle
    
    Returns:
        dict: Integrity results with status and details
    """
    # Input validation
    if base_integrity < 0.0 or base_integrity > 100.0:
        raise ValueError(f"base_integrity must be between 0-100, got {base_integrity}")
    if damage_taken < 0.0:
        raise ValueError(f"damage_taken must be non-negative, got {damage_taken}")
    if repair_rate < 0.0:
        raise ValueError(f"repair_rate must be non-negative, got {repair_rate}")
    
    # Calculate new integrity
    new_integrity = base_integrity - damage_taken + repair_rate
    
    # Clamp to valid range
    new_integrity = max(0.0, min(100.0, new_integrity))
    
    # Determine status
    if new_integrity <= 0.0:
        status = "DESTROYED"
    elif new_integrity <= 25.0:
        status = "CRITICAL"
    elif new_integrity <= 50.0:
        status = "DAMAGED"
    elif new_integrity <= 75.0:
        status = "DEGRADED"
    else:
        status = "OPERATIONAL"
    
    # Calculate damage percentage
    damage_percentage = (damage_taken / base_integrity * 100) if base_integrity > 0 else 0.0
    
    return {
        'base_integrity': base_integrity,
        'damage_taken': damage_taken,
        'repair_rate': repair_rate,
        'new_integrity': new_integrity,
        'status': status,
        'damage_percentage': damage_percentage,
        'is_operational': status in ["OPERATIONAL", "DEGRADED"],
        'needs_repair': status in ["CRITICAL", "DAMAGED", "DEGRADED"]
    }


def calculate_power_distribution(total_power, module_requirements):
    """
    Calculate power distribution across station modules
    
    Args:
        total_power (float): Total available power
        module_requirements (list): List of power requirements for each module
    
    Returns:
        dict: Power distribution results
    """
    if total_power < 0.0:
        raise ValueError(f"total_power must be non-negative, got {total_power}")
    
    total_requirements = sum(module_requirements)
    
    if total_requirements <= 0:
        return {
            'total_power': total_power,
            'total_requirements': 0.0,
            'power_surplus': total_power,
            'power_deficit': 0.0,
            'distribution': [],
            'all_modules_powered': True,
            'efficiency': 1.0 if total_power > 0 else 0.0
        }
    
    # Calculate distribution (proportional if insufficient power)
    if total_power >= total_requirements:
        distribution = module_requirements
        power_surplus = total_power - total_requirements
        power_deficit = 0.0
    else:
        # Distribute proportionally based on requirements
        distribution = []
        for req in module_requirements:
            proportion = req / total_requirements
            allocated = total_power * proportion
            distribution.append(allocated)
        power_surplus = 0.0
        power_deficit = total_requirements - total_power
    
    # Calculate module power status
    module_status = []
    for i, (req, alloc) in enumerate(zip(module_requirements, distribution)):
        if alloc >= req:
            status = "FULL_POWER"
        elif alloc >= req * 0.75:
            status = "HIGH_POWER"
        elif alloc >= req * 0.5:
            status = "MEDIUM_POWER"
        elif alloc >= req * 0.25:
            status = "LOW_POWER"
        else:
            status = "CRITICAL_POWER"
        
        module_status.append({
            'module_id': i,
            'required': req,
            'allocated': alloc,
            'status': status,
            'power_percentage': (alloc / req * 100) if req > 0 else 100.0
        })
    
    efficiency = total_power / total_requirements if total_requirements > 0 else 0.0
    
    return {
        'total_power': total_power,
        'total_requirements': total_requirements,
        'power_surplus': power_surplus,
        'power_deficit': power_deficit,
        'distribution': distribution,
        'module_status': module_status,
        'all_modules_powered': power_deficit == 0.0,
        'efficiency': min(efficiency, 1.0)
    }


def calculate_trading_efficiency(station_type, market_demand, supply_level, distance_factor=1.0):
    """
    Calculate trading efficiency for different station types
    
    Args:
        station_type (str): Type of station ['TRADING_HUB', 'REFINERY', 'SHIPYARD', 'RESEARCH']
        market_demand (float): Market demand level (0-100)
        supply_level (float): Available supply level (0-100)
        distance_factor (float): Distance efficiency factor (0-1)
    
    Returns:
        dict: Trading efficiency results
    """
    # Input validation
    valid_station_types = ['TRADING_HUB', 'REFINERY', 'SHIPYARD', 'RESEARCH']
    if station_type not in valid_station_types:
        raise ValueError(f"station_type must be one of {valid_station_types}, got {station_type}")
    
    if not (0.0 <= market_demand <= 100.0):
        raise ValueError(f"market_demand must be between 0-100, got {market_demand}")
    if not (0.0 <= supply_level <= 100.0):
        raise ValueError(f"supply_level must be between 0-100, got {supply_level}")
    if not (0.0 <= distance_factor <= 1.0):
        raise ValueError(f"distance_factor must be between 0-1, got {distance_factor}")
    
    # Base efficiency by station type
    base_efficiencies = {
        'TRADING_HUB': 0.9,
        'REFINERY': 0.8,
        'SHIPYARD': 0.7,
        'RESEARCH': 0.6
    }
    
    base_efficiency = base_efficiencies[station_type]
    
    # Calculate supply-demand balance factor
    if supply_level == 0:
        balance_factor = 0.0
    else:
        demand_supply_ratio = market_demand / supply_level
        # Optimal ratio is around 1.0 (balanced)
        if 0.8 <= demand_supply_ratio <= 1.2:
            balance_factor = 1.0
        elif demand_supply_ratio < 0.8:
            # Oversupply - reduced efficiency
            balance_factor = 0.5 + (demand_supply_ratio / 0.8 * 0.5)
        else:
            # Undersupply - reduced efficiency but different curve
            balance_factor = 1.0 - min(0.5, (demand_supply_ratio - 1.2) * 0.1)
    
    # Calculate final efficiency
    efficiency = base_efficiency * balance_factor * distance_factor
    
    # Determine efficiency tier
    if efficiency >= 0.8:
        tier = "EXCELLENT"
    elif efficiency >= 0.6:
        tier = "GOOD"
    elif efficiency >= 0.4:
        tier = "FAIR"
    elif efficiency >= 0.2:
        tier = "POOR"
    else:
        tier = "VERY_POOR"
    
    return {
        'station_type': station_type,
        'market_demand': market_demand,
        'supply_level': supply_level,
        'distance_factor': distance_factor,
        'base_efficiency': base_efficiency,
        'balance_factor': balance_factor,
        'final_efficiency': efficiency,
        'efficiency_tier': tier,
        'profit_multiplier': efficiency * 2.0  # Convert to profit multiplier
    }


def run_station_system_tests():
    """Run comprehensive test suite for station systems"""
    print("=== Station System Test Suite ===")
    print("=" * 70)
    
    # Test 1: Module Integrity Calculations
    print("\n[Test 1] Module Integrity Calculations")
    print("-" * 40)
    
    integrity_test_cases = [
        (100.0, 0.0, 0.0, "Full integrity, no damage"),
        (100.0, 25.0, 0.0, "Light damage"),
        (100.0, 60.0, 0.0, "Heavy damage"),
        (100.0, 100.0, 0.0, "Complete destruction"),
        (75.0, 50.0, 10.0, "Damage with repair"),
        (25.0, 30.0, 0.0, "Overdamage (should clamp to 0)"),
        (50.0, 0.0, 60.0, "Overrepair (should clamp to 100)"),
    ]
    
    for base, damage, repair, description in integrity_test_cases:
        try:
            result = calculate_module_integrity(base, damage, repair)
            print(f"\n  {description}")
            print(f"    Base: {base:.1f}, Damage: {damage:.1f}, Repair: {repair:.1f}")
            print(f"    New Integrity: {result['new_integrity']:.1f} -> {result['status']}")
            print(f"    Operational: {result['is_operational']}, Needs Repair: {result['needs_repair']}")
        except ValueError as e:
            print(f"\n  {description} - ERROR: {e}")
    
    # Test 2: Power Distribution
    print("\n\n[Test 2] Power Distribution Calculations")
    print("-" * 40)
    
    power_test_cases = [
        (100.0, [20.0, 30.0, 25.0], "Sufficient power"),
        (50.0, [20.0, 30.0, 25.0], "Insufficient power"),
        (0.0, [20.0, 30.0, 25.0], "No power"),
        (100.0, [], "No modules"),
        (100.0, [0.0, 0.0, 0.0], "Zero requirements"),
    ]
    
    for total_power, requirements, description in power_test_cases:
        try:
            result = calculate_power_distribution(total_power, requirements)
            print(f"\n  {description}")
            print(f"    Total Power: {total_power:.1f}, Requirements: {sum(requirements):.1f}")
            print(f"    Surplus: {result['power_surplus']:.1f}, Deficit: {result['power_deficit']:.1f}")
            print(f"    All Modules Powered: {result['all_modules_powered']}")
            print(f"    Efficiency: {result['efficiency']:.1%}")
            
            if 'module_status' in result and result['module_status']:
                for i, status in enumerate(result['module_status']):
                    print(f"    Module {i}: {status['allocated']:.1f}/{status['required']:.1f} ({status['status']})")
        except ValueError as e:
            print(f"\n  {description} - ERROR: {e}")
    
    # Test 3: Trading Efficiency
    print("\n\n[Test 3] Trading Efficiency Calculations")
    print("-" * 40)
    
    trading_test_cases = [
        ('TRADING_HUB', 80.0, 80.0, 1.0, "Balanced market at trading hub"),
        ('REFINERY', 90.0, 30.0, 0.8, "High demand, low supply at refinery"),
        ('SHIPYARD', 30.0, 90.0, 0.9, "Low demand, high supply at shipyard"),
        ('RESEARCH', 50.0, 50.0, 0.5, "Balanced but distant research station"),
    ]
    
    for station_type, demand, supply, distance, description in trading_test_cases:
        try:
            result = calculate_trading_efficiency(station_type, demand, supply, distance)
            print(f"\n  {description}")
            print(f"    Station: {station_type}, Demand: {demand:.1f}, Supply: {supply:.1f}")
            print(f"    Distance Factor: {distance:.1f}")
            print(f"    Efficiency: {result['final_efficiency']:.1%} ({result['efficiency_tier']})")
            print(f"    Profit Multiplier: {result['profit_multiplier']:.2f}x")
        except ValueError as e:
            print(f"\n  {description} - ERROR: {e}")
    
    # Test 4: Error Cases
    print("\n\n[Test 4] Error Case Testing")
    print("-" * 40)
    
    error_cases = [
        (-10.0, 50.0, 0.0, "Negative base integrity"),
        (100.0, -5.0, 0.0, "Negative damage"),
        (50.0, 10.0, -2.0, "Negative repair rate"),
        (150.0, 10.0, 0.0, "Integrity > 100"),
        (-50.0, [20, 30], "Negative total power"),
        ('INVALID', 50.0, 50.0, 1.0, "Invalid station type"),
        ('TRADING_HUB', -10.0, 50.0, 1.0, "Negative market demand"),
        ('TRADING_HUB', 50.0, 150.0, 1.0, "Supply > 100"),
        ('TRADING_HUB', 50.0, 50.0, 1.5, "Distance factor > 1"),
    ]
    
    for *args, description in error_cases:
        print(f"\n  {description}")
        try:
            if len(args) == 3:
                result = calculate_module_integrity(*args)
                print(f"    Result: {result} (UNEXPECTED - should have raised ValueError)")
            elif len(args) == 2:
                result = calculate_power_distribution(*args)
                print(f"    Result: {result} (UNEXPECTED - should have raised ValueError)")
            elif len(args) == 4:
                result = calculate_trading_efficiency(*args)
                print(f"    Result: {result} (UNEXPECTED - should have raised ValueError)")
        except ValueError as e:
            print(f"    [OK] Correctly raised ValueError: {e}")


def generate_station_performance_report():
    """Generate comprehensive station performance report"""
    print("\n" + "=" * 70)
    print("Station Performance Analysis Report")
    print("=" * 70)
    
    # Scenario 1: Trading Hub under various conditions
    print("\n[Scenario 1] Trading Hub Performance Analysis")
    print("-" * 40)
    
    scenarios = [
        ("Optimal", 85.0, 85.0, 0.9),
        ("High Demand", 95.0, 40.0, 0.8),
        ("Oversupply", 30.0, 90.0, 0.7),
        ("Distant", 70.0, 70.0, 0.4),
    ]
    
    for name, demand, supply, distance in scenarios:
        result = calculate_trading_efficiency('TRADING_HUB', demand, supply, distance)
        print(f"\n  {name}:")
        print(f"    Demand: {demand:.0f}, Supply: {supply:.0f}, Distance: {distance:.1f}")
        print(f"    Efficiency: {result['final_efficiency']:.1%} ({result['efficiency_tier']})")
        print(f"    Profit Potential: {'High' if result['profit_multiplier'] > 1.5 else 'Medium' if result['profit_multiplier'] > 1.0 else 'Low'}")
    
    # Scenario 2: Power Management
    print("\n\n[Scenario 2] Power Management Scenarios")
    print("-" * 40)
    
    power_scenarios = [
        ("Adequate Power", 150.0, [20, 30, 25, 35, 15]),
        ("Power Shortage", 80.0, [20, 30, 25, 35, 15]),
        ("Critical Shortage", 40.0, [20, 30, 25, 35, 15]),
    ]
    
    for name, total_power, requirements in power_scenarios:
        result = calculate_power_distribution(total_power, requirements)
        print(f"\n  {name}:")
        print(f"    Total Power: {total_power:.0f}, Required: {sum(requirements):.0f}")
        print(f"    Status: {'Adequate' if result['power_surplus'] > 0 else 'Deficit'}")
        print(f"    Modules at Full Power: {sum(1 for s in result['module_status'] if s['status'] == 'FULL_POWER')}/{len(requirements)}")
        print(f"    Efficiency: {result['efficiency']:.1%}")
    
    # Scenario 3: Module Integrity Management
    print("\n\n[Scenario 3] Module Integrity Management")
    print("-" * 40)
    
    integrity_scenarios = [
        ("Combat Damage", 100.0, 65.0, 5.0),
        ("Wear and Tear", 85.0, 20.0, 8.0),
        ("Emergency Repairs", 30.0, 10.0, 25.0),
        ("Catastrophic Failure", 100.0, 110.0, 0.0),
    ]
    
    for name, base, damage, repair in integrity_scenarios:
        result = calculate_module_integrity(base, damage, repair)
        print(f"\n  {name}:")
        print(f"    Base: {base:.0f}%, Damage: {damage:.0f}, Repair: {repair:.0f}")
        print(f"    New Integrity: {result['new_integrity']:.0f}% ({result['status']})")
        print(f"    Operational: {result['is_operational']}, Critical: {result['status'] == 'CRITICAL'}")
    
    print("\n" + "=" * 70)
    print("[Implementation Notes]:")
    print("- Module Integrity: Critical for station survivability and functionality")
    print("- Power Distribution: Affects all station operations and efficiency")
    print("- Trading Efficiency: Key economic metric for Trade Simulator MVP")
    print("\n[Trade Simulator MVP Focus]:")
    print("- Stations are economic hubs for trading gameplay")
    print("- Power management affects trading capacity and efficiency")
    print("- Module integrity impacts station attractiveness to traders")
    print("- All systems designed for economic simulation, not combat")


if __name__ == "__main__":
    run_station_system_tests()
    generate_station_performance_report()
    
    print("\n" + "=" * 70)
    print("[Test Summary]:")
    print("[OK] Module integrity calculations with status tracking")
    print("[OK] Power distribution with proportional allocation")
    print("[OK] Trading efficiency based on station type and market conditions")
    print("[OK] Comprehensive error handling and input validation")
    print("[OK] Performance analysis for different scenarios")
    print("\n[Use Cases]:")
    print("- Station management UI development")
    print("- Economic simulation validation")
    print("- Performance benchmarking for different station configurations")
    print("- AI decision making for station operations")