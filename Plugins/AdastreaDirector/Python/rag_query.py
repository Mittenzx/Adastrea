#!/usr/bin/env python3
"""
RAG Query Module for Plugin

This module provides query functionality for the Adastrea Director
Unreal Engine plugin. It's a streamlined version of the standalone main.py
designed to work within the plugin's IPC architecture.

Features:
- Context-aware question answering
- Conversation history management
- Source document tracking
- Performance metrics
"""

import os
import sys
import time
import copy
from typing import List, Dict, Any

# Add parent directory to path to import main modules
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../../..')))

# Disable ChromaDB telemetry
# ChromaDB checks for this variable and disables telemetry when set to "1"
os.environ["ANONYMIZED_TELEMETRY"] = "1"

# Force UTF-8 encoding for Windows
if sys.platform == "win32":
    import io
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')
    sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding='utf-8')

try:
    from dotenv import load_dotenv
    load_dotenv()
    
    from langchain_chroma import Chroma
    from langchain.chains import ConversationalRetrievalChain
    from langchain.memory import ConversationBufferMemory
    from langchain.prompts import PromptTemplate
    from llm_config import get_llm
    
except ImportError as e:
    raise ImportError(f"Missing required dependencies: {e}")


class RAGQueryAgent:
    """Simplified query agent for plugin integration."""
    
    def __init__(
        self,
        collection_name: str = "adastrea_game_docs",
        persist_directory: str = "./chroma_db_adastrea",
        model_name: str = "gpt-3.5-turbo",
        temperature: float = 0.7,
        search_type: str = "mmr",
        retrieval_k: int = 6,
        fetch_k: int = 20,
    ):
        """
        Initialize the query agent.
        
        Args:
            collection_name: Name of the collection in the vector database
            persist_directory: Directory where vector database is stored
            model_name: Name of the LLM model to use
            temperature: Temperature for response generation (0-1)
            search_type: Type of search to use ("similarity" or "mmr")
            retrieval_k: Number of documents to retrieve
            fetch_k: Number of documents to fetch before MMR reranking
        """
        self.collection_name = collection_name
        self.persist_directory = persist_directory
        self.model_name = model_name
        self.temperature = temperature
        self.search_type = search_type
        self.retrieval_k = retrieval_k
        self.fetch_k = fetch_k
        
        # Simple in-memory FIFO cache for query results
        self.query_cache: Dict[str, Dict[str, Any]] = {}
        self.cache_max_size = 50
        
        # Conversation history
        self.conversation_history: List[Dict[str, str]] = []
        
        # Initialize components
        self._initialize_components()
    
    def _initialize_components(self):
        """Initialize LLM, embeddings, vector store, and conversation chain."""
        # Initialize embeddings
        embedding_provider = os.environ.get("EMBEDDING_PROVIDER", "hf").lower()
        
        if embedding_provider == "openai":
            try:
                from langchain_openai import OpenAIEmbeddings
                self.embeddings = OpenAIEmbeddings()
            except ImportError:
                print("Error: OpenAI embeddings require 'langchain-openai' package")
                sys.exit(1)
        else:
            # Use HuggingFace embeddings (default)
            model_name = os.environ.get("HUGGINGFACE_MODEL_NAME", "all-MiniLM-L6-v2")
            try:
                from langchain_huggingface import HuggingFaceEmbeddings
            except ImportError:
                from langchain_community.embeddings import HuggingFaceEmbeddings
            
            self.embeddings = HuggingFaceEmbeddings(model_name=model_name)
        
        # Load vector store
        self.vectorstore = Chroma(
            collection_name=self.collection_name,
            embedding_function=self.embeddings,
            persist_directory=self.persist_directory,
        )
        
        # Check if database has documents
        if self.vectorstore._collection.count() == 0:
            raise ValueError(f"Database '{self.collection_name}' is empty. Please ingest documents first.")
        
        # Initialize LLM
        self.llm = get_llm(
            model_name=self.model_name,
            temperature=self.temperature,
        )
        
        # Initialize memory
        self.memory = ConversationBufferMemory(
            memory_key="chat_history",
            return_messages=True,
            output_key="answer",
        )
        
        # Create custom prompt
        prompt_template = """You are the Adastrea Director, an AI assistant specialized in helping with game development in Unreal Engine. You have access to project documentation, code, and design documents.

Use the following pieces of context to answer the question. If you don't know the answer based on the provided context, say so - don't make up information.

When answering:
- Be concise but thorough
- Reference specific documents or sections when relevant
- Provide actionable advice when appropriate
- Use technical terminology correctly
- If the question is about implementation, suggest practical approaches

Context: {context}

Question: {question}

Answer:"""
        
        PROMPT = PromptTemplate(
            template=prompt_template,
            input_variables=["context", "question"],
        )
        
        # Create conversational retrieval chain
        search_kwargs = {"k": self.retrieval_k}
        if self.search_type == "mmr":
            search_kwargs["fetch_k"] = self.fetch_k
        
        self.qa_chain = ConversationalRetrievalChain.from_llm(
            llm=self.llm,
            retriever=self.vectorstore.as_retriever(
                search_type=self.search_type,
                search_kwargs=search_kwargs
            ),
            memory=self.memory,
            return_source_documents=True,
            combine_docs_chain_kwargs={"prompt": PROMPT},
        )
    
    def _get_query_hash(self, query: str) -> str:
        """Generate a hash for query caching."""
        return str(hash(query.lower().strip()))
    
    def process_query(self, query: str, use_cache: bool = True) -> Dict[str, Any]:
        """
        Process a user query and generate a response.
        
        Args:
            query: User's question
            use_cache: Whether to use cached results
            
        Returns:
            Dictionary containing answer, sources, and metadata
        """
        try:
            start_time = time.time()
            
            # Check cache
            query_hash = self._get_query_hash(query)
            if use_cache and query_hash in self.query_cache:
                cached_result = copy.deepcopy(self.query_cache[query_hash])
                cached_result["cached"] = True
                cached_result["processing_time"] = time.time() - start_time
                return cached_result
            
            # Process query
            result = self.qa_chain({"question": query})
            processing_time = time.time() - start_time
            
            # Format response
            response = {
                "answer": result.get("answer", ""),
                "source_documents": self._format_sources(result.get("source_documents", [])),
                "processing_time": processing_time,
                "cached": False,
            }
            
            # Add to conversation history
            self.conversation_history.append({
                "query": query,
                "answer": response["answer"],
                "timestamp": time.time(),
            })
            
            # Cache result
            if use_cache:
                if len(self.query_cache) >= self.cache_max_size:
                    self.query_cache.pop(next(iter(self.query_cache)))
                self.query_cache[query_hash] = copy.deepcopy(response)
            
            return response
            
        except Exception as e:
            return {
                "answer": f"Error processing query: {str(e)}",
                "source_documents": [],
                "processing_time": 0,
                "cached": False,
                "error": str(e),
            }
    
    def _format_sources(self, source_documents: List[Any]) -> List[Dict[str, str]]:
        """Format source documents for display."""
        sources = []
        for doc in source_documents:
            sources.append({
                "content": doc.page_content[:200] + "..." if len(doc.page_content) > 200 else doc.page_content,
                "source": doc.metadata.get("source", "Unknown"),
                "filename": doc.metadata.get("filename", "Unknown"),
            })
        return sources
    
    def get_conversation_history(self) -> List[Dict[str, str]]:
        """Get conversation history."""
        return self.conversation_history
    
    def clear_conversation_history(self):
        """Clear conversation history."""
        self.conversation_history.clear()
        self.memory.clear()
    
    def get_database_info(self) -> Dict[str, Any]:
        """Get information about the loaded database."""
        try:
            count = self.vectorstore._collection.count()
            return {
                "collection_name": self.collection_name,
                "document_count": count,
                "persist_directory": self.persist_directory,
            }
        except Exception as e:
            return {
                "error": str(e)
            }


def query_documents(
    query: str,
    collection_name: str = "adastrea_docs",
    persist_dir: str = "./chroma_db",
) -> Dict[str, Any]:
    """
    Query the document database.
    
    Args:
        query: User's question
        collection_name: ChromaDB collection name
        persist_dir: Directory where database is stored
        
    Returns:
        Dictionary with answer and metadata
    """
    try:
        agent = RAGQueryAgent(
            collection_name=collection_name,
            persist_directory=persist_dir,
        )
        return agent.process_query(query)
    except Exception as e:
        return {
            "answer": f"Error: {str(e)}",
            "source_documents": [],
            "processing_time": 0,
            "cached": False,
            "error": str(e),
        }
