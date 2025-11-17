# RAG System Integration for Adastrea Director Plugin

## Overview

The Adastrea Director Unreal Engine plugin now includes full RAG (Retrieval-Augmented Generation) documentation system integration, enabling context-aware Q&A and intelligent document management directly within the Unreal Editor.

## Architecture

### Components

```
┌─────────────────────────────────────────┐
│  Unreal Engine Editor (C++)            │
│  ┌───────────────────────────────────┐ │
│  │  SAdastreaDirectorPanel (Slate)   │ │
│  │  - Query Interface                │ │
│  │  - Ingestion Controls             │ │
│  │  - Progress Display               │ │
│  └──────────┬────────────────────────┘ │
│             │ IPC (JSON over Socket)   │
└─────────────┼──────────────────────────┘
              │
┌─────────────▼──────────────────────────┐
│  Python Backend (Subprocess)           │
│  ┌───────────────────────────────────┐ │
│  │  ipc_integration.py               │ │
│  │  - IntegratedIPCServer            │ │
│  │  - Request routing                │ │
│  └──────────┬────────────────────────┘ │
│             │                           │
│  ┌──────────▼───────┐  ┌─────────────┐ │
│  │ rag_ingestion.py │  │ rag_query.py│ │
│  │                  │  │             │ │
│  │ - Ingest docs    │  │ - Process   │ │
│  │ - Progress track │  │   queries   │ │
│  │ - Hash detect    │  │ - Conv hist │ │
│  └──────────────────┘  └─────────────┘ │
│             │                ▲          │
└─────────────┼────────────────┼──────────┘
              │                │
        ┌─────▼────────────────▼─────┐
        │      ChromaDB               │
        │  (Vector Database)          │
        └────────────────────────────┘
```

## Features

### Week 5: Document Ingestion ✅

#### 1. Document Ingestion Module
- **File**: `Plugins/AdastreaDirector/Python/rag_ingestion.py`
- **Class**: `RAGIngestionAgent`
- **Features**:
  - Incremental ingestion with hash-based change detection
  - Support for multiple file types: `.md`, `.txt`, `.pdf`, `.docx`, `.py`, `.cpp`, `.h`, `.cs`
  - Language-aware chunking for code files
  - Progress tracking via JSON file
  - Efficient batch processing

#### 2. Ingestion UI
- **Location**: Adastrea Director panel in Unreal Editor
- **Controls**:
  - Documentation folder path selection with browse dialog
  - Database path configuration
  - Start/Stop ingestion buttons
  - Real-time progress bar
  - Status and details text display

#### 3. Progress Tracking System
- **Mechanism**: JSON file written by Python, read by UI Tick method
- **Update Frequency**: Every frame when ingesting
- **Progress File Location**: `<Project>/Intermediate/AdastreaDirector/ingestion_progress.json`
- **Data Format**:
  ```json
  {
    "percent": 45.5,
    "label": "Processing file 23 of 50",
    "details": "Ingesting: MyDocument.md (15 chunks)",
    "status": "processing",
    "timestamp": 1699999999.999
  }
  ```

### Week 6: Query System ✅

#### 1. Query Agent Module
- **File**: `Plugins/AdastreaDirector/Python/rag_query.py`
- **Class**: `RAGQueryAgent`
- **Features**:
  - Context-aware question answering
  - Conversation history management
  - Query result caching (50 queries, FIFO)
  - Source document tracking
  - Performance metrics

#### 2. Enhanced Query UI
- **Controls**:
  - Query input with Enter key support
  - Send Query button
  - Clear History button
  - Results display with auto-wrap

#### 3. IPC Handlers
- **New Endpoints**:
  - `query`: Process RAG queries with context
  - `ingest`: Start document ingestion
  - `db_info`: Get database statistics
  - `clear_history`: Clear conversation history

## Usage Guide

### Setup

1. **Install Python Dependencies**:
   ```bash
   cd /path/to/Adastrea-Director
   pip install -r requirements.txt
   ```

2. **Configure Environment** (optional):
   ```bash
   # Use OpenAI embeddings (default is HuggingFace)
   export EMBEDDING_PROVIDER=openai
   export OPENAI_API_KEY=your-key
   
   # Or use HuggingFace (no API key required)
   export EMBEDDING_PROVIDER=hf
   export HUGGINGFACE_MODEL_NAME=all-MiniLM-L6-v2
   ```

3. **Start Unreal Engine**:
   - The plugin loads automatically
   - Python backend starts on plugin initialization
   - IPC server listens on port 5555

### Ingesting Documentation

1. **Open Adastrea Director Panel**:
   - Menu: `Window > Developer Tools > Adastrea Director`

2. **Configure Paths**:
   - **Documentation Folder**: Browse to your docs folder (e.g., project docs, UE docs)
   - **Database Path**: Choose where to store ChromaDB (default: `<Project>/chroma_db`)

3. **Start Ingestion**:
   - Click "Start Ingestion"
   - Monitor progress bar and status text
   - Ingestion runs asynchronously in Python backend

4. **Progress Updates**:
   - Real-time progress bar shows completion percentage
   - Status text shows current operation
   - Details text shows current file being processed

### Querying Documents

1. **Enter Query**:
   - Type your question in the query input box
   - Example: "How do I create a blueprint in Unreal Engine?"

2. **Send Query**:
   - Click "Send Query" or press Enter
   - Wait for Python backend to process (usually < 2 seconds)

3. **View Results**:
   - Answer appears in results display
   - Source documents listed (if available)
   - Processing time shown in logs

4. **Follow-up Questions**:
   - The system maintains conversation context
   - Ask follow-up questions naturally
   - Click "Clear History" to start fresh conversation

## Implementation Details

### Document Processing

**Supported File Types**:
- **Documentation**: Markdown (`.md`), Text (`.txt`), PDF (`.pdf`), DOCX (`.docx`)
- **Code**: Python (`.py`), C++ (`.cpp`, `.cc`, `.h`, `.hpp`), C# (`.cs`)

**Chunking Strategy**:
- Default: 1000 chars with 200 char overlap
- Code files use language-aware splitting
- Preserves code structure (functions, classes)

**Change Detection**:
- SHA-256 hash comparison
- Only re-ingests changed files
- Deletes old chunks before updating

### Query Processing

**Retrieval Strategy**:
- MMR (Maximal Marginal Relevance) by default
- Retrieves 6 most relevant chunks
- Fetches 20 candidates before reranking

**LLM Integration**:
- Uses Gemini 1.5 Flash by default (via `llm_config.py`)
- Falls back to OpenAI if configured
- Temperature: 0.7 for balanced creativity

**Caching**:
- Stores last 50 query results
- FIFO eviction policy
- Significant speedup for repeated queries

### IPC Communication

**Request Format**:
```json
{
  "type": "query|ingest|db_info|clear_history",
  "data": "query text or JSON data"
}
```

**Response Format**:
```json
{
  "status": "success|error",
  "result": "answer text",
  "sources": [...],
  "processing_time": 1.23,
  "cached": false
}
```

## Performance

### Ingestion
- **Speed**: ~1-2 files/second (with 0.5s delay between files)
- **Hash Checking**: ~100 files/second
- **Memory**: Minimal (streaming processing)

### Query
- **Latency**: 1-3 seconds for new queries
- **Cached**: < 100ms for repeated queries
- **Token Usage**: ~1000-2000 tokens per query

## Troubleshooting

### Common Issues

1. **Python Backend Not Starting**:
   - Check `<Project>/Saved/Logs/AdastreaDirector.log`
   - Ensure Python dependencies installed
   - Verify Python path in environment

2. **Ingestion Fails**:
   - Check file permissions
   - Ensure adequate disk space
   - Verify documentation folder path exists
   - Check Python console for errors

3. **Query Returns No Results**:
   - Verify database was populated (check `db_info` request)
   - Check database path configuration
   - Ensure documents were ingested successfully

4. **Progress Bar Not Updating**:
   - Check progress file exists: `<Project>/Intermediate/AdastreaDirector/ingestion_progress.json`
   - Verify UI is ticking (not paused in debugger)
   - Check file permissions on Intermediate directory

### Logs

**Unreal Engine Logs**:
- Location: `<Project>/Saved/Logs/`
- Category: `LogAdastreaDirectorEditor`

**Python Backend Logs**:
- stdout/stderr captured by UE
- Check console output in IPC server

**Progress File**:
- Location: `<Project>/Intermediate/AdastreaDirector/ingestion_progress.json`
- Contains latest status update

## Testing

### Manual Testing Checklist

#### Ingestion
- [ ] Browse and select documentation folder
- [ ] Browse and select database path
- [ ] Start ingestion
- [ ] Verify progress bar updates
- [ ] Verify status text updates
- [ ] Stop ingestion (if needed)
- [ ] Complete full ingestion
- [ ] Verify database created at specified path

#### Query
- [ ] Enter simple query
- [ ] Verify response received
- [ ] Ask follow-up question
- [ ] Verify conversation context maintained
- [ ] Clear conversation history
- [ ] Verify history cleared

### Automated Testing

**Python Module Tests**:
```bash
cd Plugins/AdastreaDirector/Python
python3 test_rag_modules.py
```

Tests:
- ✅ Python syntax validation
- ✅ Class structure verification
- ✅ ProgressWriter functionality

## Future Enhancements

### Planned Features
1. **Source Document Display**: Show relevant chunks in UI
2. **Copy to Clipboard**: Copy responses easily
3. **Advanced Filtering**: Filter by file type, date, etc.
4. **Batch Operations**: Ingest multiple folders
5. **Progress History**: View past ingestion runs
6. **Query History**: Browse past queries
7. **Export Results**: Save Q&A sessions

### Technical Improvements
1. **Async Ingestion**: Full async support in C++
2. **Cancellation**: Proper ingestion cancellation
3. **Validation**: Pre-flight checks for paths
4. **Error Recovery**: Automatic retry on failures
5. **Compression**: Compress large databases

## API Reference

### Python API

#### `RAGIngestionAgent`

```python
agent = RAGIngestionAgent(
    collection_name="adastrea_docs",
    persist_directory="./chroma_db",
    chunk_size=1000,
    chunk_overlap=200,
    progress_writer=ProgressWriter("progress.json")
)

stats = agent.ingest_directory_incremental(
    directory="/path/to/docs",
    force_reingest=False,
    delay_between_files=0.5
)
```

#### `RAGQueryAgent`

```python
agent = RAGQueryAgent(
    collection_name="adastrea_docs",
    persist_directory="./chroma_db"
)

result = agent.process_query("What is Unreal Engine?")
# result = {
#     "answer": "...",
#     "source_documents": [...],
#     "processing_time": 1.23,
#     "cached": False
# }
```

### IPC API

#### Request: Ingest
```json
{
  "type": "ingest",
  "data": "{\"docs_dir\": \"/path/to/docs\", \"persist_dir\": \"./chroma_db\", \"progress_file\": \"progress.json\", \"force_reingest\": false}"
}
```

#### Response: Ingest
```json
{
  "status": "success",
  "stats": {
    "total_files": 100,
    "added": 25,
    "updated": 10,
    "skipped": 65,
    "errors": 0
  }
}
```

#### Request: Query
```json
{
  "type": "query",
  "data": "What is Unreal Engine?"
}
```

#### Response: Query
```json
{
  "status": "success",
  "result": "Unreal Engine is...",
  "sources": [
    {
      "content": "...",
      "source": "/path/to/doc.md",
      "filename": "doc.md"
    }
  ],
  "processing_time": 1.23,
  "cached": false
}
```

## Contributing

Contributions are welcome! Please:
1. Follow existing code style
2. Add tests for new features
3. Update documentation
4. Submit PRs to main repository

## License

[To be determined]

---

**Last Updated**: 2025-11-14  
**Plugin Version**: Phase 1, Weeks 5-6  
**Status**: Feature Complete, Testing Phase
