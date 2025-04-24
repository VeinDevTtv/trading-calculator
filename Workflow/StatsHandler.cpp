#include "StatsHandler.h"
#include "../UI/Menu.h"
#include "../Utils/InputHandler.h"
#include "../Utils.h"
#include "../Analytics/EquityStats.h"
#include <iostream>
#include <iomanip>

namespace Workflow {
    void sessionStatsMode(SessionManager& sessionManager) {
        Utils::clearScreen();
        UI::displayHeader("SESSION STATISTICS");
        
        // Check if session has trades
        auto trades = sessionManager.getAllTrades();
        if (trades.empty()) {
            std::cout << "No trades in the current session.\n";
            return;
        }
        
        // Get session stats
        auto basicStats = sessionManager.getSessionStats();
        
        // Display basic stats
        std::cout << "=== BASIC STATISTICS ===\n\n";
        std::cout << "Starting Balance: $" << std::fixed << std::setprecision(2) 
                 << basicStats.initialBalance << "\n";
        std::cout << "Current Balance:  $" << basicStats.currentBalance << "\n";
        std::cout << "Total P&L:        $" << basicStats.totalPnL 
                 << " (" << (basicStats.totalPnL / basicStats.initialBalance * 100.0) << "%)\n";
        std::cout << "Win Rate:         " << basicStats.winRate << "%\n";
        std::cout << "Win/Loss:         " << basicStats.winningTrades << "/" 
                 << basicStats.losingTrades << "\n";
        std::cout << "Average R:R:      " << std::setprecision(2) << basicStats.averageRR << "\n";
        std::cout << "Largest Win:      $" << basicStats.largestWin << "\n";
        std::cout << "Largest Loss:     $" << basicStats.largestLoss << "\n\n";
        
        // Display advanced stats
        char showAdvanced = Utils::getYesNoInput("Display advanced statistics? (y/n): ");
        if (showAdvanced == 'y') {
            Utils::clearScreen();
            UI::displayHeader("ADVANCED STATISTICS");
            
            // Use the equity analyzer to get detailed statistics
            Analytics::EquityAnalyzer analyzer;
            auto stats = analyzer.calculateStats(trades, basicStats.initialBalance);
            
            std::cout << analyzer.getStatsReport(stats) << std::endl;
            
            // Display visual representations
            std::cout << "\nWin/Loss Distribution:\n";
            
            // Calculate percentages for visualization
            double winPercent = basicStats.winRate;
            double lossPercent = 100.0 - winPercent;
            
            // Simple bar chart
            const int width = 50;
            int winWidth = static_cast<int>(width * (winPercent / 100.0));
            int lossWidth = width - winWidth;
            
            std::cout << "Wins  [" << std::string(winWidth, '#') 
                     << std::string(lossWidth, ' ') << "] " 
                     << std::setprecision(1) << winPercent << "%\n";
                     
            std::cout << "Losses[" << std::string(lossWidth, '#') 
                     << std::string(winWidth, ' ') << "] " 
                     << lossPercent << "%\n\n";
            
            // Option to export statistics
            char exportStats = Utils::getYesNoInput("Export statistics to file? (y/n): ");
            if (exportStats == 'y') {
                std::string filename;
                std::cout << "Enter filename (without extension): ";
                std::cin >> filename;
                
                // Here you would implement the export functionality
                std::cout << "Export functionality is not yet implemented.\n";
            }
        }
    }
} 