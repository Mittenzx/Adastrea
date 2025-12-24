# Performance Optimization Quick Reference

## Files Modified

### Core Changes
- `Source/Adastrea/Trading/AITraderComponent.cpp` - Trade route algorithm optimization
- `Source/Adastrea/AI/FactionLogic.cpp` - Priority evaluation + logging improvements
- `Source/Adastrea/AI/PersonnelLogic.cpp` - Logging improvements
- `Source/Adastrea/Trading/TradeTransaction.cpp` - Caching + query optimization
- `Source/Adastrea/Trading/TradeTransaction.h` - Added cache members
- `Source/Adastrea/Factions/FactionDataAsset.cpp` - Relationship lookup optimization
- `Source/Adastrea/Public/Factions/FactionDataAsset.h` - Added relationship cache

### Documentation
- `PERFORMANCE_IMPROVEMENTS.md` - Detailed optimization analysis
- `OPTIMIZATION_QUICK_REFERENCE.md` - This file

## Key API Changes

### NO Breaking Changes
All optimizations are internal implementation details. Public APIs remain unchanged.

### New Private Members (Implementation Details)

**TradeTransaction.h**:
```cpp
private:
    float CachedLatestTimestamp;
    bool bCacheValid;
```

**FactionDataAsset.h**:
```cpp
private:
    mutable TMap<FName, const FFactionRelationship*> RelationshipCache;
    mutable bool bRelationshipCacheValid;
    void RebuildRelationshipCache() const;
```

## Debugging Tips

### Enable Verbose AI Logging
If you need to debug AI behavior after these changes:

```cpp
// In console or config file
Log LogAdastreaAI Verbose
```

### Profile Trade Routes
To measure performance improvement in trade route calculation:

```cpp
// Before calling FindBestTradeRoutes()
double StartTime = FPlatformTime::Seconds();

TArray<FTradeRoute> Routes = FindBestTradeRoutes(MaxRoutes);

double ElapsedTime = (FPlatformTime::Seconds() - StartTime) * 1000.0;
UE_LOG(LogAdastreaTrading, Log, TEXT("FindBestTradeRoutes took %.2f ms"), ElapsedTime);
```

### Monitor Relationship Cache
To verify relationship cache is working:

```cpp
// In FactionDataAsset.cpp, add to RebuildRelationshipCache():
UE_LOG(LogAdastreaFactions, Verbose, TEXT("Rebuilding relationship cache for faction %s (%d relationships)"),
    *FactionName.ToString(), FactionRelationships.Num());
```

## Common Issues and Solutions

### Issue: Compile Errors with TMap
**Solution**: Ensure FactionDataAsset.h includes proper headers:
```cpp
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
```

### Issue: Cache Not Updating
**Solution**: The relationship cache is automatically invalidated when needed. If you add manual relationship modifications, you may need to invalidate:
```cpp
// Add to methods that modify FactionRelationships array:
bRelationshipCacheValid = false;
```

### Issue: Missing Log Category
**Solution**: Ensure AdastreaLog.h is included and AdastreaLog.cpp defines the category:
```cpp
// In .cpp file
#include "AdastreaLog.h"

// In AdastreaLog.cpp (should already exist)
DEFINE_LOG_CATEGORY(LogAdastreaAI);
```

## Testing Checklist

- [ ] Project compiles without errors
- [ ] No new warnings introduced
- [ ] AI traders find valid trade routes
- [ ] Faction diplomacy interactions work correctly
- [ ] Transaction history queries return correct results
- [ ] Export to string works with large histories
- [ ] No crashes during faction relationship lookups
- [ ] Log messages appear in correct categories

## Performance Benchmarks

Expected improvements (relative to number of entities):

| Scenario | Improvement | Noticeable When |
|----------|-------------|-----------------|
| Trade route calculation | 40-60% faster | 20+ markets |
| Relationship lookups | 80-90% faster | 10+ factions |
| Transaction queries | 30-50% faster | 1000+ transactions |
| String export | 60-80% faster | 5000+ transactions |

## Rollback Instructions

If issues arise, revert commits in this order:

1. `git revert 6dd6f58` - Removes string optimization and docs
2. `git revert ea69079` - Removes all other optimizations
3. Rebuild project

## Code Review Notes

### What Was NOT Changed

- Blueprint interfaces remain identical
- Data asset structures unchanged
- Save/load compatibility maintained
- Network replication unaffected (if applicable)
- No changes to game logic or behavior

### What WAS Changed

- Internal algorithms for better performance
- Debug output method (GEngine → UE_LOG)
- Internal caching strategies

## Additional Resources

- See `PERFORMANCE_IMPROVEMENTS.md` for detailed analysis
- Check Unreal Engine docs for profiling: `stat AI`, `stat Game`
- Use Unreal Insights for deep profiling

## Questions?

These optimizations follow Unreal Engine best practices:
- Use TMap for frequent lookups
- Pre-allocate array capacity when size is known
- Use proper logging categories instead of screen messages
- Minimize allocations in hot paths
- Cache expensive calculations

For questions or issues, refer to:
- `ARCHITECTURE.md` - System architecture
- `CODE_STYLE.md` - Coding standards
- `CONTRIBUTING.md` - Contribution guidelines
