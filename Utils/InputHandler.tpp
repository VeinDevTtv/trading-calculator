#include <iostream>
#include <limits>
#include <string>
#include "../Utils.h"

namespace Utils {
    template<typename T>
    T getValidInput(const std::string& prompt, T min, T max, bool hasRange) {
        T input;
        bool valid = false;
        
        while (!valid) {
            std::cout << prompt;
            
            if (std::cin >> input) {
                // Input was successfully read
                if (hasRange && (input < min || input > max)) {
                    Utils::printError("Input must be between " + std::to_string(min) + " and " + std::to_string(max));
                } else {
                    valid = true;
                }
            } else {
                // Input failed, clear the error flag
                std::cin.clear();
                Utils::printError("Invalid input. Please try again.");
            }
            
            // Clear the input buffer
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        
        return input;
    }
} 