#include "Trade.h"
#include "TradeCalculator.h"
#include "Utils.h"
#include <sstream>
#include <iomanip>
#include <random>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <iostream>

Trade::Trade() : 
    m_timestamp(std::time(nullptr)), 
    m_calculator(std::make_unique<TradeCalculator>()),
    m_params(std::make_unique<TradeParameters>()),
    m_results(std::make_unique<TradeResults>()),
    m_outcome(TradeOutcome::Pending),
    m_tp1Percentage(60.0),
    m_tp2Percentage(40.0),
    m_slInputType(InputType::Pips),
    m_tpInputType(InputType::Pips)
{
    generateId();
}

Trade::~Trade() = default;

void Trade::generateId() {
    // Generate a simple random ID using timestamp + random number
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1000, 9999);
    
    std::stringstream ss;
    ss << "TRD-" << std::put_time(std::localtime(&timestamp), "%Y%m%d-%H%M%S") 
       << "-" << ms.count() << "-" << distrib(gen);
    
    m_id = ss.str();
}

// Setup methods
void Trade::setAccountBalance(double balance) {
    if (balance <= 0) {
        throw std::invalid_argument("Account balance must be positive");
    }
    m_params->accountBalance = balance;
}

void Trade::setRiskPercentage(double riskPercent) {
    if (riskPercent <= 0 || riskPercent > 100) {
        throw std::invalid_argument("Risk percentage must be between 0 and 100");
    }
    m_params->riskPercent = riskPercent;
}

void Trade::setEntryPrice(double price) {
    if (price <= 0) {
        throw std::invalid_argument("Entry price must be positive");
    }
    m_params->entryPrice = price;
}

// Flexible input methods
void Trade::setStopLoss(double value, InputType type) {
    if (value <= 0) {
        throw std::invalid_argument("Stop loss value must be positive");
    }
    
    m_slInputType = type;
    
    if (type == InputType::Pips) {
        m_params->stopLossInPips = value;
        m_params->isStopLossPriceOverride = false;
    } else {
        m_params->stopLossPrice = value;
        m_params->isStopLossPriceOverride = true;
        // Calculate pips from price for internal use
        if (m_params->entryPrice > 0) {
            m_params->stopLossInPips = convertPriceToPoints(m_params->entryPrice, value);
        }
    }
}

void Trade::setTakeProfit(double value, InputType type) {
    if (value <= 0) {
        throw std::invalid_argument("Take profit value must be positive");
    }
    
    m_tpInputType = type;
    
    if (type == InputType::Pips) {
        m_params->takeProfitInPips = value;
        // Calculate RR ratio
        if (m_params->stopLossInPips > 0) {
            m_params->riskRewardRatio = value / m_params->stopLossInPips;
        }
    } else {
        // Calculate pips from price
        if (m_params->entryPrice > 0) {
            m_params->takeProfitInPips = convertPriceToPoints(m_params->entryPrice, value);
            if (m_params->stopLossInPips > 0) {
                m_params->riskRewardRatio = m_params->takeProfitInPips / m_params->stopLossInPips;
            }
        }
    }
}

void Trade::setTakeProfit1(double value, InputType type, double percentage) {
    setTakeProfit(value, type);
    m_tp1Percentage = percentage;
}

void Trade::setTakeProfit2(double value, InputType type, double percentage) {
    // For TP2, we'll store the full value but use percentage for calculation
    if (m_tpInputType == InputType::Pips) {
        // Convert to combined TP value based on percentage
        double fullTp = value / (percentage / 100.0);
        setTakeProfit(fullTp, type);
    } else {
        // For price, we'll handle this differently in calculateWithMultipleTargets
        setTakeProfit(value, type);
    }
    m_tp2Percentage = percentage;
}

void Trade::setInstrumentType(int instrumentTypeIndex) {
    m_params->instrumentType = static_cast<InstrumentType>(instrumentTypeIndex);
}

void Trade::setLotSizeType(int lotSizeTypeIndex) {
    m_params->lotSizeType = static_cast<LotSizeType>(lotSizeTypeIndex);
}

void Trade::setContractSize(double size) {
    if (size < 0) {
        throw std::invalid_argument("Contract size cannot be negative");
    }
    m_params->contractSize = size;
}

// Calculation methods
bool Trade::calculate() {
    try {
        *m_results = m_calculator->calculateTrade(*m_params);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error in trade calculation: " << e.what() << std::endl;
        return false;
    }
}

bool Trade::calculateWithMultipleTargets() {
    try {
        // First calculate the basic results
        *m_results = m_calculator->calculateTrade(*m_params);
        
        // Then calculate multiple targets
        TradeResults multiResults = m_calculator->calculateMultipleTargets(
            *m_params, m_tp1Percentage, m_tp2Percentage);
        
        // Copy multi-target specific results
        m_results->hasMultipleTargets = true;
        m_results->tp1Price = multiResults.tp1Price;
        m_results->tp2Price = multiResults.tp2Price;
        m_results->tp1Amount = multiResults.tp1Amount;
        m_results->tp2Amount = multiResults.tp2Amount;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error in multiple targets calculation: " << e.what() << std::endl;
        return false;
    }
}

// Simulation methods
void Trade::simulateOutcome(TradeOutcome outcome) {
    m_outcome = outcome;
}

double Trade::getUpdatedAccountBalance() const {
    double updatedBalance = m_params->accountBalance;
    
    switch (m_outcome) {
        case TradeOutcome::LossAtSL:
            updatedBalance -= m_results->riskAmount;
            break;
        case TradeOutcome::WinAtTP1:
            if (m_results->hasMultipleTargets) {
                updatedBalance += m_results->tp1Amount;
            } else {
                updatedBalance += m_results->rewardAmount;
            }
            break;
        case TradeOutcome::WinAtTP2:
            if (m_results->hasMultipleTargets) {
                updatedBalance += m_results->tp2Amount;
            } else {
                updatedBalance += m_results->rewardAmount;
            }
            break;
        case TradeOutcome::BreakEven:
            // No change to balance
            break;
        default:
            // No change for pending
            break;
    }
    
    return updatedBalance;
}

// Access methods
TradeParameters Trade::getParameters() const {
    return *m_params;
}

TradeResults Trade::getResults() const {
    return *m_results;
}

TradeOutcome Trade::getOutcome() const {
    return m_outcome;
}

std::string Trade::getOutcomeAsString() const {
    switch (m_outcome) {
        case TradeOutcome::Pending:
            return "Pending";
        case TradeOutcome::LossAtSL:
            return "Loss at SL";
        case TradeOutcome::WinAtTP1:
            return "Win at TP1";
        case TradeOutcome::WinAtTP2:
            return "Win at TP2";
        case TradeOutcome::BreakEven:
            return "Break Even";
        default:
            return "Unknown";
    }
}

// Utility methods
bool Trade::validate() const {
    // Check if all required parameters are set
    if (m_params->accountBalance <= 0) return false;
    if (m_params->riskPercent <= 0) return false;
    if (m_params->entryPrice <= 0) return false;
    
    // Either stop loss in pips or price must be set
    if (m_params->isStopLossPriceOverride) {
        if (m_params->stopLossPrice <= 0) return false;
    } else {
        if (m_params->stopLossInPips <= 0) return false;
    }
    
    // Take profit must be set
    if (m_params->takeProfitInPips <= 0) return false;
    
    return true;
}

std::string Trade::getSummary() const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    
    // Basic trade info
    ss << "Trade ID: " << m_id << "\n";
    ss << "Date: " << Utils::getFormattedTimestamp(m_timestamp) << "\n\n";
    
    // Parameters
    ss << "Account Balance: $" << m_params->accountBalance << "\n";
    ss << "Risk: " << m_params->riskPercent << "% ($" << m_results->riskAmount << ")\n";
    ss << "Entry Price: " << m_params->entryPrice << "\n";
    ss << "Stop Loss: ";
    if (m_slInputType == InputType::Pips) {
        ss << m_params->stopLossInPips << " pips (" << m_results->stopLossPrice << ")\n";
    } else {
        ss << m_results->stopLossPrice << " (";
        ss << m_params->stopLossInPips << " pips)\n";
    }
    
    // Take profit info
    if (m_results->hasMultipleTargets) {
        ss << "TP1: " << m_results->tp1Price << " (";
        ss << m_tp1Percentage << "%, $" << m_results->tp1Amount << ")\n";
        ss << "TP2: " << m_results->tp2Price << " (";
        ss << m_tp2Percentage << "%, $" << m_results->tp2Amount << ")\n";
    } else {
        ss << "Take Profit: ";
        if (m_tpInputType == InputType::Pips) {
            ss << m_params->takeProfitInPips << " pips (" << m_results->takeProfitPrice << ")\n";
        } else {
            ss << m_results->takeProfitPrice << " (";
            ss << m_params->takeProfitInPips << " pips)\n";
        }
        ss << "Potential Reward: $" << m_results->rewardAmount << "\n";
    }
    
    ss << "Risk-Reward Ratio: 1:" << m_results->riskRewardRatio << "\n";
    ss << "Position Size: " << m_results->positionSize << " lots\n";
    ss << "Instrument: " << Utils::getInstrumentTypeString(m_params->instrumentType) << "\n";
    ss << "Lot Type: " << Utils::getLotSizeTypeString(m_params->lotSizeType) << "\n";
    
    // Outcome if simulated
    if (m_outcome != TradeOutcome::Pending) {
        ss << "\nOutcome: " << getOutcomeAsString() << "\n";
        ss << "Updated Balance: $" << getUpdatedAccountBalance() << "\n";
        double pnl = getUpdatedAccountBalance() - m_params->accountBalance;
        ss << "P&L: " << (pnl >= 0 ? "+" : "") << "$" << pnl << " (";
        ss << (pnl >= 0 ? "+" : "") << (pnl / m_params->accountBalance * 100) << "%)\n";
    }
    
    return ss.str();
}

bool Trade::save(const std::string& filePath, bool append) const {
    // Create the trade data as a CSV line
    std::stringstream ss;
    
    ss << m_id << ","
       << Utils::getFormattedTimestamp(m_timestamp) << ","
       << m_params->accountBalance << ","
       << m_params->riskPercent << ","
       << m_results->riskAmount << ","
       << m_params->entryPrice << ","
       << m_results->stopLossPrice << ",";
    
    if (m_results->hasMultipleTargets) {
        ss << m_results->tp1Price << "," << m_results->tp2Price << ",";
    } else {
        ss << m_results->takeProfitPrice << ",0,";
    }
    
    ss << m_results->positionSize << ","
       << m_results->riskRewardRatio << ","
       << Utils::getInstrumentTypeString(m_params->instrumentType) << ","
       << Utils::getLotSizeTypeString(m_params->lotSizeType) << ",";
    
    // Add outcome and P&L
    ss << getOutcomeAsString() << ",";
    
    double pnl = 0.0;
    if (m_outcome != TradeOutcome::Pending) {
        pnl = getUpdatedAccountBalance() - m_params->accountBalance;
    }
    ss << pnl << "," << getUpdatedAccountBalance();
    
    // Write to file
    std::ofstream file;
    if (append) {
        file.open(filePath, std::ios::app);
    } else {
        file.open(filePath);
    }
    
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filePath << " for writing." << std::endl;
        return false;
    }
    
    // If this is a new file, add header
    if (file.tellp() == 0) {
        file << "ID,Timestamp,AccountBalance,RiskPercent,RiskAmount,EntryPrice,SLPrice,"
             << "TP1Price,TP2Price,PositionSize,RRRatio,Instrument,LotType,Outcome,PnL,UpdatedBalance"
             << std::endl;
    }
    
    file << ss.str() << std::endl;
    file.close();
    
    return true;
}

void Trade::reset() {
    m_params = std::make_unique<TradeParameters>();
    m_results = std::make_unique<TradeResults>();
    m_outcome = TradeOutcome::Pending;
    m_timestamp = std::time(nullptr);
    generateId();
}

// Time and identification
std::time_t Trade::getTimestamp() const {
    return m_timestamp;
}

void Trade::setTimestamp(std::time_t timestamp) {
    m_timestamp = timestamp;
}

std::string Trade::getId() const {
    return m_id;
}

// Helper methods
double Trade::convertPriceToPoints(double entryPrice, double targetPrice) const {
    double diff = std::abs(targetPrice - entryPrice);
    
    switch (m_params->instrumentType) {
        case InstrumentType::Forex:
            return diff * 10000.0;
        case InstrumentType::Gold:
        case InstrumentType::Indices:
            return diff * 10.0;
        default:
            return diff * 10000.0;
    }
}

double Trade::convertPipsToPrice(double entryPrice, double pips) const {
    switch (m_params->instrumentType) {
        case InstrumentType::Forex:
            return entryPrice + (pips / 10000.0);
        case InstrumentType::Gold:
        case InstrumentType::Indices:
            return entryPrice + (pips / 10.0);
        default:
            return entryPrice + (pips / 10000.0);
    }
} 