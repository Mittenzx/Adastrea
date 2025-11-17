#!/usr/bin/env python3
"""
RAG Document Ingestion Module for Plugin

This module provides document ingestion functionality for the Adastrea Director
Unreal Engine plugin. It's a streamlined version of the standalone ingest.py
designed to work within the plugin's IPC architecture.

Features:
- Incremental document ingestion
- Progress tracking via JSON file for UI updates
- Hash-based change detection
- Support for multiple document types
"""

import os
import sys
import json
import hashlib
import time
from pathlib import Path
from typing import List, Dict, Any, Optional, Tuple

# Add parent directory to path to import main modules
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../../..')))

# Disable ChromaDB telemetry
os.environ["ANONYMIZED_TELEMETRY"] = "False"

# Force UTF-8 encoding for Windows
if sys.platform == "win32":
    import io
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')
    sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding='utf-8')

try:
    from dotenv import load_dotenv
    load_dotenv()
    
    from langchain_community.document_loaders import (
        TextLoader,
        PythonLoader,
        PyPDFLoader,
        Docx2txtLoader,
    )
    from langchain_text_splitters import (
        RecursiveCharacterTextSplitter,
        Language,
    )
    from langchain_community.vectorstores import Chroma
    
    # Try to import UnstructuredMarkdownLoader
    try:
        from langchain_community.document_loaders import UnstructuredMarkdownLoader
        MARKDOWN_LOADER = UnstructuredMarkdownLoader
    except ImportError:
        MARKDOWN_LOADER = TextLoader
        
except ImportError as e:
    raise ImportError(f"Missing required dependencies: {e}")


class ProgressWriter:
    """Helper class to write progress updates to a file for UI integration."""
    
    def __init__(self, progress_file: Optional[str] = None):
        """
        Initialize the progress writer.
        
        Args:
            progress_file: Path to file where progress updates will be written (JSON format)
        """
        self.progress_file = progress_file
        self.enabled = progress_file is not None
    
    def write(self, percent: float, label: str = "", details: str = "", status: str = "processing"):
        """
        Write progress update to file.
        
        Args:
            percent: Progress percentage (0-100)
            label: Main progress label
            details: Detailed progress information
            status: Status string (processing, complete, error)
        """
        if not self.enabled:
            return
        
        try:
            progress_data = {
                'percent': min(100, max(0, percent)),
                'label': label,
                'details': details,
                'status': status,
                'timestamp': time.time()
            }
            with open(self.progress_file, 'w') as f:
                json.dump(progress_data, f)
        except Exception as e:
            print(f"[ProgressWriter] Failed to write progress: {e}", file=sys.stderr)


class RAGIngestionAgent:
    """Simplified ingestion agent for plugin integration."""
    
    def __init__(
        self,
        collection_name: str = "adastrea_docs",
        persist_directory: str = "./chroma_db",
        chunk_size: int = 1000,
        chunk_overlap: int = 200,
        embeddings: Optional[Any] = None,
        progress_writer: Optional[ProgressWriter] = None,
    ):
        """
        Initialize the ingestion agent.
        
        Args:
            collection_name: Name of the collection in the vector database
            persist_directory: Directory to persist the vector database
            chunk_size: Size of text chunks for embedding
            chunk_overlap: Overlap between chunks
            embeddings: Optional embeddings instance
            progress_writer: Optional ProgressWriter instance
        """
        self.collection_name = collection_name
        self.persist_directory = persist_directory
        self.chunk_size = chunk_size
        self.chunk_overlap = chunk_overlap
        self.progress_writer = progress_writer or ProgressWriter()
        
        # Initialize embeddings
        if embeddings is not None:
            self.embeddings = embeddings
        else:
            self._initialize_embeddings()
        
        # Initialize text splitters
        self._initialize_splitters()
    
    def _initialize_embeddings(self):
        """Initialize embeddings based on environment configuration."""
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
    
    def _initialize_splitters(self):
        """Initialize text splitters for different content types."""
        self.text_splitter = RecursiveCharacterTextSplitter(
            chunk_size=self.chunk_size,
            chunk_overlap=self.chunk_overlap,
            length_function=len,
            separators=["\n\n", "\n", " ", ""],
        )
        
        # Code-specific splitters
        self.code_splitters = {
            Language.PYTHON: RecursiveCharacterTextSplitter.from_language(
                language=Language.PYTHON,
                chunk_size=self.chunk_size,
                chunk_overlap=self.chunk_overlap,
            ),
            Language.CPP: RecursiveCharacterTextSplitter.from_language(
                language=Language.CPP,
                chunk_size=self.chunk_size,
                chunk_overlap=self.chunk_overlap,
            ),
            Language.CSHARP: RecursiveCharacterTextSplitter.from_language(
                language=Language.CSHARP,
                chunk_size=self.chunk_size,
                chunk_overlap=self.chunk_overlap,
            ),
        }
    
    def _calculate_file_hash(self, file_path: str) -> str:
        """Calculate SHA-256 hash of a file's content."""
        sha256_hash = hashlib.sha256()
        try:
            with open(file_path, "rb") as f:
                for byte_block in iter(lambda: f.read(4096), b""):
                    sha256_hash.update(byte_block)
            return sha256_hash.hexdigest()
        except Exception:
            return ""
    
    def _check_file_changed(self, file_path: str, force_reingest: bool = False) -> Tuple[bool, Optional[str], str]:
        """Check if a file has changed by comparing its hash."""
        current_hash = self._calculate_file_hash(file_path)
        
        if force_reingest or not current_hash:
            return True, None, current_hash
        
        try:
            vectorstore = Chroma(
                collection_name=self.collection_name,
                embedding_function=self.embeddings,
                persist_directory=self.persist_directory,
            )
            
            collection = vectorstore._collection
            results = collection.get(
                where={"source": file_path},
                limit=1,
                include=["metadatas"]
            )
            
            if results and results.get("metadatas") and len(results["metadatas"]) > 0:
                stored_hash = results["metadatas"][0].get("file_hash", "")
                if stored_hash == current_hash:
                    return False, stored_hash, current_hash
                return True, stored_hash, current_hash
            else:
                return True, None, current_hash
        except Exception:
            return True, None, current_hash
    
    def _delete_document_by_source(self, source: str) -> bool:
        """Delete all document chunks for a given source file."""
        try:
            vectorstore = Chroma(
                collection_name=self.collection_name,
                embedding_function=self.embeddings,
                persist_directory=self.persist_directory,
            )
            
            collection = vectorstore._collection
            results = collection.get(where={"source": source}, include=["documents"])
            
            if results and results.get("ids"):
                collection.delete(ids=results["ids"])
                return True
            return False
        except Exception:
            return False
    
    def _get_file_list(self, directory: str) -> List[str]:
        """Get a list of all supported files in a directory."""
        directory_path = Path(directory)
        supported_extensions = {
            ".md", ".txt", ".pdf", ".docx",
            ".py", ".cpp", ".cc", ".h", ".hpp", ".cs"
        }
        
        file_list = []
        for ext in supported_extensions:
            file_list.extend(directory_path.glob(f"**/*{ext}"))
        
        return [str(f) for f in file_list]
    
    def load_single_file(self, file_path: str) -> List[Any]:
        """Load a single document file."""
        file_path_obj = Path(file_path)
        
        if not file_path_obj.exists():
            return []
        
        extension = file_path_obj.suffix.lower()
        loader_mapping = {
            ".md": MARKDOWN_LOADER,
            ".txt": TextLoader,
            ".pdf": PyPDFLoader,
            ".docx": Docx2txtLoader,
            ".py": PythonLoader,
            ".cpp": TextLoader,
            ".cc": TextLoader,
            ".h": TextLoader,
            ".hpp": TextLoader,
            ".cs": TextLoader,
        }
        
        loader_class = loader_mapping.get(extension, TextLoader)
        
        try:
            loader = loader_class(file_path)
            documents = loader.load()
            return self._enrich_metadata(documents, file_path)
        except Exception as e:
            import logging
            logging.error(f"Error loading {file_path} (type: {type(e).__name__}): {e}")
            return []
    
    def _enrich_metadata(self, documents: List[Any], file_path: str) -> List[Any]:
        """Enrich document metadata."""
        file_hash = self._calculate_file_hash(file_path)
        
        for doc in documents:
            doc.metadata["file_hash"] = file_hash
            doc.metadata["filename"] = Path(file_path).name
            doc.metadata["extension"] = Path(file_path).suffix.lower()
        
        return documents
    
    def _detect_language(self, source: str) -> Optional[Language]:
        """Detect programming language based on file extension."""
        extension = Path(source).suffix.lower()
        language_map = {
            ".py": Language.PYTHON,
            ".cpp": Language.CPP,
            ".cc": Language.CPP,
            ".h": Language.CPP,
            ".hpp": Language.CPP,
            ".cs": Language.CSHARP,
        }
        return language_map.get(extension)
    
    def chunk_documents(self, documents: List[Any]) -> List[Any]:
        """Split documents into chunks."""
        if not documents:
            return []
        
        # Separate documents by language
        docs_by_language = {lang: [] for lang in self.code_splitters.keys()}
        text_docs = []
        
        for doc in documents:
            source = doc.metadata.get("source", "")
            language = self._detect_language(source)
            
            if language and language in self.code_splitters:
                docs_by_language[language].append(doc)
            else:
                text_docs.append(doc)
        
        # Chunk with appropriate splitters
        chunks = []
        for language, docs in docs_by_language.items():
            if docs:
                chunks.extend(self.code_splitters[language].split_documents(docs))
        
        if text_docs:
            chunks.extend(self.text_splitter.split_documents(text_docs))
        
        return chunks
    
    def ingest_directory_incremental(
        self,
        directory: str,
        force_reingest: bool = False,
        delay_between_files: float = 0.5
    ) -> Dict[str, Any]:
        """
        Ingest documents from a directory incrementally.
        
        Args:
            directory: Path to directory containing documents
            force_reingest: If True, re-ingest all files
            delay_between_files: Delay between processing files
            
        Returns:
            Dictionary with statistics
        """
        stats = {
            "total_files": 0,
            "skipped": 0,
            "updated": 0,
            "added": 0,
            "errors": 0,
        }
        
        directory_path = Path(directory)
        if not directory_path.exists():
            self.progress_writer.write(0, "Error", f"Directory not found: {directory}", "error")
            return stats
        
        # Get file list
        file_list = self._get_file_list(directory)
        stats["total_files"] = len(file_list)
        
        if not file_list:
            self.progress_writer.write(100, "Complete", "No supported files found", "complete")
            return stats
        
        self.progress_writer.write(0, "Starting", f"Found {len(file_list)} files", "processing")
        
        # Check if database exists once before loop
        db_exists = Path(self.persist_directory).exists()
        vectorstore = None
        
        for idx, file_path in enumerate(file_list):
            try:
                base_percent = (idx / len(file_list)) * 100
                
                # Check if file changed
                self.progress_writer.write(
                    base_percent,
                    f"Processing {idx + 1}/{len(file_list)}",
                    f"Checking: {Path(file_path).name}",
                    "processing"
                )
                
                has_changed, old_hash, current_hash = self._check_file_changed(file_path, force_reingest)
                
                if not has_changed:
                    stats["skipped"] += 1
                    continue
                
                # Load file
                self.progress_writer.write(
                    base_percent + (0.3 / len(file_list)) * 100,
                    f"Processing {idx + 1}/{len(file_list)}",
                    f"Loading: {Path(file_path).name}",
                    "processing"
                )
                documents = self.load_single_file(file_path)
                
                if not documents:
                    stats["errors"] += 1
                    continue
                
                # Chunk documents
                self.progress_writer.write(
                    base_percent + (0.6 / len(file_list)) * 100,
                    f"Processing {idx + 1}/{len(file_list)}",
                    f"Chunking: {Path(file_path).name}",
                    "processing"
                )
                chunks = self.chunk_documents(documents)
                
                if not chunks:
                    stats["errors"] += 1
                    continue
                
                # Delete old chunks if updating
                if old_hash is not None:
                    self._delete_document_by_source(file_path)
                    stats["updated"] += 1
                else:
                    stats["added"] += 1
                
                # Ingest chunks
                self.progress_writer.write(
                    base_percent + (0.9 / len(file_list)) * 100,
                    f"Processing {idx + 1}/{len(file_list)}",
                    f"Ingesting: {Path(file_path).name} ({len(chunks)} chunks)",
                    "processing"
                )
                
                if not db_exists:
                    vectorstore = Chroma.from_documents(
                        documents=chunks,
                        embedding=self.embeddings,
                        collection_name=self.collection_name,
                        persist_directory=self.persist_directory,
                    )
                    db_exists = True
                else:
                    if vectorstore is None:
                        vectorstore = Chroma(
                            collection_name=self.collection_name,
                            embedding_function=self.embeddings,
                            persist_directory=self.persist_directory,
                        )
                    vectorstore.add_documents(chunks)
                
                # Persist every 10 files for better performance
                if (idx + 1) % 10 == 0 or (idx + 1) == len(file_list):
                    vectorstore.persist()
                
                if delay_between_files > 0:
                    time.sleep(delay_between_files)
                
            except Exception as e:
                stats["errors"] += 1
                error_msg = f"Error processing {Path(file_path).name}: {str(e)}"
                import logging
                logging.error(error_msg)
                self.progress_writer.write(
                    int((idx + 1) / len(file_list) * 100),
                    "Error",
                    error_msg,
                    "error"
                )
        
        # Final progress update
        self.progress_writer.write(
            100,
            "Ingestion Complete",
            f"Processed {stats['added'] + stats['updated']} files (Added: {stats['added']}, Updated: {stats['updated']}, Skipped: {stats['skipped']}, Errors: {stats['errors']})",
            "complete"
        )
        
        return stats


def ingest_documents(
    docs_dir: str,
    collection_name: str = "adastrea_docs",
    persist_dir: str = "./chroma_db",
    progress_file: Optional[str] = None,
    force_reingest: bool = False
) -> Dict[str, Any]:
    """
    Main function to ingest documents.
    
    Args:
        docs_dir: Directory containing documents
        collection_name: ChromaDB collection name
        persist_dir: Directory to persist database
        progress_file: Path to progress file for UI updates
        force_reingest: Force re-ingestion of all files
        
    Returns:
        Dictionary with ingestion statistics
    """
    progress_writer = ProgressWriter(progress_file)
    
    agent = RAGIngestionAgent(
        collection_name=collection_name,
        persist_directory=persist_dir,
        progress_writer=progress_writer,
    )
    
    return agent.ingest_directory_incremental(
        docs_dir,
        force_reingest=force_reingest
    )
