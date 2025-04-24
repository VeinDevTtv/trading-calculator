#include "EquityStats.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace Analytics {
    EquityAnalyzer::EquityAnalyzer() {}
    
    EquityStats EquityAnalyzer::calculateStats(const std::vector<std::shared_ptr<Trade>>& trades, 
                                             double initialBalance) {
        EquityStats stats;
        stats.initialBalance = initialBalance;
        stats.totalTrades = trades.size();
        
        if (trades.empty()) {
            stats.finalBalance = initialBalance;
            return stats;
        }
        
        // Generate equity curve for various calculations
        std::vector<double> equityCurve = generateEquityCurve(trades, initialBalance);
        stats.finalBalance = equityCurve.back();
        stats.totalPnL = stats.finalBalance - initialBalance;
        stats.percentGain = (initialBalance > 0) ? (stats.totalPnL / initialBalance * 100.0) : 0.0;
        
        // Count winning trades and calculate win rate
        int winningTrades = 0;
        double totalWins = 0.0;
        double totalLosses = 0.0;
        double maxWin = 0.0;
        double maxLoss = 0.0;
        
        for (const auto& trade : trades) {
            TradeOutcome outcome = trade->getOutcome();
            
            if (outcome == TradeOutcome::WinAtTP1 || outcome == TradeOutcome::WinAtTP2) {
                winningTrades++;
                double pnl = trade->getResults().rewardAmount;
                totalWins += pnl;
                maxWin = std::max(maxWin, pnl);
            } else if (outcome == TradeOutcome::LossAtSL) {
                double pnl = trade->getResults().riskAmount;
                totalLosses += pnl;
                maxLoss = std::max(maxLoss, pnl);
            }
        }
        
        stats.winRate = (trades.size() > 0) ? (static_cast<double>(winningTrades) / trades.size() * 100.0) : 0.0;
        stats.largestWin = maxWin;
        stats.largestLoss = maxLoss;
        stats.avgWin = (winningTrades > 0) ? (totalWins / winningTrades) : 0.0;
        int losingTrades = trades.size() - winningTrades;
        stats.avgLoss = (losingTrades > 0) ? (totalLosses / losingTrades) : 0.0;
        
        // Calculate average R-multiple (expectancy)
        double totalRMultiple = 0.0;
        for (const auto& trade : trades) {
            TradeOutcome outcome = trade->getOutcome();
            if (outcome == TradeOutcome::WinAtTP1 || outcome == TradeOutcome::WinAtTP2) {
                totalRMultiple += trade->getResults().riskRewardRatio;
            } else if (outcome == TradeOutcome::LossAtSL) {
                totalRMultiple -= 1.0;  // -1R
            }
        }
        stats.avgRMultiple = (trades.size() > 0) ? (totalRMultiple / trades.size()) : 0.0;
        stats.expectancy = stats.avgRMultiple;
        
        // Calculate advanced metrics
        calculateDrawdownMetrics(stats, equityCurve);
        calculateStreaks(stats, trades);
        stats.profitFactor = calculateProfitFactor(trades);
        
        // Calculate returns for Sharpe ratio
        std::vector<double> returns;
        for (size_t i = 1; i < equityCurve.size(); ++i) {
            double ret = (equityCurve[i] - equityCurve[i-1]) / equityCurve[i-1];
            returns.push_back(ret);
        }
        stats.sharpeRatio = calculateSharpeRatio(returns);
        
        return stats;
    }
    
    std::vector<double> EquityAnalyzer::generateEquityCurve(const std::vector<std::shared_ptr<Trade>>& trades,
                                                         double initialBalance) {
        std::vector<double> curve;
        curve.push_back(initialBalance);
        
        double currentBalance = initialBalance;
        for (const auto& trade : trades) {
            TradeOutcome outcome = trade->getOutcome();
            
            if (outcome == TradeOutcome::WinAtTP1 || outcome == TradeOutcome::WinAtTP2) {
                currentBalance += trade->getResults().rewardAmount;
            } else if (outcome == TradeOutcome::LossAtSL) {
                currentBalance -= trade->getResults().riskAmount;
            }
            
            curve.push_back(currentBalance);
        }
        
        return curve;
    }
    
    void EquityAnalyzer::calculateDrawdownMetrics(EquityStats& stats, const std::vector<double>& equityCurve) {
        if (equityCurve.size() < 2) {
            return;
        }
        
        double peakBalance = equityCurve[0];
        double maxDrawdown = 0.0;
        double currentDrawdown = 0.0;
        int drawdownDuration = 0;
        int maxDrawdownDuration = 0;
        
        for (size_t i = 1; i < equityCurve.size(); ++i) {
            // Update peak if we have a new high
            if (equityCurve[i] > peakBalance) {
                peakBalance = equityCurve[i];
                // Reset drawdown duration if we make a new high
                drawdownDuration = 0;
            } else {
                // We're in a drawdown
                drawdownDuration++;
                currentDrawdown = peakBalance - equityCurve[i];
                double drawdownPercent = (peakBalance > 0) ? (currentDrawdown / peakBalance * 100.0) : 0.0;
                
                // Update max drawdown if current is larger
                if (currentDrawdown > maxDrawdown) {
                    maxDrawdown = currentDrawdown;
                    stats.maxDrawdownPercent = drawdownPercent;
                }
                
                // Update max drawdown duration
                maxDrawdownDuration = std::max(maxDrawdownDuration, drawdownDuration);
            }
        }
        
        stats.maxDrawdown = maxDrawdown;
        stats.drawdownDuration = maxDrawdownDuration;
    }
    
    void EquityAnalyzer::calculateStreaks(EquityStats& stats, const std::vector<std::shared_ptr<Trade>>& trades) {
        int currentWinStreak = 0;
        int currentLoseStreak = 0;
        int maxWinStreak = 0;
        int maxLoseStreak = 0;
        
        for (const auto& trade : trades) {
            TradeOutcome outcome = trade->getOutcome();
            
            if (outcome == TradeOutcome::WinAtTP1 || outcome == TradeOutcome::WinAtTP2) {
                // Win
                currentWinStreak++;
                currentLoseStreak = 0;
                maxWinStreak = std::max(maxWinStreak, currentWinStreak);
            } else if (outcome == TradeOutcome::LossAtSL) {
                // Loss
                currentLoseStreak++;
                currentWinStreak = 0;
                maxLoseStreak = std::max(maxLoseStreak, currentLoseStreak);
            } else {
                // Break-even or pending
                currentWinStreak = 0;
                currentLoseStreak = 0;
            }
        }
        
        stats.longestWinStreak = maxWinStreak;
        stats.longestLoseStreak = maxLoseStreak;
        
        // Current streak (positive for wins, negative for losses)
        if (currentWinStreak > 0) {
            stats.currentStreak = currentWinStreak;
        } else if (currentLoseStreak > 0) {
            stats.currentStreak = -currentLoseStreak;
        } else {
            stats.currentStreak = 0;
        }
    }
    
    double EquityAnalyzer::calculateSharpeRatio(const std::vector<double>& returns) {
        if (returns.empty()) {
            return 0.0;
        }
        
        // Calculate average return
        double avgReturn = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
        
        // Calculate standard deviation of returns
        double sumSquaredDiff = 0.0;
        for (double ret : returns) {
            double diff = ret - avgReturn;
            sumSquaredDiff += diff * diff;
        }
        
        double stdDev = std::sqrt(sumSquaredDiff / returns.size());
        
        // Calculate annualized Sharpe ratio (approximation)
        // Assuming average of one trade per day
        // Using 0 as risk-free rate for simplicity
        return (stdDev > 0) ? (avgReturn / stdDev) * std::sqrt(252.0) : 0.0;
    }
    
    double EquityAnalyzer::calculateProfitFactor(const std::vector<std::shared_ptr<Trade>>& trades) {
        double totalWins = 0.0;
        double totalLosses = 0.0;
        
        for (const auto& trade : trades) {
            TradeOutcome outcome = trade->getOutcome();
            
            if (outcome == TradeOutcome::WinAtTP1 || outcome == TradeOutcome::WinAtTP2) {
                totalWins += trade->getResults().rewardAmount;
            } else if (outcome == TradeOutcome::LossAtSL) {
                totalLosses += trade->getResults().riskAmount;
            }
        }
        
        return (totalLosses > 0) ? (totalWins / totalLosses) : 0.0;
    }
    
    std::string EquityAnalyzer::getStatsReport(const EquityStats& stats) const {
        std::ostringstream oss;
        
        oss << "=== ADVANCED EQUITY STATISTICS ===\n\n";
        
        // Basic performance 
        oss << "Basic Performance:\n";
        oss << "  Initial Balance: $" << std::fixed << std::setprecision(2) << stats.initialBalance << "\n";
        oss << "  Final Balance:   $" << stats.finalBalance << "\n";
        oss << "  Total P&L:       $" << stats.totalPnL << " (" << std::setprecision(2) << stats.percentGain << "%)\n";
        oss << "  Win Rate:        " << stats.winRate << "%\n";
        oss << "  Total Trades:    " << stats.totalTrades << "\n\n";
        
        // Advanced metrics
        oss << "Risk Metrics:\n";
        oss << "  Max Drawdown:    $" << std::setprecision(2) << stats.maxDrawdown 
            << " (" << stats.maxDrawdownPercent << "%)\n";
        oss << "  Drawdown Length: " << stats.drawdownDuration << " trades\n";
        oss << "  Sharpe Ratio:    " << std::setprecision(3) << stats.sharpeRatio << "\n";
        oss << "  Profit Factor:   " << stats.profitFactor << "\n\n";
        
        // Trade streaks
        oss << "Trade Streaks:\n";
        oss << "  Longest Win Streak:  " << stats.longestWinStreak << " trades\n";
        oss << "  Longest Loss Streak: " << stats.longestLoseStreak << " trades\n";
        oss << "  Current Streak:      ";
        if (stats.currentStreak > 0) {
            oss << stats.currentStreak << " wins\n\n";
        } else if (stats.currentStreak < 0) {
            oss << -stats.currentStreak << " losses\n\n";
        } else {
            oss << "none\n\n";
        }
        
        // Trade stats
        oss << "Trade Statistics:\n";
        oss << "  Average Win:     $" << std::setprecision(2) << stats.avgWin << "\n";
        oss << "  Average Loss:    $" << stats.avgLoss << "\n";
        oss << "  Largest Win:     $" << stats.largestWin << "\n";
        oss << "  Largest Loss:    $" << stats.largestLoss << "\n";
        oss << "  Expectancy:      " << std::setprecision(3) << stats.expectancy << "R\n";
        
        return oss.str();
    }
} 