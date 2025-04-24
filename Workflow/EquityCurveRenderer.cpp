#include "EquityCurveRenderer.h"
#include "../UI/Menu.h"
#include "../Utils.h"
#include "../Utils/InputHandler.h"
#include <iostream>
#include <iomanip>
#include <vector>

namespace Workflow {
    EquityCurveRenderer::EquityCurveRenderer(const SessionManager& sessionManager)
        : m_sessionManager(sessionManager) {}
    
    void EquityCurveRenderer::displayEquityCurve(int width, int height) const {
        Utils::clearScreen();
        UI::displayHeader("EQUITY CURVE");
        
        // Get all trades
        auto trades = m_sessionManager.getAllTrades();
        
        if (trades.empty()) {
            std::cout << "No trades available to display equity curve.\n";
            return;
        }
        
        // Generate equity curve data
        double initialBalance = m_sessionManager.getCurrentBalance();
        std::vector<double> equityCurve;
        equityCurve.push_back(initialBalance);
        
        double currentBalance = initialBalance;
        for (const auto& trade : trades) {
            auto outcome = trade->getOutcome();
            
            if (outcome == TradeOutcome::WinAtTP1 || outcome == TradeOutcome::WinAtTP2) {
                currentBalance += trade->getResults().rewardAmount;
            } else if (outcome == TradeOutcome::LossAtSL) {
                currentBalance -= trade->getResults().riskAmount;
            }
            
            equityCurve.push_back(currentBalance);
        }
        
        // Display ASCII chart
        std::cout << generateASCIIChart(width, height) << "\n\n";
        
        // Display basic statistics
        std::cout << "Starting Balance: $" << std::fixed << std::setprecision(2) 
                 << initialBalance << "\n";
        std::cout << "Final Balance:    $" << currentBalance << "\n";
        std::cout << "Net P&L:          $" << (currentBalance - initialBalance) << " (" 
                 << ((currentBalance - initialBalance) / initialBalance * 100.0) << "%)\n";
        std::cout << "Total Trades:     " << trades.size() << "\n\n";
        
        // Option to display extended statistics
        char showExtended = Utils::getYesNoInput("Display extended statistics? (y/n): ");
        if (showExtended == 'y') {
            displayExtendedStats();
        }
        
        // Option to export
        char exportCurve = Utils::getYesNoInput("Export equity curve? (y/n): ");
        if (exportCurve == 'y') {
            std::cout << "\nExport format:\n";
            std::cout << "1. CSV\n";
            std::cout << "2. PNG\n";
            
            int exportChoice = Utils::getValidInput<int>("Select format: ", 1, 2, true);
            
            std::string filename;
            std::cout << "Enter filename (without extension): ";
            std::cin >> filename;
            
            bool success = false;
            if (exportChoice == 1) {
                success = exportToCSV(filename + ".csv");
            } else {
                success = exportToPNG(filename + ".png");
            }
            
            if (success) {
                Utils::printSuccess("Equity curve exported successfully.");
            } else {
                Utils::printError("Failed to export equity curve.");
            }
        }
    }
    
    void EquityCurveRenderer::displayExtendedStats() const {
        UI::displayHeader("EXTENDED STATISTICS");
        
        // Use Analytics to calculate extended stats
        Analytics::EquityAnalyzer analyzer;
        auto trades = m_sessionManager.getAllTrades();
        auto stats = analyzer.calculateStats(trades, m_sessionManager.getCurrentBalance());
        
        // Display extended stats report
        std::cout << analyzer.getStatsReport(stats) << std::endl;
    }
    
    bool EquityCurveRenderer::exportToCSV(const std::string& filename) const {
        // Stub implementation
        std::cout << "Exporting to CSV is not yet implemented." << std::endl;
        return false;
    }
    
    bool EquityCurveRenderer::exportToPNG(const std::string& filename) const {
        // Stub implementation
        std::cout << "Exporting to PNG is not yet implemented." << std::endl;
        return false;
    }
    
    std::string EquityCurveRenderer::generateASCIIChart(int width, int height) const {
        auto trades = m_sessionManager.getAllTrades();
        
        if (trades.empty()) {
            return "No data to display.";
        }
        
        // Generate equity curve data
        double initialBalance = m_sessionManager.getCurrentBalance();
        std::vector<double> equityCurve;
        equityCurve.push_back(initialBalance);
        
        double currentBalance = initialBalance;
        for (const auto& trade : trades) {
            auto outcome = trade->getOutcome();
            
            if (outcome == TradeOutcome::WinAtTP1 || outcome == TradeOutcome::WinAtTP2) {
                currentBalance += trade->getResults().rewardAmount;
            } else if (outcome == TradeOutcome::LossAtSL) {
                currentBalance -= trade->getResults().riskAmount;
            }
            
            equityCurve.push_back(currentBalance);
        }
        
        return Utils::generateASCIIChart(equityCurve, width, height);
    }
    
    // Function wrapper for backward compatibility
    void displayEquityCurve(const SessionManager& sessionManager) {
        EquityCurveRenderer renderer(sessionManager);
        renderer.displayEquityCurve();
    }
} 