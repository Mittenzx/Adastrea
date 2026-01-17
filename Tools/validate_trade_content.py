#!/usr/bin/env python3
"""
Trade Content Validator

Validates MVP trading content for economic balance and playability.
Checks for common issues like unrealistic prices, impossible trades, etc.

Usage:
    python validate_trade_content.py
"""

import yaml
import sys
from pathlib import Path


class TradeContentValidator:
    """Validates trading content for MVP"""
    
    def __init__(self, project_root=None):
        if project_root is None:
            project_root = Path(__file__).parent.parent
        self.project_root = Path(project_root)
        self.templates_dir = self.project_root / "Assets" / "TradingTemplates"
        self.errors = []
        self.warnings = []
        
    def validate_all(self):
        """Run all validations"""
        print("=" * 60)
        print("TRADE CONTENT VALIDATOR")
        print("=" * 60)
        print()
        
        success = True
        
        # Validate trade items
        print("VALIDATING TRADE ITEMS")
        print("-" * 60)
        if not self.validate_trade_items():
            success = False
        print()
        
        # Validate markets
        print("VALIDATING MARKETS")
        print("-" * 60)
        if not self.validate_markets():
            success = False
        print()
        
        # Validate economic balance
        print("VALIDATING ECONOMIC BALANCE")
        print("-" * 60)
        if not self.validate_economy():
            success = False
        print()
        
        # Print summary
        print("=" * 60)
        print("VALIDATION SUMMARY")
        print("=" * 60)
        
        if self.errors:
            print(f"\n❌ ERRORS ({len(self.errors)}):")
            for error in self.errors:
                print(f"  • {error}")
        
        if self.warnings:
            print(f"\n⚠️  WARNINGS ({len(self.warnings)}):")
            for warning in self.warnings:
                print(f"  • {warning}")
        
        if not self.errors and not self.warnings:
            print("\n✅ All validations passed!")
            print("Trading content is ready for use.")
        elif not self.errors:
            print(f"\n⚠️  Passed with {len(self.warnings)} warnings")
            print("Review warnings but content is usable.")
        else:
            print(f"\n❌ Validation failed with {len(self.errors)} errors")
            print("Fix errors before using this content.")
        
        print("=" * 60)
        
        return success and len(self.errors) == 0
        
    def validate_trade_items(self):
        """Validate trade items"""
        items_file = self.templates_dir / "MVPTradeItems.yaml"
        
        if not items_file.exists():
            self.errors.append(f"Trade items file not found: {items_file}")
            return False
        
        with open(items_file, 'r') as f:
            items = yaml.safe_load(f)
        
        if not items:
            self.errors.append("No trade items found in template")
            return False
        
        print(f"✓ Loaded {len(items)} trade items")
        
        # Validate each item
        for i, item in enumerate(items):
            item_id = item.get('item_id', f'item_{i}')
            
            # Required fields
            required = ['item_id', 'item_name', 'description', 'category', 'base_price',
                       'volume_per_unit', 'mass_per_unit', 'legality']
            
            for field in required:
                if field not in item:
                    self.errors.append(f"{item_id}: Missing required field '{field}'")
            
            # Price validation
            base_price = item.get('base_price', 0)
            if base_price <= 0:
                self.errors.append(f"{item_id}: base_price must be > 0 (got {base_price})")
            elif base_price > 100000:
                self.warnings.append(f"{item_id}: Very high base_price ({base_price}cr). Ensure this is intentional.")
            
            # Volume validation
            volume = item.get('volume_per_unit', 0)
            if volume <= 0:
                self.errors.append(f"{item_id}: volume_per_unit must be > 0")
            elif volume > 50:
                self.warnings.append(f"{item_id}: Very large volume ({volume}m³). May not fit in starter cargo.")
            
            # Volatility validation
            volatility = item.get('volatility_multiplier', 1.0)
            if volatility < 0:
                self.errors.append(f"{item_id}: volatility_multiplier cannot be negative")
            elif volatility > 3.0:
                self.warnings.append(f"{item_id}: Very high volatility ({volatility}). Prices may be unpredictable.")
            
            # Price deviation validation
            min_dev = item.get('min_price_deviation', 0.5)
            max_dev = item.get('max_price_deviation', 2.0)
            
            if min_dev >= max_dev:
                self.errors.append(f"{item_id}: min_price_deviation must be < max_price_deviation")
            
            if min_dev < 0.1:
                self.warnings.append(f"{item_id}: Very low min_price_deviation ({min_dev}). May cause zero prices.")
            
            if max_dev > 10.0:
                self.warnings.append(f"{item_id}: Very high max_price_deviation ({max_dev}). Prices may be extreme.")
            
            # Stock validation
            typical_stock = item.get('typical_market_stock', 0)
            if typical_stock <= 0:
                self.warnings.append(f"{item_id}: typical_market_stock is 0. Item may never be in stock.")
        
        if not self.errors:
            print("✓ All trade items valid")
        
        return len(self.errors) == 0
        
    def validate_markets(self):
        """Validate market configurations"""
        markets_file = self.templates_dir / "MVPMarkets.yaml"
        
        if not markets_file.exists():
            self.errors.append(f"Markets file not found: {markets_file}")
            return False
        
        with open(markets_file, 'r') as f:
            markets = yaml.safe_load(f)
        
        if not markets:
            self.errors.append("No markets found in template")
            return False
        
        print(f"✓ Loaded {len(markets)} markets")
        
        # Validate each market
        for i, market in enumerate(markets):
            market_id = market.get('market_id', f'market_{i}')
            
            # Required fields
            required = ['market_id', 'market_name', 'market_type', 'market_size',
                       'sell_price_markup', 'buy_price_markdown', 'inventory']
            
            for field in required:
                if field not in market:
                    self.errors.append(f"{market_id}: Missing required field '{field}'")
            
            # Markup validation
            sell_markup = market.get('sell_price_markup', 1.0)
            if sell_markup < 1.0:
                self.errors.append(f"{market_id}: sell_price_markup must be >= 1.0 (player pays at least base price)")
            elif sell_markup > 3.0:
                self.warnings.append(f"{market_id}: Very high sell_markup ({sell_markup}). Player may struggle to afford items.")
            
            # Markdown validation
            buy_markdown = market.get('buy_price_markdown', 1.0)
            if buy_markdown > 1.0:
                self.errors.append(f"{market_id}: buy_price_markdown must be <= 1.0 (player sells below base price)")
            elif buy_markdown < 0.3:
                self.warnings.append(f"{market_id}: Very low buy_markdown ({buy_markdown}). Poor profit margins.")
            
            # Inventory validation
            inventory = market.get('inventory', [])
            if not inventory:
                self.warnings.append(f"{market_id}: Empty inventory. Market has nothing to trade.")
            else:
                print(f"  ✓ {market_id}: {len(inventory)} items in inventory")
                
                for inv_item in inventory:
                    item_id = inv_item.get('item_id', 'unknown')
                    
                    # Stock validation
                    current = inv_item.get('current_stock', 0)
                    max_stock = inv_item.get('max_stock', 0)
                    
                    if current > max_stock:
                        self.errors.append(f"{market_id}/{item_id}: current_stock > max_stock")
                    
                    # Supply/demand validation
                    supply = inv_item.get('supply_level', 1.0)
                    demand = inv_item.get('demand_level', 1.0)
                    
                    if supply < 0:
                        self.errors.append(f"{market_id}/{item_id}: supply_level cannot be negative")
                    if demand < 0:
                        self.errors.append(f"{market_id}/{item_id}: demand_level cannot be negative")
        
        if not self.errors:
            print("✓ All markets valid")
        
        return len(self.errors) == 0
        
    def validate_economy(self):
        """Validate overall economic balance"""
        # Load both files
        items_file = self.templates_dir / "MVPTradeItems.yaml"
        markets_file = self.templates_dir / "MVPMarkets.yaml"
        
        with open(items_file, 'r') as f:
            items = yaml.safe_load(f)
        
        with open(markets_file, 'r') as f:
            markets = yaml.safe_load(f)
        
        # Create item lookup
        items_by_id = {item['item_id']: item for item in items}
        
        # Check for profitable trade routes
        profitable_routes = 0
        
        for i, market_a in enumerate(markets):
            for market_b in markets[i+1:]:
                # Find items in both markets
                items_a = {inv['item_id']: inv for inv in market_a.get('inventory', [])}
                items_b = {inv['item_id']: inv for inv in market_b.get('inventory', [])}
                
                common_items = set(items_a.keys()) & set(items_b.keys())
                
                for item_id in common_items:
                    if item_id not in items_by_id:
                        continue
                    
                    item = items_by_id[item_id]
                    inv_a = items_a[item_id]
                    inv_b = items_b[item_id]
                    
                    # Calculate prices
                    base = item['base_price']
                    
                    # Price at market A (buying)
                    price_a = base * inv_a['supply_level'] * market_a['sell_price_markup']
                    
                    # Price at market B (selling)
                    price_b = base * inv_b['demand_level'] * market_b['buy_price_markdown']
                    
                    # Check profitability
                    if price_b > price_a * 1.2:  # 20%+ profit
                        profitable_routes += 1
        
        print(f"✓ Found {profitable_routes} potentially profitable trade routes")
        
        if profitable_routes < 5:
            self.warnings.append(f"Only {profitable_routes} profitable routes found. Consider adding more economic diversity.")
        
        # Check starter ship affordability
        cheapest_items = sorted(items, key=lambda x: x['base_price'])[:3]
        starter_capital = 1000  # Assumed starting credits
        
        affordable = [item for item in cheapest_items if item['base_price'] * 10 < starter_capital]
        
        if not affordable:
            self.warnings.append("No items affordable for starter ship (1000cr capital, 10-unit purchase). Consider lowering prices.")
        else:
            print(f"✓ {len(affordable)} items affordable for starter trades")
        
        return True


def main():
    """Main entry point"""
    validator = TradeContentValidator()
    success = validator.validate_all()
    
    return 0 if success else 1


if __name__ == "__main__":
    sys.exit(main())
