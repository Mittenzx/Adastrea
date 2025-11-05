#include "TradeTransaction.h"
#include "TradeItemDataAsset.h"
#include "MarketDataAsset.h"
#include "Misc/DateTime.h"

UTradeTransactionManager::UTradeTransactionManager()
	: MaxHistorySize(10000)
	, CachedLatestTimestamp(0.0f)
	, bCacheValid(false)
{
}

void UTradeTransactionManager::RecordTransaction(const FTradeTransaction& Transaction)
{
	TransactionHistory.Add(Transaction);
	
	// Update cached latest timestamp for performance optimization
	if (Transaction.Timestamp > CachedLatestTimestamp)
	{
		CachedLatestTimestamp = Transaction.Timestamp;
		bCacheValid = true;
	}
	
	// Prune if necessary
	PruneOldTransactions();
}

TArray<FTradeTransaction> UTradeTransactionManager::GetTransactionsByItem(FName ItemID) const
{
	TArray<FTradeTransaction> Result;
	
	// Reserve space based on estimate (assume 10% might match)
	Result.Reserve(TransactionHistory.Num() / 10);
	
	for (const FTradeTransaction& Transaction : TransactionHistory)
	{
		if (Transaction.TradeItem && Transaction.TradeItem->ItemID == ItemID)
		{
			Result.Add(Transaction);
		}
	}
	
	return Result;
}

TArray<FTradeTransaction> UTradeTransactionManager::GetTransactionsByMarket(UMarketDataAsset* Market) const
{
	TArray<FTradeTransaction> Result;
	
	// Reserve space to avoid reallocation
	Result.Reserve(TransactionHistory.Num() / 10);
	
	for (const FTradeTransaction& Transaction : TransactionHistory)
	{
		if (Transaction.Market == Market)
		{
			Result.Add(Transaction);
		}
	}
	
	return Result;
}

TArray<FTradeTransaction> UTradeTransactionManager::GetTransactionsByTrader(FName TraderID) const
{
	TArray<FTradeTransaction> Result;
	
	// Reserve space to avoid reallocation
	Result.Reserve(TransactionHistory.Num() / 10);
	
	for (const FTradeTransaction& Transaction : TransactionHistory)
	{
		if (Transaction.BuyerID == TraderID || Transaction.SellerID == TraderID)
		{
			Result.Add(Transaction);
		}
	}
	
	return Result;
}

TArray<FTradeTransaction> UTradeTransactionManager::GetTransactionsByTimeRange(float StartTime, float EndTime) const
{
	TArray<FTradeTransaction> Result;
	
	for (const FTradeTransaction& Transaction : TransactionHistory)
	{
		if (Transaction.Timestamp >= StartTime && Transaction.Timestamp <= EndTime)
		{
			Result.Add(Transaction);
		}
	}
	
	return Result;
}

int32 UTradeTransactionManager::GetTotalTradeVolume(FName ItemID, float StartTime, float EndTime) const
{
	int32 TotalVolume = 0;
	
	for (const FTradeTransaction& Transaction : TransactionHistory)
	{
		if (Transaction.TradeItem && Transaction.TradeItem->ItemID == ItemID &&
			Transaction.Timestamp >= StartTime && Transaction.Timestamp <= EndTime)
		{
			TotalVolume += Transaction.Quantity;
		}
	}
	
	return TotalVolume;
}

float UTradeTransactionManager::GetAveragePrice(FName ItemID, float StartTime, float EndTime) const
{
	float TotalPrice = 0.0f;
	int32 Count = 0;
	
	for (const FTradeTransaction& Transaction : TransactionHistory)
	{
		if (Transaction.TradeItem && Transaction.TradeItem->ItemID == ItemID &&
			Transaction.Timestamp >= StartTime && Transaction.Timestamp <= EndTime)
		{
			TotalPrice += Transaction.PricePerUnit;
			Count++;
		}
	}
	
	return Count > 0 ? TotalPrice / Count : 0.0f;
}

float UTradeTransactionManager::GetPriceTrend(FName ItemID, float TimeWindow) const
{
	if (TransactionHistory.Num() == 0)
	{
		return 0.0f;
	}

	// Use cached latest timestamp if available, otherwise calculate
	float LatestTime = bCacheValid ? CachedLatestTimestamp : 0.0f;
	if (!bCacheValid)
	{
		for (const FTradeTransaction& Transaction : TransactionHistory)
		{
			if (Transaction.Timestamp > LatestTime)
			{
				LatestTime = Transaction.Timestamp;
			}
		}
	}

	float StartTime = LatestTime - TimeWindow;
	
	// Get first half average and second half average
	float MidTime = StartTime + (TimeWindow / 2.0f);
	
	float FirstHalfAvg = GetAveragePrice(ItemID, StartTime, MidTime);
	float SecondHalfAvg = GetAveragePrice(ItemID, MidTime, LatestTime);
	
	if (FirstHalfAvg <= 0.0f)
	{
		return 0.0f;
	}
	
	// Return percentage change
	return (SecondHalfAvg - FirstHalfAvg) / FirstHalfAvg;
}

TArray<FName> UTradeTransactionManager::GetMostTradedItems(int32 Count, float StartTime, float EndTime) const
{
	// Map item IDs to trade volumes
	TMap<FName, int32> ItemVolumes;
	
	for (const FTradeTransaction& Transaction : TransactionHistory)
	{
		if (Transaction.TradeItem &&
			Transaction.Timestamp >= StartTime && Transaction.Timestamp <= EndTime)
		{
			FName ItemID = Transaction.TradeItem->ItemID;
			if (!ItemVolumes.Contains(ItemID))
			{
				ItemVolumes.Add(ItemID, 0);
			}
			ItemVolumes[ItemID] += Transaction.Quantity;
		}
	}
	
	// Sort by volume
	ItemVolumes.ValueSort([](int32 A, int32 B) {
		return A > B;
	});
	
	// Get top items
	TArray<FName> Result;
	for (const auto& Pair : ItemVolumes)
	{
		Result.Add(Pair.Key);
		if (Result.Num() >= Count)
		{
			break;
		}
	}
	
	return Result;
}

int32 UTradeTransactionManager::GetPlayerProfitLoss(FName PlayerID) const
{
	int32 TotalProfit = 0;
	
	for (const FTradeTransaction& Transaction : TransactionHistory)
	{
		if (Transaction.BuyerID == PlayerID)
		{
			// Player bought - negative to profit
			TotalProfit -= Transaction.TotalValue;
		}
		else if (Transaction.SellerID == PlayerID)
		{
			// Player sold - positive to profit
			TotalProfit += Transaction.TotalValue;
		}
	}
	
	return TotalProfit;
}

void UTradeTransactionManager::ClearHistory()
{
	TransactionHistory.Empty();
	CachedLatestTimestamp = 0.0f;
	bCacheValid = false;
}

FString UTradeTransactionManager::ExportToString() const
{
	// Performance optimization: Pre-allocate string capacity
	// Estimate ~200 characters per transaction line
	const int32 EstimatedSize = TransactionHistory.Num() * 200 + 100;
	
	TArray<FString> Lines;
	Lines.Reserve(TransactionHistory.Num() + 1);
	
	// Add header
	Lines.Add(TEXT("TransactionID,Type,ItemID,Quantity,PricePerUnit,TotalValue,BuyerID,SellerID,Timestamp"));
	
	// Build lines array first (more efficient than repeated string concatenation)
	for (const FTradeTransaction& Transaction : TransactionHistory)
	{
		FString ItemID = Transaction.TradeItem ? Transaction.TradeItem->ItemID.ToString() : TEXT("None");
		Lines.Add(FString::Printf(TEXT("%s,%d,%s,%d,%.2f,%d,%s,%s,%.2f"),
			*Transaction.TransactionID.ToString(),
			(int32)Transaction.TransactionType,
			*ItemID,
			Transaction.Quantity,
			Transaction.PricePerUnit,
			Transaction.TotalValue,
			*Transaction.BuyerID.ToString(),
			*Transaction.SellerID.ToString(),
			Transaction.Timestamp
		));
	}
	
	// Join all lines efficiently with newline delimiter
	return FString::Join(Lines, TEXT("\n"));
}

bool UTradeTransactionManager::ImportFromString(const FString& Data)
{
	// Simple CSV import
	// This is a basic implementation - production code would need more robust parsing
	TArray<FString> Lines;
	Data.ParseIntoArrayLines(Lines);
	
	if (Lines.Num() < 2)
	{
		return false;
	}
	
	// Skip header line
	for (int32 i = 1; i < Lines.Num(); i++)
	{
		TArray<FString> Fields;
		Lines[i].ParseIntoArray(Fields, TEXT(","));
		
		if (Fields.Num() < 9)
		{
			continue;
		}
		
		// Parse fields and create transaction
		// This would need proper implementation with asset lookup
	}
	
	return true;
}

void UTradeTransactionManager::PruneOldTransactions()
{
	if (TransactionHistory.Num() <= MaxHistorySize)
	{
		return;
	}
	
	// Remove oldest transactions
	int32 NumToRemove = TransactionHistory.Num() - MaxHistorySize;
	
	// Sort by timestamp
	TransactionHistory.Sort([](const FTradeTransaction& A, const FTradeTransaction& B) {
		return A.Timestamp < B.Timestamp;
	});
	
	// Remove oldest
	TransactionHistory.RemoveAt(0, NumToRemove);
	
	// Invalidate cache after pruning since we might have removed the latest timestamp
	bCacheValid = false;
}
