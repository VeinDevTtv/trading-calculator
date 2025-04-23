#include "SessionManager.h"
#include "Utils.h"
#include <algorithm>
#include <numeric>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <stdexcept>
#include <random>
#include <chrono>

SessionManager::SessionManager() : 
    m_sessionActive(false),
    m_initialBalance(0.0),
    m_currentBalance(0.0),
    m_autoSave(false),
    m_sessionFile("trading_session.csv")
{
    generateSessionId();
}

SessionManager::~SessionManager() {
    if (m_sessionActive && m_autoSave) {
        saveSession();
    }
}

void SessionManager::startNewSession(double initialBalance) {
    if (initialBalance <= 0) {
        throw std::invalid_argument("Initial balance must be positive");
    }
    
    // If there's an active session, save it
    if (m_sessionActive && m_autoSave) {
        saveSession();
    }
    
    // Clear previous session data
    m_trades.clear();
    m_initialBalance = initialBalance;
    m_currentBalance = initialBalance;
    m_sessionActive = true;
    generateSessionId();
}

void SessionManager::endSession() {
    if (m_sessionActive && m_autoSave) {
        saveSession();
    }
    m_sessionActive = false;
}

bool SessionManager::isSessionActive() const {
    return m_sessionActive;
}

double SessionManager::getCurrentBalance() const {
    return m_currentBalance;
}

std::shared_ptr<Trade> SessionManager::createTrade() {
    if (!m_sessionActive) {
        throw std::runtime_error("No active session. Start a session first.");
    }
    
    auto trade = std::make_shared<Trade>();
    trade->setAccountBalance(m_currentBalance);
    return trade;
}

bool SessionManager::addTrade(std::shared_ptr<Trade> trade) {
    if (!m_sessionActive) {
        return false;
    }
    
    if (!trade->validate()) {
        return false;
    }
    
    m_trades.push_back(trade);
    
    // If outcome is set, update the balance
    if (trade->getOutcome() != TradeOutcome::Pending) {
        updateBalance(trade);
    }
    
    // Auto-save if enabled
    if (m_autoSave) {
        trade->save(m_sessionFile);
    }
    
    return true;
}

bool SessionManager::simulateTrade(std::shared_ptr<Trade> trade, TradeOutcome outcome) {
    if (!m_sessionActive || !trade) {
        return false;
    }
    
    trade->simulateOutcome(outcome);
    updateBalance(trade);
    
    // Auto-save if enabled
    if (m_autoSave) {
        trade->save(m_sessionFile);
    }
    
    return true;
}

std::shared_ptr<Trade> SessionManager::getTrade(const std::string& id) const {
    auto it = std::find_if(m_trades.begin(), m_trades.end(), 
        [&id](const std::shared_ptr<Trade>& trade) {
            return trade->getId() == id;
        });
    
    if (it != m_trades.end()) {
        return *it;
    }
    
    return nullptr;
}

std::shared_ptr<Trade> SessionManager::getLastTrade() const {
    if (m_trades.empty()) {
        return nullptr;
    }
    
    return m_trades.back();
}

std::vector<std::shared_ptr<Trade>> SessionManager::getAllTrades() const {
    return m_trades;
}

SessionStats SessionManager::getSessionStats() const {
    SessionStats stats;
    calculateStats(stats);
    return stats;
}

std::string SessionManager::getSessionSummary() const {
    SessionStats stats = getSessionStats();
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    
    ss << "\n=== SESSION SUMMARY ===\n";
    ss << "Session ID: " << m_sessionId << "\n";
    ss << "Total Trades: " << stats.totalTrades << "\n";
    ss << "Initial Balance: $" << stats.initialBalance << "\n";
    ss << "Current Balance: $" << stats.currentBalance << "\n";
    
    if (stats.totalTrades > 0) {
        double pnlPercent = (stats.totalPnL / stats.initialBalance) * 100.0;
        ss << "Total P&L: " << (stats.totalPnL >= 0 ? "+" : "") << "$" << stats.totalPnL 
           << " (" << (stats.totalPnL >= 0 ? "+" : "") << pnlPercent << "%)\n";
        
        ss << "Win Rate: " << (stats.winRate * 100.0) << "%\n";
        ss << "Winning Trades: " << stats.winningTrades << "\n";
        ss << "Losing Trades: " << stats.losingTrades << "\n";
        if (stats.breakEvenTrades > 0) {
            ss << "Break-Even Trades: " << stats.breakEvenTrades << "\n";
        }
        
        ss << "Average RR: 1:" << stats.averageRR << "\n";
        
        if (stats.largestWin > 0) {
            ss << "Largest Win: $" << stats.largestWin << "\n";
        }
        
        if (stats.largestLoss < 0) {
            ss << "Largest Loss: $" << stats.largestLoss << "\n";
        }
        
        if (stats.losingTrades > 0) {
            ss << "Profit Factor: " << stats.profitFactor << "\n";
        }
    }
    
    return ss.str();
}

bool SessionManager::saveSession(const std::string& filename) {
    std::string targetFile = filename.empty() ? m_sessionFile : filename;
    
    // Check if we have any trades
    if (m_trades.empty()) {
        std::cerr << "No trades to save." << std::endl;
        return false;
    }
    
    // First trade will create the file and header
    bool result = m_trades.front()->save(targetFile, false);
    
    // Add remaining trades
    for (size_t i = 1; i < m_trades.size(); i++) {
        result = result && m_trades[i]->save(targetFile, true);
    }
    
    return result;
}

bool SessionManager::saveSessionAsJson(const std::string& filename) {
    std::string targetFile = filename.empty() ? 
        Utils::replaceExtension(m_sessionFile, ".json") : filename;
    
    if (m_trades.empty()) {
        std::cerr << "No trades to save." << std::endl;
        return false;
    }
    
    SessionStats stats = getSessionStats();
    
    std::ofstream file(targetFile);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << targetFile << " for writing." << std::endl;
        return false;
    }
    
    // Start JSON
    file << "{\n";
    
    // Session info
    file << "  \"sessionId\": \"" << m_sessionId << "\",\n";
    file << "  \"timestamp\": \"" << Utils::getFormattedTimestamp(std::time(nullptr)) << "\",\n";
    file << "  \"initialBalance\": " << stats.initialBalance << ",\n";
    file << "  \"currentBalance\": " << stats.currentBalance << ",\n";
    file << "  \"totalPnL\": " << stats.totalPnL << ",\n";
    file << "  \"totalTrades\": " << stats.totalTrades << ",\n";
    file << "  \"winningTrades\": " << stats.winningTrades << ",\n";
    file << "  \"losingTrades\": " << stats.losingTrades << ",\n";
    file << "  \"breakEvenTrades\": " << stats.breakEvenTrades << ",\n";
    file << "  \"winRate\": " << stats.winRate << ",\n";
    file << "  \"averageRR\": " << stats.averageRR << ",\n";
    file << "  \"largestWin\": " << stats.largestWin << ",\n";
    file << "  \"largestLoss\": " << stats.largestLoss << ",\n";
    file << "  \"profitFactor\": " << stats.profitFactor << ",\n";
    
    // Trades array
    file << "  \"trades\": [\n";
    
    for (size_t i = 0; i < m_trades.size(); i++) {
        const auto& trade = m_trades[i];
        auto params = trade->getParameters();
        auto results = trade->getResults();
        
        file << "    {\n";
        file << "      \"id\": \"" << trade->getId() << "\",\n";
        file << "      \"timestamp\": \"" << Utils::getFormattedTimestamp(trade->getTimestamp()) << "\",\n";
        file << "      \"accountBalance\": " << params.accountBalance << ",\n";
        file << "      \"riskPercent\": " << params.riskPercent << ",\n";
        file << "      \"riskAmount\": " << results.riskAmount << ",\n";
        file << "      \"entryPrice\": " << params.entryPrice << ",\n";
        file << "      \"stopLossPrice\": " << results.stopLossPrice << ",\n";
        
        if (results.hasMultipleTargets) {
            file << "      \"tp1Price\": " << results.tp1Price << ",\n";
            file << "      \"tp2Price\": " << results.tp2Price << ",\n";
            file << "      \"tp1Amount\": " << results.tp1Amount << ",\n";
            file << "      \"tp2Amount\": " << results.tp2Amount << ",\n";
        } else {
            file << "      \"takeProfitPrice\": " << results.takeProfitPrice << ",\n";
            file << "      \"rewardAmount\": " << results.rewardAmount << ",\n";
        }
        
        file << "      \"positionSize\": " << results.positionSize << ",\n";
        file << "      \"riskRewardRatio\": " << results.riskRewardRatio << ",\n";
        file << "      \"instrument\": \"" << Utils::getInstrumentTypeString(params.instrumentType) << "\",\n";
        file << "      \"lotType\": \"" << Utils::getLotSizeTypeString(params.lotSizeType) << "\",\n";
        file << "      \"outcome\": \"" << trade->getOutcomeAsString() << "\",\n";
        
        double pnl = 0.0;
        if (trade->getOutcome() != TradeOutcome::Pending) {
            pnl = trade->getUpdatedAccountBalance() - params.accountBalance;
        }
        
        file << "      \"pnl\": " << pnl << ",\n";
        file << "      \"updatedBalance\": " << trade->getUpdatedAccountBalance() << "\n";
        
        // Add comma if not the last trade
        file << "    }" << (i < m_trades.size() - 1 ? "," : "") << "\n";
    }
    
    file << "  ]\n";
    file << "}\n";
    
    file.close();
    return true;
}

bool SessionManager::loadSession(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << " for reading." << std::endl;
        return false;
    }
    
    // Clear current trades
    m_trades.clear();
    
    // Skip header
    std::string header;
    std::getline(file, header);
    
    // Read each line
    std::string line;
    while (std::getline(file, line)) {
        auto tradeCsv = Utils::parseCSVLine(line);
        if (tradeCsv.size() < 15) {
            continue; // Skip invalid lines
        }
        
        // Create a new trade
        auto trade = std::make_shared<Trade>();
        
        try {
            // Set parameters from CSV
            trade->setAccountBalance(std::stod(tradeCsv[2]));
            trade->setRiskPercentage(std::stod(tradeCsv[3]));
            trade->setEntryPrice(std::stod(tradeCsv[5]));
            
            // Set SL/TP
            trade->setStopLoss(std::stod(tradeCsv[6]), InputType::Price);
            
            // Check if TP2 is available
            if (tradeCsv[8] != "0") {
                trade->setTakeProfit1(std::stod(tradeCsv[7]), InputType::Price, 60.0);
                trade->setTakeProfit2(std::stod(tradeCsv[8]), InputType::Price, 40.0);
            } else {
                trade->setTakeProfit(std::stod(tradeCsv[7]), InputType::Price);
            }
            
            // Set instrument type and lot size type
            std::string instrument = tradeCsv[11];
            if (instrument == "Forex") {
                trade->setInstrumentType(0); // Forex
            } else if (instrument == "Gold") {
                trade->setInstrumentType(1); // Gold
            } else if (instrument == "Indices") {
                trade->setInstrumentType(2); // Indices
            }
            
            std::string lotType = tradeCsv[12];
            if (lotType == "Standard") {
                trade->setLotSizeType(0); // Standard
            } else if (lotType == "Mini") {
                trade->setLotSizeType(1); // Mini
            } else if (lotType == "Micro") {
                trade->setLotSizeType(2); // Micro
            }
            
            // Calculate
            if (tradeCsv[8] != "0") {
                trade->calculateWithMultipleTargets();
            } else {
                trade->calculate();
            }
            
            // Set outcome
            std::string outcome = tradeCsv[13];
            if (outcome == "Loss at SL") {
                trade->simulateOutcome(TradeOutcome::LossAtSL);
            } else if (outcome == "Win at TP1") {
                trade->simulateOutcome(TradeOutcome::WinAtTP1);
            } else if (outcome == "Win at TP2") {
                trade->simulateOutcome(TradeOutcome::WinAtTP2);
            } else if (outcome == "Break Even") {
                trade->simulateOutcome(TradeOutcome::BreakEven);
            }
            
            // Add trade
            m_trades.push_back(trade);
        } catch (const std::exception& e) {
            std::cerr << "Error parsing trade: " << e.what() << std::endl;
            continue; // Skip this trade
        }
    }
    
    file.close();
    
    // Update session info
    if (!m_trades.empty()) {
        m_sessionActive = true;
        m_initialBalance = m_trades.front()->getParameters().accountBalance;
        m_currentBalance = m_trades.back()->getUpdatedAccountBalance();
        m_sessionFile = filename;
        return true;
    }
    
    return false;
}

void SessionManager::updateBalance(const std::shared_ptr<Trade>& trade) {
    if (trade->getOutcome() != TradeOutcome::Pending) {
        m_currentBalance = trade->getUpdatedAccountBalance();
    }
}

void SessionManager::generateSessionId() {
    // Generate a simple random ID using timestamp + random number
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::system_clock::to_time_t(now);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1000, 9999);
    
    std::stringstream ss;
    ss << "SESSION-" << std::put_time(std::localtime(&timestamp), "%Y%m%d-%H%M%S") 
       << "-" << distrib(gen);
    
    m_sessionId = ss.str();
}

void SessionManager::calculateStats(SessionStats& stats) const {
    // Initialize stats
    stats.initialBalance = m_initialBalance;
    stats.currentBalance = m_currentBalance;
    stats.totalTrades = static_cast<int>(m_trades.size());
    stats.winningTrades = 0;
    stats.losingTrades = 0;
    stats.breakEvenTrades = 0;
    stats.largestWin = 0.0;
    stats.largestLoss = 0.0;
    double totalWins = 0.0;
    double totalLosses = 0.0;
    double totalRR = 0.0;
    
    // Process each trade
    for (const auto& trade : m_trades) {
        auto outcome = trade->getOutcome();
        auto params = trade->getParameters();
        auto results = trade->getResults();
        
        // Skip pending trades
        if (outcome == TradeOutcome::Pending) {
            continue;
        }
        
        double pnl = trade->getUpdatedAccountBalance() - params.accountBalance;
        
        if (pnl > 0) {
            stats.winningTrades++;
            totalWins += pnl;
            if (pnl > stats.largestWin) {
                stats.largestWin = pnl;
            }
        } else if (pnl < 0) {
            stats.losingTrades++;
            totalLosses += std::abs(pnl);
            if (pnl < stats.largestLoss) {
                stats.largestLoss = pnl;
            }
        } else {
            stats.breakEvenTrades++;
        }
        
        totalRR += results.riskRewardRatio;
    }
    
    // Calculate derived statistics
    stats.totalPnL = stats.currentBalance - stats.initialBalance;
    
    int completedTrades = stats.winningTrades + stats.losingTrades + stats.breakEvenTrades;
    if (completedTrades > 0) {
        stats.winRate = static_cast<double>(stats.winningTrades) / completedTrades;
        stats.averageRR = totalRR / completedTrades;
    }
    
    if (totalLosses > 0) {
        stats.profitFactor = totalWins / totalLosses;
    }
} 