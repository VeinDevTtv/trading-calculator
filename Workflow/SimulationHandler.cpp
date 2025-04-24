#include "SimulationHandler.h"
#include "../UI/Menu.h"
#include "../Utils/InputHandler.h"
#include "../Utils.h"
#include "../Risk/RiskCurveGenerator.h"
#include <iostream>
#include <iomanip>

namespace Workflow {
    void simulationMode(SessionManager& sessionManager) {
        Utils::printHeader("SIMULATION MODE");
        
        auto lastTrade = sessionManager.getLastTrade();
        if (!lastTrade) {
            Utils::printError("No trades available for simulation. Please create a trade first.");
            return;
        }
        
        Utils::printInfo("Last Trade Summary:");
        std::cout << lastTrade->getSummary() << std::endl;
        
        Utils::printInfo("Select simulation outcome:");
        std::cout << "1. Loss at Stop Loss\n";
        std::cout << "2. Win at Take Profit\n";
        
        if (lastTrade->getResults().hasMultipleTargets) {
            std::cout << "3. Win at TP1 only\n";
            std::cout << "4. Win at TP2\n";
            std::cout << "5. Break Even\n";
        } else {
            std::cout << "3. Break Even\n";
        }
        
        int outcomeChoice;
        if (lastTrade->getResults().hasMultipleTargets) {
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
                if (lastTrade->getResults().hasMultipleTargets) {
                    outcome = TradeOutcome::WinAtTP2;
                } else {
                    outcome = TradeOutcome::WinAtTP1;
                }
                break;
            case 3:
                if (lastTrade->getResults().hasMultipleTargets) {
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
        
        sessionManager.simulateTrade(lastTrade, outcome);
        
        Utils::clearScreen();
        Utils::printHeader("SIMULATION RESULTS");
        std::cout << lastTrade->getSummary() << std::endl;
        
        // Show updated account balance
        Utils::printInfo("Updated Session Info:");
        std::cout << "Current Balance: $" << sessionManager.getCurrentBalance() << std::endl;
        
        // Option to save simulation
        char saveSim = Utils::getYesNoInput("Save this simulation result? (y/n): ");
        if (saveSim == 'y') {
            if (sessionManager.getAutoSave()) {
                Utils::printSuccess("Simulation results automatically saved to session.");
            } else {
                lastTrade->save(sessionManager.getSessionFile());
                Utils::printSuccess("Simulation results saved to " + sessionManager.getSessionFile());
            }
        }
    }
} 