# Adastrea Director - Example Content

This folder contains example content and demonstrations for the Adastrea Director plugin.

## Overview

The example content helps you understand and quickly start using the Adastrea Director plugin in your Unreal Engine projects.

## Contents

### Blueprints
- **BP_AdastreaExample**: Example Blueprint showing how to interact with the Adastrea Director system
- **BP_AIAssistantDemo**: Demonstration of querying the AI assistant from Blueprints
- **BP_AutomationExample**: Example of automated task execution via Blueprints

### Documentation Assets
- **DT_QuickReference**: Data Table with quick reference commands and examples
- **T_GettingStarted**: Texture asset with getting started guide

## Usage Examples

### 1. Query the AI Assistant from Blueprint

While the plugin provides a comprehensive UI panel, you can also interact with it programmatically via Blueprints for custom automation:

**Steps:**
1. Open `BP_AdastreaExample` in the Content Browser
2. Review the event graph showing query submission
3. Customize the query string for your needs
4. Call the events to execute queries programmatically

### 2. Automated Documentation Search

Use the example Blueprints to:
- Search documentation automatically when entering specific levels
- Display context-sensitive help based on actor types
- Integrate AI assistance into your custom editor tools

### 3. Integration with Your Project

To integrate Adastrea Director into your project:

1. **Copy Example Blueprints**: Use them as templates for your own implementations
2. **Customize Queries**: Modify the query strings to match your project's needs
3. **Add to Your UI**: Integrate the assistant panel into your custom editor layouts

## Demo Level

A demo level (`DemoLevel`) is provided to showcase the plugin's capabilities:

- **Interactive Tutorial**: Follow the tutorial markers to learn key features
- **Live Examples**: See the AI assistant in action with pre-configured examples
- **Performance Showcase**: Demonstrates the plugin's responsiveness and capabilities

To open the demo level:
1. Open Unreal Engine Editor
2. Navigate to `Content/Examples/DemoLevel`
3. Double-click to open the level
4. Press Play or use Editor mode to explore

## Best Practices

### Blueprint Integration
- Keep AI queries focused and specific for best results
- Cache frequently used queries to improve performance
- Use async patterns for non-blocking operations

### Performance Tips
- The plugin uses IPC with < 1ms latency for optimal performance
- Backend starts automatically when needed
- Status indicators show system health in real-time

### Documentation Integration
- Ingest your project documentation for context-aware assistance
- Use the Ingestion tab to add game-specific guides and wikis
- Regularly update the knowledge base as your project evolves

## Additional Resources

- **Plugin README**: `Plugins/AdastreaDirector/README.md`
- **Setup Guide**: `Plugins/AdastreaDirector/SETUP_GUIDE.md`
- **UE Python API Guide**: `Plugins/AdastreaDirector/UE_PYTHON_API.md`
- **Main Project Wiki**: https://github.com/Mittenzx/Adastrea-Director/wiki

## Support

For issues, questions, or feature requests:
- GitHub Issues: https://github.com/Mittenzx/Adastrea-Director/issues
- Documentation: https://github.com/Mittenzx/Adastrea-Director/wiki

## License

This example content is provided under the same MIT License as the main project.
See LICENSE file in the root directory for details.
