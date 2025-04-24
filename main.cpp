#include "Workflow/MainMenu.h"
#include <iostream>

int main() {
    try {
        Workflow::MainMenu menu;
        menu.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
} 