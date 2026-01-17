#!/usr/bin/env python3
"""
Trading System Integration Tests

Tests for MVP trading system to ensure economic balance and functionality.
Run these tests before deploying trade content changes.

Usage:
    pytest test_trading_integration.py
    python test_trading_integration.py  # Standalone mode
"""

import sys
from pathlib import Path

# Add project root to path
project_root = Path(__file__).parent.parent
sys.path.insert(0, str(project_root / "Tools"))

try:
    from generate_mvp_trading_content import MVPTradingContentGenerator
    from validate_trade_content import TradeContentValidator
    from trade_profit_calculator import TradeProfitCalculator
except ImportError as e:
    print(f"Failed to import trading tools: {e}")
    sys.exit(1)


class TestTradingContentGeneration:
    """Test content generation pipeline"""
    
    def test_generator_creates_items(self):
        """Test that generator creates all expected trade items"""
        gen = MVPTradingContentGenerator()
        assert gen.generate_trade_items(), "Failed to generate trade items"
        
        items_dir = gen.output_dir / "Items"
        assert items_dir.exists(), "Items directory not created"
        
        # Should have 15 items
        item_files = list(items_dir.glob("*.txt"))
        assert len(item_files) == 15, f"Expected 15 items, got {len(item_files)}"
    
    def test_generator_creates_markets(self):
        """Test that generator creates all expected markets"""
        gen = MVPTradingContentGenerator()
        assert gen.generate_markets(), "Failed to generate markets"
        
        markets_dir = gen.output_dir / "Markets"
        assert markets_dir.exists(), "Markets directory not created"
        
        # Should have 5 markets
        market_files = list(markets_dir.glob("*.txt"))
        assert len(market_files) == 5, f"Expected 5 markets, got {len(market_files)}"


class TestTradingValidation:
    """Test content validation"""
    
    def test_all_items_valid(self):
        """Test that all trade items pass validation"""
        validator = TradeContentValidator()
        assert validator.validate_trade_items(), "Trade items validation failed"
        assert len(validator.errors) == 0, f"Validation errors: {validator.errors}"
    
    def test_all_markets_valid(self):
        """Test that all markets pass validation"""
        validator = TradeContentValidator()
        assert validator.validate_markets(), "Markets validation failed"
        assert len(validator.errors) == 0, f"Validation errors: {validator.errors}"
    
    def test_profitable_routes_exist(self):
        """Test that profitable trade routes exist"""
        validator = TradeContentValidator()
        validator.validate_economy()
        
        # Should find at least 10 profitable routes
        # This is extracted from validator's economy check
        calc = TradeProfitCalculator()
        routes = calc.find_best_routes(starting_capital=1000.0, cargo_capacity=10.0)
        
        assert len(routes) >= 10, f"Expected 10+ profitable routes, found {len(routes)}"


class TestEconomicBalance:
    """Test economic balance and progression"""
    
    def test_starter_ship_can_afford_trades(self):
        """Test that starter ship (1000cr, 10 cargo) can make trades"""
        calc = TradeProfitCalculator()
        routes = calc.find_best_routes(starting_capital=1000.0, cargo_capacity=10.0)
        
        # At least 3 routes should be available
        assert len(routes) >= 3, "Starter ship can't afford enough trades"
        
        # At least one route should give 30%+ profit
        profitable = [r for r in routes if r['roi_percent'] >= 30]
        assert len(profitable) >= 1, "No sufficiently profitable routes for starter"
    
    def test_price_spreads_reasonable(self):
        """Test that price spreads between markets are reasonable"""
        calc = TradeProfitCalculator()
        
        # Check a few key items
        test_items = ['water_purified', 'basic_alloys', 'basic_computers']
        test_markets = ['agricultural_station_market', 'industrial_station_market']
        
        for item_id in test_items:
            prices = []
            for market_id in test_markets:
                buy_price = calc.calculate_price(item_id, market_id, is_buying=True)
                if buy_price > 0:
                    prices.append(buy_price)
            
            if len(prices) >= 2:
                min_price = min(prices)
                max_price = max(prices)
                spread = max_price / min_price
                
                # Spread should be 1.2x to 5x (reasonable range)
                assert 1.2 <= spread <= 5.0, \
                    f"{item_id}: Price spread too extreme ({spread:.1f}x)"
    
    def test_cargo_capacity_balanced(self):
        """Test that items fit reasonably in cargo"""
        calc = TradeProfitCalculator()
        
        # Starter cargo should fit at least 5 different item types
        starter_cargo = 10.0
        affordable_items = []
        
        for item_id in calc.items:
            item = calc.items[item_id]
            volume = item['volume_per_unit']
            
            # Can fit at least 1 unit?
            if volume <= starter_cargo:
                affordable_items.append(item_id)
        
        assert len(affordable_items) >= 5, \
            f"Only {len(affordable_items)} items fit in starter cargo"
    
    def test_progression_viable(self):
        """Test that ship upgrade progression is achievable"""
        calc = TradeProfitCalculator()
        
        # Starter ship should earn enough for medium ship (50,000cr) in reasonable time
        routes = calc.find_best_routes(starting_capital=1000.0, cargo_capacity=10.0)
        
        if routes:
            best_profit = routes[0]['profit']
            
            # Number of trips needed for 50,000cr
            upgrade_cost = 50000
            trips_needed = upgrade_cost / best_profit if best_profit > 0 else 999
            
            # Should be achievable in 15-40 trips (45 min - 2 hours at 3 min/trip)
            assert 15 <= trips_needed <= 40, \
                f"Ship upgrade requires {trips_needed:.0f} trips (too many or too few)"


class TestMarketDiversity:
    """Test market economic diversity"""
    
    def test_markets_have_different_profiles(self):
        """Test that markets have distinct economic profiles"""
        calc = TradeProfitCalculator()
        
        # Check that agricultural and industrial have opposite profiles
        # for water (agricultural produces, industrial imports)
        water_ag_buy = calc.calculate_price(
            'water_purified', 'agricultural_station_market', is_buying=True
        )
        water_ind_buy = calc.calculate_price(
            'water_purified', 'industrial_station_market', is_buying=True
        )
        
        # Agricultural should be cheaper (produces water)
        assert water_ag_buy < water_ind_buy, \
            "Agricultural station not cheaper for water"
        
        # Check industrial produces metals cheaper
        metal_ag_buy = calc.calculate_price(
            'basic_alloys', 'agricultural_station_market', is_buying=True
        )
        metal_ind_buy = calc.calculate_price(
            'basic_alloys', 'industrial_station_market', is_buying=True
        )
        
        # Industrial should be cheaper (produces metals)
        assert metal_ind_buy < metal_ag_buy, \
            "Industrial station not cheaper for metals"
    
    def test_luxury_market_expensive(self):
        """Test that luxury market has higher prices"""
        calc = TradeProfitCalculator()
        
        # Check luxury market markup
        luxury_market = 'luxury_bazaar_market'
        trade_hub = 'trade_hub_market'
        
        # Sample item
        item_id = 'entertainment_systems'
        
        luxury_price = calc.calculate_price(item_id, luxury_market, is_buying=True)
        hub_price = calc.calculate_price(item_id, trade_hub, is_buying=True)
        
        if luxury_price > 0 and hub_price > 0:
            # Luxury should be 1.5x+ more expensive
            assert luxury_price >= hub_price * 1.5, \
                "Luxury market not significantly more expensive"


def run_tests():
    """Run all tests manually (no pytest required)"""
    import traceback
    
    test_classes = [
        TestTradingContentGeneration,
        TestTradingValidation,
        TestEconomicBalance,
        TestMarketDiversity
    ]
    
    total_tests = 0
    passed_tests = 0
    failed_tests = []
    
    print("=" * 60)
    print("TRADING SYSTEM INTEGRATION TESTS")
    print("=" * 60)
    print()
    
    for test_class in test_classes:
        class_name = test_class.__name__
        print(f"\n{class_name}")
        print("-" * 60)
        
        test_instance = test_class()
        
        # Get all test methods
        test_methods = [m for m in dir(test_instance) if m.startswith('test_')]
        
        for method_name in test_methods:
            total_tests += 1
            method = getattr(test_instance, method_name)
            
            try:
                method()
                print(f"  ✓ {method_name}")
                passed_tests += 1
            except AssertionError as e:
                print(f"  ✗ {method_name}")
                print(f"    Error: {e}")
                failed_tests.append((class_name, method_name, str(e)))
            except Exception as e:
                print(f"  ✗ {method_name}")
                print(f"    Exception: {e}")
                traceback.print_exc()
                failed_tests.append((class_name, method_name, str(e)))
    
    # Summary
    print("\n" + "=" * 60)
    print("TEST SUMMARY")
    print("=" * 60)
    print(f"Total Tests: {total_tests}")
    print(f"Passed: {passed_tests}")
    print(f"Failed: {len(failed_tests)}")
    
    if failed_tests:
        print("\nFailed Tests:")
        for class_name, method_name, error in failed_tests:
            print(f"  • {class_name}.{method_name}")
            print(f"    {error}")
        return 1
    else:
        print("\n✅ All tests passed!")
        return 0


if __name__ == "__main__":
    # Check if pytest is available
    try:
        import pytest
        print("Running with pytest...")
        sys.exit(pytest.main([__file__, "-v"]))
    except ImportError:
        print("pytest not available, running in standalone mode...")
        sys.exit(run_tests())
