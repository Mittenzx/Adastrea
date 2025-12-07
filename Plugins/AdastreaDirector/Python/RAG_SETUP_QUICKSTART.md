# AdastreaDirector RAG Setup - Quick Start Guide

This guide will help you set up the RAG (Retrieval-Augmented Generation) system for full AI-powered features in the AdastreaDirector plugin.

## Prerequisites

✅ Python 3.9+ installed
✅ All dependencies installed (run `python check_dependencies.py`)

## Step 1: Get an API Key

You need an AI API key. Choose one:

### Option A: Gemini (Recommended - Free Tier Available)

1. Go to: https://makersuite.google.com/app/apikey
2. Click "Get API Key" or "Create API Key"
3. Copy your key

### Option B: OpenAI (Alternative)

1. Go to: https://platform.openai.com/api-keys
2. Create an account or sign in
3. Click "Create new secret key"
4. Copy your key

## Step 2: Configure the API Key

1. Open: `c:\Unreal Projects\Adastrea\Plugins\AdastreaDirector\Python\.env`
2. Find the line: `GEMINI_KEY=your_gemini_api_key_here`
3. Replace `your_gemini_api_key_here` with your actual API key
4. Save the file

Example:
```
GEMINI_KEY=AIzaSyAbCdEfGhIjKlMnOpQrStUvWxYz1234567
```

## Step 3: Run the Setup

Open PowerShell/Terminal and run:

```powershell
cd "c:\Unreal Projects\Adastrea\Plugins\AdastreaDirector\Python"
python setup_rag.py --ingest
```

This will:
- ✅ Verify your API key
- ✅ Find all project documentation
- ✅ Process and index the documents
- ✅ Create the ChromaDB vector store

**Note:** This may take 5-10 minutes depending on documentation size.

## Step 4: Test the System

After setup completes, test it:

```powershell
cd "c:\Unreal Projects\Adastrea"
python test_ue_advanced.py
```

You should see:
```
[2] Querying project information...
Status: success
Response: [Your project information with AI-generated answers]
```

## Troubleshooting

### "No API key configured"
- Make sure you edited the `.env` file
- Check that you removed the placeholder text
- Verify there are no extra spaces

### "No documentation found"
- Make sure your project has `.md` files in `wiki/` or `Assets/` folders
- The setup script automatically searches these locations

### "Ingestion failed"
- Check your internet connection (needed for API calls)
- Verify your API key is valid
- Make sure you have write permissions to the Chromadb folder

### Re-run Ingestion

If you add new documentation:
```powershell
python setup_rag.py --reingest
```

## What's Next?

Once setup is complete:

1. **Open Unreal Engine** - The IPC server will start automatically
2. **Test Python integration** - Run `python test_ue_advanced.py`
3. **Use the plugin UI** - Window > Developer Tools > Adastrea Director
4. **Ask questions** - Query your project documentation through the AI

## Commands Reference

```powershell
# Check dependencies
python check_dependencies.py

# Check configuration
python setup_wizard.py

# Initial setup
python setup_rag.py --ingest

# Re-ingest (after adding new docs)
python setup_rag.py --reingest

# Test Unreal Engine connection
cd ..\..\..
python test_ue_advanced.py
```

## Files Created

After setup, you'll have:
- `.env` - Your API key configuration (DO NOT commit to git!)
- `../Chromadb/` - Vector database with indexed documents
- Progress logs in the Python directory

---

**Need Help?** See the full [README.md](../README.md) for detailed documentation.
