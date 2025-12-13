"""
Interactive setup assistant for AdastreaDirector
"""
import os
import sys
import webbrowser
from pathlib import Path

def clear_screen():
    """Clear the terminal screen"""
    os.system('cls' if os.name == 'nt' else 'clear')

def wait_for_enter():
    """Wait for user to press Enter"""
    input("\nPress Enter to continue...")

def setup_interactive():
    """Interactive setup wizard"""
    clear_screen()
    
    print("╔" + "=" * 58 + "╗")
    print("║" + " " * 10 + "AdastreaDirector Setup Assistant" + " " * 16 + "║")
    print("╚" + "=" * 58 + "╝")
    
    print("\nThis wizard will help you set up the RAG system in 3 easy steps.")
    wait_for_enter()
    
    # Step 1: Dependencies
    clear_screen()
    print("╔" + "=" * 58 + "╗")
    print("║  Step 1/3: Check Dependencies" + " " * 28 + "║")
    print("╚" + "=" * 58 + "╝")
    
    print("\nChecking Python dependencies...")
    import subprocess
    result = subprocess.run([sys.executable, 'check_dependencies.py'])
    
    if result.returncode != 0:
        print("\n❌ Some dependencies are missing.")
        print("The script attempted to install them.")
        print("Please re-run this setup if installation succeeded.")
        return False
    
    wait_for_enter()
    
    # Step 2: API Key
    clear_screen()
    print("╔" + "=" * 58 + "╗")
    print("║  Step 2/3: Configure API Key" + " " * 29 + "║")
    print("╚" + "=" * 58 + "╝")
    
    from env_config import get_env_file_path, ensure_env_file_exists
    env_path = get_env_file_path()
    
    # Ensure .env exists in secure location
    ensure_env_file_exists()
    
    print(f"\n📁 Secure .env location: {env_path}")
    
    # Check if API key is configured
    has_key = False
    if env_path.exists():
        with open(env_path, 'r') as f:
            content = f.read()
            has_key = ('GEMINI_KEY=' in content and 'your_gemini_api_key_here' not in content) or \
                     ('OPENAI_API_KEY=' in content and 'your_openai_api_key_here' not in content)
    
    if has_key:
        print("\n✅ API key is already configured!")
    else:
        print("\n⚠️  You need an AI API key to use the RAG system.")
        print("\nChoose an option:")
        print("  1. Gemini API (Recommended - Free tier available)")
        print("  2. OpenAI API (Alternative)")
        print("  3. I already have a key, skip to configuration")
        
        choice = input("\nEnter your choice (1-3): ").strip()
        
        if choice == '1':
            print("\n📝 Opening Gemini API key page in your browser...")
            print("   URL: https://makersuite.google.com/app/apikey")
            webbrowser.open('https://makersuite.google.com/app/apikey')
        elif choice == '2':
            print("\n📝 Opening OpenAI API key page in your browser...")
            print("   URL: https://platform.openai.com/api-keys")
            webbrowser.open('https://platform.openai.com/api-keys')
        
        print("\n" + "─" * 60)
        print("After getting your API key:")
        print(f"  1. Open: {env_path}")
        print("  2. Find: GEMINI_KEY=your_gemini_api_key_here")
        print("  3. Replace 'your_gemini_api_key_here' with your actual key")
        print("  4. Save the file")
        print("─" * 60)
        
        print("\n💡 Tip: Use Notepad or any text editor to edit the .env file")
        
        input("\nPress Enter when you've added your API key...")
        
        # Verify key was added
        if env_path.exists():
            with open(env_path, 'r') as f:
                content = f.read()
                has_key = ('GEMINI_KEY=' in content and 'your_gemini_api_key_here' not in content) or \
                         ('OPENAI_API_KEY=' in content and 'your_openai_api_key_here' not in content)
        
        if not has_key:
            print("\n❌ API key not found in .env file.")
            print("Please make sure you saved the file correctly.")
            return False
        
        print("\n✅ API key configured successfully!")
    
    wait_for_enter()
    
    # Step 3: Ingestion
    clear_screen()
    print("╔" + "=" * 58 + "╗")
    print("║  Step 3/3: Ingest Documentation" + " " * 26 + "║")
    print("╚" + "=" * 58 + "╝")
    
    # Check if already ingested
    chromadb_path = Path(__file__).parent.parent / "Chromadb"
    
    if chromadb_path.exists():
        print("\n✅ ChromaDB already exists!")
        print("\nDo you want to re-ingest all documents? (y/n): ", end='')
        reingest = input().strip().lower()
        
        if reingest == 'y':
            print("\n🔄 Starting re-ingestion...")
            result = subprocess.run([sys.executable, 'setup_rag.py', '--reingest'])
            return result.returncode == 0
        else:
            print("\n✅ Setup complete! Your RAG system is ready to use.")
            return True
    else:
        print("\n📚 Ready to ingest project documentation.")
        print("   This will index all .md files for AI queries.")
        print("   This may take 5-10 minutes...")
        
        input("\nPress Enter to start ingestion...")
        
        print("\n🔄 Starting ingestion...")
        result = subprocess.run([sys.executable, 'setup_rag.py', '--ingest'])
        
        if result.returncode == 0:
            clear_screen()
            print("\n" + "=" * 60)
            print("🎉 Setup Complete!")
            print("=" * 60)
            print("\n✅ All systems ready!")
            print("\nNext steps:")
            print("  1. Open Unreal Engine with your project")
            print("  2. The AdastreaDirector IPC server will start automatically")
            print("  3. Test with: python ..\\..\\..\\test_ue_advanced.py")
            print("  4. Or use: Window > Developer Tools > Adastrea Director")
            print("\n" + "=" * 60)
            return True
        else:
            print("\n❌ Ingestion failed. Check the error messages above.")
            return False

if __name__ == '__main__':
    try:
        success = setup_interactive()
        sys.exit(0 if success else 1)
    except KeyboardInterrupt:
        print("\n\n❌ Setup cancelled by user.")
        sys.exit(1)
    except Exception as e:
        print(f"\n❌ Error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
