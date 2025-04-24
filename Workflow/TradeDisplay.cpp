#include "TradeDisplay.h"
#include "../UI/Menu.h"
#include "../Utils.h"
#include <iostream>
#include <iomanip>

namespace Workflow {
    void displayTradeResults(std::shared_ptr<Trade> trade) {
        Utils::clearScreen();
        UI::displayHeader("TRADE CALCULATION RESULTS");
        
        // Get trade results
        auto params = trade->getParameters();
        auto results = trade->getResults();
        
        // Display basic information
        std::cout << "Account Balance: $" << std::fixed << std::setprecision(2) 
                 << params.accountBalance << "\n";
        std::cout << "Risk Per Trade:  " << params.riskPercent << "%\n";
        std::cout << "Entry Price:     " << std::setprecision(5) << params.entryPrice << "\n\n";
        
        // Display position sizing
        std::cout << "Position Size:   " << std::setprecision(2) << results.positionSize << " lot(s)\n";
        std::cout << "Risk Amount:     $" << results.riskAmount << "\n";
        std::cout << "Reward Amount:   $" << results.rewardAmount << "\n";
        std::cout << "Risk/Reward:     1:" << std::setprecision(2) << results.riskRewardRatio << "\n\n";
        
        // Display stop loss and take profit
        std::cout << "Stop Loss:       " << std::setprecision(5) << results.stopLossPrice 
                 << " (" << std::setprecision(1) << params.stopLossInPips << " pips)\n";
        
        if (results.hasMultipleTargets) {
            std::cout << "Take Profit 1:   " << std::setprecision(5) << results.tp1Price << "\n";
            std::cout << "Take Profit 2:   " << std::setprecision(5) << results.tp2Price << "\n";
            std::cout << "TP1 Reward:      $" << std::setprecision(2) << results.tp1Amount << "\n";
            std::cout << "TP2 Reward:      $" << std::setprecision(2) << results.tp2Amount << "\n";
        } else {
            std::cout << "Take Profit:     " << std::setprecision(5) << results.takeProfitPrice 
                     << " (" << std::setprecision(1) << params.takeProfitInPips << " pips)\n";
        }
        
        // Display breakeven info if available
        if (results.hasBreakEvenInfo) {
            std::cout << "\nBreak Even:      " << std::setprecision(5) << results.breakEvenPrice 
                     << " (" << std::setprecision(1) << results.breakEvenPips << " pips)\n";
        }
        
        std::cout << "\nPip Value:        $" << std::setprecision(4) << results.pipValue << "\n";
        
        // Display instrument info
        std::cout << "\nInstrument:       " << Utils::getInstrumentTypeString(params.instrumentType) << "\n";
        std::cout << "Lot Size:         " << Utils::getLotSizeTypeString(params.lotSizeType) << "\n";
        
        if (params.contractSize > 0) {
            std::cout << "Contract Size:    " << params.contractSize << "\n";
        }
        
        std::cout << std::endl;
    }
} 