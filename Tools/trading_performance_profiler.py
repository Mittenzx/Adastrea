#!/usr/bin/env python3
"""
Trading System Performance Profiler

Analyzes trading system performance characteristics:
- Market lookup times
- Price calculation complexity
- Inventory query efficiency
- Trade route pathfinding costs

Helps identify bottlenecks before implementation.

Usage:
    python3 Tools/trading_performance_profiler.py --analyze
    python3 Tools/trading_performance_profiler.py --stress-test
"""

import argparse
import time
import yaml
from pathlib import Path

class TradingPerformanceProfiler:
    """Profiles trading system performance"""
    
    def __init__(self):
        self.repo_root = Path(__file__).parent.parent
        self.items_file = self.repo_root / "Assets/TradingTemplates/MVPTradeItems.yaml"
        self.markets_file = self.repo_root / "Assets/TradingTemplates/MVPMarkets.yaml"
        
        self.items = []
        self.markets = []
        
    def load_content(self):
        """Load trading content from YAML"""
        with open(self.items_file, 'r') as f:
            data = yaml.safe_load(f)
            # Handle both list and dict formats
            if isinstance(data, list):
                self.items = data
            else:
                self.items = data.get('trade_items', [])
            
        with open(self.markets_file, 'r') as f:
            data = yaml.safe_load(f)
            # Handle both list and dict formats
            if isinstance(data, list):
                self.markets = data
            else:
                self.markets = data.get('markets', [])
            
        print(f"✅ Loaded {len(self.items)} items and {len(self.markets)} markets")
        
    def profile_market_lookups(self):
        """Profile market lookup operations"""
        print("\n📊 Profiling Market Lookups...")
        
        # Simulate 1000 market lookups
        iterations = 1000
        start = time.time()
        
        for i in range(iterations):
            # Simulate finding a market by ID (linear search worst case)
            target = self.markets[i % len(self.markets)]
            for market in self.markets:
                if market['market_id'] == target['market_id']:
                    break
                    
        elapsed = time.time() - start
        avg_lookup = (elapsed / iterations) * 1000  # Convert to ms
        
        print(f"  ⏱️  {iterations} lookups: {elapsed:.4f}s")
        print(f"  📈 Average lookup time: {avg_lookup:.6f}ms")
        
        if avg_lookup < 0.01:
            print(f"  ✅ EXCELLENT - Lookups are very fast")
        elif avg_lookup < 0.1:
            print(f"  ✅ GOOD - Acceptable performance")
        else:
            print(f"  ⚠️  NEEDS OPTIMIZATION - Consider indexing")
            
        return avg_lookup
        
    def profile_price_calculations(self):
        """Profile price calculation operations"""
        print("\n💰 Profiling Price Calculations...")
        
        # Simulate 10000 price calculations
        iterations = 10000
        start = time.time()
        
        for i in range(iterations):
            item = self.items[i % len(self.items)]
            market = self.markets[i % len(self.markets)]
            
            # Simulate price calculation
            base_price = item['base_price']
            markup = market.get('sell_price_markup', 1.2)
            inventory = market.get('inventory', [])
            
            # Find item in market inventory
            supply = 1.0
            demand = 1.0
            for inv_item in inventory:
                if inv_item['item_id'] == item['item_id']:
                    supply = inv_item.get('supply_level', 1.0)
                    demand = inv_item.get('demand_level', 1.0)
                    break
                    
            # Calculate final price
            price_modifier = (demand / supply) if supply > 0 else 1.0
            _ = base_price * markup * price_modifier  # Simulate calculation
            
        elapsed = time.time() - start
        avg_calc = (elapsed / iterations) * 1000  # Convert to ms
        
        print(f"  ⏱️  {iterations} calculations: {elapsed:.4f}s")
        print(f"  📈 Average calculation time: {avg_calc:.6f}ms")
        
        if avg_calc < 0.01:
            print(f"  ✅ EXCELLENT - Calculations are very fast")
        elif avg_calc < 0.1:
            print(f"  ✅ GOOD - Acceptable performance")
        else:
            print(f"  ⚠️  NEEDS OPTIMIZATION - Consider caching")
            
        return avg_calc
        
    def profile_inventory_queries(self):
        """Profile inventory query operations"""
        print("\n📦 Profiling Inventory Queries...")
        
        # Simulate 5000 inventory queries
        iterations = 5000
        start = time.time()
        
        for i in range(iterations):
            item = self.items[i % len(self.items)]
            market = self.markets[i % len(self.markets)]
            
            # Check if item is in stock
            inventory = market.get('inventory', [])
            found = False
            current_stock = 0
            
            for inv_item in inventory:
                if inv_item['item_id'] == item['item_id']:
                    found = True
                    current_stock = inv_item.get('current_stock', 0)
                    break
                    
            # Simulate quantity check
            needed = 10
            _ = found and current_stock >= needed  # Simulate check
            
        elapsed = time.time() - start
        avg_query = (elapsed / iterations) * 1000  # Convert to ms
        
        print(f"  ⏱️  {iterations} queries: {elapsed:.4f}s")
        print(f"  📈 Average query time: {avg_query:.6f}ms")
        
        if avg_query < 0.05:
            print(f"  ✅ EXCELLENT - Queries are very fast")
        elif avg_query < 0.2:
            print(f"  ✅ GOOD - Acceptable performance")
        else:
            print(f"  ⚠️  NEEDS OPTIMIZATION - Consider indexing inventory")
            
        return avg_query
        
    def profile_trade_route_calculation(self):
        """Profile trade route pathfinding"""
        print("\n🛣️  Profiling Trade Route Calculations...")
        
        # Simulate 100 full trade route calculations
        iterations = 100
        start = time.time()
        
        for i in range(iterations):
            # For each item, calculate profitability across all market pairs
            routes_analyzed = 0
            
            for item in self.items:
                for buy_market in self.markets:
                    # Get buy price
                    buy_inventory = buy_market.get('inventory', [])
                    buy_price = item['base_price']
                    buy_markdown = buy_market.get('buy_price_markdown', 0.8)
                    
                    for inv in buy_inventory:
                        if inv['item_id'] == item['item_id']:
                            supply = inv.get('supply_level', 1.0)
                            demand = inv.get('demand_level', 1.0)
                            buy_price = item['base_price'] * buy_markdown * (supply / demand if demand > 0 else 1.0)
                            break
                    
                    for sell_market in self.markets:
                        if sell_market['market_id'] == buy_market['market_id']:
                            continue
                            
                        # Get sell price
                        sell_inventory = sell_market.get('inventory', [])
                        sell_price = item['base_price']
                        sell_markup = sell_market.get('sell_price_markup', 1.2)
                        
                        for inv in sell_inventory:
                            if inv['item_id'] == item['item_id']:
                                supply = inv.get('supply_level', 1.0)
                                demand = inv.get('demand_level', 1.0)
                                sell_price = item['base_price'] * sell_markup * (demand / supply if supply > 0 else 1.0)
                                break
                        
                        # Calculate profit (unused but simulates calculation)
                        _ = sell_price - buy_price
                        routes_analyzed += 1
                        
        elapsed = time.time() - start
        avg_route_calc = (elapsed / iterations) * 1000  # Convert to ms
        
        print(f"  ⏱️  {iterations} full route analyses: {elapsed:.4f}s")
        print(f"  📈 Average full analysis: {avg_route_calc:.2f}ms")
        print(f"  🔢 Routes per analysis: {routes_analyzed // iterations}")
        
        if avg_route_calc < 50:
            print(f"  ✅ EXCELLENT - Route calculations are fast")
        elif avg_route_calc < 200:
            print(f"  ✅ GOOD - Acceptable performance")
        else:
            print(f"  ⚠️  NEEDS OPTIMIZATION - Consider caching or spatial indexing")
            
        return avg_route_calc
        
    def stress_test(self):
        """Run stress test with 100+ markets and 100+ items"""
        print("\n🔥 Running Stress Test (Scaled Content)...")
        
        # Simulate 10x content scale
        # Duplicate items and markets 10x
        scaled_items = self.items * 10
        scaled_markets = []
        
        for i in range(10):
            for market in self.markets:
                scaled_market = market.copy()
                scaled_market['market_id'] = f"{market['market_id']}_scaled_{i}"
                scaled_markets.append(scaled_market)
        
        print(f"  📊 Scaled to {len(scaled_items)} items and {len(scaled_markets)} markets")
        
        # Backup original
        backup_items = self.items
        backup_markets = self.markets
        
        # Replace with scaled
        self.items = scaled_items
        self.markets = scaled_markets
        
        # Run profiles
        print(f"\n  🧪 Testing with 10x scale...")
        lookup_time = self.profile_market_lookups()
        price_time = self.profile_price_calculations()
        inventory_time = self.profile_inventory_queries()
        
        # Restore original
        self.items = backup_items
        self.markets = backup_markets
        
        print(f"\n  📈 Stress Test Summary:")
        print(f"    - Market lookups: {lookup_time:.6f}ms")
        print(f"    - Price calculations: {price_time:.6f}ms")
        print(f"    - Inventory queries: {inventory_time:.6f}ms")
        
        if lookup_time < 0.1 and price_time < 0.1 and inventory_time < 0.2:
            print(f"  ✅ PASS - System scales well to large content")
        else:
            print(f"  ⚠️  WARNING - Performance degradation at scale")
            print(f"  💡 Recommend: Implement spatial indexing and caching")
            
    def analyze_all(self):
        """Run all performance analyses"""
        print("=" * 60)
        print("Trading System Performance Analysis")
        print("=" * 60)
        
        self.load_content()
        
        lookup_time = self.profile_market_lookups()
        price_time = self.profile_price_calculations()
        inventory_time = self.profile_inventory_queries()
        route_time = self.profile_trade_route_calculation()
        
        print("\n" + "=" * 60)
        print("📊 Performance Summary")
        print("=" * 60)
        print(f"Market Lookups:       {lookup_time:.6f}ms")
        print(f"Price Calculations:   {price_time:.6f}ms")
        print(f"Inventory Queries:    {inventory_time:.6f}ms")
        print(f"Route Calculations:   {route_time:.2f}ms")
        
        # Overall grade
        max_time = max(lookup_time, price_time, inventory_time)
        if max_time < 0.05 and route_time < 100:
            grade = "A+ (Excellent)"
        elif max_time < 0.1 and route_time < 200:
            grade = "A (Very Good)"
        elif max_time < 0.5 and route_time < 500:
            grade = "B (Good)"
        else:
            grade = "C (Needs Optimization)"
            
        print(f"\n🎯 Overall Grade: {grade}")
        
        # Recommendations
        print("\n💡 Recommendations:")
        if lookup_time > 0.05:
            print("  - Consider using TMap<FName, Market*> for O(1) market lookups")
        if price_time > 0.05:
            print("  - Consider caching calculated prices with dirty flags")
        if inventory_time > 0.1:
            print("  - Consider indexing inventory by ItemID in market")
        if route_time > 200:
            print("  - Consider spatial partitioning for station locations")
            print("  - Consider pre-calculating common routes")
        
        if max_time < 0.05 and route_time < 100:
            print("  ✅ No optimization needed - performance is excellent!")
            
        print("\n" + "=" * 60)

def main():
    parser = argparse.ArgumentParser(description="Profile trading system performance")
    parser.add_argument('--analyze', action='store_true', help='Run performance analysis')
    parser.add_argument('--stress-test', action='store_true', help='Run stress test with scaled content')
    
    args = parser.parse_args()
    
    profiler = TradingPerformanceProfiler()
    
    if args.stress_test:
        profiler.load_content()
        profiler.stress_test()
    elif args.analyze or len([arg for arg in vars(args) if getattr(args, arg)]) == 0:
        # Default: run analysis
        profiler.analyze_all()
    
if __name__ == '__main__':
    main()
