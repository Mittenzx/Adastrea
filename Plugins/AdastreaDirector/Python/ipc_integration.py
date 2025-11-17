#!/usr/bin/env python3
"""
Integration wrapper for IPC server with RAG system.

This module provides integration between the IPC server and the main
Adastrea Director Python backend (RAG system, planning agents, etc.).

Usage:
    python ipc_integration.py --port 5555 --enable-rag
"""

import os
import sys
import argparse
import logging
from typing import Dict, Any

# Add parent directory to path to import main modules
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../../..')))

from ipc_server import IPCServer

logger = logging.getLogger('AdastreaIPCIntegration')


class IntegratedIPCServer(IPCServer):
    """IPC Server with RAG system integration."""
    
    def __init__(
        self,
        host: str = '127.0.0.1',
        port: int = 5555,
        enable_rag: bool = False,
        enable_planning: bool = False,
        collection_name: str = 'adastrea_docs',
        persist_directory: str = './chroma_db'
    ):
        """
        Initialize integrated IPC server.
        
        Args:
            host: Host address to bind to
            port: Port to listen on
            enable_rag: Enable RAG system integration
            enable_planning: Enable planning agent integration
            collection_name: ChromaDB collection name
            persist_directory: ChromaDB persistence directory
        """
        super().__init__(host, port)
        
        self.enable_rag = enable_rag
        self.enable_planning = enable_planning
        self.query_agent = None
        self.goal_analysis_agent = None
        self.task_decomposition_agent = None
        
        # Initialize RAG system if enabled
        if enable_rag:
            try:
                self._initialize_rag(collection_name, persist_directory)
            except Exception as e:
                logger.warning(f"Failed to initialize RAG system: {e}")
                logger.warning("RAG system will not be available")
                self.enable_rag = False
        
        # Initialize planning agents if enabled
        if enable_planning:
            try:
                self._initialize_planning()
            except Exception as e:
                logger.warning(f"Failed to initialize planning agents: {e}")
                logger.warning("Planning agents will not be available")
                self.enable_planning = False
        
        # Re-register handlers with integrated versions
        if self.enable_rag or self.enable_planning:
            self._register_integrated_handlers()
    
    def _initialize_rag(self, collection_name: str, persist_directory: str):
        """
        Initialize the RAG system.
        
        Args:
            collection_name: ChromaDB collection name
            persist_directory: ChromaDB persistence directory
        """
        logger.info("Initializing RAG system...")
        
        # Import RAG query module (plugin version)
        try:
            from rag_query import RAGQueryAgent
            
            # Check if database exists
            if not os.path.exists(persist_directory):
                logger.warning(f"Database directory not found: {persist_directory}")
                logger.warning("Query functionality will be limited")
                return
            
            # Initialize query agent
            self.query_agent = RAGQueryAgent(
                collection_name=collection_name,
                persist_directory=persist_directory
            )
            
            logger.info("RAG system initialized successfully")
            
        except ImportError as e:
            logger.error(f"Failed to import required modules: {e}")
            raise
        except ValueError as e:
            logger.warning(f"Database initialization: {e}")
            logger.warning("Query functionality will be limited")
        except Exception as e:
            logger.error(f"Unexpected error initializing RAG: {e}")
            raise
    
    def _initialize_planning(self):
        """Initialize planning agents."""
        logger.info("Initializing planning agents...")
        
        try:
            from goal_analysis_agent import GoalAnalysisAgent
            from task_decomposition_agent import TaskDecompositionAgent
            
            self.goal_analysis_agent = GoalAnalysisAgent()
            self.task_decomposition_agent = TaskDecompositionAgent()
            
            logger.info("Planning agents initialized successfully")
            
        except ImportError as e:
            logger.error(f"Failed to import planning modules: {e}")
            raise
        except Exception as e:
            logger.error(f"Unexpected error initializing planning: {e}")
            raise
    
    def _register_integrated_handlers(self):
        """Register integrated request handlers."""
        if self.enable_rag:
            self.register_handler('query', self._handle_query_integrated)
            self.register_handler('ingest', self._handle_ingest)
            self.register_handler('db_info', self._handle_db_info)
            self.register_handler('clear_history', self._handle_clear_history)
        
        if self.enable_planning:
            self.register_handler('analyze', self._handle_analyze_integrated)
            self.register_handler('plan', self._handle_plan_integrated)
    
    def _handle_query_integrated(self, data: str) -> Dict[str, Any]:
        """
        Handle query request with actual RAG system.
        
        Args:
            data: Query string
            
        Returns:
            Response with answer and sources
        """
        logger.info(f"Query (RAG): {data}")
        
        if not self.query_agent:
            # Fall back to placeholder response
            return self._handle_query(data)
        
        try:
            # Use actual RAG system
            response = self.query_agent.process_query(data)
            
            return {
                'status': 'success',
                'result': response.get('answer', ''),
                'sources': response.get('source_documents', []),
                'processing_time': response.get('processing_time', 0),
                'cached': response.get('cached', False)
            }
            
        except Exception as e:
            logger.error(f"RAG query error: {e}")
            return {
                'status': 'error',
                'error': f"Query failed: {str(e)}"
            }
    
    def _handle_ingest(self, data: str) -> Dict[str, Any]:
        """
        Handle document ingestion request.
        
        Args:
            data: JSON string with ingestion parameters
            
        Returns:
            Response with ingestion statistics
        """
        import json
        from rag_ingestion import ingest_documents
        
        logger.info(f"Ingest request: {data}")
        
        try:
            # Parse ingestion parameters
            params = json.loads(data) if isinstance(data, str) else data
            docs_dir = params.get('docs_dir', '')
            progress_file = params.get('progress_file', None)
            force_reingest = params.get('force_reingest', False)
            collection_name = params.get('collection_name', 'adastrea_docs')
            persist_dir = params.get('persist_dir', './chroma_db')
            
            if not docs_dir:
                return {
                    'status': 'error',
                    'error': 'docs_dir parameter is required and must be a valid directory path'
                }
            
            # Start ingestion
            stats = ingest_documents(
                docs_dir=docs_dir,
                collection_name=collection_name,
                persist_dir=persist_dir,
                progress_file=progress_file,
                force_reingest=force_reingest
            )
            
            return {
                'status': 'success',
                'stats': stats
            }
            
        except Exception as e:
            logger.error(f"Ingestion error: {e}")
            return {
                'status': 'error',
                'error': f"Ingestion failed: {str(e)}"
            }
    
    def _handle_db_info(self, data: str) -> Dict[str, Any]:
        """
        Handle database info request.
        
        Args:
            data: Ignored
            
        Returns:
            Database information
        """
        logger.info("Database info request")
        
        if not self.query_agent:
            return {
                'status': 'error',
                'error': 'RAG system not initialized'
            }
        
        try:
            info = self.query_agent.get_database_info()
            return {
                'status': 'success',
                'info': info
            }
        except Exception as e:
            logger.error(f"DB info error: {e}")
            return {
                'status': 'error',
                'error': f"Failed to get database info: {str(e)}"
            }
    
    def _handle_clear_history(self, data: str) -> Dict[str, Any]:
        """
        Handle clear conversation history request.
        
        Args:
            data: Ignored
            
        Returns:
            Success response
        """
        logger.info("Clear history request")
        
        if not self.query_agent:
            return {
                'status': 'error',
                'error': 'RAG system not initialized'
            }
        
        try:
            self.query_agent.clear_conversation_history()
            return {
                'status': 'success',
                'message': 'Conversation history cleared'
            }
        except Exception as e:
            logger.error(f"Clear history error: {e}")
            return {
                'status': 'error',
                'error': f"Failed to clear history: {str(e)}"
            }
    
    def _handle_analyze_integrated(self, data: str) -> Dict[str, Any]:
        """
        Handle analyze request with actual goal analysis agent.
        
        Args:
            data: Goal description
            
        Returns:
            Analysis result
        """
        logger.info(f"Analyze (agent): {data}")
        
        if not self.goal_analysis_agent:
            # Fall back to placeholder response
            return self._handle_analyze(data)
        
        try:
            # Use actual goal analysis agent
            analysis = self.goal_analysis_agent.analyze_goal(data)
            
            return {
                'status': 'success',
                'analysis': {
                    'goal': data,
                    'goal_type': analysis.get('goal_type', 'unknown'),
                    'complexity': analysis.get('complexity', 'medium'),
                    'scope': analysis.get('scope', []),
                    'constraints': analysis.get('constraints', []),
                    'estimated_tasks': analysis.get('estimated_tasks', 0)
                }
            }
            
        except Exception as e:
            logger.error(f"Goal analysis error: {e}")
            return {
                'status': 'error',
                'error': f"Analysis failed: {str(e)}"
            }
    
    def _handle_plan_integrated(self, data: str) -> Dict[str, Any]:
        """
        Handle plan request with actual task decomposition agent.
        
        Args:
            data: Goal description
            
        Returns:
            Task plan
        """
        logger.info(f"Plan (agent): {data}")
        
        if not self.task_decomposition_agent:
            # Fall back to placeholder response
            return self._handle_plan(data)
        
        try:
            # First analyze the goal
            if self.goal_analysis_agent:
                analysis = self.goal_analysis_agent.analyze_goal(data)
            else:
                analysis = {'goal': data}
            
            # Then decompose into tasks
            plan = self.task_decomposition_agent.decompose_goal(analysis)
            
            return {
                'status': 'success',
                'plan': {
                    'goal': data,
                    'tasks': plan.get('tasks', []),
                    'dependencies': plan.get('dependencies', []),
                    'estimated_duration': plan.get('estimated_duration', 'unknown')
                }
            }
            
        except Exception as e:
            logger.error(f"Task decomposition error: {e}")
            return {
                'status': 'error',
                'error': f"Planning failed: {str(e)}"
            }


def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description='Integrated Adastrea Director IPC Server'
    )
    parser.add_argument(
        '--port',
        type=int,
        default=5555,
        help='Port to listen on (default: 5555)'
    )
    parser.add_argument(
        '--host',
        type=str,
        default='127.0.0.1',
        help='Host to bind to (default: 127.0.0.1)'
    )
    parser.add_argument(
        '--enable-rag',
        action='store_true',
        help='Enable RAG system integration'
    )
    parser.add_argument(
        '--enable-planning',
        action='store_true',
        help='Enable planning agents integration'
    )
    parser.add_argument(
        '--collection-name',
        type=str,
        default='adastrea_docs',
        help='ChromaDB collection name'
    )
    parser.add_argument(
        '--persist-directory',
        type=str,
        default='./chroma_db',
        help='ChromaDB persistence directory'
    )
    parser.add_argument(
        '--verbose',
        action='store_true',
        help='Enable verbose logging'
    )
    
    args = parser.parse_args()
    
    if args.verbose:
        logging.basicConfig(level=logging.DEBUG)
    
    # Create integrated server
    server = IntegratedIPCServer(
        host=args.host,
        port=args.port,
        enable_rag=args.enable_rag,
        enable_planning=args.enable_planning,
        collection_name=args.collection_name,
        persist_directory=args.persist_directory
    )
    
    try:
        server.start()
    except KeyboardInterrupt:
        logger.info("Received keyboard interrupt, shutting down...")
        server.stop()
    except Exception as e:
        logger.error(f"Server error: {e}")
        sys.exit(1)


if __name__ == '__main__':
    main()
