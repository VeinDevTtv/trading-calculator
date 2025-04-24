# Contributing to Trading Calculator

Thank you for your interest in contributing to the Trading Calculator project! This document provides guidelines and instructions for contributing to the codebase.

## Code Structure

The project is organized into several key components:

- **UI**: User interface components and menu handlers
- **Workflow**: Trade workflows and process handlers
- **Utils**: Utility functions and helpers
- **Risk**: Risk management and profiles
- **Analytics**: Statistical analysis and equity curves
- **Journal**: Trade journaling and annotations
- **Backtest**: Backtesting framework
- **Models**: Core data models

## Getting Started

1. Fork the repository
2. Clone your fork locally
3. Set up the development environment
4. Create a new branch for your feature or bugfix

## Building the Project

Use CMake to build the project:

```bash
mkdir build
cd build
cmake ..
make
```

## Pull Request Process

1. Ensure your code builds without errors
2. Add tests for new functionality
3. Update documentation as needed
4. Create a PR with a clear description of the changes

## Coding Standards

- Use consistent indentation (4 spaces)
- Follow C++17 best practices
- Keep functions small and focused
- Use namespaces for organization
- Add comments for complex logic
- Use meaningful variable and function names

## Adding New Features

1. **Risk Profiles**: When adding new risk profiles, implement the `RiskProfile` interface
2. **Analytics**: New metrics should be added to the `EquityStats` structure
3. **Journal**: Extend the `TradeJournal` class for new journaling features
4. **Backtesting**: Custom strategies should implement pattern detection in the `Backtester` class

## Issue Templates

When creating a new issue, please use one of these templates:

### Bug Report Template
```
**Description:**
A clear description of the bug

**Steps to Reproduce:**
1. Step 1
2. Step 2
3. ...

**Expected Behavior:**
What you expected to happen

**Actual Behavior:**
What actually happened

**Environment:**
- OS: [e.g., Windows 10]
- Version: [e.g., 2.0]
```

### Feature Request Template
```
**Feature Description:**
A clear description of the feature

**Use Case:**
How this feature would be used

**Proposed Implementation:**
Ideas for how to implement (optional)

**Additional Context:**
Any other relevant information
```

## License

By contributing to this project, you agree that your contributions will be licensed under the project license.

## Questions?

Feel free to open an issue or discussion for any questions about contributing to the project. 