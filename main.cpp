#include <iostream>
#include <string>
#include <iomanip>
#include <limits>
#include "TradeCalculator.h"
#include "Utils.h"

// Function prototypes
void displayHeader();
void displayMenu();
TradeParameters getTradeParameters();
void displayResults(const TradeResults& results);
template<typename T> T getValidInput(const std::string& prompt, T min = T(), T max = T(), bool hasRange = false);

int main() {
    bool running = true;
    
    // Try to load config
    TradeParameters defaultParams;
    Utils::loadConfig(defaultParams);
    
    while (running) {
        displayHeader();
        displayMenu();
        
        int choice = getValidInput<int>("Enter your choice: ", 1, 3, true);
        
        switch (choice) {
            case 1: {
                // New Trade Calculation
                TradeParameters params = getTradeParameters();
                TradeCalculator calculator;
                TradeResults results = calculator.calculateTrade(params);
                displayResults(results);
                
                // Option to save results
                std::cout << "\nDo you want to save this trade? (y/n): ";
                char save;
                std::cin >> save;
                if (save == 'y' || save == 'Y') {
                    Utils::saveTradeToFile(params, results);
                    std::cout << "Trade saved successfully!\n";
                    
                    // Ask if user wants to save as default configuration
                    std::cout << "Save this account balance and risk as default? (y/n): ";
                    char saveDefault;
                    std::cin >> saveDefault;
                    if (saveDefault == 'y' || saveDefault == 'Y') {
                        Utils::saveConfig(params);
                        std::cout << "Default configuration saved.\n";
                    }
                }
                
                std::cout << "\nPress Enter to continue...";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cin.get();
                break;
            }
            case 2: {
                // View Saved Trades
                Utils::displaySavedTrades();
                std::cout << "\nPress Enter to continue...";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cin.get();
                break;
            }
            case 3:
                // Exit
                running = false;
                break;
            default:
                std::cout << "Invalid choice. Please try again.\n";
                break;
        }
    }
    
    return 0;
}

void displayHeader() {
    system("cls"); // Clear screen (Windows)
    std::cout << "\033[1;36m"; // Cyan color, bold
    std::cout << "====================================\n";
    std::cout << "        TRADING RISK CALCULATOR     \n";
    std::cout << "====================================\n";
    std::cout << "\033[0m"; // Reset color
}

void displayMenu() {
    std::cout << "\n1. Calculate New Trade\n";
    std::cout << "2. View Saved Trades\n";
    std::cout << "3. Exit\n\n";
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
            std::cout << "Invalid input. Please try again.\n";
        }
        else if (hasRange && (value < min || value > max)) {
            std::cout << "Value must be between " << min << " and " << max << ". Please try again.\n";
        }
        else {
            valid = true;
        }
    } while (!valid);
    
    // Clear the input buffer
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    
    return value;
}

TradeParameters getTradeParameters() {
    TradeParameters params;
    
    // Try to load defaults
    Utils::loadConfig(params);
    
    std::cout << "\n--- Enter Trade Parameters ---\n\n";
    
    params.accountBalance = getValidInput<double>("Account Balance: $", 1.0, 1000000000.0, true);
    params.riskPercent = getValidInput<double>("Risk per trade (%): ", 0.01, 100.0, true);
    params.stopLossInPips = getValidInput<double>("Stop-Loss (pips): ", 0.1, 10000.0, true);
    
    std::cout << "Use Risk-Reward ratio? (y/n): ";
    char useRR;
    std::cin >> useRR;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    
    if (useRR == 'y' || useRR == 'Y') {
        params.riskRewardRatio = getValidInput<double>("Risk-Reward ratio: ", 0.1, 100.0, true);
        params.takeProfitInPips = params.stopLossInPips * params.riskRewardRatio;
    } else {
        params.takeProfitInPips = getValidInput<double>("Take-Profit (pips): ", 0.1, 10000.0, true);
        params.riskRewardRatio = params.takeProfitInPips / params.stopLossInPips;
    }
    
    params.entryPrice = getValidInput<double>("Entry Price: ", 0.00001, 1000000.0, true);
    
    int instrumentChoice = getValidInput<int>("Instrument type (1=Forex, 2=Gold, 3=Indices): ", 1, 3, true);
    switch (instrumentChoice) {
        case 1:
            params.instrumentType = InstrumentType::Forex;
            break;
        case 2:
            params.instrumentType = InstrumentType::Gold;
            break;
        case 3:
            params.instrumentType = InstrumentType::Indices;
            break;
    }
    
    int lotSizeChoice = getValidInput<int>("Lot size type (1=Standard, 2=Mini, 3=Micro): ", 1, 3, true);
    switch (lotSizeChoice) {
        case 1:
            params.lotSizeType = LotSizeType::Standard;
            break;
        case 2:
            params.lotSizeType = LotSizeType::Mini;
            break;
        case 3:
            params.lotSizeType = LotSizeType::Micro;
            break;
    }
    
    std::cout << "Override SL price? (y/n): ";
    char overrideSL;
    std::cin >> overrideSL;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    
    if (overrideSL == 'y' || overrideSL == 'Y') {
        params.stopLossPrice = getValidInput<double>("SL Price: ", 0.00001, 1000000.0, true);
        params.isStopLossPriceOverride = true;
    } else {
        params.isStopLossPriceOverride = false;
    }
    
    std::cout << "Use custom contract size? (y/n): ";
    char useCustomContract;
    std::cin >> useCustomContract;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    
    if (useCustomContract == 'y' || useCustomContract == 'Y') {
        params.contractSize = getValidInput<double>("Contract Size: ", 0.01, 1000000.0, true);
    } else {
        params.contractSize = 0; // Default value will be set in the calculator
    }
    
    return params;
}

void displayResults(const TradeResults& results) {
    std::cout << "\n\033[1;32m"; // Green, bold
    std::cout << "=== TRADE CALCULATION RESULTS ===\n";
    std::cout << "\033[0m"; // Reset
    
    std::cout << std::fixed << std::setprecision(2);
    
    std::cout << "\033[1;31m"; // Red for risk
    std::cout << "Risk Amount: $" << results.riskAmount << "\n";
    std::cout << "\033[0m"; // Reset
    
    std::cout << "\033[1;32m"; // Green for reward
    std::cout << "Potential Reward: $" << results.rewardAmount << "\n";
    std::cout << "\033[0m"; // Reset
    
    std::cout << "Risk-to-Reward Ratio: 1:" << results.riskRewardRatio << "\n";
    std::cout << "Required Position Size: " << results.positionSize << " lots\n";
    std::cout << "SL Price Level: " << results.stopLossPrice << "\n";
    std::cout << "TP Price Level: " << results.takeProfitPrice << "\n";
    
    if (results.hasBreakEvenInfo) {
        std::cout << "\nBreak-Even Point: " << results.breakEvenPrice << "\n";
        std::cout << "Break-Even Pips: " << results.breakEvenPips << "\n";
    }
    
    if (results.hasMultipleTargets) {
        std::cout << "\n=== Multiple Targets ===\n";
        std::cout << "TP1 Price: " << results.tp1Price << " (Reward: $" << results.tp1Amount << ")\n";
        std::cout << "TP2 Price: " << results.tp2Price << " (Reward: $" << results.tp2Amount << ")\n";
    }
} 