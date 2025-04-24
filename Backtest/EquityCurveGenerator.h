#pragma once

#include "Backtester.h"
#include <string>
#include <vector>
#include <filesystem>

namespace Backtest {
    /**
     * @brief Configuration for equity curve generation
     */
    struct ChartConfig {
        std::string format = "png";  // Image format (png, jpg, svg)
        int width = 1200;            // Image width in pixels
        int height = 800;            // Image height in pixels
        int dpi = 100;               // Image DPI
        bool darkMode = false;       // Dark mode theme
        std::string outputDir = "exports/charts"; // Output directory for charts
    };
    
    /**
     * @brief Utility class for generating equity curve charts
     */
    class EquityCurveGenerator {
    public:
        /**
         * @brief Constructor with configuration
         * @param config Chart configuration
         */
        explicit EquityCurveGenerator(const ChartConfig& config = ChartConfig{});
        
        /**
         * @brief Generate an equity curve image from backtest results
         * @param strategyName Name of the strategy
         * @param result Backtest result containing equity curve data
         * @return Path to the generated image file
         */
        std::string generateEquityCurve(const std::string& strategyName, 
                                       const BacktestResult& result);
        
        /**
         * @brief Generate a drawdown chart from backtest results
         * @param strategyName Name of the strategy
         * @param result Backtest result containing drawdown data
         * @return Path to the generated image file
         */
        std::string generateDrawdownChart(const std::string& strategyName,
                                         const BacktestResult& result);
        
        /**
         * @brief Generate a monthly returns heatmap
         * @param strategyName Name of the strategy
         * @param result Backtest result containing monthly returns
         * @return Path to the generated image file
         */
        std::string generateMonthlyReturnsHeatmap(const std::string& strategyName,
                                                const BacktestResult& result);
        
        /**
         * @brief Set the chart configuration
         * @param config The chart configuration
         */
        void setConfig(const ChartConfig& config);
        
    private:
        /**
         * @brief Ensure output directory exists
         */
        void ensureOutputDirExists() const;
        
        /**
         * @brief Apply common styling to charts
         */
        void applyChartStyle();
        
        ChartConfig m_config;
    };
} // namespace Backtest 