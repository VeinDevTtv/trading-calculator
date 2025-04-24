#include "MainMenu.h"
#include "Utils.h"
#include "Workflow/NewTradeWorkflow.h"
#include "Workflow/ViewSavedTrades.h"
#include "Workflow/SimulationHandler.h"
#include "Workflow/StatsHandler.h"
#include "Workflow/SettingsHandler.h"
#include "Workflow/EquityCurveRenderer.h"
#include <iostream>
#include <limits>

namespace Workflow {

MainMenu::MainMenu() : m_running(true) {
    m_sessionManager = std::make_unique<SessionManager>();
}

MainMenu::~MainMenu() = default;

void MainMenu::run() {
    // Load configuration or set defaults
    TradeParameters defaultParams;
    Utils::loadConfig(defaultParams);
    
    // Welcome banner
    Utils::printHeader("ADVANCED TRADING RISK CALCULATOR");
    std::cout << "Welcome to the Advanced Trading Risk Calculator v2.0\n\n";
    std::cout << "This application helps you calculate optimal position sizes\n";
    std::cout << "and manage risk for your trading activities.\n\n";
    std::cout << "Press Enter to continue...";
    std::cin.get();
    
    // Initialize session
    initializeSession();
    
    // Main program loop
    while (m_running) {
        Utils::clearScreen();
        displayMainMenu();
        
        int choice = Utils::getValidInput<int>("Enter your choice: ", 1, 7, true);
        handleUserChoice(choice);
        
        if (m_running && choice != 7) {
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin.get();
        }
    }
    
    Utils::clearScreen();
    std::cout << "Thank you for using the Advanced Trading Risk Calculator!\n";
}

void MainMenu::displayMainMenu() const {
    Utils::printHeader("MAIN MENU");
    
    std::cout << "1. Calculate New Trade\n";
    std::cout << "2. View Saved Trades\n";
    std::cout << "3. Simulation Mode\n";
    std::cout << "4. Session Statistics\n";
    std::cout << "5. Settings\n";
    std::cout << "6. Display Equity Curve\n";
    std::cout << "7. Exit\n\n";
}

void MainMenu::handleUserChoice(int choice) {
    switch (choice) {
        case 1:
            newTradeWorkflow();
            break;
        case 2:
            viewSavedTrades();
            break;
        case 3:
            simulationMode();
            break;
        case 4:
            sessionStatsMode();
            break;
        case 5:
            configureSettings();
            break;
        case 6:
            displayEquityCurve();
            break;
        case 7:
            exitProgram();
            break;
        default:
            Utils::printError("Invalid choice. Please try again.");
            break;
    }
}

void MainMenu::initializeSession() {
    double initialBalance = Utils::getValidInput<double>(
        "Enter your starting account balance: $", 
        1.0, 
        1000000000.0, 
        true
    );
    m_sessionManager->startNewSession(initialBalance);
}

void MainMenu::saveSessionIfNeeded() {
    if (m_sessionManager->isSessionActive()) {
        char saveSession = Utils::getYesNoInput("Do you want to save this session? (y/n): ");
        if (saveSession == 'y') {
            m_sessionManager->saveSession();
            m_sessionManager->saveSessionAsJson();
            Utils::printSuccess("Session saved!");
        }
        m_sessionManager->endSession();
    }
}

void MainMenu::newTradeWorkflow() {
    Workflow::newTradeWorkflow(*m_sessionManager);
}

void MainMenu::viewSavedTrades() {
    Workflow::viewSavedTrades(*m_sessionManager);
}

void MainMenu::simulationMode() {
    Workflow::simulationMode(*m_sessionManager);
}

void MainMenu::sessionStatsMode() {
    Workflow::sessionStatsMode(*m_sessionManager);
}

void MainMenu::configureSettings() {
    Workflow::configureSettings(*m_sessionManager);
}

void MainMenu::displayEquityCurve() {
    Workflow::displayEquityCurve(*m_sessionManager);
}

void MainMenu::exitProgram() {
    saveSessionIfNeeded();
    m_running = false;
}

} // namespace Workflow 