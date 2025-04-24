#ifndef BACKTEST_BACKTESTER_H
#define BACKTEST_BACKTESTER_H

#include <string>
#include <vector>
#include <memory>
#include "../Trade.h"
#include "../Analytics/EquityStats.h"

namespace Backtest {
    struct CandleData {
        std::time_t timestamp;
        double open;
        double high;
        double low;
        double close;
        double volume = 0.0;
    };
    
    enum class StrategyType {
        FIXED_RR,         // Fixed risk-reward ratio
        STRUCTURE_BASED,  // SL/TP based on market structure
        DYNAMIC_TARGET    // Dynamic take profit/trailing stop
    };
    
    // Configuration for backtest
    struct BacktestConfig {
        double initialBalance = 10000.0;
        double riskPerTrade = 1.0;  // Percentage of account
        double stopLossPips = 0.0;  // If fixed
        double takeProfitPips = 0.0; // If fixed
        double riskRewardRatio = 0.0; // If using fixed RR
        StrategyType strategyType = StrategyType::FIXED_RR;
        bool useCompounding = false;
        bool useLimitOrders = false;
        
        // Entry rules
        bool longEnabled = true;
        bool shortEnabled = true;
    };
    
    // Result of a backtest run
    struct BacktestResult {
        std::vector<std::shared_ptr<Trade>> trades;
        Analytics::EquityStats stats;
        std::vector<double> equityCurve;
        std::vector<double> drawdownCurve;
        
        int totalTrades = 0;
        int winningTrades = 0;
        int losingTrades = 0;
        double winRate = 0.0;
        double profitFactor = 0.0;
        double netProfit = 0.0;
    };
    
    class Backtester {
    public:
        Backtester();
        ~Backtester() = default;
        
        // Set configuration
        void setConfig(const BacktestConfig& config);
        
        // Load data from CSV file
        bool loadPriceData(const std::string& filename);
        
        // Run backtest
        BacktestResult runBacktest();
        
        // Export results
        bool exportResults(const std::string& filename) const;
        
        // Display results in console
        void displayResults() const;
        
    private:
        BacktestConfig m_config;
        std::vector<CandleData> m_priceData;
        BacktestResult m_lastResult;
        
        // Helper methods
        bool detectEntry(int index, bool& isLong) const;
        std::pair<double, double> calculateStopLossAndTakeProfit(int index, bool isLong) const;
        bool simulateTrade(int entryIndex, bool isLong, BacktestResult& result);
    };
}

#endif // BACKTEST_BACKTESTER_H 