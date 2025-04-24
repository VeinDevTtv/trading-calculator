#include "BatchBacktester.h"
#include "../Utils.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace Backtest {
    BatchBacktester::BatchBacktester() {
        // Initialize with some reasonable defaults
        m_commonConfig.initialBalance = 10000.0;
        m_commonConfig.riskPerTrade = 1.0;
        m_commonConfig.riskRewardRatio = 2.0;
        m_commonConfig.strategyType = StrategyType::FIXED_RR;
    }
    
    void BatchBacktester::addStrategyFile(const std::string& filePath) {
        // Check if file exists and has .csv extension
        if (std::filesystem::exists(filePath) && 
            filePath.substr(filePath.find_last_of(".") + 1) == "csv") {
            m_strategyFiles.push_back(filePath);
        } else {
            std::cerr << "Invalid strategy file: " << filePath << std::endl;
        }
    }
    
    bool BatchBacktester::addStrategyDirectory(const std::string& dirPath) {
        if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
            return false;
        }
        
        bool foundFiles = false;
        
        // Iterate through directory and add all CSV files
        for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
            if (entry.is_regular_file() && 
                entry.path().extension() == ".csv") {
                m_strategyFiles.push_back(entry.path().string());
                foundFiles = true;
            }
        }
        
        return foundFiles;
    }
    
    void BatchBacktester::setCommonConfig(const BacktestConfig& config) {
        m_commonConfig = config;
    }
    
    BatchBacktestResults BatchBacktester::runBatchBacktest() {
        // Clear previous results
        m_results = BatchBacktestResults();
        
        if (m_strategyFiles.empty()) {
            std::cerr << "No strategy files to backtest." << std::endl;
            return m_results;
        }
        
        // Initialize Backtester
        Backtester backtester;
        backtester.setConfig(m_commonConfig);
        
        // Process each strategy file
        for (const auto& filePath : m_strategyFiles) {
            // Extract strategy name from filename
            std::string fileName = filePath.substr(filePath.find_last_of("/\\") + 1);
            std::string strategyName = fileName.substr(0, fileName.find_last_of('.'));
            
            std::cout << "Backtesting strategy: " << strategyName << std::endl;
            
            // Load price data
            if (!backtester.loadPriceData(filePath)) {
                std::cerr << "Failed to load data for strategy: " << strategyName << std::endl;
                continue;
            }
            
            // Run backtest
            BacktestResult result = backtester.runBacktest();
            
            // Store results
            m_results.strategyNames.push_back(strategyName);
            m_results.results[strategyName] = result;
            
            // Generate equity curve image
            m_results.equityCurveImages[strategyName] = generateEquityCurveImage(strategyName, result);
            
            std::cout << "Completed backtest for " << strategyName 
                      << ": Win Rate = " << std::fixed << std::setprecision(2) 
                      << result.winRate << "%, Profit Factor = " 
                      << result.profitFactor << std::endl;
        }
        
        // Calculate aggregate statistics
        calculateAggregateStats();
        
        return m_results;
    }
    
    bool BatchBacktester::exportSummaryReport(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        // Write summary report in Markdown format
        file << "# Backtest Summary Report\n\n";
        file << "Generated on: " << Utils::getFormattedTimestamp(std::time(nullptr)) << "\n\n";
        
        file << "## Overview\n\n";
        file << "Strategies tested: " << m_results.strategyNames.size() << "\n\n";
        file << "| Strategy | Win Rate | Profit Factor | Net Profit | Max Drawdown |\n";
        file << "|----------|----------|--------------|------------|-------------|\n";
        
        for (const auto& name : m_results.strategyNames) {
            const auto& result = m_results.results.at(name);
            file << "| " << name 
                 << " | " << std::fixed << std::setprecision(2) << result.winRate << "%" 
                 << " | " << std::setprecision(2) << result.profitFactor 
                 << " | $" << std::setprecision(2) << result.netProfit 
                 << " | " << std::setprecision(2) << result.stats.maxDrawdownPercent << "%" 
                 << " |\n";
        }
        
        file << "\n## Aggregate Statistics\n\n";
        file << "Average Win Rate: " << std::fixed << std::setprecision(2) << m_results.averageWinRate << "%\n\n";
        file << "Average Profit Factor: " << std::setprecision(2) << m_results.averageProfitFactor << "\n\n";
        file << "Average Max Drawdown: " << std::setprecision(2) << m_results.averageMaxDrawdown << "%\n\n";
        file << "Best Performing Strategy: " << m_results.bestStrategy << "\n\n";
        file << "Worst Performing Strategy: " << m_results.worstStrategy << "\n\n";
        
        file.close();
        return true;
    }
    
    bool BatchBacktester::exportDetailedReport(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        // Write full markdown report
        file << generateMarkdownReport();
        
        file.close();
        return true;
    }
    
    std::string BatchBacktester::generateMarkdownReport() const {
        std::stringstream ss;
        
        ss << "# Detailed Backtest Report\n\n";
        ss << "Generated on: " << Utils::getFormattedTimestamp(std::time(nullptr)) << "\n\n";
        
        ss << "## Overview\n\n";
        ss << "Strategies tested: " << m_results.strategyNames.size() << "\n\n";
        ss << "Initial balance: $" << std::fixed << std::setprecision(2) << m_commonConfig.initialBalance << "\n\n";
        ss << "Risk per trade: " << m_commonConfig.riskPerTrade << "%\n\n";
        
        // Aggregate stats section
        ss << "## Aggregate Statistics\n\n";
        ss << "Average Win Rate: " << std::fixed << std::setprecision(2) << m_results.averageWinRate << "%\n\n";
        ss << "Average Profit Factor: " << std::setprecision(2) << m_results.averageProfitFactor << "\n\n";
        ss << "Average Max Drawdown: " << std::setprecision(2) << m_results.averageMaxDrawdown << "%\n\n";
        ss << "Best Performing Strategy: " << m_results.bestStrategy << "\n\n";
        ss << "Worst Performing Strategy: " << m_results.worstStrategy << "\n\n";
        
        // Comparison table
        ss << "## Strategy Comparison\n\n";
        ss << "| Strategy | Win Rate | Profit Factor | Net Profit | Max Drawdown | Sharpe Ratio |\n";
        ss << "|----------|----------|--------------|------------|--------------|-------------|\n";
        
        for (const auto& name : m_results.strategyNames) {
            const auto& result = m_results.results.at(name);
            ss << "| " << name 
               << " | " << std::fixed << std::setprecision(2) << result.winRate << "%" 
               << " | " << std::setprecision(2) << result.profitFactor 
               << " | $" << std::setprecision(2) << result.netProfit 
               << " | " << std::setprecision(2) << result.stats.maxDrawdownPercent << "%" 
               << " | " << std::setprecision(2) << result.stats.sharpeRatio
               << " |\n";
        }
        
        // Individual strategy sections
        ss << "\n## Individual Strategy Reports\n\n";
        
        for (const auto& name : m_results.strategyNames) {
            ss << generateStrategySection(name, m_results.results.at(name));
        }
        
        return ss.str();
    }
    
    void BatchBacktester::clearStrategyFiles() {
        m_strategyFiles.clear();
    }
    
    const BatchBacktestResults& BatchBacktester::getResults() const {
        return m_results;
    }
    
    std::string BatchBacktester::generateStrategySection(const std::string& strategyName, 
                                                       const BacktestResult& result) const {
        std::stringstream ss;
        
        ss << "### " << strategyName << "\n\n";
        
        // Basic metrics
        ss << "#### Performance Metrics\n\n";
        ss << "- **Total Trades**: " << result.totalTrades << "\n";
        ss << "- **Win Rate**: " << std::fixed << std::setprecision(2) << result.winRate << "%\n";
        ss << "- **Profit Factor**: " << std::setprecision(2) << result.profitFactor << "\n";
        ss << "- **Net Profit**: $" << std::setprecision(2) << result.netProfit << "\n";
        ss << "- **Max Drawdown**: " << std::setprecision(2) << result.stats.maxDrawdownPercent << "%\n";
        ss << "- **Sharpe Ratio**: " << std::setprecision(2) << result.stats.sharpeRatio << "\n";
        ss << "- **Longest Win Streak**: " << result.stats.longestWinStreak << "\n";
        ss << "- **Longest Lose Streak**: " << result.stats.longestLoseStreak << "\n\n";
        
        // Equity curve
        ss << "#### Equity Curve\n\n";
        
        // If we have an image path, link to it
        auto it = m_results.equityCurveImages.find(strategyName);
        if (it != m_results.equityCurveImages.end() && !it->second.empty()) {
            ss << "![" << strategyName << " Equity Curve](" << it->second << ")\n\n";
        } else {
            // Otherwise, include an ASCII chart if we have curve data
            if (!result.equityCurve.empty()) {
                ss << "```\n" << Utils::generateASCIIChart(result.equityCurve, 70, 15) << "\n```\n\n";
            } else {
                ss << "No equity curve data available.\n\n";
            }
        }
        
        // Monthly/periodic breakdown could be added here
        
        ss << "---\n\n";
        
        return ss.str();
    }
    
    void BatchBacktester::calculateAggregateStats() {
        if (m_results.strategyNames.empty()) {
            return;
        }
        
        double totalWinRate = 0.0;
        double totalProfitFactor = 0.0;
        double totalMaxDrawdown = 0.0;
        
        // Variables to track best and worst performance
        double bestNetProfit = std::numeric_limits<double>::lowest();
        double worstNetProfit = std::numeric_limits<double>::max();
        
        for (const auto& name : m_results.strategyNames) {
            const auto& result = m_results.results.at(name);
            
            // Accumulate statistics
            totalWinRate += result.winRate;
            totalProfitFactor += result.profitFactor;
            totalMaxDrawdown += result.stats.maxDrawdownPercent;
            
            // Check if this is the best or worst strategy
            if (result.netProfit > bestNetProfit) {
                bestNetProfit = result.netProfit;
                m_results.bestStrategy = name;
            }
            
            if (result.netProfit < worstNetProfit) {
                worstNetProfit = result.netProfit;
                m_results.worstStrategy = name;
            }
        }
        
        // Calculate averages
        size_t numStrategies = m_results.strategyNames.size();
        m_results.averageWinRate = totalWinRate / numStrategies;
        m_results.averageProfitFactor = totalProfitFactor / numStrategies;
        m_results.averageMaxDrawdown = totalMaxDrawdown / numStrategies;
    }
    
    std::string BatchBacktester::generateEquityCurveImage(const std::string& strategyName, 
                                                        const BacktestResult& result) {
        // Placeholder implementation - in a real system, this would generate
        // actual image files using a plotting library
        
        // For now, we'll just simulate this by creating a filename
        // In a real implementation, you would:
        // 1. Use a library like MatPlot++ or Cairo to draw the equity curve
        // 2. Save the image to a file
        // 3. Return the path to the saved image
        
        std::string imagePath = "equity_curves/" + strategyName + "_equity.png";
        
        // Placeholder note that in this stub implementation, we're not actually creating the file
        std::cout << "Note: Equity curve image generation is stubbed. Would create: " 
                  << imagePath << std::endl;
        
        // For a real implementation, return the path to the created image
        // For this stub, we'll return an empty string which will cause the report
        // to fall back to ASCII charts
        return "";
    }
} 