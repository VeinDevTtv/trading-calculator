#include "ConsoleUI.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <cmath>
#include <thread>
#include <chrono>

namespace UI {

ConsoleUI& ConsoleUI::getInstance() {
    static ConsoleUI instance;
    return instance;
}

void ConsoleUI::clearScreen() const {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void ConsoleUI::printHeader(const std::string& title) const {
    const int width = 80;
    std::cout << "\n" << createHorizontalLine('=', width) << "\n";
    std::cout << centerText(title, width) << "\n";
    std::cout << createHorizontalLine('=', width) << "\n\n";
}

void ConsoleUI::printFooter() const {
    const int width = 80;
    std::cout << "\n" << createHorizontalLine('-', width) << "\n";
}

void ConsoleUI::printSuccess(const std::string& message) const {
    setTextColor(COLOR_GREEN);
    std::cout << "✓ " << message << std::endl;
    resetTextColor();
}

void ConsoleUI::printError(const std::string& message) const {
    setTextColor(COLOR_RED);
    std::cout << "✗ " << message << std::endl;
    resetTextColor();
}

void ConsoleUI::printWarning(const std::string& message) const {
    setTextColor(COLOR_YELLOW);
    std::cout << "⚠ " << message << std::endl;
    resetTextColor();
}

void ConsoleUI::printInfo(const std::string& message) const {
    setTextColor(COLOR_CYAN);
    std::cout << "ℹ " << message << std::endl;
    resetTextColor();
}

void ConsoleUI::printColorText(const std::string& text, int colorCode) const {
    setTextColor(colorCode);
    std::cout << text;
    resetTextColor();
}

void ConsoleUI::displayMenu(const std::vector<std::string>& options) const {
    std::cout << "\n";
    for (size_t i = 0; i < options.size(); ++i) {
        std::cout << std::setw(2) << (i + 1) << ". " << options[i] << "\n";
    }
    std::cout << "\n";
}

void ConsoleUI::displayTradeSummary(const std::string& summary) const {
    const int width = 80;
    std::cout << "\n" << createHorizontalLine('-', width) << "\n";
    std::cout << summary;
    std::cout << createHorizontalLine('-', width) << "\n\n";
}

void ConsoleUI::displayTradeList(const std::vector<std::string>& trades) const {
    const int width = 80;
    std::cout << "\n" << createHorizontalLine('-', width) << "\n";
    for (const auto& trade : trades) {
        std::cout << trade << "\n";
    }
    std::cout << createHorizontalLine('-', width) << "\n\n";
}

void ConsoleUI::displayEquityCurve(const std::vector<double>& values, int width, int height) const {
    if (values.empty()) {
        printWarning("No data to display.");
        return;
    }

    // Find min and max values
    double minVal = *std::min_element(values.begin(), values.end());
    double maxVal = *std::max_element(values.begin(), values.end());
    double range = maxVal - minVal;

    // Create the chart
    std::vector<std::string> chart(height, std::string(width, ' '));
    
    // Plot points
    for (size_t i = 0; i < values.size(); ++i) {
        int x = static_cast<int>((i * (width - 1)) / (values.size() - 1));
        int y = static_cast<int>(((values[i] - minVal) * (height - 1)) / range);
        chart[height - 1 - y][x] = '●';
    }

    // Draw the chart
    std::cout << "\n";
    for (const auto& row : chart) {
        std::cout << "│ " << row << " │\n";
    }
    std::cout << "└" << std::string(width, '─') << "┘\n";
}

void ConsoleUI::showProgressBar(int current, int total) const {
    const int width = 50;
    float progress = static_cast<float>(current) / total;
    int pos = static_cast<int>(width * progress);

    std::cout << "[";
    for (int i = 0; i < width; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << static_cast<int>(progress * 100.0) << "%\r";
    std::cout.flush();
}

void ConsoleUI::showSpinner() const {
    static const char spinner[] = {'|', '/', '-', '\\'};
    static int spinnerIndex = 0;
    
    std::cout << "\r" << spinner[spinnerIndex] << " Processing... ";
    spinnerIndex = (spinnerIndex + 1) % 4;
    std::cout.flush();
}

void ConsoleUI::displayTable(const std::vector<std::string>& headers,
                           const std::vector<std::vector<std::string>>& rows) const {
    if (headers.empty() || rows.empty()) return;

    // Calculate column widths
    std::vector<size_t> colWidths(headers.size());
    for (size_t i = 0; i < headers.size(); ++i) {
        colWidths[i] = headers[i].length();
        for (const auto& row : rows) {
            if (i < row.size()) {
                colWidths[i] = std::max(colWidths[i], row[i].length());
            }
        }
    }

    // Print header
    std::cout << "\n";
    for (size_t i = 0; i < headers.size(); ++i) {
        std::cout << std::setw(colWidths[i]) << headers[i] << " | ";
    }
    std::cout << "\n";

    // Print separator
    for (size_t i = 0; i < headers.size(); ++i) {
        std::cout << std::string(colWidths[i], '-') << "-+-";
    }
    std::cout << "\n";

    // Print rows
    for (const auto& row : rows) {
        for (size_t i = 0; i < headers.size(); ++i) {
            std::cout << std::setw(colWidths[i]) << (i < row.size() ? row[i] : "") << " | ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

std::string ConsoleUI::getInput(const std::string& prompt) const {
    std::string input;
    std::cout << prompt;
    std::getline(std::cin, input);
    return input;
}

char ConsoleUI::getYesNoInput(const std::string& prompt) const {
    char response;
    bool valid = false;
    
    do {
        std::cout << prompt;
        std::cin >> response;
        response = std::tolower(response);
        
        if (response != 'y' && response != 'n') {
            printError("Please enter 'y' or 'n'.");
            std::cin.clear();
        } else {
            valid = true;
        }
        
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } while (!valid);
    
    return response;
}

template<typename T>
T ConsoleUI::getValidInput(const std::string& prompt, T min, T max, bool hasRange) const {
    T value;
    bool valid = false;
    
    while (!valid) {
        std::cout << prompt;
        if (std::cin >> value) {
            if (!hasRange || (value >= min && value <= max)) {
                valid = true;
            } else {
                printError("Value must be between " + std::to_string(min) + 
                          " and " + std::to_string(max));
            }
        } else {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            printError("Invalid input. Please enter a valid number.");
        }
    }
    
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return value;
}

void ConsoleUI::setTextColor(int colorCode) const {
    std::cout << "\033[" << colorCode << "m";
}

void ConsoleUI::resetTextColor() const {
    std::cout << "\033[0m";
}

std::string ConsoleUI::centerText(const std::string& text, int width) const {
    int padding = (width - text.length()) / 2;
    return std::string(padding, ' ') + text + std::string(width - text.length() - padding, ' ');
}

std::string ConsoleUI::createHorizontalLine(char character, int length) const {
    return std::string(length, character);
}

} // namespace UI 