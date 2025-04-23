# Advanced Trading Risk Calculator

A C++ console application for calculating trading risk, position size, and profit potential with advanced features for trade simulation, session tracking, and performance analysis.

## Features

### Core Functionality
- Calculate risk amount based on account balance and risk percentage
- Determine optimal position size based on stop loss
- Calculate take-profit levels based on risk-reward ratio
- Support for different instruments (Forex, Gold, Indices)
- Support for different lot sizes (Standard, Mini, Micro)

### Advanced Features
- **Flexible Input System**: Enter SL/TP in either pips or price format
- **Multiple Take-Profit Levels**: Support for TP1 and TP2 with different reward amounts
- **Simulation Mode**: Try different trade outcomes and track account balance changes
- **Session Tracking**: Track multiple trades in a session with comprehensive statistics
- **Equity Curve**: Visualize your trading performance with ASCII charts
- **Trade History**: Save trades to CSV and JSON formats for later analysis
- **Color-Coded Output**: Intuitive visuals with green for profit, red for risk

## Build Instructions

### Prerequisites
- C++17 compatible compiler
- CMake 3.10 or higher

### Windows with Visual Studio

1. Clone or download this repository
2. Open the folder in Visual Studio that supports CMake (2017 or newer)
3. Visual Studio should detect the CMakeLists.txt and configure the project
4. Build the solution (F7 or Build → Build All)
5. Run the application (F5 or Debug → Start Debugging)

### Windows with Command Line

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

Then run the executable from the build directory:

```bash
.\Debug\TradingCalculator.exe
```

### Linux/macOS

```bash
mkdir build
cd build
cmake ..
make
```

Then run the executable:

```bash
./TradingCalculator
```

## Usage

### Main Menu
The application presents a menu-driven interface with the following options:
1. **Calculate New Trade**: Input parameters and calculate a new trade
2. **View Saved Trades**: Browse all saved trades in the current session
3. **Simulation Mode**: Simulate outcomes of trades to see P&L impact
4. **Session Statistics**: View cumulative session stats like win rate and profit factor
5. **Settings**: Configure application settings
6. **Display Equity Curve**: Visualize your trading performance
7. **Exit**: Close the application

### Calculating a Trade
1. Enter risk percentage
2. Specify entry price
3. Choose instrument type and lot size
4. Set Stop Loss in pips or as a price level
5. Set Take Profit in pips, as a price level, or as risk-reward ratio
6. Optionally use multiple take-profit targets
7. View calculated position size, risk amount, and reward potential

### Simulation Mode
After calculating a trade, you can simulate its outcome:
- Loss at Stop Loss
- Win at Take Profit (TP1 or TP2)
- Break Even

The application will update your account balance and track your session statistics.

## Configuration

The application saves settings in a config.json file, including:
- Account balance
- Default risk percentage
- Preferred instrument type
- Preferred lot size type

## Session Data

Trades are saved in CSV format for easy import into Excel/Google Sheets. Additionally, session statistics are saved in JSON format with comprehensive metrics.

## Example Output

```
===========================================
         TRADE CALCULATION RESULTS        
===========================================

Trade ID: TRD-20230615-123045-5892
Date: 2023-06-15,12:30:45

Account Balance: $10000.00
Risk: 1.00% ($100.00)
Entry Price: 1.2000
Stop Loss: 20.00 pips (1.1980)
TP1: 1.2040 (60.00%, $120.00)
TP2: 1.2060 (40.00%, $80.00)
Risk-Reward Ratio: 1:3.00
Position Size: 0.50 lots
Instrument: Forex
Lot Type: Standard

Outcome: Win at TP1
Updated Balance: $10120.00
P&L: +$120.00 (+1.20%)
```

## Project Structure

- `main.cpp`: Application entry point and UI logic
- `TradeCalculator.h/cpp`: Core calculation engine
- `Trade.h/cpp`: Trade class for encapsulating trade data and logic
- `SessionManager.h/cpp`: Manages trading sessions and statistics
- `Utils.h/cpp`: Utility functions for file operations, console formatting, etc.

## License

This project is open source and available under the MIT License. 