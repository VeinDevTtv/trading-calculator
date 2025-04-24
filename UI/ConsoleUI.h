#ifndef CONSOLE_UI_H
#define CONSOLE_UI_H

#include <string>
#include <vector>
#include <memory>

namespace UI {
    class ConsoleUI {
    public:
        static ConsoleUI& getInstance();
        
        // Screen management
        void clearScreen() const;
        void printHeader(const std::string& title) const;
        void printFooter() const;
        
        // Text formatting
        void printSuccess(const std::string& message) const;
        void printError(const std::string& message) const;
        void printWarning(const std::string& message) const;
        void printInfo(const std::string& message) const;
        void printColorText(const std::string& text, int colorCode) const;
        
        // Menu display
        void displayMenu(const std::vector<std::string>& options) const;
        void displayTradeSummary(const std::string& summary) const;
        void displayTradeList(const std::vector<std::string>& trades) const;
        void displayEquityCurve(const std::vector<double>& values, int width = 50, int height = 15) const;
        
        // Progress indicators
        void showProgressBar(int current, int total) const;
        void showSpinner() const;
        
        // Table display
        void displayTable(const std::vector<std::string>& headers, 
                         const std::vector<std::vector<std::string>>& rows) const;
        
        // Input helpers
        std::string getInput(const std::string& prompt) const;
        char getYesNoInput(const std::string& prompt) const;
        template<typename T>
        T getValidInput(const std::string& prompt, T min, T max, bool hasRange) const;
        
    private:
        ConsoleUI() = default;
        ~ConsoleUI() = default;
        ConsoleUI(const ConsoleUI&) = delete;
        ConsoleUI& operator=(const ConsoleUI&) = delete;
        
        // Color codes
        static const int COLOR_RED = 31;
        static const int COLOR_GREEN = 32;
        static const int COLOR_YELLOW = 33;
        static const int COLOR_BLUE = 34;
        static const int COLOR_MAGENTA = 35;
        static const int COLOR_CYAN = 36;
        static const int COLOR_WHITE = 37;
        
        // Helper methods
        void setTextColor(int colorCode) const;
        void resetTextColor() const;
        std::string centerText(const std::string& text, int width) const;
        std::string createHorizontalLine(char character, int length) const;
    };
}

#endif // CONSOLE_UI_H 