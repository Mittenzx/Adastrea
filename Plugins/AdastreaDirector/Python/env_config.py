"""
Environment Configuration Utility
Provides secure path for .env file outside the repository
"""
import os
from pathlib import Path


def get_env_file_path() -> Path:
    """
    Get the secure path for the .env file.
    
    The .env file is stored in a secure location outside the repository:
    - Windows: %USERPROFILE%\.adastrea\.env
    - Linux/Mac: ~/.adastrea/.env
    
    This prevents accidentally committing API keys to the public repository.
    
    Returns:
        Path: The secure path to the .env file
    """
    # Use user's home directory for secure storage
    home_dir = Path.home()
    secure_dir = home_dir / ".adastrea"
    env_file = secure_dir / ".env"
    
    # Create directory if it doesn't exist
    secure_dir.mkdir(parents=True, exist_ok=True)
    
    return env_file


def get_env_template_path() -> Path:
    """
    Get the path to the .env.template file in the repository.
    
    Returns:
        Path: The path to the .env.template file
    """
    # Template stays in the repo as a reference
    return Path(__file__).parent / ".env.template"


def ensure_env_file_exists() -> bool:
    """
    Ensure the .env file exists in the secure location.
    If not, copy from template or create a minimal one.
    
    Returns:
        bool: True if .env file exists or was created successfully
    """
    env_path = get_env_file_path()
    
    # If it already exists, we're good
    if env_path.exists():
        return True
    
    # Try to copy from template
    template_path = get_env_template_path()
    if template_path.exists():
        import shutil
        shutil.copy(template_path, env_path)
        print(f"✅ Created .env file at: {env_path}")
        print("⚠️  Please edit this file and add your API key!")
        return True
    
    # Create minimal .env file if template doesn't exist
    minimal_content = """# Adastrea Director Configuration
# This file is stored securely outside your repository

# AI Provider Configuration
# Choose ONE:
# Option 1: Gemini (Recommended - Free tier available)
GEMINI_KEY=your_gemini_api_key_here

# Option 2: OpenAI (Alternative)
# OPENAI_API_KEY=your_openai_api_key_here

# Optional: Embedding provider (default: hf for HuggingFace)
# EMBEDDING_PROVIDER=hf
# HUGGINGFACE_MODEL_NAME=all-MiniLM-L6-v2

# Optional: ChromaDB storage location
# CHROMA_PERSIST_DIRECTORY=../Chromadb
"""
    
    env_path.write_text(minimal_content, encoding='utf-8')
    print(f"✅ Created .env file at: {env_path}")
    print("⚠️  Please edit this file and add your API key!")
    return True


def load_env_file():
    """
    Load the .env file from the secure location.
    """
    try:
        from dotenv import load_dotenv
        env_path = get_env_file_path()
        
        if not env_path.exists():
            ensure_env_file_exists()
        
        load_dotenv(dotenv_path=str(env_path))
    except ImportError:
        print("⚠️  python-dotenv not installed. Run: pip install python-dotenv")
    except Exception as e:
        print(f"⚠️  Error loading .env file: {e}")


if __name__ == "__main__":
    # For testing/debugging
    env_path = get_env_file_path()
    print(f"Secure .env location: {env_path}")
    print(f"Exists: {env_path.exists()}")
    
    if not env_path.exists():
        print("\nCreating .env file...")
        ensure_env_file_exists()
