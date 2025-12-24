# Advanced Quality Assurance Systems

This document describes the advanced performance monitoring, benchmarking, data validation, and automated testing systems implemented for the Adastrea project.

## Overview

The Adastrea project now includes comprehensive quality assurance systems that provide:

- **Performance Benchmarking**: Measure and analyze system performance across ships, combat, AI, stations, and LOD
- **Real-time Performance Monitoring**: Track performance metrics for individual actors with alerts
- **Data Validation**: Comprehensive validation of Data Assets with detailed error reporting
- **Automated Testing**: Unit tests, integration tests, and performance regression tests

## Performance Benchmarking Library

### Usage

```cpp
// Run comprehensive ship spawning benchmark
FString Result = UPerformanceBenchmarkLibrary::BenchmarkShipSpawning(WorldContext, NumShips);

// Run AI system performance test
FString Result = UPerformanceBenchmarkLibrary::BenchmarkAISystem(WorldContext, NumAgents, Duration);

// Run combat system benchmark
FString Result = UPerformanceBenchmarkLibrary::BenchmarkCombatSystem(WorldContext, NumShips, Duration);

// Run station operations benchmark
FString Result = UPerformanceBenchmarkLibrary::BenchmarkStationOperations(WorldContext, NumStations);

// Run LOD performance test
FString Result = UPerformanceBenchmarkLibrary::BenchmarkLODSystem(WorldContext, NumObjects);
```

### Blueprint Integration

All benchmarking functions are available as Blueprint-callable functions in the `UPerformanceBenchmarkLibrary` class.

## Performance Monitor Component

### Setup

1. Add the `UPerformanceMonitorComponent` to any actor
2. Configure monitoring parameters in the editor:
   - `bEnableFrameTimeTracking`: Track frame time
   - `bEnableMemoryTracking`: Monitor memory usage
   - `FrameTimeThreshold`: Alert threshold for frame time (ms)
   - `MemoryThreshold`: Alert threshold for memory usage (MB)

### Usage

```cpp
// Get performance summary
FString Summary = PerformanceMonitor->GetPerformanceSummary();

// Check if performance thresholds exceeded
bool bThresholdExceeded = PerformanceMonitor->CheckPerformanceThresholds();

// Get current metrics
float FrameTime = PerformanceMonitor->GetCurrentFrameTime();
float MemoryUsage = PerformanceMonitor->GetCurrentMemoryUsage();
```

### Blueprint Events

- `OnPerformanceAlert`: Fired when performance thresholds are exceeded
- `OnFrameTimeSpike`: Fired when frame time spikes occur

## Data Validation Library

### Usage

```cpp
// Validate all spaceship data assets
TArray<FDataValidationResult> Results;
bool bValid = UDataValidationLibrary::ValidateAllSpaceships(Results);

// Validate specific spaceship
FDataValidationResult Result;
bool bValid = UDataValidationLibrary::ValidateSpaceshipData(ShipAsset, Result);

// Validate faction relationships
bool bValid = UDataValidationLibrary::ValidateFactionRelationships(Results);

// Validate weapon balance
bool bValid = UDataValidationLibrary::ValidateWeaponBalance(WeaponAsset, Result);

// Batch validate all assets
bool bValid = UDataValidationLibrary::ValidateAllAssetsOfClass(UDataAsset::StaticClass(), Results);
```

### Validation Rules

- **Spaceships**: Rating ranges (0-100), role suitability, size categories
- **Factions**: Relationship ranges (-100 to 100), allied/hostile consistency
- **Weapons**: Positive damage values, valid firing arcs, time calculations
- **Balance**: Damage mitigation, armor calculations, performance metrics

## Automated Testing Framework

### Running Tests

```cpp
// Run all tests
TArray<FTestResult> Results;
bool bAllPassed = UAutomatedTestLibrary::RunAllTests(WorldContext, Results);

// Run specific category
bool bPassed = UAutomatedTestLibrary::RunTestCategory(WorldContext, ETestCategory::UnitTests, Results);

// Run single test
FTestResult Result;
bool bPassed = UAutomatedTestLibrary::RunSingleTest(WorldContext, TEXT("SpaceshipCalculations"), Result);
```

### Test Categories

- **Unit Tests**: Individual system validation (spaceship calculations, faction relationships, weapon damage, data validation)
- **Integration Tests**: Cross-system interactions (spaceship-weapon, faction-station, combat system)
- **Performance Tests**: Performance regression testing (spawn performance, AI performance, data loading)
- **Data Validation Tests**: Comprehensive asset validation

### Blueprint Integration

Use `UAutomatedTestBlueprintLibrary` for Blueprint access:

```blueprint
// Run all tests
Results = Run All Tests (World Context)

// Get summary
Summary = Get Test Summary (Results)

// Export results
Export Test Results (Results)
```

### Test Runner Actor

Place `ATestRunnerActor` in your level to run tests:

```cpp
// Run tests programmatically
TestRunner->RunAllTests();
TestRunner->RunTestCategory(ETestCategory::PerformanceTests);
TestRunner->RunSingleTest(TEXT("SpaceshipCalculations"));
```

## Integration with Unreal Engine

### Build System

All new systems are automatically included in the build through the standard Unreal Build Tool process. No manual configuration required.

### Editor Integration

- Performance monitoring components work in both editor and runtime
- Test runner actors can be placed in editor worlds for testing
- Data validation runs automatically on asset saves (configurable)

### Logging

All systems use Unreal's logging system:
- Performance alerts: `LogTemp` with `Warning` level
- Test results: `LogTemp` with `Log` level
- Validation errors: `LogTemp` with `Error` level

## Best Practices

### Performance Monitoring

1. Add performance monitors to critical actors (player ship, major stations)
2. Set appropriate thresholds based on target hardware
3. Monitor in development builds to catch performance regressions
4. Use alerts to identify performance bottlenecks

### Testing

1. Run unit tests regularly during development
2. Use integration tests for cross-system validation
3. Run performance tests before releases
4. Review test results and fix failures immediately

### Data Validation

1. Enable validation on all Data Assets
2. Review validation results after content changes
3. Use batch validation for comprehensive checks
4. Fix validation errors before committing changes

### Benchmarking

1. Establish performance baselines for your target hardware
2. Run benchmarks after significant changes
3. Compare results against baselines to detect regressions
4. Use results to guide optimization efforts

## Troubleshooting

### Common Issues

**Tests not running**: Ensure WorldContextObject is valid and not null

**Performance alerts not firing**: Check threshold values are appropriate for your system

**Validation not working**: Verify Data Assets are properly loaded and accessible

**Build errors**: Check that all dependencies are included in the build system

### Debug Information

Enable verbose logging to see detailed information:
```cpp
UE_LOG(LogTemp, Verbose, TEXT("Detailed performance information"));
```

## Future Enhancements

- CI/CD integration for automated test runs
- Performance profiling integration with external tools
- Extended validation rules for new asset types
- Automated performance regression detection
- Test result visualization in editor UI

## Files Added

- `PerformanceBenchmarkLibrary.h/.cpp`: Performance benchmarking functions
- `PerformanceMonitorComponent.h/.cpp`: Real-time performance monitoring
- `DataValidationLibrary.h/.cpp`: Data validation functions
- `AutomatedTestLibrary.h/.cpp`: Automated testing framework
- `AutomatedTestBlueprintLibrary.h/.cpp`: Blueprint integration for testing
- `TestRunnerActor.h/.cpp`: Actor for running tests in-game

---

**Version**: 1.0
**Date**: 2025-01-08
**Systems**: Performance Monitoring, Benchmarking, Data Validation, Automated Testing