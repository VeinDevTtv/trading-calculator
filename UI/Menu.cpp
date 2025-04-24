#include "Menu.h"
#include "../Utils.h"
#include <iostream>

namespace UI {
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
    
    void displayHeader(const std::string& title) {
        Utils::printHeader(title);
    }
    
    void displayFooter() {
        Utils::printFooter();
    }
} 