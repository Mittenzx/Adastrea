"""
Setup wizard for AdastreaDirector RAG system
"""
import os
import sys
from pathlib import Path

def setup_wizard():
    print("=" * 60)
    print("Adastrea Director - RAG System Setup Wizard")
    print("=" * 60)
    
    # Check if .env exists
    env_path = Path(__file__).parent / ".env"
    
    if not env_path.exists():
        print("\n❌ .env file not found!")
        print("Creating from template...")
        template_path = Path(__file__).parent / ".env.template"
        if template_path.exists():
            import shutil
            shutil.copy(template_path, env_path)
            print("✅ .env file created")
        else:
            print("❌ .env.template not found")
            return False
    
    # Read current .env
    with open(env_path, 'r') as f:
        env_content = f.read()
    
    # Check for API key
    has_gemini = 'GEMINI_KEY=' in env_content and 'your_gemini_api_key_here' not in env_content
    has_openai = 'OPENAI_API_KEY=' in env_content and 'your_openai_api_key_here' not in env_content
    
    if not has_gemini and not has_openai:
        print("\n⚠️  No API key configured!")
        print("\nYou need either:")
        print("  1. Gemini API Key (Recommended - Free tier available)")
        print("     Get it at: https://makersuite.google.com/app/apikey")
        print("\n  2. OpenAI API Key (Alternative)")
        print("     Get it at: https://platform.openai.com/api-keys")
        
        print("\n📝 To add your API key:")
        print(f"   1. Open: {env_path}")
        print("   2. Replace 'your_gemini_api_key_here' with your actual key")
        print("   3. Save the file")
        print("   4. Run this wizard again")
        
        return False
    
    print("\n✅ API key configured!")
    if has_gemini:
        print("   Using: Gemini API")
    elif has_openai:
        print("   Using: OpenAI API")
    
    # Check for documentation
    project_root = Path(__file__).parent.parent.parent.parent
    docs_paths = [
        project_root / "wiki",
        project_root / "Assets",
        project_root,
    ]
    
    print("\n📚 Checking for documentation...")
    doc_count = 0
    for docs_path in docs_paths:
        if docs_path.exists():
            md_files = list(docs_path.rglob("*.md"))
            doc_count += len(md_files)
    
    print(f"   Found {doc_count} markdown files")
    
    if doc_count == 0:
        print("\n⚠️  No documentation found to ingest!")
        print("   The RAG system needs documentation to work.")
        return False
    
    # Check if ChromaDB exists
    chromadb_path = Path(__file__).parent.parent / "Chromadb"
    
    if chromadb_path.exists():
        print(f"\n✅ ChromaDB found at: {chromadb_path}")
        print("   RAG system is already set up!")
        
        print("\n🔄 To re-ingest documents:")
        print("   python setup_rag.py --reingest")
        
    else:
        print(f"\n📥 Ready to ingest {doc_count} documents")
        print("\n▶️  Next step: Run document ingestion")
        print("   python setup_rag.py --ingest")
    
    print("\n" + "=" * 60)
    print("Setup Status Summary:")
    print("=" * 60)
    print(f"{'✅' if has_gemini or has_openai else '❌'} API Key")
    print(f"{'✅' if doc_count > 0 else '❌'} Documentation ({doc_count} files)")
    print(f"{'✅' if chromadb_path.exists() else '⏳'} ChromaDB Vector Store")
    print("=" * 60)
    
    return True

if __name__ == '__main__':
    success = setup_wizard()
    sys.exit(0 if success else 1)
