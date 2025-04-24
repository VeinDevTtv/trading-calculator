#include "MainMenu.h"
#include "UI/ConsoleUI.h"
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
    auto& ui = UI::ConsoleUI::getInstance();
    
    // Load configuration or set defaults
    TradeParameters defaultParams;
    Utils::loadConfig(defaultParams);
    
    // Welcome banner
    ui.clearScreen();
    ui.printHeader("ADVANCED TRADING RISK CALCULATOR");
    ui.printInfo("Welcome to the Advanced Trading Risk Calculator v2.0");
    ui.printInfo("This application helps you calculate optimal position sizes");
    ui.printInfo("and manage risk for your trading activities.");
    ui.getInput("Press Enter to continue...");
    
    // Initialize session
    initializeSession();
    
    // Main program loop
    while (m_running) {
        ui.clearScreen();
        displayMainMenu();
        
        int choice = ui.getValidInput<int>("Enter your choice: ", 1, 7, true);
        handleUserChoice(choice);
        
        if (m_running && choice != 7) {
            ui.getInput("\nPress Enter to continue...");
        }
    }
    
    ui.clearScreen();
    ui.printSuccess("Thank you for using the Advanced Trading Risk Calculator!");
}

void MainMenu::displayMainMenu() const {
    auto& ui = UI::ConsoleUI::getInstance();
    ui.printHeader("MAIN MENU");
    
    std::vector<std::string> options = {
        "Calculate New Trade",
        "View Saved Trades",
        "Simulation Mode",
        "Session Statistics",
        "Settings",
        "Display Equity Curve",
        "Exit"
    };
    
    ui.displayMenu(options);
}

void MainMenu::handleUserChoice(int choice) {
    auto& ui = UI::ConsoleUI::getInstance();
    
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
            ui.printError("Invalid choice. Please try again.");
            break;
    }
}

void MainMenu::initializeSession() {
    auto& ui = UI::ConsoleUI::getInstance();
    double initialBalance = ui.getValidInput<double>(
        "Enter your starting account balance: $", 
        1.0, 
        1000000000.0, 
        true
    );
    m_sessionManager->startNewSession(initialBalance);
    ui.printSuccess("Session initialized with balance: $" + std::to_string(initialBalance));
}

void MainMenu::saveSessionIfNeeded() {
    auto& ui = UI::ConsoleUI::getInstance();
    if (m_sessionManager->isSessionActive()) {
        char saveSession = ui.getYesNoInput("Do you want to save this session? (y/n): ");
        if (saveSession == 'y') {
            m_sessionManager->saveSession();
            m_sessionManager->saveSessionAsJson();
            ui.printSuccess("Session saved!");
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