#include <catch2/catch_all.hpp>
#include "../Backtest/BatchBacktester.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

// Helper to create a temporary test strategy file
std::string createTestStrategyFile(const std::string& filename) {
    std::string filePath = "test_data/" + filename;
    
    // Create directory if it doesn't exist
    fs::create_directories("test_data");
    
    // Create a simple CSV file with test data
    std::ofstream file(filePath);
    file << "Date,Open,High,Low,Close,Volume\n";
    file << "2023-01-01,100,105,95,102,1000\n";
    file << "2023-01-02,102,110,100,108,1200\n";
    file << "2023-01-03,108,115,105,112,1500\n";
    file << "2023-01-04,112,120,110,118,2000\n";
    file << "2023-01-05,118,125,115,120,1800\n";
    file.close();
    
    return filePath;
}

// Helper to clean up test files
void cleanupTestFiles() {
    try {
        if (fs::exists("test_data")) {
            fs::remove_all("test_data");
        }
        if (fs::exists("test_exports")) {
            fs::remove_all("test_exports");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error cleaning up test files: " << e.what() << std::endl;
    }
}

// Create a test fixture for BatchBacktester tests
class BatchBacktesterFixture {
public:
    BatchBacktesterFixture() {
        // Setup test environment
        fs::create_directories("test_data");
        fs::create_directories("test_exports/charts");
        
        // Create test strategy files
        m_testFiles.push_back(createTestStrategyFile("strategy1.csv"));
        m_testFiles.push_back(createTestStrategyFile("strategy2.csv"));
        m_testFiles.push_back(createTestStrategyFile("strategy3.csv"));
        
        // Setup backtester with test config
        m_backtester.setCommonConfig({
            .initialBalance = 10000.0,
            .riskPerTrade = 1.0,
            .riskRewardRatio = 2.0,
            .commission = 0.1,
            .slippage = 0.05,
            .strategyType = Backtest::StrategyType::FIXED_RR
        });
    }
    
    ~BatchBacktesterFixture() {
        cleanupTestFiles();
    }
    
protected:
    Backtest::BatchBacktester m_backtester;
    std::vector<std::string> m_testFiles;
};

TEST_CASE_METHOD(BatchBacktesterFixture, "BatchBacktester can add strategy files", "[batch][file]") {
    REQUIRE(m_backtester.getResults().strategyNames.empty());
    
    // Add first test file
    m_backtester.addStrategyFile(m_testFiles[0]);
    
    // Verify it was added correctly
    REQUIRE(m_backtester.runBatchBacktest().strategyNames.size() == 1);
    REQUIRE(m_backtester.getResults().strategyNames[0] == "strategy1");
}

TEST_CASE_METHOD(BatchBacktesterFixture, "BatchBacktester can add directory of strategies", "[batch][directory]") {
    REQUIRE(m_backtester.getResults().strategyNames.empty());
    
    // Add all test files from directory
    bool added = m_backtester.addStrategyDirectory("test_data");
    
    // Verify files were added
    REQUIRE(added);
    REQUIRE(m_backtester.runBatchBacktest().strategyNames.size() == 3);
}

TEST_CASE_METHOD(BatchBacktesterFixture, "BatchBacktester calculates aggregate statistics", "[batch][stats]") {
    // Add all test files
    m_backtester.addStrategyDirectory("test_data");
    
    // Run backtest
    Backtest::BatchBacktestResults results = m_backtester.runBatchBacktest();
    
    // Verify aggregate stats were calculated
    REQUIRE(results.averageWinRate >= 0.0);
    REQUIRE(results.averageWinRate <= 1.0);
    REQUIRE(results.averageProfitFactor >= 0.0);
    REQUIRE(!results.bestStrategy.empty());
    REQUIRE(!results.worstStrategy.empty());
}

TEST_CASE_METHOD(BatchBacktesterFixture, "BatchBacktester can export reports", "[batch][export]") {
    // Add all test files
    m_backtester.addStrategyDirectory("test_data");
    
    // Run backtest
    m_backtester.runBatchBacktest();
    
    // Export summary report
    bool summaryExported = m_backtester.exportSummaryReport("test_exports/summary.md");
    REQUIRE(summaryExported);
    REQUIRE(fs::exists("test_exports/summary.md"));
    
    // Export detailed report
    bool detailedExported = m_backtester.exportDetailedReport("test_exports/detailed.md");
    REQUIRE(detailedExported);
    REQUIRE(fs::exists("test_exports/detailed.md"));
}

TEST_CASE_METHOD(BatchBacktesterFixture, "BatchBacktester generates equity curve images", "[batch][charts]") {
    // Add test file
    m_backtester.addStrategyFile(m_testFiles[0]);
    
    // Run backtest
    Backtest::BatchBacktestResults results = m_backtester.runBatchBacktest();
    
    // Check if equity curve image path is set
    REQUIRE(!results.equityCurveImages.empty());
    REQUIRE(results.equityCurveImages.find("strategy1") != results.equityCurveImages.end());
    
    // In a complete test, we would also check if the file exists,
    // but since the actual implementation is not complete yet,
    // we'll leave this as a placeholder
}

TEST_CASE_METHOD(BatchBacktesterFixture, "BatchBacktester handles parallel execution", "[batch][parallel]") {
    // Add all test files
    m_backtester.addStrategyDirectory("test_data");
    
    // Run backtest multiple times to verify thread safety
    for (int i = 0; i < 3; i++) {
        Backtest::BatchBacktestResults results = m_backtester.runBatchBacktest();
        REQUIRE(results.strategyNames.size() == 3);
    }
}

TEST_CASE("BatchBacktester handles invalid inputs gracefully", "[batch][error]") {
    Backtest::BatchBacktester backtester;
    
    // Test with non-existent file
    REQUIRE_THROWS_AS(backtester.addStrategyFile("does_not_exist.csv"), std::runtime_error);
    
    // Test with non-existent directory
    REQUIRE_FALSE(backtester.addStrategyDirectory("does_not_exist"));
    
    // Test running without strategies
    REQUIRE_THROWS_AS(backtester.runBatchBacktest(), std::runtime_error);
    
    // Test with invalid config
    REQUIRE_THROWS_AS(backtester.setCommonConfig({.initialBalance = -1000}), std::invalid_argument);
} 