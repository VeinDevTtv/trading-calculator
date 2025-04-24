#include "TradeCalculator.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <mutex>

TradeCalculator::TradeCalculator() {
    // Constructor
}

TradeCalculator::~TradeCalculator() {
    // Destructor
    clearCache();
}

void TradeCalculator::clearCache() {
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    m_pipValueCache.clear();
}

void TradeCalculator::setFeePercentage(double feePercent) {
    if (feePercent < 0.0) {
        throw std::invalid_argument("Fee percentage cannot be negative");
    }
    m_feePercentage = feePercent;
}

void TradeCalculator::setFixedSpreadPips(double spreadPips) {
    if (spreadPips < 0.0) {
        throw std::invalid_argument("Spread pips cannot be negative");
    }
    m_spreadPips = spreadPips;
}

TradeResults TradeCalculator::calculateTrade(const TradeParameters& params) {
    TradeResults results;
    
    try {
        // Calculate pip value
        double pipValue = calculatePipValue(params);
        
        // Calculate stop loss and take profit prices
        double stopLossPrice = calculateStopLossPrice(params);
        double takeProfitPrice = calculateTakeProfitPrice(params);
        
        // Calculate risk amount
        double riskAmount = std::abs(params.entryPrice - stopLossPrice) * pipValue;
        results.riskAmount = riskAmount;
        
        // Calculate position size
        double riskAmountInCurrency = params.accountBalance * (params.riskPercent / 100.0);
        results.positionSize = riskAmountInCurrency / riskAmount;
        
        // Calculate reward amount
        double rewardAmount = std::abs(takeProfitPrice - params.entryPrice) * pipValue;
        results.rewardAmount = rewardAmount;
        
        // Calculate risk-reward ratio
        results.riskRewardRatio = rewardAmount / riskAmount;
        
        // Store prices
        results.stopLossPrice = stopLossPrice;
        results.takeProfitPrice = takeProfitPrice;
        
        // Calculate break even point
        if (params.riskRewardRatio > 0) {
            results.hasBreakEvenInfo = true;
            results.breakEvenPrice = calculateBreakEvenPoint(params);
            results.breakEvenPips = std::abs(results.breakEvenPrice - params.entryPrice) / pipValue;
        }
        
        // Apply fees and spread
        if (m_feePercentage > 0.0) {
            double feeAmount = results.positionSize * m_feePercentage / 100.0;
            results.riskAmount += feeAmount;
            results.rewardAmount -= feeAmount;
        }
        
        if (m_spreadPips > 0.0) {
            double spreadAmount = m_spreadPips * pipValue;
            results.riskAmount += spreadAmount;
            results.rewardAmount -= spreadAmount;
        }
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Error calculating trade: " + std::string(e.what()));
    }
    
    return results;
}

TradeResults TradeCalculator::calculateMultipleTargets(const TradeParameters& params, 
                                                      double tp1Percent, double tp2Percent) {
    TradeResults results = calculateTrade(params);
    results.hasMultipleTargets = true;
    
    // Calculate TP1 and TP2 prices
    double totalDistance = std::abs(results.takeProfitPrice - params.entryPrice);
    double tp1Distance = totalDistance * (tp1Percent / 100.0);
    double tp2Distance = totalDistance * (tp2Percent / 100.0);
    
    if (results.takeProfitPrice > params.entryPrice) {
        results.tp1Price = params.entryPrice + tp1Distance;
        results.tp2Price = params.entryPrice + tp2Distance;
    } else {
        results.tp1Price = params.entryPrice - tp1Distance;
        results.tp2Price = params.entryPrice - tp2Distance;
    }
    
    // Calculate TP1 and TP2 amounts
    double pipValue = calculatePipValue(params);
    results.tp1Amount = std::abs(results.tp1Price - params.entryPrice) * pipValue;
    results.tp2Amount = std::abs(results.tp2Price - params.entryPrice) * pipValue;
    
    return results;
}

double TradeCalculator::calculatePipValue(const TradeParameters& params) {
    CacheKey key{params.instrumentType, params.lotSizeType, params.contractSize};
    
    {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        auto it = m_pipValueCache.find(key);
        if (it != m_pipValueCache.end()) {
            return it->second;
        }
    }
    
    double pipValue;
    double lotSize;
    
    // Determine lot size
    switch (params.lotSizeType) {
        case LotSizeType::Standard:
            lotSize = 100000.0;
            break;
        case LotSizeType::Mini:
            lotSize = 10000.0;
            break;
        case LotSizeType::Micro:
            lotSize = 1000.0;
            break;
        default:
            throw std::invalid_argument("Invalid lot size type");
    }
    
    // Calculate pip value based on instrument type
    switch (params.instrumentType) {
        case InstrumentType::Forex:
            pipValue = DEFAULT_PIP_VALUE * lotSize;
            break;
        case InstrumentType::Gold:
            pipValue = GOLD_PIP_VALUE * lotSize;
            break;
        case InstrumentType::Indices:
            pipValue = INDICES_PIP_VALUE * lotSize;
            break;
        default:
            throw std::invalid_argument("Invalid instrument type");
    }
    
    // Apply custom contract size if specified
    if (params.contractSize > 0.0) {
        pipValue *= (params.contractSize / lotSize);
    }
    
    // Cache the result
    {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        m_pipValueCache[key] = pipValue;
    }
    
    return pipValue;
}

double TradeCalculator::calculateStopLossPrice(const TradeParameters& params) {
    if (params.isStopLossPriceOverride) {
        return params.stopLossPrice;
    }
    return params.entryPrice - (params.stopLossInPips * DEFAULT_PIP_VALUE);
}

double TradeCalculator::calculateTakeProfitPrice(const TradeParameters& params) {
    return params.entryPrice + (params.takeProfitInPips * DEFAULT_PIP_VALUE);
}

double TradeCalculator::calculateBreakEvenPoint(const TradeParameters& params) {
    return params.entryPrice + (params.stopLossInPips * DEFAULT_PIP_VALUE * 0.5);
} 