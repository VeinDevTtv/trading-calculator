#include "TradeCalculator.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

TradeCalculator::TradeCalculator() {
    // Constructor
}

TradeCalculator::~TradeCalculator() {
    // Destructor
}

void TradeCalculator::setFeePercentage(double feePercent) {
    m_feePercentage = feePercent;
}

void TradeCalculator::setFixedSpreadPips(double spreadPips) {
    m_spreadPips = spreadPips;
}

TradeResults TradeCalculator::calculateTrade(const TradeParameters& params) {
    TradeResults results;
    
    try {
        // Calculate risk amount in money terms
        results.riskAmount = (params.riskPercent / 100.0) * params.accountBalance;
        
        // Calculate pip value
        results.pipValue = calculatePipValue(params);
        
        // Calculate stop loss price
        results.stopLossPrice = params.isStopLossPriceOverride ? 
                               params.stopLossPrice : 
                               calculateStopLossPrice(params);
        
        // Calculate position size in lots
        double slDifference = params.isStopLossPriceOverride ? 
                             std::abs(params.entryPrice - params.stopLossPrice) * 
                             (params.instrumentType == InstrumentType::Forex ? 10000 : 10) : 
                             params.stopLossInPips;
                             
        results.positionSize = results.riskAmount / (results.pipValue * slDifference);
        
        // Round position size to 2 decimal places
        results.positionSize = std::round(results.positionSize * 100) / 100;
        
        // Calculate take profit price and reward amount
        results.riskRewardRatio = params.riskRewardRatio;
        results.takeProfitPrice = calculateTakeProfitPrice(params);
        results.rewardAmount = results.riskAmount * results.riskRewardRatio;
        
        // Add breakeven calculations if fee/spread is set
        if (m_feePercentage > 0 || m_spreadPips > 0) {
            results.hasBreakEvenInfo = true;
            results.breakEvenPrice = calculateBreakEvenPoint(params);
            results.breakEvenPips = std::abs(results.breakEvenPrice - params.entryPrice) * 
                                  (params.instrumentType == InstrumentType::Forex ? 10000 : 10);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in calculations: " << e.what() << std::endl;
    }
    
    return results;
}

TradeResults TradeCalculator::calculateMultipleTargets(const TradeParameters& params, 
                                                      double tp1Percent, double tp2Percent) {
    // First calculate regular results
    TradeResults results = calculateTrade(params);
    
    // Then add multiple target information
    results.hasMultipleTargets = true;
    
    // Calculate TP1 and TP2 based on percentages
    double totalTpPips = params.takeProfitInPips;
    double tp1Pips = totalTpPips * (tp1Percent / 100.0);
    double tp2Pips = totalTpPips * (tp2Percent / 100.0);
    
    // Calculate TP prices
    if (params.instrumentType == InstrumentType::Forex) {
        results.tp1Price = params.entryPrice + (tp1Pips / 10000.0);
        results.tp2Price = params.entryPrice + (tp2Pips / 10000.0);
    }
    else {
        results.tp1Price = params.entryPrice + (tp1Pips / 10.0);
        results.tp2Price = params.entryPrice + (tp2Pips / 10.0);
    }
    
    // Calculate reward amounts
    results.tp1Amount = results.riskAmount * (tp1Pips / params.stopLossInPips);
    results.tp2Amount = results.riskAmount * (tp2Pips / params.stopLossInPips);
    
    return results;
}

double TradeCalculator::calculatePipValue(const TradeParameters& params) {
    double pipValue = 0.0;
    double lotSize = 0.0;
    
    // Set base lot size based on type
    switch (params.lotSizeType) {
        case LotSizeType::Standard:
            lotSize = 100000;
            break;
        case LotSizeType::Mini:
            lotSize = 10000;
            break;
        case LotSizeType::Micro:
            lotSize = 1000;
            break;
    }
    
    // Handle custom contract size
    if (params.contractSize > 0) {
        lotSize = params.contractSize;
    }
    
    // Calculate pip value based on instrument type
    switch (params.instrumentType) {
        case InstrumentType::Forex:
            pipValue = 0.0001 * lotSize;
            break;
        case InstrumentType::Gold:
            pipValue = 0.1 * lotSize / 100; // Gold is typically quoted per ounce
            break;
        case InstrumentType::Indices:
            pipValue = 1.0 * lotSize / 100; // Indices typically use points
            break;
    }
    
    return pipValue;
}

double TradeCalculator::calculateStopLossPrice(const TradeParameters& params) {
    double slPrice = 0.0;
    
    // Calculate SL price based on instrument type
    switch (params.instrumentType) {
        case InstrumentType::Forex:
            slPrice = params.entryPrice - (params.stopLossInPips / 10000.0);
            break;
        case InstrumentType::Gold:
        case InstrumentType::Indices:
            slPrice = params.entryPrice - (params.stopLossInPips / 10.0);
            break;
    }
    
    return slPrice;
}

double TradeCalculator::calculateTakeProfitPrice(const TradeParameters& params) {
    double tpPrice = 0.0;
    
    // Calculate TP price based on instrument type
    switch (params.instrumentType) {
        case InstrumentType::Forex:
            tpPrice = params.entryPrice + (params.takeProfitInPips / 10000.0);
            break;
        case InstrumentType::Gold:
        case InstrumentType::Indices:
            tpPrice = params.entryPrice + (params.takeProfitInPips / 10.0);
            break;
    }
    
    return tpPrice;
}

double TradeCalculator::calculateBreakEvenPoint(const TradeParameters& params) {
    // Simple implementation - for now just add spread costs
    double spreadCost = m_spreadPips;
    double breakEvenPips = spreadCost;
    
    // Add fee cost in pips if applicable
    if (m_feePercentage > 0) {
        double feeAmount = (m_feePercentage / 100.0) * params.accountBalance;
        double pipValue = calculatePipValue(params);
        double feePips = feeAmount / pipValue;
        breakEvenPips += feePips;
    }
    
    // Calculate break-even price
    double breakEvenPrice = 0.0;
    switch (params.instrumentType) {
        case InstrumentType::Forex:
            breakEvenPrice = params.entryPrice + (breakEvenPips / 10000.0);
            break;
        case InstrumentType::Gold:
        case InstrumentType::Indices:
            breakEvenPrice = params.entryPrice + (breakEvenPips / 10.0);
            break;
    }
    
    return breakEvenPrice;
} 