#pragma once

#include "Backtester.h"
#include <string>
#include <vector>
#include <map>

namespace Backtest {
    /**
     * @brief Structure to hold batch backtest results across multiple strategies
     */
    struct BatchBacktestResults {
        std::vector<std::string> strategyNames;
        std::map<std::string, BacktestResult> results;
        std::map<std::string, std::string> equityCurveImages;
        
        // Aggregate statistics
        double averageWinRate = 0.0;
        double averageProfitFactor = 0.0;
        double averageMaxDrawdown = 0.0;
        std::string bestStrategy;
        std::string worstStrategy;
    };

    /**
     * @brief Class that handles running backtests on multiple strategy files and aggregating results
     */
    class BatchBacktester {
    public:
        /**
         * @brief Constructor with default configuration
         */
        BatchBacktester();
        
        /**
         * @brief Add a single strategy file to the batch
         * @param filePath Path to a CSV file containing price data for the strategy
         */
        void addStrategyFile(const std::string& filePath);
        
        /**
         * @brief Add all CSV files from a directory to the batch
         * @param dirPath Path to directory containing strategy CSV files
         * @return True if any files were found and added, false otherwise
         */
        bool addStrategyDirectory(const std::string& dirPath);
        
        /**
         * @brief Set configuration parameters to be used for all backtests
         * @param config The backtest configuration
         */
        void setCommonConfig(const BacktestConfig& config);
        
        /**
         * @brief Run backtests on all added strategy files
         * @return The batch backtest results
         */
        BatchBacktestResults runBatchBacktest();
        
        /**
         * @brief Export a summary report in markdown format
         * @param filename The output filename
         * @return True if export succeeded, false otherwise
         */
        bool exportSummaryReport(const std::string& filename) const;
        
        /**
         * @brief Export a detailed report in markdown format
         * @param filename The output filename
         * @return True if export succeeded, false otherwise
         */
        bool exportDetailedReport(const std::string& filename) const;
        
        /**
         * @brief Clear the list of strategy files
         */
        void clearStrategyFiles();
        
        /**
         * @brief Get the current batch backtest results
         * @return The batch backtest results
         */
        const BatchBacktestResults& getResults() const;
        
    private:
        /**
         * @brief Generate a markdown report containing all results
         * @return String containing the markdown report
         */
        std::string generateMarkdownReport() const;
        
        /**
         * @brief Generate a markdown section for a specific strategy
         * @param strategyName Name of the strategy
         * @param result Backtest result for the strategy
         * @return String containing the markdown section
         */
        std::string generateStrategySection(const std::string& strategyName, 
                                           const BacktestResult& result) const;
        
        /**
         * @brief Calculate aggregate statistics across all strategies
         */
        void calculateAggregateStats();
        
        /**
         * @brief Generate an equity curve image for a strategy
         * @param strategyName Name of the strategy
         * @param result Backtest result containing equity curve data
         * @return Path to the generated image file
         */
        std::string generateEquityCurveImage(const std::string& strategyName, 
                                            const BacktestResult& result);
        
        std::vector<std::string> m_strategyFiles;  // List of strategy file paths
        BacktestConfig m_commonConfig;             // Common configuration for all backtests
        BatchBacktestResults m_results;            // Results of the batch backtest
    };
} 