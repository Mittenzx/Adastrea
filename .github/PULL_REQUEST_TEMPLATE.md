# Pull Request Checklist

Thank you for contributing to Adastrea! Please ensure you've completed the following before submitting your PR:

## Code Quality
- [ ] Code follows the project's [coding standards](../CODE_STYLE.md)
- [ ] All new code has appropriate comments and documentation
- [ ] Changes are minimal and focused on the specific issue being addressed

## Header Files and Include Paths
- [ ] All public-facing headers are placed under `Source/Adastrea/Public/` (not in subdirectories outside Public/)
- [ ] If adding headers outside Public/, `Source/Adastrea/Adastrea.Build.cs` has been updated with appropriate `PublicIncludePaths` entries
- [ ] Include paths in source files use the correct relative paths from the module root

## Build and Testing
- [ ] Right-click `.uproject` → "Generate Visual Studio project files" (or equivalent for your IDE) has been run
- [ ] `Intermediate/` and `Binaries/` directories have been cleaned (deleted) before testing
- [ ] Full rebuild completed successfully with no errors or warnings
- [ ] All existing tests pass
- [ ] New tests added for new functionality (if applicable)
- [ ] Manual testing performed and verified

## Continuous Integration
- [ ] CI pipeline passes all checks
- [ ] No new compiler warnings introduced
- [ ] Code compiles on all target platforms (Windows, Mac, Linux if applicable)

## Documentation
- [ ] README.md updated (if user-facing changes)
- [ ] Relevant documentation files updated (Assets/, Docs/, etc.)
- [ ] PR description clearly explains what changed and why

## Pre-Merge Instructions for Reviewers
After merging this PR, reviewers should:
1. Right-click the `.uproject` file → "Generate Visual Studio project files"
2. Delete `Intermediate/` and `Binaries/` directories
3. Perform a full rebuild to ensure no include path issues remain
4. Verify no C1083 or similar include-related errors occur

---

## Description
<!-- Provide a brief description of your changes -->

## Related Issues
<!-- Link to any related issues using #issue_number -->

## Screenshots (if applicable)
<!-- Add screenshots for UI changes -->

## Additional Notes
<!-- Any additional context or information -->
