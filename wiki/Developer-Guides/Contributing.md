# Contributing to Adastrea

> **Complete Guidelines**: See [CONTRIBUTING.md](../../CONTRIBUTING.md) in the repository root for full contribution guidelines.

Thank you for your interest in contributing to Adastrea! This guide will help you get started.

## Quick Start for Contributors

### 1. Setup Your Environment
- Follow the [Installation Guide](../Getting-Started/Installation.md)
- Read the [Architecture Overview](Architecture.md)
- Review the [Code Style Guide](Code-Style.md)

### 2. Find Something to Work On
- Browse [open issues](https://github.com/Mittenzx/Adastrea/issues)
- Look for `good first issue` or `help wanted` labels
- Check the [Project Roadmap](Roadmap.md) for planned features
- Propose new features in [Discussions](https://github.com/Mittenzx/Adastrea/discussions)

### 3. Make Your Contribution
- Fork the repository
- Create a feature branch: `git checkout -b feature/your-feature`
- Make your changes following our standards
- Test thoroughly
- Submit a pull request

## Types of Contributions

### 🐛 Bug Fixes
**Always welcome!** If you find a bug:
1. Check if it's already reported
2. Create an issue with reproduction steps
3. Submit a PR with the fix
4. Include tests if possible

### ✨ New Features
**Discuss first!** For new features:
1. Open a discussion or issue to propose the feature
2. Wait for feedback from maintainers
3. Get approval before starting significant work
4. Follow the implementation plan
5. Submit PR with documentation

### 📚 Documentation
**Much appreciated!** Documentation contributions:
- Fix typos and errors
- Improve clarity and examples
- Add missing documentation
- Create tutorials and guides
- Update outdated information

### 🎨 Assets & Content
**Valuable!** Content contributions:
- Create example Data Assets
- Design ship templates
- Create faction configurations
- Build test levels
- Share YAML templates

### 🧪 Testing
**Critical!** Testing contributions:
- Write unit tests
- Create integration tests
- Perform manual testing
- Report bugs and issues
- Validate fixes

## Contribution Workflow

### 1. Fork and Clone
```bash
# Fork on GitHub, then:
git clone https://github.com/YOUR_USERNAME/Adastrea.git
cd Adastrea
git remote add upstream https://github.com/Mittenzx/Adastrea.git
```

### 2. Create Feature Branch
```bash
git checkout -b feature/your-feature-name
# Or for bug fixes:
git checkout -b fix/bug-description
```

### 3. Make Changes
- Follow [Code Style Guide](Code-Style.md)
- Write clear, focused commits
- Keep changes minimal and targeted
- Add tests for new functionality
- Update documentation

### 4. Test Locally
```bash
# Build the project
# Open in Unreal Editor
# Test your changes
# Run automated tests
python AutomationRunner.py --all
```

### 5. Commit Changes
```bash
git add .
git commit -m "feat: Add new spaceship rating system"
```

**Commit Message Format**:
- `feat:` - New feature
- `fix:` - Bug fix
- `docs:` - Documentation changes
- `refactor:` - Code refactoring
- `test:` - Adding tests
- `chore:` - Maintenance

### 6. Push and Create PR
```bash
git push origin feature/your-feature-name
# Then create PR on GitHub
```

### 7. PR Review Process
- Maintainers will review your PR
- Address feedback and requested changes
- Keep discussion professional and constructive
- Be patient - reviews may take time

## Code Standards

### C++ Code
- Follow Unreal Engine naming conventions
- Use UPROPERTY for Blueprint exposure
- Add clear comments and documentation
- Keep functions focused and small
- Initialize all variables
- Check for null pointers

### Blueprint Code
- Use descriptive names with prefixes (BP_, WBP_)
- Keep graphs clean and organized
- Use comment boxes for sections
- Add tooltips to variables
- Follow left-to-right flow

### Data Assets
- Provide default values
- Use clear, descriptive names
- Document purpose in Description field
- Test with example content
- Include YAML templates

## Documentation Standards

### Writing Style
- Clear and concise
- Beginner-friendly when appropriate
- Include examples
- Use proper markdown formatting
- Add cross-references

### File Organization
- Place system guides in `Assets/`
- Place wiki pages in `wiki/`
- Use consistent naming conventions
- Update relevant indexes
- Link related documentation

## Testing Requirements

### Before Submitting PR
- [ ] Code compiles without errors
- [ ] No new warnings introduced
- [ ] Changes work in Unreal Editor
- [ ] Blueprint integration tested
- [ ] Documentation updated
- [ ] No broken links
- [ ] Git history is clean

### Automated Tests
```bash
# Run smoke tests
python AutomationRunner.py --smoke-test

# Run screenshot tests
python AutomationRunner.py --screenshot-test

# Run all tests
python AutomationRunner.py --all
```

## What to Contribute

### High Priority
✅ Bug fixes for existing systems
✅ Documentation improvements
✅ Test coverage expansion
✅ Performance optimizations
✅ Blueprint examples

### Medium Priority
✅ New system features (discuss first)
✅ UI improvements
✅ Example content
✅ Tutorial creation
✅ Tool improvements

### Future Work
🔄 New game systems (major discussion needed)
🔄 Major architecture changes (requires approval)
🔄 Breaking changes (requires careful planning)

## Getting Help

### Resources
- 📖 [Complete Contributing Guide](../../CONTRIBUTING.md)
- 🏗️ [Architecture Guide](Architecture.md)
- 💅 [Code Style Guide](Code-Style.md)
- 📚 [Documentation Index](../Reference/Documentation-Index.md)

### Communication
- 💬 [GitHub Discussions](https://github.com/Mittenzx/Adastrea/discussions) - Questions and ideas
- 🐛 [Issue Tracker](https://github.com/Mittenzx/Adastrea/issues) - Bugs and features
- 📢 Pull Request comments - Code review feedback

## Code of Conduct

### Be Respectful
- Treat everyone with respect
- Be constructive in feedback
- Welcome newcomers
- Help others learn

### Be Professional
- Keep discussions on-topic
- Avoid inflammatory language
- Respect different opinions
- Acknowledge contributions

### Be Collaborative
- Work together toward common goals
- Share knowledge and expertise
- Help review others' contributions
- Celebrate successes

## Recognition

Contributors are recognized through:
- Git commit history
- CHANGELOG.md mentions
- GitHub contributors page
- Pull request acknowledgments

## License

By contributing, you agree that your contributions will be licensed under the MIT License. See [LICENSE](../../LICENSE) for details.

## Questions?

- **General Questions**: [GitHub Discussions](https://github.com/Mittenzx/Adastrea/discussions)
- **Bug Reports**: [Issue Tracker](https://github.com/Mittenzx/Adastrea/issues)
- **Feature Ideas**: [Discussions](https://github.com/Mittenzx/Adastrea/discussions)
- **Documentation**: [Documentation Index](../Reference/Documentation-Index.md)

---

**Thank you for contributing to Adastrea!** Every contribution helps make this project better.

*For complete guidelines, see [CONTRIBUTING.md](../../CONTRIBUTING.md)*
