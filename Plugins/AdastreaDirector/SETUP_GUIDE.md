# Adastrea Director Plugin - Quick Setup Guide

**Get started with the Adastrea Director plugin in 5 minutes!**

This guide covers the essentials for installing, using, and testing the plugin. For detailed documentation, see [README.md](README.md).

---

## Prerequisites

- **Unreal Engine 5.0+** (recommended: 5.6+)
- **Python 3.9+** installed and accessible from command line
- **Visual Studio 2019/2022** (Windows) or **Xcode 13+** (Mac) or **GCC 9+/Clang 10+** (Linux)

Quick check:
```bash
python --version   # Should show 3.9 or higher
```

---

## Installation (3 Steps)

### 1. Copy Plugin to Your Project

Copy the `Plugins/AdastreaDirector` folder to your Unreal Engine project's `Plugins` directory.

Your project structure should look like:
```
<YourProject>/
├── Content/
├── Plugins/
│   └── AdastreaDirector/    ← Plugin goes here
└── <YourProject>.uproject
```

**Windows:** Use File Explorer to copy the folder  
**Mac/Linux:** Use Finder/file manager, or command line: `cp -r /path/to/Adastrea-Director/Plugins/AdastreaDirector /path/to/YourProject/Plugins/`

### 2. Regenerate Project Files

**Windows:** Right-click `<YourProject>.uproject` → "Generate Visual Studio project files"  
**Mac:** Right-click `<YourProject>.uproject` → "Generate Xcode project files"  
**Linux:** Right-click `<YourProject>.uproject` → "Generate project files" (or use UnrealBuildTool)

### 3. Build and Launch

- Open your project in Visual Studio/Xcode
- Build the solution (Ctrl+Shift+B on Windows, Cmd+B on Mac)
- Launch Unreal Engine Editor

**Verify:** Go to Edit → Plugins → search "Adastrea Director" → should show as "Enabled"

---

## Basic Usage (3 Steps)

### 1. Open the Plugin

In Unreal Engine Editor:
- **Window** → **Developer Tools** → **Adastrea Director**
- A dockable panel will appear

### 2. Configure API Key (First Time Only)

- Click the **⚙️ Settings** button (or press `Ctrl+,`)
- Choose your LLM provider (Gemini recommended or OpenAI)
- Enter your API key
- Choose embedding provider (HuggingFace is free, no key needed)
- Click **Save**

### 3. Use the Plugin

#### Query Tab (Ask Questions)
1. Click the **Query** tab
2. Type your question: "What is Unreal Engine?"
3. Press **Enter** or click **Send Query**
4. View the AI-generated response

#### Ingestion Tab (Add Documentation)
1. Click the **Ingestion** tab
2. Click **Browse** to select your documentation folder
3. Click **Start Ingestion**
4. Wait for progress bar to complete
5. Now you can ask questions about your docs in the Query tab!

---

## Quick Testing (2 Minutes)

Verify the plugin is working correctly:

### Test 1: Python Backend Connection

```bash
# From your project root directory:
cd <YourProject>/Plugins/AdastreaDirector/Python
# (Replace <YourProject> with your actual project name)
python test_ipc.py 5555
```

**Expected:** All tests pass with ✅ checkmarks

### Test 2: Query in Unreal Engine

1. Open the plugin (Window → Developer Tools → Adastrea Director)
2. Type "test" in the Query tab
3. Click Send Query
4. **Expected:** You get a response within 1-3 seconds

### Test 3: Document Ingestion (Optional)

1. Switch to Ingestion tab
2. Select a folder with some .md or .txt files
3. Click Start Ingestion
4. **Expected:** Progress bar completes without errors

**If tests fail:** See [Troubleshooting](#troubleshooting) below

---

## Troubleshooting

### Plugin Won't Load
- Regenerate project files (right-click .uproject)
- Rebuild the solution
- Check Output Log for errors

### Python Backend Won't Connect
```bash
# Install required Python packages
# The plugin uses the main Adastrea-Director repository's Python backend

# Option 1: If you have the main Adastrea-Director repository:
pip install -r /path/to/Adastrea-Director/requirements.txt
# (Replace /path/to/Adastrea-Director with your actual repository path)

# Option 2: Or install core dependencies directly:
pip install chromadb langchain langchain-community openai python-dotenv

# Test server manually from your project directory:
cd <YourProject>/Plugins/AdastreaDirector/Python
# (Replace <YourProject> with your actual project name)
python ipc_server.py --port 5555
# Should see: "Server listening on localhost:5555"
```

### Queries Return Nothing
- Make sure you configured an API key in Settings
- Verify documents were ingested (Ingestion tab)
- Check Output Log (Window → Developer Tools → Output Log)

### Still Stuck?
- Check [TESTING_QUICK_REFERENCE.md](TESTING_QUICK_REFERENCE.md) for detailed testing
- See [README.md](README.md) for comprehensive documentation
- Report issues on the main repository: [GitHub Issues](https://github.com/Mittenzx/Adastrea-Director/issues)

---

## Next Steps

**Want to learn more?**
- [README.md](README.md) - Complete feature overview
- [TESTING_QUICK_REFERENCE.md](TESTING_QUICK_REFERENCE.md) - Quick testing commands
- [TESTING_CHECKLIST_WEEKS_1_6.md](TESTING_CHECKLIST_WEEKS_1_6.md) - Comprehensive testing guide
- [RAG_INTEGRATION.md](RAG_INTEGRATION.md) - How the AI system works
- [UE_PYTHON_API.md](UE_PYTHON_API.md) - Python API integration details

**Common workflows:**
- Ingest your project documentation → Ask questions about it
- Use Query tab for development help
- Configure Settings to switch between LLM providers

---

## Keyboard Shortcuts

- `Enter` - Send query
- `Ctrl+,` - Open Settings

---

**That's it!** You're ready to use Adastrea Director. For advanced features and detailed documentation, explore the files linked above.

---

**Last Updated:** November 2025  
**Plugin Version:** 1.0.0  
**Status:** Production Ready (Weeks 1-6 Complete)
