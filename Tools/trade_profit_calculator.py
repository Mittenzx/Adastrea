#!/usr/bin/env python3
"""
Trade Route Profit Calculator

Interactive tool for calculating trade route profitability before implementing in-game.
Helps designers optimize trading economics and validate profitable routes.

Usage:
    python trade_profit_calculator.py [--interactive] [--route <config>]
"""

import yaml
import sys
from pathlib import Path
from typing import Dict, List, Tuple


class TradeProfitCalculator:
    """Calculates and optimizes trade route profitability"""
    
    def __init__(self, project_root=None):
        if project_root is None:
            project_root = Path(__file__).parent.parent
        self.project_root = Path(project_root)
        self.templates_dir = self.project_root / "Assets" / "TradingTemplates"
        
        # Load data
        self.items = {}
        self.markets = {}
        self.load_data()
        
    def load_data(self):
        """Load trade items and markets"""
        items_file = self.templates_dir / "MVPTradeItems.yaml"
        markets_file = self.templates_dir / "MVPMarkets.yaml"
        
        if items_file.exists():
            with open(items_file, 'r') as f:
                items_list = yaml.safe_load(f)
                self.items = {item['item_id']: item for item in items_list}
        
        if markets_file.exists():
            with open(markets_file, 'r') as f:
                markets_list = yaml.safe_load(f)
                self.markets = {market['market_id']: market for market in markets_list}
    
    def calculate_price(self, item_id: str, market_id: str, is_buying: bool) -> float:
        """Calculate price for an item at a market"""
        if item_id not in self.items or market_id not in self.markets:
            return 0.0
        
        item = self.items[item_id]
        market = self.markets[market_id]
        
        # Find inventory entry for this item
        inventory_entry = None
        for entry in market.get('inventory', []):
            if entry['item_id'] == item_id:
                inventory_entry = entry
                break
        
        if not inventory_entry:
            return 0.0  # Item not available at this market
        
        base_price = item['base_price']
        supply = inventory_entry['supply_level']
        demand = inventory_entry['demand_level']
        
        # Supply/demand calculation
        if item.get('affected_by_supply_demand', True):
            supply_factor = 1.0 / max(supply, 0.1)
            demand_factor = demand
            
            # Clamp to volatility bounds
            min_dev = item['min_price_deviation']
            max_dev = item['max_price_deviation']
            volatility = item['volatility_multiplier']
            
            supply_factor = max(min_dev, min(supply_factor, max_dev))
            demand_factor = max(min_dev, min(demand_factor, max_dev))
            
            base_price *= supply_factor * demand_factor * volatility
        
        # Apply market markup/markdown
        if is_buying:
            base_price *= market['sell_price_markup']
        else:
            base_price *= market['buy_price_markdown']
        
        # Apply transaction tax
        tax_rate = market.get('transaction_tax_rate', 0.0)
        base_price *= (1.0 + tax_rate)
        
        return round(base_price, 2)
    
    def calculate_route_profit(self, item_id: str, buy_market: str, sell_market: str, 
                               quantity: int, cargo_capacity: float = 10.0) -> Dict:
        """Calculate profit for a specific trade route"""
        if item_id not in self.items:
            return {'error': 'Item not found'}
        
        item = self.items[item_id]
        
        # Check cargo space
        volume_per_unit = item['volume_per_unit']
        total_volume = volume_per_unit * quantity
        
        if total_volume > cargo_capacity:
            max_quantity = int(cargo_capacity / volume_per_unit)
            return {
                'error': 'Not enough cargo space',
                'max_quantity': max_quantity,
                'required_capacity': total_volume
            }
        
        # Calculate prices
        buy_price = self.calculate_price(item_id, buy_market, is_buying=True)
        sell_price = self.calculate_price(item_id, sell_market, is_buying=False)
        
        if buy_price == 0 or sell_price == 0:
            return {'error': 'Item not available at one or both markets'}
        
        # Calculate costs and profit
        total_cost = buy_price * quantity
        total_revenue = sell_price * quantity
        profit = total_revenue - total_cost
        roi = (profit / total_cost * 100) if total_cost > 0 else 0
        
        return {
            'item': item['item_name'],
            'quantity': quantity,
            'buy_market': self.markets[buy_market]['market_name'],
            'sell_market': self.markets[sell_market]['market_name'],
            'buy_price': buy_price,
            'sell_price': sell_price,
            'total_cost': total_cost,
            'total_revenue': total_revenue,
            'profit': profit,
            'roi_percent': round(roi, 1),
            'volume_used': total_volume,
            'cargo_efficiency': round(profit / total_volume, 2) if total_volume > 0 else 0
        }
    
    def find_best_routes(self, starting_capital: float = 1000.0, 
                        cargo_capacity: float = 10.0, 
                        top_n: int = 10) -> List[Dict]:
        """Find the most profitable trade routes"""
        routes = []
        
        for item_id in self.items:
            item = self.items[item_id]
            volume_per_unit = item['volume_per_unit']
            
            # Calculate max affordable quantity
            for buy_market_id in self.markets:
                buy_price = self.calculate_price(item_id, buy_market_id, is_buying=True)
                if buy_price == 0:
                    continue
                
                # Max quantity by cargo space
                max_by_cargo = int(cargo_capacity / volume_per_unit)
                # Max quantity by capital
                max_by_capital = int(starting_capital / buy_price)
                # Take the minimum
                max_quantity = min(max_by_cargo, max_by_capital)
                
                if max_quantity < 1:
                    continue
                
                for sell_market_id in self.markets:
                    if sell_market_id == buy_market_id:
                        continue
                    
                    result = self.calculate_route_profit(
                        item_id, buy_market_id, sell_market_id, 
                        max_quantity, cargo_capacity
                    )
                    
                    if 'error' not in result and result['profit'] > 0:
                        routes.append(result)
        
        # Sort by profit (descending)
        routes.sort(key=lambda x: x['profit'], reverse=True)
        
        return routes[:top_n]
    
    def optimize_cargo_mix(self, buy_market: str, sell_market: str, 
                          starting_capital: float = 1000.0, 
                          cargo_capacity: float = 10.0) -> Dict:
        """Find optimal mix of items for a specific route"""
        # Simple greedy algorithm: prioritize by profit per volume
        available_items = []
        
        for item_id in self.items:
            item = self.items[item_id]
            buy_price = self.calculate_price(item_id, buy_market, is_buying=True)
            sell_price = self.calculate_price(item_id, sell_market, is_buying=False)
            
            if buy_price == 0 or sell_price == 0:
                continue
            
            volume = item['volume_per_unit']
            profit_per_unit = sell_price - buy_price
            profit_per_volume = profit_per_unit / volume if volume > 0 else 0
            
            if profit_per_volume > 0:
                available_items.append({
                    'item_id': item_id,
                    'item_name': item['item_name'],
                    'buy_price': buy_price,
                    'sell_price': sell_price,
                    'volume': volume,
                    'profit_per_unit': profit_per_unit,
                    'profit_per_volume': profit_per_volume
                })
        
        # Sort by profit per volume (descending)
        available_items.sort(key=lambda x: x['profit_per_volume'], reverse=True)
        
        # Fill cargo greedily
        remaining_capital = starting_capital
        remaining_cargo = cargo_capacity
        cargo_manifest = []
        
        for item in available_items:
            # How many can we afford?
            max_by_capital = int(remaining_capital / item['buy_price'])
            # How many can we fit?
            max_by_cargo = int(remaining_cargo / item['volume'])
            # Take the minimum
            quantity = min(max_by_capital, max_by_cargo)
            
            if quantity > 0:
                cost = item['buy_price'] * quantity
                revenue = item['sell_price'] * quantity
                volume = item['volume'] * quantity
                
                cargo_manifest.append({
                    'item': item['item_name'],
                    'quantity': quantity,
                    'cost': cost,
                    'revenue': revenue,
                    'profit': revenue - cost,
                    'volume': volume
                })
                
                remaining_capital -= cost
                remaining_cargo -= volume
        
        total_cost = sum(item['cost'] for item in cargo_manifest)
        total_revenue = sum(item['revenue'] for item in cargo_manifest)
        total_profit = total_revenue - total_cost
        total_volume = sum(item['volume'] for item in cargo_manifest)
        
        return {
            'buy_market': self.markets[buy_market]['market_name'],
            'sell_market': self.markets[sell_market]['market_name'],
            'cargo_manifest': cargo_manifest,
            'total_cost': round(total_cost, 2),
            'total_revenue': round(total_revenue, 2),
            'total_profit': round(total_profit, 2),
            'roi_percent': round((total_profit / total_cost * 100) if total_cost > 0 else 0, 1),
            'cargo_used': round(total_volume, 2),
            'cargo_remaining': round(remaining_cargo, 2),
            'capital_remaining': round(remaining_capital, 2)
        }
    
    def print_route(self, route: Dict):
        """Pretty print a trade route"""
        if 'error' in route:
            print(f"❌ ERROR: {route['error']}")
            if 'max_quantity' in route:
                print(f"   Max quantity with current cargo: {route['max_quantity']}")
            return
        
        print(f"\n{'='*60}")
        print(f"📦 TRADE ROUTE: {route['item']} ({route['quantity']} units)")
        print(f"{'='*60}")
        print(f"🏪 Buy from:  {route['buy_market']}")
        print(f"   Price:     {route['buy_price']:.2f} cr/unit")
        print(f"   Total:     {route['total_cost']:.2f} cr")
        print(f"")
        print(f"🏪 Sell to:   {route['sell_market']}")
        print(f"   Price:     {route['sell_price']:.2f} cr/unit")
        print(f"   Total:     {route['total_revenue']:.2f} cr")
        print(f"")
        print(f"💰 Profit:    {route['profit']:.2f} cr ({route['roi_percent']}% ROI)")
        print(f"📦 Cargo:     {route['volume_used']:.1f} m³ used")
        print(f"⚡ Efficiency: {route['cargo_efficiency']:.2f} cr/m³")
        print(f"{'='*60}\n")
    
    def interactive_mode(self):
        """Run interactive calculator"""
        print("=" * 60)
        print("TRADE ROUTE PROFIT CALCULATOR")
        print("=" * 60)
        print()
        
        while True:
            print("\nOptions:")
            print("1. Calculate specific route")
            print("2. Find best routes")
            print("3. Optimize cargo mix")
            print("4. List items")
            print("5. List markets")
            print("6. Exit")
            
            choice = input("\nChoose option (1-6): ").strip()
            
            if choice == '1':
                self.interactive_calculate_route()
            elif choice == '2':
                self.interactive_find_best()
            elif choice == '3':
                self.interactive_optimize_cargo()
            elif choice == '4':
                self.list_items()
            elif choice == '5':
                self.list_markets()
            elif choice == '6':
                print("\nGoodbye!")
                break
            else:
                print("Invalid option")
    
    def interactive_calculate_route(self):
        """Interactive route calculation"""
        print("\n--- Calculate Trade Route ---")
        
        item_id = input("Item ID (e.g., water_purified): ").strip()
        buy_market = input("Buy market ID (e.g., agricultural_station_market): ").strip()
        sell_market = input("Sell market ID (e.g., industrial_station_market): ").strip()
        quantity = int(input("Quantity: ").strip())
        cargo = float(input("Cargo capacity (default 10): ").strip() or "10")
        
        result = self.calculate_route_profit(item_id, buy_market, sell_market, quantity, cargo)
        self.print_route(result)
    
    def interactive_find_best(self):
        """Interactive best routes finder"""
        print("\n--- Find Best Routes ---")
        
        capital = float(input("Starting capital (default 1000): ").strip() or "1000")
        cargo = float(input("Cargo capacity (default 10): ").strip() or "10")
        top_n = int(input("Number of routes to show (default 10): ").strip() or "10")
        
        routes = self.find_best_routes(capital, cargo, top_n)
        
        print(f"\n🏆 TOP {len(routes)} MOST PROFITABLE ROUTES")
        print(f"   Starting Capital: {capital:.0f} cr")
        print(f"   Cargo Capacity: {cargo:.0f} m³")
        print("=" * 60)
        
        for i, route in enumerate(routes, 1):
            print(f"\n#{i}. {route['item']} ({route['quantity']} units)")
            print(f"    {route['buy_market']} → {route['sell_market']}")
            print(f"    Profit: {route['profit']:.2f} cr ({route['roi_percent']}% ROI)")
            print(f"    Efficiency: {route['cargo_efficiency']:.2f} cr/m³")
    
    def interactive_optimize_cargo(self):
        """Interactive cargo optimization"""
        print("\n--- Optimize Cargo Mix ---")
        
        buy_market = input("Buy market ID: ").strip()
        sell_market = input("Sell market ID: ").strip()
        capital = float(input("Starting capital (default 1000): ").strip() or "1000")
        cargo = float(input("Cargo capacity (default 10): ").strip() or "10")
        
        result = self.optimize_cargo_mix(buy_market, sell_market, capital, cargo)
        
        print(f"\n🎯 OPTIMAL CARGO MIX")
        print(f"   {result['buy_market']} → {result['sell_market']}")
        print("=" * 60)
        
        for item in result['cargo_manifest']:
            print(f"\n📦 {item['item']}: {item['quantity']} units")
            print(f"   Cost: {item['cost']:.2f} cr")
            print(f"   Revenue: {item['revenue']:.2f} cr")
            print(f"   Profit: {item['profit']:.2f} cr")
            print(f"   Volume: {item['volume']:.2f} m³")
        
        print(f"\n{'='*60}")
        print(f"💰 Total Profit: {result['total_profit']:.2f} cr ({result['roi_percent']}% ROI)")
        print(f"📦 Cargo Used: {result['cargo_used']:.2f} / {cargo:.2f} m³")
        print(f"💵 Capital Used: {result['total_cost']:.2f} / {capital:.2f} cr")
        print(f"{'='*60}")
    
    def list_items(self):
        """List all available items"""
        print("\n📦 AVAILABLE TRADE ITEMS")
        print("=" * 60)
        
        for item_id, item in sorted(self.items.items()):
            print(f"{item_id:30} | {item['item_name']:25} | {item['base_price']:>6.0f} cr")
    
    def list_markets(self):
        """List all available markets"""
        print("\n🏪 AVAILABLE MARKETS")
        print("=" * 60)
        
        for market_id, market in sorted(self.markets.items()):
            print(f"{market_id:35} | {market['market_name']}")


def main():
    """Main entry point"""
    import argparse
    
    parser = argparse.ArgumentParser(description="Trade Route Profit Calculator")
    parser.add_argument('--interactive', '-i', action='store_true', help='Run in interactive mode')
    parser.add_argument('--best', '-b', action='store_true', help='Show best routes')
    parser.add_argument('--capital', type=float, default=1000.0, help='Starting capital')
    parser.add_argument('--cargo', type=float, default=10.0, help='Cargo capacity')
    
    args = parser.parse_args()
    
    calc = TradeProfitCalculator()
    
    if args.interactive:
        calc.interactive_mode()
    elif args.best:
        routes = calc.find_best_routes(args.capital, args.cargo)
        print(f"\n🏆 TOP 10 MOST PROFITABLE ROUTES")
        print(f"   Starting Capital: {args.capital:.0f} cr")
        print(f"   Cargo Capacity: {args.cargo:.0f} m³")
        
        for i, route in enumerate(routes, 1):
            print(f"\n#{i}. {route['item']} ({route['quantity']} units)")
            print(f"    {route['buy_market']} → {route['sell_market']}")
            print(f"    Profit: {route['profit']:.2f} cr ({route['roi_percent']}% ROI)")
    else:
        print("Use --interactive or --best flags. Run with --help for more options.")
    
    return 0


if __name__ == "__main__":
    sys.exit(main())
