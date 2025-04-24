#include "ViewSavedTrades.h"
#include "../UI/Menu.h"
#include "../Utils/InputHandler.h"
#include "../Utils.h"
#include <iostream>
#include <iomanip>

namespace Workflow {
    void viewSavedTrades(SessionManager& sessionManager) {
        Utils::clearScreen();
        UI::displayHeader("SAVED TRADES");
        
        // Get all trades
        auto trades = sessionManager.getAllTrades();
        
        if (trades.empty()) {
            std::cout << "No saved trades found.\n";
            return;
        }
        
        // Display summary of trades
        std::cout << "Found " << trades.size() << " trades:\n\n";
        
        // Header
        std::cout << std::left << std::setw(8) << "ID" 
                 << std::setw(20) << "Date/Time" 
                 << std::setw(12) << "Entry" 
                 << std::setw(12) << "SL" 
                 << std::setw(12) << "TP" 
                 << std::setw(10) << "Outcome" 
                 << std::setw(12) << "P&L" 
                 << std::endl;
        
        std::cout << std::string(80, '-') << std::endl;
        
        // Display each trade's basic info
        for (const auto& trade : trades) {
            auto params = trade->getParameters();
            auto results = trade->getResults();
            
            // Format trade ID (first 6 chars)
            std::string id = trade->getId().substr(0, 6);
            
            // Format date/time
            std::string timestamp = Utils::getFormattedTimestamp(trade->getTimestamp());
            
            // Format P&L
            std::string pnl;
            if (trade->getOutcome() == TradeOutcome::WinAtTP1 || 
                trade->getOutcome() == TradeOutcome::WinAtTP2) {
                pnl = "+$" + std::to_string(results.rewardAmount);
            } else if (trade->getOutcome() == TradeOutcome::LossAtSL) {
                pnl = "-$" + std::to_string(results.riskAmount);
            } else {
                pnl = "$0.00";
            }
            
            // Display the row
            std::cout << std::left << std::setw(8) << id
                     << std::setw(20) << timestamp
                     << std::fixed << std::setprecision(5)
                     << std::setw(12) << params.entryPrice
                     << std::setw(12) << results.stopLossPrice
                     << std::setw(12) << results.takeProfitPrice
                     << std::setw(10) << trade->getOutcomeAsString()
                     << std::setw(12) << pnl
                     << std::endl;
        }
        
        std::cout << std::endl;
        
        // Option to view detailed info for a specific trade
        char viewDetail = Utils::getYesNoInput("View detailed trade info? (y/n): ");
        if (viewDetail == 'y') {
            std::string tradeId;
            std::cout << "Enter trade ID: ";
            std::cin >> tradeId;
            
            // Find the trade with matching ID
            bool found = false;
            for (const auto& trade : trades) {
                if (trade->getId().find(tradeId) == 0) {
                    Utils::clearScreen();
                    UI::displayHeader("TRADE DETAILS");
                    std::cout << trade->getSummary() << std::endl;
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                Utils::printError("Trade not found.");
            }
        }
    }
} 