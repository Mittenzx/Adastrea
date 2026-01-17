#!/usr/bin/env python3
"""
MVP Trading Content Generator

Generates Unreal Engine Data Assets for the Trade Simulator MVP from YAML templates.
Automates creation of TradeItemDataAssets and MarketDataAssets.

Usage:
    python generate_mvp_trading_content.py [--items] [--markets] [--all]
    
Examples:
    python generate_mvp_trading_content.py --all
    python generate_mvp_trading_content.py --items
    python generate_mvp_trading_content.py --markets
"""

import yaml
import sys
from pathlib import Path


class MVPTradingContentGenerator:
    """Generates MVP trading Data Assets from YAML templates"""
    
    def __init__(self, project_root=None):
        if project_root is None:
            # Script is in Tools/, project root is parent
            project_root = Path(__file__).parent.parent
        self.project_root = Path(project_root)
        self.templates_dir = self.project_root / "Assets" / "TradingTemplates"
        self.output_dir = self.project_root / "Content" / "DataAssets" / "Trading" / "MVP"
        
    def generate_trade_items(self):
        """Generate TradeItemDataAsset files from YAML template"""
        items_file = self.templates_dir / "MVPTradeItems.yaml"
        
        if not items_file.exists():
            print(f"❌ ERROR: Template file not found: {items_file}")
            return False
            
        print(f"📦 Loading trade items from {items_file}")
        
        with open(items_file, 'r') as f:
            items = yaml.safe_load(f)
            
        if not items:
            print("❌ ERROR: No items found in template")
            return False
            
        print(f"✅ Loaded {len(items)} trade items")
        
        # Create output directory
        items_output = self.output_dir / "Items"
        items_output.mkdir(parents=True, exist_ok=True)
        
        # Generate Unreal Data Asset files
        generated_count = 0
        for item in items:
            asset_name = f"DA_Item_{self._to_pascal_case(item['item_id'])}"
            output_file = items_output / f"{asset_name}.uasset.txt"
            
            # Generate human-readable representation
            # (In production, this would generate actual .uasset files via Python API)
            content = self._generate_trade_item_content(item, asset_name)
            
            with open(output_file, 'w') as f:
                f.write(content)
                
            generated_count += 1
            print(f"  ✓ Generated {asset_name}")
            
        print(f"\n✅ Successfully generated {generated_count} trade item Data Assets")
        print(f"📁 Output directory: {items_output}")
        print(f"\n📝 Next steps:")
        print(f"   1. Open Unreal Editor")
        print(f"   2. Import these configurations into Data Assets")
        print(f"   3. Or use the Unreal Python API to create actual .uasset files")
        
        return True
        
    def generate_markets(self):
        """Generate MarketDataAsset files from YAML template"""
        markets_file = self.templates_dir / "MVPMarkets.yaml"
        
        if not markets_file.exists():
            print(f"❌ ERROR: Template file not found: {markets_file}")
            return False
            
        print(f"🏪 Loading markets from {markets_file}")
        
        with open(markets_file, 'r') as f:
            markets = yaml.safe_load(f)
            
        if not markets:
            print("❌ ERROR: No markets found in template")
            return False
            
        print(f"✅ Loaded {len(markets)} market configurations")
        
        # Create output directory
        markets_output = self.output_dir / "Markets"
        markets_output.mkdir(parents=True, exist_ok=True)
        
        # Generate Unreal Data Asset files
        generated_count = 0
        for market in markets:
            asset_name = f"DA_Market_{self._to_pascal_case(market['market_id'])}"
            output_file = markets_output / f"{asset_name}.uasset.txt"
            
            content = self._generate_market_content(market, asset_name)
            
            with open(output_file, 'w') as f:
                f.write(content)
                
            generated_count += 1
            print(f"  ✓ Generated {asset_name}")
            
        print(f"\n✅ Successfully generated {generated_count} market Data Assets")
        print(f"📁 Output directory: {markets_output}")
        print(f"\n📝 Next steps:")
        print(f"   1. Open Unreal Editor")
        print(f"   2. Create MarketDataAsset instances")
        print(f"   3. Copy values from generated .txt files")
        print(f"   4. Assign to BP_SpaceStationModule_Marketplace instances")
        
        return True
        
    def _generate_trade_item_content(self, item, asset_name):
        """Generate trade item Data Asset content"""
        content = f"""
# {asset_name}
# Trade Item Data Asset Configuration
# Source: MVPTradeItems.yaml

Class: /Script/Adastrea.TradeItemDataAsset
Asset Name: {asset_name}

# ============================================
# BASIC INFO
# ============================================
ItemName: {item['item_name']}
Description: |
  {item['description']}
ItemID: {item['item_id']}
Category: {item['category']}

# ============================================
# PRICING
# ============================================
BasePrice: {item['base_price']}

PriceVolatility:
  VolatilityMultiplier: {item['volatility_multiplier']}
  MinPriceDeviation: {item['min_price_deviation']}
  MaxPriceDeviation: {item['max_price_deviation']}

bAffectedBySupplyDemand: {str(item['affected_by_supply_demand']).lower()}
bAffectedByMarketEvents: {str(item['affected_by_market_events']).lower()}

# ============================================
# LEGALITY & RESTRICTIONS
# ============================================
LegalityStatus: {item['legality']}
ContrabandFineMultiplier: 1.0

# ============================================
# TRADE PROPERTIES
# ============================================
VolumePerUnit: {item['volume_per_unit']}
MassPerUnit: {item['mass_per_unit']}
StandardLotSize: {item['standard_lot_size']}
TypicalMarketStock: {item['typical_market_stock']}
ReplenishmentRate: {item['replenishment_rate']}

# ============================================
# AI TRADER BEHAVIOR
# ============================================
AITradePriority: {item['ai_trade_priority']}
bAIHoardable: {str(item['ai_hoardable']).lower()}
bAIArbitrageEnabled: {str(item['ai_arbitrage_enabled']).lower()}
BehaviorTags: []

# ============================================
# TRADING ECONOMICS
# ============================================
Price per Lot (@ base): {item['base_price'] * item['standard_lot_size']} credits
Volume per Lot: {item['volume_per_unit'] * item['standard_lot_size']} m³
Mass per Lot: {item['mass_per_unit'] * item['standard_lot_size']} kg

Profit Potential:
  Min Price: {item['base_price'] * item['min_price_deviation']:.1f} credits/unit
  Max Price: {item['base_price'] * item['max_price_deviation']:.1f} credits/unit
  Max Spread: {item['base_price'] * (item['max_price_deviation'] - item['min_price_deviation']):.1f} credits/unit
"""
        return content
        
    def _generate_market_content(self, market, asset_name):
        """Generate market Data Asset content"""
        content = f"""
# {asset_name}
# Market Data Asset Configuration
# Source: MVPMarkets.yaml

Class: /Script/Adastrea.MarketDataAsset
Asset Name: {asset_name}

# ============================================
# BASIC INFO
# ============================================
MarketName: {market['market_name']}
Description: |
  {market['description']}
MarketID: {market['market_id']}
MarketType: {market['market_type']}
MarketSize: {market['market_size']}

# ============================================
# MARKET CONFIGURATION
# ============================================
TransactionTaxRate: {market['transaction_tax_rate']}
SellPriceMarkup: {market['sell_price_markup']}
BuyPriceMarkdown: {market['buy_price_markdown']}
bAllowPlayerBuying: {str(market['allow_player_buying']).lower()}
bAllowPlayerSelling: {str(market['allow_player_selling']).lower()}
bAllowAITraders: {str(market['allow_ai_traders']).lower()}
StockRefreshRate: {market['stock_refresh_rate']}
RandomEventChance: {market['random_event_chance']}

# ============================================
# AI TRADER SETTINGS
# ============================================
AITraderCount: {market['ai_trader_count']}
AITradeFrequency: {market['ai_trade_frequency']}
bAllowAIPriceManipulation: {str(market['allow_ai_price_manipulation']).lower()}

# ============================================
# INVENTORY ({len(market['inventory'])} items)
# ============================================
"""
        
        for inv_item in market['inventory']:
            content += f"""
# Item: {inv_item['item_id']}
Inventory Entry:
  TradeItem: DA_Item_{self._to_pascal_case(inv_item['item_id'])}
  CurrentStock: {inv_item['current_stock']}
  MaxStock: {inv_item['max_stock']}
  SupplyLevel: {inv_item['supply_level']}  # {self._supply_description(inv_item['supply_level'])}
  DemandLevel: {inv_item['demand_level']}  # {self._demand_description(inv_item['demand_level'])}
  bInStock: true
"""
        
        content += f"""

# ============================================
# TRADING STRATEGY GUIDE
# ============================================

Buy from this market (cheap):
"""
        # Find cheap items (high supply, low demand)
        cheap_items = [i for i in market['inventory'] if i['supply_level'] > 1.5 or i['demand_level'] < 0.7]
        for item in cheap_items[:5]:
            content += f"  • {item['item_id']} (supply: {item['supply_level']}, demand: {item['demand_level']})\n"
            
        content += f"""
Sell to this market (expensive):
"""
        # Find expensive items (low supply, high demand)
        expensive_items = [i for i in market['inventory'] if i['supply_level'] < 0.5 or i['demand_level'] > 1.5]
        for item in expensive_items[:5]:
            content += f"  • {item['item_id']} (supply: {item['supply_level']}, demand: {item['demand_level']})\n"
            
        return content
        
    def _to_pascal_case(self, snake_str):
        """Convert snake_case to PascalCase"""
        return ''.join(word.capitalize() for word in snake_str.split('_'))
        
    def _supply_description(self, level):
        """Human-readable supply level"""
        if level > 2.0:
            return "Abundant"
        elif level > 1.5:
            return "Plentiful"
        elif level > 0.8:
            return "Normal"
        elif level > 0.4:
            return "Low"
        else:
            return "Scarce"
            
    def _demand_description(self, level):
        """Human-readable demand level"""
        if level > 2.0:
            return "Very High"
        elif level > 1.5:
            return "High"
        elif level > 0.8:
            return "Normal"
        elif level > 0.4:
            return "Low"
        else:
            return "Very Low"


def main():
    """Main entry point"""
    import argparse
    
    parser = argparse.ArgumentParser(
        description="Generate MVP Trading Content for Adastrea",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python generate_mvp_trading_content.py --all
  python generate_mvp_trading_content.py --items
  python generate_mvp_trading_content.py --markets
        """
    )
    
    parser.add_argument('--items', action='store_true', help='Generate trade item Data Assets')
    parser.add_argument('--markets', action='store_true', help='Generate market Data Assets')
    parser.add_argument('--all', action='store_true', help='Generate all content')
    
    args = parser.parse_args()
    
    # Default to --all if no options specified
    if not (args.items or args.markets or args.all):
        args.all = True
    
    generator = MVPTradingContentGenerator()
    
    success = True
    
    print("=" * 60)
    print("MVP TRADING CONTENT GENERATOR")
    print("=" * 60)
    print()
    
    if args.all or args.items:
        print("GENERATING TRADE ITEMS")
        print("-" * 60)
        if not generator.generate_trade_items():
            success = False
        print()
        
    if args.all or args.markets:
        print("GENERATING MARKETS")
        print("-" * 60)
        if not generator.generate_markets():
            success = False
        print()
        
    print("=" * 60)
    if success:
        print("✅ SUCCESS: All content generated successfully!")
        print()
        print("📋 NEXT STEPS:")
        print("  1. Review generated files in Content/DataAssets/Trading/MVP/")
        print("  2. Open Unreal Editor")
        print("  3. Create Data Asset instances using generated configurations")
        print("  4. Test in-game trading functionality")
        print()
        print("⚡ QUICK START:")
        print("  • Copy values from .txt files into Unreal Data Assets")
        print("  • Assign trade items to market inventories")
        print("  • Configure BP_SpaceStationModule_Marketplace with markets")
        print("  • Test trading between stations!")
    else:
        print("❌ FAILED: Some content generation failed")
        return 1
        
    print("=" * 60)
    return 0


if __name__ == "__main__":
    sys.exit(main())
