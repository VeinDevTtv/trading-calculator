#include "Utils.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <ctime>
#include <algorithm>
#include <cmath>

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
    printHeader("SAVED TRADES");
    
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
    
    printFooter();
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

std::vector<std::string> parseCSVLine(const std::string& line) {
    std::vector<std::string> result;
    std::stringstream ss(line);
    std::string item;
    
    while (std::getline(ss, item, ',')) {
        result.push_back(item);
    }
    
    return result;
}

std::vector<std::vector<std::string>> parseCSV(const std::string& filename) {
    std::vector<std::vector<std::string>> data;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        return data;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        data.push_back(parseCSVLine(line));
    }
    
    file.close();
    return data;
}

std::string replaceExtension(const std::string& filename, const std::string& newExtension) {
    size_t lastDot = filename.find_last_of(".");
    if (lastDot == std::string::npos) {
        // No extension found, append the new extension
        return filename + newExtension;
    }
    
    return filename.substr(0, lastDot) + newExtension;
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void printColorText(const std::string& text, int colorCode) {
    std::cout << "\033[1;" << colorCode << "m" << text << "\033[0m";
}

void printHeader(const std::string& title) {
    clearScreen();
    
    std::string line(40, '=');
    printColorText(line + "\n", COLOR_CYAN);
    
    std::string centered = title;
    int padding = (40 - static_cast<int>(title.length())) / 2;
    if (padding > 0) {
        centered = std::string(padding, ' ') + title;
    }
    
    printColorText("  " + centered + "  \n", COLOR_CYAN);
    printColorText(line + "\n", COLOR_CYAN);
    std::cout << std::endl;
}

void printFooter() {
    std::cout << std::endl;
    printColorText(std::string(40, '-') + "\n", COLOR_CYAN);
    std::cout << std::endl;
}

void printError(const std::string& message) {
    printColorText("ERROR: " + message + "\n", COLOR_RED);
}

void printSuccess(const std::string& message) {
    printColorText("SUCCESS: " + message + "\n", COLOR_GREEN);
}

void printWarning(const std::string& message) {
    printColorText("WARNING: " + message + "\n", COLOR_YELLOW);
}

void printInfo(const std::string& message) {
    printColorText("INFO: " + message + "\n", COLOR_BLUE);
}

std::string generateASCIIChart(const std::vector<double>& values, int width, int height) {
    if (values.empty()) {
        return "No data to display";
    }
    
    std::stringstream ss;
    
    // Find min and max values
    auto minmax = std::minmax_element(values.begin(), values.end());
    double min_val = *minmax.first;
    double max_val = *minmax.second;
    
    // Handle case where all values are the same
    if (min_val == max_val) {
        min_val = min_val * 0.9;
        max_val = max_val * 1.1;
    }
    
    // Adjust height to allow for labels
    int chart_height = height - 2;
    
    // Create a 2D grid for the chart
    std::vector<std::string> chart(chart_height, std::string(width, ' '));
    
    // Calculate scale
    double y_scale = (max_val - min_val) / chart_height;
    
    // Draw Y-axis labels
    ss << std::fixed << std::setprecision(0);
    ss << max_val << " ┐" << std::string(width - ss.str().length(), ' ') << std::endl;
    
    // Draw chart
    for (int y = 0; y < chart_height; y++) {
        for (int x = 0; x < std::min(width, static_cast<int>(values.size())); x++) {
            double normalized_value = (values[x] - min_val) / (max_val - min_val);
            int chart_y = chart_height - 1 - static_cast<int>(normalized_value * chart_height);
            
            if (chart_y == y) {
                chart[y][x] = 'o';
            } else if (chart_y < y) {
                chart[y][x] = '│';
            }
        }
        
        ss << chart[y] << std::endl;
    }
    
    // Draw X-axis
    ss << min_val << " └" << std::string(width - 2, '─') << "┘" << std::endl;
    
    return ss.str();
}

} // namespace Utils 