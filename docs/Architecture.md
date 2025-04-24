# Trading Calculator Architecture

This document provides an overview of the Trading Calculator architecture, including its components, their interactions, and the overall design principles.

## System Overview

The Trading Calculator is a C++ application for traders to calculate position sizes, manage risk, analyze performance, and backtest strategies. It follows a modular design with clear separation of concerns.

## Core Components

### 1. User Interface (UI/)

The UI namespace contains components for user interaction:

- **Menu.h/cpp**: Handles the display of menus and user navigation
- **ColorOutput**: Console color formatting

### 2. Workflow (Workflow/)

Workflow components orchestrate business processes:

- **NewTradeWorkflow**: Manages the process of creating a new trade
- **TradeConfigurator**: Configures parameters for a trade
- **TradeDisplay**: Renders trade information
- **ViewSavedTrades**: Displays historical trades
- **SimulationHandler**: Runs trade simulations
- **StatsHandler**: Processes and displays statistics
- **SettingsHandler**: Manages application settings
- **EquityCurveRenderer**: Visualizes equity curves

### 3. Risk Management (Risk/)

Risk-related functionality:

- **RiskProfile**: Base class for different risk profiles
- **KellyRiskProfile**: Kelly criterion implementation
- **RiskCurveGenerator**: Simulates long-term risk curves

### 4. Analytics (Analytics/)

Statistical analysis components:

- **EquityStats**: Calculates performance metrics
- **EquityAnalyzer**: Analyzes trade history and performance

### 5. Journal (Journal/)

Trade journaling system:

- **TradeJournal**: Manages journal entries and sentiments
- **JournalEntry**: Data structure for trade notes and tags

### 6. Backtesting (Backtest/)

Framework for testing strategies:

- **Backtester**: Runs strategies on historical data
- **CandleData**: OHLC data structure

### 7. Models

Core domain objects:

- **Trade**: Represents a trading position
- **TradeCalculator**: Calculates trade parameters
- **SessionManager**: Manages trading sessions

### 8. Utilities (Utils/)

Support functions:

- **InputHandler**: Handles user input validation
- **FileUtils**: File operations
- **FormatUtils**: Text formatting

## Data Flow

1. User interacts with menu (UI namespace)
2. Menu calls appropriate workflow (Workflow namespace)
3. Workflow uses models and utilities to perform operations
4. Results are processed through analytics if needed
5. Output is displayed back to the user

## Dependency Diagram

```
UI → Workflow → Models ← Analytics
     ↑          ↑
     |          |
     ↓          ↓
Utils ←→ Risk ←→ Journal
           ↑
           |
           ↓
        Backtest
```

## File Organization

- `/`: Core files and entry point
- `/UI`: User interface components
- `/Workflow`: Business process handlers
- `/Utils`: Utility functions
- `/Risk`: Risk management
- `/Analytics`: Statistical analysis
- `/Journal`: Journaling system
- `/Backtest`: Backtesting framework
- `/docs`: Documentation
- `/build`: Build artifacts (not in source control)

## Extension Points

The application can be extended in several ways:

1. **New Risk Profiles**: Inherit from `RiskProfile` class
2. **Custom Analytics**: Add metrics to `EquityStats`
3. **Backtest Strategies**: Implement new entry/exit strategies in `Backtester`
4. **Journal Features**: Extend the `TradeJournal` class

## Future Considerations

- Database integration for persistent storage
- GUI implementation
- Web API for remote access
- Multi-account management
- Portfolio-level risk management

## Design Principles

The codebase follows these principles:

- **Separation of concerns**: Each module handles a specific aspect
- **SOLID principles**: Especially single responsibility and open/closed
- **Defensive programming**: Validate inputs and handle errors gracefully
- **Progressive disclosure**: Simple interface with advanced options when needed
- **Configuration over code**: Settings-driven behavior where appropriate 