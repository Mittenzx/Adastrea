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
        enable_phase3_agents: bool = False,
        collection_name: str = 'adastrea_game_docs',
        persist_directory: str = './chroma_db_adastrea'
    ):
        """
        Initialize integrated IPC server.
        
        Args:
            host: Host address to bind to
            port: Port to listen on
            enable_rag: Enable RAG system integration
            enable_planning: Enable planning agent integration
            enable_phase3_agents: Enable Phase 3 autonomous agents (performance, bug detection, code quality)
            collection_name: ChromaDB collection name
            persist_directory: ChromaDB persistence directory
        """
        super().__init__(host, port)
        
        self.enable_rag = enable_rag
        self.enable_planning = enable_planning
        self.enable_phase3_agents = enable_phase3_agents
        self.query_agent = None
        self.goal_analysis_agent = None
        self.task_decomposition_agent = None
        
        # Phase 3 agents
        self.event_bus = None
        self.shared_context = None
        self.performance_agent = None
        self.bug_detection_agent = None
        self.code_quality_agent = None
        
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
        
        # Initialize Phase 3 agents if enabled
        if enable_phase3_agents:
            try:
                self._initialize_phase3_agents()
            except Exception as e:
                logger.warning(f"Failed to initialize Phase 3 agents: {e}")
                logger.warning("Phase 3 agents will not be available")
                self.enable_phase3_agents = False
        
        # Re-register handlers with integrated versions
        if self.enable_rag or self.enable_planning or self.enable_phase3_agents:
            self._register_integrated_handlers()
    
    def _initialize_rag(self, collection_name: str, persist_directory: str):
        """
        Initialize the RAG system.
        
        Args:
            collection_name: ChromaDB collection name
            persist_directory: ChromaDB persistence directory
        """
        logger.info("Initializing RAG system...")
        
        # Convert to absolute path for clarity
        persist_directory = os.path.abspath(persist_directory)
        logger.info(f"  Database path: {persist_directory}")
        
        # Import RAG query module (plugin version)
        try:
            from rag_query import RAGQueryAgent
            
            # Check if database exists
            if not os.path.exists(persist_directory):
                logger.warning(f"Database directory not found: {persist_directory}")
                logger.warning("Query functionality will be limited until database is created.")
                logger.warning("Use the 'ingest' command to populate the database.")
                return
            
            # Check if database has content
            db_file = os.path.join(persist_directory, "chroma.sqlite3")
            if not os.path.exists(db_file):
                logger.warning(f"Database not initialized: {persist_directory}")
                logger.warning("The directory exists but contains no database.")
                logger.warning("Use the 'ingest' command to populate the database.")
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
    
    def _initialize_phase3_agents(self):
        """Initialize Phase 3 autonomous agents."""
        logger.info("Initializing Phase 3 autonomous agents...")
        
        try:
            # Import Phase 3 modules
            phase3_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../..'))
            if phase3_root not in sys.path:
                sys.path.insert(0, phase3_root)
            from agents.phase3 import (
                EventBus,
                SharedContext,
                PerformanceProfilingAgent,
                BugDetectionAgent,
                CodeQualityAgent
            )
            from remote_control.client import UnrealRemoteControlClient
            
            # Initialize infrastructure
            self.event_bus = EventBus()
            self.shared_context = SharedContext()
            logger.info("  ✓ Event Bus and Shared Context created")
            
            # Try to create Remote Control client for UE integration
            ue_client = None
            try:
                ue_client = UnrealRemoteControlClient(host="localhost", port=30010)
                if ue_client.health_check():
                    logger.info("  ✓ Connected to Unreal Engine Remote Control")
                else:
                    logger.warning("  ⚠ Unreal Engine Remote Control not reachable")
                    ue_client = None
            except Exception as e:
                logger.warning(f"  ⚠ Remote Control client not available: {e}")
                ue_client = None
            
            # Initialize agents
            self.performance_agent = PerformanceProfilingAgent(
                event_bus=self.event_bus,
                shared_context=self.shared_context,
                target_fps=60.0,
                memory_threshold_mb=4096.0,
                remote_control_client=ue_client
            )
            logger.info("  ✓ Performance Profiling Agent created")
            
            self.bug_detection_agent = BugDetectionAgent(
                event_bus=self.event_bus,
                shared_context=self.shared_context,
                remote_control_client=ue_client
            )
            logger.info("  ✓ Bug Detection Agent created")
            
            self.code_quality_agent = CodeQualityAgent(
                event_bus=self.event_bus,
                shared_context=self.shared_context,
                remote_control_client=ue_client
            )
            logger.info("  ✓ Code Quality Agent created")
            
            logger.info("Phase 3 agents initialized successfully")
            
        except ImportError as e:
            logger.error(f"Failed to import Phase 3 modules: {e}")
            raise
        except Exception as e:
            logger.error(f"Unexpected error initializing Phase 3 agents: {e}")
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
        
        if self.enable_phase3_agents:
            # Agent lifecycle
            self.register_handler('agent_start', self._handle_agent_start)
            self.register_handler('agent_stop', self._handle_agent_stop)
            self.register_handler('agent_status', self._handle_agent_status)
            
            # Performance profiling
            self.register_handler('collect_metrics', self._handle_collect_metrics)
            self.register_handler('analyze_performance', self._handle_analyze_performance)
            self.register_handler('start_pie_profiling', self._handle_start_pie_profiling)
            
            # Bug detection
            self.register_handler('analyze_logs', self._handle_analyze_logs)
            self.register_handler('run_tests', self._handle_run_tests)
            self.register_handler('get_bugs', self._handle_get_bugs)
            
            # Code quality
            self.register_handler('analyze_code_quality', self._handle_analyze_code_quality)
            self.register_handler('analyze_blueprint', self._handle_analyze_blueprint)
            self.register_handler('get_technical_debt', self._handle_get_technical_debt)
    
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
    
    # ==================== Phase 3 Agent Handlers ====================
    
    def _handle_agent_start(self, data: str) -> Dict[str, Any]:
        """
        Start a Phase 3 agent.
        
        Args:
            data: JSON string with agent_id
            
        Returns:
            Status response
        """
        import json
        
        try:
            params = json.loads(data) if isinstance(data, str) else data
            agent_id = params.get('agent_id', 'all')
            
            if agent_id == 'all' or agent_id == 'performance':
                if self.performance_agent and not self.performance_agent.is_running():
                    self.performance_agent.start()
                    logger.info("Performance agent started")
            
            if agent_id == 'all' or agent_id == 'bug_detection':
                if self.bug_detection_agent and not self.bug_detection_agent.is_running():
                    self.bug_detection_agent.start()
                    logger.info("Bug detection agent started")
            
            if agent_id == 'all' or agent_id == 'code_quality':
                if self.code_quality_agent and not self.code_quality_agent.is_running():
                    self.code_quality_agent.start()
                    logger.info("Code quality agent started")
            
            return {
                'status': 'success',
                'message': f"Agent(s) {agent_id} started"
            }
            
        except Exception as e:
            logger.error(f"Agent start error: {e}")
            return {
                'status': 'error',
                'error': str(e)
            }
    
    def _handle_agent_stop(self, data: str) -> Dict[str, Any]:
        """
        Stop a Phase 3 agent.
        
        Args:
            data: JSON string with agent_id
            
        Returns:
            Status response
        """
        import json
        
        try:
            params = json.loads(data) if isinstance(data, str) else data
            agent_id = params.get('agent_id', 'all')
            
            if agent_id == 'all' or agent_id == 'performance':
                if self.performance_agent and self.performance_agent.is_running():
                    self.performance_agent.stop()
                    logger.info("Performance agent stopped")
            
            if agent_id == 'all' or agent_id == 'bug_detection':
                if self.bug_detection_agent and self.bug_detection_agent.is_running():
                    self.bug_detection_agent.stop()
                    logger.info("Bug detection agent stopped")
            
            if agent_id == 'all' or agent_id == 'code_quality':
                if self.code_quality_agent and self.code_quality_agent.is_running():
                    self.code_quality_agent.stop()
                    logger.info("Code quality agent stopped")
            
            return {
                'status': 'success',
                'message': f"Agent(s) {agent_id} stopped"
            }
            
        except Exception as e:
            logger.error(f"Agent stop error: {e}")
            return {
                'status': 'error',
                'error': str(e)
            }
    
    def _handle_agent_status(self, data: str) -> Dict[str, Any]:
        """
        Get status of Phase 3 agents.
        
        Args:
            data: Ignored
            
        Returns:
            Agent status information
        """
        try:
            status = {
                'performance': {
                    'running': self.performance_agent.is_running() if self.performance_agent else False,
                    'status': self.performance_agent.get_status().value if self.performance_agent else 'unknown',
                    'tasks_completed': self.performance_agent._metrics.tasks_completed if self.performance_agent and hasattr(self.performance_agent, '_metrics') else 0
                } if self.performance_agent else {'available': False},
                'bug_detection': {
                    'running': self.bug_detection_agent.is_running() if self.bug_detection_agent else False,
                    'status': self.bug_detection_agent.get_status().value if self.bug_detection_agent else 'unknown',
                    'tasks_completed': self.bug_detection_agent._metrics.tasks_completed if self.bug_detection_agent and hasattr(self.bug_detection_agent, '_metrics') else 0
                } if self.bug_detection_agent else {'available': False},
                'code_quality': {
                    'running': self.code_quality_agent.is_running() if self.code_quality_agent else False,
                    'status': self.code_quality_agent.get_status().value if self.code_quality_agent else 'unknown',
                    'tasks_completed': self.code_quality_agent._metrics.tasks_completed if self.code_quality_agent and hasattr(self.code_quality_agent, '_metrics') else 0
                } if self.code_quality_agent else {'available': False}
            }
            
            return {
                'status': 'success',
                'agents': status
            }
            
        except Exception as e:
            logger.error(f"Agent status error: {e}")
            return {
                'status': 'error',
                'error': str(e)
            }
    
    def _handle_collect_metrics(self, data: str) -> Dict[str, Any]:
        """
        Collect performance metrics.
        
        Args:
            data: JSON string with metrics (manual) or empty for UE collection
            
        Returns:
            Collected metrics
        """
        import json
        
        try:
            if not self.performance_agent:
                return {
                    'status': 'error',
                    'error': 'Performance agent not initialized'
                }
            
            try:
                params = json.loads(data) if data and isinstance(data, str) else {}
            except json.JSONDecodeError as e:
                logger.error(f"Invalid JSON in collect metrics data: {e}")
                return {
                    'status': 'error',
                    'error': 'Invalid JSON in metrics data',
                    'details': str(e)
                }
            
            # If no params, try to collect from UE
            if not params:
                metrics = self.performance_agent.collect_metrics_from_ue()
                if metrics:
                    return {
                        'status': 'success',
                        'metrics': {
                            'frame_rate': metrics.frame_rate,
                            'memory_usage_mb': metrics.memory_usage_mb,
                            'cpu_usage_percent': metrics.cpu_usage_percent,
                            'gpu_usage_percent': metrics.gpu_usage_percent,
                            'draw_calls': metrics.draw_calls,
                            'triangles': metrics.triangles
                        }
                    }
                else:
                    return {
                        'status': 'error',
                        'error': 'Failed to collect metrics from UE'
                    }
            else:
                # Manual metrics
                metrics = self.performance_agent.collect_metrics(
                    frame_rate=params.get('frame_rate', 0),
                    memory_usage_mb=params.get('memory_usage_mb', 0),
                    cpu_usage_percent=params.get('cpu_usage_percent', 0),
                    gpu_usage_percent=params.get('gpu_usage_percent', 0),
                    draw_calls=params.get('draw_calls', 0),
                    triangles=params.get('triangles', 0)
                )
                
                return {
                    'status': 'success',
                    'metrics': {
                        'frame_rate': metrics.frame_rate,
                        'memory_usage_mb': metrics.memory_usage_mb,
                        'cpu_usage_percent': metrics.cpu_usage_percent,
                        'gpu_usage_percent': metrics.gpu_usage_percent,
                        'draw_calls': metrics.draw_calls,
                        'triangles': metrics.triangles
                    }
                }
            
        except Exception as e:
            logger.error(f"Collect metrics error: {e}")
            return {
                'status': 'error',
                'error': str(e)
            }
    
    def _handle_analyze_performance(self, data: str) -> Dict[str, Any]:
        """
        Analyze performance metrics.

        Args:
            data: JSON string with metrics to analyze

        Returns:
            Performance analysis
        """
        try:
            if not self.performance_agent:
                return {
                    'status': 'error',
                    'error': 'Performance agent not initialized'
                }
            
            # Collect current metrics first
            metrics = self.performance_agent.collect_metrics_from_ue()
            if not metrics:
                return {
                    'status': 'error',
                    'error': 'No metrics available for analysis'
                }
            
            # Analyze
            analysis = self.performance_agent.analyze_performance(metrics)
            
            return {
                'status': 'success',
                'analysis': {
                    'summary': analysis.summary,
                    'bottlenecks': [
                        {
                            'type': b.bottleneck_type,
                            'severity': b.severity,
                            'description': b.description
                        }
                        for b in analysis.bottlenecks
                    ],
                    'recommendations': [
                        {
                            'title': r.title,
                            'description': r.description,
                            'priority': r.priority
                        }
                        for r in analysis.recommendations
                    ]
                }
            }
            
        except Exception as e:
            logger.error(f"Analyze performance error: {e}")
            return {
                'status': 'error',
                'error': str(e)
            }
    
    def _handle_start_pie_profiling(self, data: str) -> Dict[str, Any]:
        """
        Start PIE profiling session.
        
        Args:
            data: JSON string with duration_seconds
            
        Returns:
            Profiling results
        """
        import json
        
        try:
            if not self.performance_agent:
                return {
                    'status': 'error',
                    'error': 'Performance agent not initialized'
                }
            
            params = json.loads(data) if data and isinstance(data, str) else {}
            duration = params.get('duration_seconds', 60)
            
            analysis = self.performance_agent.start_pie_profiling(duration_seconds=duration)
            
            if analysis:
                return {
                    'status': 'success',
                    'analysis': {
                        'summary': analysis.summary,
                        'bottlenecks': [
                            {
                                'type': b.bottleneck_type,
                                'severity': b.severity,
                                'description': b.description
                            }
                            for b in analysis.bottlenecks
                        ],
                        'recommendations': [
                            {
                                'title': r.title,
                                'description': r.description,
                                'priority': r.priority
                            }
                            for r in analysis.recommendations
                        ]
                    }
                }
            else:
                return {
                    'status': 'error',
                    'error': 'PIE profiling failed'
                }
            
        except Exception as e:
            logger.error(f"PIE profiling error: {e}")
            return {
                'status': 'error',
                'error': str(e)
            }
    
    def _handle_analyze_logs(self, data: str) -> Dict[str, Any]:
        """
        Analyze log content for anomalies.
        
        Args:
            data: Log content or JSON with log_file path
            
        Returns:
            Detected anomalies
        """
        import json
        
        try:
            if not self.bug_detection_agent:
                return {
                    'status': 'error',
                    'error': 'Bug detection agent not initialized'
                }
            
            # Parse data
            try:
                params = json.loads(data) if isinstance(data, str) else data
                if 'log_file' in params:
                    # Read log file with path validation to prevent traversal
                    log_file_path = os.path.abspath(str(params['log_file']))
                    # Get the project root directory (3 levels up from this file)
                    allowed_base_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../..'))
                    if not log_file_path.startswith(allowed_base_dir):
                        return {
                            'status': 'error',
                            'error': 'Invalid log file path - must be within project directory'
                        }
                    with open(log_file_path, 'r') as f:
                        log_content = f.read()
                else:
                    log_content = params.get('log_content', data)
            except (json.JSONDecodeError, KeyError):
                # Treat as plain log content
                log_content = data
            
            anomalies = self.bug_detection_agent.analyze_logs(log_content)
            
            return {
                'status': 'success',
                'anomalies': [
                    {
                        'type': a.anomaly_type,
                        'severity': a.severity,
                        'description': a.description,
                        'location': a.location
                    }
                    for a in anomalies
                ]
            }
            
        except Exception as e:
            logger.error(f"Analyze logs error: {e}")
            return {
                'status': 'error',
                'error': str(e)
            }
    
    def _handle_run_tests(self, data: str) -> Dict[str, Any]:
        """
        Record automated test results.
        
        Note: This handler records test results that were executed externally.
        It does not run tests itself - it's for tracking/recording test execution
        results from external test runners (e.g., pytest, UE automation tests).
        
        Args:
            data: JSON string with test parameters including:
                  - test_suite: Name of the test suite
                  - test_count: Total number of tests
                  - passed: Number of tests that passed
                  - failed: Number of tests that failed
            
        Returns:
            Test results summary
        """
        import json
        
        try:
            if not self.bug_detection_agent:
                return {
                    'status': 'error',
                    'error': 'Bug detection agent not initialized'
                }
            
            params = json.loads(data) if data and isinstance(data, str) else {}
            test_suite = params.get('test_suite', 'default')
            
            results = self.bug_detection_agent.run_automated_tests(
                test_suite=test_suite,
                test_count=params.get('test_count', 10),
                passed=params.get('passed', 10),
                failed=params.get('failed', 0)
            )
            
            return {
                'status': 'success',
                'results': {
                    'test_run_id': results.test_run_id,
                    'total_tests': results.total_tests,
                    'passed': results.passed,
                    'failed': results.failed,
                    'success_rate': results.success_rate()
                }
            }
            
        except Exception as e:
            logger.error(f"Run tests error: {e}")
            return {
                'status': 'error',
                'error': str(e)
            }
    
    def _handle_get_bugs(self, data: str) -> Dict[str, Any]:
        """
        Get detected bugs.
        
        Args:
            data: JSON string with optional severity filter
            
        Returns:
            List of bug reports
        """
        import json
        
        try:
            if not self.bug_detection_agent:
                return {
                    'status': 'error',
                    'error': 'Bug detection agent not initialized'
                }
            
            params = json.loads(data) if data and isinstance(data, str) else {}
            severity = params.get('severity', None)
            
            bugs = self.bug_detection_agent.get_detected_bugs(severity=severity)
            
            return {
                'status': 'success',
                'bugs': [
                    {
                        'bug_id': b.bug_id,
                        'title': b.title,
                        'severity': b.severity,
                        'description': b.description
                    }
                    for b in bugs
                ]
            }
            
        except Exception as e:
            logger.error(f"Get bugs error: {e}")
            return {
                'status': 'error',
                'error': str(e)
            }
    
    def _handle_analyze_code_quality(self, data: str) -> Dict[str, Any]:
        """
        Analyze code quality.
        
        Args:
            data: JSON string with file_path and code_content
            
        Returns:
            Quality report
        """
        import json
        
        try:
            if not self.code_quality_agent:
                return {
                    'status': 'error',
                    'error': 'Code quality agent not initialized'
                }
            
            params = json.loads(data) if isinstance(data, str) else data
            file_path = params.get('file_path', 'unknown')
            
            # Read code content
            if 'code_content' in params:
                code_content = params['code_content']
            elif 'file_path' in params:
                # Validate file path to prevent traversal attacks
                code_file_path = os.path.abspath(str(params['file_path']))
                # Get the project root directory (3 levels up from this file)
                allowed_base_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../..'))
                if not code_file_path.startswith(allowed_base_dir):
                    return {
                        'status': 'error',
                        'error': 'Invalid file path - must be within project directory'
                    }
                if not os.path.exists(code_file_path):
                    return {
                        'status': 'error',
                        'error': 'File path does not exist'
                    }
                with open(code_file_path, 'r') as f:
                    code_content = f.read()
            else:
                return {
                    'status': 'error',
                    'error': 'No code_content or valid file_path provided'
                }
            
            report = self.code_quality_agent.analyze_code(file_path, code_content)
            
            return {
                'status': 'success',
                'report': {
                    'file_path': report.file_path,
                    'lines_of_code': report.lines_of_code,
                    'complexity_score': report.complexity_score,
                    'overall_score': report.overall_score,
                    'code_smells': len(report.code_smells),
                    'violations': len(report.violations),
                    'refactorings': len(report.refactorings)
                }
            }
            
        except Exception as e:
            logger.error(f"Analyze code quality error: {e}")
            return {
                'status': 'error',
                'error': str(e)
            }
    
    def _handle_analyze_blueprint(self, data: str) -> Dict[str, Any]:
        """
        Analyze Blueprint complexity.
        
        Args:
            data: JSON string with blueprint_path
            
        Returns:
            Blueprint analysis
        """
        import json
        
        try:
            if not self.code_quality_agent:
                return {
                    'status': 'error',
                    'error': 'Code quality agent not initialized'
                }
            
            params = json.loads(data) if isinstance(data, str) else data
            blueprint_path = params.get('blueprint_path', '')
            
            if not blueprint_path:
                return {
                    'status': 'error',
                    'error': 'blueprint_path is required'
                }
            
            report = self.code_quality_agent.analyze_blueprint_complexity(blueprint_path)
            
            if report:
                return {
                    'status': 'success',
                    'report': {
                        'file_path': report.file_path,
                        'complexity_score': report.complexity_score,
                        'overall_score': report.overall_score,
                        'code_smells': len(report.code_smells),
                        'violations': len(report.violations)
                    }
                }
            else:
                return {
                    'status': 'error',
                    'error': 'Blueprint analysis failed'
                }
            
        except Exception as e:
            logger.error(f"Analyze blueprint error: {e}")
            return {
                'status': 'error',
                'error': str(e)
            }
    
    def _handle_get_technical_debt(self, data: str) -> Dict[str, Any]:
        """
        Calculate technical debt.
        
        Args:
            data: Ignored
            
        Returns:
            Technical debt metrics
        """
        try:
            if not self.code_quality_agent:
                return {
                    'status': 'error',
                    'error': 'Code quality agent not initialized'
                }
            
            debt = self.code_quality_agent.calculate_technical_debt()
            
            return {
                'status': 'success',
                'debt': {
                    'total_debt_hours': debt.total_debt_hours,
                    'debt_ratio': debt.debt_ratio,
                    'code_smells_count': debt.code_smells_count,
                    'violations_count': debt.violations_count,
                    'high_priority_items': debt.high_priority_items,
                    'trend': debt.trend
                }
            }
            
        except Exception as e:
            logger.error(f"Get technical debt error: {e}")
            return {
                'status': 'error',
                'error': str(e)
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
        '--enable-phase3',
        action='store_true',
        help='Enable Phase 3 autonomous agents (performance, bug detection, code quality)'
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
        enable_phase3_agents=args.enable_phase3,
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
