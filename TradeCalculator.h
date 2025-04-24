#ifndef TRADE_CALCULATOR_H
#define TRADE_CALCULATOR_H

#include <string>
#include <ctime>
#include <unordered_map>
#include <mutex>

// Enums for instrument and lot size types
enum class InstrumentType {
    Forex,
    Gold,
    Indices
};

enum class LotSizeType {
    Standard,
    Mini,
    Micro
};

// Structure to hold all trade parameters
struct TradeParameters {
    double accountBalance = 10000.0;
    double riskPercent = 1.0;
    double stopLossInPips = 0.0;
    double takeProfitInPips = 0.0;
    double riskRewardRatio = 0.0;
    double entryPrice = 0.0;
    double stopLossPrice = 0.0;
    bool isStopLossPriceOverride = false;
    InstrumentType instrumentType = InstrumentType::Forex;
    LotSizeType lotSizeType = LotSizeType::Standard;
    double contractSize = 0.0; // Custom value for non-standard instruments
    std::time_t timestamp = std::time(nullptr);
};

// Structure to hold calculation results
struct TradeResults {
    double riskAmount = 0.0;
    double rewardAmount = 0.0;
    double positionSize = 0.0;
    double stopLossPrice = 0.0;
    double takeProfitPrice = 0.0;
    double riskRewardRatio = 0.0;
    double pipValue = 0.0;
    
    // Breakeven information
    bool hasBreakEvenInfo = false;
    double breakEvenPrice = 0.0;
    double breakEvenPips = 0.0;
    
    // Multiple targets support
    bool hasMultipleTargets = false;
    double tp1Price = 0.0;
    double tp2Price = 0.0;
    double tp1Amount = 0.0;
    double tp2Amount = 0.0;
};

class TradeCalculator {
public:
    TradeCalculator();
    ~TradeCalculator();
    
    // Main calculation method
    TradeResults calculateTrade(const TradeParameters& params);
    
    // Set fee/spread information
    void setFeePercentage(double feePercent);
    void setFixedSpreadPips(double spreadPips);
    
    // Enable multiple targets
    TradeResults calculateMultipleTargets(const TradeParameters& params, 
                                         double tp1Percent, double tp2Percent);
    
    // Clear cache
    void clearCache();
    
private:
    // Helper methods
    double calculatePipValue(const TradeParameters& params);
    double calculateStopLossPrice(const TradeParameters& params);
    double calculateTakeProfitPrice(const TradeParameters& params);
    double calculateBreakEvenPoint(const TradeParameters& params);
    
    // Caching
    struct CacheKey {
        InstrumentType instrumentType;
        LotSizeType lotSizeType;
        double contractSize;
        
        bool operator==(const CacheKey& other) const {
            return instrumentType == other.instrumentType &&
                   lotSizeType == other.lotSizeType &&
                   contractSize == other.contractSize;
        }
    };
    
    struct CacheKeyHash {
        std::size_t operator()(const CacheKey& key) const {
            return std::hash<int>()(static_cast<int>(key.instrumentType)) ^
                   (std::hash<int>()(static_cast<int>(key.lotSizeType)) << 1) ^
                   (std::hash<double>()(key.contractSize) << 2);
        }
    };
    
    std::unordered_map<CacheKey, double, CacheKeyHash> m_pipValueCache;
    std::mutex m_cacheMutex;
    
    // Default fee/spread settings
    double m_feePercentage = 0.0;
    double m_spreadPips = 0.0;
    
    // Constants
    static constexpr double DEFAULT_PIP_VALUE = 0.0001;
    static constexpr double GOLD_PIP_VALUE = 0.01;
    static constexpr double INDICES_PIP_VALUE = 1.0;
};

#endif // TRADE_CALCULATOR_H 