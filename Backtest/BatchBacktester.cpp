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
#include <matplot/matplot.h>
#include "EquityCurveGenerator.h"

// Platform-specific memory tracking
#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#elif defined(__unix__) || defined(__APPLE__)
#include <sys/resource.h>
#include <unistd.h>
#endif

namespace Backtest {

// JSON conversion functions for BatchConfig
void to_json(nlohmann::json& j, const BatchConfig& config) {
    j = nlohmann::json{
        {"performance", {
            {"thread_count", config.threadCount},
            {"batch_size", config.batchSize},
            {"memory_limit_mb", config.memoryLimitMB}
        }},
        {"output", {
            {"formats", config.outputFormats},
            {"chart_format", config.chartFormat},
            {"chart_width", config.chartWidth},
            {"chart_height", config.chartHeight},
            {"chart_dpi", config.chartDPI},
            {"include_charts_in_report", config.includeChartsInReport}
        }},
        {"paths", {
            {"strategy_dir", config.strategyDir},
            {"output_dir", config.outputDir},
            {"chart_dir", config.chartDir}
        }},
        {"logging", {
            {"level", config.logLevel},
            {"file", config.logFile},
            {"console", config.consoleOutput},
            {"performance_metrics", config.trackPerformance}
        }}
    };
}

void from_json(const nlohmann::json& j, BatchConfig& config) {
    // Performance settings
    if (j.contains("performance")) {
        const auto& perf = j["performance"];
        if (perf.contains("thread_count")) config.threadCount = perf["thread_count"];
        if (perf.contains("batch_size")) config.batchSize = perf["batch_size"];
        if (perf.contains("memory_limit_mb")) config.memoryLimitMB = perf["memory_limit_mb"];
    }
    
    // Output settings
    if (j.contains("output")) {
        const auto& output = j["output"];
        if (output.contains("formats")) config.outputFormats = output["formats"].get<std::vector<std::string>>();
        if (output.contains("chart_format")) config.chartFormat = output["chart_format"];
        if (output.contains("chart_width")) config.chartWidth = output["chart_width"];
        if (output.contains("chart_height")) config.chartHeight = output["chart_height"];
        if (output.contains("chart_dpi")) config.chartDPI = output["chart_dpi"];
        if (output.contains("include_charts_in_report")) config.includeChartsInReport = output["include_charts_in_report"];
    }
    
    // Path settings
    if (j.contains("paths")) {
        const auto& paths = j["paths"];
        if (paths.contains("strategy_dir")) config.strategyDir = paths["strategy_dir"];
        if (paths.contains("output_dir")) config.outputDir = paths["output_dir"];
        if (paths.contains("chart_dir")) config.chartDir = paths["chart_dir"];
    }
    
    // Logging settings
    if (j.contains("logging")) {
        const auto& logging = j["logging"];
        if (logging.contains("level")) config.logLevel = logging["level"];
        if (logging.contains("file")) config.logFile = logging["file"];
        if (logging.contains("console")) config.consoleOutput = logging["console"];
        if (logging.contains("performance_metrics")) config.trackPerformance = logging["performance_metrics"];
    }
    
    // Backtest settings
    if (j.contains("backtest")) {
        const auto& backtest = j["backtest"];
        if (backtest.contains("initial_capital")) config.backtestConfig.initialBalance = backtest["initial_capital"];
        if (backtest.contains("risk_per_trade")) config.backtestConfig.riskPerTrade = backtest["risk_per_trade"];
        if (backtest.contains("commission")) config.backtestConfig.commission = backtest["commission"];
        if (backtest.contains("slippage")) config.backtestConfig.slippage = backtest["slippage"];
    }
}

BatchBacktester::BatchBacktester() {
    spdlog::info("Initializing BatchBacktester");
    
    // Initialize with some reasonable defaults
    m_commonConfig.initialBalance = 10000.0;
    m_commonConfig.riskPerTrade = 1.0;
    m_commonConfig.riskRewardRatio = 2.0;
    m_commonConfig.strategyType = StrategyType::FIXED_RR;
    
    // Initialize batch config with defaults
    applyDefaultConfig();
}

void BatchBacktester::applyDefaultConfig() {
    // Performance defaults
    if (m_batchConfig.threadCount == 0) {
        m_batchConfig.threadCount = std::thread::hardware_concurrency();
        if (m_batchConfig.threadCount == 0) m_batchConfig.threadCount = 4; // Fallback
    }
    
    // Ensure output directories exist
    ensureDirectoriesExist();
    
    // Configure logging
    if (m_batchConfig.logLevel == "debug") {
        spdlog::set_level(spdlog::level::debug);
    } else if (m_batchConfig.logLevel == "info") {
        spdlog::set_level(spdlog::level::info);
    } else if (m_batchConfig.logLevel == "warn") {
        spdlog::set_level(spdlog::level::warn);
    } else if (m_batchConfig.logLevel == "error") {
        spdlog::set_level(spdlog::level::err);
    }
    
    spdlog::debug("Applied default configuration");
}

void BatchBacktester::ensureDirectoriesExist() const {
    try {
        // Create output directories if they don't exist
        std::filesystem::create_directories(m_batchConfig.outputDir);
        std::filesystem::create_directories(m_batchConfig.chartDir);
        
        // Create log directory if configured
        if (!m_batchConfig.logFile.empty()) {
            std::filesystem::path logPath(m_batchConfig.logFile);
            std::filesystem::create_directories(logPath.parent_path());
        }
    } catch (const std::exception& e) {
        spdlog::error("Error creating directories: {}", e.what());
    }
}

bool BatchBacktester::loadBatchConfig(const std::string& configFile) {
    try {
        // Check if config file exists
        if (!std::filesystem::exists(configFile)) {
            spdlog::error("Config file not found: {}", configFile);
            return false;
        }
        
        // Read and parse JSON file
        std::ifstream file(configFile);
        nlohmann::json j;
        file >> j;
        
        // Convert JSON to BatchConfig
        m_batchConfig = j.get<BatchConfig>();
        
        // Apply common backtest config if specified
        if (j.contains("backtest")) {
            m_commonConfig = m_batchConfig.backtestConfig;
        }
        
        spdlog::info("Loaded configuration from {}", configFile);
        applyDefaultConfig();
        return true;
        
    } catch (const std::exception& e) {
        spdlog::error("Error loading config file: {}", e.what());
        return false;
    }
}

void BatchBacktester::setBatchConfig(const BatchConfig& config) {
    m_batchConfig = config;
    applyDefaultConfig();
    spdlog::info("Set batch configuration");
}

const BatchConfig& BatchBacktester::getBatchConfig() const {
    return m_batchConfig;
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
    if (config.initialBalance <= 0) {
        throw std::invalid_argument("Initial capital must be positive");
    }
    if (config.commission < 0) {
        throw std::invalid_argument("Commission cannot be negative");
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
    
    // Start timing for total duration
    auto startTime = std::chrono::high_resolution_clock::now();
    size_t initialMemory = getCurrentMemoryUsage();
    
    spdlog::info("Starting batch backtest with {} strategies", m_strategyFiles.size());
    
    // Clear previous results
    m_results = BatchBacktestResults();
    m_results.strategyNames.reserve(m_strategyFiles.size());
    
    // Determine number of threads to use
    unsigned int numThreads = m_batchConfig.threadCount;
    if (numThreads == 0) {
        numThreads = std::thread::hardware_concurrency();
        if (numThreads == 0) numThreads = 4; // Fallback if hardware_concurrency fails
    }
    
    spdlog::info("Using {} threads for parallel processing", numThreads);
    
    // Create thread pool
    std::vector<std::future<void>> futures;
    std::mutex resultsMutex;
    std::atomic<int> completedTests{0};
    
    // Track strategy durations for performance metrics
    std::map<std::string, std::chrono::milliseconds> strategyDurations;
    std::mutex durationMutex;
    
    // Track peak memory usage
    std::atomic<size_t> peakMemoryUsage{initialMemory};
    
    // Process strategies in batches to manage memory
    const size_t batchSize = m_batchConfig.batchSize;
    size_t batchesProcessed = 0;
    
    for (size_t i = 0; i < m_strategyFiles.size(); i += batchSize) {
        size_t endIdx = std::min(i + batchSize, m_strategyFiles.size());
        batchesProcessed++;
        
        spdlog::info("Processing batch {}/{} (strategies {}-{})", 
                   batchesProcessed, 
                   (m_strategyFiles.size() + batchSize - 1) / batchSize,
                   i + 1, endIdx);
        
        for (size_t j = i; j < endIdx; ++j) {
            futures.push_back(std::async(std::launch::async, [&, j]() {
                try {
                    const std::string& filePath = m_strategyFiles[j];
                    std::string strategyName = std::filesystem::path(filePath).stem().string();
                    
                    spdlog::info("Processing strategy: {}", strategyName);
                    
                    // Start timing for strategy duration
                    auto strategyStartTime = std::chrono::high_resolution_clock::now();
                    
                    // Run backtest
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
                    
                    // Record strategy duration
                    auto strategyEndTime = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        strategyEndTime - strategyStartTime);
                    
                    {
                        std::lock_guard<std::mutex> lock(durationMutex);
                        strategyDurations[strategyName] = duration;
                    }
                    
                    // Update memory usage if tracking is enabled
                    if (m_batchConfig.trackPerformance) {
                        size_t currentMemory = getCurrentMemoryUsage();
                        size_t expected = peakMemoryUsage.load();
                        while (currentMemory > expected && 
                               !peakMemoryUsage.compare_exchange_weak(expected, currentMemory)) {
                            // Keep trying until we succeed or another thread sets a higher value
                        }
                    }
                    
                    completedTests++;
                    spdlog::info("Completed strategy {} ({}/{}) in {:.2f} seconds", 
                               strategyName, completedTests.load(), m_strategyFiles.size(),
                               duration.count() / 1000.0);
                    
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
    
    // Calculate performance metrics
    auto endTime = std::chrono::high_resolution_clock::now();
    auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    m_results.performance.totalDuration = totalDuration;
    m_results.performance.peakMemoryUsageMB = peakMemoryUsage.load() / (1024 * 1024);
    m_results.performance.batchesProcessed = batchesProcessed;
    
    // Calculate average and max strategy durations
    if (!strategyDurations.empty()) {
        auto maxDurationIt = std::max_element(
            strategyDurations.begin(), strategyDurations.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });
        
        std::chrono::milliseconds totalStrategyDuration{0};
        for (const auto& [_, duration] : strategyDurations) {
            totalStrategyDuration += duration;
        }
        
        m_results.performance.avgStrategyDuration = 
            std::chrono::milliseconds(totalStrategyDuration.count() / strategyDurations.size());
        m_results.performance.maxStrategyDuration = maxDurationIt->second;
        m_results.performance.slowestStrategy = maxDurationIt->first;
    }
    
    spdlog::info("Batch backtest completed in {:.2f} seconds", totalDuration.count() / 1000.0);
    
    if (m_batchConfig.trackPerformance) {
        spdlog::info("Performance metrics:");
        spdlog::info("  Total duration: {:.2f} seconds", totalDuration.count() / 1000.0);
        spdlog::info("  Average strategy duration: {:.2f} seconds", 
                   m_results.performance.avgStrategyDuration.count() / 1000.0);
        spdlog::info("  Max strategy duration: {:.2f} seconds ({})", 
                   m_results.performance.maxStrategyDuration.count() / 1000.0,
                   m_results.performance.slowestStrategy);
        spdlog::info("  Peak memory usage: {} MB", m_results.performance.peakMemoryUsageMB);
    }
    
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
    try {
        // Create chart config from batch config
        ChartConfig chartConfig;
        chartConfig.format = m_batchConfig.chartFormat;
        chartConfig.width = m_batchConfig.chartWidth;
        chartConfig.height = m_batchConfig.chartHeight;
        chartConfig.dpi = m_batchConfig.chartDPI;
        chartConfig.outputDir = m_batchConfig.chartDir;
        
        // Create equity curve generator
        EquityCurveGenerator generator(chartConfig);
        
        // Generate equity curve
        std::string equityCurvePath = generator.generateEquityCurve(strategyName, result);
        
        // Generate drawdown chart if data available
        if (!result.drawdowns.empty()) {
            generator.generateDrawdownChart(strategyName, result);
        }
        
        // Generate monthly returns heatmap if data available
        if (!result.monthlyReturns.empty()) {
            generator.generateMonthlyReturnsHeatmap(strategyName, result);
        }
        
        return equityCurvePath;
    } catch (const std::exception& e) {
        spdlog::error("Error generating charts for {}: {}", strategyName, e.what());
        return "";
    }
}

size_t BatchBacktester::getCurrentMemoryUsage() const {
#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        return static_cast<size_t>(pmc.WorkingSetSize);
    }
#elif defined(__unix__) || defined(__APPLE__)
    struct rusage rusage;
    if (getrusage(RUSAGE_SELF, &rusage) == 0) {
        return static_cast<size_t>(rusage.ru_maxrss * 1024);
    }
#endif
    return 0; // Fallback if platform-specific code fails
}

bool BatchBacktester::exportJsonReport(const std::string& filename) const {
    try {
        // Create the JSON structure
        nlohmann::json j;
        
        // Add aggregate statistics
        j["aggregate_stats"] = {
            {"avg_win_rate", m_results.averageWinRate},
            {"avg_profit_factor", m_results.averageProfitFactor},
            {"avg_max_drawdown", m_results.averageMaxDrawdown},
            {"best_strategy", m_results.bestStrategy},
            {"worst_strategy", m_results.worstStrategy}
        };
        
        // Add performance metrics
        j["performance"] = {
            {"total_duration_ms", m_results.performance.totalDuration.count()},
            {"avg_strategy_duration_ms", m_results.performance.avgStrategyDuration.count()},
            {"max_strategy_duration_ms", m_results.performance.maxStrategyDuration.count()},
            {"slowest_strategy", m_results.performance.slowestStrategy},
            {"peak_memory_mb", m_results.performance.peakMemoryUsageMB},
            {"batches_processed", m_results.performance.batchesProcessed}
        };
        
        // Add individual strategy results
        j["strategies"] = nlohmann::json::array();
        for (const auto& stratName : m_results.strategyNames) {
            const auto& result = m_results.results.at(stratName);
            
            nlohmann::json strategyJson = {
                {"name", stratName},
                {"total_return", result.totalReturn},
                {"win_rate", result.winRate},
                {"profit_factor", result.profitFactor},
                {"max_drawdown", result.maxDrawdown},
                {"sharpe_ratio", result.sharpeRatio},
                {"num_trades", result.numTrades}
            };
            
            // Add equity curve image if available
            auto it = m_results.equityCurveImages.find(stratName);
            if (it != m_results.equityCurveImages.end()) {
                strategyJson["equity_curve_image"] = it->second;
            }
            
            j["strategies"].push_back(strategyJson);
        }
        
        // Write to file
        std::ofstream file(filename);
        if (!file.is_open()) {
            spdlog::error("Failed to open file for writing: {}", filename);
            return false;
        }
        
        file << std::setw(4) << j << std::endl;
        spdlog::info("Exported JSON report to: {}", filename);
        return true;
        
    } catch (const std::exception& e) {
        spdlog::error("Error exporting JSON report: {}", e.what());
        return false;
    }
}

bool BatchBacktester::exportCsvReport(const std::string& filename) const {
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            spdlog::error("Failed to open file for writing: {}", filename);
            return false;
        }
        
        // Write CSV header
        file << "Strategy,Total Return (%),Win Rate (%),Profit Factor,Max Drawdown (%),"
             << "Sharpe Ratio,Number of Trades,Equity Curve Image\n";
        
        // Write strategy data rows
        for (const auto& stratName : m_results.strategyNames) {
            const auto& result = m_results.results.at(stratName);
            
            file << stratName << ","
                 << (result.totalReturn * 100) << ","
                 << (result.winRate * 100) << ","
                 << result.profitFactor << ","
                 << (result.maxDrawdown * 100) << ","
                 << result.sharpeRatio << ","
                 << result.numTrades << ",";
            
            // Add equity curve image path if available
            auto it = m_results.equityCurveImages.find(stratName);
            if (it != m_results.equityCurveImages.end()) {
                file << it->second;
            }
            file << "\n";
        }
        
        // Write aggregate statistics
        file << "\nAggregate Statistics\n";
        file << "Average Win Rate (%)," << (m_results.averageWinRate * 100) << "\n";
        file << "Average Profit Factor," << m_results.averageProfitFactor << "\n";
        file << "Average Max Drawdown (%)," << (m_results.averageMaxDrawdown * 100) << "\n";
        file << "Best Strategy," << m_results.bestStrategy << "\n";
        file << "Worst Strategy," << m_results.worstStrategy << "\n";
        
        // Write performance metrics
        file << "\nPerformance Metrics\n";
        file << "Total Duration (s)," << (m_results.performance.totalDuration.count() / 1000.0) << "\n";
        file << "Average Strategy Duration (s)," << (m_results.performance.avgStrategyDuration.count() / 1000.0) << "\n";
        file << "Max Strategy Duration (s)," << (m_results.performance.maxStrategyDuration.count() / 1000.0) << "\n";
        file << "Slowest Strategy," << m_results.performance.slowestStrategy << "\n";
        file << "Peak Memory Usage (MB)," << m_results.performance.peakMemoryUsageMB << "\n";
        file << "Batches Processed," << m_results.performance.batchesProcessed << "\n";
        
        spdlog::info("Exported CSV report to: {}", filename);
        return true;
        
    } catch (const std::exception& e) {
        spdlog::error("Error exporting CSV report: {}", e.what());
        return false;
    }
}
} 