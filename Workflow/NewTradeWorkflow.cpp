#include "NewTradeWorkflow.h"
#include "TradeConfigurator.h"
#include "TradeDisplay.h"
#include "../UI/Menu.h"
#include "../Utils/InputHandler.h"
#include "../Utils.h"
#include <iostream>
#include <limits>

namespace Workflow {
    void newTradeWorkflow(SessionManager& sessionManager) {
        UI::displayHeader("NEW TRADE CALCULATION");
        
        // Create a new trade
        auto trade = sessionManager.createTrade();
        
        // Configure trade parameters
        configureTradeParameters(trade);
        
        // Validate and calculate
        if (!trade->validate()) {
            Utils::printError("Invalid trade parameters. Please check your inputs.");
            return;
        }
        
        // Check if user wants multiple targets
        char useMultipleTargets = Utils::getYesNoInput("Do you want to use multiple take-profit targets? (y/n): ");
        
        bool success = false;
        if (useMultipleTargets == 'y') {
            success = trade->calculateWithMultipleTargets();
        } else {
            success = trade->calculate();
        }
        
        if (!success) {
            Utils::printError("Failed to calculate trade. Please check your inputs.");
            return;
        }
        
        // Display results
        displayTradeResults(trade);
        
        // Offer to simulate the trade outcome
        char simulateOutcome = Utils::getYesNoInput("Do you want to simulate this trade's outcome? (y/n): ");
        if (simulateOutcome == 'y') {
            Utils::printInfo("Select the outcome of this trade:");
            std::cout << "1. Loss at Stop Loss\n";
            std::cout << "2. Win at Take Profit\n";
            if (trade->getResults().hasMultipleTargets) {
                std::cout << "3. Win at TP1 only\n";
                std::cout << "4. Win at TP2\n";
                std::cout << "5. Break Even\n";
            } else {
                std::cout << "3. Break Even\n";
            }
            
            int outcomeChoice;
            if (trade->getResults().hasMultipleTargets) {
                outcomeChoice = Utils::getValidInput<int>("Enter your choice: ", 1, 5, true);
            } else {
                outcomeChoice = Utils::getValidInput<int>("Enter your choice: ", 1, 3, true);
            }
            
            TradeOutcome outcome;
            switch (outcomeChoice) {
                case 1:
                    outcome = TradeOutcome::LossAtSL;
                    break;
                case 2:
                    if (trade->getResults().hasMultipleTargets) {
                        outcome = TradeOutcome::WinAtTP2;
                    } else {
                        outcome = TradeOutcome::WinAtTP1;
                    }
                    break;
                case 3:
                    if (trade->getResults().hasMultipleTargets) {
                        outcome = TradeOutcome::WinAtTP1;
                    } else {
                        outcome = TradeOutcome::BreakEven;
                    }
                    break;
                case 4:
                    outcome = TradeOutcome::WinAtTP2;
                    break;
                case 5:
                    outcome = TradeOutcome::BreakEven;
                    break;
                default:
                    outcome = TradeOutcome::Pending;
                    break;
            }
            
            sessionManager.simulateTrade(trade, outcome);
            
            // Display updated results
            Utils::clearScreen();
            UI::displayHeader("TRADE SIMULATION RESULTS");
            std::cout << trade->getSummary() << std::endl;
        }
        
        // Add the trade to the session
        sessionManager.addTrade(trade);
        
        // Save trade
        if (sessionManager.getAutoSave()) {
            Utils::printSuccess("Trade automatically saved to session.");
        } else {
            char saveTrade = Utils::getYesNoInput("Do you want to save this trade? (y/n): ");
            if (saveTrade == 'y') {
                trade->save(sessionManager.getSessionFile());
                Utils::printSuccess("Trade saved to " + sessionManager.getSessionFile());
            }
        }
    }
} 