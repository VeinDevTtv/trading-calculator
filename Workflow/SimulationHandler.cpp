#include "SimulationHandler.h"
#include "../UI/Menu.h"
#include "../Utils/InputHandler.h"
#include "../Utils.h"
#include "../Risk/RiskCurveGenerator.h"
#include <iostream>
#include <iomanip>

namespace Workflow {
    void simulationMode(SessionManager& sessionManager) {
        Utils::clearScreen();
        UI::displayHeader("RISK SIMULATION MODE");
        
        std::cout << "Select simulation type:\n";
        std::cout << "1. Risk Curve Generator\n";
        std::cout << "2. Trade Sequence Simulator\n";
        std::cout << "3. Monte Carlo Analysis\n";
        
        int choice = Utils::getValidInput<int>("Enter your choice: ", 1, 3, true);
        
        switch (choice) {
            case 1: {
                // Risk Curve Generator
                Utils::clearScreen();
                UI::displayHeader("RISK CURVE GENERATOR");
                
                // Create and configure the risk curve generator
                Risk::RiskCurveGenerator curveGenerator;
                Risk::RiskSimulationParams params;
                
                // Get user input for simulation parameters
                params.initialBalance = Utils::getValidInput<double>("Initial Balance ($): ", 1.0, 10000000.0, true);
                params.numTrades = Utils::getValidInput<int>("Number of Trades: ", 10, 10000, true);
                params.winRate = Utils::getValidInput<double>("Win Rate (%): ", 1.0, 99.0, true) / 100.0;
                params.riskRewardRatio = Utils::getValidInput<double>("Risk/Reward Ratio: ", 0.1, 10.0, true);
                params.maxRiskPerTrade = Utils::getValidInput<double>("Max Risk Per Trade (%): ", 0.1, 10.0, true);
                
                // Risk strategy selection
                std::cout << "\nSelect risk strategy:\n";
                std::cout << "1. Fixed Percentage\n";
                std::cout << "2. Compounding\n";
                std::cout << "3. Kelly Criterion\n";
                
                int strategyChoice = Utils::getValidInput<int>("Enter your choice: ", 1, 3, true);
                
                if (strategyChoice == 1) {
                    params.strategy = Risk::RiskStrategy::FIXED;
                } else if (strategyChoice == 2) {
                    params.strategy = Risk::RiskStrategy::COMPOUNDING;
                } else {
                    params.strategy = Risk::RiskStrategy::KELLY_CRITERION;
                }
                
                // Set simulation parameters
                curveGenerator.setSimulationParams(params);
                
                // Set risk profile based on strategy
                if (params.strategy == Risk::RiskStrategy::KELLY_CRITERION) {
                    curveGenerator.setRiskProfile(Risk::RiskProfile::createAggressive());
                } else if (params.strategy == Risk::RiskStrategy::COMPOUNDING) {
                    curveGenerator.setRiskProfile(Risk::RiskProfile::createModerate());
                } else {
                    curveGenerator.setRiskProfile(Risk::RiskProfile::createConservative());
                }
                
                std::cout << "\nGenerating risk curve...\n";
                
                // Run simulation
                auto results = curveGenerator.generateCurve();
                
                // Display results
                Utils::clearScreen();
                UI::displayHeader("RISK SIMULATION RESULTS");
                
                std::cout << "Initial Balance: $" << std::fixed << std::setprecision(2) 
                         << params.initialBalance << "\n";
                std::cout << "Final Balance:   $" << results.finalBalance << "\n";
                std::cout << "Net Profit:      $" << (results.finalBalance - params.initialBalance) 
                         << " (" << ((results.finalBalance - params.initialBalance) / params.initialBalance * 100.0) 
                         << "%)\n";
                std::cout << "Max Drawdown:    " << std::setprecision(2) << results.maxDrawdownPercent << "%\n";
                std::cout << "Profit Factor:   " << std::setprecision(3) << results.profitFactor << "\n";
                std::cout << "Sharpe Ratio:    " << results.sharpeRatio << "\n";
                std::cout << "Longest Losing Streak: " << results.maxConsecutiveLosses << " trades\n\n";
                
                // Display equity curve chart
                std::cout << "Equity Curve:\n";
                std::cout << curveGenerator.getASCIIChart() << "\n\n";
                
                // Option to export results
                char exportResults = Utils::getYesNoInput("Export results to CSV? (y/n): ");
                if (exportResults == 'y') {
                    std::string filename;
                    std::cout << "Enter filename (without extension): ";
                    std::cin >> filename;
                    
                    if (curveGenerator.exportToCSV(filename + ".csv")) {
                        Utils::printSuccess("Results exported to " + filename + ".csv");
                    } else {
                        Utils::printError("Failed to export results.");
                    }
                }
                break;
            }
            
            case 2: {
                // Trade Sequence Simulator
                Utils::clearScreen();
                UI::displayHeader("TRADE SEQUENCE SIMULATOR");
                std::cout << "Trade sequence simulator is not yet implemented.\n";
                break;
            }
            
            case 3: {
                // Monte Carlo Analysis
                Utils::clearScreen();
                UI::displayHeader("MONTE CARLO ANALYSIS");
                std::cout << "Monte Carlo analysis is not yet implemented.\n";
                break;
            }
        }
    }
} 