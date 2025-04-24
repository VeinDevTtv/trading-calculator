#include "Backtester.h"
#include "../Utils.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <iostream>

namespace Backtest {
    Backtester::Backtester() {
        // Set default config
        m_config.initialBalance = 10000.0;
        m_config.riskPerTrade = 1.0;
        m_config.stopLossPips = 10.0;
        m_config.takeProfitPips = 20.0;
        m_config.riskRewardRatio = 2.0;
        m_config.strategyType = StrategyType::FIXED_RR;
    }
    
    void Backtester::setConfig(const BacktestConfig& config) {
        m_config = config;
    }
    
    bool Backtester::loadPriceData(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return false;
        }
        
        m_priceData.clear();
        
        std::string line;
        std::getline(file, line); // Skip header line
        
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string field;
            CandleData candle;
            
            // Parse date/time
            std::getline(ss, field, ',');
            std::tm tm = {};
            std::istringstream dateStream(field);
            dateStream >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
            candle.timestamp = std::mktime(&tm);
            
            // Parse OHLC
            std::getline(ss, field, ',');
            candle.open = std::stod(field);
            
            std::getline(ss, field, ',');
            candle.high = std::stod(field);
            
            std::getline(ss, field, ',');
            candle.low = std::stod(field);
            
            std::getline(ss, field, ',');
            candle.close = std::stod(field);
            
            // Parse volume if available
            if (std::getline(ss, field, ',')) {
                try {
                    candle.volume = std::stod(field);
                } catch (...) {
                    candle.volume = 0.0;
                }
            }
            
            m_priceData.push_back(candle);
        }
        
        file.close();
        
        // Sort by timestamp if needed
        std::sort(m_priceData.begin(), m_priceData.end(), [](const CandleData& a, const CandleData& b) {
            return a.timestamp < b.timestamp;
        });
        
        return !m_priceData.empty();
    }
    
    BacktestResult Backtester::runBacktest() {
        BacktestResult result;
        
        // Reset results
        result.trades.clear();
        result.equityCurve.clear();
        result.equityCurve.push_back(m_config.initialBalance);
        result.drawdownCurve.clear();
        result.drawdownCurve.push_back(0.0);
        
        double currentBalance = m_config.initialBalance;
        double peakBalance = currentBalance;
        
        // Simple fixed RR strategy implementation for demonstration
        for (size_t i = 1; i < m_priceData.size() - 1; ++i) {
            bool isLongEntry = false;
            
            // Check if we have an entry signal at this candle
            if (detectEntry(i, isLongEntry)) {
                // If direction is disabled in config, skip
                if ((isLongEntry && !m_config.longEnabled) || 
                    (!isLongEntry && !m_config.shortEnabled)) {
                    continue;
                }
                
                // Create a trade
                auto trade = std::make_shared<Trade>();
                trade->setAccountBalance(currentBalance);
                trade->setEntryPrice(m_priceData[i].close);
                
                // Set risk percentage (fixed or compounding)
                double riskPercent = m_config.riskPerTrade;
                trade->setRiskPercentage(riskPercent);
                
                // Calculate SL and TP based on strategy
                auto [sl, tp] = calculateStopLossAndTakeProfit(i, isLongEntry);
                
                // Set trade properties
                if (isLongEntry) {
                    // Long trade
                    trade->setStopLoss(sl, InputType::Price);
                    trade->setTakeProfit(tp, InputType::Price);
                } else {
                    // Short trade
                    trade->setStopLoss(sl, InputType::Price);
                    trade->setTakeProfit(tp, InputType::Price);
                }
                
                // Calculate position size
                trade->setInstrumentType(0); // Forex
                trade->setLotSizeType(0);    // Standard lot
                trade->calculate();
                
                // Forward test this trade to find outcome
                bool simulated = simulateTrade(i, isLongEntry, result);
                if (simulated) {
                    i += 5; // Skip a few candles after a trade (to avoid immediate re-entry)
                }
            }
        }
        
        // Calculate statistics
        Analytics::EquityAnalyzer analyzer;
        result.stats = analyzer.calculateStats(result.trades, m_config.initialBalance);
        
        // Update summary stats
        result.totalTrades = result.trades.size();
        result.winningTrades = 0;
        result.losingTrades = 0;
        
        for (const auto& trade : result.trades) {
            if (trade->getOutcome() == TradeOutcome::WinAtTP1 || 
                trade->getOutcome() == TradeOutcome::WinAtTP2) {
                result.winningTrades++;
            } else if (trade->getOutcome() == TradeOutcome::LossAtSL) {
                result.losingTrades++;
            }
        }
        
        result.winRate = (result.totalTrades > 0) ? 
            (static_cast<double>(result.winningTrades) / result.totalTrades * 100.0) : 0.0;
            
        result.netProfit = result.stats.finalBalance - m_config.initialBalance;
        result.profitFactor = result.stats.profitFactor;
        
        m_lastResult = result;
        return result;
    }
    
    bool Backtester::detectEntry(int index, bool& isLong) const {
        // Simple strategy for demonstration purposes:
        // Long entry: current close > previous close and current close > current open
        // Short entry: current close < previous close and current close < current open
        
        if (index <= 0 || index >= static_cast<int>(m_priceData.size()) - 1) {
            return false;
        }
        
        const auto& current = m_priceData[index];
        const auto& previous = m_priceData[index - 1];
        
        // Long entry conditions
        if (current.close > previous.close && current.close > current.open) {
            isLong = true;
            return true;
        }
        
        // Short entry conditions
        if (current.close < previous.close && current.close < current.open) {
            isLong = false;
            return true;
        }
        
        return false;
    }
    
    std::pair<double, double> Backtester::calculateStopLossAndTakeProfit(int index, bool isLong) const {
        double sl = 0.0, tp = 0.0;
        
        // Get the current candle
        const auto& current = m_priceData[index];
        double entryPrice = current.close;
        
        switch (m_config.strategyType) {
            case StrategyType::FIXED_RR: {
                // Using fixed pips for SL/TP
                if (isLong) {
                    sl = entryPrice - m_config.stopLossPips * 0.0001;
                    tp = entryPrice + m_config.takeProfitPips * 0.0001;
                } else {
                    sl = entryPrice + m_config.stopLossPips * 0.0001;
                    tp = entryPrice - m_config.takeProfitPips * 0.0001;
                }
                break;
            }
            
            case StrategyType::STRUCTURE_BASED: {
                // Using recent swing highs/lows for SL placement
                // Look back a few candles to find suitable levels
                double swingHigh = entryPrice;
                double swingLow = entryPrice;
                
                // Find swing points in last 10 candles
                for (int i = index - 1; i >= std::max(0, index - 10); --i) {
                    swingHigh = std::max(swingHigh, m_priceData[i].high);
                    swingLow = std::min(swingLow, m_priceData[i].low);
                }
                
                if (isLong) {
                    sl = swingLow;
                    double slDistance = entryPrice - sl;
                    tp = entryPrice + (slDistance * m_config.riskRewardRatio);
                } else {
                    sl = swingHigh;
                    double slDistance = sl - entryPrice;
                    tp = entryPrice - (slDistance * m_config.riskRewardRatio);
                }
                break;
            }
            
            default:
                // Default to some reasonable values
                if (isLong) {
                    sl = entryPrice * 0.99;
                    tp = entryPrice * 1.01;
                } else {
                    sl = entryPrice * 1.01;
                    tp = entryPrice * 0.99;
                }
                break;
        }
        
        return {sl, tp};
    }
    
    bool Backtester::simulateTrade(int entryIndex, bool isLong, BacktestResult& result) {
        if (entryIndex >= static_cast<int>(m_priceData.size()) - 1) {
            return false;
        }
        
        const auto& entryCandle = m_priceData[entryIndex];
        double entryPrice = entryCandle.close;
        
        // Create a new trade
        auto trade = std::make_shared<Trade>();
        
        // Current account balance for risk calculation
        double currentBalance = result.equityCurve.back();
        trade->setAccountBalance(currentBalance);
        trade->setRiskPercentage(m_config.riskPerTrade);
        trade->setEntryPrice(entryPrice);
        
        // Set stop loss and take profit
        auto [stopLoss, takeProfit] = calculateStopLossAndTakeProfit(entryIndex, isLong);
        trade->setStopLoss(stopLoss, InputType::Price);
        trade->setTakeProfit(takeProfit, InputType::Price);
        
        // Calculate the trade
        trade->setInstrumentType(0); // Forex
        trade->setLotSizeType(0);    // Standard lot
        trade->calculate();
        
        // Simulate the trade outcome
        TradeOutcome outcome = TradeOutcome::Pending;
        bool tradeFinished = false;
        
        // Look forward in time to see if SL or TP is hit
        for (int i = entryIndex + 1; i < static_cast<int>(m_priceData.size()) && !tradeFinished; ++i) {
            const auto& candle = m_priceData[i];
            
            if (isLong) {
                // Check if stop loss hit
                if (candle.low <= stopLoss) {
                    outcome = TradeOutcome::LossAtSL;
                    tradeFinished = true;
                }
                // Check if take profit hit
                else if (candle.high >= takeProfit) {
                    outcome = TradeOutcome::WinAtTP1;
                    tradeFinished = true;
                }
            } else {
                // Check if stop loss hit
                if (candle.high >= stopLoss) {
                    outcome = TradeOutcome::LossAtSL;
                    tradeFinished = true;
                }
                // Check if take profit hit
                else if (candle.low <= takeProfit) {
                    outcome = TradeOutcome::WinAtTP1;
                    tradeFinished = true;
                }
            }
            
            // Limit maximum trade duration 
            if (i >= entryIndex + 100) {
                // If trade doesn't hit SL or TP within 100 candles, close at current price
                double closePrice = candle.close;
                
                if (isLong) {
                    if (closePrice > entryPrice) {
                        outcome = TradeOutcome::WinAtTP1;
                    } else if (closePrice < entryPrice) {
                        outcome = TradeOutcome::LossAtSL;
                    } else {
                        outcome = TradeOutcome::BreakEven;
                    }
                } else {
                    if (closePrice < entryPrice) {
                        outcome = TradeOutcome::WinAtTP1;
                    } else if (closePrice > entryPrice) {
                        outcome = TradeOutcome::LossAtSL;
                    } else {
                        outcome = TradeOutcome::BreakEven;
                    }
                }
                
                tradeFinished = true;
            }
        }
        
        // If we ran out of data before the trade completed, close it at the last price
        if (!tradeFinished && !m_priceData.empty()) {
            outcome = TradeOutcome::Pending;
            return false;
        }
        
        // Complete the trade
        trade->simulateOutcome(outcome);
        
        // Update equity curve
        double newBalance = trade->getUpdatedAccountBalance();
        result.equityCurve.push_back(newBalance);
        
        // Update drawdown curve
        double peakBalance = *std::max_element(result.equityCurve.begin(), result.equityCurve.end());
        double currentDrawdown = (peakBalance - newBalance) / peakBalance * 100.0;
        result.drawdownCurve.push_back(currentDrawdown);
        
        // Add trade to results
        result.trades.push_back(trade);
        
        return true;
    }
    
    bool Backtester::exportResults(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        // Write header
        file << "Trade,Entry Price,SL,TP,Outcome,P&L,Balance\n";
        
        // Write each trade
        double balance = m_config.initialBalance;
        for (size_t i = 0; i < m_lastResult.trades.size(); ++i) {
            const auto& trade = m_lastResult.trades[i];
            auto results = trade->getResults();
            
            // Update balance
            if (trade->getOutcome() == TradeOutcome::WinAtTP1 || 
                trade->getOutcome() == TradeOutcome::WinAtTP2) {
                balance += results.rewardAmount;
            } else if (trade->getOutcome() == TradeOutcome::LossAtSL) {
                balance -= results.riskAmount;
            }
            
            file << i + 1 << ","
                 << trade->getParameters().entryPrice << ","
                 << results.stopLossPrice << ","
                 << results.takeProfitPrice << ","
                 << trade->getOutcomeAsString() << ",";
                 
            if (trade->getOutcome() == TradeOutcome::WinAtTP1 || 
                trade->getOutcome() == TradeOutcome::WinAtTP2) {
                file << results.rewardAmount;
            } else if (trade->getOutcome() == TradeOutcome::LossAtSL) {
                file << -results.riskAmount;
            } else {
                file << "0.00";
            }
            
            file << "," << balance << "\n";
        }
        
        file.close();
        return true;
    }
    
    void Backtester::displayResults() const {
        if (m_lastResult.trades.empty()) {
            std::cout << "No backtest results available.\n";
            return;
        }
        
        Utils::printHeader("BACKTEST RESULTS");
        
        std::cout << "Initial Balance: $" << std::fixed << std::setprecision(2) 
                 << m_config.initialBalance << "\n";
        std::cout << "Final Balance:   $" << m_lastResult.stats.finalBalance << "\n";
        std::cout << "Net Profit:      $" << m_lastResult.netProfit 
                 << " (" << m_lastResult.stats.percentGain << "%)\n";
        std::cout << "Total Trades:    " << m_lastResult.totalTrades << "\n";
        std::cout << "Win Rate:        " << std::setprecision(2) << m_lastResult.winRate << "%\n";
        std::cout << "Win/Loss:        " << m_lastResult.winningTrades << "/" 
                 << m_lastResult.losingTrades << "\n";
        std::cout << "Profit Factor:   " << std::setprecision(3) << m_lastResult.profitFactor << "\n";
        std::cout << "Max Drawdown:    " << std::setprecision(2) << m_lastResult.stats.maxDrawdownPercent << "%\n";
        std::cout << "Sharpe Ratio:    " << m_lastResult.stats.sharpeRatio << "\n\n";
        
        // Display equity curve
        std::cout << "Equity Curve:\n";
        std::cout << Utils::generateASCIIChart(m_lastResult.equityCurve, 70, 10) << "\n\n";
        
        std::cout << "Drawdown Curve:\n";
        std::cout << Utils::generateASCIIChart(m_lastResult.drawdownCurve, 70, 10) << "\n";
    }
} 