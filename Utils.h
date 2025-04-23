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
    
    // Internal utility to parse CSV
    std::vector<std::vector<std::string>> parseCSV(const std::string& filename);
}

#endif // UTILS_H 