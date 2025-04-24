#include "BatchBacktester.h"
#include "../Utils.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <thread>
#include <future>
#include <mutex>
#include <chrono>
#include <spdlog/spdlog.h>

namespace Backtest {
    BatchBacktester::BatchBacktester() {
        spdlog::info("Initializing BatchBacktester");
        // Initialize with some reasonable defaults
        m_commonConfig.initialBalance = 10000.0;
        m_commonConfig.riskPerTrade = 1.0;
        m_commonConfig.riskRewardRatio = 2.0;
        m_commonConfig.strategyType = StrategyType::FIXED_RR;
    }
    
    void BatchBacktester::addStrategyFile(const std::string& filePath) {
        if (!std::filesystem::exists(filePath)) {
            spdlog::error("Strategy file not found: {}", filePath);
            throw std::runtime_error("Strategy file not found: " + filePath);
        }
        
        if (std::filesystem::path(filePath).extension() != ".csv") {
            spdlog::warn("Non-CSV file added to batch: {}", filePath);
        }
        
        m_strategyFiles.push_back(filePath);
        spdlog::info("Added strategy file: {}", filePath);
    }
    
    bool BatchBacktester::addStrategyDirectory(const std::string& dirPath) {
        if (!std::filesystem::exists(dirPath)) {
            spdlog::error("Strategy directory not found: {}", dirPath);
            return false;
        }
        
        bool filesAdded = false;
        for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
            if (entry.path().extension() == ".csv") {
                m_strategyFiles.push_back(entry.path().string());
                filesAdded = true;
                spdlog::info("Added strategy file from directory: {}", entry.path().string());
            }
        }
        
        return filesAdded;
    }
    
    void BatchBacktester::setCommonConfig(const BacktestConfig& config) {
        // Validate configuration
        if (config.initialCapital <= 0) {
            throw std::invalid_argument("Initial capital must be positive");
        }
        if (config.commission <= 0) {
            throw std::invalid_argument("Commission must be positive");
        }
        if (config.slippage < 0) {
            throw std::invalid_argument("Slippage cannot be negative");
        }
        
        m_commonConfig = config;
        spdlog::info("Set common configuration for batch backtest");
    }
    
    BatchBacktestResults BatchBacktester::runBatchBacktest() {
        if (m_strategyFiles.empty()) {
            spdlog::error("No strategy files added for batch backtest");
            throw std::runtime_error("No strategy files added for batch backtest");
        }
        
        spdlog::info("Starting batch backtest with {} strategies", m_strategyFiles.size());
        
        // Clear previous results
        m_results = BatchBacktestResults();
        m_results.strategyNames.reserve(m_strategyFiles.size());
        
        // Determine number of threads to use
        unsigned int numThreads = std::thread::hardware_concurrency();
        if (numThreads == 0) numThreads = 4; // Fallback if hardware_concurrency fails
        
        spdlog::info("Using {} threads for parallel processing", numThreads);
        
        // Create thread pool
        std::vector<std::future<void>> futures;
        std::mutex resultsMutex;
        std::atomic<int> completedTests{0};
        
        // Process strategies in batches to manage memory
        const size_t batchSize = 10;
        for (size_t i = 0; i < m_strategyFiles.size(); i += batchSize) {
            size_t endIdx = std::min(i + batchSize, m_strategyFiles.size());
            
            for (size_t j = i; j < endIdx; ++j) {
                futures.push_back(std::async(std::launch::async, [&, j]() {
                    try {
                        const std::string& filePath = m_strategyFiles[j];
                        std::string strategyName = std::filesystem::path(filePath).stem().string();
                        
                        spdlog::info("Processing strategy: {}", strategyName);
                        
                        Backtester backtester;
                        backtester.setConfig(m_commonConfig);
                        backtester.loadData(filePath);
                        BacktestResult result = backtester.runBacktest();
                        
                        // Generate equity curve image
                        std::string imagePath = generateEquityCurveImage(strategyName, result);
                        
                        // Update results thread-safely
                        {
                            std::lock_guard<std::mutex> lock(resultsMutex);
                            m_results.strategyNames.push_back(strategyName);
                            m_results.results[strategyName] = result;
                            m_results.equityCurveImages[strategyName] = imagePath;
                        }
                        
                        completedTests++;
                        spdlog::info("Completed strategy {} ({}/{})", 
                                    strategyName, completedTests.load(), m_strategyFiles.size());
                        
                    } catch (const std::exception& e) {
                        spdlog::error("Error processing strategy {}: {}", 
                                    m_strategyFiles[j], e.what());
                    }
                }));
                
                // Wait if we've reached the thread limit
                if (futures.size() >= numThreads) {
                    for (auto& future : futures) {
                        future.wait();
                    }
                    futures.clear();
                }
            }
        }
        
        // Wait for remaining tasks
        for (auto& future : futures) {
            future.wait();
        }
        
        // Calculate aggregate statistics
        calculateAggregateStats();
        
        spdlog::info("Batch backtest completed successfully");
        return m_results;
    }
    
    bool BatchBacktester::exportSummaryReport(const std::string& filename) const {
        try {
            std::ofstream file(filename);
            if (!file.is_open()) {
                spdlog::error("Failed to open file for writing: {}", filename);
                return false;
            }
            
            file << "# Batch Backtest Summary Report\n\n";
            file << "## Aggregate Statistics\n";
            file << "- Average Win Rate: " << std::fixed << std::setprecision(2) 
                 << (m_results.averageWinRate * 100) << "%\n";
            file << "- Average Profit Factor: " << std::fixed << std::setprecision(2) 
                 << m_results.averageProfitFactor << "\n";
            file << "- Average Max Drawdown: " << std::fixed << std::setprecision(2) 
                 << (m_results.averageMaxDrawdown * 100) << "%\n";
            file << "- Best Strategy: " << m_results.bestStrategy << "\n";
            file << "- Worst Strategy: " << m_results.worstStrategy << "\n\n";
            
            file << "## Strategy Rankings\n";
            std::vector<std::pair<std::string, double>> rankings;
            for (const auto& [name, result] : m_results.results) {
                rankings.emplace_back(name, result.totalReturn);
            }
            
            std::sort(rankings.begin(), rankings.end(),
                     [](const auto& a, const auto& b) { return a.second > b.second; });
            
            for (size_t i = 0; i < rankings.size(); ++i) {
                file << (i + 1) << ". " << rankings[i].first 
                     << " (Return: " << std::fixed << std::setprecision(2) 
                     << (rankings[i].second * 100) << "%)\n";
            }
            
            spdlog::info("Exported summary report to: {}", filename);
            return true;
            
        } catch (const std::exception& e) {
            spdlog::error("Error exporting summary report: {}", e.what());
            return false;
        }
    }
    
    bool BatchBacktester::exportDetailedReport(const std::string& filename) const {
        try {
            std::ofstream file(filename);
            if (!file.is_open()) {
                spdlog::error("Failed to open file for writing: {}", filename);
                return false;
            }
            
            file << "# Detailed Batch Backtest Report\n\n";
            file << generateMarkdownReport();
            
            spdlog::info("Exported detailed report to: {}", filename);
            return true;
            
        } catch (const std::exception& e) {
            spdlog::error("Error exporting detailed report: {}", e.what());
            return false;
        }
    }
    
    std::string BatchBacktester::generateMarkdownReport() const {
        std::stringstream ss;
        
        // Add aggregate statistics section
        ss << "## Aggregate Statistics\n\n";
        ss << "- Average Win Rate: " << std::fixed << std::setprecision(2) 
           << (m_results.averageWinRate * 100) << "%\n";
        ss << "- Average Profit Factor: " << std::fixed << std::setprecision(2) 
           << m_results.averageProfitFactor << "\n";
        ss << "- Average Max Drawdown: " << std::fixed << std::setprecision(2) 
           << (m_results.averageMaxDrawdown * 100) << "%\n";
        ss << "- Best Strategy: " << m_results.bestStrategy << "\n";
        ss << "- Worst Strategy: " << m_results.worstStrategy << "\n\n";
        
        // Add individual strategy sections
        for (const auto& strategyName : m_results.strategyNames) {
            const auto& result = m_results.results.at(strategyName);
            ss << generateStrategySection(strategyName, result);
        }
        
        return ss.str();
    }
    
    std::string BatchBacktester::generateStrategySection(const std::string& strategyName, 
                                                        const BacktestResult& result) const {
        std::stringstream ss;
        
        ss << "## Strategy: " << strategyName << "\n\n";
        ss << "### Performance Metrics\n";
        ss << "- Total Return: " << std::fixed << std::setprecision(2) 
           << (result.totalReturn * 100) << "%\n";
        ss << "- Win Rate: " << std::fixed << std::setprecision(2) 
           << (result.winRate * 100) << "%\n";
        ss << "- Profit Factor: " << std::fixed << std::setprecision(2) 
           << result.profitFactor << "\n";
        ss << "- Max Drawdown: " << std::fixed << std::setprecision(2) 
           << (result.maxDrawdown * 100) << "%\n";
        ss << "- Sharpe Ratio: " << std::fixed << std::setprecision(2) 
           << result.sharpeRatio << "\n";
        ss << "- Number of Trades: " << result.numTrades << "\n\n";
        
        // Add equity curve image if available
        auto it = m_results.equityCurveImages.find(strategyName);
        if (it != m_results.equityCurveImages.end()) {
            ss << "### Equity Curve\n";
            ss << "![Equity Curve](" << it->second << ")\n\n";
        }
        
        return ss.str();
    }
    
    void BatchBacktester::clearStrategyFiles() {
        m_strategyFiles.clear();
        m_results = BatchBacktestResults();
        spdlog::info("Cleared strategy files and results");
    }
    
    const BatchBacktestResults& BatchBacktester::getResults() const {
        return m_results;
    }
    
    void BatchBacktester::calculateAggregateStats() {
        if (m_results.results.empty()) {
            spdlog::warn("No results available for aggregate statistics");
            return;
        }
        
        double totalWinRate = 0.0;
        double totalProfitFactor = 0.0;
        double totalMaxDrawdown = 0.0;
        double bestReturn = -std::numeric_limits<double>::infinity();
        double worstReturn = std::numeric_limits<double>::infinity();
        
        for (const auto& [strategyName, result] : m_results.results) {
            totalWinRate += result.winRate;
            totalProfitFactor += result.profitFactor;
            totalMaxDrawdown += result.maxDrawdown;
            
            if (result.totalReturn > bestReturn) {
                bestReturn = result.totalReturn;
                m_results.bestStrategy = strategyName;
            }
            if (result.totalReturn < worstReturn) {
                worstReturn = result.totalReturn;
                m_results.worstStrategy = strategyName;
            }
        }
        
        size_t numStrategies = m_results.results.size();
        m_results.averageWinRate = totalWinRate / numStrategies;
        m_results.averageProfitFactor = totalProfitFactor / numStrategies;
        m_results.averageMaxDrawdown = totalMaxDrawdown / numStrategies;
        
        spdlog::info("Calculated aggregate statistics across {} strategies", numStrategies);
    }
    
    std::string BatchBacktester::generateEquityCurveImage(const std::string& strategyName, 
                                                        const BacktestResult& result) {
        // TODO: Implement actual image generation using a plotting library
        // For now, return a placeholder path
        return "equity_curves/" + strategyName + ".png";
    }
} 