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

Trade::Trade() : m_outcome(TradeOutcome::Pending) {
    m_calculator = std::make_unique<TradeCalculator>();
    m_params = std::make_unique<TradeParameters>();
    m_results = std::make_unique<TradeResults>();
    generateId();
    m_timestamp = std::time(nullptr);
}

Trade::~Trade() = default;

void Trade::validateAccountBalance(double balance) const {
    if (balance <= 0.0) {
        throw TradeError("Account balance must be greater than 0");
    }
}

void Trade::validateRiskPercentage(double riskPercent) const {
    if (riskPercent <= 0.0 || riskPercent > 100.0) {
        throw TradeError("Risk percentage must be between 0 and 100");
    }
}

void Trade::validatePrice(double price) const {
    if (price <= 0.0) {
        throw TradeError("Price must be greater than 0");
    }
}

void Trade::validatePercentage(double percentage) const {
    if (percentage <= 0.0 || percentage > 100.0) {
        throw TradeError("Percentage must be between 0 and 100");
    }
}

void Trade::validateInstrumentType(int type) const {
    if (type < 0 || type > 2) {
        throw TradeError("Invalid instrument type");
    }
}

void Trade::validateLotSizeType(int type) const {
    if (type < 0 || type > 2) {
        throw TradeError("Invalid lot size type");
    }
}

void Trade::validateContractSize(double size) const {
    if (size < 0.0) {
        throw TradeError("Contract size cannot be negative");
    }
}

void Trade::setAccountBalance(double balance) {
    validateAccountBalance(balance);
    m_params->accountBalance = balance;
}

void Trade::setRiskPercentage(double riskPercent) {
    validateRiskPercentage(riskPercent);
    m_params->riskPercent = riskPercent;
}

void Trade::setEntryPrice(double price) {
    validatePrice(price);
    m_params->entryPrice = price;
}

void Trade::setStopLoss(double value, InputType type) {
    if (type == InputType::Pips) {
        if (value <= 0.0) {
            throw TradeError("Stop loss pips must be greater than 0");
        }
        m_params->stopLossInPips = value;
        m_params->isStopLossPriceOverride = false;
    } else {
        validatePrice(value);
        m_params->stopLossPrice = value;
        m_params->isStopLossPriceOverride = true;
    }
    m_slInputType = type;
}

void Trade::setTakeProfit(double value, InputType type) {
    if (type == InputType::Pips) {
        if (value <= 0.0) {
            throw TradeError("Take profit pips must be greater than 0");
        }
        m_params->takeProfitInPips = value;
    } else {
        validatePrice(value);
        m_params->takeProfitPrice = value;
    }
    m_tpInputType = type;
}

void Trade::setTakeProfit1(double value, InputType type, double percentage) {
    validatePercentage(percentage);
    m_tp1Percentage = percentage;
    setTakeProfit(value, type);
}

void Trade::setTakeProfit2(double value, InputType type, double percentage) {
    validatePercentage(percentage);
    m_tp2Percentage = percentage;
    setTakeProfit(value, type);
}

void Trade::setInstrumentType(int instrumentTypeIndex) {
    validateInstrumentType(instrumentTypeIndex);
    m_params->instrumentType = static_cast<InstrumentType>(instrumentTypeIndex);
}

void Trade::setLotSizeType(int lotSizeTypeIndex) {
    validateLotSizeType(lotSizeTypeIndex);
    m_params->lotSizeType = static_cast<LotSizeType>(lotSizeTypeIndex);
}

void Trade::setContractSize(double size) {
    validateContractSize(size);
    m_params->contractSize = size;
}

bool Trade::calculate() {
    try {
        *m_results = m_calculator->calculateTrade(*m_params);
        return true;
    } catch (const std::exception& e) {
        Utils::printError("Calculation error: " + std::string(e.what()));
        return false;
    }
}

bool Trade::calculateWithMultipleTargets() {
    try {
        *m_results = m_calculator->calculateMultipleTargets(*m_params, m_tp1Percentage, m_tp2Percentage);
        return true;
    } catch (const std::exception& e) {
        Utils::printError("Calculation error: " + std::string(e.what()));
        return false;
    }
}

void Trade::simulateOutcome(TradeOutcome outcome) {
    m_outcome = outcome;
    double profitLoss = 0.0;
    
    switch (outcome) {
        case TradeOutcome::LossAtSL:
            profitLoss = -m_results->riskAmount;
            break;
        case TradeOutcome::WinAtTP1:
            profitLoss = m_results->tp1Amount;
            break;
        case TradeOutcome::WinAtTP2:
            profitLoss = m_results->tp2Amount;
            break;
        case TradeOutcome::BreakEven:
            profitLoss = 0.0;
            break;
        default:
            profitLoss = 0.0;
            break;
    }
    
    m_params->accountBalance += profitLoss;
}

double Trade::getUpdatedAccountBalance() const {
    return m_params->accountBalance;
}

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
        case TradeOutcome::LossAtSL:
            return "Loss at Stop Loss";
        case TradeOutcome::WinAtTP1:
            return "Win at Take Profit 1";
        case TradeOutcome::WinAtTP2:
            return "Win at Take Profit 2";
        case TradeOutcome::BreakEven:
            return "Break Even";
        default:
            return "Pending";
    }
}

bool Trade::validate() const {
    try {
        validateAccountBalance(m_params->accountBalance);
        validateRiskPercentage(m_params->riskPercent);
        validatePrice(m_params->entryPrice);
        
        if (m_params->isStopLossPriceOverride) {
            validatePrice(m_params->stopLossPrice);
        } else {
            if (m_params->stopLossInPips <= 0.0) {
                throw TradeError("Stop loss pips must be greater than 0");
            }
        }
        
        if (m_tpInputType == InputType::Pips) {
            if (m_params->takeProfitInPips <= 0.0) {
                throw TradeError("Take profit pips must be greater than 0");
            }
        } else {
            validatePrice(m_params->takeProfitPrice);
        }
        
        validateInstrumentType(static_cast<int>(m_params->instrumentType));
        validateLotSizeType(static_cast<int>(m_params->lotSizeType));
        validateContractSize(m_params->contractSize);
        
        return true;
    } catch (const TradeError& e) {
        Utils::printError(e.what());
        return false;
    }
}

std::string Trade::getSummary() const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    
    ss << "Trade Summary:\n";
    ss << "ID: " << m_id << "\n";
    ss << "Timestamp: " << std::ctime(&m_timestamp);
    ss << "Account Balance: $" << m_params->accountBalance << "\n";
    ss << "Risk Percentage: " << m_params->riskPercent << "%\n";
    ss << "Entry Price: " << m_params->entryPrice << "\n";
    
    if (m_params->isStopLossPriceOverride) {
        ss << "Stop Loss Price: " << m_params->stopLossPrice << "\n";
    } else {
        ss << "Stop Loss Pips: " << m_params->stopLossInPips << "\n";
    }
    
    if (m_tpInputType == InputType::Pips) {
        ss << "Take Profit Pips: " << m_params->takeProfitInPips << "\n";
    } else {
        ss << "Take Profit Price: " << m_params->takeProfitPrice << "\n";
    }
    
    ss << "Position Size: " << m_results->positionSize << "\n";
    ss << "Risk Amount: $" << m_results->riskAmount << "\n";
    ss << "Reward Amount: $" << m_results->rewardAmount << "\n";
    ss << "Risk-Reward Ratio: " << m_results->riskRewardRatio << "\n";
    
    if (m_results->hasBreakEvenInfo) {
        ss << "Break Even Price: " << m_results->breakEvenPrice << "\n";
        ss << "Break Even Pips: " << m_results->breakEvenPips << "\n";
    }
    
    if (m_results->hasMultipleTargets) {
        ss << "TP1 Price: " << m_results->tp1Price << "\n";
        ss << "TP2 Price: " << m_results->tp2Price << "\n";
        ss << "TP1 Amount: $" << m_results->tp1Amount << "\n";
        ss << "TP2 Amount: $" << m_results->tp2Amount << "\n";
    }
    
    ss << "Outcome: " << getOutcomeAsString() << "\n";
    
    return ss.str();
}

bool Trade::save(const std::string& filePath, bool append) const {
    try {
        std::ofstream file;
        if (append) {
            file.open(filePath, std::ios::app);
        } else {
            file.open(filePath);
        }
        
        if (!file.is_open()) {
            throw TradeError("Failed to open file for writing");
        }
        
        file << getSummary() << "\n";
        file.close();
        return true;
    } catch (const std::exception& e) {
        Utils::printError("Failed to save trade: " + std::string(e.what()));
        return false;
    }
}

void Trade::reset() {
    m_params = std::make_unique<TradeParameters>();
    m_results = std::make_unique<TradeResults>();
    m_outcome = TradeOutcome::Pending;
    generateId();
    m_timestamp = std::time(nullptr);
}

std::time_t Trade::getTimestamp() const {
    return m_timestamp;
}

void Trade::setTimestamp(std::time_t timestamp) {
    m_timestamp = timestamp;
}

std::string Trade::getId() const {
    return m_id;
}

void Trade::generateId() {
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    );
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 9999);
    
    std::stringstream ss;
    ss << "TRADE_" << now_ms.count() << "_" << std::setfill('0') << std::setw(4) << dis(gen);
    m_id = ss.str();
}

double Trade::convertPriceToPoints(double entryPrice, double targetPrice) const {
    return std::abs(targetPrice - entryPrice) * 10000.0;
}

double Trade::convertPipsToPrice(double entryPrice, double pips) const {
    return entryPrice + (pips / 10000.0);
} 