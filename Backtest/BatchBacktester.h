#ifndef BACKTEST_BATCH_BACKTESTER_H
#define BACKTEST_BATCH_BACKTESTER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Backtester.h"
#include "../Analytics/EquityStats.h"

namespace Backtest {
    // Structure to hold batch backtest results
    struct BatchBacktestResults {
        std::vector<std::string> strategyNames;
        std::map<std::string, BacktestResult> results;
        std::map<std::string, std::string> equityCurveImages;
        
        // Aggregated statistics
        std::string bestStrategy;
        std::string worstStrategy;
        double averageWinRate = 0.0;
        double averageProfitFactor = 0.0;
        double averageMaxDrawdown = 0.0;
    };

    class BatchBacktester {
    public:
        BatchBacktester();
        ~BatchBacktester() = default;
        
        // Add strategy files to the batch
        void addStrategyFile(const std::string& filePath);
        
        // Add all strategy files from a directory
        bool addStrategyDirectory(const std::string& dirPath);
        
        // Set common configuration for all backtests
        void setCommonConfig(const BacktestConfig& config);
        
        // Run all queued backtests
        BatchBacktestResults runBatchBacktest();
        
        // Export results
        bool exportSummaryReport(const std::string& filename) const;
        bool exportDetailedReport(const std::string& filename) const;
        
        // Generate MD report
        std::string generateMarkdownReport() const;
        
        // Clear all queued strategy files
        void clearStrategyFiles();
        
        // Get current batch results
        const BatchBacktestResults& getResults() const;
        
    private:
        std::vector<std::string> m_strategyFiles;
        BacktestConfig m_commonConfig;
        BatchBacktestResults m_results;
        
        // Helper methods
        std::string generateStrategySection(const std::string& strategyName, 
                                           const BacktestResult& result) const;
        void calculateAggregateStats();
        std::string generateEquityCurveImage(const std::string& strategyName, 
                                            const BacktestResult& result);
    };
}

#endif // BACKTEST_BATCH_BACKTESTER_H 