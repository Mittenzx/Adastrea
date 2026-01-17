#!/usr/bin/env python3
"""
MVP Trading Scenario Builder

Creates test scenarios for rapid iteration during MVP development.
Generates pre-configured game states for specific testing needs.

Usage:
    python mvp_scenario_builder.py --scenario <name>
    python mvp_scenario_builder.py --list
"""

import json
import sys
from pathlib import Path
from datetime import datetime


class ScenarioBuilder:
    """Builds test scenarios for MVP trading"""
    
    def __init__(self):
        self.scenarios = {
            'first_trade': self.scenario_first_trade,
            'profitable_loop': self.scenario_profitable_loop,
            'cargo_management': self.scenario_cargo_management,
            'price_comparison': self.scenario_price_comparison,
            'upgrade_path': self.scenario_upgrade_path,
            'economic_crash': self.scenario_economic_crash,
            'trading_tutorial': self.scenario_trading_tutorial,
        }
    
    def scenario_first_trade(self) -> dict:
        """Scenario: Player's first trade"""
        return {
            'name': 'First Trade',
            'description': 'Tutorial scenario for first-time player',
            'difficulty': 'Beginner',
            'estimated_time': '5 minutes',
            
            'player_state': {
                'credits': 1000,
                'cargo_capacity': 10,
                'cargo_contents': {},
                'location': 'Agricultural Station',
                'ship_type': 'starter'
            },
            
            'stations': [
                {
                    'name': 'Agricultural Station Alpha',
                    'type': 'agricultural',
                    'location': (0, 0, 0),
                    'market': 'agricultural_station_market',
                    'enabled': True,
                    'highlight': True,  # Tutorial highlight
                },
                {
                    'name': 'Industrial Station Beta',
                    'type': 'industrial',
                    'location': (50000, 0, 0),  # 50km away, ~1 min flight
                    'market': 'industrial_station_market',
                    'enabled': True,
                    'highlight': True,
                }
            ],
            
            'objectives': [
                {
                    'id': 'buy_water',
                    'description': 'Buy Water at Agricultural Station',
                    'type': 'purchase',
                    'item': 'water_purified',
                    'quantity_min': 1,
                    'reward_credits': 0
                },
                {
                    'id': 'fly_to_industrial',
                    'description': 'Fly to Industrial Station Beta',
                    'type': 'navigation',
                    'target': 'Industrial Station Beta',
                    'reward_credits': 0
                },
                {
                    'id': 'sell_water',
                    'description': 'Sell Water at Industrial Station',
                    'type': 'sale',
                    'item': 'water_purified',
                    'quantity_min': 1,
                    'reward_credits': 100
                }
            ],
            
            'tutorial_hints': [
                {
                    'trigger': 'game_start',
                    'message': 'Welcome to space trading! You have 1000 credits.',
                    'duration': 5.0
                },
                {
                    'trigger': 'dock_agricultural',
                    'message': 'Press T to open trading menu',
                    'duration': 3.0
                },
                {
                    'trigger': 'open_trading_ui',
                    'message': 'Water is cheap here. Buy some to resell!',
                    'duration': 5.0
                }
            ],
            
            'success_criteria': {
                'profit_made': 100,
                'trades_completed': 2
            }
        }
    
    def scenario_profitable_loop(self) -> dict:
        """Scenario: Test profitable trade loop"""
        return {
            'name': 'Profitable Loop',
            'description': 'Optimized route for maximum profit testing',
            'difficulty': 'Intermediate',
            'estimated_time': '10 minutes',
            
            'player_state': {
                'credits': 5000,
                'cargo_capacity': 30,
                'cargo_contents': {},
                'location': 'Research Station',
                'ship_type': 'medium'
            },
            
            'stations': [
                {
                    'name': 'Research Station Gamma',
                    'type': 'research',
                    'location': (0, 0, 0),
                    'market': 'research_station_market',
                    'enabled': True
                },
                {
                    'name': 'Agricultural Station Alpha',
                    'type': 'agricultural',
                    'location': (30000, 30000, 0),
                    'market': 'agricultural_station_market',
                    'enabled': True
                },
                {
                    'name': 'Industrial Station Beta',
                    'type': 'industrial',
                    'location': (60000, 0, 0),
                    'market': 'industrial_station_market',
                    'enabled': True
                }
            ],
            
            'objectives': [
                {
                    'id': 'profit_target',
                    'description': 'Make 3000 credits profit',
                    'type': 'profit',
                    'target_amount': 3000,
                    'reward_credits': 500
                },
                {
                    'id': 'efficiency',
                    'description': 'Complete in under 3 trips',
                    'type': 'efficiency',
                    'max_trips': 3,
                    'reward_credits': 300
                }
            ],
            
            'success_criteria': {
                'profit_made': 3000,
                'time_limit_seconds': 600
            }
        }
    
    def scenario_cargo_management(self) -> dict:
        """Scenario: Test cargo space optimization"""
        return {
            'name': 'Cargo Management',
            'description': 'Learn to optimize cargo space usage',
            'difficulty': 'Intermediate',
            'estimated_time': '8 minutes',
            
            'player_state': {
                'credits': 2000,
                'cargo_capacity': 10,
                'cargo_contents': {
                    'protein_packs': 5,  # Already partially full
                },
                'location': 'Trade Hub',
                'ship_type': 'starter'
            },
            
            'stations': [
                {
                    'name': 'Central Trade Hub',
                    'type': 'trade_hub',
                    'location': (0, 0, 0),
                    'market': 'trade_hub_market',
                    'enabled': True
                },
                {
                    'name': 'Luxury Bazaar',
                    'type': 'luxury',
                    'location': (40000, 40000, 0),
                    'market': 'luxury_bazaar_market',
                    'enabled': True
                }
            ],
            
            'objectives': [
                {
                    'id': 'cargo_efficiency',
                    'description': 'Fill remaining cargo with high-value items',
                    'type': 'cargo_optimization',
                    'min_value_density': 50,  # 50 cr/m³
                    'reward_credits': 200
                },
                {
                    'id': 'profit_with_limits',
                    'description': 'Make 1000cr profit despite limited space',
                    'type': 'profit',
                    'target_amount': 1000,
                    'reward_credits': 300
                }
            ],
            
            'tutorial_hints': [
                {
                    'trigger': 'open_trading_ui',
                    'message': 'Limited cargo space! Choose high-value items.',
                    'duration': 4.0
                }
            ],
            
            'success_criteria': {
                'profit_made': 1000,
                'cargo_efficiency_min': 50
            }
        }
    
    def scenario_price_comparison(self) -> dict:
        """Scenario: Learn price comparison across markets"""
        return {
            'name': 'Price Comparison',
            'description': 'Compare prices across 4 stations',
            'difficulty': 'Beginner',
            'estimated_time': '12 minutes',
            
            'player_state': {
                'credits': 1500,
                'cargo_capacity': 10,
                'cargo_contents': {},
                'location': 'Trade Hub',
                'ship_type': 'starter'
            },
            
            'stations': [
                {
                    'name': 'Central Trade Hub',
                    'type': 'trade_hub',
                    'location': (0, 0, 0),
                    'market': 'trade_hub_market',
                    'enabled': True,
                    'highlight': True
                },
                {
                    'name': 'Agricultural Station',
                    'type': 'agricultural',
                    'location': (30000, 0, 0),
                    'market': 'agricultural_station_market',
                    'enabled': True,
                    'highlight': True
                },
                {
                    'name': 'Industrial Station',
                    'type': 'industrial',
                    'location': (0, 30000, 0),
                    'market': 'industrial_station_market',
                    'enabled': True,
                    'highlight': True
                },
                {
                    'name': 'Research Station',
                    'type': 'research',
                    'location': (30000, 30000, 0),
                    'market': 'research_station_market',
                    'enabled': True,
                    'highlight': True
                }
            ],
            
            'objectives': [
                {
                    'id': 'visit_all',
                    'description': 'Visit all 4 stations',
                    'type': 'exploration',
                    'stations_required': 4,
                    'reward_credits': 100
                },
                {
                    'id': 'find_best_route',
                    'description': 'Find the most profitable route',
                    'type': 'discovery',
                    'min_roi': 50,
                    'reward_credits': 200
                }
            ],
            
            'tutorial_hints': [
                {
                    'trigger': 'game_start',
                    'message': 'Different stations have different prices. Explore to find the best deals!',
                    'duration': 6.0
                }
            ],
            
            'success_criteria': {
                'stations_visited': 4,
                'best_route_found': True
            }
        }
    
    def scenario_upgrade_path(self) -> dict:
        """Scenario: Work toward ship upgrade"""
        return {
            'name': 'Ship Upgrade Path',
            'description': 'Earn enough for medium ship upgrade',
            'difficulty': 'Advanced',
            'estimated_time': '30 minutes',
            
            'player_state': {
                'credits': 10000,  # Partway to upgrade
                'cargo_capacity': 10,
                'cargo_contents': {},
                'location': 'Trade Hub',
                'ship_type': 'starter',
                'upgrade_available': {
                    'name': 'Medium Trader',
                    'cost': 50000,
                    'cargo_capacity': 30,
                    'speed_multiplier': 0.9
                }
            },
            
            'stations': [
                {
                    'name': 'Central Trade Hub',
                    'type': 'trade_hub',
                    'location': (0, 0, 0),
                    'market': 'trade_hub_market',
                    'enabled': True
                },
                {
                    'name': 'Agricultural Station',
                    'type': 'agricultural',
                    'location': (40000, 0, 0),
                    'market': 'agricultural_station_market',
                    'enabled': True
                },
                {
                    'name': 'Industrial Station',
                    'type': 'industrial',
                    'location': (0, 40000, 0),
                    'market': 'industrial_station_market',
                    'enabled': True
                },
                {
                    'name': 'Research Station',
                    'type': 'research',
                    'location': (40000, 40000, 0),
                    'market': 'research_station_market',
                    'enabled': True
                },
                {
                    'name': 'Luxury Bazaar',
                    'type': 'luxury',
                    'location': (80000, 20000, 0),
                    'market': 'luxury_bazaar_market',
                    'enabled': True
                }
            ],
            
            'objectives': [
                {
                    'id': 'earn_40k',
                    'description': 'Earn 40,000 more credits',
                    'type': 'profit',
                    'target_amount': 40000,
                    'reward_credits': 0
                },
                {
                    'id': 'purchase_upgrade',
                    'description': 'Purchase Medium Trader upgrade',
                    'type': 'upgrade',
                    'upgrade_name': 'Medium Trader',
                    'reward_credits': 5000  # Bonus on completion
                }
            ],
            
            'success_criteria': {
                'total_credits': 50000,
                'upgrade_purchased': True
            }
        }
    
    def scenario_economic_crash(self) -> dict:
        """Scenario: Test recovery from bad trades"""
        return {
            'name': 'Economic Crash',
            'description': 'Recover from poor trading decisions',
            'difficulty': 'Hard',
            'estimated_time': '15 minutes',
            
            'player_state': {
                'credits': 200,  # Nearly broke
                'cargo_capacity': 10,
                'cargo_contents': {
                    'rare_art': 1,  # Expensive item stuck in cargo
                },
                'location': 'Industrial Station',
                'ship_type': 'starter'
            },
            
            'stations': [
                {
                    'name': 'Industrial Station',
                    'type': 'industrial',
                    'location': (0, 0, 0),
                    'market': 'industrial_station_market',
                    'enabled': True
                },
                {
                    'name': 'Trade Hub',
                    'type': 'trade_hub',
                    'location': (30000, 0, 0),
                    'market': 'trade_hub_market',
                    'enabled': True
                },
                {
                    'name': 'Luxury Bazaar',
                    'type': 'luxury',
                    'location': (60000, 0, 0),
                    'market': 'luxury_bazaar_market',
                    'enabled': True
                }
            ],
            
            'objectives': [
                {
                    'id': 'sell_art',
                    'description': 'Sell Rare Art at Luxury Bazaar',
                    'type': 'sale',
                    'item': 'rare_art',
                    'quantity_min': 1,
                    'reward_credits': 0
                },
                {
                    'id': 'recover_capital',
                    'description': 'Build back to 2000 credits',
                    'type': 'profit',
                    'target_amount': 1800,
                    'reward_credits': 500
                }
            ],
            
            'tutorial_hints': [
                {
                    'trigger': 'game_start',
                    'message': 'You made a bad trade. Rare Art sells best at Luxury markets.',
                    'duration': 5.0
                }
            ],
            
            'success_criteria': {
                'total_credits': 2000,
                'item_sold': 'rare_art'
            }
        }
    
    def scenario_trading_tutorial(self) -> dict:
        """Scenario: Complete tutorial sequence"""
        return {
            'name': 'Trading Tutorial',
            'description': 'Full tutorial covering all trading mechanics',
            'difficulty': 'Beginner',
            'estimated_time': '20 minutes',
            
            'player_state': {
                'credits': 1000,
                'cargo_capacity': 10,
                'cargo_contents': {},
                'location': 'Trade Hub',
                'ship_type': 'starter',
                'tutorial_progress': 0
            },
            
            'stations': [
                {
                    'name': 'Central Trade Hub',
                    'type': 'trade_hub',
                    'location': (0, 0, 0),
                    'market': 'trade_hub_market',
                    'enabled': True,
                    'tutorial_station': True
                },
                {
                    'name': 'Agricultural Station',
                    'type': 'agricultural',
                    'location': (30000, 0, 0),
                    'market': 'agricultural_station_market',
                    'enabled': True,
                    'tutorial_station': True
                },
                {
                    'name': 'Industrial Station',
                    'type': 'industrial',
                    'location': (60000, 0, 0),
                    'market': 'industrial_station_market',
                    'enabled': True,
                    'tutorial_station': True
                }
            ],
            
            'tutorial_steps': [
                {
                    'id': 'step1_navigation',
                    'title': 'Navigation',
                    'description': 'Learn to fly to stations',
                    'objectives': ['Dock at Agricultural Station'],
                    'hints': ['Use WASD to fly', 'Get close to station to dock']
                },
                {
                    'id': 'step2_buying',
                    'title': 'Buying Goods',
                    'description': 'Learn to purchase trade goods',
                    'objectives': ['Buy Water (any quantity)'],
                    'hints': ['Press T when docked', 'Click Buy button']
                },
                {
                    'id': 'step3_selling',
                    'title': 'Selling Goods',
                    'description': 'Learn to sell for profit',
                    'objectives': ['Sell Water at Industrial Station'],
                    'hints': ['Different stations have different prices']
                },
                {
                    'id': 'step4_profit',
                    'title': 'Making Profit',
                    'description': 'Complete a profitable trade',
                    'objectives': ['Make 100cr profit'],
                    'hints': ['Buy low, sell high']
                },
                {
                    'id': 'step5_cargo',
                    'title': 'Cargo Management',
                    'description': 'Learn about cargo capacity',
                    'objectives': ['Fill cargo to 80%+'],
                    'hints': ['Watch cargo bar in trading UI']
                }
            ],
            
            'success_criteria': {
                'tutorial_completed': True,
                'all_steps_passed': True
            }
        }
    
    def build_scenario(self, scenario_name: str) -> dict:
        """Build a specific scenario"""
        if scenario_name not in self.scenarios:
            raise ValueError(f"Unknown scenario: {scenario_name}")
        
        scenario_func = self.scenarios[scenario_name]
        scenario_data = scenario_func()
        
        # Add metadata
        scenario_data['generated_at'] = datetime.now().isoformat()
        scenario_data['version'] = '1.0'
        scenario_data['scenario_id'] = scenario_name
        
        return scenario_data
    
    def list_scenarios(self):
        """List all available scenarios"""
        print("=" * 60)
        print("AVAILABLE SCENARIOS")
        print("=" * 60)
        print()
        
        for scenario_id in self.scenarios:
            scenario = self.build_scenario(scenario_id)
            print(f"📦 {scenario['name']}")
            print(f"   ID: {scenario_id}")
            print(f"   Difficulty: {scenario['difficulty']}")
            print(f"   Time: {scenario['estimated_time']}")
            print(f"   Description: {scenario['description']}")
            print()
    
    def export_scenario(self, scenario_name: str, output_path: Path):
        """Export scenario to JSON file"""
        scenario = self.build_scenario(scenario_name)
        
        output_path.parent.mkdir(parents=True, exist_ok=True)
        
        with open(output_path, 'w') as f:
            json.dump(scenario, f, indent=2)
        
        print(f"✅ Exported scenario to: {output_path}")
        print(f"   Scenario: {scenario['name']}")
        print(f"   Difficulty: {scenario['difficulty']}")
        print(f"   Estimated Time: {scenario['estimated_time']}")


def main():
    """Main entry point"""
    import argparse
    
    parser = argparse.ArgumentParser(description="MVP Trading Scenario Builder")
    parser.add_argument('--list', '-l', action='store_true', help='List all available scenarios')
    parser.add_argument('--scenario', '-s', type=str, help='Scenario to build')
    parser.add_argument('--output', '-o', type=str, help='Output file path (JSON)')
    
    args = parser.parse_args()
    
    builder = ScenarioBuilder()
    
    if args.list:
        builder.list_scenarios()
        return 0
    
    if args.scenario:
        output_path = Path(args.output) if args.output else Path(f"scenarios/{args.scenario}.json")
        
        try:
            builder.export_scenario(args.scenario, output_path)
            return 0
        except ValueError as e:
            print(f"❌ Error: {e}")
            print("\nUse --list to see available scenarios")
            return 1
    
    # Default: show help
    parser.print_help()
    return 0


if __name__ == "__main__":
    sys.exit(main())
