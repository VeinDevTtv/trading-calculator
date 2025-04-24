#include "InputHandler.h"
#include "../Utils.h"
#include <iostream>
#include <limits>

namespace Utils {
    char getYesNoInput(const std::string& prompt) {
        char input;
        bool valid = false;
        
        while (!valid) {
            std::cout << prompt;
            
            if (std::cin >> input) {
                input = std::tolower(input);
                if (input == 'y' || input == 'n') {
                    valid = true;
                } else {
                    Utils::printError("Please enter 'y' for yes or 'n' for no.");
                }
            } else {
                std::cin.clear();
                Utils::printError("Invalid input. Please try again.");
            }
            
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        
        return input;
    }
} 