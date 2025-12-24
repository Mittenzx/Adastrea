"""
Comprehensive Unreal Engine Project Analysis
Analyzes project structure, compilation status, and identifies improvements
"""
import socket
import json
import sys
from pathlib import Path

def send_ue_request(request_type, data=None, timeout=30):
    """Send a request to Unreal Engine IPC server"""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(timeout)
        sock.connect(('127.0.0.1', 5555))
        
        request = {'type': request_type, 'data': data or {}}
        sock.sendall((json.dumps(request) + '\n').encode('utf-8'))
        
        response = sock.recv(16384).decode('utf-8')
        result = json.loads(response)
        
        sock.close()
        return result
    except Exception as e:
        return {'status': 'error', 'error': str(e)}

def analyze_project_structure():
    """Analyze project file structure"""
    print("\n" + "=" * 70)
    print("PROJECT STRUCTURE ANALYSIS")
    print("=" * 70)
    
    project_root = Path("c:/Unreal Projects/Adastrea")
    
    # Source code analysis
    source_path = project_root / "Source" / "Adastrea"
    if source_path.exists():
        cpp_files = list(source_path.rglob("*.cpp"))
        h_files = list(source_path.rglob("*.h"))
        
        print(f"\n📁 Source Code:")
        print(f"   C++ Files: {len(cpp_files)} .cpp, {len(h_files)} .h")
        print(f"   Total: {len(cpp_files) + len(h_files)} files")
        
        # Count by system
        systems = {}
        for file in cpp_files + h_files:
            parent = file.parent.name
            if parent not in ["Adastrea", "Public"]:
                systems[parent] = systems.get(parent, 0) + 1
        
        print(f"\n   Systems detected: {len(systems)}")
        for system, count in sorted(systems.items(), key=lambda x: x[1], reverse=True)[:10]:
            print(f"     - {system}: {count} files")
    
    # Content analysis
    content_path = project_root / "Content"
    if content_path.exists():
        blueprints = list(content_path.rglob("*.uasset"))
        maps = list(content_path.rglob("*.umap"))
        
        print(f"\n📦 Content:")
        print(f"   Blueprints/Assets: {len(blueprints)}")
        print(f"   Maps: {len(maps)}")
    
    # Plugin analysis
    plugins_path = project_root / "Plugins"
    if plugins_path.exists():
        plugin_files = list(plugins_path.rglob("*.uplugin"))
        print(f"\n🔌 Plugins: {len(plugin_files)}")
        for plugin in plugin_files:
            print(f"     - {plugin.stem}")

def check_compilation_errors():
    """Check for common compilation issues"""
    print("\n" + "=" * 70)
    print("COMPILATION STATUS CHECK")
    print("=" * 70)
    
    project_root = Path("c:/Unreal Projects/Adastrea")
    
    # Check for Intermediate folder (indicates successful build)
    intermediate = project_root / "Intermediate"
    if intermediate.exists():
        print("\n✅ Intermediate folder exists (project has been built)")
    else:
        print("\n⚠️  Intermediate folder missing (project may not have been built)")
    
    # Check for Binaries
    binaries = project_root / "Binaries"
    if binaries.exists():
        dll_files = list(binaries.rglob("*.dll"))
        exe_files = list(binaries.rglob("*.exe"))
        print(f"✅ Binaries folder exists: {len(dll_files)} DLLs, {len(exe_files)} EXEs")
    else:
        print("⚠️  Binaries folder missing (project needs to be built)")
    
    # Check for common error-prone files
    source_path = project_root / "Source" / "Adastrea"
    if source_path.exists():
        issues = []
        
        # Check for missing GENERATED_BODY macros
        for h_file in source_path.rglob("*.h"):
            try:
                content = h_file.read_text(encoding='utf-8')
                if "UCLASS" in content and "GENERATED_BODY" not in content:
                    issues.append(f"Missing GENERATED_BODY: {h_file.name}")
            except:
                pass
        
        if issues:
            print(f"\n⚠️  Potential issues found: {len(issues)}")
            for issue in issues[:5]:
                print(f"     - {issue}")
        else:
            print("\n✅ No obvious compilation issues detected")

def query_ue_systems():
    """Query Unreal Engine for system information"""
    print("\n" + "=" * 70)
    print("UNREAL ENGINE SYSTEM QUERY")
    print("=" * 70)
    
    # Try to get project info via RAG
    print("\n📊 Querying project systems...")
    result = send_ue_request('query', {
        'query': 'List all game systems in the Adastrea project and their current status'
    })
    
    if result.get('status') == 'success':
        answer = result.get('result', 'No result')
        if 'RAG' not in answer and 'not initialized' not in answer:
            print(f"\n{answer[:500]}...")
        else:
            print("\n⏳ RAG system still initializing (restart Unreal Editor to activate)")
    
    # Get metrics
    print("\n📈 Checking performance metrics...")
    result = send_ue_request('metrics')
    if result.get('data'):
        print(f"   Metrics available: {json.dumps(result['data'], indent=2)}")
    else:
        print("   ⏳ No metrics available yet")

def identify_improvements():
    """Identify potential improvements"""
    print("\n" + "=" * 70)
    print("IMPROVEMENT RECOMMENDATIONS")
    print("=" * 70)
    
    project_root = Path("c:/Unreal Projects/Adastrea")
    recommendations = []
    
    # Check documentation coverage
    source_path = project_root / "Source" / "Adastrea"
    if source_path.exists():
        cpp_count = len(list(source_path.rglob("*.cpp")))
        md_in_source = len(list(source_path.rglob("*.md")))
        
        if md_in_source < 5:
            recommendations.append({
                'priority': 'Medium',
                'category': 'Documentation',
                'issue': f'Only {md_in_source} README files in Source/ for {cpp_count} C++ files',
                'action': 'Add system-level README.md files to major subsystems'
            })
    
    # Check for testing infrastructure
    test_files = list(project_root.rglob("*Test*.cpp")) + list(project_root.rglob("*Test*.h"))
    if len(test_files) < 5:
        recommendations.append({
            'priority': 'High',
            'category': 'Testing',
            'issue': f'Only {len(test_files)} test files found',
            'action': 'Implement unit tests for core game systems'
        })
    
    # Check for Blueprint integration
    content_path = project_root / "Content"
    if content_path.exists():
        bp_count = len(list(content_path.rglob("*.uasset")))
        if bp_count < 20:
            recommendations.append({
                'priority': 'Medium',
                'category': 'Content',
                'issue': f'Only {bp_count} Blueprint assets found',
                'action': 'Create Blueprint wrappers for C++ game systems'
            })
    
    # Check for performance profiling
    perf_path = project_root / "Performance"
    if not perf_path.exists():
        recommendations.append({
            'priority': 'Low',
            'category': 'Performance',
            'issue': 'No dedicated performance tracking directory',
            'action': 'Set up performance profiling and benchmarking system'
        })
    
    # Output recommendations
    if recommendations:
        print(f"\n Found {len(recommendations)} improvement opportunities:\n")
        
        for i, rec in enumerate(recommendations, 1):
            print(f"{i}. [{rec['priority']}] {rec['category']}")
            print(f"   Issue: {rec['issue']}")
            print(f"   Action: {rec['action']}")
            print()
    else:
        print("\n✅ No immediate improvements identified!")

def main():
    print("=" * 70)
    print("ADASTREA PROJECT ANALYSIS")
    print("=" * 70)
    
    # Test connection first
    print("\n🔌 Testing Unreal Engine connection...")
    result = send_ue_request('ping')
    if result.get('status') == 'success':
        print("✅ Connected to Unreal Engine IPC server")
    else:
        print("⚠️  Not connected to Unreal Engine (analysis will be limited)")
        print(f"   Error: {result.get('error', 'Unknown')}")
    
    # Run analyses
    analyze_project_structure()
    check_compilation_errors()
    query_ue_systems()
    identify_improvements()
    
    print("\n" + "=" * 70)
    print("ANALYSIS COMPLETE")
    print("=" * 70)
    print("\n💡 Next Steps:")
    print("   1. Review recommendations above")
    print("   2. Restart Unreal Editor to activate RAG system")
    print("   3. Run this script again for AI-powered insights")
    print("   4. Use: Window > Developer Tools > Adastrea Director for queries")

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print("\n\nAnalysis cancelled by user.")
        sys.exit(1)
    except Exception as e:
        print(f"\n\nError during analysis: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
