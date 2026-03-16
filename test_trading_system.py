#!/usr/bin/env python3
"""
Trading System Test Script
Tests the Adastrea trading system data assets and calculations.
"""

import os
import sys
import yaml
import json
from pathlib import Path

def load_trade_item_data_assets():
    """Load and validate trade item data assets."""
    print("Loading Trade Item Data Assets...")
    
    trade_items_path = Path("Content/DataAssets/Trading/MVP/Items")
    if not trade_items_path.exists():
        print(f"[ERROR] Trade items directory not found: {trade_items_path}")
        return []
    
    trade_items = []
    for item_file in trade_items_path.glob("DA_Item_*.uasset.txt"):
        try:
            # Try different encodings
            for encoding in ['utf-8', 'cp1252', 'latin-1']:
                try:
                    with open(item_file, 'r', encoding=encoding) as f:
                        content = f.read()
                    break
                except UnicodeDecodeError:
                    continue
            else:
                # If all encodings fail, read as binary and decode with errors ignored
                with open(item_file, 'rb') as f:
                    content = f.read().decode('utf-8', errors='ignore')
            
            # Parse the structured text file
            item_data = {}
            current_section = None
            
            for line in content.split('\n'):
                line = line.strip()
                if not line or line.startswith('#'):
                    continue
                
                if line.startswith('Class:'):
                    item_data['Class'] = line.split(': ')[1]
                elif line.startswith('Asset Name:'):
                    item_data['AssetName'] = line.split(': ')[1]
                elif line.startswith('ItemName:'):
                    item_data['ItemName'] = line.split(': ')[1]
                elif line.startswith('ItemID:'):
                    item_data['ItemID'] = line.split(': ')[1]
                elif line.startswith('Category:'):
                    item_data['Category'] = line.split(': ')[1]
                elif line.startswith('BasePrice:'):
                    item_data['BasePrice'] = float(line.split(': ')[1])
                elif line.startswith('VolumePerUnit:'):
                    item_data['VolumePerUnit'] = float(line.split(': ')[1])
                elif line.startswith('MassPerUnit:'):
                    item_data['MassPerUnit'] = float(line.split(': ')[1])
                elif line.startswith('StandardLotSize:'):
                    item_data['StandardLotSize'] = int(line.split(': ')[1])
            
            if item_data:
                trade_items.append(item_data)
                print(f"  [OK] Loaded: {item_data['ItemName']} ({item_data['ItemID']})")
        
        except Exception as e:
            print(f"  [ERROR] Error loading {item_file.name}: {e}")
    
    print(f"Total trade items loaded: {len(trade_items)}")
    return trade_items

def load_market_data_assets():
    """Load and validate market data assets."""
    print("\nLoading Market Data Assets...")
    
    markets_path = Path("Content/DataAssets/Trading/MVP/Markets")
    if not markets_path.exists():
        print(f"[ERROR] Markets directory not found: {markets_path}")
        return []
    
    markets = []
    for market_file in markets_path.glob("DA_Market_*.uasset.txt"):
        try:
            # Try different encodings
            for encoding in ['utf-8', 'cp1252', 'latin-1']:
                try:
                    with open(market_file, 'r', encoding=encoding) as f:
                        content = f.read()
                    break
                except UnicodeDecodeError:
                    continue
            else:
                # If all encodings fail, read as binary and decode with errors ignored
                with open(market_file, 'rb') as f:
                    content = f.read().decode('utf-8', errors='ignore')
            
            # Parse the structured text file
            market_data = {}
            
            for line in content.split('\n'):
                line = line.strip()
                if not line or line.startswith('#'):
                    continue
                
                if line.startswith('Class:'):
                    market_data['Class'] = line.split(': ')[1]
                elif line.startswith('Asset Name:'):
                    market_data['AssetName'] = line.split(': ')[1]
                elif line.startswith('MarketName:'):
                    market_data['MarketName'] = line.split(': ')[1]
                elif line.startswith('MarketID:'):
                    market_data['MarketID'] = line.split(': ')[1]
                elif line.startswith('StationType:'):
                    market_data['StationType'] = line.split(': ')[1]
                elif line.startswith('MarketType:'):
                    # Some markets use MarketType instead of StationType
                    market_data['StationType'] = line.split(': ')[1]
                elif line.startswith('FactionAffiliation:'):
                    market_data['FactionAffiliation'] = line.split(': ')[1]
                elif 'FactionAffiliation' not in market_data and 'Neutral' in line:
                    # Default to Neutral if not specified
                    market_data['FactionAffiliation'] = 'Neutral'
                elif line.startswith('TransactionTaxRate:'):
                    market_data['TransactionTaxRate'] = float(line.split(': ')[1])
                elif line.startswith('StockRefreshRate:'):
                    market_data['StockRefreshRate'] = float(line.split(': ')[1])
                elif line.startswith('RandomEventChance:'):
                    market_data['RandomEventChance'] = float(line.split(': ')[1])
            
            if market_data:
                markets.append(market_data)
                print(f"  [OK] Loaded: {market_data['MarketName']} ({market_data['StationType']})")
        
        except Exception as e:
            print(f"  [ERROR] Error loading {market_file.name}: {e}")
    
    print(f"[SUMMARY] Total markets loaded: {len(markets)}")
    return markets

def validate_trade_items(trade_items):
    """Validate trade item data for consistency."""
    print("\n[VALIDATE] Validating Trade Items...")
    
    validation_errors = []
    
    for item in trade_items:
        # Check required fields
        required_fields = ['ItemName', 'ItemID', 'Category', 'BasePrice', 'VolumePerUnit', 'MassPerUnit']
        for field in required_fields:
            if field not in item:
                validation_errors.append(f"{item.get('ItemName', 'Unknown')}: Missing {field}")
        
        # Check data types and ranges
        if 'BasePrice' in item and item['BasePrice'] <= 0:
            validation_errors.append(f"{item['ItemName']}: BasePrice must be positive")
        
        if 'VolumePerUnit' in item and item['VolumePerUnit'] <= 0:
            validation_errors.append(f"{item['ItemName']}: VolumePerUnit must be positive")
        
        if 'MassPerUnit' in item and item['MassPerUnit'] <= 0:
            validation_errors.append(f"{item['ItemName']}: MassPerUnit must be positive")
        
        if 'StandardLotSize' in item and item['StandardLotSize'] <= 0:
            validation_errors.append(f"{item['ItemName']}: StandardLotSize must be positive")
    
    if validation_errors:
        print(f"[ERROR] Validation errors found: {len(validation_errors)}")
        for error in validation_errors:
            print(f"  - {error}")
        return False
    else:
        print("[OK] All trade items validated successfully!")
        return True

def validate_markets(markets):
    """Validate market data for consistency."""
    print("\n[VALIDATE] Validating Markets...")
    
    validation_errors = []
    
    for market in markets:
        # Check required fields
        required_fields = ['MarketName', 'MarketID', 'StationType']
        for field in required_fields:
            if field not in market:
                validation_errors.append(f"{market.get('MarketName', 'Unknown')}: Missing {field}")
        
        # FactionAffiliation is optional, default to Neutral
        if 'FactionAffiliation' not in market:
            market['FactionAffiliation'] = 'Neutral'
        
        # Check data types and ranges
        if 'TransactionTaxRate' in market and (market['TransactionTaxRate'] < 0 or market['TransactionTaxRate'] > 1):
            validation_errors.append(f"{market['MarketName']}: TransactionTaxRate must be between 0 and 1")
        
        if 'StockRefreshRate' in market and market['StockRefreshRate'] <= 0:
            validation_errors.append(f"{market['MarketName']}: StockRefreshRate must be positive")
        
        if 'RandomEventChance' in market and (market['RandomEventChance'] < 0 or market['RandomEventChance'] > 1):
            validation_errors.append(f"{market['MarketName']}: RandomEventChance must be between 0 and 1")
    
    if validation_errors:
        print(f"[ERROR] Validation errors found: {len(validation_errors)}")
        for error in validation_errors:
            print(f"  - {error}")
        return False
    else:
        print("[OK] All markets validated successfully!")
        return True

def calculate_trading_metrics(trade_items, markets):
    """Calculate trading metrics and profitability analysis."""
    print("\n[METRICS] Trading Metrics Analysis...")
    
    # Calculate price ranges for each item
    print("\n[PRICE] Price Ranges (per unit):")
    for item in trade_items:
        base_price = item['BasePrice']
        min_price = base_price * 0.4  # From MinPriceDeviation
        max_price = base_price * 3.0  # From MaxPriceDeviation
        spread = max_price - min_price
        
        print(f"  {item['ItemName']}:")
        print(f"    Base: {base_price:,.0f} credits")
        print(f"    Range: {min_price:,.0f} - {max_price:,.0f} credits")
        print(f"    Spread: {spread:,.0f} credits ({spread/base_price*100:.1f}% of base)")
    
    # Calculate cargo efficiency (value per volume)
    print("\n[CARGO] Cargo Efficiency (value per m³):")
    efficiency_data = []
    for item in trade_items:
        value_per_volume = item['BasePrice'] / item['VolumePerUnit']
        value_per_mass = item['BasePrice'] / item['MassPerUnit']
        efficiency_data.append({
            'Item': item['ItemName'],
            'ValuePerVolume': value_per_volume,
            'ValuePerMass': value_per_mass,
            'Volume': item['VolumePerUnit'],
            'Mass': item['MassPerUnit']
        })
    
    # Sort by value per volume (most efficient first)
    efficiency_data.sort(key=lambda x: x['ValuePerVolume'], reverse=True)
    
    for i, data in enumerate(efficiency_data[:5]):  # Top 5
        print(f"  {i+1}. {data['Item']}:")
        print(f"     {data['ValuePerVolume']:,.0f} credits/m³")
        print(f"     {data['ValuePerMass']:,.0f} credits/kg")
    
    # Market specialization analysis
    print("\n[MARKET] Market Specialization Analysis:")
    for market in markets:
        station_type = market.get('StationType', 'Unknown')
        tax_rate = market.get('TransactionTaxRate', 0.05)
        refresh_rate = market.get('StockRefreshRate', 24.0)
        event_chance = market.get('RandomEventChance', 0.1)
        
        print(f"  {market['MarketName']} ({station_type}):")
        print(f"    Transaction Tax: {tax_rate*100:.1f}%")
        print(f"    Stock Refresh: Every {refresh_rate:.1f} hours")
        print(f"    Event Chance: {event_chance*100:.1f}%")
        
        # Suggest profitable items based on station type
        if 'Agricultural' in market['MarketName'] or 'Farming' in market['MarketName']:
            print("    Suggested Trade: Food -> Industrial stations")
        elif 'Industrial' in market['MarketName']:
            print("    Suggested Trade: Materials -> Research stations")
        elif 'Research' in market['MarketName']:
            print("    Suggested Trade: Technology -> Agricultural stations")
        elif 'Luxury' in market['MarketName'] or 'Bazaar' in market['MarketName']:
            print("    Suggested Trade: Luxury items -> Wealthy stations")
        elif 'Trade' in market['MarketName'] or 'Exchange' in market['MarketName']:
            print("    Suggested Trade: General goods -> All stations")

def generate_trading_report(trade_items, markets):
    """Generate a comprehensive trading report."""
    print("\n[REPORT] Generating Trading System Report...")
    
    report = {
        'summary': {
            'total_trade_items': len(trade_items),
            'total_markets': len(markets),
            'categories': list(set(item['Category'] for item in trade_items)),
            'station_types': list(set(market['StationType'] for market in markets))
        },
        'trade_items': trade_items,
        'markets': markets,
        'analysis': {
            'most_valuable_item': max(trade_items, key=lambda x: x['BasePrice'])['ItemName'],
            'least_valuable_item': min(trade_items, key=lambda x: x['BasePrice'])['ItemName'],
            'highest_volume_item': max(trade_items, key=lambda x: x['VolumePerUnit'])['ItemName'],
            'lowest_volume_item': min(trade_items, key=lambda x: x['VolumePerUnit'])['ItemName']
        }
    }
    
    # Save report to file
    report_path = Path("trading_system_report.json")
    with open(report_path, 'w', encoding='utf-8') as f:
        json.dump(report, f, indent=2, ensure_ascii=False)
    
    print(f"[OK] Report saved to: {report_path}")
    
    # Print summary
    print("\n[SUMMARY] Trading System Summary:")
    print(f"  Total Trade Items: {report['summary']['total_trade_items']}")
    print(f"  Total Markets: {report['summary']['total_markets']}")
    print(f"  Categories: {', '.join(report['summary']['categories'])}")
    print(f"  Station Types: {', '.join(report['summary']['station_types'])}")
    print(f"  Most Valuable Item: {report['analysis']['most_valuable_item']}")
    print(f"  Least Valuable Item: {report['analysis']['least_valuable_item']}")

def main():
    """Main test function."""
    print("=" * 60)
    print("ADASTREA TRADING SYSTEM TEST")
    print("=" * 60)
    
    # Change to project directory
    project_dir = Path(__file__).parent
    os.chdir(project_dir)
    
    # Load data assets
    trade_items = load_trade_item_data_assets()
    markets = load_market_data_assets()
    
    if not trade_items or not markets:
        print("[ERROR] Failed to load data assets. Exiting.")
        return 1
    
    # Validate data
    items_valid = validate_trade_items(trade_items)
    markets_valid = validate_markets(markets)
    
    if not items_valid or not markets_valid:
        print("[ERROR] Data validation failed. Exiting.")
        return 1
    
    # Calculate metrics
    calculate_trading_metrics(trade_items, markets)
    
    # Generate report
    generate_trading_report(trade_items, markets)
    
    print("\n" + "=" * 60)
    print("[OK] TRADING SYSTEM TEST COMPLETED SUCCESSFULLY!")
    print("=" * 60)
    
    return 0

if __name__ == "__main__":
    sys.exit(main())