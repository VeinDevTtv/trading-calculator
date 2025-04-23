#ifndef UTILS_H
#define UTILS_H

#include "TradeCalculator.h"
#include <string>
#include <vector>

namespace Utils {
    // File operations
    bool saveTradeToFile(const TradeParameters& params, const TradeResults& results);
    bool loadConfig(TradeParameters& params);
    bool saveConfig(const TradeParameters& params);
    void displaySavedTrades();
    
    // String formatting utilities
    std::string getFormattedTimestamp(std::time_t timestamp);
    std::string getInstrumentTypeString(InstrumentType type);
    std::string getLotSizeTypeString(LotSizeType type);
    
    // Config file paths
    const std::string CONFIG_FILE = "config.json";
    const std::string TRADES_FILE = "trades.csv";
    
    // Enhanced utilities
    std::vector<std::string> parseCSVLine(const std::string& line);
    std::vector<std::vector<std::string>> parseCSV(const std::string& filename);
    std::string replaceExtension(const std::string& filename, const std::string& newExtension);
    
    // Console UI utilities
    void clearScreen();
    void printColorText(const std::string& text, int colorCode);
    void printHeader(const std::string& title);
    void printFooter();
    void printError(const std::string& message);
    void printSuccess(const std::string& message);
    void printWarning(const std::string& message);
    void printInfo(const std::string& message);
    
    // Color codes
    const int COLOR_RED = 31;
    const int COLOR_GREEN = 32;
    const int COLOR_YELLOW = 33;
    const int COLOR_BLUE = 34;
    const int COLOR_MAGENTA = 35;
    const int COLOR_CYAN = 36;
    const int COLOR_WHITE = 37;
    
    // ASCII chart generation
    std::string generateASCIIChart(const std::vector<double>& values, int width = 50, int height = 10);
}

#endif // UTILS_H 