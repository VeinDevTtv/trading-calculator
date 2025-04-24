#include "SettingsHandler.h"
#include "../UI/Menu.h"
#include "../Utils/InputHandler.h"
#include "../Utils.h"
#include <iostream>
#include <string>

namespace Workflow {
    void configureSettings(SessionManager& sessionManager) {
        Utils::clearScreen();
        UI::displayHeader("SETTINGS");
        
        bool exitSettings = false;
        
        while (!exitSettings) {
            std::cout << "1. Auto-save trades: " 
                     << (sessionManager.getAutoSave() ? "Enabled" : "Disabled") << "\n";
            std::cout << "2. Session file: " << sessionManager.getSessionFile() << "\n";
            std::cout << "3. Risk profiles\n";
            std::cout << "4. Journal settings\n";
            std::cout << "5. Backtester settings\n";
            std::cout << "6. Exit settings\n\n";
            
            int choice = Utils::getValidInput<int>("Enter your choice: ", 1, 6, true);
            
            switch (choice) {
                case 1: {
                    // Toggle auto-save
                    bool currentSetting = sessionManager.getAutoSave();
                    sessionManager.setAutoSave(!currentSetting);
                    Utils::printSuccess("Auto-save " + std::string(!currentSetting ? "enabled" : "disabled"));
                    break;
                }
                
                case 2: {
                    // Change session file
                    std::string filename;
                    std::cout << "Enter new session file path: ";
                    std::cin >> filename;
                    sessionManager.setSessionFile(filename);
                    Utils::printSuccess("Session file changed to: " + filename);
                    break;
                }
                
                case 3: {
                    // Risk profile settings
                    Utils::clearScreen();
                    UI::displayHeader("RISK PROFILE SETTINGS");
                    std::cout << "Risk profile settings are not yet implemented.\n";
                    std::cout << "Press Enter to continue...";
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cin.get();
                    break;
                }
                
                case 4: {
                    // Journal settings
                    Utils::clearScreen();
                    UI::displayHeader("JOURNAL SETTINGS");
                    std::cout << "Journal settings are not yet implemented.\n";
                    std::cout << "Press Enter to continue...";
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cin.get();
                    break;
                }
                
                case 5: {
                    // Backtester settings
                    Utils::clearScreen();
                    UI::displayHeader("BACKTESTER SETTINGS");
                    std::cout << "Backtester settings are not yet implemented.\n";
                    std::cout << "Press Enter to continue...";
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cin.get();
                    break;
                }
                
                case 6: {
                    // Exit settings
                    exitSettings = true;
                    break;
                }
            }
            
            if (!exitSettings) {
                Utils::clearScreen();
                UI::displayHeader("SETTINGS");
            }
        }
        
        // Save settings to config file
        TradeParameters params;
        Utils::loadConfig(params);
        Utils::saveConfig(params);
        Utils::printSuccess("Settings saved.");
    }
} 