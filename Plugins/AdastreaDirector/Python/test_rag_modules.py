#!/usr/bin/env python3
"""
Test RAG modules structure (syntax and code validation)
"""

import sys
import os
import ast
import json
import tempfile

def test_python_syntax():
    """Test that Python files have valid syntax."""
    print("Testing Python file syntax...")
    
    files_to_test = [
        'rag_ingestion.py',
        'rag_query.py',
        'ipc_integration.py'
    ]
    
    for filename in files_to_test:
        print(f"  Checking {filename}...")
        try:
            with open(filename, 'r') as f:
                code = f.read()
                ast.parse(code)
            print(f"    ✓ {filename} has valid syntax")
        except SyntaxError as e:
            print(f"    ✗ {filename} has syntax error: {e}")
            return False
    
    return True

def test_class_structure():
    """Test that required classes and methods exist."""
    print("\nTesting class structure...")
    
    # Test rag_ingestion.py
    print("  Checking rag_ingestion.py...")
    with open('rag_ingestion.py', 'r') as f:
        content = f.read()
        required_items = [
            'class ProgressWriter',
            'class RAGIngestionAgent',
            'def ingest_directory_incremental',
            'def _check_file_changed',
            'def chunk_documents',
        ]
        for item in required_items:
            assert item in content, f"Missing: {item}"
        print("    ✓ RAGIngestionAgent class structure verified")
    
    # Test rag_query.py
    print("  Checking rag_query.py...")
    with open('rag_query.py', 'r') as f:
        content = f.read()
        required_items = [
            'class RAGQueryAgent',
            'def process_query',
            'def get_conversation_history',
            'def clear_conversation_history',
            'def get_database_info',
        ]
        for item in required_items:
            assert item in content, f"Missing: {item}"
        print("    ✓ RAGQueryAgent class structure verified")
    
    # Test ipc_integration.py
    print("  Checking ipc_integration.py...")
    with open('ipc_integration.py', 'r') as f:
        content = f.read()
        required_items = [
            'class IntegratedIPCServer',
            '_handle_query_integrated',
            '_handle_ingest',
            '_handle_db_info',
            '_handle_clear_history',
        ]
        for item in required_items:
            assert item in content, f"Missing: {item}"
        print("    ✓ IntegratedIPCServer structure verified")
    
    return True

def test_progress_writer_logic():
    """Test ProgressWriter logic (structure validation only to avoid langchain dependency)."""
    print("\nTesting ProgressWriter structure...")
    
    # Verify ProgressWriter class structure exists in rag_ingestion.py
    with open('rag_ingestion.py', 'r') as f:
        content = f.read()
        required_items = [
            'class ProgressWriter',
            'def __init__',
            'def write',
            'self.progress_file',
            'self.enabled',
        ]
        for item in required_items:
            assert item in content, f"Missing: {item}"
    
    print("    ✓ ProgressWriter structure verified")
    
    # Note: Full functional testing requires langchain dependencies.
    # The ProgressWriter writes JSON progress updates with format:
    # {'percent': 0-100, 'label': str, 'details': str, 'status': str, 'timestamp': float}
    
    return True

def main():
    """Run all tests."""
    print("RAG Modules Structure Tests")
    print("=" * 50)
    
    try:
        if not test_python_syntax():
            return False
        
        if not test_class_structure():
            return False
        
        if not test_progress_writer_logic():
            return False
        
        print("\n" + "=" * 50)
        print("✓ All structure tests passed!")
        print("\nNote: Full runtime testing requires dependencies:")
        print("  pip install -r requirements.txt")
        return True
        
    except Exception as e:
        print(f"\n✗ Test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

if __name__ == '__main__':
    success = main()
    sys.exit(0 if success else 1)
