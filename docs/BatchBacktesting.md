# Batch Backtesting System

The BatchBacktester is a high-performance component that allows backtesting multiple trading strategies in parallel with advanced visualization and reporting capabilities.

## Features

- **Parallel Processing**: Efficiently process multiple strategies in parallel using a configurable thread pool
- **Visual Reporting**: Generate equity curves, drawdown charts, and monthly returns heatmaps
- **Memory Management**: Process strategies in batches to control memory usage
- **Performance Monitoring**: Track detailed performance metrics for optimization
- **Multiple Export Formats**: Export results in Markdown, CSV, and JSON formats
- **Configuration System**: Highly configurable via JSON configuration file

## Usage

### Basic Usage

```cpp
#include "Backtest/BatchBacktester.h"
#include <iostream>

int main() {
    // Create backtester
    Backtest::BatchBacktester backtester;
    
    // Set backtest configuration
    Backtest::BacktestConfig config;
    config.initialBalance = 10000.0;
    config.riskPerTrade = 1.0;
    config.commission = 0.1;
    config.slippage = 0.05;
    backtester.setCommonConfig(config);
    
    // Add strategies
    backtester.addStrategyDirectory("data/strategies");
    
    // Run backtests
    Backtest::BatchBacktestResults results = backtester.runBatchBacktest();
    
    // Export reports
    backtester.exportSummaryReport("exports/summary.md");
    backtester.exportDetailedReport("exports/detailed.md");
    backtester.exportJsonReport("exports/results.json");
    backtester.exportCsvReport("exports/results.csv");
    
    return 0;
}
```

### Configuration File

You can configure the batch backtest system using a JSON configuration file:

```cpp
// Load configuration from file
backtester.loadBatchConfig("config/batch_config.json");
```

Example configuration file (`batch_config.json`):

```json
{
    "performance": {
        "thread_count": 8,
        "batch_size": 10,
        "memory_limit_mb": 2048
    },
    "output": {
        "formats": ["csv", "markdown", "json"],
        "chart_format": "png",
        "chart_width": 1200,
        "chart_height": 800,
        "chart_dpi": 100,
        "include_charts_in_report": true
    },
    "paths": {
        "strategy_dir": "data/strategies",
        "output_dir": "exports",
        "chart_dir": "exports/charts"
    },
    "logging": {
        "level": "info",
        "file": "logs/batch_backtest.log",
        "console": true,
        "performance_metrics": true
    },
    "backtest": {
        "initial_capital": 10000,
        "risk_per_trade": 1.0,
        "commission": 0.1,
        "slippage": 0.05
    }
}
```

## Configuration Options

### Performance Settings

- `thread_count`: Number of threads to use (0 = auto-detect)
- `batch_size`: Number of strategies to process in each batch
- `memory_limit_mb`: Memory limit in MB

### Output Settings

- `formats`: List of output formats (`markdown`, `csv`, `json`)
- `chart_format`: Chart image format (`png`, `jpg`, `svg`)
- `chart_width`: Chart width in pixels
- `chart_height`: Chart height in pixels
- `chart_dpi`: Chart resolution (DPI)
- `include_charts_in_report`: Whether to include charts in reports

### Path Settings

- `strategy_dir`: Directory containing strategy files
- `output_dir`: Directory for output files
- `chart_dir`: Directory for chart images

### Logging Settings

- `level`: Log level (`debug`, `info`, `warn`, `error`)
- `file`: Log file path
- `console`: Whether to output logs to console
- `performance_metrics`: Whether to track and log performance metrics

### Backtest Settings

- `initial_capital`: Initial capital for backtests
- `risk_per_trade`: Risk per trade (%)
- `commission`: Commission per trade (%)
- `slippage`: Slippage per trade (%)

## Running Tests

The BatchBacktester includes comprehensive unit and integration tests using Catch2. To run the tests:

```bash
# Build and run tests
cmake --build build --target unit_tests
cd build && ctest -V
```

## Performance Tips

1. **Thread Count**: Set `thread_count` based on your CPU. For CPU-bound tasks, set it to the number of physical cores. For I/O-bound tasks, you can set it higher.

2. **Batch Size**: Adjust `batch_size` based on your system's memory. Smaller batches use less memory but may reduce parallelism.

3. **Chart Generation**: Chart generation can be memory-intensive. If you're experiencing memory issues, consider disabling chart generation for large batches.

4. **Logging**: For production use, set the log level to `info` or `warn` to reduce I/O overhead.

5. **Memory Management**: Monitor the `peak_memory_usage` metric to optimize batch size and thread count for your specific system. 