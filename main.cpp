#include <iostream>
#include <string>
#include <iomanip>
#include <limits>
#include <memory>
#include <vector>
#include "TradeCalculator.h"
#include "Utils.h"
#include "Trade.h"
#include "SessionManager.h"
#include "UI/Menu.h"
#include "Utils/InputHandler.h"
#include "Workflow/NewTradeWorkflow.h"
#include "Workflow/ViewSavedTrades.h"
#include "Workflow/SimulationHandler.h"
#include "Workflow/StatsHandler.h"
#include "Workflow/SettingsHandler.h"
#include "Workflow/EquityCurveRenderer.h"
#include "Workflow/MainMenu.h"

int main() {
    try {
        Workflow::MainMenu menu;
        menu.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}

void displayMainMenu() {
    Utils::printHeader("MAIN MENU");
    
    std::cout << "1. Calculate New Trade\n";
    std::cout << "2. View Saved Trades\n";
    std::cout << "3. Simulation Mode\n";
    std::cout << "4. Session Statistics\n";
    std::cout << "5. Settings\n";
    std::cout << "6. Display Equity Curve\n";
    std::cout << "7. Exit\n\n";
}

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
    char useMultipleTargets = getYesNoInput("Do you want to use multiple take-profit targets? (y/n): ");
    
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
    char simulateOutcome = getYesNoInput("Do you want to simulate this trade's outcome? (y/n): ");
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
            outcomeChoice = getValidInput<int>("Enter your choice: ", 1, 5, true);
        } else {
            outcomeChoice = getValidInput<int>("Enter your choice: ", 1, 3, true);
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
        char saveTrade = getYesNoInput("Do you want to save this trade? (y/n): ");
        if (saveTrade == 'y') {
            trade->save(sessionManager.getSessionFile());
            Utils::printSuccess("Trade saved to " + sessionManager.getSessionFile());
        }
    }
}

void configureTradeParameters(std::shared_ptr<Trade> trade) {
    // Get basic parameters
    double riskPercent = getValidInput<double>("Risk per trade (%): ", 0.01, 100.0, true);
    trade->setRiskPercentage(riskPercent);
    
    double entryPrice = getValidInput<double>("Entry Price: ", 0.00001, 1000000.0, true);
    trade->setEntryPrice(entryPrice);
    
    // Instrument selection
    std::cout << "\nSelect instrument type:\n";
    std::cout << "1. Forex\n";
    std::cout << "2. Gold\n";
    std::cout << "3. Indices\n";
    int instrumentChoice = getValidInput<int>("Enter your choice: ", 1, 3, true);
    trade->setInstrumentType(instrumentChoice - 1);
    
    // Lot size selection
    std::cout << "\nSelect lot size type:\n";
    std::cout << "1. Standard (100,000)\n";
    std::cout << "2. Mini (10,000)\n";
    std::cout << "3. Micro (1,000)\n";
    int lotSizeChoice = getValidInput<int>("Enter your choice: ", 1, 3, true);
    trade->setLotSizeType(lotSizeChoice - 1);
    
    // Custom contract size
    char useCustomContract = getYesNoInput("Use custom contract size? (y/n): ");
    if (useCustomContract == 'y') {
        double contractSize = getValidInput<double>("Contract Size: ", 0.01, 1000000.0, true);
        trade->setContractSize(contractSize);
    }
    
    // Stop Loss
    std::cout << "\nHow would you like to specify Stop Loss?\n";
    std::cout << "1. In pips\n";
    std::cout << "2. As price level\n";
    int slChoice = getValidInput<int>("Enter your choice: ", 1, 2, true);
    
    if (slChoice == 1) {
        double stopLossInPips = getValidInput<double>("Stop-Loss (pips): ", 0.1, 10000.0, true);
        trade->setStopLoss(stopLossInPips, InputType::Pips);
    } else {
        double stopLossPrice = getValidInput<double>("Stop-Loss price: ", 0.00001, 1000000.0, true);
        trade->setStopLoss(stopLossPrice, InputType::Price);
    }
    
    // Take Profit
    std::cout << "\nHow would you like to specify Take Profit?\n";
    std::cout << "1. In pips\n";
    std::cout << "2. As price level\n";
    std::cout << "3. As risk-reward ratio\n";
    int tpChoice = getValidInput<int>("Enter your choice: ", 1, 3, true);
    
    if (tpChoice == 1) {
        double takeProfitInPips = getValidInput<double>("Take-Profit (pips): ", 0.1, 10000.0, true);
        trade->setTakeProfit(takeProfitInPips, InputType::Pips);
    } else if (tpChoice == 2) {
        double takeProfitPrice = getValidInput<double>("Take-Profit price: ", 0.00001, 1000000.0, true);
        trade->setTakeProfit(takeProfitPrice, InputType::Price);
    } else {
        double rrRatio = getValidInput<double>("Risk-Reward ratio: ", 0.1, 100.0, true);
        
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
    char viewDetailed = getYesNoInput("View detailed trade? (y/n): ");
    
    if (viewDetailed == 'y') {
        int tradeIndex = getValidInput<int>("Enter trade number: ", 1, static_cast<int>(trades.size()), true);
        
        Utils::clearScreen();
        Utils::printHeader("TRADE DETAILS");
        
        auto selectedTrade = trades[tradeIndex - 1];
        std::cout << selectedTrade->getSummary() << std::endl;
    }
}

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
        outcomeChoice = getValidInput<int>("Enter your choice: ", 1, 5, true);
    } else {
        outcomeChoice = getValidInput<int>("Enter your choice: ", 1, 3, true);
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
    char saveSim = getYesNoInput("Save this simulation result? (y/n): ");
    if (saveSim == 'y') {
        if (sessionManager.getAutoSave()) {
            Utils::printSuccess("Simulation results automatically saved to session.");
        } else {
            lastTrade->save(sessionManager.getSessionFile());
            Utils::printSuccess("Simulation results saved to " + sessionManager.getSessionFile());
        }
    }
}

void sessionStatsMode(SessionManager& sessionManager) {
    Utils::printHeader("SESSION STATISTICS");
    
    if (!sessionManager.isSessionActive()) {
        Utils::printError("No active session. Please start a session first.");
        return;
    }
    
    std::cout << sessionManager.getSessionSummary() << std::endl;
    
    // Option to save session
    char saveSession = getYesNoInput("Save current session? (y/n): ");
    if (saveSession == 'y') {
        sessionManager.saveSession();
        sessionManager.saveSessionAsJson();
        Utils::printSuccess("Session saved!");
    }
}

void configureSettings(SessionManager& sessionManager) {
    Utils::printHeader("SETTINGS");
    
    bool done = false;
    while (!done) {
        std::cout << "1. Toggle Auto-Save (current: " << (sessionManager.getAutoSave() ? "ON" : "OFF") << ")\n";
        std::cout << "2. Change Session File (current: " << sessionManager.getSessionFile() << ")\n";
        std::cout << "3. Return to Main Menu\n\n";
        
        int choice = getValidInput<int>("Enter your choice: ", 1, 3, true);
        
        switch (choice) {
            case 1: {
                bool currentSetting = sessionManager.getAutoSave();
                sessionManager.setAutoSave(!currentSetting);
                Utils::printSuccess(std::string("Auto-Save turned ") + (!currentSetting ? "ON" : "OFF"));
                break;
            }
            case 2: {
                std::cout << "Enter new session file name: ";
                std::string filename;
                std::cin >> filename;
                sessionManager.setSessionFile(filename);
                Utils::printSuccess("Session file changed to: " + filename);
                break;
            }
            case 3:
                done = true;
                break;
        }
        
        if (!done) {
            std::cout << std::endl;
        }
    }
}

void displayEquityCurve(const SessionManager& sessionManager) {
    Utils::printHeader("EQUITY CURVE");
    
    auto trades = sessionManager.getAllTrades();
    if (trades.empty()) {
        Utils::printInfo("No trades found in the current session.");
        return;
    }
    
    // Gather balance history
    std::vector<double> balanceHistory;
    balanceHistory.push_back(sessionManager.getSessionStats().initialBalance);
    
    for (const auto& trade : trades) {
        if (trade->getOutcome() != TradeOutcome::Pending) {
            balanceHistory.push_back(trade->getUpdatedAccountBalance());
        }
    }
    
    // Display some basic stats
    double initialBalance = balanceHistory.front();
    double currentBalance = balanceHistory.back();
    double totalPnL = currentBalance - initialBalance;
    double pnlPercent = (totalPnL / initialBalance) * 100.0;
    
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Initial Balance: $" << initialBalance << "\n";
    std::cout << "Current Balance: $" << currentBalance << "\n";
    std::cout << "Total P&L: " << (totalPnL >= 0 ? "+" : "") << "$" << totalPnL;
    std::cout << " (" << (totalPnL >= 0 ? "+" : "") << pnlPercent << "%)\n\n";
    
    // Generate and display ASCII chart
    std::cout << "Equity Curve:\n\n";
    std::cout << Utils::generateASCIIChart(balanceHistory, 50, 15) << std::endl;
}

// Template for getting valid input of various types with optional range checking
template<typename T>
T getValidInput(const std::string& prompt, T min, T max, bool hasRange) {
    T value;
    bool valid = false;
    
    do {
        std::cout << prompt;
        std::cin >> value;
        
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            Utils::printError("Invalid input. Please try again.");
        }
        else if (hasRange && (value < min || value > max)) {
            Utils::printError("Value must be between " + std::to_string(min) + " and " + std::to_string(max) + ". Please try again.");
        }
        else {
            valid = true;
        }
    } while (!valid);
    
    // Clear the input buffer
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    
    return value;
}

char getYesNoInput(const std::string& prompt) {
    char response;
    bool valid = false;
    
    do {
        std::cout << prompt;
        std::cin >> response;
        response = std::tolower(response);
        
        if (response != 'y' && response != 'n') {
            Utils::printError("Please enter 'y' or 'n'.");
            std::cin.clear();
        } else {
            valid = true;
        }
        
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } while (!valid);
    
    return response;
} 