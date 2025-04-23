# Trading Risk Calculator

A C++ console application for calculating trading risk, position size, and profit potential.

## Features

- Calculate risk amount based on account balance and risk percentage
- Determine optimal position size based on stop loss
- Calculate take-profit levels based on risk-reward ratio
- Support for different instruments (Forex, Gold, Indices)
- Support for different lot sizes (Standard, Mini, Micro)
- Save trades to CSV file for later review
- Color-coded console output for better visualization
- Optional breakeven calculations considering fees and spread

## Build Instructions

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

1. From the main menu, select "Calculate New Trade"
2. Enter your trading parameters:
   - Account balance
   - Risk percentage
   - Stop-loss in pips
   - Take-profit in pips or risk-reward ratio
   - Entry price
   - Instrument and lot size type
3. The application will calculate and display:
   - Risk amount in dollars
   - Potential reward in dollars
   - Required position size in lots
   - Stop-loss and take-profit price levels
4. Optionally save the trade for future reference

## Configuration

The application will save your account balance and default risk percentage in a config file. This will be automatically loaded next time you run the application.

## Example

```
Account Balance: $10000
Risk per trade (%): 1
Stop-Loss (pips): 20
Risk-Reward ratio: 3
Entry price: 1.2000
Instrument type: Forex

==> Risk: $100.00
==> Reward: $300.00
==> Required Lot Size: 0.50
==> SL Price Level: 1.1980
==> TP Price Level: 1.2060
``` 