#!/usr/bin/env python3
"""
Test for Trading Purchase System Implementation

Tests the complete purchase logic implementation that was marked as TODO
in TradingSystemBlueprintGuide_UPDATED_SAMPLE.md
"""

import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from TradingPurchaseSystem import (
    TradingPurchaseSystem,
    PlayerInventory,
    MarketState,
    TradeItem,
    PurchaseStatus
)


def test_purchase_validation():
    """Test purchase validation logic"""
    print("=== Testing Purchase Validation ===")
    
    # Setup
    market = MarketState(
        market_id="test_market",
        name="Test Market",
        is_open=True,
        items={
            "test_item": TradeItem(
                item_id="test_item",
                name="Test Item",
                base_price=100.0,
                current_price=105.0,
                quantity_available=50,
                weight=1.0,
                volume=0.5,
                category="test",
                rarity="common"
            )
        },
        price_multiplier=1.05,
        tax_rate=0.10
    )
    
    player = PlayerInventory(
        credits=2000.0,
        cargo_capacity=50.0,
        cargo_used=10.0,
        items={}
    )
    
    purchase_system = TradingPurchaseSystem()
    
    # Test 1: Valid purchase
    status, message, cost = purchase_system.validate_purchase(
        player, market, "test_item", 10
    )
    assert status == PurchaseStatus.SUCCESS, f"Expected SUCCESS, got {status}"
    assert cost == 105.0 * 10 * 1.10, f"Cost calculation incorrect: {cost}"
    print("  Test 1: Valid purchase passes validation")
    
    # Test 2: Insufficient credits
    player.credits = 100.0
    status, message, cost = purchase_system.validate_purchase(
        player, market, "test_item", 10
    )
    assert status == PurchaseStatus.INSUFFICIENT_CREDITS, f"Expected INSUFFICIENT_CREDITS, got {status}"
    print("  Test 2: Insufficient credits detected")
    
    # Test 3: Insufficient cargo space
    player.credits = 5000.0  # Give enough credits
    player.cargo_used = 46.0  # Only 4.0 space left (needs 5.0)
    status, message, cost = purchase_system.validate_purchase(
        player, market, "test_item", 10  # Needs 5.0 volume
    )
    assert status == PurchaseStatus.INSUFFICIENT_CARGO_SPACE, f"Expected INSUFFICIENT_CARGO_SPACE, got {status}"
    print("  Test 3: Insufficient cargo space detected")
    
    # Test 4: Item not available
    status, message, cost = purchase_system.validate_purchase(
        player, market, "nonexistent_item", 1
    )
    assert status == PurchaseStatus.ITEM_NOT_AVAILABLE, f"Expected ITEM_NOT_AVAILABLE, got {status}"
    print("  Test 4: Non-existent item detected")
    
    # Test 5: Market closed
    market.is_open = False
    status, message, cost = purchase_system.validate_purchase(
        player, market, "test_item", 1
    )
    assert status == PurchaseStatus.MARKET_CLOSED, f"Expected MARKET_CLOSED, got {status}"
    print("  Test 5: Closed market detected")
    
    print("All validation tests passed!\n")


def test_purchase_execution():
    """Test purchase execution logic"""
    print("=== Testing Purchase Execution ===")
    
    # Setup
    market = MarketState(
        market_id="exec_market",
        name="Execution Market",
        is_open=True,
        items={
            "exec_item": TradeItem(
                item_id="exec_item",
                name="Execution Item",
                base_price=50.0,
                current_price=52.5,
                quantity_available=100,
                weight=2.0,
                volume=1.0,
                category="exec",
                rarity="common"
            )
        },
        price_multiplier=1.05,
        tax_rate=0.10
    )
    
    player = PlayerInventory(
        credits=5000.0,
        cargo_capacity=100.0,
        cargo_used=20.0,
        items={"existing_item": 5}
    )
    
    purchase_system = TradingPurchaseSystem()
    
    # Test successful purchase
    initial_credits = player.credits
    initial_cargo = player.cargo_used
    initial_market_quantity = market.items["exec_item"].quantity_available
    
    status, message, transaction = purchase_system.execute_purchase(
        player, market, "exec_item", 10
    )
    
    assert status == PurchaseStatus.SUCCESS, f"Purchase failed: {message}"
    assert transaction is not None, "No transaction returned"
    assert player.credits == initial_credits - (52.5 * 10 * 1.10), "Credits not deducted correctly"
    assert player.cargo_used == initial_cargo + (1.0 * 10), "Cargo not updated correctly"
    assert player.items["exec_item"] == 10, "Item not added to inventory"
    assert market.items["exec_item"].quantity_available == initial_market_quantity - 10, "Market quantity not updated"
    
    print("   Purchase execution successful")
    print(f"  Transaction ID: {transaction['transaction_id']}")
    print(f"  Credits before: {initial_credits:.2f}, after: {player.credits:.2f}")
    print(f"  Cargo before: {initial_cargo:.2f}, after: {player.cargo_used:.2f}")
    print(f"  Market quantity before: {initial_market_quantity}, after: {market.items['exec_item'].quantity_available}")
    
    # Test transaction history
    history = purchase_system.get_transaction_history()
    assert len(history) == 1, f"Expected 1 transaction in history, got {len(history)}"
    assert history[0]["transaction_id"] == transaction["transaction_id"], "Transaction ID mismatch"
    
    print("   Transaction history updated correctly")
    print("All execution tests passed!\n")


def test_ui_refresh_data():
    """Test UI refresh data generation"""
    print("=== Testing UI Refresh Data ===")
    
    # Setup
    market = MarketState(
        market_id="ui_market",
        name="UI Market",
        is_open=True,
        items={
            "ui_item": TradeItem(
                item_id="ui_item",
                name="UI Item",
                base_price=100.0,
                current_price=110.0,
                quantity_available=50,
                weight=1.0,
                volume=0.5,
                category="ui",
                rarity="common"
            )
        },
        price_multiplier=1.10,
        tax_rate=0.05
    )
    
    player = PlayerInventory(
        credits=2000.0,
        cargo_capacity=80.0,
        cargo_used=30.0,
        items={"other_item": 3}
    )
    
    purchase_system = TradingPurchaseSystem()
    
    # Execute a purchase
    status, message, transaction = purchase_system.execute_purchase(
        player, market, "ui_item", 5
    )
    
    assert status == PurchaseStatus.SUCCESS, f"Purchase failed: {message}"
    
    # Get UI refresh data
    ui_data = purchase_system.get_ui_refresh_data(player, market, transaction)
    
    # Verify data structure
    assert "player" in ui_data, "Player data missing"
    assert "transaction" in ui_data, "Transaction data missing"
    assert "inventory_update" in ui_data, "Inventory update missing"
    assert "market_update" in ui_data, "Market update missing"
    
    # Verify values
    assert ui_data["player"]["credits"] == player.credits, "Player credits mismatch"
    assert ui_data["player"]["cargo_used"] == player.cargo_used, "Player cargo mismatch"
    assert ui_data["player"]["cargo_percentage"] == (player.cargo_used / player.cargo_capacity) * 100, "Cargo percentage incorrect"
    
    assert ui_data["transaction"]["id"] == transaction["transaction_id"], "Transaction ID mismatch"
    assert ui_data["transaction"]["item_name"] == "UI Item", "Item name mismatch"
    assert ui_data["transaction"]["quantity"] == 5, "Quantity mismatch"
    
    assert ui_data["inventory_update"]["item_id"] == "ui_item", "Inventory item ID mismatch"
    assert ui_data["inventory_update"]["new_quantity"] == 5, "Inventory quantity mismatch"
    
    assert ui_data["market_update"]["item_id"] == "ui_item", "Market item ID mismatch"
    assert ui_data["market_update"]["new_quantity"] == 45, "Market quantity mismatch"
    
    print("   UI refresh data structure correct")
    print("   All values calculated correctly")
    print("UI refresh tests passed!\n")


def test_error_handling():
    """Test error handling scenarios"""
    print("=== Testing Error Handling ===")
    
    # Setup
    market = MarketState(
        market_id="error_market",
        name="Error Market",
        is_open=True,
        items={
            "error_item": TradeItem(
                item_id="error_item",
                name="Error Item",
                base_price=100.0,
                current_price=100.0,
                quantity_available=10,
                weight=1.0,
                volume=1.0,
                category="error",
                rarity="common"
            )
        },
        price_multiplier=1.0,
        tax_rate=0.0
    )
    
    player = PlayerInventory(
        credits=100.0,
        cargo_capacity=10.0,
        cargo_used=0.0,
        items={}
    )
    
    purchase_system = TradingPurchaseSystem()
    
    # Test invalid quantity
    status, message, transaction = purchase_system.execute_purchase(
        player, market, "error_item", 0
    )
    assert status == PurchaseStatus.QUANTITY_INVALID, f"Expected QUANTITY_INVALID, got {status}"
    print("   Invalid quantity handled")
    
    # Test negative quantity
    status, message, transaction = purchase_system.execute_purchase(
        player, market, "error_item", -5
    )
    assert status == PurchaseStatus.QUANTITY_INVALID, f"Expected QUANTITY_INVALID, got {status}"
    print("   Negative quantity handled")
    
    # Test quantity exceeds availability
    status, message, transaction = purchase_system.execute_purchase(
        player, market, "error_item", 20
    )
    assert status == PurchaseStatus.ITEM_NOT_AVAILABLE, f"Expected ITEM_NOT_AVAILABLE, got {status}"
    print("   Excess quantity handled")
    
    print("All error handling tests passed!\n")


def main():
    """Run all tests"""
    print("=" * 60)
    print("Trading Purchase System Test Suite")
    print("Implementing TODO from TradingSystemBlueprintGuide_UPDATED_SAMPLE.md")
    print("=" * 60)
    print()
    
    try:
        test_purchase_validation()
        test_purchase_execution()
        test_ui_refresh_data()
        test_error_handling()
        
        print("=" * 60)
        print("ALL TESTS PASSED!")
        print("Purchase logic implementation complete and working.")
        print("=" * 60)
        
        # Show example usage
        print("\nExample usage:")
        print("```python")
        print("from TradingPurchaseSystem import TradingPurchaseSystem, PlayerInventory, MarketState")
        print()
        print("# Create system")
        print("purchase_system = TradingPurchaseSystem()")
        print()
        print("# Validate purchase")
        print("status, message, cost = purchase_system.validate_purchase(")
        print("    player, market, 'iron_ore', 10")
        print(")")
        print()
        print("# Execute purchase")
        print("status, message, transaction = purchase_system.execute_purchase(")
        print("    player, market, 'iron_ore', 10")
        print(")")
        print()
        print("# Get UI refresh data")
        print("ui_data = purchase_system.get_ui_refresh_data(player, market, transaction)")
        print("```")
        
    except AssertionError as e:
        print(f"TEST FAILED: {e}")
        return 1
    except Exception as e:
        print(f"UNEXPECTED ERROR: {e}")
        return 1
    
    return 0


if __name__ == "__main__":
    sys.exit(main())