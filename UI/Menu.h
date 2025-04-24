#ifndef UI_MENU_H
#define UI_MENU_H

#include <string>
#include "../SessionManager.h"

namespace UI {
    /**
     * Main menu display and handler
     */
    void displayMainMenu();
    
    /**
     * Displays a formatted header with a title
     * @param title The title to display in the header
     */
    void displayHeader(const std::string& title);
    
    /**
     * Displays a formatted footer
     */
    void displayFooter();
}

#endif // UI_MENU_H 