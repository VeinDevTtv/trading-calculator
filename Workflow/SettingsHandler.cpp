#include "SettingsHandler.h"
#include "../UI/Menu.h"
#include "../Utils/InputHandler.h"
#include "../Utils.h"
#include <iostream>
#include <string>

namespace Workflow {
    void configureSettings(SessionManager& sessionManager) {
        Utils::printHeader("SETTINGS");
        
        bool done = false;
        while (!done) {
            std::cout << "1. Toggle Auto-Save (current: " << (sessionManager.getAutoSave() ? "ON" : "OFF") << ")\n";
            std::cout << "2. Change Session File (current: " << sessionManager.getSessionFile() << ")\n";
            std::cout << "3. Return to Main Menu\n\n";
            
            int choice = Utils::getValidInput<int>("Enter your choice: ", 1, 3, true);
            
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
        
        // Save settings to config file
        TradeParameters params;
        Utils::loadConfig(params);
        Utils::saveConfig(params);
        Utils::printSuccess("Settings saved.");
    }
} 