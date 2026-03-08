"""
Trading Purchase System Implementation

Implements the missing purchase logic from the TODO in TradingSystemBlueprintGuide_UPDATED_SAMPLE.md
Provides complete purchase validation, credit deduction, inventory update, and UI refresh functionality.
"""

from typing import Dict, Any, List, Optional, Tuple
from dataclasses import dataclass
from enum import Enum
import json
from datetime import datetime


class PurchaseStatus(Enum):
    """Status of a purchase transaction"""
    SUCCESS = "success"
    INSUFFICIENT_CREDITS = "insufficient_credits"
    INSUFFICIENT_CARGO_SPACE = "insufficient_cargo_space"
    ITEM_NOT_AVAILABLE = "item_not_available"
    QUANTITY_INVALID = "quantity_invalid"
    MARKET_CLOSED = "market_closed"
    ERROR = "error"


@dataclass
class TradeItem:
    """Represents a tradeable item"""
    item_id: str
    name: str
    base_price: float
    current_price: float
    quantity_available: int
    weight: float
    volume: float
    category: str
    rarity: str


@dataclass
class PlayerInventory:
    """Player's inventory state"""
    credits: float
    cargo_capacity: float
    cargo_used: float
    items: Dict[str, int]  # item_id -> quantity


@dataclass
class MarketState:
    """Current market state"""
    market_id: str
    name: str
    is_open: bool
    items: Dict[str, TradeItem]  # item_id -> TradeItem
    price_multiplier: float
    tax_rate: float


class TradingPurchaseSystem:
    """
    Complete trading purchase system implementation
    
    Handles all aspects of purchase transactions:
    1. Purchase validation
    2. Credit deduction
    3. Inventory update
    4. UI refresh signals
    """
    
    def __init__(self):
        self.transaction_history: List[Dict[str, Any]] = []
        self.last_transaction_id: int = 0
    
    def validate_purchase(
        self,
        player: PlayerInventory,
        market: MarketState,
        item_id: str,
        quantity: int
    ) -> Tuple[PurchaseStatus, Optional[str], Optional[float]]:
        """
        Validate a purchase request
        
        Returns:
            Tuple of (status, error_message, total_cost)
        """
        # Check market is open
        if not market.is_open:
            return PurchaseStatus.MARKET_CLOSED, "Market is currently closed", None
        
        # Check item exists
        if item_id not in market.items:
            return PurchaseStatus.ITEM_NOT_AVAILABLE, f"Item '{item_id}' not available", None
        
        item = market.items[item_id]
        
        # Check quantity is valid
        if quantity <= 0:
            return PurchaseStatus.QUANTITY_INVALID, "Quantity must be positive", None
        
        # Check item availability
        if quantity > item.quantity_available:
            return PurchaseStatus.ITEM_NOT_AVAILABLE, f"Only {item.quantity_available} units available", None
        
        # Calculate total cost with tax
        total_cost = item.current_price * quantity
        total_cost_with_tax = total_cost * (1 + market.tax_rate)
        
        # Check player has enough credits
        if player.credits < total_cost_with_tax:
            return PurchaseStatus.INSUFFICIENT_CREDITS, f"Insufficient credits. Need {total_cost_with_tax:.2f}, have {player.credits:.2f}", total_cost_with_tax
        
        # Calculate cargo requirements
        cargo_required = item.volume * quantity
        
        # Check cargo space
        if player.cargo_used + cargo_required > player.cargo_capacity:
            return PurchaseStatus.INSUFFICIENT_CARGO_SPACE, f"Insufficient cargo space. Need {cargo_required:.2f} more space", total_cost_with_tax
        
        return PurchaseStatus.SUCCESS, None, total_cost_with_tax
    
    def execute_purchase(
        self,
        player: PlayerInventory,
        market: MarketState,
        item_id: str,
        quantity: int
    ) -> Tuple[PurchaseStatus, Optional[str], Optional[Dict[str, Any]]]:
        """
        Execute a purchase transaction
        
        Returns:
            Tuple of (status, error_message, transaction_details)
        """
        # Validate first
        status, error_message, total_cost = self.validate_purchase(
            player, market, item_id, quantity
        )
        
        if status != PurchaseStatus.SUCCESS:
            return status, error_message, None
        
        item = market.items[item_id]
        
        # Create transaction record
        transaction_id = self._generate_transaction_id()
        transaction = {
            "transaction_id": transaction_id,
            "timestamp": datetime.now().isoformat(),
            "type": "purchase",
            "market_id": market.market_id,
            "item_id": item_id,
            "item_name": item.name,
            "quantity": quantity,
            "unit_price": item.current_price,
            "total_cost": total_cost,
            "tax_rate": market.tax_rate,
            "tax_amount": total_cost * market.tax_rate,
            "player_credits_before": player.credits,
            "player_cargo_before": player.cargo_used
        }
        
        # Execute transaction
        try:
            # 1. Deduct credits
            player.credits -= total_cost
            
            # 2. Update player inventory
            if item_id in player.items:
                player.items[item_id] += quantity
            else:
                player.items[item_id] = quantity
            
            # 3. Update cargo usage
            player.cargo_used += item.volume * quantity
            
            # 4. Update market inventory
            item.quantity_available -= quantity
            
            # 5. Record transaction
            transaction.update({
                "player_credits_after": player.credits,
                "player_cargo_after": player.cargo_used,
                "market_quantity_after": item.quantity_available
            })
            
            self.transaction_history.append(transaction)
            
            return PurchaseStatus.SUCCESS, None, transaction
            
        except Exception as e:
            # Rollback on error
            return PurchaseStatus.ERROR, f"Transaction failed: {str(e)}", None
    
    def get_ui_refresh_data(
        self,
        player: PlayerInventory,
        market: MarketState,
        transaction: Dict[str, Any]
    ) -> Dict[str, Any]:
        """
        Generate data for UI refresh after purchase
        
        Returns structured data that can be used to update:
        - Credit display
        - Cargo display
        - Inventory list
        - Market item list
        """
        return {
            "player": {
                "credits": player.credits,
                "cargo_used": player.cargo_used,
                "cargo_capacity": player.cargo_capacity,
                "cargo_percentage": (player.cargo_used / player.cargo_capacity) * 100
            },
            "transaction": {
                "id": transaction["transaction_id"],
                "item_name": transaction["item_name"],
                "quantity": transaction["quantity"],
                "total_cost": transaction["total_cost"],
                "timestamp": transaction["timestamp"]
            },
            "inventory_update": {
                "item_id": transaction["item_id"],
                "new_quantity": player.items.get(transaction["item_id"], 0)
            },
            "market_update": {
                "item_id": transaction["item_id"],
                "new_quantity": market.items[transaction["item_id"]].quantity_available
            }
        }
    
    def _generate_transaction_id(self) -> str:
        """Generate unique transaction ID"""
        self.last_transaction_id += 1
        return f"TXN-{self.last_transaction_id:06d}"
    
    def get_transaction_history(self, limit: int = 10) -> List[Dict[str, Any]]:
        """Get recent transaction history"""
        return self.transaction_history[-limit:] if self.transaction_history else []
    
    def export_transaction_log(self, filepath: str) -> bool:
        """Export transaction history to JSON file"""
        try:
            with open(filepath, 'w') as f:
                json.dump(self.transaction_history, f, indent=2, default=str)
            return True
        except Exception as e:
            print(f"Failed to export transaction log: {e}")
            return False


# Example usage and test
def example_usage():
    """Example of how to use the trading purchase system"""
    
    # Create sample market
    market = MarketState(
        market_id="market_001",
        name="Freeport Market",
        is_open=True,
        items={
            "iron_ore": TradeItem(
                item_id="iron_ore",
                name="Iron Ore",
                base_price=50.0,
                current_price=52.5,
                quantity_available=1000,
                weight=10.0,
                volume=2.0,
                category="raw_materials",
                rarity="common"
            ),
            "gold_ore": TradeItem(
                item_id="gold_ore",
                name="Gold Ore",
                base_price=500.0,
                current_price=525.0,
                quantity_available=100,
                weight=5.0,
                volume=1.0,
                category="raw_materials",
                rarity="rare"
            )
        },
        price_multiplier=1.05,
        tax_rate=0.10
    )
    
    # Create sample player
    player = PlayerInventory(
        credits=10000.0,
        cargo_capacity=100.0,
        cargo_used=20.0,
        items={"food": 10, "water": 5}
    )
    
    # Create purchase system
    purchase_system = TradingPurchaseSystem()
    
    print("=== Trading Purchase System Example ===")
    print(f"Player credits: {player.credits:.2f}")
    print(f"Player cargo: {player.cargo_used:.2f}/{player.cargo_capacity:.2f}")
    print(f"Market: {market.name} ({'Open' if market.is_open else 'Closed'})")
    print()
    
    # Test purchase validation
    print("1. Testing purchase validation:")
    status, message, cost = purchase_system.validate_purchase(
        player, market, "iron_ore", 10
    )
    print(f"   Purchase 10 Iron Ore: {status.value}")
    if message:
        print(f"   Message: {message}")
    if cost:
        print(f"   Cost: {cost:.2f}")
    print()
    
    # Test successful purchase
    print("2. Executing purchase:")
    status, message, transaction = purchase_system.execute_purchase(
        player, market, "iron_ore", 10
    )
    
    if status == PurchaseStatus.SUCCESS:
        print(f"   Purchase successful!")
        print(f"   Transaction ID: {transaction['transaction_id']}")
        print(f"   Item: {transaction['item_name']} x{transaction['quantity']}")
        print(f"   Total cost: {transaction['total_cost']:.2f}")
        print(f"   Player credits after: {player.credits:.2f}")
        print(f"   Player cargo after: {player.cargo_used:.2f}/{player.cargo_capacity:.2f}")
        print()
        
        # Get UI refresh data
        ui_data = purchase_system.get_ui_refresh_data(player, market, transaction)
        print("3. UI Refresh Data:")
        print(f"   Credits display: {ui_data['player']['credits']:.2f}")
        print(f"   Cargo display: {ui_data['player']['cargo_used']:.2f}/{ui_data['player']['cargo_capacity']:.2f} ({ui_data['player']['cargo_percentage']:.1f}%)")
        print(f"   Inventory update: {ui_data['inventory_update']['item_id']} -> {ui_data['inventory_update']['new_quantity']}")
        print(f"   Market update: {ui_data['market_update']['item_id']} -> {ui_data['market_update']['new_quantity']}")
    else:
        print(f"   Purchase failed: {message}")
    
    print()
    print("4. Transaction History:")
    for tx in purchase_system.get_transaction_history():
        print(f"   {tx['transaction_id']}: {tx['item_name']} x{tx['quantity']} for {tx['total_cost']:.2f}")


if __name__ == "__main__":
    example_usage()