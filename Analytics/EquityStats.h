#ifndef ANALYTICS_EQUITY_STATS_H
#define ANALYTICS_EQUITY_STATS_H

#include <vector>
#include <string>
#include <memory>
#include "../Trade.h"

namespace Analytics {
    // Extended statistics for equity analysis
    struct EquityStats {
        // Basic stats
        double initialBalance = 0.0;
        double finalBalance = 0.0;
        double totalPnL = 0.0;
        double percentGain = 0.0;
        double winRate = 0.0;
        int totalTrades = 0;
        
        // Advanced metrics
        double maxDrawdown = 0.0;
        double maxDrawdownPercent = 0.0;
        int drawdownDuration = 0;
        double sharpeRatio = 0.0;
        double profitFactor = 0.0;
        
        // Streak statistics
        int longestWinStreak = 0;
        int longestLoseStreak = 0;
        int currentStreak = 0;
        
        // Trade metrics
        double avgWin = 0.0;
        double avgLoss = 0.0;
        double largestWin = 0.0;
        double largestLoss = 0.0;
        
        // R-multiples
        double avgRMultiple = 0.0;
        double expectancy = 0.0;  // Average R-multiple gain/loss per trade
    };
    
    class EquityAnalyzer {
    public:
        EquityAnalyzer();
        ~EquityAnalyzer() = default;
        
        // Calculate statistics from trade history
        EquityStats calculateStats(const std::vector<std::shared_ptr<Trade>>& trades, 
                                  double initialBalance);
                                  
        // Calculate equity curve as vector of balances
        std::vector<double> generateEquityCurve(const std::vector<std::shared_ptr<Trade>>& trades,
                                              double initialBalance);
        
        // Get formatted stats report as string
        std::string getStatsReport(const EquityStats& stats) const;
        
    private:
        // Helper methods
        void calculateDrawdownMetrics(EquityStats& stats, 
                                     const std::vector<double>& equityCurve);
                                     
        void calculateStreaks(EquityStats& stats,
                             const std::vector<std::shared_ptr<Trade>>& trades);
                             
        double calculateSharpeRatio(const std::vector<double>& returns);
        
        double calculateProfitFactor(const std::vector<std::shared_ptr<Trade>>& trades);
    };
}

#endif // ANALYTICS_EQUITY_STATS_H 