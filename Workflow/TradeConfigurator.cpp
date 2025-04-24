#include "TradeConfigurator.h"
#include "../Utils/InputHandler.h"
#include "../Utils.h"
#include <iostream>

namespace Workflow {
    void configureTradeParameters(std::shared_ptr<Trade> trade, std::shared_ptr<Risk::RiskProfile> riskProfile) {
        // Get basic parameters
        double riskPercent;
        
        if (riskProfile) {
            // Use risk profile for default risk
            riskPercent = riskProfile->getDefaultRisk();
            std::cout << "Using " << riskProfile->getName() << " risk profile: " << riskPercent << "%\n";
            
            // Allow user to override
            char overrideRisk = Utils::getYesNoInput("Do you want to override the default risk? (y/n): ");
            if (overrideRisk == 'y') {
                riskPercent = Utils::getValidInput<double>("Risk per trade (%): ", 0.01, 100.0, true);
            }
        } else {
            // Standard risk input
            riskPercent = Utils::getValidInput<double>("Risk per trade (%): ", 0.01, 100.0, true);
        }
        
        trade->setRiskPercentage(riskPercent);
        
        double entryPrice = Utils::getValidInput<double>("Entry Price: ", 0.00001, 1000000.0, true);
        trade->setEntryPrice(entryPrice);
        
        // Instrument selection
        std::cout << "\nSelect instrument type:\n";
        std::cout << "1. Forex\n";
        std::cout << "2. Gold\n";
        std::cout << "3. Indices\n";
        int instrumentChoice = Utils::getValidInput<int>("Enter your choice: ", 1, 3, true);
        trade->setInstrumentType(instrumentChoice - 1);
        
        // Lot size selection
        std::cout << "\nSelect lot size type:\n";
        std::cout << "1. Standard (100,000)\n";
        std::cout << "2. Mini (10,000)\n";
        std::cout << "3. Micro (1,000)\n";
        std::cout << "4. Custom\n";
        int lotSizeChoice = Utils::getValidInput<int>("Enter your choice: ", 1, 4, true);
        
        if (lotSizeChoice == 4) {
            // Custom lot size
            double contractSize = Utils::getValidInput<double>("Enter custom contract size: ", 1.0, 1000000.0, true);
            trade->setContractSize(contractSize);
        } else {
            trade->setLotSizeType(lotSizeChoice - 1);
        }
        
        // Get stop loss
        std::cout << "\nStop Loss input method:\n";
        std::cout << "1. Price\n";
        std::cout << "2. Pips\n";
        int slChoice = Utils::getValidInput<int>("Enter your choice: ", 1, 2, true);
        
        if (slChoice == 1) {
            // Price-based SL
            double slPrice = Utils::getValidInput<double>("Stop Loss Price: ", 0.00001, 1000000.0, true);
            trade->setStopLoss(slPrice, InputType::Price);
        } else {
            // Pips-based SL
            double slPips = Utils::getValidInput<double>("Stop Loss (pips): ", 0.1, 10000.0, true);
            trade->setStopLoss(slPips, InputType::Pips);
        }
        
        // Get take profit
        std::cout << "\nTake Profit input method:\n";
        std::cout << "1. Price\n";
        std::cout << "2. Pips\n";
        std::cout << "3. R:R Ratio\n";
        int tpChoice = Utils::getValidInput<int>("Enter your choice: ", 1, 3, true);
        
        if (tpChoice == 1) {
            // Price-based TP
            double tpPrice = Utils::getValidInput<double>("Take Profit Price: ", 0.00001, 1000000.0, true);
            trade->setTakeProfit(tpPrice, InputType::Price);
        } else if (tpChoice == 2) {
            // Pips-based TP
            double tpPips = Utils::getValidInput<double>("Take Profit (pips): ", 0.1, 10000.0, true);
            trade->setTakeProfit(tpPips, InputType::Pips);
        } else {
            // R:R Ratio
            double rrRatio = Utils::getValidInput<double>("Risk:Reward Ratio (e.g., 2 for 1:2): ", 0.1, 100.0, true);
            // The Trade class will calculate TP based on SL and RR
            // For multiple TP, we'll handle this in the calculateWithMultipleTargets method
        }
    }
} 