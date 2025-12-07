"""
Check and install dependencies for AdastreaDirector
"""
import subprocess
import sys

def check_dependency(package_name, import_name=None):
    """Check if a package is installed"""
    if import_name is None:
        import_name = package_name.replace('-', '_')
    
    try:
        __import__(import_name)
        return True
    except ImportError:
        return False

def main():
    print("Checking AdastreaDirector Python dependencies...\n")
    
    dependencies = [
        ('langchain', 'langchain'),
        ('langchain-community', 'langchain_community'),
        ('langchain-text-splitters', 'langchain_text_splitters'),
        ('chromadb', 'chromadb'),
        ('python-dotenv', 'dotenv'),
        ('google-generativeai', 'google.generativeai'),
    ]
    
    missing = []
    
    for package, import_name in dependencies:
        if check_dependency(package, import_name):
            print(f"✅ {package}")
        else:
            print(f"❌ {package} - MISSING")
            missing.append(package)
    
    if missing:
        print(f"\n⚠️  Missing {len(missing)} dependencies")
        print("\nInstalling missing packages...")
        
        for package in missing:
            print(f"\nInstalling {package}...")
            try:
                subprocess.check_call([sys.executable, '-m', 'pip', 'install', package])
                print(f"✅ {package} installed successfully")
            except subprocess.CalledProcessError as e:
                print(f"❌ Failed to install {package}: {e}")
                return False
        
        print("\n✅ All dependencies installed successfully!")
        return True
    else:
        print("\n✅ All dependencies are already installed!")
        return True

if __name__ == '__main__':
    success = main()
    sys.exit(0 if success else 1)
