#!/usr/bin/env python3
"""
Adastrea Content Generation Quick Start
Run this script to generate content quickly and easily
"""

import sys
import os
from pathlib import Path

def main():
    print("🚀 Adastrea Content Generation Quick Start")
    print("=" * 50)

    # Check if we're in the right directory
    if not Path("Adastrea.uproject").exists():
        print("❌ Error: Please run this script from the Adastrea project root directory")
        print("   (the directory containing Adastrea.uproject)")
        sys.exit(1)

    print("✅ Found Adastrea.uproject - correct directory")

    # Check for required files
    required_files = [
        "UltimateContentGenerator.py",
        "MassContentGenerator.py",
        "AdvancedContentGenerator.py",
        "ScenePopulator.py",
        "Source/Adastrea/AutomatedTestLibrary.cpp",
        "Source/Adastrea/DataValidationLibrary.cpp",
        "Source/Adastrea/Performance/PerformanceBenchmarkLibrary.cpp"
    ]

    missing_files = []
    for file in required_files:
        if not Path(file).exists():
            missing_files.append(file)

    if missing_files:
        print("❌ Missing required files:")
        for file in missing_files:
            print(f"   - {file}")
        print("\nPlease ensure all content generation files are present.")
        sys.exit(1)

    print("✅ All required files found")

    # Options menu
    print("\nSelect content generation option:")
    print("1. 🚀 Generate EVERYTHING (Ultimate Generation)")
    print("2. 📊 Generate Data Assets Only")
    print("3. 🎭 Populate Scene Only")
    print("4. ✅ Run Quality Assurance Only")
    print("5. ⚡ Run Performance Tests Only")
    print("6. 📖 Show Help/Documentation")

    while True:
        try:
            choice = input("\nEnter your choice (1-6): ").strip()

            if choice == "1":
                run_ultimate_generation()
                break
            elif choice == "2":
                run_data_asset_generation()
                break
            elif choice == "3":
                run_scene_population()
                break
            elif choice == "4":
                run_quality_assurance()
                break
            elif choice == "5":
                run_performance_tests()
                break
            elif choice == "6":
                show_help()
                break
            else:
                print("❌ Invalid choice. Please enter 1-6.")
        except KeyboardInterrupt:
            print("\n\n👋 Goodbye!")
            sys.exit(0)

def run_ultimate_generation():
    """Run the ultimate content generation"""
    print("\n🚀 Starting Ultimate Content Generation...")
    print("This will generate:")
    print("  • 1000+ Data Assets (ships, factions, weapons, commodities, quests)")
    print("  • 100+ Blueprints (stations, modules)")
    print("  • 2000+ Scene Actors (ships, stations, asteroids, effects)")
    print("  • Complete quality assurance and performance testing")
    print("\n⚠️  Warning: This may take several minutes and use significant memory")

    confirm = input("\nContinue? (y/N): ").strip().lower()
    if confirm != 'y':
        print("❌ Generation cancelled")
        return

    try:
        # Import and run ultimate generator
        from UltimateContentGenerator import UltimateContentGenerator
        generator = UltimateContentGenerator()
        success = generator.run_full_generation()

        if success:
            print("\n🎉 Ultimate content generation completed successfully!")
            print("📄 Check CONTENT_GENERATION_REPORT.txt for detailed statistics")
        else:
            print("\n❌ Content generation failed. Check the error messages above.")

    except Exception as e:
        print(f"\n❌ Error during generation: {str(e)}")
        print("💡 Make sure you're running this from within Unreal Editor's Python environment")

def run_data_asset_generation():
    """Generate data assets only"""
    print("\n📊 Generating Data Assets...")

    try:
        from MassContentGenerator import MassContentGenerator
        generator = MassContentGenerator()

        print("Generating trading commodities...")
        commodities = generator.generate_trading_commodities(50)

        print("Generating quest database...")
        quests = generator.generate_quest_database(100)

        print("Generating AI behaviors...")
        ai_behaviors = generator.generate_ai_behaviors(30)

        print(f"✅ Generated {len(commodities)} commodities, {len(quests)} quests, {len(ai_behaviors)} AI behaviors")

    except Exception as e:
        print(f"❌ Error: {str(e)}")

def run_scene_population():
    """Populate scene only"""
    print("\n🎭 Populating Scene...")

    try:
        from ScenePopulator import populate_complete_sector
        content = populate_complete_sector()
        total_actors = sum(len(actors) for actors in content.values())
        print(f"✅ Scene populated with {total_actors} actors")

    except Exception as e:
        print(f"❌ Error: {str(e)}")

def run_quality_assurance():
    """Run quality assurance tests"""
    print("\n✅ Running Quality Assurance...")

    try:
        import unreal
        from AutomatedTestLibrary import UAutomatedTestLibrary
        from DataValidationLibrary import UDataValidationLibrary

        world = unreal.UnrealEditorSubsystem().get_editor_world()

        # Run tests
        test_results = []
        tests_passed = UAutomatedTestLibrary.RunAllTests(world, test_results)

        # Run validation
        validation_results = []
        validation_passed = UDataValidationLibrary.ValidateAllAssetsOfClass(unreal.UDataAsset, validation_results)

        passed_count = sum(1 for result in test_results if result.bPassed)
        print(f"✅ Tests: {passed_count}/{len(test_results)} passed")
        print(f"✅ Validation: {len(validation_results)} assets checked")

    except Exception as e:
        print(f"❌ Error: {str(e)}")

def run_performance_tests():
    """Run performance benchmarks"""
    print("\n⚡ Running Performance Tests...")

    try:
        import unreal
        from PerformanceBenchmarkLibrary import UPerformanceBenchmarkLibrary

        world = unreal.UnrealEditorSubsystem().get_editor_world()

        print("Benchmarking ship spawning...")
        ship_result = UPerformanceBenchmarkLibrary.BenchmarkShipSpawning(world, 25)

        print("Benchmarking AI system...")
        ai_result = UPerformanceBenchmarkLibrary.BenchmarkAISystem(world, 5, 1.0)

        print("✅ Performance benchmarks completed")

    except Exception as e:
        print(f"❌ Error: {str(e)}")

def show_help():
    """Show help and documentation"""
    print("\n📖 Adastrea Content Generation Help")
    print("=" * 40)

    print("\n📁 FILE OVERVIEW:")
    print("• UltimateContentGenerator.py - Complete content generation pipeline")
    print("• MassContentGenerator.py - Generate large amounts of content")
    print("• AdvancedContentGenerator.py - Create Data Assets and Blueprints")
    print("• ScenePopulator.py - Populate levels with actors")
    print("• Quality Assurance Libraries - Test and validate content")

    print("\n🎯 CONTENT TYPES GENERATED:")
    print("• Spaceships: Fighters, freighters, capital ships with varied stats")
    print("• Factions: Military, corporate, merchant with relationships")
    print("• Weapons: Lasers, plasma, railguns with damage/range stats")
    print("• Stations: Trading hubs, military bases, mining outposts")
    print("• Commodities: 100+ tradeable goods with market dynamics")
    print("• Quests: 200+ missions with objectives and rewards")
    print("• AI Behaviors: Personality-driven NPC behaviors")
    print("• Environments: Asteroid fields, nebulae, jump gates")

    print("\n🛠️  USAGE IN UNREAL EDITOR:")
    print("1. Open Unreal Editor")
    print("2. Open Python Console (Window → Developer Tools → Python Console)")
    print("3. Run: import UltimateContentGenerator")
    print("4. Run: UltimateContentGenerator.main()")

    print("\n📊 COMMAND LINE USAGE:")
    print("python UltimateContentGenerator.py  # Run ultimate generation")
    print("python MassContentGenerator.py --generate-all")
    print("python ScenePopulator.py --config scene_config.yaml")

    print("\n📚 DOCUMENTATION:")
    print("• MASSIVE_CONTENT_GENERATION_GUIDE.md - Complete usage guide")
    print("• QUALITY_ASSURANCE_SYSTEMS.md - Testing and validation")
    print("• CONTENT_GENERATION_REPORT.txt - Generation statistics")

    print("\n💡 TIPS:")
    print("• Start with smaller generations to test the system")
    print("• Use the quality assurance tools to validate content")
    print("• Check performance benchmarks after generation")
    print("• Save your project before running large generations")

if __name__ == "__main__":
    main()