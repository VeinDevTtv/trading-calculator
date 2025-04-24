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
        Utils::printHeader("NEW TRADE CALCULATION");
        
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
            Utils::printHeader("TRADE SIMULATION RESULTS");
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

    void configureTradeParameters(std::shared_ptr<Trade> trade) {
        // Get basic parameters
        double riskPercent = Utils::getValidInput<double>("Risk per trade (%): ", 0.01, 100.0, true);
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
        int lotSizeChoice = Utils::getValidInput<int>("Enter your choice: ", 1, 3, true);
        trade->setLotSizeType(lotSizeChoice - 1);
        
        // Custom contract size
        char useCustomContract = Utils::getYesNoInput("Use custom contract size? (y/n): ");
        if (useCustomContract == 'y') {
            double contractSize = Utils::getValidInput<double>("Contract Size: ", 0.01, 1000000.0, true);
            trade->setContractSize(contractSize);
        }
        
        // Stop Loss
        std::cout << "\nHow would you like to specify Stop Loss?\n";
        std::cout << "1. In pips\n";
        std::cout << "2. As price level\n";
        int slChoice = Utils::getValidInput<int>("Enter your choice: ", 1, 2, true);
        
        if (slChoice == 1) {
            double stopLossInPips = Utils::getValidInput<double>("Stop-Loss (pips): ", 0.1, 10000.0, true);
            trade->setStopLoss(stopLossInPips, InputType::Pips);
        } else {
            double stopLossPrice = Utils::getValidInput<double>("Stop-Loss price: ", 0.00001, 1000000.0, true);
            trade->setStopLoss(stopLossPrice, InputType::Price);
        }
        
        // Take Profit
        std::cout << "\nHow would you like to specify Take Profit?\n";
        std::cout << "1. In pips\n";
        std::cout << "2. As price level\n";
        std::cout << "3. As risk-reward ratio\n";
        int tpChoice = Utils::getValidInput<int>("Enter your choice: ", 1, 3, true);
        
        if (tpChoice == 1) {
            double takeProfitInPips = Utils::getValidInput<double>("Take-Profit (pips): ", 0.1, 10000.0, true);
            trade->setTakeProfit(takeProfitInPips, InputType::Pips);
        } else if (tpChoice == 2) {
            double takeProfitPrice = Utils::getValidInput<double>("Take-Profit price: ", 0.00001, 1000000.0, true);
            trade->setTakeProfit(takeProfitPrice, InputType::Price);
        } else {
            double rrRatio = Utils::getValidInput<double>("Risk-Reward ratio: ", 0.1, 100.0, true);
            
            // Get stop loss in pips
            auto params = trade->getParameters();
            double slPips = params.stopLossInPips;
            double tpPips = slPips * rrRatio;
            
            trade->setTakeProfit(tpPips, InputType::Pips);
        }
    }

    void displayTradeResults(std::shared_ptr<Trade> trade) {
        Utils::clearScreen();
        Utils::printHeader("TRADE CALCULATION RESULTS");
        
        std::cout << trade->getSummary() << std::endl;
    }
} 