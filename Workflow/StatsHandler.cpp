#include "StatsHandler.h"
#include "../UI/Menu.h"
#include "../Utils/InputHandler.h"
#include "../Utils.h"
#include "../Analytics/EquityStats.h"
#include <iostream>
#include <iomanip>

namespace Workflow {
    void sessionStatsMode(SessionManager& sessionManager) {
        Utils::printHeader("SESSION STATISTICS");
        
        if (!sessionManager.isSessionActive()) {
            Utils::printError("No active session. Please start a session first.");
            return;
        }
        
        std::cout << sessionManager.getSessionSummary() << std::endl;
        
        // Option to save session
        char saveSession = Utils::getYesNoInput("Save current session? (y/n): ");
        if (saveSession == 'y') {
            sessionManager.saveSession();
            sessionManager.saveSessionAsJson();
            Utils::printSuccess("Session saved!");
        }
    }
} 