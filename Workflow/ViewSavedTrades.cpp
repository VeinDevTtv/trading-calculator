#include "ViewSavedTrades.h"
#include "../UI/Menu.h"
#include "../Utils/InputHandler.h"
#include "../Utils.h"
#include <iostream>
#include <iomanip>

namespace Workflow {
    void viewSavedTrades(SessionManager& sessionManager) {
        Utils::printHeader("SAVED TRADES");
        
        auto trades = sessionManager.getAllTrades();
        if (trades.empty()) {
            Utils::printInfo("No trades found in the current session.");
            return;
        }
        
        int count = 1;
        for (const auto& trade : trades) {
            std::cout << count << ". ";
            Utils::printColorText("Trade: " + trade->getId() + "\n", Utils::COLOR_CYAN);
            
            auto params = trade->getParameters();
            auto results = trade->getResults();
            std::string outcome = trade->getOutcomeAsString();
            
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "   Date: " << Utils::getFormattedTimestamp(trade->getTimestamp()) << "\n";
            std::cout << "   Entry: " << params.entryPrice;
            
            // Color-coded SL and TP
            std::cout << " | SL: ";
            Utils::printColorText(std::to_string(results.stopLossPrice), Utils::COLOR_RED);
            
            if (results.hasMultipleTargets) {
                std::cout << " | TP1: ";
                Utils::printColorText(std::to_string(results.tp1Price), Utils::COLOR_GREEN);
                std::cout << " | TP2: ";
                Utils::printColorText(std::to_string(results.tp2Price), Utils::COLOR_GREEN);
            } else {
                std::cout << " | TP: ";
                Utils::printColorText(std::to_string(results.takeProfitPrice), Utils::COLOR_GREEN);
            }
            
            std::cout << "\n";
            std::cout << "   Risk: $" << results.riskAmount;
            std::cout << " | Reward: $" << results.rewardAmount;
            std::cout << " | RR: 1:" << results.riskRewardRatio << "\n";
            
            if (trade->getOutcome() != TradeOutcome::Pending) {
                std::cout << "   Outcome: ";
                
                if (outcome == "Loss at SL") {
                    Utils::printColorText(outcome, Utils::COLOR_RED);
                } else if (outcome == "Win at TP1" || outcome == "Win at TP2") {
                    Utils::printColorText(outcome, Utils::COLOR_GREEN);
                } else {
                    Utils::printColorText(outcome, Utils::COLOR_YELLOW);
                }
                
                double pnl = trade->getUpdatedAccountBalance() - params.accountBalance;
                std::cout << " | P&L: ";
                if (pnl >= 0) {
                    Utils::printColorText("+$" + std::to_string(pnl), Utils::COLOR_GREEN);
                } else {
                    Utils::printColorText("-$" + std::to_string(std::abs(pnl)), Utils::COLOR_RED);
                }
                std::cout << "\n";
            } else {
                std::cout << "   Outcome: Pending\n";
            }
            
            std::cout << std::endl;
            count++;
        }
        
        // Option to view detailed trade
        std::cout << "Would you like to view detailed information for a specific trade?\n";
        char viewDetailed = Utils::getYesNoInput("View detailed trade? (y/n): ");
        
        if (viewDetailed == 'y') {
            int tradeIndex = Utils::getValidInput<int>("Enter trade number: ", 1, static_cast<int>(trades.size()), true);
            
            Utils::clearScreen();
            Utils::printHeader("TRADE DETAILS");
            
            auto selectedTrade = trades[tradeIndex - 1];
            std::cout << selectedTrade->getSummary() << std::endl;
        }
    }
} 