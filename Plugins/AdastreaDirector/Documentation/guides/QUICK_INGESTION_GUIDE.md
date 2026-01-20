# Quick Reference: Game Repository Ingestion for Plugin Testing

## TL;DR - Get Started in 60 Seconds

```bash
# 1. Install dependencies (if not already done)
pip install -r requirements.txt

# 2. Run the quick ingestion script
./quick_ingest_game.sh

# 3. Configure plugin to use:
#    - Database: ./chroma_db_adastrea
#    - Collection: adastrea_game_docs
```

That's it! Your plugin can now query game documentation.

## What This Does

1. **Clones** the Mittenzx/Adastrea game repository (public, no token needed)
2. **Processes** documentation from Docs/, Source/, and Content/ directories
3. **Creates** a ChromaDB vector database at `./chroma_db_adastrea`
4. **Ready** for plugin testing with game-specific queries

## Common Issues

### "Cannot reach huggingface.co"

**First-time only**: The system needs to download an embedding model (~90MB) from HuggingFace.

**Solutions:**
1. Run on a machine with internet access
2. Use OpenAI embeddings instead:
   ```bash
   export EMBEDDING_PROVIDER=openai
   export OPENAI_API_KEY=your-key
   ./quick_ingest_game.sh
   ```

### "Plugin can't find database"

**Verify paths match:**
```bash
# Check database exists
ls -la ./chroma_db_adastrea

# In plugin, use these exact values:
# Database path: ./chroma_db_adastrea
# Collection: adastrea_game_docs
```

## Testing the Plugin

Once ingested, test with these queries in the plugin:

1. **Basic**: "What is the Adastrea game about?"
2. **Code**: "How do spaceship controls work?"
3. **Follow-up**: "Tell me more about that feature"

Expected: Relevant answers from game documentation

## More Information

- **Full Guide**: [GAME_REPO_INGESTION_GUIDE.md](../../GAME_REPO_INGESTION_GUIDE.md)
- **Plugin RAG Integration**: [RAG_INTEGRATION.md](RAG_INTEGRATION.md)
- **Troubleshooting**: See the full guide above

## Statistics

View ingestion stats anytime:
```bash
python3 ingest_game_repo.py --stats
```

## Re-ingestion

Only needed if game docs change:
```bash
python3 ingest_game_repo.py --force
```

---

**Last Updated**: 2025-12-20  
**For**: Adastrea Director Plugin Testing
