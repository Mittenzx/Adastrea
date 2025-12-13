"""
Migration Script: Move .env file to secure location outside repository

This script migrates your existing .env file from the repository to a secure
location outside the repo, preventing accidental commits of API keys.

New location:
- Windows: %USERPROFILE%\.adastrea\.env
- Linux/Mac: ~/.adastrea/.env
"""

import os
import sys
from pathlib import Path
import shutil


def migrate_env_file():
    """Migrate .env file from repo to secure location."""
    
    print("=" * 70)
    print("AdastreaDirector - Environment File Migration")
    print("=" * 70)
    
    # Import after the banner
    from env_config import get_env_file_path, get_env_template_path
    
    # Old location (in repo)
    old_env_path = Path(__file__).parent / ".env"
    
    # New location (secure, outside repo)
    new_env_path = get_env_file_path()
    
    print(f"\nOld location: {old_env_path}")
    print(f"New location: {new_env_path}")
    print()
    
    # Check if old .env exists
    if not old_env_path.exists():
        print("ℹ️  No .env file found in repository.")
        
        # Check if already migrated
        if new_env_path.exists():
            print("✅ .env file already exists in secure location.")
            print("\n✨ Migration complete! Your API keys are safe.")
            return True
        else:
            print("⚠️  Creating new .env file in secure location...")
            from env_config import ensure_env_file_exists
            if ensure_env_file_exists():
                print("\n✅ New .env file created.")
                print(f"📝 Please edit: {new_env_path}")
                print("   And add your API key.")
                return True
            else:
                print("❌ Failed to create .env file.")
                return False
    
    # Read the old file
    print("📖 Reading existing .env file...")
    try:
        with open(old_env_path, 'r', encoding='utf-8') as f:
            env_content = f.read()
    except Exception as e:
        print(f"❌ Error reading old .env file: {e}")
        return False
    
    # Check if it has actual configuration (not just template)
    has_real_key = False
    if 'GEMINI_KEY=' in env_content or 'OPENAI_API_KEY=' in env_content:
        # Check if it's not just the template placeholder
        if 'your_gemini_api_key_here' not in env_content and \
           'your_openai_api_key_here' not in env_content:
            has_real_key = True
    
    # Create secure directory if needed
    new_env_path.parent.mkdir(parents=True, exist_ok=True)
    
    # If new location already exists, ask about overwrite
    if new_env_path.exists():
        print(f"\n⚠️  File already exists at: {new_env_path}")
        
        if has_real_key:
            response = input("Overwrite existing file? (y/N): ").strip().lower()
            if response != 'y':
                print("❌ Migration cancelled.")
                print(f"\n💡 Manual option: Edit {new_env_path} with your settings")
                return False
        else:
            print("ℹ️  Old file appears to be a template. Keeping existing secure file.")
            
            # Delete the template from repo
            try:
                old_env_path.unlink()
                print(f"✅ Removed template file from repository: {old_env_path}")
            except Exception as e:
                print(f"⚠️  Could not remove old file: {e}")
                print("   You can safely delete it manually.")
            
            print("\n✨ Migration complete! Using existing secure configuration.")
            return True
    
    # Copy file to new location
    print(f"\n📋 Copying .env to secure location...")
    try:
        shutil.copy2(old_env_path, new_env_path)
        print(f"✅ Copied to: {new_env_path}")
    except Exception as e:
        print(f"❌ Error copying file: {e}")
        return False
    
    # Verify the copy
    if not new_env_path.exists():
        print("❌ Copy verification failed!")
        return False
    
    # Backup and delete old file
    print(f"\n🗑️  Removing .env from repository...")
    try:
        # Create a backup just in case
        backup_path = old_env_path.with_suffix('.env.backup')
        shutil.copy2(old_env_path, backup_path)
        print(f"📦 Created backup: {backup_path}")
        
        # Delete the old file
        old_env_path.unlink()
        print(f"✅ Removed from repository: {old_env_path}")
        print(f"\n⚠️  Remember to delete the backup after verifying: {backup_path}")
        
    except Exception as e:
        print(f"⚠️  Could not remove old file: {e}")
        print("   You can manually delete it - it's safe to remove now.")
    
    # Success!
    print("\n" + "=" * 70)
    print("✨ Migration Complete!")
    print("=" * 70)
    print(f"\n✅ Your .env file is now stored securely at:")
    print(f"   {new_env_path}")
    print("\n✅ This location is outside your repository.")
    print("✅ Your API keys will not be committed to git.")
    
    if has_real_key:
        print("\n✅ Your API key has been migrated successfully.")
    else:
        print("\n⚠️  Don't forget to add your API key to the new location!")
        print(f"   Edit: {new_env_path}")
    
    print("\n💡 All Python scripts now use the secure location automatically.")
    
    return True


if __name__ == "__main__":
    try:
        success = migrate_env_file()
        sys.exit(0 if success else 1)
    except KeyboardInterrupt:
        print("\n\n❌ Migration cancelled by user.")
        sys.exit(1)
    except Exception as e:
        print(f"\n❌ Unexpected error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
