#!/usr/bin/env python3
"""
Test RAG modules structure (syntax and code validation)
"""

import sys
import os
import ast

# Get the directory where this test file is located
TEST_DIR = os.path.dirname(os.path.abspath(__file__))

def test_python_syntax():
    """Test that Python files have valid syntax."""
    print("Testing Python file syntax...")
    
    files_to_test = [
        'rag_ingestion.py',
        'rag_query.py',
        'ipc_integration.py'
    ]
    
    for filename in files_to_test:
        filepath = os.path.join(TEST_DIR, filename)
        print(f"  Checking {filename}...")
        with open(filepath, 'r') as f:
            code = f.read()
            # This will raise SyntaxError if syntax is invalid
            ast.parse(code)
        print(f"    ✓ {filename} has valid syntax")

def test_class_structure():
    """Test that required classes and methods exist."""
    print("\nTesting class structure...")
    
    # Test rag_ingestion.py
    print("  Checking rag_ingestion.py...")
    with open(os.path.join(TEST_DIR, 'rag_ingestion.py'), 'r') as f:
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
    with open(os.path.join(TEST_DIR, 'rag_query.py'), 'r') as f:
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
    with open(os.path.join(TEST_DIR, 'ipc_integration.py'), 'r') as f:
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

def test_progress_writer_logic():
    """Test ProgressWriter logic (structure validation only to avoid langchain dependency)."""
    print("\nTesting ProgressWriter structure...")
    
    # Verify ProgressWriter class structure exists in rag_ingestion.py
    with open(os.path.join(TEST_DIR, 'rag_ingestion.py'), 'r') as f:
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

def main():
    """Run all tests."""
    print("RAG Modules Structure Tests")
    print("=" * 50)
    
    try:
        test_python_syntax()
        test_class_structure()
        test_progress_writer_logic()
        
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
