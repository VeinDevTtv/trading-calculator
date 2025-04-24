#pragma once

#include "Backtester.h"
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <nlohmann/json.hpp>

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
        
        // Performance metrics
        struct PerformanceMetrics {
            std::chrono::milliseconds totalDuration{0};
            std::chrono::milliseconds avgStrategyDuration{0};
            std::chrono::milliseconds maxStrategyDuration{0};
            std::string slowestStrategy;
            size_t peakMemoryUsageMB = 0;
            size_t batchesProcessed = 0;
        } performance;
    };

    /**
     * @brief Batch backtest configuration parameters
     */
    struct BatchConfig {
        // Performance settings
        unsigned int threadCount = 0; // 0 means use hardware concurrency
        size_t batchSize = 10;
        size_t memoryLimitMB = 2048;
        
        // Output settings
        std::vector<std::string> outputFormats = {"markdown"};
        std::string chartFormat = "png";
        int chartWidth = 1200;
        int chartHeight = 800;
        int chartDPI = 100;
        bool includeChartsInReport = true;
        
        // Path settings
        std::string strategyDir = "data/strategies";
        std::string outputDir = "exports";
        std::string chartDir = "exports/charts";
        
        // Logging settings
        std::string logLevel = "info";
        std::string logFile = "logs/batch_backtest.log";
        bool consoleOutput = true;
        bool trackPerformance = true;
        
        // Backtest settings
        BacktestConfig backtestConfig;
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
         * @brief Load batch configuration from a JSON file
         * @param configFile Path to JSON configuration file
         * @return True if configuration was loaded successfully, false otherwise
         */
        bool loadBatchConfig(const std::string& configFile);
        
        /**
         * @brief Set batch configuration parameters
         * @param config The batch configuration
         */
        void setBatchConfig(const BatchConfig& config);
        
        /**
         * @brief Get the current batch configuration
         * @return The batch configuration
         */
        const BatchConfig& getBatchConfig() const;
        
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
         * @brief Export results in JSON format
         * @param filename The output filename
         * @return True if export succeeded, false otherwise
         */
        bool exportJsonReport(const std::string& filename) const;
        
        /**
         * @brief Export results in CSV format
         * @param filename The output filename
         * @return True if export succeeded, false otherwise
         */
        bool exportCsvReport(const std::string& filename) const;
        
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
        
        /**
         * @brief Track memory usage of the current process
         * @return Current memory usage in MB
         */
        size_t getCurrentMemoryUsage() const;
        
        /**
         * @brief Create all necessary directories for output files
         */
        void ensureDirectoriesExist() const;
        
        /**
         * @brief Apply default configuration values if not set
         */
        void applyDefaultConfig();
        
        std::vector<std::string> m_strategyFiles;  // List of strategy file paths
        BacktestConfig m_commonConfig;             // Common configuration for all backtests
        BatchConfig m_batchConfig;                 // Batch processing configuration
        BatchBacktestResults m_results;            // Results of the batch backtest
    };
    
    // JSON conversion functions for BatchConfig
    void to_json(nlohmann::json& j, const BatchConfig& config);
    void from_json(const nlohmann::json& j, BatchConfig& config);
} 