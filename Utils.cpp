#include "Utils.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <ctime>

namespace Utils {

bool saveTradeToFile(const TradeParameters& params, const TradeResults& results) {
    bool isNewFile = false;
    std::ofstream file;
    
    // Check if file exists
    std::ifstream checkFile(TRADES_FILE);
    isNewFile = !checkFile.good();
    checkFile.close();
    
    // Open file for append
    file.open(TRADES_FILE, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open trades file for writing." << std::endl;
        return false;
    }
    
    // Write header if new file
    if (isNewFile) {
        file << "Date,Time,Account Balance,Risk %,Risk Amount,Reward Amount,RR Ratio,"
             << "Entry Price,SL Price,TP Price,SL Pips,TP Pips,Position Size,Instrument,Lot Type" << std::endl;
    }
    
    // Format timestamp
    std::string timestamp = getFormattedTimestamp(params.timestamp);
    
    // Format and write trade data
    file << timestamp << ","
         << params.accountBalance << ","
         << params.riskPercent << ","
         << results.riskAmount << ","
         << results.rewardAmount << ","
         << results.riskRewardRatio << ","
         << params.entryPrice << ","
         << results.stopLossPrice << ","
         << results.takeProfitPrice << ","
         << params.stopLossInPips << ","
         << params.takeProfitInPips << ","
         << results.positionSize << ","
         << getInstrumentTypeString(params.instrumentType) << ","
         << getLotSizeTypeString(params.lotSizeType) << std::endl;
    
    file.close();
    return true;
}

// Parse a JSON value from a string - very simple implementation
std::string parseJsonValue(const std::string& json, const std::string& key) {
    std::string keySearch = "\"" + key + "\"";
    size_t keyPos = json.find(keySearch);
    if (keyPos == std::string::npos) {
        return "";
    }
    
    size_t colonPos = json.find(":", keyPos);
    if (colonPos == std::string::npos) {
        return "";
    }
    
    size_t valueStart = json.find_first_not_of(" \t\n\r", colonPos + 1);
    if (valueStart == std::string::npos) {
        return "";
    }
    
    // Check if value is a string
    if (json[valueStart] == '"') {
        size_t valueEnd = json.find("\"", valueStart + 1);
        if (valueEnd == std::string::npos) {
            return "";
        }
        return json.substr(valueStart + 1, valueEnd - valueStart - 1);
    }
    
    // Assume value is a number or boolean
    size_t valueEnd = json.find_first_of(",}\n", valueStart);
    if (valueEnd == std::string::npos) {
        valueEnd = json.length();
    }
    
    return json.substr(valueStart, valueEnd - valueStart);
}

bool loadConfig(TradeParameters& params) {
    std::ifstream file(CONFIG_FILE);
    if (!file.is_open()) {
        // Config file doesn't exist yet, just return false
        return false;
    }
    
    // Read the entire file
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string jsonContent = buffer.str();
    file.close();
    
    // Parse JSON values
    std::string accountBalanceStr = parseJsonValue(jsonContent, "accountBalance");
    if (!accountBalanceStr.empty()) {
        params.accountBalance = std::stod(accountBalanceStr);
    }
    
    std::string riskPercentStr = parseJsonValue(jsonContent, "riskPercent");
    if (!riskPercentStr.empty()) {
        params.riskPercent = std::stod(riskPercentStr);
    }
    
    std::string instrumentTypeStr = parseJsonValue(jsonContent, "instrumentType");
    if (!instrumentTypeStr.empty()) {
        int type = std::stoi(instrumentTypeStr);
        params.instrumentType = static_cast<InstrumentType>(type);
    }
    
    std::string lotSizeTypeStr = parseJsonValue(jsonContent, "lotSizeType");
    if (!lotSizeTypeStr.empty()) {
        int type = std::stoi(lotSizeTypeStr);
        params.lotSizeType = static_cast<LotSizeType>(type);
    }
    
    return true;
}

bool saveConfig(const TradeParameters& params) {
    std::ofstream file(CONFIG_FILE);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open config file for writing." << std::endl;
        return false;
    }
    
    // Write JSON format
    file << "{\n";
    file << "  \"accountBalance\": " << params.accountBalance << ",\n";
    file << "  \"riskPercent\": " << params.riskPercent << ",\n";
    file << "  \"instrumentType\": " << static_cast<int>(params.instrumentType) << ",\n";
    file << "  \"lotSizeType\": " << static_cast<int>(params.lotSizeType) << "\n";
    file << "}" << std::endl;
    
    file.close();
    return true;
}

void displaySavedTrades() {
    // Check if file exists
    std::ifstream checkFile(TRADES_FILE);
    if (!checkFile.good()) {
        std::cout << "No saved trades found." << std::endl;
        return;
    }
    checkFile.close();
    
    // Parse CSV file
    std::vector<std::vector<std::string>> trades = parseCSV(TRADES_FILE);
    
    if (trades.size() <= 1) {
        std::cout << "No trades found in the file." << std::endl;
        return;
    }
    
    // Display header
    std::cout << "\n\033[1;36m"; // Cyan color
    std::cout << "=== SAVED TRADES ===\n";
    std::cout << "\033[0m"; // Reset color
    
    // Print table header
    std::cout << std::left 
              << std::setw(20) << "Date/Time" 
              << std::setw(12) << "Risk $" 
              << std::setw(12) << "Reward $" 
              << std::setw(10) << "RR Ratio" 
              << std::setw(10) << "Position" 
              << std::setw(10) << "SL Pips"
              << std::setw(10) << "TP Pips"
              << std::setw(10) << "Instrument"
              << std::endl;
    
    std::cout << std::string(94, '-') << std::endl;
    
    // Print each trade (skip header row)
    for (size_t i = 1; i < trades.size(); i++) {
        const auto& trade = trades[i];
        if (trade.size() >= 14) { // Ensure we have enough columns
            std::cout << std::left 
                      << std::setw(20) << trade[0] 
                      << std::setw(12) << trade[4] 
                      << std::setw(12) << trade[5] 
                      << std::setw(10) << trade[6] 
                      << std::setw(10) << trade[12] 
                      << std::setw(10) << trade[10]
                      << std::setw(10) << trade[11]
                      << std::setw(10) << trade[13]
                      << std::endl;
        }
    }
}

std::string getFormattedTimestamp(std::time_t timestamp) {
    std::tm* tm = std::localtime(&timestamp);
    std::ostringstream oss;
    oss << std::put_time(tm, "%Y-%m-%d,%H:%M:%S");
    return oss.str();
}

std::string getInstrumentTypeString(InstrumentType type) {
    switch(type) {
        case InstrumentType::Forex: return "Forex";
        case InstrumentType::Gold: return "Gold";
        case InstrumentType::Indices: return "Indices";
        default: return "Unknown";
    }
}

std::string getLotSizeTypeString(LotSizeType type) {
    switch(type) {
        case LotSizeType::Standard: return "Standard";
        case LotSizeType::Mini: return "Mini";
        case LotSizeType::Micro: return "Micro";
        default: return "Unknown";
    }
}

std::vector<std::vector<std::string>> parseCSV(const std::string& filename) {
    std::vector<std::vector<std::string>> data;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        return data;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        std::vector<std::string> row;
        std::string cell;
        std::istringstream lineStream(line);
        
        while (std::getline(lineStream, cell, ',')) {
            row.push_back(cell);
        }
        
        data.push_back(row);
    }
    
    file.close();
    return data;
}

} // namespace Utils 