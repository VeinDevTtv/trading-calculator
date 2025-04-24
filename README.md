# Advanced Trading Risk Calculator

<p align="center">
  <img src="docs/logo.png" alt="Trading Calculator Logo" width="200"/>
</p>

A comprehensive C++ application for traders to calculate position sizes, manage risk, analyze performance, and backtest strategies.

## Features

### Core Functionality
- Position sizing based on account balance and risk percentage
- Support for different instruments (Forex, Gold, Indices)
- Multiple lot size types (Standard, Mini, Micro, Custom)
- Price or pip-based stop loss and take profit inputs
- Multiple target calculations with partial profit taking
- Session tracking with win/loss statistics

### Advanced Risk Management
- Custom risk profiles (Conservative, Moderate, Aggressive)
- Dynamic risk allocation (Fixed, Compounding, Kelly Criterion)
- Visual risk curve generator for long-term strategy simulation

### Enhanced Trade Analytics
- Comprehensive equity curve visualization
- Advanced performance metrics:
  - Max drawdown and drawdown duration
  - Sharpe ratio
  - Profit factor
  - Longest winning/losing streaks
- Export data to CSV for external analysis

### Trade Journal System
- Attach notes to each trade (psychology, setup reasoning)
- Sentiment tags (FOMO, Revenge, Disciplined, etc.)
- Journal export capabilities

### Strategy Backtesting
- Import historical price data (OHLC format)
- Test strategies with fixed R:R setups
- Structure-based stop loss and take profit placement
- Analyze performance across market conditions

## Installation

### Prerequisites
- C++17 compatible compiler
- CMake 3.10 or higher

### Building from Source
```bash
# Clone the repository
git clone https://github.com/yourusername/trading-calculator.git
cd trading-calculator

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
make

# Run the application
./TradingCalculator
```

## Usage Examples

### Basic Position Sizing
```
Enter your starting account balance: $10000
Risk per trade (%): 1
Entry Price: 1.2500
Stop Loss (pips): 50
Take Profit (pips): 100
```

### Multiple Targets
```
Do you want to use multiple take-profit targets? (y/n): y
TP1 percentage: 60
TP1 (pips): 75
TP2 (pips): 150
```

### Risk Curve Simulation
```
Initial Balance ($): 10000
Number of Trades: 100
Win Rate (%): 55
Risk/Reward Ratio: 2
Max Risk Per Trade (%): 2
Select risk strategy: Kelly Criterion
```

![Risk Curve Example](docs/risk_curve.png)

## Project Structure

The application is organized into logical components:

- **UI/**: User interface elements
- **Workflow/**: Trade process handlers
- **Utils/**: Utility functions
- **Risk/**: Risk management profiles
- **Analytics/**: Statistical analysis
- **Journal/**: Trade journaling system
- **Backtest/**: Strategy backtesting framework
- **docs/**: Documentation

For detailed architecture information, see [Architecture Overview](docs/Architecture.md).

## Contributing

Contributions are welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgements

- Special thanks to all contributors
- Inspired by professional trading risk management practices 