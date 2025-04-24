#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "SessionManager.h"
#include <memory>

namespace Workflow {
    class MainMenu {
    public:
        MainMenu();
        ~MainMenu();

        void run();
        void displayMainMenu() const;
        void handleUserChoice(int choice);
        void newTradeWorkflow();
        void viewSavedTrades();
        void simulationMode();
        void sessionStatsMode();
        void configureSettings();
        void displayEquityCurve();
        void exitProgram();

    private:
        std::unique_ptr<SessionManager> m_sessionManager;
        bool m_running;
        
        void initializeSession();
        void saveSessionIfNeeded();
    };
}

#endif // MAIN_MENU_H 