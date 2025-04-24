#ifndef UTILS_INPUT_HANDLER_H
#define UTILS_INPUT_HANDLER_H

#include <string>

namespace Utils {
    /**
     * Get a valid input of type T from the user
     * @param prompt The prompt to display to the user
     * @param min The minimum allowed value (optional)
     * @param max The maximum allowed value (optional)
     * @param hasRange Whether to enforce min/max range (optional)
     * @return The validated input value
     */
    template<typename T>
    T getValidInput(const std::string& prompt, T min = T(), T max = T(), bool hasRange = false);

    /**
     * Get a yes/no input from the user
     * @param prompt The prompt to display to the user
     * @return 'y' for yes, 'n' for no
     */
    char getYesNoInput(const std::string& prompt);
}

#include "InputHandler.tpp"

#endif // UTILS_INPUT_HANDLER_H 